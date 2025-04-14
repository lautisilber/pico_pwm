//
//  StomaSenseSession.cpp
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 11/04/2025.
//

#include <StomaSense/protocol.h>
#include <cstring>

using namespace StomaSense;

inline bool Protocol::_tick_none(weight_t weight, millis_t millis)
{
    _next_step_idx(); // shouldn't forget to go to next step, should I?
    return false;     // Why on earth would this be true !?
}

inline bool Protocol::_tick_wait(weight_t weight, millis_t millis)
{
    ProtocolStep_Wait *step = &_protocol_steps[_curr_step_idx].step.wait;
    if (step->_initial_step)
    {
        step->_initial_step = false;
        step->_start_time = millis();
    }
    else
    {
        if (has_timer_elapsed(millis(), step->_start_time, step->wait_ms))
        {
            step->_initial_step = true;
            _next_step_idx();
        }
    }
    return false;
}

inline bool Protocol::_tick_hold_weight(weight_t weight, millis_t millis)
{
    ProtocolStep_HoldWeight *step = &_protocol_steps[_curr_step_idx].step.hold_weight;
    switch (step->_step_stage)
    {
    case ProtocolStep_HoldWeight::Stage::FIRST_STEP: // check_weight
        if (step->weight > weight)
            step->_step_stage = ProtocolStep_HoldWeight::Stage::REACHING_WEIGHT_BY_WATERING; // need to water
        else
            step->_step_stage = ProtocolStep_HoldWeight::Stage::REACHING_WEIGHT_BY_DRYING; // need to dry
        return step->_step_stage == ProtocolStep_HoldWeight::Stage::REACHING_WEIGHT_BY_WATERING;
    case ProtocolStep_HoldWeight::Stage::REACHING_WEIGHT_BY_WATERING: // holding weight
        if (weight >= step->weight)
        {
            // already reached weight, go to holding weight
            step->_step_stage = ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_DRYING;
            step->_start_time = millis();
            return false;
        }
        return true;
    case ProtocolStep_HoldWeight::Stage::REACHING_WEIGHT_BY_DRYING:
        if (weight <= step->weight)
        {
            // already dryed to weight, go to holding weight
            step->_step_stage = ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_DRYING;
            step->_start_time = millis();
        }
        return false;
    case ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_WATERING:
        if (step->wait_ms == 0 || has_timer_elapsed(millis(), step->_start_time, step->wait_ms))
        {                                 // finished
            step->_reset_internal_vars(); // reset this protocol
            _next_step_idx();             // advance protocol
            return false;
        }
        if (weight >= step->weight)
        { // too heavy
            step->_step_stage = ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_DRYING;
            return false; // go to HOLDING_WEIGHT_AND_DRYING
        }
        return true; // if it should water
    case ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_DRYING:
        // have to be careful not to overshoot the weight_tolerance
        if (step->wait_ms == 0 || has_timer_elapsed(millis(), step->_start_time, step->wait_ms))
        {                                 // finished
            step->_reset_internal_vars(); // reset this protocol
            _next_step_idx();             // advance protocol
            return false;
        }
        if (weight < step->weight - step->weight_tolerance)
        { // too light
            step->_step_stage = ProtocolStep_HoldWeight::Stage::HOLDING_WEIGHT_AND_WATERING;
            return true; // go to HOLDING_WEIGHT_AND_WATERING
        }
        return false;
    default:
        // error
        STOMASENSE_PANIC("This shouldn't be reachable");
        return false;
    }
}

inline bool Protocol::_tick_oscillate_in_range(weight_t weight, millis_t millis)
{
    ProtocolStep_OscillateInWeightRange *step = &_protocol_steps[_curr_step_idx].step.oscilate_in_range;
    switch (step->_step_stage)
    {
    case ProtocolStep_OscillateInWeightRange::Stage::FIRST_STEP:
        if (step->upper_weight > weight)
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::REACHING_WEIGHT_BY_WATERING; // need to water
        else
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::REACHING_WEIGHT_BY_DRYING; // need to dry
        return step->_step_stage == ProtocolStep_OscillateInWeightRange::Stage::REACHING_WEIGHT_BY_WATERING;
    case ProtocolStep_OscillateInWeightRange::Stage::REACHING_WEIGHT_BY_WATERING:
        if (weight >= step->lower_weight)
        {
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::WATERING_TO_WEIGHT;
            return false;
        }
        return true;
    case ProtocolStep_OscillateInWeightRange::Stage::REACHING_WEIGHT_BY_DRYING:
        if (weight <= step->upper_weight)
        {
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::DRYING_TO_WEIGHT;
        }
        return false;
    case ProtocolStep_OscillateInWeightRange::Stage::WATERING_TO_WEIGHT:
        if (weight >= step->upper_weight)
        {
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::DRYING_TO_WEIGHT;
            if (++step->_curr_cycle >= step->n_cycles)
            {
                step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::FIRST_STEP;
                step->_curr_cycle = 0;
                _next_step_idx();
                return false;
            }
            return false;
        }
        return true;
    case ProtocolStep_OscillateInWeightRange::Stage::DRYING_TO_WEIGHT:
        if (weight <= step->lower_weight)
        {
            step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::WATERING_TO_WEIGHT;
            if (++step->_curr_cycle >= step->n_cycles)
            {
                step->_step_stage = ProtocolStep_OscillateInWeightRange::Stage::FIRST_STEP;
                step->_curr_cycle = 0;
                _next_step_idx();
                return false;
            }
            return true;
        }
        return false;
    default:
        // error
        return false;
    }
}

bool Protocol::tick(weight_t weight, millis_t millis)
{
    switch (_protocol_steps[_curr_step_idx].type)
    {
    case ProtocolStepType::NONE:
        return _tick_none(weight, millis);

    case ProtocolStepType::WAIT:
        return _tick_wait(weight, millis);

    case ProtocolStepType::HOLD_WEIGHT:
        return _tick_hold_weight(weight, millis);

    case ProtocolStepType::OSCILLATE_IN_RANGE:
        return _tick_oscillate_in_range(weight, millis);

    default:
        // error
        return false;
    };
}

bool Protocol::add_step(const ProtocolStep *protocol_step)
{
    if (_n_protocol_steps >= STOMA_SENSE_MAX_PROTOCOL_STEPS - 1)
        return false;
    memcpy(&_protocol_steps[(_n_protocol_steps++) - 1], protocol_step, sizeof(ProtocolStep));
    return true;
}

bool Protocol::add_steps(const ProtocolStep *protocol_steps, size_t size)
{
    if (size > STOMA_SENSE_MAX_PROTOCOL_STEPS)
    {
        return false;
    }
    _n_protocol_steps = size; // Had forgotten to add this line!!!
    memcpy(_protocol_steps, protocol_steps, sizeof(ProtocolStep) * size);
    return true;
}

bool Protocol::add_steps(std::initializer_list<ProtocolStep> protocol_steps)
{
    return add_steps(protocol_steps.begin(), protocol_steps.size());
}