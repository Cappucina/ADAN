FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    build-essential \
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
