//
//  StomaSenseSession.h
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 14/02/2025.
//

#ifndef _STOMA_SENSE_SESSION_H_
#define _STOMA_SENSE_SESSION_H_

#include <StomaSense/defs.h>
#include <StomaSense/datatypes.h>

namespace StomaSense
{

    /// Protocol

    enum ProtocolStepType : uint8_t
    {
        NONE,
        WAIT,
        HOLD_WEIGHT,
        OSCILLATE_IN_RANGE
    };

    struct ProtocolStep_Wait
    {
        // data
        time_ms_t wait_ms;

        // vars (internal use)
        time_ms_t _start_time = 0;
        uint8_t _initial_step = true;

        //    ProtocolStep_Wait(time_ms_t wait_ms) : wait_ms(wait_ms) {}
        //    ProtocolStep_Wait() : ProtocolStep_Wait(0) {}
    };

    struct ProtocolStep_HoldWeight
    {
        // data
        weight_t weight;
        weight_t weight_tolerance;
        time_ms_t wait_ms;

        enum Stage : uint8_t
        {
            FIRST_STEP,
            REACHING_WEIGHT_BY_WATERING,
            REACHING_WEIGHT_BY_DRYING,
            HOLDING_WEIGHT_AND_WATERING,
            HOLDING_WEIGHT_AND_DRYING
        };

        // vars (internal use)
        time_ms_t _start_time = 0;
        // 0 -> first step
        // 1 -> reaching weight by watering
        // 2 -> reaching weight by drying
        // 3 -> holding weight
        Stage _step_stage = FIRST_STEP;

        void _reset_internal_vars()
        {
            _step_stage = FIRST_STEP;
        }
    };

    // always gets to the upper weight and then starts the oscillation
    struct ProtocolStep_OscillateInWeightRange
    {
        // data
        weight_t lower_weight, upper_weight;
        uint8_t n_cycles;

        enum Stage : uint8_t
        {
            FIRST_STEP,
            REACHING_WEIGHT_BY_WATERING,
            REACHING_WEIGHT_BY_DRYING,
            WATERING_TO_WEIGHT,
            DRYING_TO_WEIGHT
        };

        // vars (internal use)
        // 0 -> first step
        // 1 -> reaching weight by watering
        // 2 -> reaching weight by drying
        // 3 -> watering to weight
        // 4 -> drying to weight
        Stage _step_stage = FIRST_STEP;
        uint8_t _curr_cycle = 0;

        void _reset_internal_vars()
        {
            _step_stage = FIRST_STEP;
            _curr_cycle = 0;
        }
    };

    struct ProtocolStep
    {
        ProtocolStepType type;
        union
        {
            ProtocolStep_Wait wait;
            ProtocolStep_HoldWeight hold_weight;
            ProtocolStep_OscillateInWeightRange oscilate_in_range;
        } step;

        ProtocolStep() : type(ProtocolStepType::NONE), step{} {}

        static ProtocolStep make_none()
        {
            ProtocolStep step;
            step.type = ProtocolStepType::NONE;
            return step;
        }

        static ProtocolStep make_wait(time_ms_t wait_ms)
        {
            ProtocolStep step;
            step.type = ProtocolStepType::WAIT;
            step.step.wait.wait_ms = wait_ms;
            return step;
        }

        static ProtocolStep make_hold_weight(weight_t weight, weight_t weight_tolerance, time_ms_t wait_ms)
        {
            if (weight < static_cast<weight_t>(0))
            {
                STOMASENSE_PANIC("weight shouldn't be less than 0 (was %f)", weight);
            }
            ProtocolStep step;
            step.type = ProtocolStepType::HOLD_WEIGHT;
            step.step.hold_weight.weight = weight;
            step.step.hold_weight.weight_tolerance = weight_tolerance;
            step.step.hold_weight.wait_ms = wait_ms;
            step.step.hold_weight._reset_internal_vars();
            return step;
        }

        static ProtocolStep make_oscillate_in_range(weight_t lower_weight, weight_t upper_weight, uint8_t n_cycles)
        {
            if (lower_weight < static_cast<weight_t>(0))
            {
                STOMASENSE_PANIC("lower_weight shouldn't be less than 0 (was %f)", lower_weight);
            }
            if (upper_weight < lower_weight)
            {
                STOMASENSE_PANIC("upper_weight (%f) shouldn't be less than lower_weight (%f)", upper_weight, lower_weight);
            }
            ProtocolStep step;
            step.type = ProtocolStepType::OSCILLATE_IN_RANGE;
            step.step.oscilate_in_range.lower_weight = lower_weight;
            step.step.oscilate_in_range.upper_weight = upper_weight;
            step.step.oscilate_in_range.n_cycles = n_cycles;
            step.step.oscilate_in_range._reset_internal_vars();
            return step;
        }
    };

    class Protocol
    {
    protected:
        ProtocolStep _protocol_steps[STOMA_SENSE_MAX_PROTOCOL_STEPS];
        uint8_t _n_protocol_steps = 0;
        uint8_t _curr_step_idx = 0;

    private:
        void _next_step_idx()
        {
            if (_curr_step_idx >= _n_protocol_steps - 1) // instead of >= is was a <= OMG
                _curr_step_idx = 0;
            else
                ++_curr_step_idx;
        }

        // for testing
    public:
        const ProtocolStep *get_curr_protocol_step() const { return &_protocol_steps[_curr_step_idx]; }

    public:
        void clear_steps() { _n_protocol_steps = 0; }
        bool add_step(const ProtocolStep *protocol_step);
        bool add_steps(const ProtocolStep *protocol_steps, size_t size);
        bool add_steps(std::initializer_list<ProtocolStep> protocol_steps);

        uint8_t get_curr_step_idx() const { return _curr_step_idx; }
        const ProtocolStep *get_protocol_steps() const { return _protocol_steps; }
        uint8_t get_n_steps() const { return _n_protocol_steps; }

        Protocol() {}
        Protocol(const ProtocolStep *protocol_steps, size_t size)
        {
            if (size > STOMA_SENSE_MAX_PROTOCOL_STEPS)
            {
                STOMASENSE_PANIC("Can't initialize Protocol because size was bigger than STOMA_SENSE_MAX_PROTOCOL_STEPS");
            }
            add_steps(protocol_steps, size);
        }
        Protocol(std::initializer_list<ProtocolStep> protocol_steps)
            : Protocol(protocol_steps.begin(), protocol_steps.size())
        {
        }

    private:
        inline bool _tick_none(weight_t weight, millis_t millis);
        inline bool _tick_wait(weight_t weight, millis_t millis);
        inline bool _tick_hold_weight(weight_t weight, millis_t millis);
        inline bool _tick_oscillate_in_range(weight_t weight, millis_t millis);

    public:
        // returns true if it should water
        bool tick(weight_t weight, millis_t millis);
    };

}

#endif /* _STOMA_SENSE_SESSION_H_ */