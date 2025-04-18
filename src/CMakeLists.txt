add_library(PicoPWM)

target_sources(PicoPWM PRIVATE PicoPWM.c)

target_link_libraries(PicoPWM PUBLIC pico_stdlib hardware_pwm hardware_clocks)

target_include_directories(PicoPWM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})