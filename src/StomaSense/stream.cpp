#include <StomaSense/stream.h>

#include <StomaSense/defs.h>

using namespace StomaSense;

size_t Stream::write(const uint8_t *buffer, size_t length)
{
    // inspired in https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Print.cpp#L34
    size_t n = 0;
    while (length--)
    {
        if (write(*buffer++))
            n++;
        else
            break;
    }
    return n;
}

int Stream::timedRead()
{
    // inspired in https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Stream.cpp#L31
    int c;
    _start_ms = millis();
    do
    {
        c = read();
        if (c >= 0)
            return c;
    } while (!has_timer_elapsed(millis(), _start_ms, STOMASENSE_STREAM_READ_TIMEOUT_MS));
    return -1; // -1 indicates timeout
}

size_t Stream::readBytes(char *buffer, size_t length)
{
    // inspired in https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Stream.cpp#L202
    size_t count = 0;
    while (count < length)
    {
        int c = timedRead();
        if (c < 0)
            break;
        *buffer++ = (char)c;
        count++;
    }
    return count;
}