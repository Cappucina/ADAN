.SILENT:

docker:
	clear || cls
	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

	python3 scripts/container.py || python scripts/container.py || py scripts/container.py

	-docker rm -f adan-dev-container >/dev/null 2>&1
	if [ -z "$$(docker ps -a -q -f name=^/adan-dev-container$$)" ]; then \
		docker run -dit --name adan-dev-container -v $$(pwd):/workspace adan-dev-container /bin/sh >/dev/null 2>&1; \
	elif [ -z "$$(docker ps -q -f name=^/adan-dev-container$$)" ]; then \
		docker start adan-dev-container >/dev/null 2>&1; \
	fi

	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

compile: docker
	docker exec -i adan-dev-container sh -c "rm -rf compiled && mkdir -p compiled && gcc src/*.c tests/*.c -I include -o compiled/main"

run: compile
	docker exec -it adan-dev-container compiled/main

# 
#  CODESPACES EXCLUSIVE
# 
codespace:
	chmod +x ./scripts/codespaces.sh
	./scripts/codespaces.sh setup