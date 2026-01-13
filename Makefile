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

all: build

build: clean
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build $(BUILD_DIR) -- -s

build-release:
	@$(MAKE) build BUILD_TYPE=Release

build-debug:
	@$(MAKE) build BUILD_TYPE=Debug

run: build-release
	@$(RELEASE_DIR)/$(EXE)

test: build-debug
	@$(DEBUG_DIR)/$(EXE) --tests

debug: build-debug
	@$(DEBUG_DIR)/$(EXE)

clean:
	@rm -rf $(BUILD_DIR) || rmdir $(BUILD_DIR)

format:
	@python scripts/format.py

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

ifeq ($(OS),Windows_NT)
    INSTALL_CMD = mkdir C:\tools\adan && copy $(BUILD_DIR)\$(EXE) C:\tools\adan
else ifeq ($(UNAME_S),Darwin)
    INSTALL_CMD = sudo cp $(BUILD_DIR)/$(EXE) /usr/local/bin/ && \
                  sudo mkdir -p /usr/local/share/man/{man7,man1} && \
                  sudo cp ./man/adan.7 /usr/local/share/man/man7/ && \
                  sudo cp ./man/adan.1 /usr/local/share/man/man1/
else ifeq ($(UNAME_S),Linux)
    INSTALL_CMD = sudo cp $(BUILD_DIR)/$(EXE) /usr/local/bin/ && \
                  sudo mkdir -p /usr/local/share/man/{man7,man1} && \
                  sudo cp ./man/adan.7 /usr/local/share/man/man7/ && \
                  sudo cp ./man/adan.1 /usr/local/share/man/man1/
endif

install: $(BUILD_DIR)/$(EXE)
	@$(INSTALL_CMD)

# 
#  Ignore the following lines UNLESS you are working in a
#  GitHub Codespaces environment.5
# 
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh
