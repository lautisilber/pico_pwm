#ifndef _PICO_STEPPER_H_
#define _PICO_STEPPER_H_

#include <pico/stdlib.h>

// you need to define this function
extern void pico_stepper_delay_us(uint32_t delay);

enum PicoStepperStepType
{
    PICO_STEPPER_STEP_TYPE_NORMAL = 1,
    PICO_STEPPER_STEP_TYPE_WAVE,
    PICO_STEPPER_STEP_TYPE_HALF
};

struct PicoStepper
{
    uint8_t pin1, pin2, pin3, pin4;
    enum PicoStepperStepType step_type;

    bool inverted;

    int8_t curr_step;
    int32_t curr_pos;

    bool init_flag;
};

extern void pico_stepper_init(struct PicoStepper *stepper, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, enum PicoStepperStepType step_type, bool inverted);
extern void pico_stepper_release(const struct PicoStepper *stepper);

extern void pico_stepper_move_steps(struct PicoStepper *stepper, int32_t steps);
extern void pico_stepper_move_to_pos(struct PicoStepper *stepper, int32_t pos);

int32_t pico_stepper_get_curr_pos(const struct PicoStepper *stepper)
{
    return stepper->curr_pos;
}

void pico_stepper_set_curr_pos_forced(struct PicoStepper *stepper, int32_t pos)
{
    stepper->curr_pos = pos;
}

#endif /* _PICO_STEPPER_H_ */