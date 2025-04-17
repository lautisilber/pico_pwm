cmake_minimum_required(VERSION 3.13...3.27)

set(PROJECT_NAME StomaSense_rp2040_firmware)
set(MAIN_TARGET_NAME StomaSense_rp2040)
set(TEST_TARGET_NAME StomaSense_rp2040_test)

# initialize pico-sdk from submodule
# note: this must happen before project()
# include(${CMAKE_SOURCE_DIR}/pico-sdk/pico_sdk_init.cmake)
# if (DEFINED ENV{PICO_SDK_PATH})
# message($ENV{PICO_SDK_PATH})
# message(FATAL "KKOOK")
# else()
# message(FATAL "EEEERRR")
# endif()
set(PICO_SDK_PATH "$ENV{HOME}/dev/embedded/pico-sdk")
message(${PICO_SDK_PATH})
# set(PICO_PLATFORM "rp2040")
include(pico_sdk_import.cmake)

# Set any variables required for importing libraries
SET(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel)
# initialize freertos
include(${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FREERTOS_KERNEL_import.cmake)
# include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FREERTOS_KERNEL_import.cmake)

# initialize the Raspberry Pi Pico SDK
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

# initialize libs
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)

if (NOT TARGET BME280_SensorAPI)
    message(FATAL_ERROR "Didn't load BME280_SensorAPI")
endif()
if (NOT TARGET ArduinoJson)
    message(FATAL_ERROR "Didn't load ArduinoJson")
endif()

# rest of your project
add_subdirectory(src)