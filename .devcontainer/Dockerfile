FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG CONAN_VERSION=2.12.2
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        clang \
        curl \
        git \
        llvm \
        llvm-dev \
        ninja-build \
        openjdk-17-jdk \
        pkg-config \
        python3-pip \
        python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir --break-system-packages "conan==${CONAN_VERSION}"

USER vscode
