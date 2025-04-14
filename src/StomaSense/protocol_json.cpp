//
//  StomaSenseProtocolWithJson.cpp
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 11/04/2025.
//

#include <StomaSense/protocol_json.h>

#include <ArduinoJson.h>

namespace StomaSense
{
    namespace JsonConstants
    {
        const char *key_n_steps = "n_steps";
        const char *key_protocol_steps = "steps";
        const char *key_type = "type";
        const char *key_step = "step";

        // Hold Weight
        const char *key_weight = "weight";
        const char *key_weight_tolerance = "weight_tol";
        const char *key_wait_ms = "wait_ms";

        // Oscillate In Range
        const char *key_lower_weight = "weight_lo";
        const char *key_upper_weight = "weight_up";
        const char *key_n_cycles = "n_cycles";
    }

    void ProtocolWithJson::to_json(JsonObject *obj) const
    {
        (*obj)[JsonConstants::key_n_steps] = _n_protocol_steps;
        JsonArray arr = (*obj)[JsonConstants::key_protocol_steps].to<JsonArray>();

        for (uint8_t i = 0; i < _n_protocol_steps; ++i)
        {
            const ProtocolStep *step = &_protocol_steps[i];
            JsonDocument step_doc;
            JsonObject step_obj = step_doc.as<JsonObject>();

            switch (step->type)
            {
            case ProtocolStepType::NONE:
                step_obj[JsonConstants::key_type] = static_cast<uint8_t>(ProtocolStepType::NONE);
                break;
            case ProtocolStepType::WAIT:
                step_obj[JsonConstants::key_type] = static_cast<uint8_t>(ProtocolStepType::WAIT);
                {
                    JsonObject step_obj_step = step_obj[JsonConstants::key_step].to<JsonObject>();
                    step_obj_step[JsonConstants::key_wait_ms] = step->step.wait.wait_ms;
                }
                break;
            case ProtocolStepType::HOLD_WEIGHT:
                step_obj[JsonConstants::key_type] = static_cast<uint8_t>(ProtocolStepType::HOLD_WEIGHT);
                {
                    JsonObject step_obj_step = step_obj[JsonConstants::key_step].to<JsonObject>();
                    step_obj_step[JsonConstants::key_weight] = step->step.hold_weight.weight;
                    step_obj_step[JsonConstants::key_weight_tolerance] = step->step.hold_weight.weight_tolerance;
                    step_obj_step[JsonConstants::key_wait_ms] = step->step.hold_weight.wait_ms;
                }
                break;
            case ProtocolStepType::OSCILLATE_IN_RANGE:
                step_obj[JsonConstants::key_type] = static_cast<uint8_t>(ProtocolStepType::OSCILLATE_IN_RANGE);
                {
                    JsonObject step_obj_step = step_obj[JsonConstants::key_step].to<JsonObject>();
                    step_obj_step[JsonConstants::key_lower_weight] = step->step.oscilate_in_range.lower_weight;
                    step_obj_step[JsonConstants::key_upper_weight] = step->step.oscilate_in_range.upper_weight;
                    step_obj_step[JsonConstants::key_n_cycles] = step->step.oscilate_in_range.n_cycles;
                }
                break;
            }

            arr.add(step_obj);
        }
    }

    bool ProtocolWithJson::from_json(JsonObject *obj)
    {
        // validate first

        // n_protocol_steps
        if (!(*obj)[JsonConstants::key_n_steps].is<uint8_t>())
        {
            STOMASENSE_WARN("not a uint8_t");
            return false;
        }
        if ((*obj)[JsonConstants::key_n_steps].is<uint8_t>() > STOMA_SENSE_MAX_PROTOCOL_STEPS)
        {
            STOMASENSE_WARN("n_steps is greater than STOMA_SENSE_MAX_PROTOCOL_STEPS");
            return false;
        }

        // check steps array
        if (!(*obj)[JsonConstants::key_protocol_steps].is<JsonArray>())
        {
            STOMASENSE_WARN("not an array");
            return false;
        }
        JsonArray arr = (*obj)[JsonConstants::key_protocol_steps].as<JsonArray>();
        if (arr.size() > STOMA_SENSE_MAX_PROTOCOL_STEPS)
        {
            STOMASENSE_WARN("arr size is greater than STOMA_SENSE_MAX_PROTOCOL_STEPS");
            return false;
        }

        // create a temp protocol
        Protocol protocol;
        for (const JsonVariant v : arr)
        {
            // check that it's well formed
            if (!v.is<JsonObject>())
            {
                STOMASENSE_WARN("not an object");
                return false;
            }
            JsonObject step_obj = v.as<JsonObject>();

            if (!step_obj[JsonConstants::key_type].is<uint8_t>())
            {
                STOMASENSE_WARN("not a uint8_t");
                return false;
            }
            if (!step_obj[JsonConstants::key_step].is<JsonObject>())
            {
                STOMASENSE_WARN("not an object");
                return false;
            }

            uint8_t type = step_obj[JsonConstants::key_type].as<uint8_t>();
            JsonObject step_obj_step = step_obj[JsonConstants::key_step].as<JsonObject>();

            ProtocolStep step;
            switch (type)
            {
            case ProtocolStepType::NONE:
                step.type = ProtocolStepType::NONE;
                break;
            case ProtocolStepType::WAIT:
                step.type = ProtocolStepType::WAIT;
                if (!step_obj_step[JsonConstants::key_wait_ms].is<time_ms_t>())
                {
                    STOMASENSE_WARN("not a time_ms_t");
                    return false;
                }
                step.step.wait.wait_ms = step_obj_step[JsonConstants::key_wait_ms].as<time_ms_t>();
                break;
            case ProtocolStepType::HOLD_WEIGHT:
                step.type = ProtocolStepType::HOLD_WEIGHT;
                if (!step_obj_step[JsonConstants::key_weight].is<weight_t>())
                {
                    STOMASENSE_WARN("not a weight_t");
                    return false;
                }
                if (!step_obj_step[JsonConstants::key_weight_tolerance].is<time_ms_t>())
                {
                    STOMASENSE_WARN("not a weight_t");
                    return false;
                }
                if (!step_obj_step[JsonConstants::key_wait_ms].is<time_ms_t>())
                {
                    STOMASENSE_WARN("not a time_ms_t");
                    return false;
                }
                step.step.hold_weight.weight = step_obj_step[JsonConstants::key_weight].as<weight_t>();
                step.step.hold_weight.weight_tolerance = step_obj_step[JsonConstants::key_weight_tolerance].as<weight_t>();
                step.step.hold_weight.wait_ms = step_obj_step[JsonConstants::key_wait_ms].as<time_ms_t>();
                break;
            case ProtocolStepType::OSCILLATE_IN_RANGE:
                step.type = ProtocolStepType::OSCILLATE_IN_RANGE;
                if (!step_obj_step[JsonConstants::key_lower_weight].is<weight_t>())
                {
                    STOMASENSE_WARN("not a weight_t");
                    return false;
                }
                if (!step_obj_step[JsonConstants::key_upper_weight].is<time_ms_t>())
                {
                    STOMASENSE_WARN("not a weight_t");
                    return false;
                }
                if (!step_obj_step[JsonConstants::key_n_cycles].is<uint8_t>())
                {
                    STOMASENSE_WARN("not a uint8_t");
                    return false;
                }
                step.step.oscilate_in_range.lower_weight = step_obj_step[JsonConstants::key_lower_weight].as<weight_t>();
                step.step.oscilate_in_range.upper_weight = step_obj_step[JsonConstants::key_upper_weight].as<weight_t>();
                step.step.oscilate_in_range.n_cycles = step_obj_step[JsonConstants::key_n_cycles].as<uint8_t>();
                break;
            }

            if (!protocol.add_step(&step))
            {
                STOMASENSE_WARN("not enough space");
                return false;
            }
        }

        if (!add_steps(protocol.get_protocol_steps(), protocol.get_n_steps()))
        {
            STOMASENSE_WARN("not enough space");
            return false;
        }

        return true;
    }
}