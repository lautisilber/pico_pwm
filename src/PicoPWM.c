#include "PicoPWM.h"

#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=1
// https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2

#define MAX_UINT16_T 65535
#define S_TO_NS 1000000

static uint32_t pwm_source_clock_hz = 0;
#define PWM_GLOBAL_INIT() (pwm_source_clock_hz != 0)

static inline bool pwm_has_init_unsafe(struct PicoPWM *pwm)
{
    return pwm->init_flag && PWM_GLOBAL_INIT();
}

void pico_pwm_global_init()
{
    if (PWM_GLOBAL_INIT()) return;
    pwm_source_clock_hz = clock_get_hz(clk_sys);
}

bool pico_pwm_hw_enable_unsafe(struct PicoPWM *pwm, bool enable)
{
    if (!pwm_has_init_unsafe(pwm) || pwm->pwm_hw_init == enable)
        return false;
    pwm_set_enabled(pwm->slice_num, enable);
    pwm->pwm_hw_init = enable;
    return true;
}

bool pico_pwm_hw_enable(struct PicoPWM *pwm, bool enable)
{
    if (!mutex_enter_timeout_ms(&pwm->mux, 1000)) return false;
    bool res = pico_pwm_hw_enable_unsafe(pwm, enable);
    mutex_exit(&pwm->mux);
    return true;
}

void pico_pwm_init(struct PicoPWM *pwm, uint8_t pin, bool inverted)
{
    if (!PWM_GLOBAL_INIT())
        pico_pwm_global_init();

    if (!mutex_is_initialized(&pwm->mux))
        mutex_init(&pwm->mux);

    mutex_enter_blocking(&pwm->mux);

    pwm->pin = pin;
    pwm->inverted = inverted;

    pwm->slice_num = pwm_gpio_to_slice_num(pwm->pin);
    pwm->channel = pwm_gpio_to_channel(pwm->pin);

    gpio_set_function(pwm->pin, GPIO_FUNC_PWM);

    pwm->init_flag = true;
    mutex_exit(&pwm->mux);
}

static inline void pwm_get_div_int_frac_from_div16(uint8_t *div_int, uint8_t *div_frac, uint32_t divider16)
{
    *div_int = (uint8_t)(divider16 / 16);
    *div_frac = (uint8_t)(divider16 & 0xF);
}

static void inline pwm_set_duty_unsafe(struct PicoPWM *pwm, uint16_t d)
{
    pwm->duty_cycle = d;
    pwm->config_level = (uint16_t)(pwm->config_wrap * d / MAX_UINT16_T);
    if (pwm->inverted)
        pwm->config_level = pwm->config_wrap - pwm->config_level;
}

static void pwm_set_freq_duty_unsafe(struct PicoPWM *pwm, uint32_t f, uint16_t d)
{
    // https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
    if (!PWM_GLOBAL_INIT()) return;
    pwm->freq = f;
    pwm->config_divider16 = pwm_source_clock_hz / f / 4096 +
                         (pwm_source_clock_hz % (f * 4096) != 0);
    if (pwm->config_divider16 / 16 == 0)
    pwm->config_divider16 = 16;
    pwm->config_wrap = (uint16_t)(pwm_source_clock_hz * 16 / pwm->config_divider16 / f - 1);
    // pwm->config_level = (uint16_t)(pwm->config_wrap * d / 100);
    pwm_set_duty_unsafe(pwm, d);
}

static inline uint16_t duty_cycle_from_ns(uint32_t frequency, uint16_t wrap, uint32_t ns)
{
    // get total time in one wrap in ns
    uint32_t ns_in_wrap = (S_TO_NS / frequency) * wrap;
    uint16_t duty_cycle = MAX_UINT16_T * ns / ns_in_wrap;
    return duty_cycle;
}

static inline uint32_t ns_from_duty_cycle(uint32_t frequency, uint16_t wrap, uint16_t duty_cycle)
{
    // get total time in one wrap in ns
    uint32_t ns_in_wrap = (S_TO_NS / frequency) * wrap;
    uint32_t ns = ns_in_wrap * duty_cycle / MAX_UINT16_T;
    return ns;
}

bool pico_pwm_set_freq_and_duty_u16_unsafe(struct PicoPWM *pwm, uint32_t frequency, uint16_t duty_cycle)
{
    // 0 <= duty_cycle <= 65535
    if (!pwm_has_init_unsafe(pwm)) return false;
    pwm_set_freq_duty_unsafe(pwm, frequency, duty_cycle);

    uint8_t div_int, div_frac;
    pwm_get_div_int_frac_from_div16(&div_int, &div_frac, pwm->config_divider16);
    pwm_set_clkdiv_int_frac(pwm->slice_num, div_int, div_frac);
    pwm_set_wrap(pwm->slice_num, pwm->config_wrap);
    pwm_set_chan_level(pwm->slice_num, pwm->channel, pwm->config_level);

    pwm->ns_per_cycle = ns_from_duty_cycle(pwm->freq, pwm->config_wrap, duty_cycle);

    pico_pwm_hw_enable(pwm, true);
    return true;
}

bool pico_pwm_set_freq_and_duty_u16(struct PicoPWM *pwm, uint32_t frequency, uint16_t duty_cycle)
{
    if (!mutex_enter_timeout_ms(&pwm->mux, 1000)) return false;
    bool res = pico_pwm_set_freq_and_duty_u16_unsafe(pwm, frequency, duty_cycle);
    mutex_exit(&pwm->mux);
    return res;
}

bool pico_pwm_set_duty_u16_unsafe(struct PicoPWM *pwm, uint16_t duty_cycle)
{
    if (!pwm_has_init_unsafe(pwm)) return false;
    pwm_set_duty_unsafe(pwm, duty_cycle);
    pwm_set_chan_level(pwm->slice_num, pwm->channel, pwm->config_level);

    pwm->ns_per_cycle = ns_from_duty_cycle(pwm->freq, pwm->config_wrap, duty_cycle);

    pico_pwm_hw_enable(pwm, true);
    return true;
}

bool pico_pwm_set_duty_u16(struct PicoPWM *pwm, uint16_t duty_cycle)
{
    if (!mutex_enter_timeout_ms(&pwm->mux, 1000)) return false;
    bool res = pico_pwm_set_duty_u16_unsafe(pwm, duty_cycle);
    mutex_exit(&pwm->mux);
    return res;
}

bool pico_pwm_set_duty_ns_unsafe(struct PicoPWM *pwm, uint32_t ns)
{
    if (!pwm_has_init_unsafe(pwm)) return false;
    pwm->duty_cycle = duty_cycle_from_ns(pwm->freq, pwm->config_wrap, ns);
    pico_pwm_set_duty_u16(pwm, pwm->duty_cycle);
    return true;
}

bool pico_pwm_set_duty_ns(struct PicoPWM *pwm, uint32_t ns)
{
    if (!mutex_enter_timeout_ms(&pwm->mux, 1000)) return false;
    bool res = pico_pwm_set_duty_ns_unsafe(pwm, ns);
    mutex_exit(&pwm->mux);
    return res;
}