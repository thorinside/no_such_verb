# Story 1.6: Document Bootloader/QSPI Coordination

Status: Ready for Development

## Story

As a future developer maintaining No Such Verb,
I want clear documentation of QSPI usage relative to bootloader,
so that future enhancements don't break state persistence.

---

## Context & Background

After Stories 1.1-1.5 are complete and tested, this story ensures the work is documented for future maintainers. This prevents accidental breakage when new features are added.

---

## Acceptance Criteria

- [ ] `docs/architecture.md` updated with bootloader/QSPI section
- [ ] QSPI memory layout diagram in documentation
- [ ] Code comments explain bootloader coordination timing
- [ ] README includes QSPI state persistence information
- [ ] Troubleshooting section addresses common issues
- [ ] Clear explanation of why 100ms delay is necessary (from Story 1.1)
- [ ] Safe memory address ranges documented
- [ ] Rate-limiting strategy explained

---

## Deliverables

### 1. Update `/docs/architecture.md`

Add new section after "Deployment Architecture":

```markdown
## Bootloader & QSPI Coordination

### Critical Timing: 100ms Bootloader Delay

The Daisy Patch.init() bootloader loads firmware from SD card during startup.
This process temporarily holds QSPI resources. To prevent state corruption:

1. Application must delay 100ms after hw.Init()
2. This allows bootloader to complete and release QSPI
3. Only then is PersistentStorage safe to initialize
4. See Story 1.1 investigation for detailed timing analysis

### QSPI Memory Layout

- Bootloader region: 0x90000000-0x901FFFFF (reserved)
- Firmware region: 0x90200000-0x907FFFFF (reserved)
- PersistentStorage: 0x90800000-0x9080FFFF (safe for application)

### Rate-Limited Writes

Settings saved to QSPI with 100ms throttling:
- Max 1 write per 100ms prevents QSPI wear
- Ensures reliability across power cycles
- Safe during bootloader operations

### Future Enhancements

If adding new QSPI features:
1. Verify memory layout (don't overlap bootloader)
2. Maintain 100ms delay in main()
3. Rate-limit any new writes
4. Test with bootloader/SD loading
```

### 2. Create `/docs/qspi-memory-layout.md`

```markdown
# QSPI Memory Layout Diagram

## Physical Memory Map

```
QSPI Flash (8MB): 0x90000000 - 0x97FFFFFF

┌─────────────────────────────────────────┐
│ Bootloader Region (2MB)                 │
│ 0x90000000 - 0x901FFFFF (RESERVED)     │
│ DO NOT USE - bootloader critical        │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ Firmware Code (6MB)                     │
│ 0x90200000 - 0x907FFFFF (RESERVED)     │
│ Loaded from SD card by bootloader       │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ PersistentStorage (64KB) - SAFE         │
│ 0x90800000 - 0x9080FFFF                │
│ Application persistent settings here    │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ Future Application Data                 │
│ 0x90810000 - 0x97FFFFFF                │
│ Available for expansion                 │
└─────────────────────────────────────────┘
```

## Key Rules

1. **Never write to bootloader region** (0x90000000-0x901FFFFF)
2. **Firmware code is read-only** (0x90200000-0x907FFFFF)
3. **PersistentStorage is application territory** (0x90800000-0x9080FFFF)
4. **Always delay 100ms before QSPI init** - Wait for bootloader to complete

## Memory Verification

Use `build/no_such_verb.map` to verify:
- Linker script assigns regions correctly
- No overlap between bootloader and application
- PersistentStorage in safe address range
```

### 3. Update `/docs/development-guide.md`

Add troubleshooting section:

```markdown
## Troubleshooting State Persistence

### Issue: Settings Lost After Power Cycle

**Symptoms:**
- LED doesn't match previous state
- Button state not recovered
- Module always starts with defaults

**Cause:** QSPI initialization timing or memory conflict

**Solution:**
1. Verify 100ms delay in main() after hw.Init()
2. Check Story 1.1 findings on bootloader timing
3. Verify QSPI address range (should be 0x90800000+)
4. Inspect `build/no_such_verb.map` for overlaps

### Issue: Settings Corruption on Power Loss

**Symptoms:**
- Occasional data corruption
- Settings invalid after power failure
- Module boots with wrong state

**Cause:** Incomplete QSPI write during power loss

**Solution:**
- This is expected without UPS power
- Add checksum validation (future enhancement)
- Implement atomic writes (future enhancement)

### Issue: Bootloader Fails After Persistence Change

**Symptoms:**
- Bootloader hangs or fails to load
- Module doesn't respond
- SD card not recognized

**Cause:** QSPI memory layout conflict

**Solution:**
1. Verify memory layout per Story 1.3
2. Ensure no overlap with bootloader regions
3. Review linker script changes
4. Check if QSPI address changed accidentally
```

### 4. Update `/README.md`

Add section:

```markdown
## State Persistence

The No Such Verb module automatically saves the overdrive button state to QSPI flash.
When powered on, it recovers the previous state automatically.

**Important:** The bootloader loads firmware from SD card. This process requires
careful QSPI coordination to prevent data corruption. See docs/architecture.md
for details on bootloader/QSPI timing.
```

---

## Acceptance Tests

**Test 1: Documentation Complete**
- [ ] All mentioned files updated
- [ ] Sections added to architecture.md
- [ ] Memory layout diagram clear and accurate
- [ ] Troubleshooting section covers common issues

**Test 2: Documentation Accurate**
- [ ] All memory addresses match Story 1.1 findings
- [ ] Timing values match implementation (100ms)
- [ ] No contradictions or ambiguities
- [ ] Code examples are correct

**Test 3: Future-Proof**
- [ ] Future developers can understand QSPI usage
- [ ] Clear rules for avoiding corruption
- [ ] Troubleshooting section helps resolve issues
- [ ] Memory layout prevents accidental changes

---

## Prerequisites

- Story 1.1 (Investigation) complete
- Story 1.2 (Bootloader-Safe QSPI Init) complete
- Story 1.3 (Validate QSPI Memory Layout) complete
- Story 1.4 (LED State Sync) complete
- Story 1.5 (Safe QSPI Write) complete

---

## Testing & Review

✅ **VALIDATION PASSED** - Documentation is essential
✅ **APPROVED - READY FOR DEVELOPMENT**

---

**Related Documentation:**
- Epic PRD: `/docs/bmm-PRD.md`
- Epic Breakdown: `/docs/bmm-epics.md`
- Architecture: `/docs/architecture.md`
