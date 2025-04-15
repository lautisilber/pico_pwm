#ifndef _STOMASENSE_STREAM_H_
#define _STOMASENSE_STREAM_H_

#include <pico/stdlib.h>
#include <StomaSense/defs.h>

#define STOMASENSE_STREAM_READ_TIMEOUT_MS 10000

namespace StomaSense {
    class Stream {
    public:
        virtual size_t write(uint8_t c) = 0;
        virtual int read() = 0; // return -1 if not available
        
        size_t write(const uint8_t *buffer, size_t length);
        size_t readBytes(char *buffer, size_t length);

        // virtual size_t available() = 0;

        size_t print(const char *buf);
        size_t println(const char *buf);

    private:
        time_ms_t _start_ms;
        int timedRead();
    };
}



#endif /* _STOMASENSE_STREAM_H_ */