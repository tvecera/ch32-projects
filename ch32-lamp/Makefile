# Target MCU: CH32V002x4
MCU=CH32V002x4

# Device class for CH32LibSDK
DEVCLASS=ch32base

# Path to CH32LibSDK root directory (without trailing '/')
CH32_ROOT_PATH?=../../CH32LibSDK
CH32LIBSDK_OPENOCD_DIR?=../../OpenOCD

# Flag - do not include boot section
NOBOOT=1

# Include CH32LibSDK build system (stage 1 - configuration)
include ${CH32_ROOT_PATH}/Makefile1.inc


##############################################################################
# Project Configuration
##############################################################################

# Output binary name
TARGET=lamp

# Output directory
TARGETDIR=Device

##############################################################################
# Source Files
##############################################################################

# ASM source files
ASRC +=

# C source files
CSRC += src/main.c
CSRC += src/sound.c
CSRC += src/adpcm.c
CSRC += src/input.c
CSRC += src/key.c

# C++ source files
SRC +=

# Additional linker flags (show memory usage)
LDFLAGS += -Wl,--print-memory-usage

# reduce program size
EXTRA_AFLAGS += -D REDUCE_CRT0_SIZE=1

override COMP = riscv-wch-elf-

##############################################################################
# Build System (stage 2)
##############################################################################

# Include CH32LibSDK build system (stage 2 - build rules)
include ${CH32_ROOT_PATH}/Makefile2.inc

##############################################################################
# Create .siz file from .elf file

./$(TARGET).siz: ./$(TARGET).elf
	@echo     siz		 $@
	@$(SZ) $< > $@
	@cat $@
