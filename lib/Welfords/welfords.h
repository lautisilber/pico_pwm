//
//  Welfords.h
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 12/04/2025.
//

#ifndef _WELFORDS_H_
#define _WELFORDS_H_

#include <pico/stdlib.h>

// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

namespace Welfords {
    typedef struct Aggregate {
        uint32_t count = 0;
        float mean = 0, M2 = 0;
    } Aggregate;

    void update(Aggregate *agg, float new_value);
    bool finalize(Aggregate *agg, float *mean, float *stdev);
}

#endif /* _WELFORDS_H_ */