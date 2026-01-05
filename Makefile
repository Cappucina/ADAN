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
INC = ./include
BUILD_DIR = ./build

SRCS = ./source/main.c \
       ./source/common/diagnostic.c \
       ./source/lex/lexer.c \
       # add more files explicity when they have code in them

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -O2 -Wundef -Wconversion -pedantic -std=c17 -march=native -funroll-loops -I./include -I../include

build: clean format
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(BUILD_DIR)/$(EXE)

compile: build

run: build
	$(BUILD_DIR)/$(EXE)

debug: clean format
	@clear
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -DDEBUG $(SRCS) -o $(BUILD_DIR)/$(EXE)
	@./$(BUILD_DIR)/$(EXE)

clean:
	@rm -rf $(BUILD_DIR)

format:
	@find source include -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i

# 
#  Ignore the following lines UNLESS you are working in a
#  GitHub Codespaces environment.
# 
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh
