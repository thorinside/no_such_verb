# No Such Verb - User Stories & Epics

This directory contains user stories and epics for the No Such Verb firmware project.

## Available Epics

### Epic 1: Firmware State Persistence & Recovery

**Epic ID:** EPIC-001-STATE-PERSIST-v2
**Status:** Complete ✅
**Priority:** Critical (Blocker)
**Type:** Bug Fix Epic

#### Overview
The firmware currently loses operational state (button toggle, LED status) when powered down. Investigation revealed that the **bootloader's SD card loading mechanism interferes with QSPI state persistence**, preventing reliable state recovery. **This epic has been successfully completed.**

#### Epic Contains 6 Stories (All Complete)

1. ✅ **Story 1.1: Investigate Bootloader/QSPI Conflict** ([story-1.1.md](./story-1.1.md))
2. ✅ **Story 1.2: Implement Bootloader-Safe QSPI Initialization** ([story-1.2.md](./story-1.2.md))
3. ✅ **Story 1.3: Validate QSPI Memory Layout Safety** ([story-1.3.md](./story-1.3.md))
4. ✅ **Story 1.4: Fix LED State Synchronization** ([story-1.4.md](./story-1.4.md))
5. ✅ **Story 1.5: Implement Bootloader-Safe QSPI Write** ([story-1.5.md](./story-1.5.md))
6. ✅ **Story 1.6: Document Bootloader/QSPI Coordination** ([story-1.6.md](./story-1.6.md))

---

### Epic 2: Filter Randomization/AM LFO Control Remapping

**Epic ID:** EPIC-002-FILTER-AM-CONTROL-v1
**Status:** Draft
**Priority:** High (Enhancement)
**Type:** UI/UX Enhancement Epic

#### Overview
Improve the user interface control scheme by remapping how the existing filter randomization/amplitude modulation LFO effect is controlled. The toggle switch will be repurposed from controlling calculation order to enabling/disabling the modulation effect, with the overdrive gain knob serving dual purpose based on switch state.

#### Epic Contains 5 Stories

1. **Story 2.1: Decouple Toggle Switch from Calculation Order** ([story-2.1.md](./story-2.1.md))
2. **Story 2.2: Remap Toggle Switch to Filter Randomization/AM LFO** ([story-2.2.md](./story-2.2.md))
3. **Story 2.3: Remap Overdrive Knob to Control Modulation Intensity** ([story-2.3.md](./story-2.3.md))
4. **Story 2.4: Maintain Overdrive Button Functionality** ([story-2.4.md](./story-2.4.md))
5. **Story 2.5: Update User Documentation** ([story-2.5.md](./story-2.5.md))

#### Epic Documentation

- **Product Requirements**: See [../bmm-PRD.md](../bmm-PRD.md)
- **Epic Breakdown**: See [../bmm-epics.md](../bmm-epics.md) for full story details and acceptance criteria
- **Epic Details**: See [../prd/epic-2.md](../prd/epic-2.md) for complete epic specification

---

## Project Context

**Project:** No Such Verb (Daisy Patch.init())
**Language:** C++
**Framework:** Daisy SDK + DaisySP
**Build:** Makefile-based
**Key Issue:** Bootloader interferes with QSPI state persistence

See `/docs/architecture.md` and `/docs/development-guide.md` for technical context.

---

## Filing Issues

When stories reference bugs or blockers:
- Reference this epic ID: `EPIC-001-STATE-PERSIST-v2`
- Link to specific story number (Story 0-5)
- Include test case results

---

**Last Updated:** October 19, 2025
**Created By:** Bob (Scrum Master)
**Owner:** Neal (Product)
