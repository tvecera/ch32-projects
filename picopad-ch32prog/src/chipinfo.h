/**
 * @file chipinfo.h
 * @brief Display detailed CH32 chip information
 */

#ifndef CHIP_INFO_H
#define CHIP_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Display detailed information about connected CH32 chip
 * @note Shows chip name, flash size, UUID, part type, read protection status,
 *       and additional registers (USER/RDPR, DATA0/1, WRPR0-3, UNIID1-3)
 */
void RunChipInfo();

#ifdef __cplusplus
}
#endif

#endif
