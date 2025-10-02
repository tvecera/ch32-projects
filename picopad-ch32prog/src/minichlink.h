/**
 * Simplified CH32 debug protocol implementation
 *
 * Based on minichlink from ch32v003fun by Charles Lohr (CNLohr)
 * Original: https://github.com/cnlohr/ch32v003fun/tree/master/minichlink
 * Author: Charles Lohr
 *
 * This simplified version supports only CH32V00x and CH32X03x MCU families
 */

#ifndef _MINICHLINK_H
#define _MINICHLINK_H

#ifdef __cplusplus
extern "C" {

#endif

/**
 * @brief Initialize SWIO interface and halt target MCU
 * @return 0 on success, negative error code on failure
 * @note Powers on target MCU, configures debug module, and verifies connection
 */
int MinichlinkSetupInterface();

/**
 * @brief Display information about connected CH32 chip
 * @param full If True, display additional registers (USER/RDPR, DATA0/1, WRPR0-3, UNIID1-3)
 * @return 0 on success, negative error code on failure
 * @note Shows chip name, flash size, UUID, part type, and read protection status
 */
int MinichlinkPrintChipInfo(Bool full);

/**
 * @brief Get flash size of target chip
 * @return Flash size in bytes
 * @note Automatically detects chip and returns flash_size from chip database
 */
u32 MinichlinkGetFlashSize();

/**
 * @brief Erase flash memory on target MCU
 * @param address Start address for sector erase (ignored if type=1)
 * @param length Length in bytes for sector erase (ignored if type=1)
 * @param type Erase type: 0=sector erase, 1=whole chip erase
 * @return 0 on success, negative error code on failure
 * @note Automatically unlocks flash before erasing
 */
int MinichlinkErase(u32 address, u32 length, u8 type);

/**
 * @brief Flash binary image to target MCU
 * @param offset Flash address to write to (typically 0x08000000)
 * @param len Length of binary image in bytes
 * @param image Pointer to binary image data
 * @return 0 on success, negative error code on failure
 * @note Performs whole chip erase before programming
 */
int MinichlinkFlash(u64 offset, u32 len, const u8* image);

#ifdef __cplusplus
}
#endif

#endif
