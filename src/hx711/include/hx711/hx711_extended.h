#ifndef _HX711_EXTENDED_H_
#define _HX711_EXTENDED_H_

#include <hx711/hx711.h>
#include <StomaSense/defs.h>
#include <StomaSense/jsonifyable.h>

#include <limits>
#include <initializer_list>
#include <math.h>

namespace hx711 {
    namespace HX711Constants {
        // if STOMA_SENSE_N_SCALES = 16, n_multiplexers = 4
        constexpr uint8_t n_multiplexers = static_cast<uint8_t>(log2(STOMA_SENSE_N_SCALES));
        static_assert(n_multiplexers <= std::numeric_limits<uint8_t>::max(), "n_multiplexers is greater than max(uint8_t). loops involving n_multiplexers won't work");
    }

    struct HX711CalibrationWithJson : public HX711Calibration, public StomaSense::Jsonifyable {
        void to_json(JsonObject *obj) const override;
        bool from_json(JsonObject *obj) override;
    };

    class HX711Multiplexed : public HX711 {
        private:
            StomaSense::pin_size_t _mutliplexer_pins[HX711Constants::n_multiplexers];
            bool set_multiplexer_for_scale(StomaSense::scale_t scale);
    
        public:
            HX711Multiplexed(StomaSense::pin_size_t pin_sck, StomaSense::pin_size_t pin_dout, std::initializer_list<StomaSense::pin_size_t> multiplexer_pins, HX711Gain gain=HX711Gain::A128);
    
            void begin();
    
            bool read_raw_stats(StomaSense::scale_t scale, uint32_t n, float *mean, float *stdev, uint32_t *resulting_n, uint32_t timeout_ms=5000);
            bool read_calib_stats(uint32_t n, HX711CalibrationWithJson *calib, float *mean, float *stdev, uint32_t *resulting_n, uint32_t timeout_ms=5000);
        
            bool calib_offset(uint32_t n, HX711CalibrationWithJson *calib, uint32_t *resulting_n, uint32_t timeout_ms=5000);
            bool calib_slope(uint32_t n, float weight, float weight_error, HX711CalibrationWithJson *calib, uint32_t *resulting_n, uint32_t timeout_ms=5000);
        };
}


#endif /* _HX711_EXTENDED_H_ */