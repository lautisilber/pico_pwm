set(LIB_NAME PicoServo)


add_library(${LIB_NAME} STATIC
    PicoServo.c
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
)

if (NOT TARGET PicoPioPWM)
    message(FATAL_ERROR "Didn't load PicoPioPWM")
endif()

target_link_libraries(${LIB_NAME} PUBLIC
    PicoPioPWM
)