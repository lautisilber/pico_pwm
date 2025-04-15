#include <hx711/hx711_extended.h>
#include <pico/stdlib.h>

using namespace StomaSense;
using namespace hx711;

namespace hx711
{
    namespace JsonConstants
    {
        const char *key_offset = "offset";
        const char *key_slope = "slope";
        const char *key_offset_e = "offset_e";
        const char *key_slope_e = "slope_e";
        const char *key_set_offset = "set_offset";
        const char *key_set_slope = "set_slope";
    }
}

void HX711CalibrationWithJson::to_json(JsonObject *obj) const
{
    (*obj)[JsonConstants::key_offset] = offset;
    (*obj)[JsonConstants::key_slope] = slope;
    (*obj)[JsonConstants::key_offset_e] = offset_e;
    (*obj)[JsonConstants::key_slope_e] = slope_e;
    (*obj)[JsonConstants::key_set_offset] = set_offset;
    (*obj)[JsonConstants::key_set_slope] = set_slope;
}

bool HX711CalibrationWithJson::from_json(JsonObject *obj)
{
    if (!(*obj)[JsonConstants::key_offset].is<float>())
    {
        STOMASENSE_WARN("wasn't float");
        return false;
    }
    if (!(*obj)[JsonConstants::key_slope].is<float>())
    {
        STOMASENSE_WARN("wasn't float");
        return false;
    }
    if (!(*obj)[JsonConstants::key_offset_e].is<float>())
    {
        STOMASENSE_WARN("wasn't float");
        return false;
    }
    if (!(*obj)[JsonConstants::key_slope_e].is<float>())
    {
        STOMASENSE_WARN("wasn't float");
        return false;
    }
    if (!(*obj)[JsonConstants::key_set_offset].is<bool>())
    {
        STOMASENSE_WARN("wasn't bool");
        return false;
    }
    if (!(*obj)[JsonConstants::key_set_slope].is<bool>())
    {
        STOMASENSE_WARN("wasn't bool");
        return false;
    }

    offset = (*obj)[JsonConstants::key_offset].as<float>();
    slope = (*obj)[JsonConstants::key_slope].as<float>();
    offset_e = (*obj)[JsonConstants::key_offset_e].as<float>();
    slope_e = (*obj)[JsonConstants::key_slope_e].as<float>();
    set_offset = (*obj)[JsonConstants::key_set_offset].as<float>();
    set_slope = (*obj)[JsonConstants::key_set_slope].as<float>();

    return true;
}

HX711Multiplexed::HX711Multiplexed(pin_size_t pin_sck, pin_size_t pin_dout, std::initializer_list<pin_size_t> multiplexer_pins, HX711Gain gain)
    : HX711(pin_sck, pin_dout, gain)
{
    if (multiplexer_pins.size() != HX711Constants::n_multiplexers)
    {
        STOMASENSE_PANIC("length of multiplexer pin array doesn't match HX711Constants::n_multiplexers");
    }
    memcpy(_mutliplexer_pins, multiplexer_pins.begin(), sizeof(pin_size_t) * HX711Constants::n_multiplexers);
}

void HX711Multiplexed::begin()
{
    for (uint8_t i = 0; i < HX711Constants::n_multiplexers; ++i)
    {
        gpio_init(_mutliplexer_pins[i]);
        gpio_set_dir(_mutliplexer_pins[i], GPIO_OUT);
    }
}

bool HX711Multiplexed::set_multiplexer_for_scale(scale_t scale)
{
    if (scale >= STOMA_SENSE_N_SCALES)
    {
        STOMASENSE_ERROR("can't set multiplexer for scale with number >= STOMA_SENSE_N_SCALES");
        return false;
    }

    for (uint8_t i = 0; i < HX711Constants::n_multiplexers; ++i)
    {
        bool state = scale & (1 < i);
        gpio_put(_mutliplexer_pins[i], state);
    }
    return true;
}

bool HX711Multiplexed::read_raw_stats(scale_t scale, uint32_t n, float *mean, float *stdev, uint32_t *resulting_n, time_ms_t timeout_ms)
{
    if (!set_multiplexer_for_scale(scale))
        return false;

    return HX711::read_raw_stats(n, mean, stdev, resulting_n);
}

bool HX711Multiplexed::read_calib_stats(uint32_t n, HX711CalibrationWithJson *calib, float *mean, float *stdev, uint32_t *resulting_n, time_ms_t timeout_ms)
{
    if (!set_multiplexer_for_scale(calib->scale))
        return false;

    return HX711::read_calib_stats(n, calib, mean, stdev, resulting_n);
}

bool HX711Multiplexed::calib_offset(uint32_t n, HX711CalibrationWithJson *calib, uint32_t *resulting_n, time_ms_t timeout_ms)
{
    if (!set_multiplexer_for_scale(calib->scale))
        return false;

    return HX711::calib_offset(n, calib, resulting_n);
}

bool HX711Multiplexed::calib_slope(uint32_t n, float weight, float weight_error, HX711CalibrationWithJson *calib, uint32_t *resulting_n, time_ms_t timeout_ms)
{
    if (!set_multiplexer_for_scale(calib->scale))
        return false;

    return HX711::calib_slope(n, weight, weight_error, calib, resulting_n);
}
