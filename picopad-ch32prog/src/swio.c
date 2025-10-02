/**
 * @file swio.c
 * @brief Single Wire Debug Interface (SWIO) protocol implementation
 *
 * Based on picorvd by Austin Appleby (aappleby)
 * Original: https://github.com/aappleby/picorvd/blob/master/src/PicoSWIO.cpp
 * Author: Austin Appleby
 */

#include "../include.h"
#include "swio_pio.pio.h"

/**
 * @brief Busy-wait loop for precise timing
 * @param count Number of iterations to wait
 * @note NOINLINE ensures compiler doesn't optimize away the delay
 * @note Used for 8µs reset pulse (~100 iterations at 125 MHz / 12 = 10.4 MHz)
 */
NOINLINE void BusyWait(const int count) {
    volatile int c = count;
    while (c)
        c = c - 1;
}

volatile u8 swio_debug = 0;
volatile u8 swio_initialized = 0;
u8 current_pin_swio = PIN_SWIO;

/**
 * @brief Initialize SWIO interface and PIO state machine
 * @note Configures PIN_SWIO for PIO0
 */
void SwioInit() {
    if (swio_initialized) return;

    // Configure GPIO
    GPIO_Init(current_pin_swio);
    GPIO_Drive(current_pin_swio, GPIO_DRIVE_STRENGTH_2MA);
    GPIO_Slow(current_pin_swio);
    GPIO_Fnc(current_pin_swio, GPIO_FUNC_PIO0);

    McuPowerOn();
    WaitMs(10);

    const uint pio_sm = 0;

    // Reset PIO module
    pio0->ctrl = 0b000100010001;
    pio_sm_set_enabled(pio0, pio_sm, False);

    // Upload PIO program
    pio_clear_instruction_memory(pio0);
    const uint pio_offset = pio_add_program(pio0, &singlewire_program);

    // Configure PIO module
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, pio_offset + singlewire_wrap_target, pio_offset + singlewire_wrap);
    sm_config_set_sideset(&c, 1, /*optional*/ False, /*pindirs*/ True);
    sm_config_set_out_pins(&c, current_pin_swio, 1);
    sm_config_set_in_pins(&c, current_pin_swio);
    sm_config_set_set_pins(&c, current_pin_swio, 1);
    sm_config_set_sideset_pins(&c, current_pin_swio);
    sm_config_set_out_shift(&c, /*shift_right*/ False, /*autopull*/ False, /*pull_threshold*/ 32);
    sm_config_set_in_shift(&c, /*shift_right*/ False, /*autopush*/ True, /*push_threshold*/ 32);

    // 125 mhz / 12 = 96 nanoseconds per tick, close enough to 100 ns.
    sm_config_set_clkdiv(&c, 12);

    pio_sm_init(pio0, pio_sm, pio_offset, &c);
    pio_sm_set_pins(pio0, pio_sm, 0);
    pio_sm_set_enabled(pio0, pio_sm, True);

    // Grab pin and send an 8 usec low pulse to reset debug module
    // If we use the sdk functions to do this we get jitter :/
    sio_hw->gpio_clr = (1 << current_pin_swio);
    sio_hw->gpio_oe_set = (1 << current_pin_swio);
    iobank0_hw->io[current_pin_swio].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    BusyWait(100); // ~8 usec
    sio_hw->gpio_oe_clr = (1 << current_pin_swio);
    iobank0_hw->io[current_pin_swio].ctrl = GPIO_FUNC_PIO0 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;

    // Enable debug output pin on target
    SwioWriteReg32(DMSHDWCFGR, 0x5AA50400);
    SwioWriteReg32(DMCFGR, 0x5AA50400);

    // Reset debug module on target
    SwioWriteReg32(DMCONTROL, 0x00000000);
    SwioWriteReg32(DMCONTROL, 0x00000001);

    swio_initialized = 1;
}

/**
 * @brief Convert 7-bit register address to human-readable name
 * @param addr 7-bit register address
 * @return Register name as string, or hex value if unknown
 * @note Used for debug output when swio_debug is enabled
 */
static const char* AddrToRegname(const u8 addr) {
    switch (addr) {
        case DMCPBR:
            return "DMCPBR";
        case DMCFGR:
            return "DMCFGR";
        case DMSHDWCFGR:
            return "DMSHDWCFGR";
        case DMDATA0:
            return "DMDATA0";
        case DMDATA1:
            return "DMDATA1";
        case DMCONTROL:
            return "DMDMCONTROL";
        case DMSTATUS:
            return "DMDMSTATUS";
        case DMHARTINFO:
            return "DMHARTINFO";
        case DMABSTRACTCS:
            return "DMABSTRACTCS";
        case DMCOMMAND:
            return "DMCOMMAND";
        case DMABSTRACTAUTO:
            return "DMABSTRACTAUTO";
        case DMPROGBUF0:
            return "DMPROGBUF0";
        case DMPROGBUF1:
            return "DMPROGBUF1";
        case DMPROGBUF2:
            return "DMPROGBUF2";
        case DMPROGBUF3:
            return "DMPROGBUF3";
        case DMPROGBUF4:
            return "DMPROGBUF4";
        case DMPROGBUF5:
            return "DMPROGBUF5";
        case DMPROGBUF6:
            return "DMPROGBUF6";
        case DMPROGBUF7:
            return "DMPROGBUF7";
        case DMHALTSUM0:
            return "DMHALTSUM0";
        default:
            MemPrint(DecNumBuf, 5, "0x%02x", addr);
            return DecNumBuf;
    }
}

/**
 * @brief Write 32-bit value to debug module register via SWIO
 * @param reg_7_bit 7-bit register address (e.g., DMCONTROL, DMDATA0)
 * @param command 32-bit value to write
 * @return 0 on success, ERROR_NOT_INITIALIZED or ERROR_NOT_POWERED on failure
 * @note Uses PIO state machine for precise timing
 * @note Prints debug output if swio_debug flag is enabled
 */
int SwioWriteReg32(const u8 reg_7_bit, const u32 command) {
    if (!swio_initialized)
        return ERROR_NOT_INITIALIZED;
    if (!powered)
        return ERROR_NOT_POWERED;
    if (swio_debug) {
        Print("Write %15s 0x%08x\n", AddrToRegname(reg_7_bit), command);
    }
    pio_sm_put_blocking(pio0, 0, ((~reg_7_bit) << 1) | 0);
    pio_sm_put_blocking(pio0, 0, ~command);

    return 0;
}

/**
 * @brief Read 32-bit value from debug module register via SWIO
 * @param reg_7_bit 7-bit register address (e.g., DMSTATUS, DMDATA0)
 * @param result Pointer to store read value
 * @return 0 on success, ERROR_NOT_INITIALIZED or ERROR_NOT_POWERED on failure
 * @note Uses PIO state machine for precise timing
 * @note Prints debug output if swio_debug flag is enabled
 */
int SwioReadReg32(const u8 reg_7_bit, u32* result) {
    if (!swio_initialized)
        return ERROR_NOT_INITIALIZED;
    if (!powered)
        return ERROR_NOT_POWERED;
    pio_sm_put_blocking(pio0, 0, ((~reg_7_bit) << 1) | 1);
    *result = pio_sm_get_blocking(pio0, 0);
    if (swio_debug) {
        Print("Read  %15s 0x%08x\n", AddrToRegname(reg_7_bit), result);
    }
    return 0;
}
