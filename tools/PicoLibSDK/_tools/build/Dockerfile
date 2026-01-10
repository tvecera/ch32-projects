# Multi-architecture build container for elf2uf2 and LoaderCrc
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    g++-aarch64-linux-gnu \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
