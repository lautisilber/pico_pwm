message(${MAIN_TARGET_NAME})
message(${TEST_TARGET_NAME})

add_executable(${MAIN_TARGET_NAME}
    main.cpp
)

add_executable(${TEST_TARGET_NAME}
    test_peripherals_commands.cpp
)

add_subdirectory(StomaSense)
add_subdirectory(bme280)
add_subdirectory(hx711)
add_subdirectory(SmartComm)

if (NOT TARGET tinyusb_device)
    message(FATAL_ERROR "Won't be able to output serial through USB because TinyUSB submodule is not initialized in the SDK. Please run command 'git submodule update --init --recursive' in the pico-sdk directory")
endif()

foreach(PN IN LISTS MAIN_TARGET_NAME TEST_TARGET_NAME)

    set_target_properties(${PN}
        PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS off
    )

    target_include_directories(${PN} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
    )

    target_link_libraries(${PN} PRIVATE
        pico_stdlib
        hardware_pwm

        pico_async_context_freertos
        FreeRTOS-Kernel-Heap4
        ArduinoJson

        StomaSense
        SmartComm

        PicoStepper
        PicoServo
        PicoPWM

        # tinyusb_device
        # tinyusb_board
    )

    # enable usb output, disable uart output
    pico_enable_stdio_usb(${PN} 1)
    pico_enable_stdio_uart(${PN} 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(${PN})

endforeach()