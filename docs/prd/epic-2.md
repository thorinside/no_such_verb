# Epic 2: Filter Randomization/AM LFO Control Remapping

**Epic ID:** EPIC-002-FILTER-AM-CONTROL-v1
**Status:** Draft
**Priority:** High (UI Enhancement)
**Target Sprint:** Sprint 2

## Overview

This epic focuses on improving the user interface control scheme for the No Such Verb module by remapping how the existing filter randomization/amplitude modulation LFO effect is controlled. The primary change involves repurposing the toggle switch from controlling calculation order to acting as an enable/disable gate for the modulation effect, with the overdrive gain knob serving dual purpose based on the switch state.

## Business Value

### User Benefits
- **Cleaner Overdrive Control**: Users can perform smooth overdrive sweeps without unwanted modulation artifacts when the toggle is off
- **Intuitive Modulation Control**: Direct on/off control for the modulation effect with variable intensity control
- **Improved Performance Workflow**: More logical grouping of related controls enhances live performance usability

### Technical Benefits
- **Better UI/UX**: More intuitive control mapping that groups related functionality
- **Code Simplification**: Removes less-used calculation order toggle in favor of more valuable feature control
- **Maintainability**: Cleaner separation of concerns between overdrive and modulation effects

## Technical Requirements

### Control Remapping
1. **Toggle Switch (B8)**
   - Current: Controls calculation order
   - New: Enable/disable filter randomization/AM LFO effect

2. **Overdrive Gain Knob**
   - When Toggle OFF: Controls only overdrive gain (clean sweeps)
   - When Toggle ON: Controls modulation intensity/depth (0-100%)

3. **Button B7**
   - Unchanged: Continue to control overdrive on/off

### Implementation Considerations
- Leverage existing filter randomization/AM LFO code
- Maintain state persistence using Epic 1 infrastructure
- Ensure smooth transitions without audio glitches
- Preserve all existing audio quality

## Success Criteria

- Toggle switch successfully controls modulation effect enable/disable
- Overdrive knob provides clean sweeps when modulation is disabled
- Overdrive knob controls modulation intensity when enabled
- No regression in audio quality or performance
- State properly persisted and recovered on power cycle
- Documentation updated to reflect new control scheme

## Risk Assessment

**Low Risk** - This is primarily a control remapping exercise using existing functionality:
- No new DSP algorithms required
- Existing modulation code already tested
- UI changes are straightforward button/knob remappings
- Can be easily reverted if issues arise

## Dependencies

- **Epic 1**: State persistence must be functional for settings to be retained
- **Existing Code**: Filter randomization/AM LFO implementation must be working

## Timeline

**Estimated Duration**: 1 week (5 stories)

### Story Breakdown
1. **Story 2.1**: Decouple toggle from calculation order (2 points)
2. **Story 2.2**: Remap toggle to modulation control (2-3 points)
3. **Story 2.3**: Implement dual-purpose overdrive knob (3 points)
4. **Story 2.4**: Validate overdrive button independence (2 points)
5. **Story 2.5**: Update documentation (1-2 points)

**Total Points**: 10-12 story points

## Acceptance Criteria

### Functional
- [ ] Toggle switch no longer affects calculation order
- [ ] Toggle switch enables/disables modulation effect
- [ ] Overdrive knob controls modulation intensity when toggle is ON
- [ ] Overdrive knob provides clean gain control when toggle is OFF
- [ ] Button B7 continues to control overdrive on/off
- [ ] All settings persist across power cycles

### Non-Functional
- [ ] No audio glitches during control changes
- [ ] Smooth parameter transitions
- [ ] No performance degradation
- [ ] Clear visual feedback (LED indicators if applicable)

### Documentation
- [ ] README updated with new control mappings
- [ ] User guide explains interaction between controls
- [ ] Version history documents changes
- [ ] Code comments explain dual-purpose logic

## Notes

- This epic improves usability without adding complexity to the codebase
- The existing modulation effect code remains unchanged
- Focus is on control mapping and user experience enhancement
- Consider future expansion for additional modulation parameters