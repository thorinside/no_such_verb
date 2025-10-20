# No Such Verb - Epic Breakdown

**Author:** Neal
**Date:** October 19, 2025
**Project Level:** Intermediate
**Target Scale:** Medium (Bug Fix Epic)

---

## Overview

This document provides the detailed epic breakdown for No Such Verb, expanding on the high-level epic list in the [bmm-PRD.md](./bmm-PRD.md).

Each epic includes:

- Expanded goal and value proposition
- Complete story breakdown with user stories
- Acceptance criteria for each story
- Story sequencing and dependencies

**Epic Sequencing Principles:**

- Epic 1 (EPIC-001) establishes foundational bootloader/QSPI coordination and state persistence
- Subsequent epics build on this foundation for preset management, MIDI integration, etc.
- Stories within epics are vertically sliced and sequentially ordered
- No forward dependencies - each story builds only on previous work

---

## EPIC-001: Firmware State Persistence & Recovery

**Epic ID:** EPIC-001-STATE-PERSIST-v2
**Status:** Draft
**Priority:** Critical (Blocker for live performance usage)
**Effort:** 10-15 story points
**Duration:** 1-3 weeks (with parallelization)

### Epic Goal

Enable the No Such Verb module to automatically restore its previous operational state upon power-on, including button toggle state and LED indicators, while resolving the bootloader/QSPI conflict that currently prevents reliable state persistence.

### Epic Value Proposition

**For Users:** Live performers can confidently power cycle their module between songs without losing effect configuration, enabling seamless integration into eurorack setups.

**For Development:** Establishes safe bootloader/QSPI coordination pattern for future persistent storage features and eliminates critical data corruption issue.

### Stories in EPIC-001

---

## Story 1.1: Investigate Bootloader/QSPI Conflict

**As a** firmware developer,
**I want** to understand how the bootloader uses QSPI and memory resources,
**So that** I can design a safe state persistence mechanism.

**Acceptance Criteria:**

1. Bootloader version, source location, and behavior documented
2. QSPI memory layout identified with specific addresses
   - Bootloader region (addresses and size)
   - Firmware region (addresses and size)
   - Safe application region (addresses and size)
3. Bootloader execution timeline documented with timing values
   - Bootloader start time
   - SD card loading phase duration
   - Bootloader completion point
   - When QSPI becomes safe for application
4. Minimum required delay before QSPI access determined (e.g., 100ms)
5. Root cause of state corruption identified and explained
6. Safe QSPI address range specified for PersistentStorage
7. Any SD card loading interference with QSPI documented

**Prerequisites:** None (first story)

**Story Points:** 2-3

---

## Story 1.2: Implement Bootloader-Safe QSPI Initialization

**As a** firmware developer,
**I want** to initialize QSPI only after the bootloader completes,
**So that** settings read/write operations don't conflict with bootloader SD loading.

**Acceptance Criteria:**

1. QSPI initialization delayed by recommended value from Story 1.1 (e.g., 100ms)
2. PersistentStorage initialized in safe QSPI region identified in Story 1.1
3. Settings struct successfully read from QSPI without corruption
4. Version field validated (mismatched versions trigger safe defaults)
5. Serial debug output shows proper initialization sequence
6. Bootloader still successfully loads firmware from SD
7. LED set to match recovered button state before AudioCallback starts

**Prerequisites:** Story 1.1 (investigation findings required)

**Story Points:** 2-3

---

## Story 1.3: Validate QSPI Memory Layout Safety

**As a** firmware developer,
**I want** to verify that PersistentStorage uses non-bootloader QSPI space,
**So that** settings don't overwrite bootloader or firmware code.

**Acceptance Criteria:**

1. Linker map file analyzed for section assignments
2. No overlap verified between bootloader, firmware, and PersistentStorage regions
3. All QSPI addresses match Story 1.1 findings
4. Memory layout documented in main.cpp with comments
5. Builds are reproducible across multiple clean compilations
6. Runtime QSPI storage address logged and verified
7. Bootloader continues to function with validated memory layout

**Prerequisites:** Stories 1.1, 1.2 (memory layout from Story 1.1, initialization from Story 1.2)

**Story Points:** 2

---

## Story 1.4: Fix LED State Synchronization

**As a** user operating No Such Verb,
**I want** the LED to reflect the recovered button state immediately after boot,
**So that** I can see the module's state as soon as it powers on.

**Acceptance Criteria:**

1. Button state successfully loaded from QSPI during main() initialization
2. LED (CV_OUT_2) output set correctly:
   - 5V when overdrive enabled
   - 0V when overdrive disabled
3. LED state synchronized before AudioCallback starts (no transient states)
4. No LED flicker during boot sequence
5. LED state correct even if QSPI load fails (safe default: OFF)
6. LED continues to sync with button toggles during operation
7. Multiple power cycles verify consistent behavior

**Prerequisites:** Stories 1.1-1.3 (QSPI must be functional)

**Story Points:** 1-2

---

## Story 1.5: Implement Bootloader-Safe QSPI Write

**As a** module operator,
**I want** settings saved safely to QSPI without corrupting data,
**So that** power failures during save don't break the module.

**Acceptance Criteria:**

1. QSPI write failures detected and logged via MIDI debug
2. Writes rate-limited to maximum 1 per 100ms (prevent QSPI wear)
3. Write operations are atomic (no partial data writes)
4. Invalid settings detected on load (version mismatch or checksum)
5. Corrupted settings trigger recovery to safe defaults
6. Module continues operating normally even if save fails
7. Bootloader SD loading not affected by QSPI write operations
8. Multiple rapid power cycles verify data integrity

**Prerequisites:** Stories 1.1-1.4 (QSPI infrastructure in place)

**Story Points:** 2-3

---

## Story 1.6: Document Bootloader/QSPI Coordination

**As a** future developer maintaining No Such Verb,
**I want** clear documentation of QSPI usage relative to bootloader,
**So that** future enhancements don't break state persistence.

**Acceptance Criteria:**

1. architecture.md updated with new "Bootloader & QSPI Coordination" section including:
   - Timing requirements for 100ms delay
   - QSPI memory layout with specific addresses
   - Rate-limiting strategy explanation
2. QSPI memory layout diagram created (ASCII or embedded)
3. Code comments in main.cpp explaining bootloader coordination
4. README.md updated with state persistence section
5. Troubleshooting guide created covering:
   - Settings not persisting
   - Settings corrupted after power loss
   - Bootloader fails after state persistence changes
   - LED state desynchronized
6. Future enhancement notes documented

**Prerequisites:** Stories 1.1-1.5 (implementation complete)

**Story Points:** 1-2

---

## Story Sequencing & Dependencies

### Critical Path

```
Story 1.1 (Investigation) [CRITICAL]
    ↓ (findings required by all others)

Story 1.2 (Safe Init) ────┐ [CRITICAL]
    ↓                     │
Story 1.3 (Validation)    │ [CRITICAL]
    ↓                     │
Stories 1.4 & 1.5 ────────┤ (can parallelize)
(LED Sync & Writes)       │
    ↓                     │
Story 1.6 (Documentation) │ [FINAL]
    ↓
Epic Complete
```

### Story Dependencies

| Story | Depends On | Can Start After |
|-------|-----------|-----------------|
| 1.1 | None | Immediately |
| 1.2 | 1.1 | Story 1.1 complete |
| 1.3 | 1.1, 1.2 | Stories 1.1 & 1.2 complete |
| 1.4 | 1.1, 1.2, 1.3 | Stories 1.1-1.3 complete |
| 1.5 | 1.1, 1.2, 1.3 | Stories 1.1-1.3 complete |
| 1.6 | 1.1-1.5 | Stories 1.1-1.5 complete |

### Parallelization Opportunities

- **Phase 1 (Sequential):** Story 1.1 (investigation) - 2-3 hours
- **Phase 2 (Sequential):** Stories 1.2 & 1.3 can run in parallel - 3-4 hours each
- **Phase 3 (Parallel):** Stories 1.4 & 1.5 can run in parallel - 2-3 hours each
- **Phase 4 (Final):** Story 1.6 (documentation) - 1-2 hours

**Timeline with Parallelization:**
- Sequential (no parallelization): 2-3 weeks
- Optimal (max parallelization): 1-2 weeks
- Recommended: 1.5 weeks

---

## Story Guidelines Reference

**Story Format:**

```
**Story [EPIC.N]: [Story Title]**

As a [user type],
I want [goal/desire],
So that [benefit/value].

**Acceptance Criteria:**
1. [Specific testable criterion]
2. [Another specific criterion]
3. [etc.]

**Prerequisites:** [Dependencies on previous stories, if any]
```

**Story Requirements:**

- **Vertical slices** - Complete, testable functionality delivery
- **Sequential ordering** - Logical progression within epic
- **No forward dependencies** - Only depend on previous work in same epic
- **AI-agent sized** - Completable in 2-4 hour focused session
- **Value-focused** - Integrate technical enablers into user-visible value

---

**For implementation:** Use the `create-story` workflow to generate individual story implementation plans from this epic breakdown.

**Document Status:** ✅ Ready for Development
**Next Step:** Assign Story 1.1 to firmware developer (critical path)

---

## EPIC-002: Filter Randomization/AM LFO Control Remapping

**Epic ID:** EPIC-002-FILTER-AM-CONTROL-v1
**Status:** Draft
**Priority:** High (UI Enhancement)
**Effort:** 6-10 story points
**Duration:** 1 week

### Epic Goal

Remap the existing UI controls to provide better access to the filter randomization/amplitude modulation LFO feature: the toggle switch will enable/disable the effect (instead of controlling calculation order), and the overdrive gain knob will control the effect intensity only when the switch is enabled, allowing for cleaner overdrive sweeps when the effect is disabled.

### Epic Value Proposition

**For Users:** Live performers gain intuitive control over the existing filter randomization/AM LFO effect, with the ability to perform clean overdrive sweeps without unwanted modulation, or introduce controlled modulation intensity when desired for creative sound design.

**For Development:** Improves UI/UX by repurposing the toggle switch from a less-used calculation order control to a more valuable effect gate, creating a more logical control scheme where related parameters are grouped together.

### Stories in EPIC-002

---

## Story 2.1: Decouple Toggle Switch from Calculation Order

**As a** firmware developer,
**I want** to remove the toggle switch's control over calculation order,
**So that** it can be repurposed for controlling the filter randomization/AM LFO effect.

**Acceptance Criteria:**

1. Toggle switch (B8) button handler disconnected from calculation order logic
2. Calculation order fixed to optimal default setting
3. No regression in audio processing quality
4. Existing overdrive functionality (B7) remains unchanged
5. Code refactored to remove or bypass calculation order toggle infrastructure
6. Serial debug confirms switch no longer affects calculation order
7. Audio output verified to remain consistent regardless of switch position

**Prerequisites:** Epic 1 complete (state persistence functional)

**Story Points:** 2

---

## Story 2.2: Remap Toggle Switch to Filter Randomization/AM LFO

**As a** module operator,
**I want** the toggle switch to enable or disable the existing filter randomization/AM LFO effect,
**So that** I have direct control over when modulation is applied to my signal.

**Acceptance Criteria:**

1. Toggle switch (B8) controls existing filter randomization/AM LFO enable state
2. When enabled, existing modulation effect becomes active
3. When disabled, signal passes through without modulation
4. State persisted to QSPI (using existing persistence from Epic 1)
5. LED indicator reflects modulation state:
   - Consider using existing LED or adding visual feedback
6. Toggle state recovered correctly on power cycle
7. No audio glitches when toggling during playback

**Prerequisites:** Story 2.1 (switch decoupled from calculation order)

**Story Points:** 2-3

---

## Story 2.3: Remap Overdrive Knob to Control Modulation Intensity

**As a** performer using No Such Verb,
**I want** the overdrive gain knob to control the filter randomization/AM LFO intensity when the toggle is on,
**So that** I can adjust the modulation depth while keeping overdrive sweeps clean when modulation is off.

**Acceptance Criteria:**

1. When toggle switch OFF:
   - Overdrive gain knob has no effect on modulation
   - Knob continues to control overdrive gain normally
   - Clean overdrive sweeps possible without modulation artifacts
2. When toggle switch ON:
   - Overdrive gain knob controls modulation intensity/depth (0-100%)
   - Modulation intensity scales linearly with knob position
   - Existing modulation parameters properly scaled
3. Smooth transitions when adjusting knob (no stepping artifacts)
4. Knob position correctly interpreted in both modes
5. Audio engine properly applies modulation based on intensity setting
6. No performance degradation from dual-purpose knob logic

**Prerequisites:** Story 2.2 (toggle switch remapped to modulation)

**Story Points:** 3

---

## Story 2.4: Maintain Overdrive Button Functionality

**As a** user,
**I want** button B7 to continue controlling overdrive on/off independently,
**So that** I can still enable/disable overdrive regardless of filter randomization state.

**Acceptance Criteria:**

1. Button B7 continues to toggle overdrive effect on/off
2. Overdrive state independent of filter randomization state
3. Both effects can be active simultaneously without conflicts
4. Overdrive gain still affected by knob position (when available)
5. LED indicators correctly show both states (if applicable)
6. State persistence includes both overdrive and randomization settings
7. Clean signal path when both effects are enabled

**Prerequisites:** Stories 2.1-2.3 (new control scheme in place)

**Story Points:** 2

---

## Story 2.5: Update User Documentation

**As a** future user of No Such Verb,
**I want** clear documentation of the new control scheme,
**So that** I understand how to use the filter randomization/AM LFO and overdrive features.

**Acceptance Criteria:**

1. README.md updated with new control mapping:
   - Toggle switch (B8): Filter randomization/AM LFO on/off
   - Overdrive knob: Dual purpose based on toggle state
   - Button B7: Overdrive on/off
2. User guide section created explaining:
   - How to achieve clean overdrive sweeps (toggle OFF)
   - How to introduce modulation effects (toggle ON)
   - Interaction between controls and dual-purpose knob
3. Patch examples documented showing common use cases
4. Troubleshooting section for new features
5. Version history updated with Epic 2 changes
6. Any MIDI documentation updated if applicable

**Prerequisites:** Stories 2.1-2.4 (implementation complete)

**Story Points:** 1-2

---

## Story Sequencing & Dependencies

### Critical Path

```
Story 2.1 (Decouple Switch) [CRITICAL]
    ↓
Story 2.2 (Filter Toggle) [CRITICAL]
    ↓
Story 2.3 (Knob Dual Purpose) [CRITICAL]
    ↓
Story 2.4 (Maintain B7) [VALIDATION]
    ↓
Story 2.5 (Documentation) [FINAL]
    ↓
Epic Complete
```

### Story Dependencies

| Story | Depends On | Can Start After |
|-------|-----------|-----------------|
| 2.1 | Epic 1 | Epic 1 complete |
| 2.2 | 2.1 | Story 2.1 complete |
| 2.3 | 2.2 | Story 2.2 complete |
| 2.4 | 2.1, 2.2, 2.3 | Stories 2.1-2.3 complete |
| 2.5 | 2.1-2.4 | Stories 2.1-2.4 complete |

### Parallelization Opportunities

- **Phase 1:** Story 2.1 (decouple) - 2-3 hours
- **Phase 2:** Story 2.2 (toggle implementation) - 3-4 hours
- **Phase 3:** Story 2.3 (knob dual purpose) - 3-4 hours
- **Phase 4:** Story 2.4 (validation) - 2 hours
- **Phase 5:** Story 2.5 (documentation) - 1-2 hours

**Timeline:**
- Sequential: 1.5-2 weeks
- Recommended: 1 week (focused development)
