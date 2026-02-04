BUILD_DIR = build
BINARY = adan

.PHONY: all build run clean

all: run

build: clean
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run: build
	clear
	@./$(BUILD_DIR)/$(BINARY)

clean:
	@rm -rf $(BUILD_DIR)