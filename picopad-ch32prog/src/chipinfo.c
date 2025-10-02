/**
 * @file chipinfo.c
 * @brief Display detailed CH32 chip information
 */

#include "../include.h"

/**
 * @brief Display detailed information about connected CH32 chip
 * @note Shows chip name, flash size, UUID, part type, read protection status,
 *       and additional registers (USER/RDPR, DATA0/1, WRPR0-3, UNIID1-3)
 */
void RunChipInfo() {
    u8 key;

    Print("\f\26CH32 Chip Info\27\3\n");
    Print("Initialize SWIO...\n");
    SwioInit();
    MinichlinkSetupInterface();
    MinichlinkPrintChipInfo(True);

    Print("Press any key to continue...\n");
    KeyFlush();
    while (KeyGet() == NOKEY) {}
}
