#include "StomaSense/defs.h"

StomaSense::time_ms_t StomaSense::millis() {
    return to_ms_since_boot(get_absolute_time());
}