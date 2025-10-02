/**
 * @file flash.c
 * @brief Flash binary files from SD card to CH32 MCU
 */

#include "../include.h"

char SampPath[APPPATH_PATHMAX + 10 + 1];
u8 buffer[65536]; // 64kB buffer for binary file

/**
 * @brief Flash binary file from SD card to target MCU
 * @note Interactive file selection from /TINYBOY/ directory (BIN files only)
 * @note Performs: chip detection → file validation → whole chip erase → program → power cycle
 * @note File size must not exceed chip flash size (truncated with warning if larger)
 * @note Target address: 0x08000000 (start of flash memory)
 */
void RunFlash() {
    // Stop drawing to prepare for file selection UI
    DrawPrintStop();

    // Initialize SWIO and check chip connection
    SwioInit();
    MinichlinkSetupInterface();

    // Get chip flash size to validate binary file size
    const u32 chip_flash_size = MinichlinkGetFlashSize();
    if (chip_flash_size == 0) {
        FileSelDispBigErr("Cannot read chip flash size");
        return;
    }

    // Set up file selection path (/TINYBOY/ directory on SD card)
    GetHomePath(SampPath, "/TINYBOY");

    // Initialize file selection UI (BIN files only, green theme)
    FileSelInit(SampPath, "TINYBOY", "BIN", &FileSelColGreen);

    // Let user select a file (or exit with Y button)
    if (!FileSel()) {
        ResetToBootLoader();
        return;
    }

    // Try to open the selected file
    if (!FileOpen(&FileSelPrevFile, FileSelTempBuf)) {
        FileSelDispBigErr("Cannot open file");
        return;
    }

    // Switch back to text UI for programming progress
    DrawPrintStart();
    Print("\f\26Flash Binary File: %s\27\3\n", FileSelLastName);

    // Get file size and validate against chip flash size
    u32 file_size = FileSelLastNameSize;
    if (file_size > chip_flash_size) {
        Print("\26Warning: File size (%d bytes) exceeds chip flash (%d bytes)\27\n", file_size, chip_flash_size);
        Print("Truncating to chip flash size...\n");
        file_size = chip_flash_size;
        Print("Press any key to continue...\n");
        while (KeyGet() == NOKEY) {
        }
    }

    // Additional safety check: our buffer is 64kB max
    if (file_size > sizeof(buffer)) {
        Print("\26Warning: File too large for buffer (%d > %d bytes)327\n", file_size, (int)sizeof(buffer));
        Print("Truncating to buffer size...\n");
        file_size = sizeof(buffer);
        Print("Press any key to continue...\n");
        while (KeyGet() == NOKEY) {
        }
    }

    // Read binary file into buffer
    Print("Reading %d bytes from file...\n", file_size);
    FileRead(&FileSelPrevFile, buffer, file_size);
    FileClose(&FileSelPrevFile);

    Print("Programming flash (%d bytes)...\n", file_size);
    MinichlinkFlash(0x08000000, file_size, buffer);
    Print("\n\26Image written.\27\n\n");

    Print("Power cycling MCU...\n");
    McuPowerCycle(200);

    // Wait for user confirmation
    Print("Press any key to continue...\n");
    KeyFlush();
    while (KeyGet() == NOKEY) {
    }
}
