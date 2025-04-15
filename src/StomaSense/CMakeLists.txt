set(LIB_NAME StomaSense)

add_library(${LIB_NAME} STATIC
    defs.cpp
    protocol.cpp
    stream.cpp
    jsonifyable.cpp
    protocol_json.cpp
    pump.cpp
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_include_directories(${PROJECT_NAME} PRIVATE include)

if (NOT TARGET ArduinoJson)
    message(FATAL_ERROR "Didn't load ArduinoJson")
endif()
if (NOT TARGET PicoStepper)
    message(FATAL_ERROR "Didn't load PicoStepper")
endif()
if (NOT TARGET PicoPWM)
    message(FATAL_ERROR "Didn't load PicoPWM")
endif()
if (NOT TARGET PicoServo)
    message(FATAL_ERROR "Didn't load PicoServo")
endif()

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
    pico_time
    hardware_watchdog
)

target_link_libraries(${LIB_NAME} PUBLIC
    ArduinoJson
    
    PicoStepper
    PicoPWM
    PicoServo
)