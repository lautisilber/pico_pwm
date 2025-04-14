#ifdef _TEST_TASKS_H_
#error "This should only be called once from main"
#else
#define _TEST_TASKS_H_

#include "FreeRTOS.h"
#include "task.h"

#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <cstring>
#include <stdio.h>

#include <StomaSense/defs.h>



// Struct to hold task parameters
typedef struct {
    const char* name;
    uint32_t delay_ms;
} print_task_params_t;

void print_task(void *params) {
    print_task_params_t *cfg = (print_task_params_t *)params;
    while (1) {
        printf("%s running on core %u\n", cfg->name, get_core_num());
        vTaskDelay(pdMS_TO_TICKS(cfg->delay_ms));
    }
}

void led_fade_task(void *params) {
    constexpr uint16_t pwm_wrap = 255;
    // Set up PWM
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    StomaSense::pin_size_t slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_enabled(slice_num, true);

    int brightness = 0;
    int direction = 1; // 1 = increasing, -1 = decreasing

    while (1) {
        pwm_set_gpio_level(LED_PIN, brightness);
        brightness += direction;
        if (brightness >= pwm_wrap || brightness <= 0) {
            direction = -direction;
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // adjust for fade speed
    }
}



#endif /* _TEST_TASKS_H_ */