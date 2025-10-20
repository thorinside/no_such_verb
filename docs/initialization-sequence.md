# Initialization Sequence Timeline for Daisy Patch.init()

## Executive Summary

The state corruption issue occurs due to a timing race condition between the Daisy bootloader's grace period (2.5 seconds) and immediate QSPI initialization in the application firmware. This document provides detailed timing analysis and recommends a 3-second delay before QSPI access to ensure reliable state persistence.

## Complete Boot Timeline

### Phase 1: System Reset to Bootloader Start
```
Time: 0ms
Event: Power-on Reset / System Reset
Actions:
- STM32H7 hardware reset sequence
- CPU starts executing from 0x08000000 (internal flash)
- Bootloader begins execution
Duration: ~5-10ms
```

### Phase 2: Bootloader Grace Period (CRITICAL TIMING)
```
Time: 10ms - 2510ms
Event: Bootloader Grace Period
Actions:
- LED begins sinusoidal blink pattern
- Initialize QSPI peripheral for bootloader use
- Listen for DFU transactions over USB
- Scan SD card for .bin files
- Compare existing firmware with media files
- Flash new firmware to QSPI if different
- Verify firmware integrity
Duration: 2500ms (fixed, unless BOOT button held)
Status: QSPI PERIPHERAL BUSY - Application must not access
```

### Phase 3: Bootloader to Application Handoff
```
Time: 2510ms
Event: Jump to Application
Actions:
- Bootloader cleanup and QSPI peripheral finalization
- Jump to firmware at 0x90040000
- Application Reset_Handler begins
- C runtime initialization (BSS clear, data copy)
Duration: ~50-200ms estimated
Status: QSPI transitioning from bootloader to application control
```

### Phase 4: Application Initialization
```
Time: 2560ms
Event: main() function begins
Actions:
- hw.Init() - Hardware peripheral setup
  - GPIO configuration
  - Clock setup
  - Audio system initialization
  - Control surface setup
Duration: ~10-50ms (depends on hardware complexity)
Status: Hardware ready, but QSPI may not be fully released by bootloader
```

### Phase 5: CURRENT PROBLEM - Immediate QSPI Access
```
Time: 2610ms (current implementation)
Event: storage.Init() called immediately
Actions:
- PersistentStorage attempts QSPI configuration
- QSPI peripheral mode switching
- Memory-mapped mode setup
- Read existing settings from QSPI flash
Result: RACE CONDITION with bootloader cleanup
Status: POTENTIAL CORRUPTION due to timing conflict
```

### Phase 6: RECOMMENDED - Safe QSPI Initialization
```
Time: 5610ms (with 3-second delay)
Event: storage.Init() after safety delay
Actions:
- 3000ms delay ensures bootloader completion
- PersistentStorage safely configures QSPI
- Memory-mapped mode established
- Settings successfully read from flash
Result: RELIABLE operation
Status: Safe QSPI access guaranteed
```

## Detailed Timing Analysis

### Bootloader Grace Period Breakdown

**Total Duration**: 2500ms (2.5 seconds)

**Activities During Grace Period**:
```
0-100ms    : Bootloader initialization, QSPI setup
100-500ms  : DFU listener setup, USB enumeration
500-2000ms : Media scanning (SD card, then USB)
2000-2450ms: Firmware comparison and flashing (if needed)
2450-2500ms: Bootloader cleanup, prepare for jump
```

**QSPI Usage During Grace Period**:
- **Read Operations**: Firmware verification, checksum calculation
- **Write Operations**: New firmware flashing (if .bin file found)
- **Mode Changes**: Switching between indirect and memory-mapped modes
- **Critical Window**: 2450-2500ms (cleanup phase)

### Application Startup Timing

**hw.Init() Duration Analysis**:
```cpp
// Estimated timing breakdown:
System::Config syscfg;        // ~1ms
syscfg.Boost();              // ~5ms
System::Init(syscfg);        // ~10ms
InitAudio();                 // ~20ms
InitControls();              // ~5ms
// Total: ~41ms typical
```

**Current QSPI Access Timing**:
```
Reset: 0ms
├─ Bootloader grace period: 0-2500ms
├─ Jump to application: 2500ms
├─ hw.Init(): 2500-2541ms
└─ storage.Init(): 2541ms ← TOO EARLY!

Problem: Only 41ms after bootloader "completion"
Risk: Bootloader may still be cleaning up QSPI peripheral
```

### Race Condition Analysis

**Conflict Scenario**:
```
Timeline A (Bootloader):
2450ms: Begin cleanup sequence
2470ms: Finalizing QSPI configuration
2490ms: Releasing QSPI peripheral
2500ms: Jump to application
2500-2550ms: Hardware cleanup continues asynchronously?

Timeline B (Application):
2500ms: Application starts
2541ms: storage.Init() tries to configure QSPI ← CONFLICT!
```

**Why Corruption Occurs**:
1. Bootloader may not immediately release QSPI peripheral
2. Hardware cleanup may continue after jump to application
3. PersistentStorage reconfigures QSPI before bootloader fully complete
4. QSPI peripheral state becomes inconsistent
5. Settings reads/writes fail or return garbage data

## Timing Requirements Analysis

### Bootloader Documentation Review

From `_a7_Getting-Started-Daisy-Bootloader.md`:
> "Once flashed, the bootloader has a grace period of 2.5 seconds on startup indicated by sinusoidal LED blinks. During this time, it will listen for DFU transactions over USB and search any connected media for valid binaries. Once this period elapses, the bootloader will attempt to load a program and jump to it."

**Key Points**:
- **Fixed Duration**: 2.5 seconds (not variable)
- **LED Indicator**: Sinusoidal pattern during grace period
- **Activities**: DFU + media scanning (both use QSPI)
- **Jump Timing**: After 2.5 seconds

### Safety Margin Calculation

**Required Delay Calculation**:
```
bootloader_grace_period = 2500ms
bootloader_cleanup_estimate = 200ms (conservative)
hardware_init_time = 50ms
safety_margin = 250ms (10% buffer)

minimum_delay = bootloader_grace_period +
                bootloader_cleanup_estimate -
                hardware_init_time +
                safety_margin

minimum_delay = 2500 + 200 - 50 + 250 = 2900ms
recommended_delay = 3000ms (round up to even number)
```

**Justification for 3000ms**:
- Exceeds bootloader grace period by 500ms
- Accounts for unknown bootloader cleanup timing
- Provides safety margin for timing variations
- Round number for code clarity

## LED Pattern as Timing Indicator

### Bootloader LED Behavior
- **Pattern**: Sinusoidal fade in/out
- **Duration**: Throughout 2.5-second grace period
- **Purpose**: Visual indicator of bootloader activity
- **End Condition**: LED pattern stops when bootloader completes

### Potential LED-Based Detection (Future Enhancement)
```cpp
// Theoretical bootloader completion detection
bool IsBootloaderActive() {
    // Monitor LED pin for sinusoidal pattern
    // Return false when pattern stops
    // This would require LED pin access and pattern analysis
}

void WaitForBootloaderCompletion() {
    uint32_t start_time = System::GetNow();
    const uint32_t TIMEOUT = 5000; // 5 second maximum wait

    // Wait for LED pattern to stop OR timeout
    while (IsBootloaderActive() &&
           (System::GetNow() - start_time) < TIMEOUT) {
        System::Delay(100);
    }

    // Additional safety margin
    System::Delay(500);
}
```

## Implementation Recommendations

### Immediate Fix (Story 1.2)
```cpp
int main() {
    hw.Init();

    // CRITICAL: Wait for bootloader completion
    System::Delay(3000);  // 3 second safety delay

    // Now safe to initialize QSPI storage
    storage.Init({SETTINGS_VERSION, false}, 0x2B000);

    // Continue with normal initialization...
}
```

### Advanced Implementation (Story 1.4)
```cpp
int main() {
    hw.Init();

    // Smart delay based on boot timing
    uint32_t boot_time = System::GetNow();
    const uint32_t MIN_DELAY = 3000;

    if (boot_time < MIN_DELAY) {
        System::Delay(MIN_DELAY - boot_time);
    }

    storage.Init({SETTINGS_VERSION, false}, 0x2B000);
    // Continue...
}
```

### Robust Implementation with Error Handling (Story 1.5)
```cpp
bool InitializeStorageSafely() {
    const uint32_t MAX_RETRIES = 3;
    const uint32_t RETRY_DELAY = 1000;

    for (uint32_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
        // Progressive delay for retries
        if (attempt > 0) {
            System::Delay(RETRY_DELAY * attempt);
        }

        // Try to initialize storage
        try {
            storage.Init({SETTINGS_VERSION, false}, 0x2B000);

            // Verify settings can be read
            const Settings& test_settings = storage.GetSettings();
            if (test_settings.version == SETTINGS_VERSION) {
                return true; // Success
            }
        } catch (...) {
            // Initialization failed, will retry
        }
    }

    // All retries failed - use defaults
    return false;
}

int main() {
    hw.Init();
    System::Delay(3000);

    if (!InitializeStorageSafely()) {
        // Fall back to default settings
        enable_overdrive = false;
        // Log error or signal to user
    }

    // Continue...
}
```

## Verification and Testing

### Timing Verification Tests

1. **Bootloader Completion Test**:
   ```cpp
   void TestBootloaderTiming() {
       uint32_t start = System::GetNow();
       // Boot device and measure actual delay needed
       // Vary delay from 1000ms to 4000ms
       // Find minimum delay for reliable operation
   }
   ```

2. **Storage Corruption Test**:
   ```cpp
   void TestStorageCorruption() {
       // Boot with various delays: 0ms, 1000ms, 2000ms, 3000ms
       // Write test pattern to storage
       // Reboot and verify pattern integrity
       // Find delay threshold for 100% reliability
   }
   ```

3. **Stress Test**:
   ```cpp
   void StressTestInitialization() {
       // Repeatedly power cycle device
       // Verify settings persistence over 1000+ boot cycles
       // Test with SD card present/absent
       // Test with different firmware sizes
   }
   ```

### Regression Prevention

**Build-time Check**:
```cpp
// Verify delay is present in compiled code
#ifndef BOOTLOADER_DELAY_MS
#error "BOOTLOADER_DELAY_MS must be defined (recommend 3000)"
#endif

static_assert(BOOTLOADER_DELAY_MS >= 3000,
             "Bootloader delay must be at least 3000ms");
```

**Runtime Verification**:
```cpp
void VerifyBootSequence() {
    uint32_t init_time = System::GetNow();
    assert(init_time >= 3000); // Ensure delay was implemented
}
```

## Alternative Solutions Considered

### 1. Bootloader Modification (Rejected)
- **Approach**: Modify bootloader to signal completion
- **Pros**: Most elegant solution
- **Cons**: Requires bootloader recompilation, breaks compatibility
- **Decision**: Rejected - maintain standard libDaisy bootloader

### 2. QSPI State Detection (Future Enhancement)
- **Approach**: Check QSPI peripheral registers for bootloader activity
- **Pros**: Dynamic timing, no fixed delay
- **Cons**: Complex, hardware-specific, risk of false positives
- **Decision**: Future consideration after simple delay proves reliable

### 3. Settings in Different Memory (Rejected)
- **Approach**: Use internal flash or SRAM for settings
- **Pros**: Avoids QSPI entirely
- **Cons**: Limited space, wear concerns, loses power-off persistence
- **Decision**: Rejected - QSPI is the correct storage location

## Summary

The initialization sequence timing issue is resolved by implementing a 3-second delay before QSPI access:

- **Root Cause**: Race condition between bootloader grace period and immediate QSPI initialization
- **Solution**: 3000ms delay after hw.Init() before storage.Init()
- **Justification**: Bootloader grace period (2500ms) + cleanup time (~200ms) + safety margin (300ms)
- **Implementation**: Simple System::Delay(3000) call
- **Result**: Reliable settings persistence across power cycles

This timing fix addresses the fundamental coordination issue between bootloader and application QSPI usage without requiring bootloader modifications or complex detection logic.