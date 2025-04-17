#include <PicoServo.h>

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
    pio_pwm_set_period_us(&servo->pwm, SERVO_PERIOD_US);
}

bool pico_servo_release(struct PicoServo *servo)
{
    return pico_pio_pwm_release(&servo->pwm);
}

void pico_servo_deinit(struct PicoServo *servo)
{
    pico_pio_pwm_deinit(&servo->pwm);
}

bool pico_servo_set_angle(struct PicoServo *servo, uint8_t angle)
{
    angle = clamp(angle, servo->min_angle, servo->max_angle);
    uint32_t period = map((uint32_t)angle, (uint32_t)servo->min_angle, (uint32_t)servo->max_angle,
                          servo->min_us, servo->max_us);
    bool res = pico_pio_pwm_set_period_us(&servo->pwm, period);
    if (res)
        servo->angle = angle;
    return res;
}

bool pico_servo_sweep(struct PicoServo *servo, uint8_t goal_angle, uint32_t delay_ms, uint32_t resolution_us)
{
    if (!servo->pwm.claimed)
        return false;

    int8_t dir = (goal_angle > servo->angle ? 1 : -1);
    uint8_t curr_angle = servo->angle;
    uint8_t total_angles = (dir ? goal_angle - curr_angle : curr_angle - goal_angle);
    uint32_t curr_us = map((uint32_t)curr_angle, (uint32_t)servo->min_angle,
                           (uint32_t)servo->max_angle, servo->min_us, servo->max_us);
    uint32_t next_us;
    for (uint8_t i = 0; i < total_angles; ++i)
    {
        next_us = map((uint32_t)(curr_angle + dir), (uint32_t)servo->min_angle,
                      (uint32_t)servo->max_angle, servo->min_us, servo->max_us);
        for (int32_t us = curr_us; (dir > 0 ? us < next_us : us > next_us); us += (dir * (int32_t)resolution_us))
        {
            pico_pio_pwm_set_duty_ns(&servo->pwm, us);
        }
        pico_pio_pwm_set_duty_ns(&servo->pwm, next_us);
        curr_us = next_us;
    }
    return true;
}