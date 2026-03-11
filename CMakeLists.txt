# https://cliutils.gitlab.io/modern-cmake/README.html#shouldn-t-i-support-the-oldest-version-possible
cmake_minimum_required(VERSION 3.10)

project(ADAN LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/backend/linker)

set(EMBEDDED_DATA_HEADER ${CMAKE_BINARY_DIR}/embedded_libs_data.h)

add_custom_command(
	OUTPUT ${EMBEDDED_DATA_HEADER}
	COMMAND ${CMAKE_COMMAND} -E rm -f ${EMBEDDED_DATA_HEADER}
	COMMAND ${CMAKE_COMMAND}
		-DCOUNT=3
		-DVAR_0=LIB_IO_ADN
		"-DPATH_0=${CMAKE_SOURCE_DIR}/libs/io/io.adn"
		-DVAR_1=LIB_IO_H
		"-DPATH_1=${CMAKE_SOURCE_DIR}/libs/io/io.h"
		-DVAR_2=LIB_RUNTIME_H
		"-DPATH_2=${CMAKE_SOURCE_DIR}/src/backend/runtime/runtime.h"
		"-DOUTPUT_FILE=${EMBEDDED_DATA_HEADER}"
		-P ${CMAKE_SOURCE_DIR}/cmake/embed_libs.cmake
	DEPENDS
		${CMAKE_SOURCE_DIR}/libs/io/io.adn
		${CMAKE_SOURCE_DIR}/libs/io/io.h
		${CMAKE_SOURCE_DIR}/src/backend/runtime/runtime.h
		${CMAKE_SOURCE_DIR}/cmake/embed_libs.cmake
)

add_custom_target(embedded_libs_data DEPENDS ${EMBEDDED_DATA_HEADER})

file(GLOB_RECURSE SOURCES "src/*.c" "libs/*.c")

add_executable(adan ${SOURCES})
add_dependencies(adan embedded_libs_data)
target_include_directories(adan PRIVATE src libs ${CMAKE_BINARY_DIR})

if (TARGET adan_linker)
	target_link_libraries(adan PRIVATE adan_linker)
endif()