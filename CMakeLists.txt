cmake_minimum_required(VERSION 3.10)
project(adan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER_ID "GNU")

set(SOURCES
    source/main.c
    source/common/fs.c
    source/common/diagnostic.c
    source/common/platform.c
    source/common/error.c
    source/common/buffer.c
    source/driver/flags.c
    source/lex/lexer.c
    source/parse/parser.c
    source/semantic/semantic.c
    source/tests/test.c
    source/tests/flags_test.c
    source/tests/lexer_test.c
    source/tests/diagnostic_test.c
    source/tests/parser_test.c
    source/tests/semantic_test.c
)

add_executable(adan ${SOURCES})

target_include_directories(adan PRIVATE
    include
    source
    source/common
    source/lex
    source/syntax
    source/ir
    source/semantic
    source/gen
    source/driver
    source/tests
)

target_compile_options(adan PRIVATE
    -g -Wall -Wextra -Werror -O2 -Wundef -Wconversion -pedantic -std=c17 -march=native -funroll-loops
)
