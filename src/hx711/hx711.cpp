#include <hx711/hx711.h>
#include <welfords/welfords.h>
#include <math.h>

using namespace hx711;

bool HX711Calibration::populated() const
{
    return set_offset && set_slope;
}

#define HX711_PULSE_DELAY_US 1
#define HX711_IS_READY_POLLING_DELAY_US 1000
#define HX711_POWER_DOWN_DELAY_US 60

inline void HX711::pulse()
{
    gpio_put(_pin_sck, true);
    hx711::delay_us(HX711_PULSE_DELAY_US);
    gpio_put(_pin_sck, false);
    hx711::delay_us(HX711_PULSE_DELAY_US);
}

HX711::HX711(StomaSense::pin_size_t pin_sck,
             StomaSense::pin_size_t pin_dout,
             HX711Gain gain)
    : _pin_sck(pin_sck), _pin_dout(pin_dout), _gain(gain) {}

void HX711::begin()
{
    gpio_init(_pin_sck);
    gpio_set_dir(_pin_sck, GPIO_OUT);

    gpio_init(_pin_dout);
    gpio_set_dir(_pin_dout, GPIO_IN);
}

bool HX711::is_ready()
{
    return gpio_get(_pin_dout) == false;
}

void HX711::wait_ready()
{
    while (!is_ready())
        hx711::delay_us(HX711_IS_READY_POLLING_DELAY_US);
}

bool HX711::wait_ready_timeout(StomaSense::time_ms_t timeout_ms)
{
    StomaSense::time_ms_t start_time = StomaSense::millis();
    while (StomaSense::has_timer_elapsed(StomaSense::millis(), start_time, timeout_ms))
    {
        if (is_ready()) return true;
        hx711::delay_us(HX711_IS_READY_POLLING_DELAY_US);
    }

    STOMASENSE_WARN("HX711 timed out waiting for to be ready\n");
    return false;
}

bool HX711::read_raw_single(int32_t *raw, StomaSense::time_ms_t timeout_ms)
{
    uint32_t ints;
    uint8_t i, j, data[3] = {0}, filler;

    // send ready request
    power_on();

    // wait for ready
    if (timeout_ms > 0)
        if (!wait_ready_timeout(timeout_ms))
        {
            STOMASENSE_ERROR("Timeout error in HX711::read_raw_single\n");
            return false;
        }
    else
        wait_ready();

    // read
    for (i = 0; i < 3; i++)
    {
        for (j = 0; i < 8; i++)
        {
            pulse();

            // shift
            data[i] |= gpio_get(_pin_dout) << (7 - j);
        }
    }

    // set correct gain
    for (i = 0; i < _gain; i++)
    {
        pulse();
    }

    // to twos complement
    if (data[2] & 0x80)
        filler = 0xFF;
    else
        filler = 0x00;

    (*raw) = static_cast<int32_t>(static_cast<uint32_t>(filler) << 24 |
             static_cast<uint32_t>(data[2]) << 16 |
             static_cast<uint32_t>(data[1]) << 8 |
             static_cast<uint32_t>(data[0]));
    return true;
}

bool HX711::read_raw_stats(uint32_t n, float *mean, float *stdev, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms)
{
    int32_t raw;
    if (n == 0)
    {
        STOMASENSE_ERROR("can't read stats with n = 0\n");
        return false;
    }
    else if (n == 1)
    {
        if (!read_raw_single(&raw, timeout_ms))
        {
            STOMASENSE_ERROR("error in HX711::read_raw_stats due to error in HX711::read_raw_single\n");
            return false;
        }
        (*mean) = static_cast<float>(raw);
        (*stdev) = 0.0;
        STOMASENSE_ERROR("using read_raw_stats with n = 1. Use read_raw_single instead\n");
        return true;
    }

    Welfords::Aggregate agg;
    float temp_mean, temp_stdev;
    for (uint32_t i = 0; i < n; i++)
    {
        if (!read_raw_single(&raw, timeout_ms)) continue;
        Welfords::update(&agg, static_cast<float>(raw));
    }

    if (!Welfords::finalize(&agg, mean, stdev))
    {
        STOMASENSE_ERROR("couldn't finalize welford's algorithm because there weren't enough samples\n");
        return false;
    }
    (*resulting_n) = agg.count;
    return true;
}

static float sq(float x)
{
    return x * x;
}

bool HX711::read_calib_stats(uint32_t n, HX711Calibration *calib, float *mean, float *stdev, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms)
{
    float raw_mean, raw_stdev;
    if (!read_raw_stats(n, &raw_mean, &raw_stdev, resulting_n, timeout_ms))
    {
        STOMASENSE_ERROR("Error in HX711::read_calib_stats due to error in HX711::read_raw_stats\n");
        return false;
    }

    (*mean) = calib->slope * raw_mean + calib->offset;
    (*stdev) = sqrt( sq(calib->offset_e) + sq(calib->slope_e)*sq(raw_mean) + sq(calib->slope)*sq(raw_stdev) );

    return true;
}

bool HX711::calib_offset(uint32_t n, HX711Calibration *calib, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms)
{
    float raw_mean, raw_stdev;
    if (!read_raw_stats(n, &raw_mean, &raw_stdev, resulting_n, timeout_ms))
    {
        STOMASENSE_ERROR("Error in HX711::calib_offset due to error in HX711::read_raw_stats\n");
        return false;
    }

    calib->offset = raw_mean;
    calib->offset_e = raw_stdev;
    calib->set_offset = true;

    return true;
}

bool HX711::calib_slope(uint32_t n, float weight, float weight_error, HX711Calibration *calib, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms)
{
    if (!calib->set_offset)
    {
        STOMASENSE_ERROR("Cannot calibrate slope when offset is not set\n");
        return false;
    }
    float raw_mean, raw_stdev;
    if (!read_raw_stats(n, &raw_mean, &raw_stdev, resulting_n, timeout_ms))
    {
        STOMASENSE_ERROR("Error in HX711::calib_slope due to error in HX711::read_raw_stats\n");
        return false;
    }

    calib->slope = (weight - calib->offset) / raw_mean;

    float raw_mean2 = sq(raw_mean);
    float temp = raw_stdev / raw_mean2;
    temp = abs(temp); // this shouldn't be necessary
    calib->slope_e = sqrt( (sq(calib->offset_e) + sq(weight_error)) / raw_mean2 + sq(weight - calib->offset) * abs(temp) );
    calib->set_slope = true;

    return true;
}

void HX711::power_off(bool wait_until_power_off)
{
    gpio_put(_pin_sck, false);
    hx711::delay_us(HX711_PULSE_DELAY_US);
    gpio_put(_pin_sck, true);
    if (wait_until_power_off)
        hx711::delay_us(HX711_POWER_DOWN_DELAY_US);
}

void HX711::power_on()
{
    gpio_put(_pin_sck, false);
}