#include "PicoStepper.h"

#define abs(x) ((x)<0 ? (-x) : (x))
#define conditional_negation(i, invert) ((i)*((1-(invert)) - (invert)))
// #define conditional_negation(i, invert) ((invert) ? -(i) : (i))
#define cycle(v, min, max) (v > max ? min : (v < min ? max : v))
#define STEPPER_STEP_DELAY_US 1000

static const bool step_normal[4][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};
#define STEP_NORMAL_LAST_STEP 3

static const bool step_wave[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}
};
#define STEP_WAVE_LAST_STEP 3

static const bool step_half[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};
#define STEP_HALF_LAST_STEP 7

static void make_step(struct PicoStepper *stepper, int8_t dir)
{
    // int8_t step_dir = conditional_negation(dir, stepper->inverted);
    int8_t step_dir = conditional_negation(dir, stepper->inverted);
    // if (stepper->inverted)
    //     step_dir = -dir;
    // else
    //     step_dir = dir;
    switch(stepper->step_type)
    {
    case PICO_STEPPER_STEP_TYPE_NORMAL: {
        int8_t step = 0;
        for (uint8_t i = 0; i <= STEP_NORMAL_LAST_STEP; ++i) {
            gpio_put(stepper->pin1, step_normal[step][0]);
            gpio_put(stepper->pin2, step_normal[step][1]);
            gpio_put(stepper->pin3, step_normal[step][2]);
            gpio_put(stepper->pin4, step_normal[step][3]);

            step += step_dir;
            step = cycle(step, 0, STEP_NORMAL_LAST_STEP);
            pico_stepper_delay_us(STEPPER_STEP_DELAY_US);
        }
        stepper->curr_pos += dir;
        break;
    }
    case PICO_STEPPER_STEP_TYPE_WAVE: {
        int8_t step = 0;
        for (uint8_t i = 0; i <= STEP_WAVE_LAST_STEP; ++i) {
            gpio_put(stepper->pin1, step_wave[step][0]);
            gpio_put(stepper->pin2, step_wave[step][1]);
            gpio_put(stepper->pin3, step_wave[step][2]);
            gpio_put(stepper->pin4, step_wave[step][3]);

            step += step_dir;
            step = cycle(step, 0, STEP_WAVE_LAST_STEP);
            pico_stepper_delay_us(STEPPER_STEP_DELAY_US);
        }
        stepper->curr_pos += dir;
        break;
    }
    case PICO_STEPPER_STEP_TYPE_HALF: {
        int8_t step = 0;
        for (uint8_t i = 0; i <= STEP_HALF_LAST_STEP; ++i) {
            gpio_put(stepper->pin1, step_half[step][0]);
            gpio_put(stepper->pin2, step_half[step][1]);
            gpio_put(stepper->pin3, step_half[step][2]);
            gpio_put(stepper->pin4, step_half[step][3]);

            step += step_dir;
            step = cycle(step, 0, STEP_HALF_LAST_STEP);
            pico_stepper_delay_us(STEPPER_STEP_DELAY_US);
        }
        stepper->curr_pos += dir;
        break;
    }
    default:
        gpio_put(stepper->pin1, false);
        gpio_put(stepper->pin2, false);
        gpio_put(stepper->pin3, false);
        gpio_put(stepper->pin4, false);
        break;
    }
}

void pico_stepper_init(struct PicoStepper *stepper, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, enum PicoStepperStepType step_type, bool inverted)
{
    if (stepper->init_flag) return;

    stepper->pin1 = pin1;
    stepper->pin2 = pin2;
    stepper->pin3 = pin3;
    stepper->pin4 = pin4;

    stepper->inverted = inverted;
    stepper->step_type = step_type; // LPM!!!

    gpio_init(pin1);
    gpio_set_dir(pin1, GPIO_OUT);

    gpio_init(pin2);
    gpio_set_dir(pin2, GPIO_OUT);

    gpio_init(pin3);
    gpio_set_dir(pin3, GPIO_OUT);

    gpio_init(pin4);
    gpio_set_dir(pin4, GPIO_OUT);

    stepper->init_flag = true;
    pico_stepper_release(stepper);
}

void pico_stepper_release(const struct PicoStepper *stepper)
{
    if (!stepper->init_flag) return;

    gpio_put(stepper->pin1, false);
    gpio_put(stepper->pin2, false);
    gpio_put(stepper->pin3, false);
    gpio_put(stepper->pin4, false);
}

void pico_stepper_move_steps(struct PicoStepper *stepper, int32_t steps)
{
    if (steps == 0) return;
    const int32_t abs_steps = abs(steps);
    int8_t dir = (steps > 0 ? 1 : -1);

    for (int32_t i = 0; i < abs_steps; ++i)
    {
        make_step(stepper, dir);
    }
}

void pico_stepper_move_to_pos(struct PicoStepper *stepper, int32_t pos)
{
    const int32_t steps = pos - stepper->curr_pos;
    pico_stepper_move_steps(stepper, steps);
}

int32_t pico_stepper_get_curr_pos(const struct PicoStepper *stepper)
{
    return stepper->curr_pos;
}

void pico_stepper_set_curr_pos_forced(struct PicoStepper *stepper, int32_t pos)
{
    stepper->curr_pos = pos;
}