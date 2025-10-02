/**
 * @file swio.c
 * @brief Single Wire Debug Interface (SWIO) protocol implementation
 *
 * Based on picorvd by Austin Appleby (aappleby)
 * Original: https://github.com/aappleby/picorvd/blob/master/src/PicoSWIO.cpp
 * Author: Austin Appleby
 */

#ifndef SWIO_H
#define SWIO_H

#ifdef __cplusplus
extern "C" {
#endif

extern volatile u8 swio_debug;
extern u8 current_pin_swio;


#define DMDATA0        0x04
#define DMDATA1        0x05
#define DMCONTROL      0x10
#define DMSTATUS       0x11
#define DMHARTINFO     0x12
#define DMABSTRACTCS   0x16
#define DMCOMMAND      0x17
#define DMABSTRACTAUTO 0x18
#define DMPROGBUF0     0x20
#define DMPROGBUF1     0x21
#define DMPROGBUF2     0x22
#define DMPROGBUF3     0x23
#define DMPROGBUF4     0x24
#define DMPROGBUF5     0x25
#define DMPROGBUF6     0x26
#define DMPROGBUF7     0x27
#define DMHALTSUM0     0x40

#define DMCPBR         0x7C
#define DMCFGR         0x7D
#define DMSHDWCFGR     0x7E
#define DMCHIPID       0x7F

#define ERROR_NOT_INITIALIZED  (-1)  // SwioInit was not called
#define ERROR_NOT_POWERED      (-2)  // PowerOn was not called

/**
 * @brief Initialize SWIO interface and PIO state machine
 * @note Configures GPIO27 for PIO0, uploads singlewire program, and resets debug module
 * @note Sends 8µs low pulse to reset target debug module
 * @note Only initializes once (idempotent)
 */
void SwioInit();

#define WriteReg32(reg_7_bit, command) SwioWriteReg32(reg_7_bit, command)

/**
 * @brief Write 32-bit value to debug module register via SWIO
 * @param reg_7_bit 7-bit register address (e.g., DMCONTROL, DMDATA0)
 * @param command 32-bit value to write
 * @return 0 on success, ERROR_NOT_INITIALIZED or ERROR_NOT_POWERED on failure
 * @note Uses PIO state machine for precise timing
 */
int SwioWriteReg32(u8 reg_7_bit, u32 command);

#define ReadReg32(reg_7_bit, result) SwioReadReg32(reg_7_bit, result)

/**
 * @brief Read 32-bit value from debug module register via SWIO
 * @param reg_7_bit 7-bit register address (e.g., DMSTATUS, DMDATA0)
 * @param result Pointer to store read value
 * @return 0 on success, ERROR_NOT_INITIALIZED or ERROR_NOT_POWERED on failure
 * @note Uses PIO state machine for precise timing
 */
int SwioReadReg32(u8 reg_7_bit, u32* result);

#ifdef __cplusplus
}
#endif

#endif
