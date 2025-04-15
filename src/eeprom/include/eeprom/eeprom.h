#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <pico/stdlib.h>
#include <pico/flash.h>
#include <hardware/flash.h>

// https://github.com/raspberrypi/pico-examples/blob/master/flash/program/flash_program.c

#define RP2040_FLASH_SIZE_KB          16 * 1024 // 16 MB
#define EEPROM_FLASH_TARGET_SIZE_KB   1024
#define EEPROM_FLASH_TARGET_OFFSET_KB (RP2040_FLASH_SIZE_KB - EEPROM_FLASH_TARGET_SIZE_KB - XIP_BASE)

namespace eeprom {
    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + EEPROM_FLASH_TARGET_OFFSET_KB);

    template <typename T>
    extern void put(T *data);

    template <typename T>
    extern void get(T *data);
}




#endif /* _EEPROM_H_ */