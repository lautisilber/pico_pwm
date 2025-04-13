cmake_minimum_required(VERSION 3.13...3.27)

set(PROJECT_NAME StomaSense_rp2040)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(${CMAKE_SOURCE_DIR}/pico-sdk/pico_sdk_init.cmake)

# Set any variables required for importing libraries
SET(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel)
# initialize freertos
include(${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FREERTOS_KERNEL_import.cmake)

# initialize the Raspberry Pi Pico SDK
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

# initialize libs
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)

# rest of your project
add_subdirectory(src)