# No Such Verb - Product Requirements Document (PRD)

**Author:** Neal
**Date:** October 19, 2025
**Project Level:** Intermediate
**Target Scale:** Medium (Experimental Audio Effects Module)

---

## Goals and Background Context

### Goals

**Primary Goal:** Fix critical firmware state persistence issue preventing the No Such Verb module from remembering operational state across power cycles, enabling live performance workflow integration.

**Objectives:**
1. Enable automatic recovery of button toggle state (overdrive on/off) on power-on
2. Synchronize LED indicators with recovered state immediately at boot
3. Resolve bootloader/QSPI conflict causing state corruption
4. Maintain bootloader functionality for SD card firmware loading
5. Implement reliable, rate-limited QSPI persistence mechanism
6. Document bootloader/QSPI coordination for future maintainers

### Background Context

The No Such Verb is an experimental audio effects pedal firmware for the Daisy Patch.init() platform, providing reverb, overdrive, and jitter modulation effects. Currently, the module loses all operational state when powered down, forcing users to manually reconfigure effects after every power cycle. This severely limits usability in live performance settings where modular gear needs to maintain state between songs.

**Root Cause:** Investigation revealed that the Daisy Patch.init() bootloader loads firmware from SD card during startup, temporarily holding QSPI resources. When the application attempts QSPI access before the bootloader completes, settings become corrupted or inaccessible, causing state recovery to fail silently.

**Current Impact:**
- Module unusable for live performance (can't maintain effect configuration)
- Settings not persisted despite partial QSPI implementation
- Every boot returns to default state
- LED state desynchronized from button state

**Success Criteria:**
- 100% state recovery rate on all power cycles
- Zero QSPI data corruption across 1000+ cycles
- LED accurately reflects button state immediately after boot
- Bootloader continues to load firmware from SD without interference
- Code is clean, well-documented, and maintainable

---

## Requirements

### Functional Requirements

1. **Investigate Bootloader/QSPI Conflict**
   - Analyze Daisy bootloader behavior and QSPI access patterns
   - Identify exact timing when QSPI becomes safe for application use
   - Document bootloader memory regions and any reservations
   - Determine minimum required delay before QSPI initialization
   - Specify safe QSPI address range for application data storage

2. **Implement Bootloader-Safe QSPI Initialization**
   - Add configurable delay in main() after hw.Init()
   - Initialize PersistentStorage only after bootloader completes
   - Load Settings struct from QSPI with version validation
   - Apply safe defaults if QSPI load fails or data is corrupted
   - Set LED to reflect loaded button state before audio processing starts

3. **Validate QSPI Memory Layout Safety**
   - Analyze linker map for PersistentStorage section assignment
   - Verify no overlap between bootloader, firmware, and storage regions
   - Document QSPI memory layout with specific addresses
   - Ensure builds are reproducible across compilations
   - Add comments in code explaining memory layout reasoning

4. **Fix LED State Synchronization**
   - Load button state from QSPI during initialization
   - Set CV_OUT_2 to 5V when overdrive enabled, 0V when disabled
   - Synchronize LED before AudioCallback starts (no transient states)
   - Provide safe fallback (LED off) if QSPI read fails
   - Maintain LED sync during runtime button toggles

5. **Implement Bootloader-Safe QSPI Write Operations**
   - Detect QSPI write failures and log via MIDI debug output
   - Rate-limit writes to maximum 1 per 100ms (prevent wear)
   - Ensure atomic writes (no partial data persistence)
   - Continue operation gracefully even if save fails
   - Trigger save only on state changes (button toggle)

6. **Document Bootloader/QSPI Coordination**
   - Update architecture.md with bootloader section and QSPI timing details
   - Create QSPI memory layout diagram with addresses and regions
   - Add code comments explaining why 100ms delay is necessary
   - Update README.md with state persistence information
   - Create troubleshooting guide for common persistence issues

### Non-Functional Requirements

**Performance:**
- Boot time: < 500ms (including bootloader delay)
- State recovery time: < 10ms (QSPI read)
- LED sync latency: < 5ms
- Audio processing: Zero degradation in quality or latency

**Reliability:**
- State persistence: 100% recovery rate on all cold boots
- Data integrity: Zero corruption across 1000+ power cycles
- Bootloader compatibility: Zero impact on firmware SD loading
- Error handling: Graceful degradation with safe defaults

**Resource Usage:**
- Binary size: ≤ 120KB (currently 116KB, +0-4KB acceptable)
- SRAM usage: < 90% (currently 82%, maintain headroom)
- QSPI usage: ≤ 2% (currently 1.5%)
- Build time: < 15 seconds

**Quality:**
- Compilation: Zero errors, zero warnings
- Code style: Follows existing conventions (lowercase_with_underscores)
- Testability: All code paths covered by acceptance tests
- Maintainability: Clear documentation for future enhancements

---

## User Journeys

### Journey 1: Live Performance Setup Recovery
**User:** Performance artist using No Such Verb in eurorack
**Timeline:**
1. Rehearsal: Configure overdrive ON, reverb feedback 0.7, other parameters
2. After song: Power off module for 10 minutes
3. Next song: Power on module
4. Expected: LED ON (matches previous overdrive), same settings recovered
5. Result: Module ready to play without reconfiguration

**Value:** Artists can confidently power cycle between songs without losing effect state

### Journey 2: Multi-Song Set
**User:** Live performer with 5-song set
1. Song 1: Configure effects, perform, power down
2. Song 2: Power on → settings recovered → perform
3. Song 3: Power on → settings recovered → perform
4. ...repeats for Songs 4-5
5. After set: All performances consistent in effect settings

**Value:** Entire set maintains consistent effect configuration without manual reconfiguration

### Journey 3: Bootloader Firmware Update
**User:** Developer updating firmware via SD card bootloader
1. Module powered on with previous settings saved in QSPI
2. Bootloader loads new firmware from SD
3. New firmware initializes (waits for bootloader to complete)
4. Result: Bootloader and firmware work together without conflict

**Value:** Firmware updates don't introduce QSPI corruption

---

## UX Design Principles

**Transparency:** State recovery happens silently and immediately - user sees effects restored without any intervention or feedback (except LED state)

**Reliability:** Module always boots to a valid state (either recovered or safe default)

**Non-Intrusive:** No UI changes or new controls needed - state persistence is implicit and automatic

**Consistency:** What you see on the LED is what you'll get (button state == LED state always)

---

## User Interface Design Goals

**LED State Feedback:**
- LED (CV_OUT_2) accurately reflects button state at all times
- ON (5V) when overdrive enabled
- OFF (0V) when overdrive disabled
- Immediate visual confirmation of recovered state on boot

**No New Controls:** State persistence is completely transparent - no new buttons, menus, or settings required

---

## Epic List

| Epic | Goal | Stories | Points | Status |
|------|------|---------|--------|--------|
| **EPIC-001: Firmware State Persistence & Recovery** | Enable automatic state recovery on power-on, resolve bootloader/QSPI conflict | 6 | 10-15 | Draft |

> **Note:** Detailed epic breakdown with full story specifications is available in [bmm-epics.md](./bmm-epics.md)

---

## Out of Scope

**Not in This Release:**
- Multi-preset system (save/recall multiple configurations)
- MIDI parameter recall over USB
- Settings export/import via USB or web interface
- Auto-save on timed intervals (only manual on button press)
- Parametric EQ (current tone control sufficient)
- Additional effect parameters beyond current 4 CV inputs
- Hardware preset buttons
- LCD/OLED display integration
- Wireless state synchronization

**Future Enhancements (Separate Epics):**
- Preset management system with naming and organization
- DAW integration via MIDI
- Advanced state versioning and migration
- Hardware preset recall buttons
- Extended parameter persistence (reverb size, decay, etc.)

**Known Limitations (Accepted):**
- Single state snapshot only (not multiple presets)
- No built-in UPS for power-loss protection (software graceful fallback only)
- QSPI wear not actively monitored (rate-limiting mitigates)
- Settings not encrypted (trusted environment assumption)

---

**Document Status:** ✅ Ready for Review
**Next Step:** Review and approve, then begin Epic EPIC-001 development
