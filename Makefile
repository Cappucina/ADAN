BUILD_DIR = build
BINARY = adan

SAMPLE = samples/testing.adn
SAMPLE_LL = $(patsubst %.adn,%.ll,$(SAMPLE))
SAMPLE_OUT = samples/testing

.PHONY: all build run emit link clean format build-macos-arm64 build-macos-x86_64 build-macos

all: build run

build: clean
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

emit: build
	@echo "Emitting LLVM IR to $(SAMPLE_LL)"
	@./$(BUILD_DIR)/$(BINARY) -f $(SAMPLE) -r

link: build
	@echo "Compiling and linking $(SAMPLE) -> $(SAMPLE_OUT)"
	@./$(BUILD_DIR)/$(BINARY) -f $(SAMPLE) -o $(SAMPLE_OUT)

run: link
	@clear
	@./$(SAMPLE_OUT)

build-macos-arm64:
	@mkdir -p build-macos-arm64
	@cmake -S . -B build-macos-arm64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-macos-arm64.cmake
	@cmake --build build-macos-arm64
	@echo "macOS arm64 binary: build-macos-arm64/adan"

build-macos-x86_64:
	@mkdir -p build-macos-x86_64
	@cmake -S . -B build-macos-x86_64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-macos-x86_64.cmake
	@cmake --build build-macos-x86_64
	@echo "macOS x86_64 binary: build-macos-x86_64/adan"

build-macos: build-macos-arm64 build-macos-x86_64
	@echo "Both macOS binaries built."

clean:
	@rm -rf $(BUILD_DIR) build-macos-arm64 build-macos-x86_64
	@rm -f $(SAMPLE_LL) $(SAMPLE_OUT)

format:
	@find ./src ./libs -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

install:
	@bash ./dependencies.sh

push:
	@bash ./push.sh