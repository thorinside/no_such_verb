# Engineering Backlog

This backlog collects cross-cutting or future action items that emerge from reviews and planning.

Routing guidance:

- Use this file for non-urgent optimizations, refactors, or follow-ups that span multiple stories/epics.
- Must-fix items to ship a story belong in that story's `Tasks / Subtasks`.
- Same-epic improvements may also be captured under the epic Tech Spec `Post-Review Follow-ups` section.

| Date | Story | Epic | Type | Severity | Owner | Status | Notes |
| ---- | ----- | ---- | ---- | -------- | ----- | ------ | ----- |
| 2025-01-27 | 2.3 | 2 | Performance | Med | TBD | Open | Optimize modulation_intensity calculation outside audio loop when filterModulationEnabled is false |
| 2025-01-27 | 2.3 | 2 | TechDebt | Low | TBD | Open | Add parameter smoothing to modulation intensity for consistency with overdrive parameters |
| 2025-01-27 | 2.3 | 2 | TechDebt | Low | TBD | Open | Abstract left/right channel processing to reduce code duplication in main.cpp |