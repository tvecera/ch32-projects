#include "../include.h"

/**
 * @brief Erase entire flash memory of connected CH32 chip
 * @note Performs: SWIO init → chip detection → whole chip erase → power cycle
 * @note This operation cannot be undone - all flash content will be lost
 */
void RunErase() {
    // Display warning header
    Print("\f\26Erase WHOLE CHIP!!!\27\3\n");
    Print("Initialize SWIO...\n");
    SwioInit();
    MinichlinkSetupInterface();

    Print("Erasing whole chip...\n");
    MinichlinkErase(0, 0, 1);
    Print("\n\26Done!\27\n\n");

    Print("Power cycling MCU...\n");
    McuPowerCycle(200);

    Print("Press any key to continue...\n");
    KeyFlush();
    while (KeyGet() == NOKEY) {}
}
