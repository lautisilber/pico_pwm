#include <pico/stdlib.h>
#include <stdio.h>
#include <StomaSense/defs.h>
#include <StomaSense/pin_defs.h>
// #include <tusb.h>
// #include <SmartComm.h>

// class StdioStream : public StomaSense::Stream
// {
// public:
//     size_t write(uint8_t c) override {
//         return stdio_putchar(c);
//     }

//     int read() override {
//         return stdio_getchar_timeout_us(1000);
//     }

//     size_t available() override {
//         uint8_t c;
//         tud_task();
//         return tud_cdc_peek(&c) ? (int) c : -1;
//     }
// };

using namespace StomaSense;

// StdioStream stdio_stream;

// // create the "OK" command which only answers with "OK"
// SMART_CMD_CREATE(cmdOk, "OK", [](Stream *stream, const SmartCmdArguments *args, const char *cmd) {
//     stream->println("OK");
// });

// // create the array of commands (note the & before the class names)
// const SmartCmdBase *cmds[] = {
//     &cmdOk//, &cmdHello, &cmdOne
// };

// // create the CmartComm main class with the length of the cmds array
// // between angled brackets <>
// SmartComm<ARRAY_LENGTH(cmds)> sc(cmds, stdio_stream);



int main() {
    stdio_init_all();

    for (;;)
    {
        int c = getchar_timeout_us(1000);
        if (c < 0) continue;

        switch(c) {
        case 'a':
            printf("OK");
            break;
        default:
           putchar(c);
           break;
        }
    }
    sleep_ms(1);

    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);
}