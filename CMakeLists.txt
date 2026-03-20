cmake_minimum_required(VERSION 3.10)
project(ADAN LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EMBEDDED_LIBS_HEADER "${CMAKE_BINARY_DIR}/embedded_libs_data.h")
file(WRITE "${EMBEDDED_LIBS_HEADER}" "// Auto-generated embedded libs\n\n")

include(${CMAKE_SOURCE_DIR}/cmake/embed_libs.cmake)

set(EMBED_DIRS 
	"${CMAKE_SOURCE_DIR}/libs"
	"${CMAKE_SOURCE_DIR}/src/backend/runtime"
)

set(ALL_EMBED_FILES "")
foreach(DIR ${EMBED_DIRS})
	file(GLOB_RECURSE FILES
		"${DIR}/*.adn"
		"${DIR}/*.c"
		"${DIR}/*.h"
	)
	list(APPEND ALL_EMBED_FILES ${FILES})
endforeach()

foreach(LIB_FILE ${ALL_EMBED_FILES})
	if(LIB_FILE MATCHES "^${CMAKE_SOURCE_DIR}/libs/")
		file(RELATIVE_PATH LIB_REL "${CMAKE_SOURCE_DIR}/libs" "${LIB_FILE}")
	else()
		file(RELATIVE_PATH LIB_REL "${CMAKE_SOURCE_DIR}/src/backend/runtime" "${LIB_FILE}")
	endif()

	string(REPLACE "/" ";" PARTS "${LIB_REL}")
	list(LENGTH PARTS LEN)
	if (LEN GREATER 1)
		list(GET PARTS 0 DIR)
		set(FILE_PART "")
		math(EXPR LAST_IDX "${LEN} - 1")
		foreach(I RANGE 1 ${LAST_IDX})
			list(GET PARTS ${I} P)
			if (FILE_PART STREQUAL "")
				set(FILE_PART "${P}")
			else()
				set(FILE_PART "${FILE_PART}_${P}")
			endif()
		endforeach()
		
		string(REPLACE "." "_" FILE_ID "${FILE_PART}")
		string(TOUPPER "${DIR}" DIR_UP)
		string(TOUPPER "${FILE_ID}" FILE_UP)
		
		if (FILE_UP MATCHES "^${DIR_UP}_")
			set(MACRO_NAME "${FILE_UP}")
		else()
			set(MACRO_NAME "${DIR_UP}_${FILE_UP}")
		endif()
	else()
		string(REPLACE "." "_" LIB_ID "${LIB_REL}")
		string(TOUPPER "${LIB_ID}" MACRO_NAME)
	endif()

	embed_file_as_c_string("${LIB_FILE}" "${EMBEDDED_LIBS_HEADER}" "LIB_${MACRO_NAME}")
endforeach()

file(APPEND "${EMBEDDED_LIBS_HEADER}" "#ifndef LIB_RUNTIME_ADN\n#define LIB_RUNTIME_ADN (void*)0\n#endif\n")

add_custom_target(embedded_libs_data ALL DEPENDS "${EMBEDDED_LIBS_HEADER}")

include_directories(${CMAKE_BINARY_DIR})
add_subdirectory(src/backend/linker)

file(GLOB_RECURSE SOURCES "src/*.c" "libs/*.c")

add_executable(adan ${SOURCES})
add_dependencies(adan embedded_libs_data)
target_include_directories(adan PRIVATE src libs ${CMAKE_BINARY_DIR})

if (TARGET adan_linker)
	target_link_libraries(adan PRIVATE adan_linker)
endif()