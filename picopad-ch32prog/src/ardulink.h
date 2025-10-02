/**
 * @file ardulink.h
 * @brief Ardulink protocol bridge for CH32 SWIO programmer
 *
 * Adapted from arduino-ch32v003-swio by BlueSyncLine
 * Original: https://gitlab.com/BlueSyncLine/arduino-ch32v003-swio
 * Author: BlueSyncLine
 */

#ifndef ARDULINK_H
#define ARDULINK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enter Ardulink protocol bridge mode
 * @note Creates USB CDC serial bridge for external tools
 * @note Protocol commands:
 *       - '!' = Start communication
 *       - '?' = Test connection
 *       - 'p' = Power on target MCU
 *       - 'P' = Power off target MCU
 *       - 'w' = Write 32-bit register
 *       - 'r' = Read 32-bit register
 * @note Button controls in bridge mode:
 *       - A: Restart minichlink connection
 *       - X: Power cycle target MCU
 *       - Y: Exit bridge mode
 */
void RunArdulink();

#ifdef __cplusplus
}
#endif

#endif
