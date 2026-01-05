.SILENT:

UNAME := $(shell uname)
NUM_JOBS := 1

ifeq ($(UNAME), Linux)
	NUM_JOBS := $(shell nproc)
else ifeq ($(UNAME), Darwin)
	NUM_JOBS := $(shell sysctl -n hw.ncpu)
endif

MAKEFLAGS += "-j $(NUM_JOBS)"

EXE = adan
SRC = ./source
BUILD_DIR = ./build

SRCS = ./source/main.c \
       ./source/common/diagnostic.c \
       ./source/common/platform.c \
       ./source/common/error.c \
       ./source/driver/flags.c \
       ./source/lex/lexer.c \
       ./source/test/test.c \
       ./source/test/diagnostic_test.c \
       ./source/test/flags_test.c \
       ./source/test/lexer_test.c \

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -O2 -Wundef -Wconversion -pedantic -std=c17 -march=native -funroll-loops -I./include \
		 -I./source -I./source/common -I./source/lex -I./source/parse -I./source/ir -I./source/semantic -I./source/gen \
		 -I./source/driver -I./source/test

build: clean format
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(BUILD_DIR)/$(EXE)

compile: build

run: build
	$(BUILD_DIR)/$(EXE) $(ARGS)

tests: build
	$(BUILD_DIR)/$(EXE) --tests

test: tests

debug: clean format
	@clear
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -DDEBUG $(SRCS) -o $(BUILD_DIR)/$(EXE)
	@./$(BUILD_DIR)/$(EXE)

clean:
	@rm -rf $(BUILD_DIR)

format:
	@find source include -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i

help:
	@echo "ADAN Makefile targets:"
	@echo "  build        - Clean, format, and compile the project"
	@echo "  compile      - Alias for build"
	@echo "  run          - Build and run the executable (pass ARGS=... for arguments)"
	@echo "  tests        - Build and run the test suite"
	@echo "  debug        - Build with debug symbols and run immediately"
	@echo "  clean        - Remove the build directory"
	@echo "  format       - Format all C source and header files with clang-format"
	@echo "  codespaces   - Setup for GitHub Codespaces environment"
	@echo "  help         - Display this help message"

# 
#  Ignore the following lines UNLESS you are working in a
#  GitHub Codespaces environment.
# 
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh
