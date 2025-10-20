# Story 2.5: Update User Documentation

Status: Ready

## Story

As a future user of No Such Verb,
I want clear documentation of the new control scheme,
So that I understand how to use the filter randomization/AM LFO and overdrive features.

## Acceptance Criteria

1. README.md updated with new control mapping:
   - Toggle switch (B8): Filter randomization/AM LFO on/off
   - Overdrive knob: Controls overdrive gain + square noise (always), plus modulation when enabled
   - Button B7: Overdrive on/off
2. User guide section created explaining:
   - How to achieve clean overdrive sweeps (toggle OFF)
   - How to introduce modulation effects (toggle ON)
   - Interaction between controls and dual-purpose knob
3. Patch examples documented showing common use cases
4. Troubleshooting section for new features
5. Version history updated with Epic 2 changes
6. Any MIDI documentation updated if applicable

## Tasks / Subtasks

- [ ] Task 1: Update README.md control mapping (AC: 1)
  - [ ] Locate current controls documentation section
  - [ ] Update B8 description from calculation order to modulation enable
  - [ ] Document dual-purpose nature of overdrive knob
  - [ ] Confirm B7 description still accurate
  - [ ] Add clear control summary table

- [ ] Task 2: Create user guide section (AC: 2)
  - [ ] Write "Getting Started" with new controls
  - [ ] Document clean overdrive sweep technique
  - [ ] Explain modulation effect and control
  - [ ] Describe knob behavior in each mode
  - [ ] Add tips for live performance use

- [ ] Task 3: Document patch examples (AC: 3)
  - [ ] Example 1: Clean overdrive/noise sweep (modulation OFF)
  - [ ] Example 2: Subtle modulation with overdrive and noise
  - [ ] Example 3: Heavy modulation with full intensity
  - [ ] Example 4: Switching effects during performance
  - [ ] Explain how all three parameters scale together

- [ ] Task 4: Create troubleshooting section (AC: 4)
  - [ ] "Overdrive knob not affecting gain" → Check modulation toggle
  - [ ] "No modulation effect" → Verify toggle and knob position
  - [ ] "Clicking sounds when switching" → Normal, minimal artifact
  - [ ] "Settings not persisting" → Check Epic 1 implementation
  - [ ] Common issues and solutions

- [ ] Task 5: Update version history (AC: 5)
  - [ ] Add Epic 2 entry with version number
  - [ ] List all control remapping changes
  - [ ] Note any breaking changes
  - [ ] Credit implementation date
  - [ ] Link to detailed epic documentation

- [ ] Task 6: Review and update technical docs (AC: 6)
  - [ ] Check for MIDI CC documentation
  - [ ] Update if MIDI controls affected
  - [ ] Verify architecture.md reflects changes
  - [ ] Update development-guide.md if needed
  - [ ] Cross-reference with Epic 1 docs

- [ ] Task 7: Final review and formatting (AC: All)
  - [ ] Ensure consistent terminology throughout
  - [ ] Check all cross-references and links
  - [ ] Verify markdown formatting
  - [ ] Test all code examples if any
  - [ ] Get user feedback on clarity

## Dev Notes

### Documentation Structure
```
README.md
├── Overview
├── Features
├── Controls (UPDATE)
│   ├── Buttons
│   ├── Knobs (UPDATE)
│   └── Toggle Switches (UPDATE)
├── Getting Started (UPDATE)
├── Patch Examples (NEW)
├── Troubleshooting (NEW/UPDATE)
├── Version History (UPDATE)
└── Technical Details
```

### Key Messaging Points
- Emphasize improved usability and unified control
- Highlight clean overdrive/noise sweep capability
- Explain that knob controls intensity of all active effects
- Square noise always paired with overdrive gain
- Modulation is an optional addition to the mix

### Documentation Style Guide
- Use clear, concise language
- Include visual aids if possible (diagrams, tables)
- Provide concrete examples
- Avoid technical jargon in user sections
- Keep technical details in separate section

### Version Numbering
Consider semantic versioning:
- If breaking change: Major version bump (2.0.0)
- If new feature only: Minor version bump (1.1.0)
- If just fixes: Patch version bump (1.0.1)

## Project Structure Notes

### Files to Update
- `/README.md` - Primary user documentation
- `/docs/user-guide.md` - If exists
- `/docs/development-guide.md` - Developer notes
- `/docs/architecture.md` - Technical changes
- `/CHANGELOG.md` - If exists

### Documentation Testing
- Have someone unfamiliar with changes review
- Test all instructions step-by-step
- Verify examples work as described
- Check for ambiguous language

### Cross-References
- Link to Epic 1 for state persistence
- Reference story documentation for details
- Point to architecture docs for technical info

## References

- [Source: docs/bmm-epics.md#Story-2.5]
- [Source: docs/prd/epic-2.md#Documentation]
- [Source: docs/stories/story-1.6.md] - Documentation patterns from Epic 1
- All Epic 2 stories for implementation details

## Dev Agent Record

### Context Reference

- [Story Context XML](./story-context-2.5.xml)

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
**Prerequisites:** Stories 2.1-2.4 (implementation complete)
**Story Points:** 1-2