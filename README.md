# Reverb for Patch.init()

This is a combination of a few DaisySP algorithms and integration with the Patch.init() hardware. Inspired by Benjie Jiao's MiniVerb and taking it into a slightly darker dimension with some added square noise and filtering.

Controls:

CV1/5 - Send Level (dry/wet mix)
CV2/6 - Overdrive/Modulation Intensity (dual-purpose knob)
CV3/7 - Feedback/HP Filter
CV4/8 - Reverb LP Filter

Button (B7) - Overdrive on/off
Toggle (B8) - Filter Modulation/AM LFO enable

**Control Interactions:**
- **CV2 Knob** serves dual purposes:
  - **When Overdrive enabled**: Controls overdrive gain + square noise intensity
  - **When Modulation enabled**: Also controls AM LFO modulation intensity
  - All three effects (overdrive, noise, modulation) scale together with this knob
- **LED Indicator**: ON when overdrive OR filter modulation is enabled, OFF when both disabled

Audio In and Out per usual

## State Persistence

The No Such Verb module automatically saves both overdrive and modulation states to SD card storage. When powered on, it recovers the previous states automatically, so the LED and effect settings match what you had before powering off.

**Important:** The Daisy Patch.init() bootloader loads firmware from SD card during startup. This process requires careful QSPI coordination to prevent data corruption. The firmware implements a 3000ms delay after hardware initialization to allow the bootloader to complete before accessing QSPI storage.

For technical details on bootloader/QSPI timing and memory layout, see [docs/architecture.md](docs/architecture.md#bootloader--qspi-coordination).

## Getting Started

### Basic Operation
1. **Connect your audio source** to the input and your speakers/headphones to the output
2. **Set initial levels**: Start with CV1 (Send Level) at 50% for a balanced dry/wet mix
3. **Adjust reverb character**: Use CV3 (Feedback) and CV4 (LP Filter) to shape the reverb tail

### Using the New Control Scheme

#### Clean Overdrive/Noise Sweeps (Modulation OFF)
1. **Toggle B8 OFF** (LED will reflect overdrive-only state)
2. **Press Button B7** to enable overdrive (LED turns on)
3. **Turn CV2 knob** to control overdrive gain and square noise intensity together
   - Low settings: Subtle warmth with minimal noise
   - High settings: Heavy overdrive with prominent square wave texture

#### Adding Modulation Effects (Modulation ON)
1. **Toggle B8 ON** to enable filter modulation/AM LFO (LED turns on if not already)
2. **Adjust CV2 knob** to control modulation intensity
   - The same knob now controls THREE effects simultaneously:
     - Overdrive gain (if overdrive enabled)
     - Square noise level (if overdrive enabled)
     - AM LFO modulation depth
3. **Live Performance Tip**: You can switch B8 on/off during performance for dynamic effect changes

#### Understanding the Dual-Purpose Knob
The CV2 knob is the **intensity control** for all active effects:
- **Overdrive OFF, Modulation OFF**: Knob has no effect
- **Overdrive ON, Modulation OFF**: Knob controls overdrive + noise only
- **Overdrive OFF, Modulation ON**: Knob controls modulation only
- **Both ON**: Knob controls all three effects with unified intensity

**Pro Tip**: This unified control makes it easy to dial in the perfect balance between clean reverb, gritty overdrive, and dynamic modulation with just one knob movement.

## Patch Examples

### Example 1: Clean Overdrive/Noise Sweep (Modulation OFF)
**Perfect for**: Subtle warmth to gritty textures without modulation artifacts
- **B7 (Overdrive)**: ON (LED should be on)
- **B8 (Modulation)**: OFF
- **CV1 (Send)**: 40-60% (adjust dry/wet to taste)
- **CV2 (Intensity)**: Start at 20% and sweep up to 80%
- **CV3 (Feedback)**: 50-70% for medium reverb tail
- **CV4 (LP Filter)**: 60-80% to tame harsh frequencies
- **Result**: Smooth transition from clean reverb to warm overdrive with increasing square noise texture

### Example 2: Subtle Modulation with Overdrive and Noise
**Perfect for**: Adding movement to reverb tails with controlled grit
- **B7 (Overdrive)**: ON
- **B8 (Modulation)**: ON (LED on, both effects active)
- **CV1 (Send)**: 50-70% (more wet signal for modulation effect)
- **CV2 (Intensity)**: 30-50% (moderate level for all three effects)
- **CV3 (Feedback)**: 60-80% (longer tails show modulation better)
- **CV4 (LP Filter)**: 40-60% (let some brightness through for clarity)
- **Result**: Warm overdrive with gentle square noise and subtle AM LFO creating breathing reverb tails

### Example 3: Heavy Modulation with Full Intensity
**Perfect for**: Dramatic effects, sound design, aggressive textures
- **B7 (Overdrive)**: ON
- **B8 (Modulation)**: ON
- **CV1 (Send)**: 70-90% (heavily processed signal)
- **CV2 (Intensity)**: 70-100% (maximum effect intensity)
- **CV3 (Feedback)**: 80-95% (long, heavily processed tails)
- **CV4 (LP Filter)**: 30-50% (darker tone to control harshness)
- **Result**: Heavily overdriven signal with prominent square noise and strong AM LFO modulation creating rhythmic, aggressive textures

### Example 4: Switching Effects During Performance
**Perfect for**: Live performance dynamics, building intensity
- **Starting position**: B7 OFF, B8 OFF (clean reverb only)
- **CV2**: Set to desired intensity level (50-70%)
- **Performance sequence**:
  1. Start clean: Both toggles OFF
  2. Add overdrive: Press B7 (instant grit and noise at preset intensity)
  3. Add modulation: Toggle B8 ON (adds breathing AM LFO without changing overdrive)
  4. Remove overdrive: Press B7 OFF (keeps modulation, removes grit)
  5. Back to clean: Toggle B8 OFF (pure reverb)
- **Pro Tip**: Set your CV2 intensity before the performance so each effect toggle gives consistent results

### How All Three Parameters Scale Together
When both overdrive and modulation are enabled, the CV2 knob controls:
1. **Overdrive Gain**: 10% minimum → 40% maximum (mapped from knob position)
2. **Square Noise Level**: 0% → 0.3% intensity (scales with knob)
3. **AM LFO Depth**: 0% → 100% modulation (scales with knob)

All three effects maintain their relative balance as you adjust the CV2 knob, making it intuitive to find the sweet spot between clean reverb and heavily processed textures.

## Troubleshooting

### Common Issues with New Features

#### "CV2 Overdrive knob not affecting gain"
**Problem**: Turning the CV2 knob doesn't change the overdrive amount
**Solution**: Check that Button B7 is pressed (overdrive enabled). The LED should be ON when overdrive is active. If both B7 and B8 are OFF, the CV2 knob has no effect.

#### "No modulation effect heard"
**Problem**: Toggle B8 is ON but no AM LFO modulation is audible
**Solutions**:
- Verify CV2 knob is above 0% (modulation intensity needs to be > 0)
- Check that CV1 (Send Level) has some wet signal (modulation affects the reverb output)
- Increase CV3 (Feedback) to create longer reverb tails that show modulation better
- Ensure audio is actually going through the reverb (CV1 shouldn't be at 0%)

#### "Clicking sounds when switching B7/B8"
**Status**: This is normal behavior
**Explanation**: Brief audio artifacts when switching effects are minimal and expected. The system applies the changes as quickly as possible but some clicking is unavoidable during live parameter changes.

#### "Settings not persisting between power cycles"
**Problem**: Overdrive and modulation states don't save
**Solutions**:
- Check that SD card is properly inserted and formatted
- LED blink patterns after button/toggle press indicate save status:
  - 2 quick flashes: Save successful
  - 3 medium flashes: File write error
  - 4 slow flashes: SD card not mounted
  - 5 very slow flashes: SD mount failure
- If SD card issues persist, settings will use safe defaults (both effects OFF)

#### "LED behavior seems wrong"
**Expected LED States**:
- **OFF**: Both overdrive (B7) and modulation (B8) are disabled
- **ON**: Either overdrive OR modulation (or both) are enabled
- **Blinking**: Temporary feedback during save operations or SD card errors

#### "Effects sound different than expected"
**Check These Settings**:
- **CV4 (LP Filter)**: Lower values (darker sound) vs higher values (brighter sound)
- **CV3 (Feedback/HP Filter)**: Controls both reverb time AND high-pass filtering
- **Effect Order**: Overdrive is always applied post-reverb (this is the optimal default and cannot be changed)

### Epic 1 Documentation References
If you're upgrading from Epic 1 and need information about state persistence, bootloader coordination, or QSPI storage, see the Epic 1 implementation notes in the existing sections above.

## Version History

### Version 2.0.0 - Epic 2: Filter Randomization/AM LFO Control (2025-10-20)
**Major Features:**
- **BREAKING CHANGE**: Toggle B8 remapped from noise position to filter modulation/AM LFO enable
- **NEW**: Filter randomization/AM LFO modulation controlled by toggle B8
- **ENHANCED**: CV2 knob now dual-purpose - controls overdrive/noise AND modulation intensity
- **ENHANCED**: LED now indicates when either overdrive OR modulation is enabled

**Control Changes:**
- **B8 Toggle**: Changed from "noise before/after reverb" to "modulation enable/disable"
- **CV2 Knob**: Enhanced to control three effects simultaneously when enabled:
  - Overdrive gain (when B7 enabled)
  - Square noise level (when B7 enabled)
  - AM LFO modulation depth (when B8 enabled)
- **LED Behavior**: Now indicates overdrive OR modulation state (was overdrive-only)

**Technical Implementation:**
- Unified intensity control for all effects via single knob
- Modulation affects reverb output with AM LFO (1-25 Hz range)
- Effect order remains: Input → HP Filter → Reverb → Modulation → Overdrive → Output
- Settings persistence expanded to include modulation state

**Migration from v1.x:**
- Toggle B8 behavior has changed - verify your patches after upgrading
- CV2 knob behavior enhanced but backward compatible
- LED will behave differently if you used modulation toggle

### Version 1.x - Epic 1: State Persistence & Bootloader Coordination
**Features:**
- Overdrive button state persistence via SD card storage
- QSPI/bootloader coordination with 3000ms startup delay
- Bootloader-safe memory layout implementation
- SD card settings storage with error handling and LED feedback

For detailed technical documentation on Epic 1 features, see existing sections above.

## Using

By default the make file will compile the firmware so that it can be placed on an SD card and placed in the Patch.init(), you will have to perform a `make program-boot` on your device to install the bootloader. This way you can manage firmwares on the SD and not have to hook the module up to USB.
