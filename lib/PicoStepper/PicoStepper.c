#include "PicoStepper.h"

#define abs(x) ((x)<0 ? (-x) : (x))
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
    switch(stepper->step_type)
    {
    case PICO_STEPPER_STEP_TYPE_NORMAL: 
        stepper->curr_step += dir;
        if (stepper->curr_step < 0) stepper->curr_step = STEP_NORMAL_LAST_STEP;
        else if (stepper->curr_step > STEP_NORMAL_LAST_STEP) stepper->curr_step = 0;

        gpio_put(stepper->pin1, step_normal[stepper->curr_step][0]);
        gpio_put(stepper->pin2, step_normal[stepper->curr_step][1]);
        gpio_put(stepper->pin3, step_normal[stepper->curr_step][2]);
        gpio_put(stepper->pin4, step_normal[stepper->curr_step][3]);

        stepper->curr_pos += dir*2;
        break;
    case PICO_STEPPER_STEP_TYPE_WAVE:
        stepper->curr_step += dir;
        if (stepper->curr_step < 0) stepper->curr_step = STEP_WAVE_LAST_STEP;
        else if (stepper->curr_step > STEP_WAVE_LAST_STEP) stepper->curr_step = 0;

        gpio_put(stepper->pin1, step_wave[stepper->curr_step][0]);
        gpio_put(stepper->pin2, step_wave[stepper->curr_step][1]);
        gpio_put(stepper->pin3, step_wave[stepper->curr_step][2]);
        gpio_put(stepper->pin4, step_wave[stepper->curr_step][3]);

        stepper->curr_pos += dir*2;
        break;
    case PICO_STEPPER_STEP_TYPE_HALF:
        stepper->curr_step += dir;
        if (stepper->curr_step < 0) stepper->curr_step = STEP_HALF_LAST_STEP;
        else if (stepper->curr_step > STEP_HALF_LAST_STEP) stepper->curr_step = 0;

        gpio_put(stepper->pin1, step_half[stepper->curr_step][0]);
        gpio_put(stepper->pin2, step_half[stepper->curr_step][1]);
        gpio_put(stepper->pin3, step_half[stepper->curr_step][2]);
        gpio_put(stepper->pin4, step_half[stepper->curr_step][3]);

        stepper->curr_pos += dir;
        break;
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
    int8_t dir = (steps > 0 ^ stepper->inverted ? 1 : -1);

    for (int32_t i = 0; i < abs_steps; ++i)
    {
        make_step(stepper, dir);
        pico_stepper_delay_us(STEPPER_STEP_DELAY_US);
    }
}

void pico_stepper_move_to_pos(struct PicoStepper *stepper, int32_t pos)
{
    const int32_t steps = pos - stepper->curr_pos;
    pico_stepper_move_steps(stepper, steps);
}

// extern void move_to_pos(struct PicoStepper *stepper, int32_t steps);