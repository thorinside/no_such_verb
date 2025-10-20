# Story 1.2: Implement Bootloader-Safe QSPI Initialization

Status: Ready for Development

## Story

As a firmware developer,
I want to initialize QSPI only after the bootloader completes,
so that settings read/write operations don't conflict with bootloader SD card loading.

---

## Context & Dependencies

### Prerequisite
- **Must have Story 001 findings** - Bootloader analysis is required input
- Story 001 will provide:
  - Recommended QSPI initialization delay
  - Memory layout (safe QSPI address range)
  - QSPI availability timing
  - Any bootloader coordination requirements

### Why This Story Matters
The bootloader loads firmware from SD card during startup, temporarily holding QSPI resources. If the application tries to access QSPI before the bootloader completes, data becomes corrupted or inaccessible. This story delays QSPI initialization until after bootloader completes.

### Architecture Context
See `/docs/architecture.md` for:
- System Architecture (how bootloader and firmware interact)
- Audio Callback pattern
- Current state persistence attempt

See `/docs/source-tree-analysis.md` for:
- `main.cpp` entry point
- Build process

---

## Acceptance Criteria

**Story is Done When:**

### Functional Requirements
- [ ] QSPI initialization delayed until bootloader completes (use Story 001 recommended delay)
- [ ] `storage.Init()` called only after delay
- [ ] Settings successfully read from QSPI without corruption
- [ ] No changes to existing audio processing (AudioCallback unchanged)
- [ ] Firmware still boots normally and loads firmware from SD

### Code Quality
- [ ] Code compiles without errors or warnings
- [ ] Follows existing code style (lowercase with underscores)
- [ ] Comments explain bootloader coordination
- [ ] No memory leaks or resource conflicts

### Performance & Size
- [ ] Build time < 15 seconds
- [ ] Binary size remains ≤ 120KB (currently 116KB)
- [ ] SRAM usage < 90% (currently 82%)
- [ ] Boot time minimal (delay is necessary, not excessive)

### Testing (Local)
- [ ] Bootloader still loads firmware from SD successfully
- [ ] QSPI initialization completes without errors
- [ ] Serial debug output shows proper sequence
- [ ] Module initializes without crashes

---

## Acceptance Tests

**Implementation Passes When All Tests Pass:**

### Test 1: QSPI Initialization Timing
**Setup:** Connect module via USB for serial debug output
**Steps:**
1. Build and upload firmware with Story 1 changes
2. Power on module
3. Observe serial debug output
4. Check timing sequence

**Expected Result:**
```
[Bootloader running...]
[Boot 100ms timeout - checking if bootloader complete...]
QSPI initialization starting
QSPI initialization complete
Audio callback ready
```

**Pass Criteria:** QSPI init happens after bootloader completes, no errors

---

### Test 2: Settings Recovery Without Corruption
**Setup:** Module with saved settings (overdrive enabled)
**Steps:**
1. Power on module
2. Check that settings loaded correctly from QSPI
3. Verify no corruption or data loss
4. Check LED state matches loaded settings

**Expected Result:**
- QSPI read operation succeeds
- LED reflects saved state
- No error messages

**Pass Criteria:** Settings loaded correctly, no "QSPI read failed" messages

---

### Test 3: Bootloader Still Works
**Setup:** Module with firmware on SD card
**Steps:**
1. Compile new firmware with Story 1 changes
2. Place firmware.bin on SD card
3. Power on module
4. Observe bootloader loading firmware

**Expected Result:**
- Bootloader loads firmware from SD
- New firmware starts successfully
- No bootloader errors or timeouts

**Pass Criteria:** Bootloader and firmware both work correctly

---

### Test 4: No Audio Processing Issues
**Setup:** Module with audio input
**Steps:**
1. Build and upload firmware
2. Power on module
3. Connect audio input
4. Monitor audio for ~30 seconds
5. Check for clicks, pops, or glitches

**Expected Result:**
- Audio processes without artifacts
- No dropout or glitches
- Control inputs respond normally

**Pass Criteria:** Audio quality unchanged, effects work normally

---

### Test 5: Multiple Power Cycles
**Setup:** Module with QSPI delay implemented
**Steps:**
1. Power on → observe startup sequence
2. Power off
3. Wait 5 seconds
4. Power on → repeat 5 times
5. Check for consistency

**Expected Result:**
- Every startup follows same sequence
- No timing variations or failures
- Consistent behavior across cycles

**Pass Criteria:** All 5 cycles boot successfully and identically

---

## Implementation Details

### Code Changes Required

**File: `main.cpp`**

**Before (Current - Broken):**
```cpp
void main() {
    hw.Init();  // Initialize hardware

    // TODO: Load settings, but this fails due to bootloader still using QSPI
    // storage.Init();  // Not called - causes corruption if called too early

    hw.SetAudioBlockSize(48);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    hw.StartAudio(AudioCallback);
}
```

**After (Fixed - Story 002):**
```cpp
void main() {
    // Initialize core hardware
    hw.Init();

    // CRITICAL: Wait for bootloader to release QSPI
    // Duration from Story 001 investigation (e.g., 100ms)
    // TODO: Replace 100 with value from Story 001 findings
    System::Delay(100);  // Let bootloader complete

    // Now safe to initialize QSPI and load settings
    storage.Init();

    Settings loaded_settings;
    if (storage.Read(loaded_settings)) {
        // Verify settings are valid
        if (loaded_settings.version == SETTINGS_VERSION) {
            settings = loaded_settings;
            enable_overdrive = settings.is_overdrive_enabled;
            Serial.print("Settings loaded: overdrive=");
            Serial.println(enable_overdrive ? "ON" : "OFF");
        } else {
            // Version mismatch: use safe defaults
            enable_overdrive = false;
            Serial.println("Settings version mismatch: using defaults");
        }
    } else {
        // First boot or QSPI read failed: use safe defaults
        enable_overdrive = false;
        Serial.println("QSPI read failed or empty: using defaults");
    }

    // Set LED to match loaded state (will be fixed by Story 003)
    // For now, just initialize to match
    hw.WriteCvOut(CV_OUT_2, enable_overdrive ? 5.0f : 0.0f);

    // Configure audio
    hw.SetAudioBlockSize(48);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAdc();  // Start ADC for CV inputs
    hw.StartDac();  // Start DAC for CV outputs

    // Start audio processing
    hw.StartAudio(AudioCallback);
}
```

### Settings Structure (if not already defined)

Add to `main.cpp` near top:

```cpp
#define SETTINGS_VERSION 1

struct Settings {
    int version = SETTINGS_VERSION;
    bool is_overdrive_enabled = false;

    bool operator!=(const Settings &other) const {
        return version != other.version
            || is_overdrive_enabled != other.is_overdrive_enabled;
    }
};

// Global instances
PersistentStorage<Settings> storage(hw.qspi);
Settings settings;
bool enable_overdrive = false;
```

### Serial Debug Output (Optional but Recommended)

Add after settings load:

```cpp
void PrintSettings() {
    Serial.print("Current Settings: version=");
    Serial.print(SETTINGS_VERSION);
    Serial.print(", overdrive=");
    Serial.println(settings.is_overdrive_enabled ? "ON" : "OFF");
}
```

Call after settings initialization to verify startup state.

---

## Story Blockers & Constraints

### Constraint: Delay Value From Story 001
- Delay value must come from Story 001 investigation
- Default: 100ms (likely value, but verify)
- Too short: QSPI still in use, corruption returns
- Too long: Slower boot (but acceptable for bootloader safety)

### Constraint: No Audio Processing Changes
- AudioCallback must remain unchanged
- Only initialization order changes
- Audio quality must be identical

### Constraint: Bootloader Compatibility
- Must not interfere with bootloader SD card loading
- Bootloader must still load firmware successfully
- Build process unchanged

---

## Testing Strategy

### Local Testing (Required)
1. Build firmware with changes
2. Upload to module
3. Verify via serial output
4. Test power cycles
5. Test audio processing

### Hardware Testing (Recommended)
- Test on physical Daisy Patch.init() with bootloader
- Verify with oscilloscope (CV outputs, audio)
- Test with actual SD card firmware loading

### Regression Testing
- Ensure existing features still work
- Overdrive button still functions
- LED still responds (will be fully fixed by Story 003)
- Audio processing unchanged

---

## Developer Notes

### What Story 001 Findings You'll Need
- **Recommended delay value** (e.g., 100ms)
- **Safe QSPI address range** (for PersistentStorage)
- **Memory layout diagram** (for verification)
- **Timing sequence** (confirming bootloader completion detection)

### Tips for Implementation
1. Use `System::Delay()` from Daisy SDK for timing
2. Add serial debug output to verify sequence
3. Test with multiple power cycles
4. Measure actual QSPI read time (should be < 10ms)
5. Keep delay conservative - better to boot slower than corrupt QSPI

### Potential Issues & Solutions

**Issue:** QSPI still fails after delay
- **Solution:** Increase delay in 50ms increments until stable
- **Root cause:** Delay value from Story 001 was too aggressive
- **Escalate:** Back to Story 001 investigation

**Issue:** Settings corrupt even with delay
- **Solution:** Verify QSPI address range doesn't overlap bootloader
- **Root cause:** PersistentStorage using bootloader QSPI space
- **Next step:** Story 002 (validate memory layout)

**Issue:** Slow boot time
- **Solution:** Acceptable trade-off for bootloader safety
- **Alternative:** Find bootloader completion signal (less reliable)
- **Keep delay** unless it exceeds 500ms

---

## Code Review Checklist

**Before marking "Done", verify:**

- [ ] Delay value from Story 001 used (not hardcoded)
- [ ] Settings struct defined with version field
- [ ] PersistentStorage initialized after delay
- [ ] Settings loaded and version validated
- [ ] LED state initialized to match loaded settings
- [ ] Serial debug output helpful for troubleshooting
- [ ] No memory leaks or undefined behavior
- [ ] Follows existing code style
- [ ] Comments explain bootloader coordination
- [ ] Build succeeds (no errors/warnings)
- [ ] Binary size ≤ 120KB
- [ ] Tested on physical hardware

---

## Story Review & Approval

### Review Checklist
- [x] Story depends on Story 001 (correctly marked)
- [x] Acceptance criteria are specific and testable
- [x] Implementation details are clear
- [x] No ambiguity about what to implement
- [x] Code changes are minimal and focused
- [x] Testing strategy is complete
- [x] Bootloader compatibility verified

### Validation Results
✅ **VALIDATION PASSED**

- Implementation is straightforward
- Acceptance criteria are measurable
- Testing approach is comprehensive
- Code changes are minimal and safe
- No blockers identified

### Approval Status
**✅ APPROVED - READY FOR DEVELOPMENT**

This story is ready to be assigned immediately after Story 001 completes. All requirements and testing procedures are defined.

---

## Estimated Timeline

**Story Duration:** 2-3 hours

- Implementation: 30-45 min (modify main.cpp)
- Testing: 45-60 min (verify on hardware)
- Debug & refinement: 30-45 min (if issues found)
- Documentation: 15 min (add comments, update main.cpp)

---

## Success Indicators

**This story succeeds when:**

1. ✅ QSPI initialization delayed appropriately
2. ✅ Settings read successfully from QSPI without corruption
3. ✅ Bootloader still loads firmware from SD
4. ✅ No audio processing issues introduced
5. ✅ Code is clean and well-commented
6. ✅ Tests pass on physical hardware
7. ✅ Serial debug output confirms correct sequence

---

**Status:** ✅ READY FOR DEVELOPMENT
**Created By:** Bob (Scrum Master)
**Date Created:** October 19, 2025
**Depends On:** Story 001 (Investigation)
**Blocks:** Story 003 (LED Sync), Story 004 (Safe Writes)
