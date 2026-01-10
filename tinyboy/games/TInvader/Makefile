
##############################################################################
#               Include Makefile 1st stage - prepare MCU type
##############################################################################

# Setup device class
MCU=CH32V002x4
DEVCLASS=ch32base

# Path to root directory from the project directory (without trailing '/' delimiter)
CH32_ROOT_PATH?=../../../sdk/CH32LibSDK
DEVICE_DIR?=../../src

# Include CH32LibSDK build system (stage 1 - configuration)
include ${CH32_ROOT_PATH}/Makefile1.inc

# Include common RVPC device code
include ${DEVICE_DIR}/_makefile.inc

##############################################################################
#                           Project base configuration
##############################################################################

# Target project name
TARGET=TInvader

# Destination directory
TARGETDIR=Games

##############################################################################
#                             Input files
##############################################################################

# ASM source files
ASRC +=

# C source files
CSRC += src/main.c
CSRC += src/Tiny-invaders.c

# C++ source files
SRC +=

##############################################################################
# Build System (stage 2)
##############################################################################

# Additional linker flags (show memory usage)
LDFLAGS += -Wl,--print-memory-usage

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

