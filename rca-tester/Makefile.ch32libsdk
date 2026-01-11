##############################################################################
# CH32LibSDK Makefile for Composite Video Project
# Framework: https://github.com/Panda381/CH32LibSDK
#
# Toolchain: xpack-riscv-none-elf-gcc (recommended)
#   - Download: https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack
#   - WCH toolchain (riscv-wch-elf-gcc) has issues with -flto causing
#     incorrect .constprop function clones that break bounds checking
#
# Usage (via wrapper scripts):
#   ./c.sh ch32libsdk ch32v003   - build
#   ./d.sh ch32libsdk ch32v003   - clean
#   ./e.sh ch32libsdk ch32v003   - flash
#   ./r.sh ch32libsdk ch32v003   - reset
#   ./x.sh ch32libsdk ch32v003   - clean, build, flash
##############################################################################

# Target MCU: CH32V002x4, CH32V003x4
MCU?=CH32V002x4

# Device class for CH32LibSDK
DEVCLASS=ch32base

# Path to CH32LibSDK root directory (without trailing '/')
CH32_ROOT_PATH?=../sdk/CH32LibSDK
RCA_LIB_ROOT_PATH?=../rca-library

# Include CH32LibSDK build system (stage 1 - configuration)
include ${CH32_ROOT_PATH}/Makefile1.inc

# Include rca-library Makefile fragment
include ${RCA_LIB_ROOT_PATH}/Makefile.ch32libsdk

##############################################################################
# Project Configuration
##############################################################################

# Output binary name
TARGET=rca-tester

# Output directory
TARGETDIR=TOOLS

##############################################################################
# Source Files
##############################################################################

# ASM source files
ASRC +=

#-----------------------------------------------------------------------------
# Application source files
#-----------------------------------------------------------------------------
CSRC += src/main.c
CSRC += src/draw.c
CSRC += src/print.c
CSRC += src/key.c
CSRC += src/snd.c

# C++ source files
SRC +=

# Additional linker flags (show memory usage)
LDFLAGS += -Wl,--print-memory-usage
EXTRA_AFLAGS += -D REDUCE_CRT0_SIZE=1
CFLAGS += -flto
LDFLAGS += -flto

override COMP = riscv-none-elf-

##############################################################################
# Build System (stage 2)
##############################################################################

# Include CH32LibSDK build system (stage 2 - build rules)
include ${CH32_ROOT_PATH}/Makefile2.inc

# Override architecture for xpack toolchain (add zicsr+zifencei for SDK inline asm)
ifeq ($(findstring CH32V003,$(MCU)),CH32V003)
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei -mabi=ilp32e
else
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei_zmmul -mabi=ilp32e
endif

CFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(CFLAGS))
LDFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(LDFLAGS))
AFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(AFLAGS))
# Define to use standard interrupt attribute instead of WCH-specific

CSRC := $(filter-out %ch32base.c,$(CSRC))

##############################################################################
# Create .siz file from .elf file

./$(TARGET).siz: ./$(TARGET).elf
	@echo     siz		 $@
	@$(SZ) $< > $@
	@cat $@
