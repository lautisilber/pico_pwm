#ifndef _PICO_STEPPER_H_
#define _PICO_STEPPER_H_

#include <pico/stdlib.h>

#if __cplusplus
extern "C" {
#endif

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

    int32_t curr_pos;

    bool init_flag;
};

extern void pico_stepper_init(struct PicoStepper *stepper, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, enum PicoStepperStepType step_type, bool inverted);
extern void pico_stepper_release(const struct PicoStepper *stepper);

extern void pico_stepper_move_steps(struct PicoStepper *stepper, int32_t steps);
extern void pico_stepper_move_to_pos(struct PicoStepper *stepper, int32_t pos);

extern int32_t pico_stepper_get_curr_pos(const struct PicoStepper *stepper);
extern void pico_stepper_set_curr_pos_forced(struct PicoStepper *stepper, int32_t pos);

#if __cplusplus
}
#endif

#endif /* _PICO_STEPPER_H_ */