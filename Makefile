# .SILENT:

EXE = adan
SRC = ./source
INC = ./include
BUILD_DIR = ./build

SRCS = $(shell find $(SRC) -name '*.c')

CC = gcc
CFLAGS = -g -Wall -Wextra -I./include -I../include

build:
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(BUILD_DIR)/$(EXE)

compile: clean build

run: build
	./$(BUILD_DIR)/$(EXE)

debug: clean
	@clear
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -DDEBUG $(SRCS) -o $(BUILD_DIR)/$(EXE)
	@./$(BUILD_DIR)/$(EXE)

clean:
	@rm -rf $(BUILD_DIR) $(EXE)

format:
	@find source include -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i

# 
#  Ignore the following lines UNLESS you are working in a
#   GitHub Codespaces environment.
# 
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh