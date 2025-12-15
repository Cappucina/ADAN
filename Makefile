.SILENT:

docker:
	clear 2>/dev/null || true
	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

	python3 scripts/container.py || python scripts/container.py || python scripts/container.py

	-docker rm -f adan-dev-container >/dev/null 2>&1
	if [ -z "$$(docker ps -a -q -f name=^/adan-dev-container$$)" ]; then \
			docker run -dit --name adan-dev-container -u $$(id -u):$$(id -g) -v $$(pwd):/workspace adan-dev-container /bin/sh >/dev/null 2>&1; \
	elif [ -z "$$(docker ps -q -f name=^/adan-dev-container$$)" ]; then \
		docker start adan-dev-container >/dev/null 2>&1; \
	fi

	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

format:
	docker exec -i adan-dev-container sh -c "python3 ./scripts/beautifier.py --file ./compiled/assembled.s > assembled.tmp && mv assembled.tmp ./compiled/assembled.s"

compile: docker
	docker exec -i adan-dev-container sh -c "rm -rf compiled"
	docker exec -i adan-dev-container sh -c "mkdir -p compiled"
	# Define BUILDING_COMPILER_MAIN so runtime-only symbols in lib/adan
	# (e.g. read_file_source) are excluded when building the compiler and
	# therefore don't conflict with identical symbols in src/main.c.
	docker exec -i adan-dev-container sh -c "gcc -DBUILDING_COMPILER_MAIN src/*.c tests/*.c lib/adan/*.c -I include -o compiled/main"
	# Ensure files created by Docker are owned by the local user so local
	# builds can overwrite them without permission errors.
	sudo chown -R $$(id -u):$$(id -g) compiled || true

execute:
# 	docker exec -i adan-dev-container sh -c "cd /workspace && sudo compiled/main examples/stack-overflow-test.adn"
# 	docker exec -i adan-dev-container sh -c "cd /workspace && sudo compiled/main examples/simple.adn"
	docker exec -i adan-dev-container sh -c "cd /workspace && compiled/main examples/my-program.adn"
	# Build `compiled/program` inside the Linux container so host/OS differences
	# (Mach-O vs ELF, assembler syntax) don't cause assembly errors.
	docker exec -i adan-dev-container sh -c "gcc -I include -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1 || clang -I include compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1"
	docker exec -i adan-dev-container sh -c "./compiled/program"
	
	make format -silent

debug: compile
	make execute -silent

# 
#  CODESPACES EXCLUSIVE
# 
codespace:
	chmod +x ./scripts/codespaces.sh
	./scripts/codespaces.sh setup