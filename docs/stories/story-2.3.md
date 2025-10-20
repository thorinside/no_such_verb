# Story 2.3: Remap Overdrive Knob to Control Modulation Intensity

Status: Ready

## Story

As a performer using No Such Verb,
I want the overdrive gain knob to additionally control the filter randomization/AM LFO intensity when the toggle is on,
So that I can adjust all effect intensities together while keeping modulation optional.

## Acceptance Criteria

1. When toggle switch OFF:
   - Overdrive gain knob controls overdrive gain and square noise
   - No modulation applied regardless of knob position
   - Clean overdrive/noise sweeps without modulation
2. When toggle switch ON:
   - Overdrive gain knob controls overdrive gain, square noise, AND modulation intensity
   - All three parameters scale together with knob position
   - Modulation intensity tracks with overdrive/noise levels
3. Smooth transitions when adjusting knob (no stepping artifacts)
4. Knob position correctly interpreted in both modes
5. Audio engine properly applies modulation based on intensity setting
6. No performance degradation from dual-purpose knob logic

## Tasks / Subtasks

- [ ] Task 1: Analyze current knob implementation (AC: 1, 2, 4)
  - [ ] Locate overdrive gain knob reading code
  - [ ] Identify knob value range and scaling
  - [ ] Map current overdrive gain application
  - [ ] Document knob update frequency and smoothing

- [ ] Task 2: Implement additional knob control for modulation (AC: 1, 2, 4)
  - [ ] Create modulation intensity variable (0.0 to 1.0)
  - [ ] Knob always controls overdrive gain and square noise
  - [ ] When toggle OFF: no modulation applied
  - [ ] When toggle ON: knob also controls modulation intensity
  - [ ] Scale all three parameters from single knob value

- [ ] Task 3: Connect knob to modulation parameters (AC: 2, 5)
  - [ ] Identify modulation depth/intensity parameters
  - [ ] Scale knob value to appropriate modulation range
  - [ ] Apply scaled value to modulation algorithm
  - [ ] Ensure full range utilization (0% to 100%)

- [ ] Task 4: Implement smooth transitions (AC: 3)
  - [ ] Add parameter smoothing/filtering if not present
  - [ ] Use exponential smoothing or low-pass filter
  - [ ] Prevent zipper noise from rapid changes
  - [ ] Test with fast knob movements

- [ ] Task 5: Maintain overdrive independence (AC: 1)
  - [ ] Ensure overdrive gain unaffected when modulation enabled
  - [ ] Preserve existing overdrive behavior when modulation off
  - [ ] Test overdrive sweeps with modulation disabled
  - [ ] Verify no cross-contamination between effects

- [ ] Task 6: Optimize performance (AC: 6)
  - [ ] Profile dual-purpose logic overhead
  - [ ] Minimize conditional checks in audio callback
  - [ ] Consider pre-computing mode-dependent values
  - [ ] Ensure no audio dropouts or CPU spikes

- [ ] Task 7: Testing and validation (AC: All)
  - [ ] Test knob in both modes (modulation on/off)
  - [ ] Verify smooth parameter changes
  - [ ] Test full range of knob positions
  - [ ] Check for audio artifacts or glitches
  - [ ] Verify overdrive remains clean when modulation off
  - [ ] Test rapid mode switching while turning knob
  - [ ] Document test results and measurements

## Dev Notes

### Implementation Approach
- The knob always controls overdrive gain and square noise
- When modulation enabled, it additionally controls modulation intensity
- All parameters scale together from the same knob value
- This creates a unified "intensity" control for all effects

### Code Locations
- Knob reading: Look for ADC or control input processing
- Overdrive gain application: Audio callback, overdrive processing section
- Modulation intensity: Where modulation depth is applied

### Parameter Mapping
- Knob typically returns 0.0 to 1.0 (or 0-4095 for raw ADC)
- Overdrive gain: May need specific scaling (e.g., 1.0 to 10.0)
- Square noise level: Scale appropriately for noise generation
- Modulation intensity: 0.0 to 1.0 for depth (only when enabled)
- All three parameters derived from single knob value
- Consider logarithmic vs linear scaling for musicality

### Smoothing Strategy
- One-pole filter: `smoothed = smoothed * 0.95 + new_value * 0.05`
- Adjust coefficient based on desired response time
- Balance between responsiveness and smooth operation

### Testing Standards
- Use sine wave input for clear modulation visibility
- Oscilloscope to verify modulation depth changes
- Spectrum analyzer to check for artifacts
- Subjective listening tests for musicality

## Project Structure Notes

### Files to Modify
- `main.cpp` - Knob reading and parameter routing
- Audio callback - Modulation intensity application
- Consider creating helper functions for clarity

### Code Organization
- Keep dual-purpose logic clean and readable
- Comment thoroughly due to non-obvious behavior
- Consider enum for knob modes if complexity grows

### Potential Enhancements (Future)
- Store separate knob positions per mode
- Visual feedback for current knob function
- MIDI CC mapping for external control

## References

- [Source: docs/bmm-epics.md#Story-2.3]
- [Source: docs/prd/epic-2.md#Control-Remapping]
- [Source: docs/stories/story-2.2.md] - Modulation enable implementation

## Dev Agent Record

### Context Reference

- [Story Context XML](./story-context-2.3.xml)

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
**Prerequisites:** Story 2.2 (toggle switch remapped to modulation)
**Story Points:** 3