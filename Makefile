BUILD_DIR = build
BINARY = adan

SAMPLE = samples/hello.adn
SAMPLE_LL = $(patsubst %.adn,%.ll,$(SAMPLE))
SAMPLE_OUT = samples/hello

.PHONY: all build run emit link clean format

all: build run

build: clean
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

emit: build $(SAMPLE_LL)

$(SAMPLE_LL): build
	@echo "Emitting LLVM IR to $(SAMPLE_LL)"
	@./$(BUILD_DIR)/$(BINARY) -f $(SAMPLE)

link: $(SAMPLE_OUT)

$(SAMPLE_OUT): $(SAMPLE_LL)
	@echo "Linking $(SAMPLE_LL) -> $(SAMPLE_OUT)"
	@clang $(SAMPLE_LL) libs/io/print.c -o $(SAMPLE_OUT)

run: link
	@clear
	@./$(SAMPLE_OUT)

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(SAMPLE_LL) $(SAMPLE_OUT)

format:
	@find ./src ./libs -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

install:
	@bash ./dependencies.sh