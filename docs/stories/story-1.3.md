# Story 1.3: Validate QSPI Memory Layout Safety

Status: Ready for Development

## Story

As a firmware developer,
I want to verify that PersistentStorage uses non-bootloader QSPI space,
so that settings don't overwrite bootloader or firmware code.

---

## Context & Dependencies

### Prerequisites
- Story 001 complete (bootloader memory layout documented)
- Story 002 complete (QSPI initialization working)

### Why This Story Matters
If PersistentStorage uses QSPI regions reserved by the bootloader, writing settings will corrupt bootloader or firmware, causing device to become unusable or unbootable. This story verifies memory layout safety.

### Architecture Context
See `/docs/architecture.md` - Deployment Architecture section
See Story 001 findings for QSPI memory map

---

## Acceptance Criteria

- [ ] PersistentStorage allocation verified to NOT overlap bootloader space
- [ ] Linker script reviewed and no conflicting sections found
- [ ] Bootloader space clearly reserved/protected from application writes
- [ ] Memory layout documented in code comments with addresses
- [ ] Build process confirms safe memory allocation
- [ ] Verification performed multiple times (reproducible)
- [ ] Comments explain QSPI memory map and safe regions
- [ ] Linker map file reviewed and documented
- [ ] No hardcoded addresses (use defines instead)
- [ ] Clear separation of bootloader vs. application regions
- [ ] Memory map diagram updated with addresses
- [ ] Bootloader regions clearly marked as "reserved"
- [ ] Application storage region clearly marked as "safe"
- [ ] Comments in main.cpp reference memory layout
- [ ] Build succeeds and generates linker map
- [ ] Linker map analyzed for overlaps
- [ ] Runtime logging shows QSPI storage address
- [ ] Multiple builds confirm consistent memory layout

---

## Acceptance Tests

**Story Passes When All Tests Pass:**

### Test 1: Linker Map Verification
**Setup:** Build firmware and analyze linker output
**Steps:**
1. Execute: `make clean && make > build.log`
2. Examine build artifacts:
   - Check `build/no_such_verb.map` for section addresses
   - Search for "QSPI" and "flash" sections
   - Identify bootloader sections
   - Identify PersistentStorage sections
3. Compare addresses against Story 001 memory map

**Expected Result:**
- Bootloader sections: [addresses from Story 001]
- PersistentStorage section: Higher address (no overlap)
- Gap between sections: At least 4KB buffer (recommended)

**Pass Criteria:** No address overlap between bootloader and storage

---

### Test 2: Runtime Address Verification
**Setup:** Add debug logging to firmware
**Steps:**
1. Add code to log QSPI storage address at startup:
   ```cpp
   void main() {
       hw.Init();
       System::Delay(100);

       storage.Init();

       // Debug: Log QSPI storage location
       uint32_t storage_addr = (uint32_t)&storage;
       Serial.print("PersistentStorage at: 0x");
       Serial.println(storage_addr, HEX);

       // Continue...
   }
   ```
2. Upload and run firmware
3. Observe serial output
4. Compare address to Story 001 bootloader map

**Expected Result:**
- Storage address logged successfully
- Address is in "safe for application" range from Story 001
- Not in bootloader-reserved range
- Multiple boots show same address (reproducible)

**Pass Criteria:** Runtime address matches expected safe region

---

### Test 3: Linker Script Safety
**Setup:** Review build configuration
**Steps:**
1. Find linker script: `libDaisy/core/STM32H7.ld` (or similar)
2. Examine MEMORY sections:
   - Identify bootloader regions
   - Identify application regions
   - Verify QSPI section defined
3. Check no overlapping MEMORY definitions
4. Verify PersistentStorage uses QSPI, not FLASH

**Expected Result:**
- Clean memory region definitions
- No overlaps
- QSPI clearly allocated for application use
- Bootloader space protected

**Pass Criteria:** Linker script properly partitions QSPI memory

---

### Test 4: Build Consistency
**Setup:** Multiple clean builds
**Steps:**
1. `make clean`
2. `make` (first time)
3. Record build output, linker map, memory addresses
4. `make clean`
5. `make` (second time)
6. Compare output to first build
7. Repeat 2 more times (4 total builds)

**Expected Result:**
- All 4 builds identical
- Memory addresses unchanged
- No non-deterministic allocation
- Reproducible builds

**Pass Criteria:** Consistent memory layout across builds

---

### Test 5: Bootloader Functionality Preserved
**Setup:** With verified memory layout
**Steps:**
1. Build firmware with memory layout validated
2. Upload to module
3. Power cycle module 5 times
4. Observe bootloader loading firmware from SD each time
5. Check for bootloader errors or timeouts

**Expected Result:**
- Bootloader loads firmware successfully every time
- No timeouts or errors
- Consistent boot sequence
- No corruption despite QSPI writes

**Pass Criteria:** Bootloader continues working reliably

---

## Implementation Details

### Code Changes Required

**File: `main.cpp` - Add memory layout comments and logging**

```cpp
// ============================================================================
// QSPI Memory Layout (Safe for Application Storage)
// ============================================================================
// Based on bootloader analysis (Story 001):
//
// QSPI Physical Address Space (8MB):
//   0x90000000 - 0x901FFFFF: Bootloader (2MB - RESERVED)
//   0x90200000 - 0x907FFFFF: Firmware Code (6MB - RESERVED)
//   0x90800000 - 0x9080FFFF: PersistentStorage (64KB - SAFE FOR APP)
//   0x90810000 - 0x907FFFFF: Future expansion (available)
//
// PersistentStorage is allocated by libDaisy in safe region.
// No manual allocation needed, but address verified for safety.
// ============================================================================

void main() {
    hw.Init();
    System::Delay(100);  // Wait for bootloader

    storage.Init();

    // DEBUG: Verify QSPI storage location
    #ifdef DEBUG_BUILD
    uint32_t storage_addr = (uint32_t)(&storage);
    Serial.print("PersistentStorage location: 0x");
    Serial.println(storage_addr, HEX);

    // Expected: 0x90800000 (adjust if different from Story 001 findings)
    if (storage_addr >= 0x90800000 && storage_addr < 0x90810000) {
        Serial.println("✓ Storage in safe QSPI region");
    } else {
        Serial.println("✗ WARNING: Storage outside expected region!");
    }
    #endif

    // Continue with settings loading...
}
```

### Linker Map Analysis

Create a checklist file: `/docs/stories/linker-map-verification.md`

```markdown
# Linker Map Verification Checklist

## Expected Memory Regions (from Story 001)

- [ ] Bootloader space: [Address from Story 001]
- [ ] Firmware space: [Address from Story 001]
- [ ] QSPI storage: [Safe address from Story 001]

## Actual Build Artifacts

- [ ] build/no_such_verb.map reviewed
- [ ] QSPI sections identified
- [ ] Bootloader sections identified
- [ ] No overlap detected
- [ ] Gap > 4KB between regions

## Verification Results

- [ ] Linker map consistent with Story 001 findings
- [ ] PersistentStorage in correct region
- [ ] Build reproducible (multiple builds identical)
- [ ] Runtime address matches expected address
```

---

## Story Blockers & Constraints

### Constraint: Story 001 Findings Required
- Must have bootloader memory map from Story 001
- Must know expected QSPI address ranges
- Cannot verify without this information

### Constraint: No Code Changes to Critical Sections
- Don't modify bootloader loading code
- Don't modify linker script (verify only)
- Don't move QSPI regions (let libDaisy allocate)

### Constraint: Build Process Unchanged
- No changes to Makefile
- Build must still produce same binary size
- No new dependencies

---

## Testing Strategy

### Local Testing (Required)
1. Build firmware
2. Analyze linker map
3. Add debug logging
4. Upload and verify runtime address
5. Multiple clean builds to verify reproducibility

### Regression Testing
- Ensure all existing tests still pass
- Bootloader still works
- Audio still processes correctly
- Settings can be written/read

---

## Developer Notes

### Tools You'll Need
- `arm-none-eabi-readelf` (or similar) to read linker map
- `make` to build firmware
- Text editor to view linker map file
- Serial monitor to verify runtime address

### Linker Map Location
- After build: `build/no_such_verb.map`
- Look for sections like: `.qspi`, `.data`, `.text`, etc.
- Search for "MEMORY" region definitions
- Compare to bootloader expected ranges

### Things to Check
1. **MEMORY Regions** - Are QSPI regions properly partitioned?
2. **Section Assignments** - Is PersistentStorage assigned to QSPI?
3. **Address Ranges** - Do they match Story 001 findings?
4. **No Overlaps** - Is there separation between bootloader and app?
5. **Consistent** - Are builds reproducible?

### If Issues Found

**Issue:** Storage address outside expected range
- Check Story 001 findings are accurate
- May need to manually specify QSPI address in linker script
- Escalate if unclear

**Issue:** Bootloader and storage overlap
- Critical blocker - cannot proceed with story
- Contact Daisy support or investigate further
- This indicates Story 001 findings were incomplete

**Issue:** Build not reproducible
- Unusual situation
- Check for timestamps or build artifacts
- Try full clean build

---

## Code Review Checklist

**Before marking "Done", verify:**

- [ ] Linker map analyzed and documented
- [ ] No overlap between bootloader and storage
- [ ] Storage address in expected range
- [ ] Debug logging added to main.cpp
- [ ] Comments explain QSPI memory layout
- [ ] Build still succeeds (no new warnings)
- [ ] Binary size unchanged
- [ ] Runtime address matches expected value
- [ ] Multiple builds are reproducible
- [ ] Bootloader still functions correctly

---

## Story Review & Approval

### Review Checklist
- [x] Story depends on Story 001 and 002 (correctly marked)
- [x] Acceptance criteria are specific and testable
- [x] No code changes to critical sections
- [x] Testing strategy is clear
- [x] Blocker for memory corruption identified and addressed
- [x] Bootloader compatibility preserved

### Validation Results
✅ **VALIDATION PASSED**

- This is critical verification work
- No implementation risks identified
- Testing approach is sound
- Linker map analysis is straightforward

### Approval Status
**✅ APPROVED - READY FOR DEVELOPMENT**

This story can start as soon as Story 002 completes. It's a straightforward verification task with clear acceptance criteria.

---

## Estimated Timeline

**Story Duration:** 1.5-2 hours

- Linker map analysis: 30 min (read Story 001 findings, analyze build output)
- Add debug logging: 15 min (add comments and serial print)
- Testing: 30 min (build, upload, verify address)
- Verification and documentation: 15 min
- Buffer for issues: 15 min

---

## Success Indicators

**This story succeeds when:**

1. ✅ Linker map verified safe (no overlaps with bootloader)
2. ✅ Memory layout documented in code comments
3. ✅ Runtime address verified to match expected range
4. ✅ Builds are reproducible (consistent across multiple builds)
5. ✅ Bootloader continues working
6. ✅ Clear documentation for future reference

---

**Status:** ✅ READY FOR DEVELOPMENT
**Created By:** Bob (Scrum Master)
**Date Created:** October 19, 2025
**Depends On:** Story 001, Story 002
**Blocks:** Story 004, Story 005
