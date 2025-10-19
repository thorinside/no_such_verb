# Building no_such_verb

## Prerequisites

- ARM GNU Toolchain (arm-none-eabi-gcc, arm-none-eabi-g++, arm-none-eabi-ar)
- GNU Make
- git

## Quick Start

```bash
./setup.sh
make
```

## What setup.sh Does

The `setup.sh` script automates the entire dependency setup process:

1. Verifies ARM toolchain is installed
2. Clones libDaisy and DaisySP repositories
3. Initializes and updates nested submodules for both libraries
4. Applies const-correctness patch to libDaisy's patched HAL driver
5. Builds libDaisy
6. Builds DaisySP
7. Builds DaisySP-LGPL module

## Building the Plugin

After `./setup.sh` completes successfully, build the plugin:

```bash
make
```

Output files:
- `build/no_such_verb.bin` - Binary firmware image for SD card deployment
- `build/no_such_verb.elf` - ELF executable for debugging
- `build/no_such_verb.hex` - Intel HEX format

## Deployment

By default, this generates firmware for BOOT_QSPI (SD card boot):

```bash
make program-boot   # Program bootloader (hold boot, press reset)
make program-dfu    # Program firmware via DFU (press reset, then boot quickly)
```

## Cleaning

```bash
make clean          # Clean plugin build only
./setup.sh clean    # Clean all builds (not yet implemented, do manually)
```

To fully clean:
```bash
rm -rf build libDaisy DaisySP
```

## Architecture

This project uses the Daisy platform with two main dependencies:

- **libDaisy** - Hardware abstraction layer for Daisy Seed/Patch.init()
- **DaisySP** - Signal processing DSP library

Both are built as static libraries that link into the final firmware.
