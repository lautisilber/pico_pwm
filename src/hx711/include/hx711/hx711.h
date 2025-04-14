#ifndef _HX711_H_
#define _HX711_H_

#include <pico/stdlib.h>

#include <StomaSense/defs.h>

namespace hx711
{
    extern void delay_us(uint32_t delay);

    enum HX711Gain : uint8_t
    {
        A128 = 1,
        B64 = 3,
        B32 = 2,
    };

    struct HX711Calibration
    {
        float offset, slope, offset_e, slope_e;
        bool set_offset = false, set_slope = false;

        bool populated() const;
    };

    class HX711
    {
    private:
        const StomaSense::pin_size_t _pin_sck, _pin_dout;
        const HX711Gain _gain;

        inline void pulse();

    public:
        HX711(StomaSense::pin_size_t pin_sck, StomaSense::pin_size_t pin_dout, HX711Gain gain = HX711Gain::A128);

        void begin();

        bool is_ready();
        void wait_ready();
        bool wait_ready_timeout(StomaSense::time_ms_t timeout_ms = 5000);

        bool read_raw_single(int32_t *raw, StomaSense::time_ms_t timeout_ms = 5000);
        bool read_raw_stats(uint32_t n, float *mean, float *stdev, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms = 5000);
        bool read_calib_stats(uint32_t n, HX711Calibration *calib, float *mean, float *stdev, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms = 5000);

        bool calib_offset(uint32_t n, HX711Calibration *calib, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms = 5000);
        bool calib_slope(uint32_t n, float weight, float weight_error, HX711Calibration *calib, uint32_t *resulting_n, StomaSense::time_ms_t timeout_ms = 5000);

        void power_off(bool wait_until_power_off = false);
        void power_on();
    };

}

#endif /* _HX711_H_ */