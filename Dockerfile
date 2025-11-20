FROM alpine:latest

RUN apk update && apk add --no-cache \
    build-base \
    clang \
    llvm \
    lld \
    cmake \
    git \
    bash

ENV CC=clang
ENV CXX=clang++

WORKDIR /workspace

CMD ["/bin/bash"]