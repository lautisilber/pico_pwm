#include <PicoServo.h>
#include <stdio.h>

#define SERVO_DEFAULT_MIN_PULSE_WIDTH_US 1000 // uncalibrated default, the shortest duty cycle sent to a servo
#define SERVO_DEFAULT_MAX_PULSE_WIDTH_US 2000 // uncalibrated default, the longest duty cycle sent to a servo
#define SERVO_DEFAULT_MIN_ANGLE 0             // uncalibrated default, the longest duty cycle sent to a servo
#define SERVO_DEFAULT_MAX_ANGLE 180           // uncalibrated default, the longest duty cycle sent to a servo
#define SERVO_PERIOD_US 20000
#define MAX_SERVOS 8 // number of PIO state machines available, assuming nobody else is using them

#define clamp(v, left, right) ((v) > (right) ? (right) : ((v) < (left) ? (left) : (v)))
#define map(v, in_min, in_max, out_min, out_max) \
    (((v) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))



bool pico_servo_init(struct PicoServo *servo, uint8_t pin, bool inverted)
{
    bool res = pico_pio_pwm_init(&servo->pwm, pin, inverted);
    if (!res)
        return false;

    servo->min_us = SERVO_DEFAULT_MIN_PULSE_WIDTH_US;
    servo->max_us = SERVO_DEFAULT_MAX_PULSE_WIDTH_US;

    servo->min_angle = SERVO_DEFAULT_MIN_ANGLE;
    servo->max_angle = SERVO_DEFAULT_MAX_ANGLE;

    return true;
}

void pico_servo_set_min_max_us(struct PicoServo *servo, uint32_t min_us, uint32_t max_us)
{
    servo->min_us = min_us;
    servo->max_us = max_us;
}

void pico_servo_set_min_max_angle(struct PicoServo *servo, uint8_t min_angle, uint8_t max_angle)
{
    servo->min_angle = min_angle;
    servo->max_angle = max_angle;
}

bool pico_servo_attach(struct PicoServo *servo)
{
    return pico_pio_pwm_set_period_us(&servo->pwm, SERVO_PERIOD_US);
}

bool pico_servo_release(struct PicoServo *servo)
{
    return pico_pio_pwm_release(&servo->pwm);
}

void pico_servo_deinit(struct PicoServo *servo)
{
    pico_pio_pwm_deinit(&servo->pwm);
}

static inline uint32_t pico_servo_angle_to_us(struct PicoServo *servo, uint8_t angle)
{
    return map((uint32_t)angle, (uint32_t)servo->min_angle, (uint32_t)servo->max_angle,
               servo->min_us, servo->max_us);

    // mapping
    // uint32_t dx = (uint32_t)servo->min_angle - (uint32_t)servo->max_angle;
    // uint32_t in_span = (uint32_t)servo->max_angle - (uint32_t)servo->min_angle;
    // uint32_t out_span = servo->max_us - servo->min_us;
    // return (dx * out_span) / in_span + servo->min_us;
}

bool pico_servo_set_angle(struct PicoServo *servo, uint8_t angle)
{
    angle = clamp(angle, servo->min_angle, servo->max_angle);
    const uint32_t duty_us = pico_servo_angle_to_us(servo, angle);
    // bool res = pico_pio_pwm_set_period_us(&servo->pwm, period);
    bool res = pico_pio_pwm_set_duty_us(&servo->pwm, duty_us);
    if (res)
        servo->angle = angle;
    return res;
}

bool pico_servo_sweep(struct PicoServo *servo, uint8_t goal_angle, uint32_t delay_ms, uint32_t resolution_us)
{
    if (!servo->pwm.claimed)
        return false;

    if (servo->angle == goal_angle) return true;
    
    int32_t curr_us = pico_servo_angle_to_us(servo, servo->angle);
    int32_t final_us = pico_servo_angle_to_us(servo, goal_angle);
    const int8_t dir = (goal_angle > servo->angle ? 1 : -1);
    const int32_t step =  (int32_t)dir * resolution_us;

    for (int32_t us = curr_us; (dir > 0 && us < final_us) || (dir < 0 && us > final_us); us += step)
    {
        printf("sweep us: %lu\n", us);
        pico_pio_pwm_set_duty_us(&servo->pwm, us);
        pico_servo_delay_ms(delay_ms);
    }

    pico_servo_set_angle(servo, goal_angle);
    return true;

    // const int8_t dir = (goal_angle > servo->angle ? 1 : -1);
    // const bool sub_angle_definition = (resolution_us < pico_servo_angle_to_us(servo, 1));

    // uint8_t next_angle;
    // while (servo->angle != goal_angle)
    // {
    //     next_angle = servo->angle + dir;
    //     if (sub_angle_definition)
    //     {
    //         int32_t us_step = (int32_t)((dir) * ((int32_t)resolution_us));
    //         printf("a (%li)\n", us_step);
    //         int32_t next_us = pico_servo_angle_to_us(servo, next_angle);
    //         for (int32_t us = pico_servo_angle_to_us(servo, servo->angle);
    //              (dir > 0 && us < next_us-1) || (dir < 0 && us > next_us+1);
    //              us += us_step)
    //         {
    //             printf("sweep us: %lu\n", us);
    //             pico_pio_pwm_set_duty_us(&servo->pwm, us);
    //             pico_servo_delay_ms(delay_ms);
    //         }
    //     }

    //     printf("sweep us: %lu\n", pico_servo_angle_to_us(servo, next_angle));
    //     pico_servo_set_angle(servo, next_angle);
    //     pico_servo_delay_ms(delay_ms);
    // }
}