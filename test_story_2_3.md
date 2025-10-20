# Story 2.3 Test Validation Plan

## Test Overview
Manual validation of overdrive knob dual-purpose control functionality for modulation intensity when toggle switch is ON.

## Test Environment
- Hardware: Daisy Patch SM with No Such Verb firmware
- Audio Input: Sine wave generator (1kHz, -20dB)
- Audio Output: Oscilloscope or spectrum analyzer
- Controls: CV knob 1 (overdrive/modulation), Button B7 (overdrive enable), Toggle B8 (modulation enable)

## Test Cases

### AC1: Toggle Switch OFF - Clean Operation
**Objective:** Verify overdrive knob controls only overdrive gain and square noise when toggle is OFF.

**Setup:**
1. Toggle switch B8 OFF (filterModulationEnabled = false)
2. Button B7 ON (enable_overdrive = true)
3. Input: 1kHz sine wave

**Test Steps:**
1. Set knob to minimum position
2. Verify overdrive level at MIN_OVERDRIVE (0.1f)
3. Verify square noise present but minimal
4. Verify NO jitter modulation applied
5. Sweep knob from min to max
6. Verify overdrive scales from MIN_OVERDRIVE to MAX_OVERDRIVE
7. Verify square noise scales with knob
8. Verify modulation remains OFF throughout sweep

**Expected Results:**
- Overdrive effect present and scaling with knob
- Square noise present and scaling with knob
- No jitter/AM modulation applied
- Clean audio output without modulation artifacts

### AC2: Toggle Switch ON - Unified Control
**Objective:** Verify overdrive knob controls overdrive gain, square noise, AND modulation intensity when toggle is ON.

**Setup:**
1. Toggle switch B8 ON (filterModulationEnabled = true)
2. Button B7 ON (enable_overdrive = true)
3. Input: 1kHz sine wave

**Test Steps:**
1. Set knob to minimum position
2. Verify overdrive at MIN_OVERDRIVE
3. Verify square noise minimal
4. Verify jitter modulation at 0% intensity (no effect)
5. Sweep knob to 50% position
6. Verify overdrive at mid-range
7. Verify square noise at mid-level
8. Verify jitter modulation at 50% intensity (moderate effect)
9. Sweep knob to maximum position
10. Verify overdrive at MAX_OVERDRIVE
11. Verify square noise at maximum
12. Verify jitter modulation at 100% intensity (full effect)

**Expected Results:**
- All three effects scale together from single knob
- Modulation depth tracks with overdrive/noise levels
- Full range utilization (0% to 100% modulation)

### AC3: Smooth Transitions
**Objective:** Verify no stepping artifacts when adjusting knob.

**Setup:**
1. Toggle ON, overdrive ON
2. Audio monitoring with spectrum analyzer

**Test Steps:**
1. Slowly sweep knob from min to max over 5 seconds
2. Monitor for zipper noise or stepping
3. Rapidly move knob (fast sweeps)
4. Monitor for audio artifacts

**Expected Results:**
- Smooth parameter changes without zipper noise
- No audible stepping artifacts
- Clean transitions during rapid movements

### AC4: Mode Switching
**Objective:** Verify knob position correctly interpreted in both modes.

**Setup:**
1. Set knob to 75% position
2. Toggle ON, overdrive ON

**Test Steps:**
1. Note current effect levels (overdrive + noise + modulation)
2. Turn toggle OFF
3. Verify overdrive and noise maintain same levels
4. Verify modulation disappears
5. Turn toggle ON
6. Verify all effects return to previous levels

**Expected Results:**
- Knob position remembered across mode switches
- Overdrive/noise unaffected by toggle state
- Modulation appears/disappears based on toggle

### AC5: Audio Engine Integration
**Objective:** Verify audio engine properly applies modulation based on intensity.

**Setup:**
1. Toggle ON, overdrive ON
2. Knob at various positions

**Test Steps:**
1. Set knob to 25%, verify weak modulation
2. Set knob to 50%, verify moderate modulation
3. Set knob to 75%, verify strong modulation
4. Set knob to 100%, verify maximum modulation
5. Compare modulation depth at each setting

**Expected Results:**
- Modulation depth proportional to knob position
- Jitter effect visible in audio output
- Intensity scaling linear with knob movement

### AC6: Performance Validation
**Objective:** Verify no performance degradation from dual-purpose logic.

**Setup:**
1. Audio monitoring for dropouts
2. Test both modes

**Test Steps:**
1. Run audio for extended period (5 minutes) in each mode
2. Monitor for audio dropouts or glitches
3. Rapidly switch between modes while adjusting knob
4. Monitor CPU usage if possible

**Expected Results:**
- No audio dropouts or glitches
- Smooth operation in both modes
- No performance degradation

## Integration Tests

### Test 1: Complete Workflow
1. Start with all effects OFF
2. Enable overdrive → verify overdrive + noise only
3. Adjust knob → verify both scale together
4. Enable modulation → verify all three effects active
5. Adjust knob → verify all three scale together
6. Disable modulation → verify back to overdrive + noise only
7. Disable overdrive → verify clean signal

### Test 2: Edge Cases
1. Knob at 0% with modulation ON → no effects
2. Knob at 100% with modulation OFF → max overdrive/noise only
3. Rapid toggle switching while adjusting knob
4. Button/toggle pressed simultaneously

## Pass/Fail Criteria

**PASS Requirements:**
- All 6 acceptance criteria validated
- No audio artifacts or dropouts
- Smooth parameter control
- Correct mode behavior
- No performance regression

**FAIL Conditions:**
- Modulation active when toggle OFF
- Overdrive/noise affected by toggle state
- Audio artifacts during knob adjustment
- Incorrect parameter scaling
- Performance degradation

## Test Results Log

| Test Case | Date | Result | Notes |
|-----------|------|--------|--------|
| AC1 - Toggle OFF | | | |
| AC2 - Toggle ON | | | |
| AC3 - Smooth Transitions | | | |
| AC4 - Mode Switching | | | |
| AC5 - Audio Engine | | | |
| AC6 - Performance | | | |

## Implementation Notes
- Overdrive controlled by: `enable_overdrive` flag
- Square noise controlled by: `enable_overdrive` flag
- Modulation controlled by: `filterModulationEnabled` flag
- All scale from: `jitter_mix_level` (CV knob 1)
- Smoothing provided by: KnobOnePoleFilter (overdrive only)