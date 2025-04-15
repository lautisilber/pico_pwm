/*
    PIO-based Servo class for Rasperry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
    Original Copyright (c) 2015 Michael C. Miller. All right reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "PicoServo.h"

#include <hardware/clocks.h>
#include <hardware/pio.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

///// Arduino bindings //////
#define LOW 0
#define HIGH 1

#define INPUT  GPIO_IN
#define OUTPUT GPIO_OUT

static inline void digitalWrite(uint8_t pin, bool state)
{
    gpio_put(pin, state);
}

static inline void delayMicroseconds(uint32_t us)
{
    sleep_us(us);
}

static inline int usToPIOCycles(int us) {
    // Parenthesis needed to guarantee order of operations to avoid 32bit overflow
    return (us * (clock_get_hz(clk_sys) / 1'000'000));
}

static inline bool pio_program_prepare(const pio_program *prg, PIO *pio, uint *sm, uint *offset, uint pin)
{
    bool res = pio_claim_free_sm_and_add_program_for_gpio_range(prg, pio, sm, offset, pin, 1, true); // maybe gpio_count is 2?
    // hard_assert(res);
    return res;
}

/////////////////////////////

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

#include "pico_servo.pio.h"
// static PIOProgram _servoPgm(&servo_program);

// Similar to map but will have increased accuracy that provides a more
// symmetrical api (call it and use result to reverse will provide the original value)
int improved_map(int value, int minIn, int maxIn, int minOut, int maxOut) {
    const int rangeIn = maxIn - minIn;
    const int rangeOut = maxOut - minOut;
    const int deltaIn = value - minIn;
    // fixed point math constants to improve accuracy of divide and rounding
    constexpr int fixedHalfDecimal = 1;
    constexpr int fixedDecimal = fixedHalfDecimal * 2;

    return ((deltaIn * rangeOut * fixedDecimal) / (rangeIn) + fixedHalfDecimal) / fixedDecimal + minOut;
}

//-------------------------------------------------------------------
// PicoServo class methods

PicoServo::PicoServo(uint8_t pin) 
: _pin(pin), _pio_program(&servo_program)
{}

PicoServo::~PicoServo() {
    detach();
}

void PicoServo::init() const
{
    gpio_init(_pin);
    gpio_set_dir(_pin, GPIO_OUT);
}

bool PicoServo::attach() {
    return attach(DEFAULT_MIN_PULSE_WIDTH, DEFAULT_MAX_PULSE_WIDTH);
}

bool PicoServo::attach(int minUs, int maxUs) {
    return attach(minUs, maxUs, _valueUs);
}

bool PicoServo::attach(int minUs, int maxUs, int value) {
    // keep the min and max within 200-3000 us, these are extreme
    // ranges and should support extreme servos while maintaining
    // reasonable ranges
    _maxUs = max(250, min(3000, maxUs));
    _minUs = max(200, min(_maxUs, minUs));

    if (!_attached) {

        // if (!_servoPgm.prepare(&_pio, &_smIdx, &_pgmOffset, pin, 1)) {
        //     // ERROR, no free slots
        //     return -1;
        // }

        bool res = pio_program_prepare(_pio_program.pgm, &(_pio_program.pio), &(_pio_program.sm), &(_pio_program.offset), _pin);
        return false;
        
        _attached = true;
        servo_program_init(_pio_program.pio, _pio_program.sm, _pio_program.offset, _pin);
        pio_sm_set_enabled(_pio_program.pio, _pio_program.sm, false);
        pio_sm_put_blocking(_pio_program.pio, _pio_program.sm, usToPIOCycles(REFRESH_INTERVAL) / 3);
        pio_sm_exec(_pio_program.pio, _pio_program.sm, pio_encode_pull(false, false));
        pio_sm_exec(_pio_program.pio, _pio_program.sm, pio_encode_out(pio_isr, 32));
        write(value);
        pio_sm_exec(_pio_program.pio, _pio_program.sm, pio_encode_pull(false, false));
        pio_sm_exec(_pio_program.pio, _pio_program.sm, pio_encode_mov(pio_x, pio_osr));
        pio_sm_set_enabled(_pio_program.pio, _pio_program.sm, true);
    }

    write(value);

    return true;
}

void PicoServo::detach() {
    if (_attached) {
        // Set a 0 for the width and then wait for the halt loop
        pio_sm_put_blocking(_pio_program.pio, _pio_program.sm, 0);
        delayMicroseconds(5);  // Avoid race condition
        do {
            // Do nothing until we are stuck in the halt loop (avoid short pulses
        } while (pio_sm_get_pc(_pio_program.pio, _pio_program.sm) != servo_offset_halt + _pio_program.offset);
        pio_sm_set_enabled(_pio_program.pio, _pio_program.sm, false);
        pio_sm_unclaim(_pio_program.pio, _pio_program.sm);
        _attached = false;
        _valueUs = DEFAULT_NEUTRAL_PULSE_WIDTH;
    }
}

void PicoServo::write(int value) {
    // treat any value less than 200 as angle in degrees (values equal or larger are handled as microseconds)
    if (value < 200) {
        // assumed to be 0-180 degrees servo
        value = constrain(value, 0, 180);
        value = improved_map(value, 0, 180, _minUs, _maxUs);
    }
    writeMicroseconds(value);
}

void PicoServo::writeMicroseconds(int value) {
    value = constrain(value, _minUs, _maxUs);
    _valueUs = value;
    if (_attached) {
        pio_sm_clear_fifos(_pio_program.pio, _pio_program.sm); // Remove any old updates that haven't yet taken effect
        pio_sm_put_blocking(_pio_program.pio, _pio_program.sm, usToPIOCycles(value) / 3);
    }
}

bool PicoServo::attached() {
    return _attached;
}