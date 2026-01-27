cmake_minimum_required(VERSION 3.10)
project(adan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER_ID "GNU")

file(GLOB_RECURSE SRC_FILES source/*.c)
list(APPEND SRC_FILES ${CMAKE_SOURCE_DIR}/grammar.tab.c)

add_executable(adan ${SRC_FILES})

target_include_directories(adan PRIVATE include)
target_compile_options(adan PRIVATE
    -g -Wall -Wextra -O2 -Wundef -Wconversion -pedantic -std=c17 -march=native -funroll-loops
)
