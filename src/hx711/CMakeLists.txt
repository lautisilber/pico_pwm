set(LIB_NAME hx711)

add_library(${LIB_NAME} STATIC
    hx711.cpp
    hx711_extended.cpp
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_include_directories(${PROJECT_NAME} PRIVATE include)

if (NOT TARGET Welfords)
    message(FATAL_ERROR "Didn't load Welfords")
endif()

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
    hardware_gpio

    StomaSense
    Welfords
)