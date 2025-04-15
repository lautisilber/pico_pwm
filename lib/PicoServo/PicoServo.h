/*
    PIO-based Servo class for Rasperry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

/*
    A servo is activated by creating an instance of the Servo class passing
    the desired pin to the attach() method.
    The servos are pulsed in the background using the value most recently
    written using the write() method.

    The methods are:
     Servo - Class for manipulating servo motors connected to Arduino pins.
       attach(pin)  - Attaches a servo motor to an i/o pin.
       attach(pin, min, max) - Attaches to a pin setting min and max values in microseconds
                               default min is 1000, max is 2000
       write()     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
       writeMicroseconds() - Sets the servo pulse width in microseconds
       read()      - Gets the last written servo pulse width as an angle between 0 and 180.
       readMicroseconds()   - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
       attached()  - Returns true if there is a servo attached.
       detach()    - Stops an attached servos from pulsing its i/o pin.
*/

#ifndef _PICO_SERVO_H_
#define _PICO_SERVO_H_

#include <pico/stdlib.h>
#include <hardware/pio.h>

// The following values are in us (microseconds).
// Since the defaults can be overwritten in the new attach() member function,
// they were modified from the Arduino AVR defaults to be in the safe range
// of publicly available specifications. While this implies that many 180°
// servos do not operate the full 0° to 180° sweep using these, it also prevents
// unsuspecting damage. For Arduino AVR, the same change is being discussed.
#define DEFAULT_MIN_PULSE_WIDTH      1000 // uncalibrated default, the shortest duty cycle sent to a servo
#define DEFAULT_MAX_PULSE_WIDTH      2000 // uncalibrated default, the longest duty cycle sent to a servo 
#define DEFAULT_NEUTRAL_PULSE_WIDTH  1500 // default duty cycle when servo is attached
#define REFRESH_INTERVAL            20000 // classic default period to refresh servos in microseconds 
#define MAX_SERVOS                      8 // number of PIO state machines available, assuming nobody else is using them


class PicoServo {
public:
    PicoServo(uint8_t pin);
    ~PicoServo();

    void init() const;

    // attach the given pin to the next free channel, sets pinMode, returns channel number or 0 if failure.
    // returns channel number or 0 if failure.
    bool attach();

    // attach the given pin to the next free channel, sets pinMode, min, and max values for write().
    // returns channel number or 0 if failure.
    bool attach(uint32_t min, uint32_t max);

    // attach the given pin to the next free channel, sets pinMode, min, and max values for write(),
    // and sets the initial value, the same as write().
    // returns channel number or 0 if failure.
    bool attach(uint32_t min, uint32_t max, uint32_t value);

    void detach();
    void write(uint8_t angle);              // if value is < 200 its treated as an angle, otherwise as pulse width in microseconds
    void writeMicroseconds(uint32_t value); // Write pulse width in microseconds
    bool attached();                        // return true if this servo is attached, otherwise false

private:
    bool     _attached = false;
    const uint8_t  _pin;
    uint32_t _minUs = DEFAULT_MIN_PULSE_WIDTH;
    uint32_t _maxUs = DEFAULT_MAX_PULSE_WIDTH;
    uint32_t _valueUs = DEFAULT_NEUTRAL_PULSE_WIDTH;

    struct PIOProgram
    {
        const pio_program_t *pgm;
        PIO pio = nullptr;
        uint sm = 0;
        uint offset = 0;

        PIOProgram(const pio_program_t *_pgm) : pgm(_pgm) {}
    };
    PIOProgram _pio_program;
};

#endif /* _PICO_SERVO_H_ */