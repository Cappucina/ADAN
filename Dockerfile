# 
#  Choose Alpine Linux for its very light distribution
# 

FROM alpine:3.21

RUN apk add --no-cache \
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
