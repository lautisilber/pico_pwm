set(LIB_NAME Welfords)

add_library(${LIB_NAME} STATIC
    welfords.cpp
)

# target_sources(${LIB_NAME} PRIVATE )

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${LIB_NAME} PRIVATE
    pico_stdlib
)