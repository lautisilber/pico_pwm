add_executable(${PROJECT_NAME}
    main.cpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 11
    CXX_EXTENSIONS off
)

# target_include_directories(${TARGET_NAME} PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Config
# )
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(StomaSense)
add_subdirectory(Welfords)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    hardware_pwm

    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    ArduinoJson

    StomaSense
)

if (NOT TARGET tinyusb_device)
    message(FATAL_ERROR "Won't be able to output serial through USB because TinyUSB submodule is not initialized in the SDK. Please run command 'git submodule update --init --recursive' in the pico-sdk directory")
endif()


# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})