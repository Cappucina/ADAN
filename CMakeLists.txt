# https://cliutils.gitlab.io/modern-cmake/README.html#shouldn-t-i-support-the-oldest-version-possible
cmake_minimum_required(VERSION 3.10)

project(ADAN LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.c" "libs/*.c")

add_executable(adan ${SOURCES})
target_include_directories(adan PRIVATE src libs)