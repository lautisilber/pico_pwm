#ifndef _PWM_H_
#define _PWM_H_

#include <pico/stdlib.h>

// https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=1
// https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2

#if __cplusplus
extern "C" {
#endif

struct PicoPWM
{
    uint8_t pin;
    bool inverted;

    bool init_flag;

    uint slice_num;
    uint channel;

    // curr output
    bool pwm_hw_init;
    uint32_t freq;
    uint16_t duty_cycle;
    uint32_t ns_per_cycle;

    // config
    uint16_t config_wrap;
    uint32_t config_divider16;
    uint16_t config_level;
};

extern void pmw_pico_global_init();
extern void pwm_pico_init(struct PicoPWM *pwm, uint8_t pin, bool inverted);
extern void pwm_pico_hw_enable(struct PicoPWM *pwm, bool enable);

extern void pwm_pico_set_freq_and_duty_u16(struct PicoPWM *pwm, uint32_t frequency, uint16_t duty_cycle);
extern void pwm_pico_set_duty_u16(struct PicoPWM *pwm, uint16_t duty_cycle);
extern void pwm_pico_set_duty_ns(struct PicoPWM *pwm, uint32_t ns);

#if __cplusplus
}
#endif

#endif /* _PWM_H_ */