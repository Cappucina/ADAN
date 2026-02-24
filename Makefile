BUILD_DIR = build
BINARY = adan

.PHONY: all build run clean format

all: build run

build: clean
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run:
	clear
	@./$(BUILD_DIR)/$(BINARY) -f ./samples/hello.adn

clean:
	@rm -rf $(BUILD_DIR)

format:
	@find ./src ./libs -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +