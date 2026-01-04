# RCA Composite Video Library for CH32V00x

A library for generating composite video signal (RCA output) on CH32V002 / CH32V003 (RISC-V) microcontrollers.
Supports graphics and text modes with minimal RAM and Flash memory usage.

## Composite Video (RCA) Basics

### What is Composite Video?

Composite video (CVBS - Composite Video Baseband Signal) is an analog video signal that combines image information
and synchronization pulses into a single wire. This standard originated in the era of black-and-white televisions
and is still supported by most CRT and some LCD televisions today.

### Signal Structure

```
Voltage (V)
    ^
1.0 |████████          ████████     <- White
    |
0.3 |        ██████████        ███  <- Black level
    |
0.0 |██                           ██ <- Sync pulses
    +---------------------------------> Time
      |  |        |              |
      |  |        |              +-- HSYNC (4.7 us)
      |  |        +-- Active video area
      |  +-- Back porch (5.7 us)
      +-- Front porch (1.5 us)
```

### Signal Levels

| Level | Voltage | Description                             |
|-------|---------|----------------------------------------|
| Sync  | 0.0 V   | Horizontal and vertical synchronization |
| Black | 0.3 V   | Black color (blanking level)            |
| White | 1.0 V   | White color (maximum brightness)        |

### PAL Timing (used in this library)

| Parameter       | Value      | Description                  |
|-----------------|------------|------------------------------|
| Line duration   | 64 us      | Period of one line           |
| HSYNC pulse     | 4.7 us     | Horizontal sync              |
| Back porch      | 5.7 us     | Time before active video     |
| Active video    | ~52 us     | Area for image data          |
| Line count      | 312        | Lines per frame (half PAL)   |
| Frame rate      | ~50 Hz     | Display refresh rate         |
| Line frequency  | 15.625 kHz | Line frequency               |

### How the Library Generates the Signal

1. **TIM2 PWM** generates SYNC pulses on pin PD4 (or PC1)
2. **TIM2 CC2 interrupt** triggers pixel rendering at the correct time
3. **Bit-bang/SPI** outputs image data on video pin (PD6 or PC6)
4. **IRQ handler in RAM** ensures constant timing without flash wait-states

```
TIM2 Counter: 0 -----> TIM_HSYNC_PULSE -----> TIM_VIDEO_START -----> TIM_LINE_PERIOD
                |                       |                      |
                v                       v                      v
           SYNC LOW                SYNC HIGH              IRQ triggers
                                   (black level)          pixel output
```

## Video Modes

### Mode Overview

| VMODE | Resolution  | Type     | Framebuffer | Total RAM (SPI) | Description                            |
|-------|-------------|----------|-------------|-----------------|----------------------------------------|
| 0     | 128x64 px   | Graphics | 1024 B      | 1282 B          | Low resolution, 4x vertical scale      |
| 1     | 160x120 px  | Graphics | 2400 B      | 2652 B          | Medium resolution, 2x vertical scale   |
| 6     | 40x30 chars | Text     | 1200 B      | 1606 B          | 8x8 font, 320x240 effective pixels     |
| 7     | 32x24 chars | Text     | 768 B       | 1180 B          | 8x8 font, wider pixels                 |
| 8     | 23x18 chars | Text     | 414 B       | 854 B           | 8x14 font (stretched), smallest RAM    |

*Total RAM = Framebuffer + IRQ handler in RAM + state variables. SPI mode has a smaller handler.*

### VMODE 0 - Graphics 128x64

- **Resolution:** 128x64 pixels
- **Framebuffer:** 1024 bytes (1 bit/pixel, 8 pixels/byte)
- **Vertical scale:** 4x (each line is drawn 4 times)
- **Active lines:** 256
- **Suitable for:** simple graphics applications, games

### VMODE 1 - Graphics 160x120

- **Resolution:** 160x120 pixels
- **Framebuffer:** 2400 bytes (1 bit/pixel, 8 pixels/byte)
- **Vertical scale:** 2x (bit-bang), 1x (SPI)
- **Active lines:** 240 (bit-bang), 120 (SPI)
- **Suitable for:** more detailed graphics, images
- **CH32V002 only** (requires 4 KB RAM)

### VMODE 6 - Text 40x30

- **Resolution:** 40x30 characters (320x240 effective pixels)
- **Framebuffer:** 1200 bytes (1 byte/character)
- **Font:** 8x8 pixels, 64 or 128 characters
- **Suitable for:** terminal, text games

### VMODE 7 - Text 32x24

- **Resolution:** 32x24 characters (256x192 effective pixels)
- **Framebuffer:** 768 bytes (1 byte/character)
- **Font:** 8x8 pixels with wider pixels
- **Suitable for:** larger text, better readability

### VMODE 8 - Text 23x18

- **Resolution:** 23x18 characters (184x252 effective pixels)
- **Framebuffer:** 414 bytes (1 byte/character)
- **Font:** 8x14 pixels (8 rows stretched to 14)
- **Suitable for:** minimal RAM, BASIC interpreter
- **Original RVPC video mode**

## Compatibility

### CH32FUN / CH32V002 / CH32V003

| VMODE | Bit-bang | SPI | RAM (bit-bang) | RAM (SPI) | Flash       | Notes                                                             |
|-------|----------|-----|----------------|-----------|-------------|-------------------------------------------------------------------|
| 0     | OK       | OK  | 1368 B         | 1282 B    | —           | SPI: narrower width, centered                                     |
| 1     | OK       | OK  | 2738 B         | 2652 B    | —           | SPI: no 1:1 scale, centered. Not available for CH32V003, requires > 2 KB RAM |
| 6     | OK       | OK  | 1608 B         | 1606 B    | 2-4 KB font | Centered                                                          |
| 7     | OK       | OK  | 1206 B         | 1180 B    | 2-4 KB font | Centered                                                          |
| 8     | OK       | OK  | 888 B          | 854 B     | 2-4 KB font | 2x scale, 14px height (rows 2,4 only 1x), SPI: narrower width     |

### CH32LIBSDK / CH32V002 / CH32V003

| VMODE | Bit-bang | SPI | RAM (bit-bang) | RAM (SPI) | Flash       | Notes                                                             |
|-------|----------|-----|----------------|-----------|-------------|-------------------------------------------------------------------|
| 0     | OK       | OK  | 1368 B         | 1282 B    | —           | SPI: narrower width, centered                                     |
| 1     | OK       | OK  | 2738 B         | 2652 B    | —           | SPI: no 1:1 scale, centered. Not available for CH32V003, requires > 2 KB RAM |
| 6     | OK       | OK  | 1608 B         | 1606 B    | 2-4 KB font | Centered                                                          |
| 7     | OK       | OK  | 1206 B         | 1180 B    | 2-4 KB font | Centered                                                          |
| 8     | OK       | OK  | 888 B          | 854 B     | 2-4 KB font | 2x scale, 14px height (rows 2,4 only 1x), SPI: narrower width     |

## Wiring Diagram

### Basic Wiring (Bit-bang, 5V power)

```
        CH32V00x
     +--------------+
     |              |
PD6 -| VIDEO_PIN    |----[ 330R ]---+-----> VIDEO (RCA Center)
     |              |               |
PD4 -| SYNC_PIN     |----[ 680R ]---+
     |              |
GND -| GND          |---------------------> GND (RCA Shield)
     |              |
     +--------------+
```

### SPI Mode (USE_RCA_SPI=1)

In SPI mode, pin **PC6** (SPI1_MOSI) is used for video data.

```
        CH32V00x
     +--------------+
     |              |
PC6 -| VIDEO_PIN    |----[ 330R ]---+-----> VIDEO (RCA Center)
     | (SPI1_MOSI)  |               |
     |              |               |
PD4 -| SYNC_PIN     |----[ 680R ]---+
     |              |
GND -| GND          |---------------------> GND (RCA Shield)
     |              |
     +--------------+
```

*Note: SYNC_PIN can also be PC1, see `rca_video.c`.*

### Signal Levels

The resistor divider creates correct voltage levels into the TV's 75 ohm load:

| State | SYNC_PIN  | VIDEO_PIN | Output Voltage |
|-------|-----------|-----------|----------------|
| Sync  | LOW (0V)  | LOW (0V)  | ~0.0 V         |
| Black | HIGH (5V) | LOW (0V)  | ~0.3 V         |
| White | HIGH (5V) | HIGH (5V) | ~1.0 V         |

### Resistor Calculation

For 5V GPIO and 75 ohm TV input impedance:

- **SYNC (680 ohm):** 5V x 75/(680+75) ~ 0.50V (black level)
- **VIDEO (330 ohm):** Adds ~0.9V to sync level to achieve white

## Clock Source (HSI vs HSE)

The library requires 50 MHz system clock for correct video timing. This can be achieved in two ways:

### HSI - Internal Oscillator (USE_HSI=1)

**High-Speed Internal** is an RC oscillator integrated in the chip.

| Property        | Value                          |
|-----------------|--------------------------------|
| Frequency       | ~24 MHz (trimmed to ~25 MHz)   |
| Accuracy        | +/-3% (typical), +/-5% (max)   |
| Temperature drift | Significant                  |
| Required hardware | None                         |

**Advantages:**

- No external components
- Instant start

**Disadvantages:**

- Less accurate timing - image may slightly "float"
- Some TVs may have synchronization problems
- Frequency changes with temperature

**Configuration:**

```c
#define USE_HSI     1
```

### HSE - External Crystal (USE_HSI=0)

**High-Speed External** uses a precise crystal oscillator.

| Property        | Value                    |
|-----------------|--------------------------|
| Frequency       | 25 MHz (exact)           |
| Accuracy        | +/-20 ppm (typical)      |
| Temperature drift | Minimal                |
| Required hardware | Crystal + 2x capacitor |

**Advantages:**

- Accurate and stable timing
- Reliable synchronization on all TVs
- Consistent behavior

**Disadvantages:**

- Requires external components
- Longer startup (crystal stabilization)
- Uses pins PA1/PA2

**Configuration:**

```c
#define USE_HSI     0
```

### External Crystal Wiring

```
                         25 MHz
                        +------+
          PA1 (OSC_IN)--+      +--PA2 (OSC_OUT)
               |        | XTAL |        |
               |        +------+        |
               |                        |
             [22pF]                  [22pF]
               |                        |
              GND                      GND
```

**Recommended components:**

| Component   | Value    | Notes                                    |
|-------------|----------|------------------------------------------|
| Crystal     | 25 MHz   | HC49 or SMD, 12-20 pF load capacitance   |
| Capacitors  | 2x 22 pF | Ceramic, place as close to MCU as possible |

**Important for breadboard:**

Crystal oscillators are sensitive to parasitic capacitance. On a breadboard:

- Use **shorter wires** (crystal as close to MCU as possible)
- Try **smaller capacitors** (10-15 pF) - breadboard adds its own capacitance
- Place capacitors **directly at MCU pins**, not at the crystal

### Recommendations

| Use Case                | Recommended Source          |
|-------------------------|-----------------------------|
| Prototyping, experiments | HSI                        |
| Production devices      | HSE                         |
| Battery-powered         | HSI (lower startup consumption) |
| Precise timing critical | HSE                         |

### Observation: CH32V003 vs CH32V002

Based on practical tests, **CH32V003 has significantly less jitter** when using the internal oscillator (HSI) than CH32V002:

| MCU      | HSI jitter | Image stability                 |
|----------|------------|--------------------------------|
| CH32V003 | Low        | Stable image, minimal wobble   |
| CH32V002 | Higher     | Slight wobble, some TVs more sensitive |

**Recommendation:** For HSI mode, prefer CH32V003 if RAM is sufficient (2 KB).

## Build and Toolchain

### Recommended Toolchain

The library was primarily developed and tested with **xpack-riscv-none-elf-gcc 15.2.0**.

| Toolchain                       | Status        | Notes                                   |
|---------------------------------|---------------|-----------------------------------------|
| xpack-riscv-none-elf-gcc 15.2.0 | Recommended   | Fully functional, including LTO         |
| xpack-riscv-none-elf-gcc 14.x   | OK            | Functional                              |
| riscv-wch-elf-gcc (WCH)         | Issues        | -flto causes incorrect .constprop clones |

**Download xpack toolchain:**

- https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases

**Setting PATH:**

```bash
export PATH="/path/to/xpack-riscv-none-elf-gcc-15.2.0-1/bin:$PATH"
```

### CH32Fun - Specific Flags

For xpack toolchain, architecture flags need to be modified:

```makefile
# Override architecture for xpack toolchain (add zicsr+zifencei)
ifeq ($(findstring CH32V003,$(TARGET_MCU)),CH32V003)
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei -mabi=ilp32e
else
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei_zmmul -mabi=ilp32e
endif

# Replace original -march in CFLAGS_ARCH
CURRENT_ARCH = $(filter -march=%,$(CFLAGS_ARCH))
CFLAGS_ARCH := $(subst $(CURRENT_ARCH),$(firstword $(ARCHCFG_XPACK)),$(CFLAGS_ARCH))

# Consistent settings with ch32libsdk
CFLAGS := $(subst -msmall-data-limit=8,-msmall-data-limit=0,$(CFLAGS))
CFLAGS += -funsigned-char
CFLAGS += -fno-ipa-cp
```

**Flag explanation:**

| Flag                           | Reason                                                 |
|--------------------------------|--------------------------------------------------------|
| `-march=rv32ec_zicsr_zifencei` | Xpack GCC 15+ requires explicit CSR/FENCE extensions   |
| `_zmmul`                       | Hardware multiplier (CH32V002 only, not V003)          |
| `-msmall-data-limit=0`         | Disable small data section for consistent timing       |
| `-funsigned-char`              | Char is unsigned (ch32libsdk compatibility)            |
| `-fno-ipa-cp`                  | Disable IPA constant propagation (more stable inlining)|

### CH32LibSDK - Specific Flags

```makefile
# Include CH32LibSDK Makefile1.inc first

# LTO optimization (important for code size)
CFLAGS += -flto
LDFLAGS += -flto

# Reduced CRT0 (smaller startup code)
EXTRA_AFLAGS += -D REDUCE_CRT0_SIZE=1

# Include CH32LibSDK Makefile2.inc

# Override architecture for xpack toolchain (AFTER Makefile2.inc!)
ifeq ($(findstring CH32V003,$(MCU)),CH32V003)
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei -mabi=ilp32e
else
ARCHCFG_XPACK = -march=rv32ec_zicsr_zifencei_zmmul -mabi=ilp32e
endif

CFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(CFLAGS))
LDFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(LDFLAGS))
AFLAGS := $(subst $(ARCHCFG),$(ARCHCFG_XPACK),$(AFLAGS))
```

**Flag explanation:**

| Flag                  | Reason                                           |
|-----------------------|--------------------------------------------------|
| `-flto`               | Link-Time Optimization - smaller and faster code |
| `REDUCE_CRT0_SIZE=1`  | Smaller startup code (~100 B Flash savings)      |
| Architecture override | Xpack GCC 15+ requires explicit zicsr/zifencei   |

### Interrupt Timing Optimization

For minimal video signal jitter, the library uses specific settings:

#### 1. Disabling HPE (Hardware Prologue/Epilogue)

```c
// in rca_config.h
#define FUNCONF_ENABLE_HPE  0
```

**What is HPE?**
CH32V00x has a hardware HPE function that automatically saves/restores registers when entering/exiting interrupts.

**Why disable it?**

- HPE adds **variable latency** when entering IRQ
- Number of saved registers depends on context -> inconsistent timing
- For video, **deterministic** handler entry is critical
- Without HPE, IRQ handler has constant latency of ~12 cycles

#### 2. INTSYSCR Register Configuration

```c
// in rca_init()
asm volatile("li t0, 0x03; csrw 0x804, t0" ::: "t0");
```

**What is INTSYSCR (0x804)?**
Interrupt System Control Register - controls interrupt behavior.

| Bit | Name    | Value | Meaning                  |
|-----|---------|-------|--------------------------|
| 0   | HWSTKEN | 1     | Hardware stack enable    |
| 1   | INESTEN | 1     | Interrupt nesting enable |

**Why set to 0x03?**

- **HWSTKEN=1:** Enables hardware stack for faster context switch
- **INESTEN=1:** Enables interrupt nesting (necessary for consistent timing)
- This setting matches CH32LibSDK default behavior
- CH32Fun has different default values -> would have different timing without this

**Important:** This setting must be done **before** the first interrupt, hence it's in `rca_init()`.

## API Reference

### Initialization and Control

```c
// Initialize video output
// Configures HSI, GPIO, TIM2 and NVIC.
void rca_init();

// Stop video output
// Deactivates timer, interrupts and restores HSI.
void rca_stop();
```

### Global Variables

```c
extern u8 FrameBuf[FRAMESIZE];  // Framebuffer (pixels or characters)
extern volatile u32 DispLine;   // Current line (0..311), updated in IRQ
extern volatile u32 DispFrame;  // Frame counter, incremented at each VSYNC
```

### Callback

```c
// Sets a function to be called at the end of each frame (during VBLANK).
// Warning: Runs in IRQ context! Must be very fast.
void rca_set_frame_callback(void (*callback)());
```

### Helper Functions

```c
// Returns whether VSYNC period is in progress.
static inline u8 rca_in_vsync();

// Blocks execution until the start of the next VSYNC period.
// Use before updating FrameBuf to prevent "tearing".
static inline void rca_wait_vsync();
```

### Text Modes

```c
// Updates internal font pointers.
// Must be called after changing font at runtime in text modes.
void rca_update_font();
```

## Configuration

### Basic Configuration (config.h)

| Macro         | Values           | Description                            |
|---------------|------------------|----------------------------------------|
| `VMODE`       | 0, 1, 6, 7, 8    | Video mode                             |
| `USE_RCA_SPI` | 0, 1             | 0=bit-bang, 1=SPI                      |
| `USE_HSI`     | 0, 1             | 0=external crystal, 1=internal oscillator |
| `FONT`        | FontBold8x8, ... | Font for drawing text                  |

### Advanced Configuration (rca_config.h)

| Macro             | Default  | Description                    |
|-------------------|----------|--------------------------------|
| `HSE_VALUE`       | 25000000 | External crystal frequency     |
| `HSI_VALUE`       | 25000000 | Internal oscillator frequency  |
| `PLLCLK_MUL`      | 2        | PLL multiplier (25x2=50 MHz)   |
| `SYSCLK_DIV`      | 1        | System clock divider           |
| `TIM_VIDEO_START` | auto     | Video start offset (ns)        |

### Changing Pins

To change VIDEO or SYNC pins, you need to modify:

1. **Pin definitions** in `rca_video.c`:
    - `VIDEO_PIN`, `VIDEO_PIN_MASK`, `VIDEO_GPIO_BCR`
    - `SYNC_PIN`

2. **GPIO clocks** in `rca_init()`:
    - `RCC_PxClkEnable()` for the used port

3. **TIM2 remapping** for SYNC_PIN:
   ```c
   // CH32V002/V006/V007:
   // 0: PD4  1: PC1  2: PC5  3: PC1  4: PC0  5: PA0  6: PB1  7: PD3
   GPIO_Remap_TIM2(n);
   ```

## License

MIT License - see source file headers.

## Links

- [CH32LibSDK](https://github.com/Panda381/CH32LibSDK) by Miroslav Nemecek (Panda38)
- [ch32fun](https://github.com/cnlohr/ch32fun) by CNLohr
