EXE := adan

RELEASE_DIR := ./build/release
DEBUG_DIR := ./build/debug

BUILD_TYPE ?= Release

ifeq ($(BUILD_TYPE),Debug)
	BUILD_DIR := $(DEBUG_DIR)
else
	BUILD_DIR := $(RELEASE_DIR)
endif

UNAME := $(shell uname -s 2>/dev/null || echo UNKNOWN)

.PHONY: all build build-release build-debug run test debug clean format help install

all: build

build: clean
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)" 2>nul
else
	@rm -rf "$(BUILD_DIR)"
endif

format:
	@python scripts/format.py