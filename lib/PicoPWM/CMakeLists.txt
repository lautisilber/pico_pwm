set(LIB_NAME PicoPWM)

add_library(${LIB_NAME} STATIC
    PicoPWM.c
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE include)

if (NOT TARGET hardware_pwm)
    message(FATAL_ERROR "Didn't load hardware_pwm")
endif()

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
    hardware_pwm
    hardware_clocks
)