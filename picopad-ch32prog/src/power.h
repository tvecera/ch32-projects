/**
 * @file power.h
 * @brief Power control for target MCU with multiple power modes
 *
 * Supports three power connection modes:
 * 1. 3V3 Direct - Direct connection to Picopad 3.3V rail (no power control)
 * 2. GPIO Direct - Direct power from GPIO pin (max 12 mA, GPIO HIGH = ON)
 * 3. PNP Switch - High-side PNP switching circuit (recommended, GPIO LOW = ON)
 *
 * Default mode is PNP Switch. Modes can be changed at runtime.
 */

#ifndef POWER_H
#define POWER_H

#ifdef __cplusplus
extern "C" {
#endif


#define POWER_MODE_3V3_DIRECT  1   ///< Direct connection to 3.3V (no power control)
#define POWER_MODE_GPIO_DIRECT 2   ///< Direct GPIO power (max 12mA, HIGH=ON)
#define POWER_MODE_PNP_SWITCH  3   ///< PNP transistor switching (recommended, LOW=ON)

extern volatile u8 powered;
extern volatile u8 current_power_mode;
extern volatile u8 current_pin_power;

/**
 * @brief Initialize power control GPIO (PIN_POWER)
 * @note Default mode is PNP Switch
 * @note Automatically turns on power after initialization
 */
void McuPowerInit();

/**
 * @brief Turn on target MCU power
 * @note Behavior depends on current power mode:
 *       - 3V3 Direct: No-op (power always on)
 *       - GPIO Direct: GPIO HIGH = ON
 *       - PNP Switch: GPIO LOW = ON
 */
void McuPowerOn();

/**
 * @brief Turn off target MCU power
 * @note Behavior depends on current power mode:
 *       - 3V3 Direct: No-op (cannot turn off)
 *       - GPIO Direct: GPIO LOW = OFF
 *       - PNP Switch: GPIO HIGH = OFF
 */
void McuPowerOff();

/**
 * @brief Check if target MCU power is currently on
 * @return 1 if powered, 0 if not powered
 */
u8 McuPowerIsOn();

/**
 * @brief Power cycle the target MCU (off → delay → on)
 * @param delay_ms Delay in milliseconds between power off and on (default 200ms if 0)
 * @note In 3V3 Direct mode, this only adds delay (cannot cycle power)
 */
void McuPowerCycle(u16 delay_ms);

/**
 * @brief Set power mode
 * @param mode New power mode to use
 * @note Performs safe power cycle when switching modes
 * @note Prints mode change information to console
 */
void McuPowerSetMode(u8 mode);

/**
 * @brief Get current power mode
 * @return Current power mode
 */
u8 McuPowerGetMode();

/**
 * @brief Get human-readable name of current power mode
 * @return String with mode name ("3V3 Direct", "GPIO Direct", or "PNP Switch")
 */
const char* McuPowerGetModeName();

#ifdef __cplusplus
}
#endif

#endif
