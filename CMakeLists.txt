cmake_minimum_required(VERSION 3.13...3.27)

set(PROJECT_NAME StomaSense_rp2040_firmware)
set(MAIN_TARGET_NAME StomaSense_rp2040)
set(TEST_TARGET_NAME StomaSense_rp2040_test)

# initialize pico-sdk from submodule
# note: this must happen before project()
# include(${CMAKE_SOURCE_DIR}/pico-sdk/pico_sdk_init.cmake)

if (DEFINED ENV{PICO_SDK_PATH})
message("Using local pico-sdk in " $ENV{PICO_SDK_PATH})
else()
set (PICO_SDK_FETCH_FROM_GIT TRUE)
message("Fetching pico-sdk from git")
endif()

set(PICO_SDK_PATH "$ENV{HOME}/dev/embedded/pico-sdk")
message(${PICO_SDK_PATH})
include(pico_sdk_import.cmake)

# Set any variables required for importing libraries
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel)
# initialize freertos
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FREERTOS_KERNEL_import.cmake)

# initialize the Raspberry Pi Pico SDK
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

# initialize libs
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)

# rest of your project
add_subdirectory(src)