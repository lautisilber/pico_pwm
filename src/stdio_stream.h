#ifndef _STDIO_STREAM_H_
#define _STDIO_STREAM_H_

#include <StomaSense/stream.h>
#include <SmartComm.h>
#include <pico/stdio.h>

class StdioStream : public StomaSense::Stream
{
public:
    size_t write(uint8_t c) override {
        return stdio_putchar(c);
    }

    int read() override {
        return stdio_getchar_timeout_us(1000);
    }

    static inline void set_available_cb(void (*fn)(void*))
    {
        stdio_set_chars_available_callback(fn, NULL);
    }
};


#endif /* _STDIO_STREAM_H_ */