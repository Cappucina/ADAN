# Prefer Arch Linux for its minimalist base image and rolling updates
FROM archlinux:latest

RUN sudo pacman -Syu --noconfirm && \
    sudo pacman -S --noconfirm git base-devel go python-pip && \
    pip install --no-cache-dir mkdocs mkdocs-material

RUN sudo pacman -S \
    cmake \
    clang-15 \
    llvm-15 \
    llvm-15-dev \
    pkg-config \
    libssl-dev \
    ca-certificates \
    wget

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"
RUN rustup install stable && rustup default stable

WORKDIR /ADAN-workspace

CMD ["/bin/bash"]