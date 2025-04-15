#ifndef _STOMASENSE_PUMP_H_
#define _STOMASENSE_PUMP_H_

#include <StomaSense/defs.h>

namespace StomaSense
{

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
        WateringData watering_data;
    };

    /// pump

    namespace Pump
    {
        extern void init();
    }

}

#endif /* _STOMASENSE_PUMP_H_ */