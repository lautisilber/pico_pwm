set(LIB_NAME BME280_SensorAPI)

add_library(${LIB_NAME} STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/BME280_SensorAPI/bme280.c
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE include)

# target_link_libraries(${LIB_NAME} PRIVATE
#     pico_stdlib
# )