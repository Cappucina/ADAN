.SILENT:

PROJECT_DIR := $(shell pwd)

docker:
	@docker rm adan-c --force >/dev/null 2>&1
	@if [ -z "$$(docker ps -a -q -f name=^/adan-c$$)" ]; then \
		docker run -dit --name adan-c -v $(PROJECT_DIR):/workspace adan_c /bin/sh >/dev/null 2>&1; \
	else \
		if [ -z "$$(docker ps -q -f name=^/adan-c$$)" ]; then \
			docker start adan-c >/dev/null 2>&1; \
		fi \
	fi

compile: docker
	-docker exec -it adan-c rm -rf compiled
	docker exec -it adan-c mkdir -p compiled
	docker exec -it adan-c gcc src/*.c tests/*.c -I include -o compiled/main

run: compile
	docker exec -it adan-c compiled/main
