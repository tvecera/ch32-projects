# Target MCU: CH32V002x4
MCU?=CH32V002x4

# Device class for CH32LibSDK
DEVCLASS=ch32base

# Path to CH32LibSDK root directory (without trailing '/')
CH32_ROOT_PATH?=../sdk/CH32LibSDK

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

##############################################################################
# Create .siz file from .elf file

./$(TARGET).siz: ./$(TARGET).elf
	@echo     siz		 $@
	@$(SZ) $< > $@
	@cat $@
