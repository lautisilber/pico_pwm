#ifndef _PICO_PWM_H_
#define _PICO_PWM_H_

#include "pico/stdlib.h"
#include "pico/sync.h"

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

    // mutex
    mutex_t mux;
};

extern void pico_pmw_global_init();
extern void pico_pwm_init(struct PicoPWM *pwm, uint8_t pin, bool inverted);
extern bool pico_pwm_hw_enable(struct PicoPWM *pwm, bool enable);

extern bool pico_pwm_set_freq_and_duty_u16(struct PicoPWM *pwm, uint32_t frequency, uint16_t duty_cycle);
extern bool pico_pwm_set_duty_u16(struct PicoPWM *pwm, uint16_t duty_cycle);
extern bool pico_pwm_set_duty_ns(struct PicoPWM *pwm, uint32_t ns);

extern bool pico_pwm_hw_enable_safe(struct PicoPWM *pwm, bool enable);
extern bool pico_pwm_set_freq_and_duty_u16_safe(struct PicoPWM *pwm, uint32_t frequency, uint16_t duty_cycle);
extern bool pico_pwm_set_duty_u16_safe(struct PicoPWM *pwm, uint16_t duty_cycle);
extern bool pico_pwm_set_duty_ns_safe(struct PicoPWM *pwm, uint32_t ns);

#if __cplusplus
}
#endif

#endif /* _PICO_PWM_H_ */