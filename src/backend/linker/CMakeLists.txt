cmake_minimum_required(VERSION 3.10)

add_library(adan_linker STATIC linker.c)
target_include_directories(adan_linker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(adan_linker PROPERTIES
    OUTPUT_NAME adan_linker
)
