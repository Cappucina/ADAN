FROM alpine:3.21

RUN apk add --no-cache \
	build-base \
	clang \
	llvm \
	lld \
	cmake \
	git \
	bash \
	sudo \
	python3

ENV CC=clang
ENV CXX=clang++

WORKDIR /workspace

CMD ["/bin/bash"]
