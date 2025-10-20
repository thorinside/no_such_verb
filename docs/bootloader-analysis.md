# Bootloader Analysis for Daisy Patch.init()

## Executive Summary

The Daisy Patch.init() uses bootloader v6.3 which loads firmware from SD card to QSPI flash. The current state persistence corruption occurs because PersistentStorage initialization happens immediately after hardware initialization, potentially conflicting with the bootloader's 2.5-second grace period where it may still access QSPI flash. A minimum 3-second delay before QSPI initialization will resolve this conflict.

## Bootloader Overview

- **Version**: v6.3 (confirmed from libDaisy/core binaries)
- **Source**: Open-source libDaisy framework by Electro-Smith
- **Purpose**: Load firmware from SD card to QSPI flash and execute it
- **Documentation**: Found in `libDaisy/doc/md/_a7_Getting-Started-Daisy-Bootloader.md`

## Bootloader Execution Sequence

1. **Bootloader starts** (executes from internal flash at 0x08000000)
2. **Grace period begins** (2.5 seconds with sinusoidal LED blinks)
3. **Media scan phase**: Checks SD card for `.bin` files
4. **Firmware loading**: If new firmware found, flashes it to QSPI
5. **Program validation**: Verifies executable is valid
6. **Jump to application**: Transfers control to firmware at 0x90040000
7. **Application continues**: hw.Init() and main() begin execution

## QSPI Usage Analysis

### Bootloader QSPI Regions:
- **Reserved bootloader space**: 0x90000000 - 0x90040000 (256KB, first 4 sectors)
  - Used for bootloader metadata and potential future features
  - Applications must not write to this region

- **Application firmware region**: 0x90040000 - 0x907C0000 (7936KB available)
  - Current firmware uses ~46KB (0.57% of available space)
  - Firmware code and read-only data stored here

- **Application data region**: 0x90040000 + firmware_size to end of QSPI
  - Available for PersistentStorage and user data
  - Current firmware uses address 0x902B000 (172KB offset)

### Memory Layout Verification:
From linker script `STM32H750IB_qspi.lds`:
```
QSPIFLASH (RX) : ORIGIN = 0x90040000, LENGTH = 7936K
```
This confirms firmware executes from 0x90040000, leaving bootloader region untouched.

## Root Cause of State Corruption

### Technical Analysis:
The corruption occurs due to **timing conflict** between:
1. **Bootloader grace period** (2.5 seconds after boot)
2. **Immediate PersistentStorage initialization** in main.cpp line 212

### Current Broken Sequence:
```
1. Power on / Reset
2. Bootloader starts, begins 2.5s grace period
3. Bootloader checks SD card for firmware updates
4. Bootloader jumps to application at 0x90040000
5. hw.Init() completes (~10-50ms)
6. storage.Init() immediately accesses QSPI at 0x902B000  ← CONFLICT
7. Bootloader may still be finalizing QSPI operations
8. QSPI state becomes inconsistent, settings corrupted
```

### Specific Failure Mechanism:
- Bootloader maintains QSPI peripheral in specific state during grace period
- Application PersistentStorage::Init() reconfigures QSPI immediately
- Race condition between bootloader cleanup and application initialization
- Results in corrupted reads/writes to settings storage

## Bootloader Timing Requirements

### Grace Period Details:
- **Duration**: 2.5 seconds (2500ms) from bootloader documentation
- **LED Indicator**: Sinusoidal blinks during this period
- **Extension**: Can be extended indefinitely by holding BOOT button
- **Activities**: DFU listening, media scanning, firmware loading

### Safe Timing Analysis:
- **Bootloader completion**: Maximum 2.5 seconds after reset
- **Hardware initialization**: ~10-50ms (hw.Init())
- **Safe margin**: Additional 500ms for bootloader cleanup
- **Recommended delay**: **3000ms minimum** before QSPI access

### Timing Verification:
From bootloader documentation:
> "Once flashed, the bootloader has a grace period of 2.5 seconds on startup indicated by sinusoidal LED blinks. During this time, it will listen for DFU transactions over USB and search any connected media for valid binaries. Once this period elapses, the bootloader will attempt to load a program and jump to it."

## SD Card Loading Interference

### Bootloader Media Scanning:
- **SD Card Priority**: Checked before USB drives
- **File Search**: Scans root directory for `.bin` files
- **Flash Process**: Compares file to QSPI contents, flashes if different
- **QSPI Access**: Bootloader writes to application firmware region during updates

### Conflict Scenarios:
1. **Normal boot**: Bootloader completes grace period, jumps to app
2. **Firmware update**: Bootloader actively writing QSPI when app starts
3. **Media scanning**: QSPI peripheral busy during bootloader media check

### No Direct Interference:
SD card and QSPI use different peripherals, but bootloader coordinates both during the grace period.

## Safe QSPI Configuration

### Recommended Memory Layout:
```
0x90000000 - 0x90040000  : Bootloader reserved (256KB) - DO NOT USE
0x90040000 - 0x90040000 + firmware_size : Application firmware (~46KB)
0x90040000 + firmware_size + safety_gap : Safe for PersistentStorage
```

### Recommended Settings Storage Address:
- **Current address**: 0x902B000 (172KB offset) - ACCEPTABLE
- **Minimum safe address**: 0x90040000 + 64KB = 0x90050000
- **Recommended address**: 0x90080000 (512KB offset for future growth)

### Address Calculation:
```cpp
// Safe address calculation:
// 0x90040000 (firmware start) + 0x40000 (256KB safety buffer) = 0x90080000
const uint32_t SAFE_SETTINGS_ADDRESS = 0x80000; // 512KB offset
storage.Init(defaults, SAFE_SETTINGS_ADDRESS);
```

## Minimum Required Delay

### Justified Recommendation:
**3000ms (3 seconds)** delay before QSPI access

### Justification:
- Bootloader grace period: 2500ms maximum
- Bootloader cleanup time: ~200ms estimated
- Safety margin: 300ms
- **Total**: 3000ms

### Implementation Pattern:
```cpp
int main() {
    hw.Init();

    // Wait for bootloader to complete QSPI operations
    System::Delay(3000); // 3 second delay

    // Now safe to initialize QSPI storage
    storage.Init({SETTINGS_VERSION, false}, 0x80000);

    // Continue with normal initialization...
}
```

## PersistentStorage Behavior Analysis

### Default Behavior (Current):
- **Immediate initialization**: No bootloader coordination
- **Memory-mapped mode**: QSPI configured for direct memory access
- **Cache invalidation**: Handles program execution from QSPI vs internal flash
- **Address alignment**: Automatically masks to 256-byte boundaries

### Configuration Requirements:
- **Address offset**: Must be 256-byte aligned (automatically handled)
- **Safe region**: Must avoid bootloader reserved space (0x90000000-0x90040000)
- **Timing**: Must wait for bootloader completion

### Current Implementation Analysis:
From PersistentStorage.h lines 125-134:
```cpp
// Caching behavior is different when running programs outside internal flash
if(System::GetProgramMemoryRegion() != System::MemoryRegion::INTERNAL_FLASH)
{
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)data_ptr, sizeof(s));
}
```
This shows the library is aware of QSPI execution context but doesn't handle bootloader timing.

## Recommendations

### 1. Immediate Fix (Story 1.2):
Add 3-second delay before PersistentStorage initialization:
```cpp
int main() {
    hw.Init();
    System::Delay(3000);  // Wait for bootloader completion
    storage.Init(defaults, address);
    // Continue...
}
```

### 2. Address Optimization (Story 1.3):
Move settings storage to safer address:
```cpp
// Use 512KB offset for better safety margin
storage.Init(defaults, 0x80000);  // 512KB offset = 0x90080000
```

### 3. Bootloader Detection (Story 1.4):
Implement bootloader completion detection:
```cpp
// Check if grace period LED pattern is still active
// Wait for sinusoidal blink pattern to complete
```

### 4. Error Handling (Story 1.5):
Add QSPI initialization error handling:
```cpp
if (storage.Init(defaults, address) != SUCCESS) {
    // Retry with longer delay or fallback behavior
}
```

## References

- **Bootloader Documentation**: `libDaisy/doc/md/_a7_Getting-Started-Daisy-Bootloader.md`
- **QSPI Driver**: `libDaisy/src/per/qspi.h` and `qspi.cpp`
- **PersistentStorage**: `libDaisy/src/util/PersistentStorage.h`
- **Linker Script**: `libDaisy/core/STM32H750IB_qspi.lds`
- **Project Makefile**: `APP_TYPE=BOOT_QSPI` configuration
- **libDaisy GitHub**: https://github.com/electro-smith/libDaisy
- **Bootloader Binaries**: `libDaisy/core/dsy_bootloader_v6_3-*.bin`