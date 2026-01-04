
# ASM source files
ifeq ($(USE_RCA),0)
  ASRC += ${DEVICE_DIR}/rvpc_vga_asm.S
  CSRC += ${DEVICE_DIR}/fonts/chars64/text_mode_font_80.c
  CSRC += ${DEVICE_DIR}/fonts/chars64/text_mode_font_81.c
  CSRC += ${DEVICE_DIR}/fonts/chars64/text_mode_font_thin_8x8_64.c
  CSRC += ${DEVICE_DIR}/fonts/chars64/text_mode_font_bold_8x8_64.c
  CSRC += ${DEVICE_DIR}/fonts/chars128/text_mode_font_rvpc.c
  CSRC += ${DEVICE_DIR}/fonts/chars128/text_mode_font_zx.c
  CSRC += ${DEVICE_DIR}/fonts/chars128/text_mode_font_thin_8x8.c
  CSRC += ${DEVICE_DIR}/fonts/chars128/text_mode_font_bold_8x8.c

  DEFINE += -D USE_RCA=0
else
  DEFINE += -D USE_RCA=1
endif

CSRC += ${DEVICE_DIR}/rvpc_vga.c
CSRC += ${DEVICE_DIR}/rvpc_draw.c
CSRC += ${DEVICE_DIR}/rvpc_key.c
CSRC += ${DEVICE_DIR}/rvpc_snd.c
CSRC += ${DEVICE_DIR}/rvpc_init.c

DEFINE += -D USE_RVPC=1
EXTRA_AFLAGS += -D REDUCE_CRT0_SIZE=1

ifeq ($(MCU),)
MCU=CH32V003x4
endif

