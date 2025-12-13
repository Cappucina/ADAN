.SILENT:

docker:
	clear || cls
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
	docker exec -i adan-dev-container sh -c "sudo compiled/main examples/my-program.adn"
	docker exec -i adan-dev-container sh -c "sudo gcc -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program"
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