EXE = adan

build: clean
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake -B $(BUILD_DIR)

clean:
	@rm -rf build