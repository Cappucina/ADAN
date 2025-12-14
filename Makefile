.SILENT:

docker:
	clear 2>/dev/null || true
	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

	python3 scripts/container.py || python scripts/container.py || python scripts/container.py

	-docker rm -f adan-dev-container >/dev/null 2>&1
	if [ -z "$$(docker ps -a -q -f name=^/adan-dev-container$$)" ]; then \
		docker run -dit --name adan-dev-container -v $$(pwd):/workspace adan-dev-container /bin/sh >/dev/null 2>&1; \
	elif [ -z "$$(docker ps -q -f name=^/adan-dev-container$$)" ]; then \
		docker start adan-dev-container >/dev/null 2>&1; \
	fi

	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

format:
	docker exec -i adan-dev-container sh -c "python3 ./scripts/beautifier.py --file ./compiled/assembled.s > assembled.tmp && mv assembled.tmp ./compiled/assembled.s"

compile: docker
	docker exec -i adan-dev-container sh -c "sudo rm -rf compiled"
	docker exec -i adan-dev-container sh -c "sudo mkdir -p compiled"
	# Define BUILDING_COMPILER_MAIN so runtime-only symbols in lib/adan
	# (e.g. read_file_source) are excluded when building the compiler and
	# therefore don't conflict with identical symbols in src/main.c.
	docker exec -i adan-dev-container sh -c "sudo gcc -DBUILDING_COMPILER_MAIN src/*.c tests/*.c lib/adan/*.c -I include -o compiled/main"

execute:
# 	docker exec -i adan-dev-container sh -c "sudo compiled/main examples/stack-overflow-test.adn"
# 	docker exec -i adan-dev-container sh -c "sudo compiled/main examples/simple.adn"
	docker exec -i adan-dev-container sh -c "sudo compiled/main examples/my-program.adn"
	@ARCH=$$(uname -m 2>/dev/null || echo "x86_64"); \
	if [ "$$ARCH" = "arm64" ] || [ "$$ARCH" = "aarch64" ]; then \
		# On Apple Silicon, use x86_64 arch to assemble x86-64 assembly (via Rosetta) \
		clang -I include -arch x86_64 compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1 || \
		gcc -I include -m64 -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1; \
	else \
		gcc -I include -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1 || \
		clang -I include compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1; \
	fi
	docker exec -i adan-dev-container sh -c "sudo ./compiled/program"
	
	make format -silent

debug: compile
	make execute -silent

# 
#  CODESPACES EXCLUSIVE
# 
codespace:
	chmod +x ./scripts/codespaces.sh
	./scripts/codespaces.sh setup