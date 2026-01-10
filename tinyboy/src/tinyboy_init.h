
// ****************************************************************************
//
//                        TinyBoy - Device init
//
// ****************************************************************************

#ifndef _DEVINIT_H
#define _DEVINIT_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize device
void DevInit();

// Terminate device
void DevTerm();

#if USE_RCA
// Display update - send frame buffer to the display
void DispUpdate();

// RCA mode: I2C emulation functions for framebuffer writes
// Display select SSD1306 page, start transfer data
void DispI2C_SelectPage(int page);

// write a byte to framebuffer
void DispI2C_Write(u8 data);

// stop I2C communication (no-op for RCA mode)
void DispI2C_Stop(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // _DEVINIT_H
