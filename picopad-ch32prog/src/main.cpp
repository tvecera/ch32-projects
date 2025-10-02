/**
 * @file main.cpp
 * @brief Main menu and application entry point for CH32 programmer
 * @details This file contains the main application logic for the CH32 programmer,
 *          handling user input, configuration, and interactions with CH32V00X/CH32V03X chips
 *          on the Picopad console.
 */

#include "../include.h"

#define CONFIG_FILE "CH32PROG.TXT"   ///< Configuration file name
#define MAX_LINE_LENGTH 16           ///< Maximum length of a configuration file line
#define MIN_POWER_MODE 1             ///< Minimum valid power mode value
#define MAX_POWER_MODE 3             ///< Maximum valid power mode value

/**
 * @brief Flag indicating if the configuration file is invalid
 */
Bool invalid_config = False;

/**
 * @brief Display the main menu with available operations
 * @details Prints the main menu to the console, showing supported operations
 *          and current configuration. Highlights invalid configuration if present.
 */
void PrintMenu() {
    Print("\f\26CH32 programmer for Picopad\27\3\n");
    Print("(Support only CH32V00X/CH32V03X chips)\n");
    if (invalid_config) { Print("\24Invalid config file !!!\27\n"); }
    Print("PIN_SWIO = %d, PIN_POWER = %d\n\n", current_pin_swio, current_pin_power);

    Print(" - A     - Chip info\n");
    Print(" - B     - Flash binary file\n");
    Print(" - X     - Full erase chip\n");
    Print(" - UP    - Power cycle MCU\n");
    Print(" - LEFT  - Power mode: \26%s\27\n", McuPowerGetModeName());
    Print(" - RIGHT - Pico Ardulink\n");
    Print(" - DOWN  - Enable/disable print debug\n");
    Print(" - Y     - Exit\n\n");
}

/**
 * @brief Check if a pin is in the allowed set
 * @param pin Pin number to validate
 * @return True if the pin is in the set {0, 1, 14, 26, 27, 28}, false otherwise
 */
Bool IsValidPin(const s32 pin) {
    const s32 validPins[] = {0, 1, 14, 26, 27, 28};
    const u8 numValidPins = sizeof(validPins) / sizeof(validPins[0]);
    for (u8 i = 0; i < numValidPins; i++) {
        if (pin == validPins[i]) {
            return True;
        }
    }
    return False;
}

/**
 * @brief Read a line from the configuration file and parse an integer value
 * @param file Pointer to the open file
 * @param buf Buffer to store the read line
 * @param valueIdx Index where the numeric value starts in the line
 * @param checkPin If true, validate against allowed pins; if false, validate against min/max
 * @param minValue Minimum allowed value (used if checkPin is false)
 * @param maxValue Maximum allowed value (used if checkPin is false)
 * @return Parsed integer value or -1 if invalid or parsing fails
 */
s32 ReadConfigLine(sFile* file, char* buf, const u8 valueIdx, const Bool checkPin, const s32 minValue,
                   const s32 maxValue) {
    u8 idx = 0;
    char ch;

    // Read until newline, carriage return, null, or buffer limit
    while (idx < MAX_LINE_LENGTH - 1) {
        if (FileRead(file, &ch, 1) != 1) {
            break; // EOF or error
        }
        if (ch == '\n' || ch == '\r' || ch == '\0') {
            break;
        }
        buf[idx++] = ch;
    }
    buf[idx] = '\0'; // Null-terminate the buffer

    // Skip empty lines or lines shorter than valueIdx
    if (idx <= valueIdx) {
        return -1;
    }

    const s32 value = StrToInt(&buf[valueIdx]);

    // Validate value based on pin or range check
    if (checkPin) {
        if (!IsValidPin(value)) {
            return -1;
        }
    } else {
        if (value < minValue || value > maxValue) {
            return -1;
        }
    }

    return value;
}

/**
 * @brief Load configuration from the SD card
 * @details Reads PIN_SWIO, PIN_POWER, and POWER_MODE from the configuration file.
 *          Sets invalid_config to true if any value is invalid or the file cannot be read.
 */
void ReadConfiguration() {
    // Load configuration from SD card if available
    if (SDConnect() && DiskMount() && DiskMounted()) {
        char buf[MAX_LINE_LENGTH];
        sFile file;
        SetDir("/");
        if (FileOpen(&file, CONFIG_FILE)) {
            // Read PIN_SWIO
            s32 value = ReadConfigLine(&file, buf, 9, True, 0, 0);
            if (value != -1) {
                current_pin_swio = value;
            } else {
                invalid_config = True;
            }

            // Read PIN_POWER
            value = ReadConfigLine(&file, buf, 10, True, 0, 0);
            if (value != -1) {
                // Ensure PIN_POWER is different from PIN_SWIO
                if (value != current_pin_swio) {
                    current_pin_power = value;
                } else {
                    invalid_config = True;
                }
            } else {
                invalid_config = True;
            }

            // Read POWER_MODE
            value = ReadConfigLine(&file, buf, 11, False, MIN_POWER_MODE, MAX_POWER_MODE);
            if (value != -1) {
                current_power_mode = value;
            } else {
                invalid_config = True;
            }

            FileClose(&file);
        } else {
            invalid_config = True;
        }
    } else {
        invalid_config = True;
    }

    if (invalid_config) {
        current_pin_swio = PIN_SWIO;
        current_pin_power = PIN_POWER;
        current_power_mode = POWER_MODE;
    }
}

/**
 * @brief Write formatted data to a file
 * @param file Pointer to the open file
 * @param fmt Format string for the output
 * @param ... Variable arguments for the format string
 * @return Number of bytes written to the file
 */
u32 WriteToFile(sFile* file, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const u32 result = FilePrintArg(file, fmt, args);
    va_end(args); // Moved before return to ensure cleanup
    return result;
}

/**
 * @brief Save configuration to the SD card
 * @details Writes PIN_SWIO, PIN_POWER, and POWER_MODE to the configuration file.
 *          Creates a new file if it does not exist, or overwrites the existing one.
 */
void WriteConfiguration() {
    // Save configuration to SD card if available
    if (SDConnect() && DiskMount() && DiskMounted()) {
        sFile file;
        SetDir("/");
        Bool fileExists = False;

        if (!FileExist(CONFIG_FILE)) {
            fileExists = FileCreate(&file, CONFIG_FILE);
        } else {
            fileExists = FileOpen(&file, CONFIG_FILE);
        }

        if (fileExists) {
            WriteToFile(&file, "PIN_SWIO %u\nPIN_POWER %u\nPOWER_MODE %u\n", current_pin_swio, current_pin_power,
                        current_power_mode);
            FileClose(&file);
        }
    }
}

/**
 * @brief Main application entry point
 * @details Initializes power control, loads configuration, and enters the main menu loop.
 *          Handles user input to perform operations on CH32V00X/CH32V03X chips.
 * @note Button mappings:
 *       - A: Display chip information
 *       - B: Flash binary file from SD card
 *       - X: Erase entire chip
 *       - UP: Power cycle target MCU
 *       - LEFT: Cycle power mode (3V3 Direct / GPIO Direct / PNP Switch)
 *       - RIGHT: Enter Ardulink protocol bridge mode
 *       - DOWN: Toggle SWIO debug output
 *       - Y: Exit to bootloader
 */
[[noreturn]] int main() {
    // Initialize default configuration
    current_pin_swio = PIN_SWIO;
    current_pin_power = PIN_POWER;
    current_power_mode = POWER_MODE_PNP_SWITCH;

    ReadConfiguration();

    McuPowerInit();
    PrintMenu();
    KeyFlush();

    while (True) {
        u8 key = KeyGet();
        switch (key) {
            case KEY_A:
                RunChipInfo();
                KeyFlush();
                PrintMenu();
                break;
            case KEY_B:
                RunFlash();
                KeyFlush();
                PrintMenu();
                break;
            case KEY_X:
                RunErase();
                KeyFlush();
                PrintMenu();
                break;
            case KEY_UP:
                McuPowerCycle(200);
                break;
            case KEY_LEFT: {
                const u8 next_mode = (McuPowerGetMode() == POWER_MODE_PNP_SWITCH)
                                         ? POWER_MODE_GPIO_DIRECT
                                         : (McuPowerGetMode() == POWER_MODE_GPIO_DIRECT)
                                         ? POWER_MODE_3V3_DIRECT
                                         : POWER_MODE_PNP_SWITCH;
                McuPowerSetMode(next_mode);
                WriteConfiguration();
                PrintMenu();
                break;
            }
            case KEY_RIGHT:
                RunArdulink();
                KeyFlush();
                PrintMenu();
                break;
            case KEY_DOWN:
                swio_debug = !swio_debug;
                Print("\vDebug %s\n", swio_debug ? "\26enabled\27" : "\26disabled\27");
                break;
            case KEY_Y:
                McuPowerOff();
                ResetToBootLoader();
                break;
            default:
                break;
        }
    }
}
