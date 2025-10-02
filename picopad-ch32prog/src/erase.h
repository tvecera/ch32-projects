/**
 * @file erase.h
 * @brief Whole chip erase functionality for CH32 MCU
 */

#ifndef ERASE_H
#define ERASE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Erase entire flash memory of connected CH32 chip
 * @note Performs: SWIO init → chip detection → whole chip erase → power cycle
 * @note This operation cannot be undone - all flash content will be lost
 */
void RunErase();

#ifdef __cplusplus
}
#endif

#endif
