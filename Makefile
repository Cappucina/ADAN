.SILENT:

UNAME := $(shell uname)
NUM_JOBS := 1

ifeq ($(UNAME), Linux)
	NUM_JOBS := $(shell nproc)
else ifeq ($(UNAME), Darwin)
	NUM_JOBS := $(shell sysctl -n hw.ncpu)
endif

MAKEFLAGS += "-j $(NUM_JOBS) -s"

SRC = ./source
BUILD_DIR = ./build
EXE = $(BUILD_DIR)/adan

# WTF is this?
SRCS = ./source/main.c \
	   ./source/common/fs.c \
       ./source/common/diagnostic.c \
       ./source/common/platform.c \
       ./source/common/error.c \
       ./source/common/buffer.c \
       ./source/driver/flags.c \
       ./source/lex/lexer.c \
       ./source/parse/parser.c \
       ./source/tests/test.c \
       ./source/tests/flags_test.c \
       ./source/tests/lexer_test.c \
       ./source/tests/diagnostic_test.c \
	   ./source/tests/parser_test.c \

CC = gcc
CFLAGS = -g -Wall -Wextra -O2 -Wundef -Wconversion -pedantic -std=c17 -march=native -funroll-loops -I./include \
		 -I./source -I./source/common -I./source/lex -I./source/syntax -I./source/ir -I./source/semantic -I./source/gen \
		 -I./source/driver -I./source/tests

build: clean format
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE)

compile: build

execute:
	$(EXE) $(ARGS)

run:
	$(MAKE) build
	$(MAKE) execute

tests: build
	$(EXE) --tests

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

install: $(EXE) ./man/adan.1 ./man/adan.7
	sudo cp $(EXE) /usr/local/bin/
	sudo mkdir -p /usr/local/share/man/{man7,man1}
	sudo cp ./man/adan.7 /usr/local/share/man/man7/
	sudo cp ./man/adan.1 /usr/local/share/man/man1/

# 
#  Ignore the following lines UNLESS you are working in a
#  GitHub Codespaces environment.5
# 
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh