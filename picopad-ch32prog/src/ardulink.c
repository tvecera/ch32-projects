/**
 * @file ardulink.c
 * @brief Ardulink protocol bridge implementation for CH32 SWIO programmer
 *
 * Adapted from arduino-ch32v003-swio by BlueSyncLine
 * Original: https://gitlab.com/BlueSyncLine/arduino-ch32v003-swio
 * Author: BlueSyncLine
 */

#include "../include.h"

// Protocol commands
#define PROTOCOL_START '!'
#define PROTOCOL_ACK '+'
#define PROTOCOL_NACK '0'
#define PROTOCOL_TEST '?'
#define PROTOCOL_POWER_ON 'p'
#define PROTOCOL_POWER_OFF 'P'
#define PROTOCOL_WRITE_REG 'w'
#define PROTOCOL_READ_REG 'r'

/**
 * @brief Enter Ardulink protocol bridge mode
 * @note Creates USB CDC serial bridge for external tools
 * @note Protocol commands:
 *       - '!' = Start communication
 *       - '?' = Test connection
 *       - 'p' = Power on target MCU
 *       - 'P' = Power off target MCU
 *       - 'w' = Write 32-bit register (format: 'w' <reg> <value>)
 *       - 'r' = Read 32-bit register (format: 'r' <reg>)
 * @note Responses: '+' = ACK (success), '0' = NACK (error)
 * @note Button controls in bridge mode:
 *       - A: Restart Ardulink connection
 *       - X: Power cycle target MCU
 *       - Y: Exit bridge mode
 */
void RunArdulink() {
  u8 reg;
  u32 val;
  u8 key;

  Print("\f\26CH32 Ardulink for Picopad\27\3\n\n");
  Print(" - A - to restart Ardulink connection\n");
  Print(" - X - power off and on MCU\n");
  Print(" - Y - back\n");
  Print("Connecting as CDC device...\n\n");
  KeyFlush();

  UsbDevInit(&UsbDevCdcSetupDesc);
  WaitMs(1000);
  while (!UsbCdcIsMounted()) {
    if (KeyGet() == KEY_Y) {
      UsbTerm();
      return;
    }
  }

  while (True) {
    Print("Waiting for Minichlink...\n");
    while (!UsbCdcReadReady()) {
      key = KeyGet();
      if (key == KEY_Y) {
        UsbTerm();
        return;
      }
    }

    // Initialize SWIO
    Print("Initialize SWIO...\n");
    SwioInit();

    // Send start signal
    UsbCdcWriteChar(PROTOCOL_START);

    while (True) {
      key = KeyGet();

      if (key == KEY_X) {
        McuPowerCycle(200);
      } else if (key == KEY_A) {
        break;
      }  else if (key == KEY_Y) {
        UsbTerm();
        return;
      }

      switch (UsbCdcReadChar()) {
      case PROTOCOL_TEST:
        UsbCdcWriteChar(PROTOCOL_ACK);
        break;

      case PROTOCOL_POWER_ON:
        McuPowerOn();
        UsbCdcWriteChar(PROTOCOL_ACK);
        break;

      case PROTOCOL_POWER_OFF:
        McuPowerOff();
        UsbCdcWriteChar(PROTOCOL_ACK);
        break;

      case PROTOCOL_WRITE_REG:
        UsbCdcReadData(&reg, sizeof(uint8_t));
        UsbCdcReadData(&val, sizeof(uint32_t));
        if (SwioWriteReg32(reg, val)) {
          Print("\24ERROR writing register %02x\27\n", reg);
          UsbCdcWriteChar(PROTOCOL_NACK);
        } else {
          UsbCdcWriteChar(PROTOCOL_ACK);
        }
        break;

      case PROTOCOL_READ_REG:
        UsbCdcReadData(&reg, sizeof(uint8_t));
        if (SwioReadReg32(reg, &val)) {
          Print("\24ERROR reading register %02x\27\n", reg);
          UsbCdcWriteChar(PROTOCOL_NACK);
        }
        UsbCdcWriteData(&val, sizeof(uint32_t));
        break;

      default:
        break;
      }
    }
  }
}
