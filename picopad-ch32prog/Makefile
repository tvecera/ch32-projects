TARGET=CH32PROG
DEVICE=picopad10
DEVCLASS=picopad

# ASM source files
ASRC +=

# C source files
CSRC += src/power.c
CSRC += src/swio.c
CSRC += src/ardulink.c
CSRC += src/minichlink.c
CSRC += src/chipinfo.c
CSRC += src/erase.c
CSRC += src/flash.c

# C++ source files
SRC += src/main.cpp

# Path to root directory from the project directory (without trailing '/' delimiter)
PICO_ROOT_PATH =../sdk/PicoLibSDK

# Detect OS and architecture for elf2uf2
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
    ifeq ($(UNAME_M),arm64)
        override UF = ../tools/PicoLibSDK/_tools/aarch64-macos/elf2uf2
    else ifeq ($(UNAME_M),x86_64)
        override UF = ../tools/PicoLibSDK/_tools/x86_64-macos/elf2uf2
    else
        $(error Unsupported architecture: $(UNAME_M))
    endif
else ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),aarch64)
        override UF = ../tools/PicoLibSDK/_tools/aarch64-linux-gnu/elf2uf2
    else ifeq ($(UNAME_M),x86_64)
        override UF = ../tools/PicoLibSDK/_tools/x86_64-linux-gnu/elf2uf2
    else
        $(error Unsupported architecture: $(UNAME_M))
    endif
else
    $(error Unsupported operating system: $(UNAME_S))
endif

# Makefile includes
include ${PICO_ROOT_PATH}/Makefile.inc
