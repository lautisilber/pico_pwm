#include <pico/stdlib.h>
#include <stdio.h>

#include <StomaSense/defs.h>
#include <StomaSense/pump.h>
#include <StomaSense/pin_defs.h>
#include <SmartComm.h>

#include <PicoPWM.h>
#include <PicoStepper.h>
#include <PicoServo.h>

namespace StomaSense
{
    void mutex_stepper_claim_blocking() {}
    void mutex_stepper_release() {}

    void mutex_servo_claim_blocking() {}
    void mutex_servo_release() {}
}

void pico_stepper_delay_us(uint32_t us)
{
    sleep_us(us);
}

void pico_servo_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

using namespace StomaSense;

// // create the "OK" command which only answers with "OK"
// SMART_CMD_CREATE(cmdOk, "OK", [](printf_like_fn printf_like, const SmartCmdArguments *args, const char *cmd) {
//     printf_like("OK");
// });

// // create the array of commands (note the & before the class names)
// const SmartCmd *cmds[] = {
//     &cmdOk//, &cmdHello, &cmdOne
// };

// // create the CmartComm main class with the length of the cmds array
// // between angled brackets <>
// SmartComm<ARRAY_LENGTH(cmds)> sc(cmds, printf, nullptr, nullptr);

// void char_available_cb(void *params)
// {
//     int c;
//     while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
//         if (c < 0) break;
//         if (!sc.put_char_in_buff(c)) break;
//     }
// }

static PicoStepper stepper;
static PicoPWM pump;
static PicoServo servo = {};

void test_stepper()
{
    printf("Stepper test start\n");
    pico_stepper_init(
        &stepper,
        STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4,
        PicoStepperStepType::PICO_STEPPER_STEP_TYPE_HALF, true);
    printf("Stepper init\n");

    pico_stepper_set_curr_pos_forced(&stepper, 0);
    printf("Stepper pos: %li\n", pico_stepper_get_curr_pos(&stepper));
    pico_stepper_move_to_pos(&stepper, 500);
    printf("Stepper pos: %li\n", pico_stepper_get_curr_pos(&stepper));
    sleep_ms(1000);
    pico_stepper_move_to_pos(&stepper, 0);
    printf("Stepper pos: %li\n", pico_stepper_get_curr_pos(&stepper));
    printf("Stepper test end\n");
    pico_stepper_release(&stepper);
}

void test_pump()
{
    // pico_pwm_init(&pump, PUMP_PIN, false);
    // pico_pwm_set_freq_and_duty_u16(&pump, 100000, 0);
    // Pump::init();
    printf("Not implemented (errors with extern definitions of variables)\n");
}

void test_servo()
{
    printf("Servo test start\n");
    pico_servo_init(&servo, SERVO_PIN, true);
    
    bool r = pico_servo_attach(&servo);
    printf("Servo attach %s\n", (r ? "true" : "false"));
    if (!r) return;

    const uint32_t delay_ms = 20, resolution_us = 5;

    pico_servo_set_angle(&servo, 90);
    pico_servo_sweep(&servo, 180, delay_ms, resolution_us);
    pico_servo_release(&servo);
    sleep_ms(500);
    pico_servo_sweep(&servo, 0, delay_ms, resolution_us);
    sleep_ms(500);
    pico_servo_sweep(&servo, 90, delay_ms, resolution_us);
    pico_servo_deinit(&servo);
}

int main()
{
    stdio_init_all();
    // stdio_set_chars_available_callback(char_available_cb, nullptr);

    for (;;)
    {
        int c = getchar_timeout_us(1000);
        if (c < 0)
            continue;

        switch (c)
        {
        case 'h':
            printf("o, s, p, e");
            break;
        case 'o':
            printf("OK");
            break;
        case 's':
            test_stepper();
            break;
        case 'p':
            test_pump();
            break;
        case 'e':
            test_servo();
            break;
        default:
            // putchar(c);
            printf("Error");
            break;
        }
        putchar('\n');
    }

    // sc.tick();
    // sleep_ms(1);

    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);
}