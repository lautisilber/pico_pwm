#ifndef _STOMASENSE_DEFS_H_
#define _STOMASENSE_DEFS_H_

#include <stdio.h>
#include <hardware/watchdog.h>
#include <pico/time.h>

#include <limits>
#include <type_traits>

#define STOMA_SENSE_SCALES_MULTIPLEXER_N_PINS 4
#define STOMA_SENSE_N_SCALES (2 << STOMA_SENSE_SCALES_MULTIPLEXER_N_PINS - 1)

#define STOMA_SENSE_WATERING_QUEUE_SIZE 8

#define STOMA_SENSE_STEPPER_MIN_POS std::numeric_limits<int32_t>::min()
#define STOMA_SENSE_STEPPER_MAX_POS std::numeric_limits<int32_t>::max()
#define STOMA_SENSE_SERVO_MAX_ANGLE 128

#define STOMA_SENSE_MAX_PROTOCOL_STEPS 32
#define STOMA_SENSE_PROTOCOL_MAX_CYCLES 128

#define STOMA_SENSE_MAX_WATERING_GOALS_STACK 32

#define _STOMASENSE_PRINT_INTERNAL(level, ...)          \
    do                                                       \
    {                                                        \
        printf("%s | %s:%u -> ", level, __FILE__, __LINE__); \
        printf(__VA_ARGS__);                            \
    } while (0)
#define STOMASENSE_DEBUG(...) _STOMASENSE_PRINT_INTERNAL("DEBUG", __VA_ARGS__)
#define STOMASENSE_WARN(...) _STOMASENSE_PRINT_INTERNAL("WARN", __VA_ARGS__)
#define STOMASENSE_ERROR(...) _STOMASENSE_PRINT_INTERNAL("ERROR", __VA_ARGS__)
#define STOMASENSE_PANIC(...)                             \
    do                                                         \
    {                                                          \
        _STOMASENSE_PRINT_INTERNAL("PANIC", __VA_ARGS__); \
        watchdog_reboot(0, 0, 1000);                           \
    } while (0)

namespace StomaSense
{
    typedef unsigned long (*const millis_t)(void);

    typedef uint8_t pin_size_t;
    
    typedef uint8_t scale_t;
    typedef int32_t stepper_pos_t;
    typedef uint8_t servo_angle_t;
    
    typedef uint8_t watering_goal_idx_t;
    typedef uint8_t pump_intensity_t;
    
    typedef unsigned long time_ms_t;
    
    typedef float weight_t;
    
    static_assert(STOMA_SENSE_N_SCALES >= (1 << sizeof(scale_t)) - 1,
    "scale_t type is not large enough to store STOMA_SENSE_N_SCALES scales");
    
    static_assert(std::is_unsigned<scale_t>::value, "scale_t type should be unsigned");
    static_assert(STOMA_SENSE_STEPPER_MAX_POS >= (1 << sizeof(stepper_pos_t)) - 1 &&
    STOMA_SENSE_STEPPER_MIN_POS <= (1 << sizeof(stepper_pos_t)) - 1,
    "stepper_pos_t type is not large enough to store all stepper positions");
    static_assert(STOMA_SENSE_SERVO_MAX_ANGLE >= (1 << sizeof(servo_angle_t)) - 1,
    "servo_angle_t type is not large enough to store STOMA_SENSE_SERVO_MAX_ANGLE angles");
    
    static_assert(STOMA_SENSE_MAX_WATERING_GOALS_STACK >= (1 << sizeof(watering_goal_idx_t)) - 1,
    "goal_idx_t type is not large enough to store STOMA_SENSE_MAX_WATER_PLANNING_STACK water planning goals");
    
    extern time_ms_t millis();

    extern void atomic_section_start();
    extern void atomic_section_end();

    inline bool has_timer_elapsed(time_ms_t curr_millis, time_ms_t start_time, time_ms_t timedelta)
    {
        // https://www.norwegiancreations.com/2018/10/arduino-tutorial-avoiding-the-overflow-issue-when-using-millis-and-micros/
        // curr_millis is the current global elapsed ms since boot
        // start_time is the ms since boot when the timer is started
        // timedelta is the ms the timer should run for
        return curr_millis - start_time >= timedelta;
    }

    template <typename T, typename U>
    T interpolate(T start, T end, U t)
    {
        static_assert(std::is_arithmetic<T>::value, "T should be an arithmetic type");
        static_assert(std::is_arithmetic<U>::value, "U should be an arithmetic type");

        if (t <= static_cast<U>(0))
            return start;
        else if (t >= static_cast<U>(1))
            return end;

        // shouldn't be necessary to differenciate between the two cases
        // but if T is strictly positive, there's issues, so it's easier
        // to differenciate
        else if (start <= end)
        {
            T diff = end - start;
            T interp = static_cast<T>(t * static_cast<U>(diff));
            return start + interp;
        }
        else
        {
            T diff = end - start;
            T interp = static_cast<T>(t * static_cast<U>(diff));
            return start - interp;
        }
    }

    template <typename T>
    T map_range(T value, T in_min, T in_max, T out_min, T out_max)
    {
        static_assert(std::is_arithmetic<T>::value, "T should be an arithmetic type");

        const T in_range = in_max - in_min;
        const T out_range = out_max - out_min;
        const T slope = out_range / in_range;
        return (value - in_min) * slope + out_min;
    }

}

#endif /* _STOMASENSE_DEFS_H_ */