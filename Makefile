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
MAKEFLAGS += --no-print-directory

.PHONY: build clean format bison-build

build: clean
	$(MAKE) format
	$(MAKE) bison-build

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

# Utility targets

bison-build:
	@rm -rf grammar.tab.c grammar.tab.h
	@bison -d grammar.y