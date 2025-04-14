#ifndef _BME_280_H_
#define _BME_280_H_

#include <pico/stdlib.h>

namespace BME280
{
    extern bool begin();
    extern bool read(float *hum, float *temp, float *pres);
    extern bool can_read();

    extern bool sleep();
    extern bool wake_up();
    extern bool reset();
}

#endif /* _BME_280_H_ */