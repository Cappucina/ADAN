.SILENT:

docker:
	clear 2>/dev/null || true
# 	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

	python3 scripts/container.py || python scripts/container.py || python scripts/container.py

	if [ -z "$$(docker ps -a -q -f name=^/adan-dev-container$$)" ]; then \
		docker run -dit --name adan-dev-container -u $$(id -u):$$(id -g) -v $$(pwd):/workspace adan-dev-container /bin/sh >/dev/null 2>&1; \
	elif [ -z "$$(docker ps -q -f name=^/adan-dev-container$$)" ]; then \
		docker start adan-dev-container >/dev/null 2>&1; \
	fi

# 	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

format:
	docker exec -i adan-dev-container sh -c "python3 ./scripts/beautifier.py --file ./compiled/assembled.s > assembled.tmp && mv assembled.tmp ./compiled/assembled.s"

compile: docker
	docker exec -i adan-dev-container sh -c "rm -rf compiled"
	docker exec -i adan-dev-container sh -c "mkdir -p compiled"
	docker exec -i adan-dev-container sh -c "gcc -DBUILDING_COMPILER_MAIN src/*.c tests/*.c lib/adan/*.c -I include -I lib/adan/include -o compiled/main"
	
	sudo chown -R $$(id -u):$$(id -g) compiled || true

execute:
	docker exec -i adan-dev-container sh -c "cd /workspace && compiled/main examples/my-program.adn"

	docker exec -i adan-dev-container sh -c "gcc -I include -I lib/adan/include -I lib/adan/include -no-pie compiled/assembled.s lib/adan/*.c -lm -o compiled/program 2>&1 || clang -I include -I lib/adan/include -I lib/adan/include compiled/assembled.s lib/adan/*.c -lm -o compiled/program 2>&1"
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