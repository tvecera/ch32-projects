/**
 * Simplified CH32 debug protocol implementation
 *
 * Based on minichlink from ch32v003fun by Charles Lohr (CNLohr)
 * Original: https://github.com/cnlohr/ch32v003fun/tree/master/minichlink
 * Author: Charles Lohr
 *
 * This simplified version supports only CH32V00x and CH32X03x MCU families
 */

#include "../include.h"

// Supported chip families
enum RiscVChip {
    CHIP_UNKNOWN = 0x00,
    CHIP_CH32V00x = 0x4e, // CH32V002/004/005/006/007
    CHIP_CH32X03x = 0x0d, // CH32X033/035
};

// Programming protocol (only PROTOCOL_DEFAULT is supported)
enum ProgProtocol { PROTOCOL_DEFAULT = 0x01 };

enum MemoryArea { DEFAULT_AREA = 0, PROGRAM_AREA = 1 };

struct RiscVChip_s {
    char name_str[10]; // Name of the chip
    enum RiscVChip family_id; // ChipID[3]
    uint16_t model_id; // ChipID[4-5] & 0xFFF0
    uint32_t ram_base; // RAM base address (0x20000000 for CH32V00x/CH32X03x)
    uint32_t ram_size; // RAM size in bytes
    uint32_t sector_size; // Flash sector size for erasing and writing
    uint32_t flash_size; // Flash size in bytes
    uint32_t options_offset; // Offset of the Option Bytes partition
    uint8_t no_autoexec; // 1 if chip lacks autoexec functionality in Debug Module
    uint16_t isp_id_mask; // ID mask for ISP bootloader detection
};

// Supported chips - CH32V00x family, CH32X03x family
// CH32V00x family - compact 8-pin RISC-V microcontrollers
const struct RiscVChip_s ch32v002 = {
    .name_str = "CH32V002",
    .family_id = CHIP_CH32V00x,
    .model_id = 0x0020,
    .ram_base = 0x20000000,
    .ram_size = 4096,
    .sector_size = 256,
    .flash_size = 16 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0,
};

const struct RiscVChip_s ch32v004 = {
    .name_str = "CH32V004",
    .family_id = CHIP_CH32V00x,
    .model_id = 0x0040,
    .ram_base = 0x20000000,
    .ram_size = 6 * 1024,
    .sector_size = 256,
    .flash_size = 32 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0,
};

const struct RiscVChip_s ch32v005 = {
    .name_str = "CH32V005",
    .family_id = CHIP_CH32V00x,
    .model_id = 0x0050,
    .ram_base = 0x20000000,
    .ram_size = 6 * 1024,
    .sector_size = 256,
    .flash_size = 32 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0,
};

const struct RiscVChip_s ch32v006 = {
    .name_str = "CH32V006",
    .family_id = CHIP_CH32V00x,
    .model_id = 0x0060,
    .ram_base = 0x20000000,
    .ram_size = 8 * 1024,
    .sector_size = 256,
    .flash_size = 62 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0,
};

const struct RiscVChip_s ch32v007 = {
    .name_str = "CH32V007",
    .family_id = CHIP_CH32V00x,
    .model_id = 0x0070,
    .ram_base = 0x20000000,
    .ram_size = 8 * 1024,
    .sector_size = 256,
    .flash_size = 62 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0,
};

// CH32X03x family - extended peripherals with USB support
const struct RiscVChip_s ch32x033 = {
    .name_str = "CH32x033",
    .family_id = CHIP_CH32X03x,
    .model_id = 0x0330,
    .ram_base = 0x20000000,
    .ram_size = 20 * 1024,
    .sector_size = 256,
    .flash_size = 62 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0x235a,
};

const struct RiscVChip_s ch32x035 = {
    .name_str = "CH32x035",
    .family_id = CHIP_CH32X03x,
    .model_id = 0x0350,
    .ram_base = 0x20000000,
    .ram_size = 20 * 1024,
    .sector_size = 256,
    .flash_size = 62 * 1024,
    .options_offset = 0x1FFFF800,
    .no_autoexec = 0,
    .isp_id_mask = 0x235f,
};

// Flash status register bits
#define FLASH_STATR_WRPRTERR  ((u32)0x00000010) /* Write Protection Error */

// Flash control register bits (FLASH_CTLR at 0x40022010)
#define FLASH_CTLR_MER        ((u32)0x00000004) /* Mass Erase */
#define FLASH_CTLR_OPTPG      ((u32)0x00000010) /* Option Byte Programming */
#define FLASH_CTLR_OPTER      ((u32)0x00000020) /* Option Byte Erase */
#define FLASH_CTLR_STRT       ((u32)0x00000040) /* Start */
#define FLASH_CTLR_LOCK       ((u32)0x00000080) /* Lock */
#define FLASH_CTLR_OPTWRE     ((u32)0x00000200) /* Option Bytes Write Enable */
#define FLASH_CTLR_PAGE_PG    ((u32)0x00010000) /* Page Programming 64Byte */
#define FLASH_CTLR_PAGE_ER    ((u32)0x00020000) /* Page Erase 64Byte */
#define FLASH_CTLR_BUF_LOAD   ((u32)0x00040000) /* Buffer Load */
#define FLASH_CTLR_BUF_RST    ((u32)0x00080000) /* Buffer Reset */

#define FLASH_CTLR            ((u32)0x40022010)
#define FLASH_STATR           ((u32)0x4002200c)
#define FLASH_KEYR            ((u32)0x40022004)
#define FLASH_OBTKEYR         ((u32)0x4002201c)

#define OBKEYR                ((u32)0x40022008)
#define MODEKEYR              ((u32)0x40022024)

#define HALT_MODE_HALT_AND_RESET    0
#define HALT_MODE_HALT_BUT_NO_RESET 5

// Convert a 4-character string to an int.
#define STTAG( x ) (*((uint32_t*)(x)))

#define MAX_FLASH_SECTORS 262144
#define STR_(x) #x
#define STR(x) STR_(x)

// State tracking for optimized read/write sequences
u32 statetag; // Current operation state tag (RDSQ, WRSQ, etc.)
u32 currentstateval; // Current address for sequential operations
u32 flash_unlocked; // Flash unlock status
int lastwriteflags; // Last write operation flags (flash vs RAM)

// Chip information
u32 ram_base;
u32 ram_size;
u32 flash_size;
uint sector_size;
const struct RiscVChip_s* target_chip;
enum RiscVChip target_chip_type;
u32 target_chip_id;
int nr_registers_for_debug; // 16 for CH32V00x/CH32X03x
enum MemoryArea current_area = PROGRAM_AREA;

static int WaitForDoneOp(u8 ignore);
static int GetUUID(u8* buffer);
static int Halt();
static void StaticUpdatePROGBUFRegs();
static int ReadWord(u32 address_to_read, u32* data);
static int ReadHalfWord(u32 address_to_read, u16* data);
static int ReadByte(u32 address_to_read, u8* data);
static int WriteWord(u32 address_to_write, u32 data);
static int WriteHalfWord(u32 address_to_write, u16 data);
static int WriteByte(u32 address_to_write, u8 data);
static int InternalUnlockFlash();
static int WaitForFlash();
static int DetermineChipType();
static int ReadBinaryBlob(u32 address_to_read_from, u32 read_size, u8* blob);
static int WriteBinaryBlob(u64 address_to_write, u32 blob_size, const u8* blob);

INLINE u8 IsAddressFlash(const u32 addy) { return !(addy & 0xe0000000); }
INLINE u8 VoidHighLevelState() {
    statetag = STTAG("VOID");
    return 0;
}

static int WaitForDoneOp(const u8 ignore) {
    u32 rrv;
    int timeout = 100;

    do {
        const int r = ReadReg32(DMABSTRACTCS, &rrv);
        if (r) return r;
    }
    while ((rrv & (1 << 12)) && timeout--);

    if (((rrv >> 8) & 7) || (rrv & (1 << 12))) {
        if (!ignore) {
            const char* errortext = 0;
            switch ((rrv >> 8) & 7) {
                case 1:
                    errortext = "Command in execution";
                    break;
                case 2:
                    errortext = "Abstract Command Unsupported";
                    break;
                case 3:
                    errortext = "Exception executing Abstract Command";
                    break;
                case 4:
                    errortext = "Processor not halted.";
                    break;
                case 5:
                    errortext = "Bus Error";
                    break;
                case 6:
                    errortext = "Parity Bit";
                    break;
                default:
                    errortext = "Other Error";
                    break;
            }

            u32 temp;
            ReadReg32(DMSTATUS, &temp);
            Print("\24Fault on op (DMABSTRACTS = %08x) (%d) (%s) DMSTATUS: %08x\27\n", rrv, timeout, errortext, temp);
        }
        WriteReg32(DMABSTRACTCS, 0x00000700);
        return -9;
    }
    return 0;
}

/**
 * @brief Read 8-byte unique ID from target MCU
 * @param buffer Pointer to 8-byte buffer for UUID storage
 * @return Always returns 0
 * @note UUID is stored at 0x1FFFF7E8 on CH32V00x and CH32X03x
 */
static int GetUUID(u8* buffer) {
    u8 local_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    WriteReg32(DMABSTRACTAUTO, 0);
    WriteReg32(DMPROGBUF0, 0x90024000); // c.ebreak <<== c.lw x8, 0(x8)
    // CH32V00x and CH32X03x UUID location
    WriteReg32(DMDATA0, 0x1ffff7e8);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(0);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, (u32 *)local_buffer);
    WriteReg32(DMDATA0, 0x1ffff7ec);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(0);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, (u32 *)(local_buffer + 4));
    *((u32*)buffer) = local_buffer[0] << 24 | local_buffer[1] << 16 | local_buffer[2] << 8 | local_buffer[3];
    *(((u32*)buffer) + 1) = local_buffer[4] << 24 | local_buffer[5] << 16 | local_buffer[6] << 8 | local_buffer[7];

    return 0;
}

/**
 * @brief Halt target MCU execution
 * @return Always returns 0
 * @note Waits 10ms for halt to complete (important if MCU is in middle of flash operation)
 */
static int Halt() {
    WriteReg32(DMSHDWCFGR, 0x5aa50000 | (1<<10)); // Shadow Config Reg
    WriteReg32(DMCFGR, 0x5aa50000 | (1<<10)); // CFGR (1<<10 == Allow output from slave)
    WriteReg32(DMCONTROL, 0x80000001); // Make the debug module work properly.
    WriteReg32(DMCONTROL, 0x80000001); // Re-initiate a halt request.
    WriteReg32(DMCONTROL, 0x80000001); // Re-initiate a halt request.
    // WriteReg32(DMCONTROL, 0x00000001 ); // Clear Halt Request.  This is recommended, but not doing it seems more stable.
    // Sometimes, even if the processor is halted but the MSB is clear, it will spuriously start?
    flash_unlocked = 0;
    // In case processor halt process needs to complete, i.e. if it was in the middle of a flash op.
    // DelayUS(3000 );
    WaitUs(10000);

    return 0;
}

/**
 * @brief Initialize debug program buffer registers with static values
 * @note Sets up x10-x13 with commonly used addresses for optimized read/write operations
 * @note x10 = DATA0 address, x11 = DATA1 address, x12 = FLASH_STATR, x13 = flash control flags
 */
static void StaticUpdatePROGBUFRegs() {
    u32 rr;
    if (ReadReg32(DMHARTINFO, &rr)) {
        Print("\24Error: Could not get hart info.\27\n");
        return;
    }

    const u32 data0offset = 0xe0000000 | (rr & 0x7ff);

    // Putting DATA0's location into x10, and DATA1's location into x11 is universal for all continued code.
    WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.
    WriteReg32(DMDATA0, data0offset); // DATA0's location in memory.
    WriteReg32(DMCOMMAND, 0x0023100a); // Copy data to x10
    WriteReg32(DMDATA0, data0offset + 4); // DATA1's location in memory.
    WriteReg32(DMCOMMAND, 0x0023100b); // Copy data to x11
    WriteReg32(DMDATA0, FLASH_STATR); // FLASH->STATR, add 4 to get FLASH->CTLR
    WriteReg32(DMCOMMAND, 0x0023100c); // Copy data to x12

    // v003 requires bufload every word.
    // x035 requires bufload every word in spite of what the datasheet says.
    // CR_PAGE_PG = FTPG = 0x00010000 | CR_BUF_LOAD = 0x00040000
    // We just don't do the write on the v20x/v30x.
    WriteReg32(DMDATA0, 0x00010000|0x00040000);
    WriteReg32(DMCOMMAND, 0x0023100d); // Copy data to x13
}

/**
 * @brief Read 32-bit word from target MCU memory
 * @param address_to_read Memory address to read from
 * @param data Pointer to store read value
 * @return 0 on success, negative error code on failure
 */
static int ReadWord(const u32 address_to_read, u32* data) {
    int r = 0;

    int autoincrement = 1;
    // Don't autoincrement when checking flash flag.
    if (address_to_read == FLASH_CTLR || address_to_read == FLASH_STATR) { autoincrement = 0; }

    if (statetag != STTAG("RDSQ") || address_to_read != currentstateval) {
        //|| autoincrement != autoincrement
        if (statetag != STTAG("RDSQ")) {
            if (statetag != STTAG("WRSQ")) { StaticUpdatePROGBUFRegs(); }
            WriteReg32(DMABSTRACTAUTO, 0); // Disable Autoexec.
            // c.lw x8,0(x11) // Pull the address from DATA1
            // c.lw x9,0(x8)  // Read the data at that location.
            WriteReg32(DMPROGBUF0, 0x40044180);
            if (autoincrement) {
                // c.addi x8, 4
                // c.sw x9, 0(x10) // Write back to DATA0
                WriteReg32(DMPROGBUF1, 0xc1040411);
            } else {
                // c.nop
                // c.sw x9, 0(x10) // Write back to DATA0
                WriteReg32(DMPROGBUF1, 0xc1040001);
            }
            // c.sw x8, 0(x11) // Write addy to DATA1
            // c.ebreak
            WriteReg32(DMPROGBUF2, 0x9002c180);
            if (!target_chip->no_autoexec) {
                WriteReg32(DMABSTRACTAUTO, 1); // Enable Autoexec (different kind of autoinc than outer autoinc)
            }
        }

        WriteReg32(DMDATA1, address_to_read);
        if (!target_chip->no_autoexec) { WriteReg32(DMCOMMAND, 0x00240000); }

        statetag = STTAG("RDSQ");
        currentstateval = address_to_read;
        WaitForDoneOp(1);
    }

    if (autoincrement) { currentstateval += 4; }

    if (target_chip->no_autoexec) { WriteReg32(DMCOMMAND, 0x00240000); }
    r |= ReadReg32(DMDATA0, data);

    if (currentstateval == ram_base + ram_size) {
        WaitForDoneOp(1); // Ignore any post-errors.
    }
    return r;
}

/**
 * @brief Read 16-bit halfword from target MCU memory
 * @param address_to_read Memory address to read from
 * @param data Pointer to store read value
 * @return 0 on success, negative error code on failure
 */
static int ReadHalfWord(const u32 address_to_read, u16* data) {
    int ret = 0;
    VoidHighLevelState();
    statetag = STTAG("XXXX");

    WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.

    // lh x8,0(x9)  // Read halfword from the address.
    WriteReg32(DMPROGBUF0, 0x00049403);
    WriteReg32(DMPROGBUF1, 0x00100073); // c.ebreak

    WriteReg32(DMDATA0, address_to_read);
    WriteReg32(DMCOMMAND, 0x00231009); // Copy data to x9
    WriteReg32(DMCOMMAND, 0x00241000); // Only execute.
    WriteReg32(DMCOMMAND, 0x00221008); // Read x8 into DATA0.

    ret |= WaitForDoneOp(0);
    currentstateval = -1;

    if (ret) Print("\24Fault on ReadHalfWord\27\n");

    u32 rr;
    ret |= ReadReg32(DMDATA0, &rr);
    *data = rr;
    return ret;
}

/**
 * @brief Read 8-bit byte from target MCU memory
 * @param address_to_read Memory address to read from
 * @param data Pointer to store read value
 * @return 0 on success, negative error code on failure
 */
static int ReadByte(const u32 address_to_read, u8* data) {
    int ret = 0;
    VoidHighLevelState();
    statetag = STTAG("XXXX");

    WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.

    // lb x8,0(x9)  // Read byte from the address.
    WriteReg32(DMPROGBUF0, 0x00048403);
    WriteReg32(DMPROGBUF1, 0x00100073); // c.ebreak

    WriteReg32(DMDATA0, address_to_read);
    WriteReg32(DMCOMMAND, 0x00231009); // Copy data to x9
    WriteReg32(DMCOMMAND, 0x00241000); // Only execute.
    WriteReg32(DMCOMMAND, 0x00221008); // Read x8 into DATA0.

    ret |= WaitForDoneOp(0);
    if (ret) Print("\24Fault on ReadByte\27\n");
    currentstateval = -1;

    u32 rr;
    ret |= ReadReg32(DMDATA0, &rr);
    *data = rr;
    return ret;
}

/**
 * @brief Write 32-bit word to target MCU memory
 * @param address_to_write Memory address to write to
 * @param data Value to write
 * @return 0 on success, negative error code on failure
 */
static int WriteWord(const u32 address_to_write, const u32 data) {
    int ret = 0;

    const u8 is_flash = IsAddressFlash(address_to_write);

    if (statetag != STTAG("WRSQ") || is_flash != lastwriteflags) {
        int did_disable_req = 0;
        if (statetag != STTAG("WRSQ")) {
            WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.
            did_disable_req = 1;
            if (statetag != STTAG("RDSQ")) { StaticUpdatePROGBUFRegs(); }

            // Different address, so we don't need to re-write all the program regs.
            // c.lw x8,0(x10) // Get the value to write.
            // c.lw x9,0(x11) // Get the address to write to.
            WriteReg32(DMPROGBUF0, 0x41844100);
            // c.sw x8,0(x9)  // Write to the address.
            // c.addi x9, 4
            WriteReg32(DMPROGBUF1, 0x0491c080);
        }
        if (is_flash) {
            // A little weird - we need to wait until the buf load is done here to continue.
            // x12 = 0x4002200C (FLASH_STATR)
            //
            // c254 c.sw x13,4(x12) // Acknowledge the page write.  (BUT ONLY ON x035 / v003)
            // /otherwise c.nop
            // 4200 c.lw x8,0(x12)  // Start checking to see when buf load is done.
            // 8809 c.andi x8, 2    // Only look at WR_BSY (seems to be rather undocumented)
            // /8805 c.andi x8, 1    // Only look at BSY if we're not on a v30x / v20x
            // fc75 c.bnez x8, -4
            // c.ebreak
            WriteReg32(DMPROGBUF2, 0x0001c184);
            // CH32V00x and CH32X03x use 0x4200c254
            WriteReg32(DMPROGBUF3, 0x4200c254);
            // CH32V00x and CH32X03x use 0xfc758805
            WriteReg32(DMPROGBUF4, 0xfc758805);
            WriteReg32(DMPROGBUF5, 0x90029002);
        } else {
            // c.sw x9,0(x11)
            // c.ebreak
            WriteReg32(DMPROGBUF2, 0x9002c184);
            // WriteReg32(DMPROGBUF3, 0x90029002 ); // c.ebreak (nothing needs to be done if not flash)
        }

        WriteReg32(DMDATA1, address_to_write);
        WriteReg32(DMDATA0, data);

        if (target_chip->no_autoexec) {
            ret |= WriteReg32(DMCOMMAND, 0x00240000); // Execute.
        } else if (did_disable_req) {
            WriteReg32(DMCOMMAND, 0x00240000); // Execute.
            WriteReg32(DMABSTRACTAUTO, 1); // Enable Autoexec.
        }

        lastwriteflags = is_flash;
        statetag = STTAG("WRSQ");
        currentstateval = address_to_write;
    } else {
        if (address_to_write != currentstateval) { WriteReg32(DMDATA1, address_to_write); }

        WriteReg32(DMDATA0, data);
        if (target_chip->no_autoexec) {
            ret |= WriteReg32(DMCOMMAND, 0x00240000); // Execute.
        }
    }

    if (is_flash) { ret |= WaitForDoneOp(0); }

    currentstateval += 4;
    return ret;
}

/**
 * @brief Write 16-bit halfword to target MCU memory
 * @param address_to_write Memory address to write to
 * @param data Value to write
 * @return 0 on success, negative error code on failure
 */
static int WriteHalfWord(const u32 address_to_write, const u16 data) {
    int ret = 0;
    VoidHighLevelState();
    statetag = STTAG("XXXX");

    WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.

    // sh x8,0(x9)  // Write to the address.
    WriteReg32(DMPROGBUF0, 0x00849023);
    WriteReg32(DMPROGBUF1, 0x00100073); // c.ebreak

    WriteReg32(DMDATA0, address_to_write);
    WriteReg32(DMCOMMAND, 0x00231009); // Copy data to x9
    WriteReg32(DMDATA0, data);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute program.

    ret |= WaitForDoneOp(0);
    currentstateval = -1;

    if (ret) Print("\24Fault on WriteHalfWord\27\n");
    return ret;
}

/**
 * @brief Write 8-bit byte to target MCU memory
 * @param address_to_write Memory address to write to
 * @param data Value to write
 * @return 0 on success, negative error code on failure
 */
static int WriteByte(u32 address_to_write, u8 data) {
    int ret = 0;
    VoidHighLevelState();
    statetag = STTAG("XXXX");

    WriteReg32(DMABSTRACTAUTO, 0x00000000); // Disable Autoexec.

    // sb x8,0(x9)  // Write byte to the address.
    WriteReg32(DMPROGBUF0, 0x00848023);
    WriteReg32(DMPROGBUF1, 0x00100073); // c.ebreak

    WriteReg32(DMDATA0, address_to_write);
    WriteReg32(DMCOMMAND, 0x00231009); // Copy data to x9
    WriteReg32(DMDATA0, data);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute program.

    ret |= WaitForDoneOp(0);
    if (ret) Print("\24Fault on WriteByte\27\n");
    currentstateval = -1;
    return ret;
}

/**
 * @brief Unlock flash memory for programming
 * @return 0 on success, negative error code on failure
 * @note Writes unlock sequences to FLASH_KEYR, OBKEYR, and MODEKEYR registers
 * @note Checks for read protection and warns user if enabled
 */
static int InternalUnlockFlash() {
    int ret = 0;
    u32 rw;

    ReadWord(FLASH_CTLR, &rw); // FLASH->CTLR = 0x40022010
    if (rw & 0x8080) {
        ret = WriteWord(FLASH_KEYR, 0x45670123); // FLASH->KEYR = 0x40022004
        if (ret) goto reterr;
        ret = WriteWord(FLASH_KEYR, 0xCDEF89AB);
        if (ret) goto reterr;
        ret = WriteWord(OBKEYR, 0x45670123); // OBKEYR = 0x40022008  // For user word unlocking
        if (ret) goto reterr;
        ret = WriteWord(OBKEYR, 0xCDEF89AB);
        if (ret) goto reterr;
        ret = WriteWord(MODEKEYR, 0x45670123); // MODEKEYR = 0x40022024
        if (ret) goto reterr;
        ret = WriteWord(MODEKEYR, 0xCDEF89AB);
        if (ret) goto reterr;

        ret = ReadWord(FLASH_CTLR, &rw); // FLASH->CTLR = 0x40022010
        if (ret) goto reterr;

        if (rw & 0x8080) {
            Print("\24Error: Flash is not unlocked (CTLR = %08x)\27\n", rw);
            return -9;
        }
    }

    ReadWord(FLASH_OBTKEYR, &rw); //(FLASH_OBTKEYR)
    if (rw & 2) {
        Print("\26WARNING: Your part appears to have flash [read] locked.  Cannot program unless unlocked.\27\n");
    }
    flash_unlocked = 1;

    return 0;
reterr:
    Print("\24Error unlocking flash, got code %d from underlying system\27\n", ret);
    return ret;
}

/**
 * @brief Wait for flash operation to complete
 * @return 0 on success, -1 on timeout, -44 on write protection error
 * @note Polls FLASH_STATR register BSY and WRBSY flags with 5000-iteration timeout
 */
static int WaitForFlash() {
    u32 rw, timeout = 0;
    do {
        rw = 0;
        ReadWord(FLASH_STATR, &rw); // FLASH_STATR => 0x4002200C
        if (timeout++ > 5000) {
            Print("\24Warning: Flash timed out. STATR = %08x\27\n", rw);
            return -1;
        }
    }
    while (rw & 3); // BSY flag for 003, or WRBSY for other processors.

    if (rw & FLASH_STATR_WRPRTERR) {
        Print("\24Memory Protection Error\27\n");
        return -44;
    }

    return 0;
}

/**
 * @brief Detect and identify connected CH32 chip
 * @return 0 on success, -1 on communication error, -2 on unsupported chip
 * @note Reads chip ID, flash size, UUID, and read protection status
 * @note Supports only CH32V00x and CH32X03x families
 * @note Prints chip information to console
 */
static int DetermineChipType() {
    u32 rr;
    u32 old_data0;
    u32 old_x8;
    u32 marchid = 0;

    if (ReadReg32(DMHARTINFO, &rr)) {
        Print("\24Error: Could not get hart info.\27\n");
        return -1;
    }

    WriteReg32(DMCONTROL, 0x80000001); // Make the debug module work properly.
    WriteReg32(DMCONTROL, 0x80000001); // Initiate halt request.

    // Tricky, this function needs to clean everything up because it may be used entering debugger.
    ReadReg32(DMDATA0, &old_data0);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, &old_x8);

    WriteReg32(DMABSTRACTCS, 0x08000700); // Clear out any dmabstractcs errors.

    WriteReg32(DMABSTRACTAUTO, 0x00000000);
    WriteReg32(DMCOMMAND, 0x00220000 | 0xf12);
    WriteReg32(DMCOMMAND, 0x00220000 | 0xf12); // Need to double-read, not sure why.
    ReadReg32(DMDATA0, &marchid);
    WriteReg32(DMPROGBUF0, 0x90024000); // c.ebreak <<== c.lw x8, 0(x8)

    u32 chip_id = 0;
    u32 vendor_bytes = 0;
    u32 sevenf_id = 0;
    u8 read_protection = 0;
    ReadReg32(0x7f, &sevenf_id);

    // Only support CH32V00x and CH32X03x families
    u32 chip_id_address = 0x1ffff7c4;
    const u32 flash_size_address = 0x1ffff7e0;
    const u32 masked_id = sevenf_id & 0xfff00000;
    const u32 masked_id2 = sevenf_id & 0xfff00f00;

    // CH32V00x family detection (CH32V002, V004, V005, V006, V007) and CH32X03x family detection (CH32X033, CH32X035)
    if (masked_id == 0x00200000) {
        target_chip = &ch32v002;
        chip_id_address = 0x1ffff704;
    } else if (masked_id == 0x00400000) {
        target_chip = &ch32v004;
        chip_id_address = 0x1ffff704;
    } else if (masked_id == 0x00500000) {
        target_chip = &ch32v005;
        chip_id_address = 0x1ffff704;
    } else if (masked_id == 0x00600000) {
        target_chip = &ch32v006;
        chip_id_address = 0x1ffff704;
    } else if (masked_id == 0x00700000) {
        target_chip = &ch32v007;
        chip_id_address = 0x1ffff704;
    } else if (masked_id2 == 0x3300600) {
        target_chip = &ch32x033;
        target_chip_id = sevenf_id;
    } else if (masked_id2 == 0x3500600) {
        target_chip = &ch32x035;
        target_chip_id = sevenf_id;
    }

    if (target_chip == NULL) {
        Print("\24Unknown chip type. Supported only CH32V00x, CH32X033, CH32X035.\27\n");
        Print("marchid : %08x\n", marchid);
        Print("HARTINFO: %08x\n", rr);
        return -2;
    }

    // Read chip_id if not already set
    WriteReg32(DMDATA0, chip_id_address);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(1);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, &chip_id);
    target_chip_id = chip_id;

    // Read flash size
    WriteReg32(DMDATA0, flash_size_address);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(1);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, &vendor_bytes);

    flash_size = vendor_bytes & 0xFFFF;

    uint32_t one;
    int two;
    WriteReg32(DMDATA0, 0x4002201c);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(1);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, &one);
    WriteReg32(DMDATA0, 0x40022020);
    WriteReg32(DMCOMMAND, 0x00271008); // Copy data to x8, and execute.
    WaitForDoneOp(1);
    WriteReg32(DMCOMMAND, 0x00221008); // Copy data from x8.
    ReadReg32(DMDATA0, (uint32_t*)&two);

    if ((one & 2) || two != -1) read_protection = 1;

    // All supported chips (CH32V00x, CH32X03x) use PROTOCOL_DEFAULT
    target_chip_type = target_chip->family_id;
    if (flash_size == 0) { flash_size = target_chip->flash_size / 1024; }
    ram_base = target_chip->ram_base;
    ram_size = target_chip->ram_size;
    sector_size = target_chip->sector_size;
    // CH32V00x and CH32X03x both use 16 registers
    nr_registers_for_debug = 16;

    const u8* part_type = (u8*)&target_chip_id;
    u8 uuid[8];
    GetUUID(uuid);
    Print("Detected: \26%s\27\n", target_chip->name_str);
    Print("Flash Storage: \26%d kB\27\n", flash_size);
    Print("Part UUID: \26%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\27\n", uuid[0], uuid[1], uuid[2], uuid[3], uuid[4],
          uuid[5], uuid[6], uuid[7]);
    Print("Part Type: \26%02x-%02x-%02x-%02x\27\n", part_type[3], part_type[2], part_type[1], part_type[0]);
    Print("Read protection: \26%s\27\n", (read_protection > 0) ? "enabled" : "disabled");

    // Cleanup
    WriteReg32(DMDATA0, old_x8);
    WriteReg32(DMCOMMAND, 0x00231008); // Copy data to x8
    WriteReg32(DMDATA0, old_data0);

    statetag = STTAG("XXXX");

    return 0;
}

/**
 * @brief Read binary data block from target MCU memory
 * @param address_to_read_from Start address to read from
 * @param read_size Number of bytes to read
 * @param blob Pointer to buffer for storing read data
 * @return 0 on success, negative error code on failure
 * @note Automatically selects optimal read width (word/halfword/byte) based on alignment
 */
static int ReadBinaryBlob(u32 address_to_read_from, u32 read_size, u8* blob) {
    u32 rpos = address_to_read_from;
    const u32 rend = address_to_read_from + read_size;

    while (rpos < rend) {
        int r;
        int remain = rend - rpos;
        if ((rpos & 3) == 0 && remain >= 4) {
            u32 rw;
            r = ReadWord(rpos, &rw);
            if (r) return r;
            int rem = remain;
            if (rem > 4) rem = 4;
            memcpy(blob, &rw, rem);
            blob += 4;
            rpos += 4;
        } else {
            if ((rpos & 1)) {
                u8 rw;
                r = ReadByte(rpos, &rw);
                if (r) return r;
                memcpy(blob, &rw, 1);
                blob += 1;
                rpos += 1;
                remain -= 1;
            }
            if ((rpos & 2) && remain >= 2) {
                u16 rw;
                r = ReadHalfWord(rpos, &rw);
                if (r) return r;
                memcpy(blob, &rw, 2);
                blob += 2;
                rpos += 2;
                remain -= 2;
            }
            if (remain >= 1) {
                u8 rw;
                r = ReadByte(rpos, &rw);
                if (r) return r;
                memcpy(blob, &rw, 1);
                blob += 1;
                rpos += 1;
                remain -= 1;
            }
        }
    }

    const int r = WaitForDoneOp(0);
    if (r) Print("\24Fault on DefaultReadBinaryBlob\27\n");
    return r;
}

/**
 * @brief Write binary data block to target MCU memory
 * @param address_to_write Start address to write to
 * @param blob_size Number of bytes to write
 * @param blob Pointer to data to write
 * @return 0 on success, negative error code on failure
 */
static int WriteBinaryBlob(u64 address_to_write, const u32 blob_size, const u8* blob) {
    Print("Writing %d bytes to 0x%08x\n", blob_size, address_to_write);
    // We can't write into flash when mapped to 0x00000000
    if (address_to_write < 0x01000000) { address_to_write |= 0x08000000; }

    const u8 is_flash = IsAddressFlash(address_to_write);

    if (blob_size == 0) return 0;

    if (is_flash && !flash_unlocked) {
        int rw;
        if ((rw = InternalUnlockFlash())) return rw;
    }

    WaitForFlash();
    const int sectorsize = sector_size;

    u8 tempblock[sectorsize];
    const int sblock = address_to_write / sectorsize;
    const int eblock = (address_to_write + blob_size + (sectorsize - 1)) / sectorsize;
    int rsofar = 0;

    for (int b = sblock; b < eblock; b++) {
        int offset_in_block = address_to_write - (b * sectorsize);
        if (offset_in_block < 0) offset_in_block = 0;
        int end_o_plus_one_in_block = (address_to_write + blob_size) - (b * sectorsize);
        if (end_o_plus_one_in_block > sectorsize) end_o_plus_one_in_block = sectorsize;
        const int base = b * sectorsize;

        if (offset_in_block == 0 && end_o_plus_one_in_block == sectorsize) {
            if (is_flash) {
                MinichlinkErase(base, sectorsize, 0);
                // CH32V00x, CH32X03x use buffer reset
                WriteWord(FLASH_CTLR, FLASH_CTLR_PAGE_PG);
                // THIS IS REQUIRED, (intptr_t)&FLASH->CTLR = 0x40022010
                WriteWord(FLASH_CTLR, FLASH_CTLR_BUF_RST | FLASH_CTLR_PAGE_PG); // (intptr_t)&FLASH->CTLR = 0x40022010
            }

            for (int j = 0; j < sectorsize / 4; j++) {
                u32 writeword;
                memcpy(&writeword, blob + rsofar, 4);
                // WriteWord automatically handles flash buffer loading for CH32V00x and CH32X03x
                WriteWord(j * 4 + base, writeword);
                // On the v2xx, v3xx, you also need to make sure FLASH->STATR & 2 is not set.  This is only an issue when running locally.

                rsofar += 4;
            }

            if (is_flash) {
                // CH32V00x, CH32X03x
                WriteWord(0x40022014, base); //0x40022014 -> FLASH->ADDR
                // Give the programmer a headsup this next operation could take a while.
                WriteWord(FLASH_CTLR, FLASH_CTLR_PAGE_PG | FLASH_CTLR_STRT); // 0x40022010 -> FLASH->CTLR
            }
        } else {
            //Ok, we have to do something wacky.
            if (is_flash) {
                ReadBinaryBlob(base, sectorsize, tempblock);

                // Permute tempblock
                const int tocopy = end_o_plus_one_in_block - offset_in_block;
                memcpy(tempblock + offset_in_block, blob + rsofar, tocopy);
                rsofar += tocopy;

                MinichlinkErase(base, sectorsize, 0);
                // CH32V00x, CH32X03x use buffer reset
                WriteWord(FLASH_CTLR, FLASH_CTLR_PAGE_PG);
                // THIS IS REQUIRED, (intptr_t)&FLASH->CTLR = 0x40022010
                WriteWord(FLASH_CTLR, FLASH_CTLR_BUF_RST | FLASH_CTLR_PAGE_PG); // (intptr_t)&FLASH->CTLR = 0x40022010

                for (int j = 0; j < sectorsize / 4; j++) {
                    // WARNING: Just so you know, this is ACTUALLY doing the write AND if writing to flash, doing the following:
                    // FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD AFTER it does the write.  THIS IS REQUIRED on the 003
                    WriteWord(j * 4 + base, *(u32*)(tempblock + j * 4));
                }

                // CH32V00x, CH32X03x
                WriteWord(0x40022014, base); //0x40022014 -> FLASH->ADDR
                WriteWord(FLASH_CTLR, FLASH_CTLR_PAGE_PG | FLASH_CTLR_STRT); // 0x40022010 -> FLASH->CTLR
            } else {
                // Accessing RAM.  Be careful to only do the needed operations.
                for (int j = 0; j < sectorsize; j++) {
                    u32 taddy = j * 4;
                    if (offset_in_block <= taddy && end_o_plus_one_in_block >= taddy + 4) {
                        WriteWord(taddy + base, *(u32*)(blob + rsofar));
                        rsofar += 4;
                    } else if ((offset_in_block & 1) || (end_o_plus_one_in_block & 1)) {
                        // Bytes only.
                        for (int k = 0; k < 4; k++) {
                            if (taddy >= offset_in_block && taddy < end_o_plus_one_in_block) {
                                WriteByte(taddy + base, *(u32*)(blob + rsofar));
                                rsofar++;
                            }
                            taddy++;
                        }
                    } else {
                        // Half-words
                        for (int l = 0; l < 4; l += 2) {
                            if (taddy >= offset_in_block && taddy < end_o_plus_one_in_block) {
                                WriteHalfWord(taddy + base, *(u32*)(blob + rsofar));
                                rsofar += 2;
                            }
                            taddy += 2;
                        }
                    }
                }
            }
        }
    }

    WaitUs(100); // Why do we need this? (We seem to need this on the WCH programmers?)
    return 0;
}

/**
 * @brief Initialize SWIO interface and halt target MCU
 * @return 0 on success, negative error code on failure
 * @note Powers on target MCU, configures debug module, and verifies connection
 * @note Waits 16ms after power-on for MCU stabilization
 */
int MinichlinkSetupInterface() {
    McuPowerOn();
    WaitUs(16000);
    WriteReg32(DMSHDWCFGR, 0x5aa50000 | (1<<10)); // Shadow Config Reg
    WriteReg32(DMCFGR, 0x5aa50000 | (1<<10)); // CFGR (1<<10 == Allow output from slave)
    WriteReg32(DMSHDWCFGR, 0x5aa50000 | (1<<10)); // sometimes doing this just once isn't enough
    WriteReg32(DMCFGR, 0x5aa50000 | (1<<10)); // And this is about as fast as checking, so why not.
    WriteReg32(DMCONTROL, 0x80000001); // Make the debug module work properly.
    WriteReg32(DMCONTROL, 0x80000001);
    // Why do we repeat this commands? Because in some cases they can be lost, if debug module is still starting
    // and it's better to be send these crucial commands twice fot better chance for success.

    // Read back chip status.  This is really basic.
    u32 reg = 0;
    const int r = ReadReg32(DMSTATUS, &reg);
    if (r >= 0) {
        // Valid R.
        if (reg == 0x00000000 || reg == 0xffffffff) {
            Print("\24Error: Setup chip failed. Got code %08x\27\n", reg);
            return -9;
        }
    } else {
        Print("\24Error: Could not read dmstatus. r = %d\27\n", r);
        return r;
    }

    statetag = STTAG("STRT");
    return 0;
}

/**
 * @brief Display information about connected CH32 chip
 * @param full If True, display additional registers (USER/RDPR, DATA0/1, WRPR0-3, UNIID1-3)
 * @return 0 on success, negative error code on failure
 * @note Shows chip name, flash size, UUID, part type, and read protection status
 * @note Halts MCU before reading chip information
 */
int MinichlinkPrintChipInfo(const Bool full) {
    Halt();
    const int ret = DetermineChipType();
    if (ret) return ret;

    if (full) {
        u32 reg;
        if (ReadWord(target_chip->options_offset, &reg)) goto fail;
        Print("USER/RDPR  : %04x/%04x\n", reg >> 16, reg & 0xFFFF);
        if (ReadWord(0x1FFFF804, &reg)) goto fail;
        Print("DATA1/DATA0: %04x/%04x\n", reg >> 16, reg & 0xFFFF);
        if (ReadWord(0x1FFFF808, &reg)) goto fail;
        Print("WRPR1/WRPR0: %04x/%04x\n", reg >> 16, reg & 0xFFFF);
        if (ReadWord(0x1FFFF80c, &reg)) goto fail;
        Print("WRPR3/WRPR2: %04x/%04x\n", reg >> 16, reg & 0xFFFF);
        if (ReadWord(0x1FFFF7E8, &reg)) goto fail;
        Print("R32_ESIG_UNIID1: %08x\n", reg);
        if (ReadWord(0x1FFFF7EC, &reg)) goto fail;
        Print("\vR32_ESIG_UNIID2: %08x\n", reg);
        if (ReadWord(0x1FFFF7F0, &reg)) goto fail;
        Print("R32_ESIG_UNIID3: %08x\n", reg);
    }
    return 0;
fail:
    Print("\24Error: Failed to get chip details\27\n");
    return -11;
}

/**
 * @brief Get flash size of target chip
 * @return Flash size in bytes
 * @note Automatically detects chip and returns flash_size from chip database
 */
u32 MinichlinkGetFlashSize() {
    MinichlinkPrintChipInfo(False);

    return target_chip->flash_size;
}

/**
 * @brief Erase flash memory on target MCU
 * @param address Start address for sector erase (ignored if type=1)
 * @param length Length in bytes for sector erase (ignored if type=1)
 * @param type Erase type: 0=sector erase, 1=whole chip erase
 * @return 0 on success, negative error code on failure
 * @note Automatically unlocks flash before erasing
 */
int MinichlinkErase(const u32 address, const u32 length, const u8 type) {
    if (!flash_unlocked || type == 1) {
        int rw;
        if ((rw = InternalUnlockFlash()))
            return rw;
    }

    if (type == 1) {
        MinichlinkPrintChipInfo(False);
        // Whole-chip flash
        statetag = STTAG("XXXX");
        Print("Whole-chip erase\n");
        if (WriteWord(FLASH_CTLR, 0)) goto flashoperr; // FLASH->CTLR = 0x40022004
        if (WriteWord(FLASH_CTLR, FLASH_CTLR_MER)) goto flashoperr;
        if (WriteWord(FLASH_CTLR, FLASH_CTLR_STRT | FLASH_CTLR_MER)) goto flashoperr;
        WaitForDoneOp(0);
        WaitForFlash();
        VoidHighLevelState();
    } else {
        // 16.4.7, Step 3: Check the BSY bit of the FLASH_STATR register to confirm that there are no other programming operations in progress.
        // skip (we make sure at the end)
        u32 chunk_to_erase = address;

        chunk_to_erase = chunk_to_erase & ~(sector_size - 1);
        while (chunk_to_erase < address + length) {
            // Step 4:  set PAGE_ER of FLASH_CTLR(0x40022010)
            if (WriteWord(FLASH_CTLR, FLASH_CTLR_PAGE_ER)) goto flashoperr; // CR_PAGE_ER is FTER
            // Step 5: Write the first address of the fast erase page to the FLASH_ADDR register.
            if (WriteWord(0x40022014, chunk_to_erase)) goto flashoperr;
            //if (PrepForLongOp) PrepForLongOp(dev);
            // Give the programmer a headsup this next operation could take a while.

            // Step 6: Set the STAT/STRT bit of FLASH_CTLR register to '1' to initiate a fast page erase (64 bytes) action.
            if (WriteWord(FLASH_CTLR, (1 << 6) | FLASH_CTLR_PAGE_ER)) goto flashoperr;

            if (WaitForFlash()) return -99;

            chunk_to_erase += sector_size;
        }
    }

    return 0;
flashoperr:
    Print("\24Error: Flash operation error\27\n");
    return -93;
}

/**
 * @brief Flash binary image to target MCU
 * @param offset Flash address to write to (typically 0x08000000)
 * @param len Length of binary image in bytes
 * @param image Pointer to binary image data
 * @return 0 on success, negative error code on failure
 * @note Performs whole chip erase before programming
 */
int MinichlinkFlash(const u64 offset, const u32 len, const u8* image) {
    MinichlinkErase(0, 0, 1); // type=1 means whole chip erase
    printf("Writing image\n");
    if (WriteBinaryBlob(offset, len, image)) {
        Print("\24Error: Fault writing image.\27\n");
        return -13;
    }

    return 0;
}
