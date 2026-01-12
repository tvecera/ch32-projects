# CH32 Projects

A collection of example projects and experiments for CH32-series microcontrollers from WCH (Nanjing Qinheng
Microelectronics). This repository serves as a personal playground for embedded development with CH32 devices, focusing
on low-cost, open-source hardware hacking.

## Featured Projects

### picopad-ch32prog

- **Description**: RISC-V microcontroller programmer for WCH CH32V, CH32V00X, and CH32V03X series, running on the
  Picopad handheld device. It enables programming CH32 microcontrollers directly from the Picopad's SD card without
  requiring a PC connection.
- **Repository**: [tvecera/ch32-projects/tree/main/picopad-ch32prog](https://github.com/tvecera/ch32-projects/tree/main/picopad-ch32prog)

### ch32-lamp – Music Player (BeatleBeat)

- **Description**: Music module for playing mono WAV (IMA ADPCM) files from an SD card. Based on the BeatleBeat design
  from CH32LibSDK, using the CH32V002x4 MCU. Features BTL PWM output for louder sound without external amplifier.
- **Repository**: [tvecera/ch32-projects/tree/main/ch32-lamp](https://github.com/tvecera/ch32-projects/tree/main/ch32-lamp)

### Tinyboy Console

- **Description**: An adaptation of the CH32V003-GameConsole project by Stefan Wagner for CH32LibSDK, using the
  newer CH32V002J4M6 MCU instead of the discontinued CH32V003. Supports two video outputs: original I2C OLED display
  or RCA composite video output using the rca-library.
- **Repository**: [tvecera/ch32-projects/tree/main/tinyboy](https://github.com/tvecera/ch32-projects/tree/main/tinyboy)

### Olimex RVPC Port to CH32LibSDK

- **Description**: Port of the RVPC (the EURO 1.00 RISC-V personal computer with VGA, keyboard, and Woz-like monitor)
  from Olimex to the CH32LibSDK. In addition to the SDK port, RCA composite video output support has been added
  using the rca-library.
- **Repository**: [tvecera/ch32-projects/tree/main/rvpc](https://github.com/tvecera/ch32-projects/tree/main/rvpc)

### RCA Composite Video Library

- **Description**: A library for generating composite video signal (RCA output) on CH32V002/CH32V003 microcontrollers.
  Supports multiple graphics modes (128x64, 160x120) and text modes (40x30, 32x24, 23x18 characters) with minimal
  RAM and Flash usage. Features both bit-bang and SPI output modes, works with internal oscillator (HSI) or external
  crystal (HSE).
- **Repository**: [tvecera/ch32-projects/tree/main/rca-library](https://github.com/tvecera/ch32-projects/tree/main/rca-library)

### RCA Tester

- **Description**: A simple composite video tester and demonstration of the rca-library capabilities. Generates
  PAL composite video signals with multiple test patterns (grid, crosshatch, circles, checkerboard, etc.) and text
  modes with various fonts. Includes complete hardware design files for DIY manufacturing (KiCAD schematics, PCB
  for laser etching, 3D printable case). Supports both CH32LibSDK and ch32fun frameworks.
- **Repository**: [tvecera/ch32-projects/tree/main/rca-tester](https://github.com/tvecera/ch32-projects/tree/main/rca-tester)

### Pimitachi – 3D Case and Labels

- **Description**: 3D printable case, labels, and laser-etched PCB design for Pimitachi – a Tamagotchi-style virtual pet
  game running on the CH32V002J4M6 MCU. Includes LightBurn project for PCB fabrication with a 10W laser engraver,
  printable labels (photo paper + transparent film), and assembly instructions.
- **Original project**: [Pimitachi on breatharian.eu](https://www.breatharian.eu/hw/ch32libsdk/#pimitachi)
- **Repository**: [tvecera/ch32-projects/tree/main/pimitachi](https://github.com/tvecera/ch32-projects/tree/main/pimitachi)

## Supported Platforms

Build system supports **Linux** and **macOS** (both x86_64 and ARM64/Apple Silicon).

## Getting Started

### Cloning the Repository

This repository uses Git submodules for SDK dependencies. Clone with:

```bash
git clone --recursive https://github.com/tvecera/ch32-projects.git
```

Or if you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

### SDK Submodules

The repository includes three SDK submodules in the `sdk/` directory:

| SDK | Description |
|-----|-------------|
| [CH32LibSDK](https://github.com/Panda381/CH32LibSDK) | SDK for CH32V/X series MCUs |
| [PicoLibSDK](https://github.com/Panda381/PicoLibSDK) | SDK for Raspberry Pi Pico / RP2040 / RP2350 |
| [ch32fun](https://github.com/cnlohr/ch32fun) | Lightweight SDK for CH32V003 |

### Required Toolchains

Different projects require different toolchains:

#### For CH32 Projects (ch32-lamp, rvpc, rca-library, tinyboy)

- **RISC-V GCC Toolchain** - `riscv-none-elf-gcc`

**Recommended: xPack RISC-V GCC (cross-platform):**

Download pre-built binaries from [xPack RISC-V GCC releases](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) and add to PATH.

**macOS/Linux (npm):**
```bash
npm install --location=global @xpack-dev-tools/riscv-none-elf-gcc@latest
```

#### For Picopad Projects (picopad-ch32prog)

- **ARM GCC Toolchain** - `arm-none-eabi-gcc`

**macOS (Homebrew):**
```bash
brew install arm-none-eabi-gcc
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt install gcc-arm-none-eabi
```

#### Additional Tools

- **make** - Build automation
- **minichlink** - CH32 programmer/debugger (included in `tools/minichlink/`)