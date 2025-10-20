# No Such Verb - User Stories & Epics

This directory contains user stories and epics for the No Such Verb firmware project.

## Available Epics

### Epic: Firmware State Persistence & Recovery

**Epic ID:** EPIC-001-STATE-PERSIST-v2
**Status:** Draft
**Priority:** Critical (Blocker)
**Type:** Bug Fix Epic

#### Overview
The firmware currently loses operational state (button toggle, LED status) when powered down. Investigation revealed that the **bootloader's SD card loading mechanism interferes with QSPI state persistence**, preventing reliable state recovery.

#### Epic Contains 6 Stories

1. **Story 1.1: Investigate Bootloader/QSPI Conflict** ([story-1.1.md](./story-1.1.md))
2. **Story 1.2: Implement Bootloader-Safe QSPI Initialization** ([story-1.2.md](./story-1.2.md))
3. **Story 1.3: Validate QSPI Memory Layout Safety** ([story-1.3.md](./story-1.3.md))
4. **Story 1.4: Fix LED State Synchronization** ([story-1.4.md](./story-1.4.md))
5. **Story 1.5: Implement Bootloader-Safe QSPI Write** ([story-1.5.md](./story-1.5.md))
6. **Story 1.6: Document Bootloader/QSPI Coordination** ([story-1.6.md](./story-1.6.md))

#### Epic Documentation

- **Product Requirements**: See [../bmm-PRD.md](../bmm-PRD.md)
- **Epic Breakdown**: See [../bmm-epics.md](../bmm-epics.md) for full story details and acceptance criteria

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
