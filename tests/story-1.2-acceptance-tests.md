# Story 1.2: Bootloader-Safe QSPI Initialization - Acceptance Tests

## Test Suite Overview

This test suite validates the implementation of bootloader-safe QSPI initialization as required by Story 1.2. All tests must be performed on physical Daisy Patch.init() hardware to validate timing and hardware interactions.

## Prerequisites

- Daisy Patch.init() hardware
- USB cable for serial debugging
- SD card with firmware.bin capability
- Audio input/output cables
- Serial monitor (115200 baud)
- Stopwatch for timing validation

## Test Environment Setup

1. **Hardware Setup:**
   - Connect Daisy Patch.init() via USB
   - Open serial monitor at 115200 baud
   - Prepare SD card for firmware loading tests

2. **Firmware Build:**
   ```bash
   make clean
   make
   ```
   - Verify build completes without errors
   - Verify binary size ≤ 120KB
   - Note actual binary size for documentation

---

## Test 1: QSPI Initialization Timing

**Test ID:** T1.2.001
**AC Reference:** Functional Requirements - QSPI initialization delayed until bootloader completes

### Setup
- Connect module via USB for serial debug output
- Ensure no saved settings exist (fresh flash or factory reset)

### Steps
1. Build and upload firmware with Story 1.2 changes
2. Power on module
3. Observe serial debug output immediately
4. Record timing sequence
5. Verify 3000ms delay occurs before QSPI initialization

### Expected Result
```
[Power-on]
[Hardware initialization]
[3000ms delay occurs - no QSPI access]
[QSPI initialization starts after delay]
[Settings load attempt]
[Audio processing begins]
```

### Pass Criteria
- ✅ QSPI init happens after 3000ms bootloader delay
- ✅ No QSPI corruption or error messages
- ✅ Boot sequence completes successfully
- ✅ Timing follows Story 1.1 recommendations

### Actual Results
- **Start Time:** ________________
- **QSPI Init Time:** ________________
- **Delay Duration:** ________________
- **Pass/Fail:** ________________
- **Notes:** ________________

---

## Test 2: Settings Recovery Without Corruption

**Test ID:** T1.2.002
**AC Reference:** Functional Requirements - Settings successfully read from QSPI without corruption

### Setup
- Module with saved settings (run Test 3 first if needed)
- Overdrive mode previously enabled and saved

### Steps
1. Ensure settings are saved (enable overdrive, trigger save)
2. Power off module completely
3. Wait 10 seconds
4. Power on module
5. Observe settings loading during boot
6. Check LED state matches loaded settings
7. Verify no corruption messages

### Expected Result
- QSPI read operation succeeds
- LED CV_OUT_2 reflects saved overdrive state (5.0V if enabled, 0.0V if disabled)
- No "QSPI read failed" or corruption messages
- Settings version matches expected value

### Pass Criteria
- ✅ Settings loaded correctly from QSPI
- ✅ LED state matches saved settings
- ✅ No error messages in serial output
- ✅ Settings version validation passes

### Actual Results
- **Overdrive State Loaded:** ________________
- **LED Output Voltage:** ________________
- **Error Messages:** ________________
- **Pass/Fail:** ________________
- **Notes:** ________________

---

## Test 3: Bootloader Still Works

**Test ID:** T1.2.003
**AC Reference:** Functional Requirements - Firmware still boots normally and loads firmware from SD

### Setup
- Module with SD card slot accessible
- Fresh firmware.bin compiled with Story 1.2 changes

### Steps
1. Compile firmware with Story 1.2 changes
2. Copy firmware.bin to root of SD card
3. Insert SD card into module
4. Power on module
5. Observe bootloader loading sequence
6. Verify firmware starts correctly after loading

### Expected Result
- Bootloader detects firmware.bin on SD card
- Bootloader loads firmware from SD to QSPI flash
- New firmware starts successfully
- No bootloader errors or timeouts
- 3000ms delay occurs before application QSPI access

### Pass Criteria
- ✅ Bootloader loads firmware from SD successfully
- ✅ Firmware boot sequence completes
- ✅ No bootloader timeout or loading errors
- ✅ QSPI delay still functions after SD loading

### Actual Results
- **Bootloader Load Time:** ________________
- **Firmware Start Success:** ________________
- **QSPI Delay Working:** ________________
- **Pass/Fail:** ________________
- **Notes:** ________________

---

## Test 4: No Audio Processing Issues

**Test ID:** T1.2.004
**AC Reference:** Functional Requirements - No changes to existing audio processing

### Setup
- Module with audio input connected
- Audio source (sine wave or music)
- Audio monitoring capability

### Steps
1. Build and upload firmware with Story 1.2 changes
2. Power on module and wait for boot completion
3. Connect audio input
4. Monitor audio output for ~30 seconds
5. Test all control inputs (knobs, buttons)
6. Listen for clicks, pops, or glitches
7. Verify effects processing works normally

### Expected Result
- Audio processes without artifacts during or after boot
- No dropout or glitches related to QSPI timing
- Control inputs respond normally
- Overdrive, reverb, and other effects work as expected
- AudioCallback function unchanged

### Pass Criteria
- ✅ Audio quality unchanged from previous version
- ✅ No boot-related audio artifacts
- ✅ All effects work normally
- ✅ Control responsiveness maintained

### Actual Results
- **Audio Quality:** ________________
- **Boot Artifacts Present:** ________________
- **Effects Functionality:** ________________
- **Pass/Fail:** ________________
- **Notes:** ________________

---

## Test 5: Multiple Power Cycles

**Test ID:** T1.2.005
**AC Reference:** Testing Requirements - Multiple power cycles for consistency

### Setup
- Module with QSPI delay implemented
- Serial monitor connected for timing observation

### Steps
1. Power on → observe startup sequence → record timing
2. Power off completely
3. Wait 5 seconds
4. Power on → repeat observation → record timing
5. Repeat steps 2-4 for total of 5 power cycles
6. Compare timing consistency across all cycles

### Expected Result
- Every startup follows same sequence
- 3000ms delay consistent across all cycles
- QSPI initialization timing stable
- No variations in boot behavior

### Pass Criteria
- ✅ All 5 cycles boot successfully
- ✅ Timing consistency within ±100ms
- ✅ No failed boots or corrupted states
- ✅ Identical behavior across power cycles

### Cycle Results
| Cycle | Boot Time | QSPI Init Time | Success | Notes |
|-------|-----------|---------------|---------|-------|
| 1     | _________ | _____________ | _______ | _____ |
| 2     | _________ | _____________ | _______ | _____ |
| 3     | _________ | _____________ | _______ | _____ |
| 4     | _________ | _____________ | _______ | _____ |
| 5     | _________ | _____________ | _______ | _____ |

**Overall Pass/Fail:** ________________

---

## Code Quality Tests

### Test 6: Build and Performance Validation

**Test ID:** T1.2.006
**AC Reference:** Code Quality - Build time, binary size, SRAM usage

### Steps
1. Clean build from scratch: `make clean && time make`
2. Check binary size: `ls -la build/*.bin`
3. Verify no compiler warnings or errors
4. Estimate SRAM usage from map file
5. Validate code style follows existing patterns

### Pass Criteria
- ✅ Build time < 15 seconds
- ✅ Binary size ≤ 120KB (currently 116KB baseline)
- ✅ No compiler errors or warnings
- ✅ SRAM usage < 90% (currently 82% baseline)

### Results
- **Build Time:** ________________
- **Binary Size:** ________________
- **Compiler Warnings:** ________________
- **Estimated SRAM Usage:** ________________
- **Pass/Fail:** ________________

---

## Test Environment Information

### Hardware Configuration
- **Device:** Daisy Patch.init()
- **Bootloader Version:** v6.3 (from Story 1.1)
- **QSPI Flash:** ________________
- **Test Date:** ________________

### Software Configuration
- **libDaisy Version:** ________________
- **Compiler Version:** ________________
- **Build Configuration:** ________________

### Test Execution Summary

| Test ID | Test Name | Status | Notes |
|---------|-----------|---------|--------|
| T1.2.001 | QSPI Timing | _______ | ______ |
| T1.2.002 | Settings Recovery | _______ | ______ |
| T1.2.003 | Bootloader Compatibility | _______ | ______ |
| T1.2.004 | Audio Processing | _______ | ______ |
| T1.2.005 | Power Cycle Consistency | _______ | ______ |
| T1.2.006 | Build & Performance | _______ | ______ |

**Overall Test Suite Status:** ________________
**Test Execution Date:** ________________
**Tester:** ________________
**Review Required:** Yes/No

---

## Troubleshooting Guide

### Common Issues and Solutions

**Issue: QSPI still fails after 3000ms delay**
- **Solution:** Increase delay in 500ms increments
- **Root Cause:** Bootloader taking longer than expected
- **Escalation:** Review Story 1.1 timing analysis

**Issue: Settings corrupt even with delay**
- **Solution:** Verify QSPI address doesn't overlap bootloader
- **Root Cause:** Memory layout conflict
- **Next Step:** Check address 0x2B000 usage

**Issue: Boot time too slow**
- **Solution:** This is expected trade-off for bootloader safety
- **Acceptable:** Up to 500ms additional delay
- **Critical:** If delay exceeds 5000ms, investigate

### Debug Serial Output Examples

**Normal Boot Sequence:**
```
[0ms] Hardware Init Complete
[3000ms] QSPI Init Starting
[3010ms] Settings Load Success
[3050ms] Audio Processing Ready
```

**Failed Boot Sequence:**
```
[0ms] Hardware Init Complete
[3000ms] QSPI Init Starting
[3010ms] ERROR: QSPI Read Failed
[3020ms] Using Default Settings
```

---

## Acceptance Criteria Validation

This test suite validates all Story 1.2 Acceptance Criteria:

### Functional Requirements
- [ ] QSPI initialization delayed until bootloader completes ← T1.2.001
- [ ] `storage.Init()` called only after delay ← T1.2.001
- [ ] Settings successfully read from QSPI without corruption ← T1.2.002
- [ ] No changes to existing audio processing ← T1.2.004
- [ ] Firmware still boots normally and loads from SD ← T1.2.003

### Code Quality
- [ ] Code compiles without errors or warnings ← T1.2.006
- [ ] Follows existing code style ← T1.2.006 (manual review)
- [ ] Comments explain bootloader coordination ← T1.2.006 (manual review)
- [ ] No memory leaks or resource conflicts ← T1.2.006

### Performance & Size
- [ ] Build time < 15 seconds ← T1.2.006
- [ ] Binary size remains ≤ 120KB ← T1.2.006
- [ ] SRAM usage < 90% ← T1.2.006
- [ ] Boot time minimal (delay is necessary, not excessive) ← T1.2.001

### Testing (Local)
- [ ] Bootloader still loads firmware from SD successfully ← T1.2.003
- [ ] QSPI initialization completes without errors ← T1.2.001, T1.2.002
- [ ] Serial debug output shows proper sequence ← T1.2.001
- [ ] Module initializes without crashes ← T1.2.005

**Test Suite Complete:** All acceptance criteria covered by test cases.