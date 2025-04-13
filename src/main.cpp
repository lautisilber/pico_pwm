#include "FreeRTOS.h"
#include "task.h"
#include <ArduinoJson.h>
#include <StomaSense/defs.h>
#include <pico/stdlib.h>

#define LED_PIN 25

#include "test_tasks.h"

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Define task parameters
    static print_task_params_t print1_cfg = { .name = "Print Task 1", .delay_ms = 2000 };
    static print_task_params_t print2_cfg = { .name = "Print Task 2", .delay_ms = 1300 };

    // Task handles
    TaskHandle_t print_handle_1, print_handle_2, fade_handle;

    // Create tasks
    xTaskCreate(print_task, "Print1", 256, &print1_cfg, 2, &print_handle_1);
    xTaskCreate(print_task, "Print2", 256, &print2_cfg, 2, &print_handle_2);
    xTaskCreate(led_fade_task, "Fade", 256, NULL, 1, &fade_handle);


    // Pin print_task_1 to core 0
    vTaskCoreAffinitySet(print_handle_1, (1 << 0));
    // Pin print_task_2 to core 1
    vTaskCoreAffinitySet(print_handle_2, (1 << 1));
    // Let the LED task run on either core
    vTaskCoreAffinitySet(fade_handle, (1 << 0) | (1 << 1));

    vTaskStartScheduler();

    while (1); // should never reach here
    return 0;
}