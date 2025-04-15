#ifndef _STOMASENSE_PUMP_H_
#define _STOMASENSE_PUMP_H_

#include <StomaSense/defs.h>

#include <PicoPWM.h>
#include <PicoStepper.h>
#include <PicoServo.h>

namespace StomaSense
{
    extern void delay_ms(time_ms_t ms);
    
    extern void mutex_watering_queue_access_claim_blocking();
    extern void mutex_watering_queue_access_release();

    extern void mutex_servo_claim_blocking();
    extern void mutex_servo_release();

    extern void mutex_stepper_claim_blocking();
    extern void mutex_stepper_release();

    /// Watering data

    struct WateringIntensity
    {
        pump_intensity_t intensity;
        time_ms_t time_ms;

        static void interpolate(WateringIntensity *res, WateringIntensity *wi1, WateringIntensity *wi2, float t)
        {
            res->intensity = StomaSense::interpolate(wi1->intensity, wi2->intensity, t);
            res->time_ms = StomaSense::interpolate(wi1->time_ms, wi2->time_ms, t);
        }
    };

    struct WateringData
    {
        WateringIntensity min_intensity;
        WateringIntensity max_intensity;

        void interpolate(WateringIntensity *res, float t)
        {
            WateringIntensity::interpolate(res, &min_intensity, &max_intensity, t);
        }

        float last_intensity_increase;
    };

    /// Scale position

    struct Position
    {
        stepper_pos_t stepper;
        servo_angle_t servo;
        WateringData watering_data;
    };

    /// scale

    struct Scale {
        scale_t scale_n;
        Position pos;
    };

    /// pump

    namespace Pump
    {
        extern void init();
        extern bool add_scale(const Scale *scale);
        extern bool overwrite_scale(const Scale *scale, scale_t scale_n);

        extern PicoStepper *get_stepper();
        extern PicoPWM *get_pump();
        extern PicoServo *get_servo();

        extern bool add_scale_to_watering_queue(scale_t scale_n);
        extern bool pop_next_target(scale_t *scale);

        extern const Scale *water_next_target();
    }

}

#endif /* _STOMASENSE_PUMP_H_ */