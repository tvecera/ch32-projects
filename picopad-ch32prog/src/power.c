/**
 * @file power.c
 * @brief Power control for target MCU with multiple power modes
 *
 * Implements software-controlled power switching for CH32 target MCU.
 * Supports three power connection modes:
 *
 * 1. 3V3 Direct Mode:
 *    - CH32 connected directly to Picopad 3.3V rail
 *    - No power control available (always on)
 *    - Simplest connection, no external components needed
 *
 * 2. GPIO Direct Mode:
 *    - CH32 powered directly from PIN_POWER (GPIO)
 *    - GPIO HIGH = power ON, GPIO LOW = power OFF (inverted logic vs PNP)
 *    - Maximum current: 12 mA safe limit (16 mA absolute max)
 *    - Requires 100nF + 10µF capacitors on CH32 VDD
 *    - Only suitable for standalone CH32 chips without peripherals
 *
 * 3. PNP Switch Mode (default, recommended):
 *    - Uses PIN_POWER to control PNP transistor (S8550/2N3906)
 *    - GPIO LOW = power ON, GPIO HIGH = power OFF
 *    - Circuit: R1=1kΩ, R2=10kΩ, R3=1kΩ, R4=47kΩ (optional)
 *    - Output voltage: ~3.0-3.2V (V_CE(sat) ≈ 0.1-0.3V drop)
 *    - Maximum output current: ~25mA with shown resistor values
 */

#include "../include.h"
#include "power.h"

volatile u8 powered = 0;
volatile u8 power_initialized = 0;
volatile u8 current_power_mode = POWER_MODE_PNP_SWITCH;
volatile u8 current_pin_power = PIN_POWER;

/**
 * @brief Initialize power control GPIO (PIN_POWER)
 * @note Initial state: power OFF, then turn ON after 10ms delay
 */
void McuPowerInit() {
    switch (current_power_mode) {
        case POWER_MODE_GPIO_DIRECT:
        case POWER_MODE_PNP_SWITCH:
            GPIO_Init(current_pin_power);
            current_power_mode == POWER_MODE_GPIO_DIRECT ? GPIO_Out0(current_pin_power) : GPIO_Out1(current_pin_power);
            GPIO_DirOut(current_pin_power);
            GPIO_Drive(current_pin_power, GPIO_DRIVE_STRENGTH_2MA);
            GPIO_Slow(current_pin_power);
            break;
        default:
            // POWER_MODE_3V3_DIRECT requires no GPIO setup
            break;
    }
    // Wait for GPIO to stabilize
    WaitMs(10);

    power_initialized = 1;
    powered = 0;

    // Auto power-on
    McuPowerOn();
    WaitMs(100); // Wait for target MCU power to stabilize
}

/**
 * @brief Turn on target MCU power
 * @note Behavior depends on current power mode
 * @note No-op if power is already on or not initialized
 */
void McuPowerOn() {
    if (!power_initialized) return;
    if (powered) return;

    switch (current_power_mode) {
        case POWER_MODE_GPIO_DIRECT:
            GPIO_Out1(current_pin_power); // HIGH = ON (direct GPIO)
            powered = 1;
            break;
        case POWER_MODE_PNP_SWITCH:
            GPIO_Out0(current_pin_power); // LOW = ON (PNP transistor)
            powered = 1;
            break;
        default:
            // POWER_MODE_3V3_DIRECT No-op, power is always on
            powered = 1;
            break;
    }
}

/**
 * @brief Turn off target MCU power
 * @note Behavior depends on current power mode
 * @note No-op if power is already off or not initialized
 */
void McuPowerOff() {
    if (!power_initialized) return;
    if (!powered) return;

    switch (current_power_mode) {
        case POWER_MODE_GPIO_DIRECT:
            GPIO_Out0(current_pin_power); // LOW = OFF (direct GPIO)
            powered = 0;
            break;
        case POWER_MODE_PNP_SWITCH:
            GPIO_Out1(current_pin_power); // HIGH = OFF (PNP transistor)
            powered = 0;
            break;
        default:
            // POWER_MODE_3V3_DIRECT No-op, power is always on
            powered = 1;
            break;
    }
}

/**
 * @brief Check if target MCU power is currently on
 * @return 1 if powered, 0 if not powered
 */
u8 McuPowerIsOn() {
    return powered;
}

/**
 * @brief Power cycle the target MCU (off → delay → on)
 * @param delay_ms Delay in milliseconds between power off and on (default 200ms if 0)
 * @note Useful for resetting the target MCU to a known state
 * @note In 3V3 Direct mode, only adds delay (cannot cycle power)
 */
void McuPowerCycle(u16 delay_ms) {
    if (delay_ms == 0) delay_ms = 200; // Default delay

    McuPowerOff();
    WaitMs(delay_ms);
    McuPowerOn();
}

/**
 * @brief Set power mode
 * @param mode New power mode to use
 * @note Performs safe power cycle when switching modes
 */
void McuPowerSetMode(const u8 mode) {
    if (!power_initialized) return;
    if (mode == current_power_mode) return;

    // Safe mode switch: turn off, change mode, turn on
    McuPowerOff();
    WaitMs(100);

    current_power_mode = mode;

    McuPowerOn();
    WaitMs(100);
}

/**
 * @brief Get current power mode
 * @return Current power mode
 */
u8 McuPowerGetMode() {
    return current_power_mode;
}

/**
 * @brief Get human-readable name of current power mode
 * @return String with mode name
 */
const char* McuPowerGetModeName() {
    switch (current_power_mode) {
        case POWER_MODE_3V3_DIRECT:
            return "3V3 Direct";
        case POWER_MODE_GPIO_DIRECT:
            return "GPIO Direct";
        case POWER_MODE_PNP_SWITCH:
            return "PNP Switch";
        default:
            return "Unknown";
    }
}
