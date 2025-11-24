.SILENT:

docker:
	clear || cls
	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"
# 
# 	Build our Docker Image
# 
	python3 scripts/container.py || python scripts/container.py || py scripts/container.py

# 
# 	Start Container
# 
	docker rm adan-dev-container --force >/dev/null 2>&1
	if [ -z "$$(docker ps -a -q -f name=^/adan-dev-container$$)" ]; then \
		docker run -dit --name adan-dev-container -v ./:/workspace adan-dev-container /bin/sh >/dev/null 2>&1; \
	else \
		if [ -z "$$(docker ps -q -f name=^/adan-dev-container$$)" ]; then \
			docker start adan-dev-container >/dev/null 2>&1; \
		fi \
	fi
	echo "<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>-<>>><<<>>><<<>>><<<>>><<<>"

compile: docker
	-docker exec -it adan-dev-container rm -rf compiled
	docker exec -it adan-dev-container mkdir -p compiled
	docker exec -it adan-dev-container gcc src/*.c tests/*.c -I include -o compiled/main

run: compile
	docker exec -it adan-dev-container compiled/main
