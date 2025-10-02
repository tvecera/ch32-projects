/**
 * @file flash.h
 * @brief Flash binary files from SD card to CH32 MCU
 */

#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flash binary file from SD card to target MCU
 * @note Interactive file selection from /TINYBOY/ directory (BIN files only)
 * @note Performs: chip detection → file validation → whole chip erase → program → power cycle
 * @note File size must not exceed chip flash size (truncated with warning if larger)
 * @note Target address: 0x08000000 (start of flash memory)
 */
void RunFlash();

#ifdef __cplusplus
}
#endif

#endif
