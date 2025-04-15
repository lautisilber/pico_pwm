set(LIB_NAME PicoServo)


add_library(${LIB_NAME} STATIC
    PicoServo.cpp
)

pico_generate_pio_header(${LIB_NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_servo.pio)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
    hardware_pio
)