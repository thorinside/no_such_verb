# Story 2.4: Maintain Overdrive Button Functionality

Status: Ready

## Story

As a user,
I want button B7 to continue controlling overdrive on/off independently,
So that I can still enable/disable overdrive regardless of filter randomization state.

## Acceptance Criteria

1. Button B7 continues to toggle overdrive effect on/off
2. Overdrive state independent of filter randomization state
3. Both effects can be active simultaneously without conflicts
4. Overdrive gain still affected by knob position (when available)
5. LED indicators correctly show both states (if applicable)
6. State persistence includes both overdrive and randomization settings
7. Clean signal path when both effects are enabled

## Tasks / Subtasks

- [ ] Task 1: Verify B7 button isolation (AC: 1, 2)
  - [ ] Confirm B7 handler unaffected by Epic 2 changes
  - [ ] Test overdrive toggle functionality
  - [ ] Verify no interference from B8 (modulation toggle)
  - [ ] Document current overdrive implementation

- [ ] Task 2: Test effect independence (AC: 2, 3)
  - [ ] Enable overdrive only - verify operation
  - [ ] Enable modulation only - verify operation
  - [ ] Enable both effects - verify no conflicts
  - [ ] Test all four combinations (both off/on, each alone)

- [ ] Task 3: Validate knob behavior with overdrive (AC: 4)
  - [ ] When modulation OFF: knob controls overdrive gain + square noise
  - [ ] When modulation ON: knob controls overdrive gain + square noise + modulation
  - [ ] Verify all parameters scale together appropriately
  - [ ] Document the unified intensity control model

- [ ] Task 4: Verify LED indicators (AC: 5)
  - [ ] Check overdrive LED (if separate from modulation)
  - [ ] Verify correct state display for both effects
  - [ ] Test LED states across all effect combinations
  - [ ] Ensure no LED conflicts or confusion

- [ ] Task 5: Validate state persistence (AC: 6)
  - [ ] Both effect states saved to QSPI
  - [ ] Both states correctly recovered on boot
  - [ ] Test various state combinations across power cycles
  - [ ] Verify Settings struct includes both flags

- [ ] Task 6: Audio path validation (AC: 7)
  - [ ] Test signal flow with both effects active
  - [ ] Check for proper effect ordering
  - [ ] Verify no signal degradation or artifacts
  - [ ] Measure CPU usage with both effects

- [ ] Task 7: Integration testing (AC: All)
  - [ ] Full system test with all Epic 2 changes
  - [ ] Regression testing of Epic 1 features
  - [ ] Performance testing under load
  - [ ] Edge case testing (rapid button presses, etc.)
  - [ ] Document any issues or limitations

## Dev Notes

### Implementation Approach
- This is primarily a validation and integration story
- Ensure Stories 2.1-2.3 haven't broken overdrive functionality
- May need minor adjustments for effect coordination

### Testing Focus
- Comprehensive testing of all control combinations
- Special attention to dual-effect scenarios
- Performance impact of running both effects

### Effect Signal Flow
```
Input → [Overdrive?] → [Modulation?] → Output
```
- Determine if this order is optimal
- Consider if effects should be parallel or serial
- Document the chosen architecture

### Knob Behavior Model
When considering the overdrive gain with dual-purpose knob:
- Option A: Overdrive uses last set gain when modulation active
- Option B: Overdrive uses default/fixed gain when modulation active
- Option C: Add separate gain control (future enhancement)

### Testing Standards
- Systematic testing of all 4 state combinations
- Audio quality assessment with spectrum analyzer
- CPU profiling with both effects active
- Latency measurements if applicable

## Project Structure Notes

### Integration Points
- Button handlers for B7 and B8
- Audio callback for effect processing
- Settings struct for state management
- LED control for status display

### Validation Checklist
- [ ] B7 toggles overdrive correctly
- [ ] B8 toggles modulation correctly
- [ ] Knob behaves as expected in all modes
- [ ] LEDs show correct states
- [ ] States persist across power cycles
- [ ] Audio quality maintained
- [ ] No performance degradation

### Risk Mitigation
- Keep overdrive and modulation code clearly separated
- Add debug output for state verification
- Consider adding diagnostic mode for troubleshooting

## References

- [Source: docs/bmm-epics.md#Story-2.4]
- [Source: docs/prd/epic-2.md#Control-Remapping]
- [Source: docs/stories/story-1.4.md] - LED state management
- [Source: docs/stories/story-2.1.md] - Initial decoupling
- [Source: docs/stories/story-2.2.md] - Modulation implementation
- [Source: docs/stories/story-2.3.md] - Dual-purpose knob

## Dev Agent Record

### Context Reference

- [Story Context XML](./story-context-2.4.xml)

### Agent Model Used

Claude Code (claude-opus-4-1-20250805)

### Debug Log References

<!-- Will be populated during implementation -->

### Completion Notes List

<!-- Will be populated during implementation -->

### File List

<!-- Will be populated with modified files during implementation -->

---

**Created:** October 20, 2025
**Epic:** EPIC-002-FILTER-AM-CONTROL-v1
**Prerequisites:** Stories 2.1-2.3 (new control scheme in place)
**Story Points:** 2