.SILENT:

ADAN = adan
SOURCE = ./source

OUTPUT_DIRECTORY = ./build
OUTPUT_TYPE ?= Debug

compile:
	cmake -B $(OUTPUT_DIRECTORY) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$(OUTPUT_TYPE)
	cmake --build $(OUTPUT_DIRECTORY) -- -s

# `run ARGS="..."`
run: clean compile
	$(OUTPUT_DIRECTORY)/$(ADAN) $(ARGS)

test: compile
	$(OUTPUT_DIRECTORY)/$(ADAN) --tests

clean:
	rm -rf $(OUTPUT_DIRECTORY)

format:
	python scripts/format.py

help:
	@echo "ADAN Makefile guide:"
	@echo "  compile       - Clean, format, and compile the compiler."
	@echo "  run           - Compile and immediately run the executable. Pass `ARGS=...` for arguments."
	@echo "  test          - Run the test suite to identify compiler errors."
	@echo "  clean         - Remove the output directory."
	@echo "  format        - Reformat all source files. Keeps consistency between programmers."
	@echo "  codespaces    - Only run when using a GitHub Codespaces environment."
	@echo "  help          - Displays this message."

#
#  Ignore the follow lines UNLESS you are working in a
#  GitHub Codespaces environment.
#
codespaces:
	@chmod +x ./scripts/codespaces.sh
	@./scripts/codespaces.sh