#include <stdio.h>

#include "pico/stdlib.h"
#include "PicoPWM.h"

#define PIN 10

struct PicoPWM pwm = {};
const uint16_t max_u16 = 65535;
const uint32_t delay_us = 10;

int main()
{
    stdio_init_all();

    const uint32_t period_us = 20 * 1000;
    const uint32_t duty_us_min = 1200;
    const uint32_t duty_us_max = 1700;
    const uint32_t step = 5;

    pico_pwm_init(&pwm, PIN, false);
    pico_pwm_set_freq_and_duty_u16(&pwm, 1000000, 0);
    sleep_ms(500);
    for (;;)
    {
        for (uint16_t i = 0; i < max_u16; ++i)
        {
            pico_pwm_set_duty_u16(&pwm, i);
            sleep_us(delay_us);
        }
        for (uint16_t i = max_u16; i > 0; --i)
        {
            pico_pwm_set_duty_u16(&pwm, i);
            sleep_us(delay_us);
        }
    }

}