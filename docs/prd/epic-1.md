# Epic 1: Firmware State Persistence & Recovery

**Epic ID:** EPIC-001-STATE-PERSIST-v2
**Status:** Draft
**Priority:** Critical (Blocker for live performance usage)
**Effort:** 10-15 story points
**Duration:** 1-3 weeks (with parallelization)

---

## Epic Goal

Enable the No Such Verb module to automatically restore its previous operational state upon power-on, including button toggle state and LED indicators, while resolving the bootloader/QSPI conflict that currently prevents reliable state persistence.

## Epic Value Proposition

**For Users:** Live performers can confidently power cycle their module between songs without losing effect configuration, enabling seamless integration into eurorack setups.

**For Development:** Establishes safe bootloader/QSPI coordination pattern for future persistent storage features and eliminates critical data corruption issue.

## Success Criteria

- [ ] Bootloader/QSPI conflict identified and documented
- [ ] QSPI initialization properly delayed (100ms after hw.Init())
- [ ] Settings persist reliably without corruption across 1000+ power cycles
- [ ] Button state recovered on every boot with 100% success rate
- [ ] LED synchronized with recovered state before audio starts
- [ ] Bootloader continues working without interference
- [ ] All code clean, documented, and maintainable

## Stories in Epic 1

### Story 1.1: Investigate Bootloader/QSPI Conflict

**As a** firmware developer,
**I want** to understand how the bootloader uses QSPI and memory resources,
**So that** I can design a safe state persistence mechanism.

**Story Points:** 2-3

### Story 1.2: Implement Bootloader-Safe QSPI Initialization

**As a** firmware developer,
**I want** to initialize QSPI only after the bootloader completes,
**So that** settings read/write operations don't conflict with bootloader SD loading.

**Story Points:** 2-3

### Story 1.3: Validate QSPI Memory Layout Safety

**As a** firmware developer,
**I want** to verify that PersistentStorage uses non-bootloader QSPI space,
**So that** settings don't overwrite bootloader or firmware code.

**Story Points:** 2

### Story 1.4: Fix LED State Synchronization

**As a** user operating No Such Verb,
**I want** the LED to reflect the recovered button state immediately after boot,
**So that** I can see the module's state as soon as it powers on.

**Story Points:** 1-2

### Story 1.5: Implement Bootloader-Safe QSPI Write

**As a** module operator,
**I want** settings saved safely to QSPI without corrupting data,
**So that** power failures during save don't break the module.

**Story Points:** 2-3

### Story 1.6: Document Bootloader/QSPI Coordination

**As a** future developer maintaining No Such Verb,
**I want** clear documentation of QSPI usage relative to bootloader,
**So that** future enhancements don't break state persistence.

**Story Points:** 1-2

## Story Dependencies

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

## Timeline

**Timeline with Parallelization:**
- Sequential (no parallelization): 2-3 weeks
- Optimal (max parallelization): 1-2 weeks
- Recommended: 1.5 weeks

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Bootloader findings inconclusive | Low | Critical | Escalate to Daisy support, research thoroughly |
| QSPI still corrupts after fix | Low | High | Add checksum validation (future epic) |
| Memory layout overlap | Low | High | Validate in Story 1.3 with linker analysis |
| Performance degradation | Very Low | Low | Already 2-3 hours boot latency acceptable |
| Bootloader breaks | Very Low | Critical | Extensive testing, have revert plan |

---

**Related Documentation:**
- PRD: `/docs/bmm-PRD.md`
- Epic Breakdown: `/docs/bmm-epics.md`
- Architecture: `/docs/architecture.md`
