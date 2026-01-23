MAKEFLAGS += --no-print-directory

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

build:
	@$(MAKE) clean
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

build-release:
	@$(MAKE) build BUILD_TYPE=Release

build-debug:
	@$(MAKE) build BUILD_TYPE=Debug

run: build-release
	@$(RELEASE_DIR)/$(EXE)

test: build-debug
	@clear || cls
	@$(DEBUG_DIR)/$(EXE) --tests

debug: build-debug
	@$(DEBUG_DIR)/$(EXE)

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)" 2>nul
else
	@rm -rf "$(BUILD_DIR)"
endif

format:
	@python scripts/format.py

help:
	@echo "ADAN Makefile targets:"
	@echo "  build         - Clean and compile the project (default)"
	@echo "  build-release - Build in release mode"
	@echo "  build-debug   - Build in debug mode"
	@echo "  run           - Build and run the release executable"
	@echo "  test          - Build and run the test suite (debug mode)"
	@echo "  debug         - Build with debug symbols and run"
	@echo "  clean         - Remove the build directory"
	@echo "  format        - Format all source files"
	@echo "  help          - Display this help message"

ifeq ($(OS),Windows_NT)
	INSTALL_CMD := mkdir C:\tools\adan && copy $(BUILD_DIR)\$(EXE) C:\tools\adan
else ifeq ($(UNAME),Darwin)
	INSTALL_CMD := sudo cp $(BUILD_DIR)/$(EXE) /usr/local/bin/ && \
				 sudo mkdir -p /usr/local/share/man/{man7,man1} && \
				 sudo cp ./man/adan.7 /usr/local/share/man/man7/ && \
				 sudo cp ./man/adan.1 /usr/local/share/man/man1/
else ifeq ($(UNAME),Linux)
	INSTALL_CMD := sudo cp $(BUILD_DIR)/$(EXE) /usr/local/bin/ && \
				 sudo mkdir -p /usr/local/share/man/{man7,man1} && \
				 sudo cp ./man/adan.7 /usr/local/share/man/man7/ && \
				 sudo cp ./man/adan.1 /usr/local/share/man/man1/
endif

install: $(BUILD_DIR)/$(EXE)
	@$(INSTALL_CMD)