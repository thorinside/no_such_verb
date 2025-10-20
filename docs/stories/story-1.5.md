# Story 1.5: Implement Bootloader-Safe QSPI Write

Status: Ready for Development

## Story

As a module operator,
I want settings saved safely to QSPI without corrupting data,
so that power failures during save don't break the module.

---

## Context & Background

After Stories 001-004, the module can read settings from QSPI. This story implements safe writing to ensure settings are persisted reliably during operation without causing bootloader conflicts.

---

## Acceptance Criteria

- [ ] QSPI write failures detected and logged (via MIDI debug)
- [ ] Writes only occur after bootloader completion (already ensured by Story 002)
- [ ] Invalid settings detected on load (checksum or version mismatch)
- [ ] Corrupted settings trigger recovery to safe defaults
- [ ] Write operations are atomic (no partial writes)
- [ ] Rate-limiting prevents excessive QSPI wear (max 1x/100ms)
- [ ] No interference with bootloader during save operations

---

## Acceptance Tests

**Test 1: Successful Settings Save**
- Setup: Button pressed, overdrive toggled
- Steps: Enable overdrive (press button)
- Expected: Settings saved to QSPI within 100ms
- Pass: No "QSPI write failed" message, LED changes immediately

**Test 2: Save Rate Limiting**
- Setup: Rapid button toggles
- Steps: Press button 50 times as fast as possible
- Expected: Only final state persisted (not intermediate)
- Pass: QSPI writes throttled to ~1 per 100ms

**Test 3: Write Failure Handling**
- Setup: Force QSPI write to fail (or simulate via code)
- Steps: Toggle button, trigger save
- Expected: Error logged, module continues operating
- Pass: Graceful failure, no crash

**Test 4: Persist Across Power Cycle**
- Setup: Toggle overdrive, save settings
- Steps: Toggle overdrive 5 times, power off, power on
- Expected: Final overdrive state restored
- Pass: State persisted correctly through cycle

**Test 5: Multiple Rapid Power Cycles**
- Setup: Settings in QSPI
- Steps: Power on/off 10 times rapidly
- Expected: No corruption, bootloader still works
- Pass: Reliable across rapid cycles

---

## Implementation Details

**Code Change in AudioCallback:**

```cpp
static uint32_t last_save_time = 0;
bool save_settings = false;  // Set when button pressed

if (save_settings) {
    uint32_t now = System::GetNow();

    // Rate limit: max 1 save per 100ms
    if (now - last_save_time >= 100) {
        // Save settings to QSPI
        if (storage.Write(settings)) {
            // Successful save
            #ifdef DEBUG_BUILD
            Serial.println("Settings saved to QSPI");
            #endif
        } else {
            // Save failed - log error but continue
            Serial.println("ERROR: QSPI write failed");
        }

        save_settings = false;
        last_save_time = now;
    }
}
```

**In Button Handler:**

```cpp
if (button.Pressed() && !button_pressed) {
    button_pressed = true;
    enable_overdrive = !enable_overdrive;
    hw.WriteCvOut(CV_OUT_2, enable_overdrive ? 5.0f : 0.0f);

    // Update settings struct
    settings.is_overdrive_enabled = enable_overdrive;

    // Flag for save (will be throttled)
    save_settings = true;
}
```

---

## Prerequisites

- Story 001 (Investigation) complete
- Story 002 (Bootloader-Safe QSPI Init) complete
- Story 003 (Validate QSPI Memory Layout) complete

---

## Testing & Review

✅ **VALIDATION PASSED** - Clear rate-limiting strategy
✅ **APPROVED - READY FOR DEVELOPMENT**

---

**Related Documentation:**
- Epic PRD: `/docs/bmm-PRD.md`
- Epic Breakdown: `/docs/bmm-epics.md`
- Architecture: `/docs/architecture.md`
