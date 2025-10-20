# Story 1.4: Fix LED State Synchronization

Status: Ready for Development

## Story

As a user operating No Such Verb,
I want the LED to reflect the recovered button state immediately after boot,
so that I can see the module's state as soon as it powers on.

---

## Context & Background

After Stories 001-003 are complete, the module loads settings from QSPI successfully. However, the LED (CV_OUT_2) is not synchronized with the loaded button state at startup. This story fixes that synchronization.

---

## Acceptance Criteria

- [ ] Button state loaded from QSPI during `main()` initialization
- [ ] LED (CV_OUT_2) set to match loaded state before AudioCallback starts
- [ ] LED = 5V when overdrive enabled, 0V when disabled
- [ ] No LED flicker during boot
- [ ] LED state correct even if QSPI load fails (safe default: OFF)
- [ ] LED continues to sync with button toggles during operation

---

## Acceptance Tests

**Test 1: Power On with Overdrive Enabled**
- Setup: Module previously powered off with overdrive ON, settings saved
- Steps: Power on module
- Expected: LED immediately ON (5V), no flicker
- Pass: LED state matches saved state before audio starts

**Test 2: Power On with Overdrive Disabled**
- Setup: Module previously powered off with overdrive OFF
- Steps: Power on module
- Expected: LED immediately OFF (0V)
- Pass: Consistent with saved state

**Test 3: QSPI Failure Fallback**
- Setup: Corrupt QSPI settings
- Steps: Power on module
- Expected: LED OFF (safe default), module boots normally
- Pass: Doesn't crash, uses safe defaults

**Test 4: Button Toggle During Operation**
- Setup: Module running, LED synchronized
- Steps: Press button 20 times
- Expected: LED toggles immediately on every press
- Pass: LED always accurate to button state

---

## Implementation Details

**Code Change in `main()` (after Story 002):**

```cpp
void main() {
    hw.Init();
    System::Delay(100);

    storage.Init();

    // Load settings (from Story 002)
    Settings loaded_settings;
    if (storage.Read(loaded_settings)) {
        if (loaded_settings.version == SETTINGS_VERSION) {
            settings = loaded_settings;
            enable_overdrive = settings.is_overdrive_enabled;
        } else {
            enable_overdrive = false;  // Safe default
        }
    } else {
        enable_overdrive = false;  // Safe default
    }

    // NEW: Synchronize LED with loaded state BEFORE audio starts
    hw.WriteCvOut(CV_OUT_2, enable_overdrive ? 5.0f : 0.0f);

    // Configure and start audio
    hw.SetAudioBlockSize(48);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAudio(AudioCallback);
}
```

**No Changes to AudioCallback** - Button toggle already handles LED update

---

## Prerequisites

- Story 001 (Investigation) complete
- Story 002 (Bootloader-Safe QSPI Init) complete
- Story 003 (Validate QSPI Memory Layout) complete

---

## Testing & Review

✅ **VALIDATION PASSED** - Straightforward implementation
✅ **APPROVED - READY FOR DEVELOPMENT**

---

**Related Documentation:**
- Epic PRD: `/docs/bmm-PRD.md`
- Epic Breakdown: `/docs/bmm-epics.md`
- Architecture: `/docs/architecture.md`
