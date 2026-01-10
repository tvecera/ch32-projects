CSRC += ${DEVICE_DIR}/tinyboy_disp.c
CSRC += ${DEVICE_DIR}/tinyboy_draw.c
CSRC += ${DEVICE_DIR}/tinyboy_key.c
CSRC += ${DEVICE_DIR}/tinyboy_snd.c
CSRC += ${DEVICE_DIR}/tinyboy_bat.c
CSRC += ${DEVICE_DIR}/tinyboy_init.c

DEFINE += -D USE_TINYBOY=1
EXTRA_AFLAGS += -D REDUCE_CRT0_SIZE=1

MCU=CH32V002x4