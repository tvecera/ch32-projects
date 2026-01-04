##############################################################################
# CH32LibSDK Makefile for RVPC
# Framework: https://github.com/Panda381/CH32LibSDK
#
# Toolchain: xpack-riscv-none-elf-gcc (recommended)
#   - Download: https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack
#   - WCH toolchain (riscv-wch-elf-gcc) has issues with -flto causing
#     incorrect .constprop function clones that break bounds checking
#
# Usage (via wrapper scripts):
#   ./c.sh ch32v002   - build
#   ./d.sh ch32v002   - clean
#   ./e.sh ch32v002   - flash
#   ./r.sh ch32v002   - reset
#   ./x.sh ch32v002   - clean, build, flash
##############################################################################

# Target MCU: CH32V002x4, CH32V003x4
MCU?=CH32V003x4
USE_RCA?=0

# Device class for CH32LibSDK
DEVCLASS=ch32base

# Path to CH32LibSDK root directory (without trailing '/')
CH32_ROOT_PATH?=../../../../ch32/CH32LibSDK
CH32LIBSDK_OPENOCD_DIR?=../../../../ch32/OpenOCD
RCA_LIB_ROOT_PATH?=../../../rca-library
DEVICE_DIR?=../../src

# Include CH32LibSDK build system (stage 1 - configuration)
include ${CH32_ROOT_PATH}/Makefile1.inc

# Include common RVPC device code
include ${DEVICE_DIR}/_makefile.inc

ifeq ($(USE_RCA),1)
# Include rca-library Makefile fragment
include ${RCA_LIB_ROOT_PATH}/Makefile.ch32libsdk
endif

##############################################################################
# Project Configuration
##############################################################################

# Target project name
TARGET=TDDug

# Output directory
TARGETDIR=Games

##############################################################################
# Source Files
##############################################################################

# ASM source files
ASRC +=

# C source files
CSRC +=

# C++ source files
SRC += src/main.cpp
SRC += src/ClassTDDUG.cpp
SRC += src/FastTinyDriver.cpp
SRC += src/PictureTDDUG.cpp
SRC += src/Tiny-DDug.cpp

##############################################################################
# Build System (stage 2)
##############################################################################

# Additional linker flags (show memory usage)
LDFLAGS += -Wl,--print-memory-usage
EXTRA_CPPFLAGS += -Wno-register

override COMP = riscv-none-elf-

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
