EXE = adan

build: clean
	@cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake -B ./build

clean:
	@rm -rf ./build