EXE = adan

RELEASE_DIR = ./build/release
DEBUG_DIR = ./build/debug

BUILD_TYPE ?= Release

ifeq ($(BUILD_TYPE),Debug)
    BUILD_DIR = $(DEBUG_DIR)
else
    BUILD_DIR = $(RELEASE_DIR)
endif

UNAME := $(shell uname -s 2>/dev/null || echo UNKNOWN)

build: clean
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake -B $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)

format:
	@python scripts/format.py