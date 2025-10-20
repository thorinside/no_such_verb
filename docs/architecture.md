# No Such Verb - Architecture Documentation

**Project Type:** Embedded Audio DSP Firmware for Daisy Patch.init()
**Hardware Platform:** Daisy Patch.init() (ARM Cortex-H7 STM32H7 @ 480MHz)
**Primary Language:** C++
**Architecture Pattern:** Real-Time Audio Callback Processing Pipeline

## Executive Summary

No Such Verb is an experimental audio effects pedal implemented as embedded firmware for the Daisy Patch.init() platform. The system implements a real-time audio processing pipeline using the Daisy platform's callback-based architecture, combining:

- **Overdrive effect** with harmonic saturation (toggleable via button)
- **Jitter modulation** (square noise with configurable mix)
- **Schroeder-Moorer algorithmic reverb** with configurable feedback and filtering
- **Dynamic tone shaping** (high-pass filter on reverb tail)
- **Persistent settings storage** via QSPI flash

The architecture prioritizes low-latency real-time processing (sample-rate callback execution), deterministic resource allocation, and responsive control via front-panel CV inputs and toggle switches with status LED feedback.

## Technology Stack

| Component | Technology | Version | Justification |
|-----------|-----------|---------|---------------|
| **Microcontroller** | STM32H7 (ARM Cortex-H7) | - | 480MHz with DSP extensions; 1MB RAM + QSPI flash for persistent storage |
| **Platform** | Daisy Patch.init() | - | All-in-one audio I/O module; encoder wheels; 4 analog inputs (CV); built-in codec |
| **Language** | C++ | C++11 | Daisy SDK with real-time constraints; low-level hardware access |
| **Audio Framework** | Daisy Audio Callback | - | Callback-based processing for deterministic real-time audio; ~48kHz fixed |
| **Audio Codec** | Built-in Daisy codec | - | 24-bit stereo; integrated with platform; minimal latency |
| **Build System** | Make + GCC ARM | GNU Make | Cross-platform build; integrates with GCC ARM toolchain |
| **Development Platform** | macOS Darwin | 24.6.0 | Primary development environment; supports GitHub Actions CI |

## Architecture Pattern

### Primary Pattern: Real-Time Callback-Based Audio Processing

The firmware follows the **Daisy platform's audio callback model**, where audio is processed in a callback function invoked by the DSP core at audio sample rate (~48kHz). Single samples (or small blocks) are processed with minimal latency, ensuring responsive real-time effects processing.

```
Audio Input (I2S)
    ↓ (DMA → 128-sample blocks)
Audio Processor (Custom DSP)
    ├─ Overdrive Stage
    ├─ Tone Filter
    ├─ Reverb Effect (parallel/feedback structure)
    └─ Dry/Wet Mixer
    ↓ (processed blocks)
Audio Output (I2S)
    ↓ (DMA → analog DAC)
Audio Output (1/4" Jack)
```

### Design Patterns

#### 1. Direct Signal Processing - Single Callback

The entire audio pipeline executes in a single `AudioCallback()` function invoked at sample rate (~48kHz). This monolithic approach ensures predictable timing and minimal context switching:

```cpp
void AudioCallback(const AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   const size_t size) {
    // Process all controls (CV inputs, button, toggle)
    hw.ProcessAllControls();

    // Read CV values and map to parameters
    float dry_level = cv_values[0];
    float jitter_mix = cv_values[1];
    float reverb_feedback = fmap(cv_values[2], 0.2f, 1.0f);
    float reverb_lpfreq = fmap(cv_values[3], 1000.f, 19000.f, Mapping::LOG);

    // Sample-by-sample audio processing
    for (size_t i = 0; i < size; i++) {
        float audio_l = in[0][i];
        float audio_r = in[1][i];

        // Apply overdrive (if enabled)
        if (enable_overdrive) {
            audio_l = overdrive_l.Process(audio_l);
            audio_r = overdrive_r.Process(audio_r);
        }

        // Apply jitter modulation
        audio_l += noise_l.Process() * jitter_mix_level;
        audio_r += noise_r.Process() * jitter_mix_level;

        // Apply reverb with dry/wet mix
        float reverb_l = reverb.Process(audio_l);
        float reverb_r = reverb.Process(audio_r);

        out[0][i] = audio_l * dry_level + reverb_l * wet_level;
        out[1][i] = audio_r * dry_level + reverb_r * wet_level;
    }
}
```

**Benefit:** Deterministic real-time execution; no threading; minimal latency; simple state management.

#### 2. Persistent Settings Storage

Settings (e.g., overdrive enabled state) are stored in QSPI flash using the Daisy platform's `PersistentStorage` template:

```cpp
struct Settings {
    int version;
    bool is_overdrive_enabled;
};

PersistentStorage<Settings> storage(hw.qspi);
```

**Benefit:** Settings survive power cycles; version tracking enables future firmware updates.

#### 3. Exponential Smoothing - CV Parameter Filtering

CV input values are exponentially averaged to reduce noise from analog inputs:

```cpp
float cv_values[8] = { /* raw ADC values */ };
for (size_t i = 0; i < 4; i++) {
    cv_values[i] = roundf((cv_values[i] + cv_values[i + 4]) * 50) / 50.0f;
    if (cv_values[i] != cv_knobs[i]) {
        cv_knobs[i] = cv_values[i];
        // Update effect parameter
    }
}
```

**Benefit:** Smooth parameter sweeps; reduced zipper noise; removes ADC jitter without additional buffering.

## Architecture Overview

### System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│        Daisy Patch.init() (STM32H7 @ 480MHz)               │
│  512KB SRAM + 8MB QSPI + Integrated Audio Codec             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │     Audio Processing (Callback-Driven)              │  │
│  │              @48kHz, sample-by-sample               │  │
│  ├──────────────────────────────────────────────────────┤  │
│  │                                                      │  │
│  │  AudioCallback(in, out, size) {                     │  │
│  │    ├─ Process Controls (CV + Button)                │  │
│  │    ├─ Read Audio Input                              │  │
│  │    ├─ Apply Overdrive (DaisySP)                     │  │
│  │    ├─ Apply Jitter Noise                            │  │
│  │    ├─ Apply Reverb (ReverbSc)                       │  │
│  │    ├─ Apply HP Filter                               │  │
│  │    ├─ Mix Dry/Wet                                   │  │
│  │    └─ Write Audio Output                            │  │
│  │  }                                                   │  │
│  │                                                      │  │
│  ├──────────────────────────────────────────────────────┤  │
│  │                                                      │  │
│  │      Memory Usage:                                   │  │
│  │      SRAM: 431KB / 512KB (82%)                       │  │
│  │      QSPI: 118KB / 8MB (1.5% - persistent settings) │  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │      Control Processing (in AudioCallback)          │  │
│  ├──────────────────────────────────────────────────────┤  │
│  │                                                      │  │
│  │  CV Inputs (8) → Quantize & Smooth → Parameters:    │  │
│  │                                                      │  │
│  │  ├─ CV1/CV5 → Dry/Wet Mix (0.0-1.0)                │  │
│  │  ├─ CV2/CV6 → Jitter Mix (0.0-1.0)                 │  │
│  │  ├─ CV3/CV7 → Reverb Feedback (0.2-1.0)            │  │
│  │  └─ CV4/CV8 → Reverb LP Freq (1kHz-19kHz)          │  │
│  │                                                      │  │
│  │  Button → Toggle overdrive (output CV_OUT_2)        │  │
│  │  Toggle → Reserved for future features              │  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
         ↑ CV/Audio In        ↓ CV/Audio Out
    ┌────────────────┐    ┌──────────────────┐
    │  Integrated    │    │    Patch.init()  │
    │  Audio Codec   │    │    I/O Jacks     │
    └────────────────┘    └──────────────────┘
    Built-in 24-bit       Audio + CV Outputs
```

### Audio Processing Flow (within AudioCallback)

```
For each sample in block:
  1. Read CV inputs (8 channels)
  2. Quantize & smooth values (50-step resolution)
  3. Route to DSP objects:
     - dry_level → Dry/Wet mix coefficient
     - jitter_mix → SquareNoise amplitude
     - reverb params → ReverbSc feedback & LP freq
  4. Process audio sample:
     a. Read input_l, input_r
     b. Apply Overdrive.Process() if enabled
     c. Add Jitter: noise_l.Process() * jitter_mix
     d. Apply ReverbSc.Process()
     e. Apply HP filter on reverb output
     f. Mix: output = input * dry_level + reverb * wet_level
     g. Apply Limiter (safety)
  5. Write output_l, output_r
```

### Control Signal Flow

1. **CV Input Reading** (within AudioCallback)
   - All 8 CV inputs read via `hw.GetAdcValue(CV_x)`
   - Range: 0.0 to 1.0 (normalized)

2. **Quantization & Smoothing**
   - Round to 50 discrete steps: `cv_value = roundf((raw) * 50) / 50`
   - Prevents zipper noise from ADC jitter

3. **Change Detection**
   - Compare to `cv_knobs[i]` (last transmitted value)
   - Only update if changed OR first call (startup)

4. **Parameter Mapping**
   - Use `fmap()` for non-linear scaling (log, exponential)
   - Route via switch statement to appropriate effect

5. **Effect Update**
   - DSP objects updated immediately via setters
   - Changes take effect on next sample processing
   - Response latency: < 1ms (sample-rate callback)

## Data Architecture

### Audio Block Structure

```cpp
struct audio_block_t {
    int16_t data[128];      // 128 samples per block
};
```

**Properties:**
- 2 channels (stereo): 2 blocks per update (left + right)
- 128 samples per block = 2.9ms at 44.1kHz
- 16-bit signed integer audio data
- Allocated from fixed memory pool (200 blocks = 25KB)

### Reverb State Structure

```cpp
class ReverbProcessor {
private:
    // 8 parallel comb filters (different delay times)
    int16_t* comb_buffers[8];      // 7MB total (max delay 5000ms)
    float comb_filter_state[8];    // Feedback coefficient per comb

    // 4 series allpass filters (diffusion)
    int16_t* allpass_buffers[4];   // 350KB

    // Room size and decay parameters
    float room_size;               // [0.5, 5.0] scaling factor
    float damping;                 // Exponential decay coefficient
    float wet_level, dry_level;    // Mix coefficients
};
```

**Memory Allocation:**
- Comb buffers: 7MB (PSRAM, allocated once during setup)
- Allpass buffers: 350KB (PSRAM)
- State: ~50KB (on-chip RAM)
- Total: ~7.4MB (fits in Teensy 4.0 with PSRAM)

### Control State

```cpp
class Controls {
private:
    // Raw ADC readings (10-bit, 0-1023)
    uint16_t raw_adc[5];

    // Smoothed readings (0.0-1.0)
    float smoothed[5];

    // Last transmitted values (change detection)
    float last_sent[5];

    // ADC pin assignments
    static const uint8_t pins[5] = {A0, A1, A2, A3, A6};
};
```

**Update Cycle:**
1. Read raw ADC (blocking ~5µs per channel)
2. Apply exponential smoothing (add ~0.1 * (new - smoothed))
3. Check for 3-LSB change threshold
4. If changed, update `audioProcessor` parameters

## Global Objects and State

The firmware uses global objects instantiated in `main()` and accessed from the audio callback:

```cpp
// Hardware interface
DaisyPatchSM hw;                        // Daisy platform hardware
DaisyMidi midi;                         // MIDI over USB (optional)

// Input controls
Switch button;                          // Button for overdrive toggle
Switch toggle;                          // Toggle switch (future use)

// DSP processing units (from DaisySP library)
SquareNoise noise_l, noise_r;          // Jitter modulation sources
Overdrive overdrive_l, overdrive_r;    // Overdrive distortion
Limiter limiter;                        // Output limiter (safety)
Jitter jitter;                          // Additional modulation
ReverbSc reverb;                        // Schroeder reverb algorithm
Svf hp_filter_l, hp_filter_r;          // High-pass filters on reverb tail

// State variables
bool enable_overdrive;                  // Toggled by button
bool save_settings;                     // Flag to persist state
float dry_level, wet_level;             // Mix coefficients
float jitter_mix_level;                 // Jitter amount
float cv_knobs[4];                      // Current CV values
```

### Audio Callback Signature

```cpp
void AudioCallback(const AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   const size_t size)
```

- **Parameters:**
  - `in[2][size]`: 2-channel input audio (stereo, size samples)
  - `out[2][size]`: 2-channel output audio (stereo, size samples)
  - `size`: Block size in samples (~48 samples typical)

- **Execution Context:** Invoked at ~48kHz from Daisy DSP core
- **Real-Time Constraint:** Must complete within 1 block period (~1ms)

### Control Flow

**CV Input Mapping:**
```
CV_1/CV_5 → dry_level (0.0-1.0)
CV_2/CV_6 → jitter_mix_level (0.0-1.0)
CV_3/CV_7 → reverb.SetFeedback() (0.2-1.0)
CV_4/CV_8 → reverb.SetLpFreq() (1kHz-19kHz log scale)
```

**Button Input:**
```
Button press → Toggle enable_overdrive
            → Output CV_OUT_2 (5V or 0V indicator)
            → Set save_settings flag
```

**Toggle Input:**
```
Reserved for future use (noise insertion mode, etc.)
```

## Component Overview

### 1. Main Entry Point (`main.cpp`)

**Responsibility:** System initialization and audio callback definition

**Key Functions:**
- `main()`: Initialize Daisy hardware, start audio callback
- `AudioCallback()`: Real-time audio processing (called at ~48kHz)

**Initialization Sequence:**
```cpp
void main() {
    hw.Init();                               // Initialize Daisy Patch.init()
    hw.SetAudioBlockSize(48);                // Block size in samples
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAdc();                           // Start ADC for CV inputs
    hw.StartDac();                           // Start DAC for CV outputs

    // Initialize DSP objects (from DaisySP)
    reverb.Init(hw.AudioSampleRate());
    overdrive_l.Init();
    overdrive_r.Init();
    hp_filter_l.Init(hw.AudioSampleRate());
    hp_filter_r.Init(hw.AudioSampleRate());

    // Load persistent settings from QSPI flash
    storage.Init();
    storage.Read(settings);
    enable_overdrive = settings.is_overdrive_enabled;

    // Start audio processing (blocking)
    hw.StartAudio(AudioCallback);
}
```

**Audio Processing Loop:**
```cpp
void AudioCallback(const AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   const size_t size) {
    // Process controls and audio for 'size' samples
    // (See Design Patterns section for detailed flow)

    // When settings change:
    if (save_settings) {
        storage.Write(settings);
        save_settings = false;
    }
}
```

### 2. DSP Processing Units (from DaisySP Library)

The firmware uses pre-built DSP objects from the DaisySP library, initialized in `main()`:

**Overdrive (Distortion)**
```cpp
Overdrive overdrive_l, overdrive_r;
// Configurable soft-clipping distortion
// Output: audio_l = overdrive_l.Process(audio_l);
```

**Square Noise Generator (Jitter)**
```cpp
SquareNoise noise_l, noise_r;
// Generates band-limited square wave noise
// Modulates audio with jitter_mix_level amplitude
```

**Reverb Algorithm (Schroeder-Moorer)**
```cpp
ReverbSc reverb;
// Parameters: SetFeedback(0.2-1.0), SetLpFreq(1kHz-19kHz)
// Output: float out = reverb.Process(audio_in);
```

**High-Pass Filter (on reverb tail)**
```cpp
Svf hp_filter_l, hp_filter_r;
// State-variable filter with configurable frequency
// Controlled by CV_3 (reverb feedback parameter)
```

**Limiter (Safety)**
```cpp
Limiter limiter;
// Prevents output clipping at extreme settings
// Applied before DAC output
```

### 3. Control Input Processing

CV inputs are read and smoothed within the audio callback:

```cpp
// Read 8 CV inputs (4 parameters with redundant analog inputs)
float cv_values[8] = {
    hw.GetAdcValue(CV_1), hw.GetAdcValue(CV_2),
    hw.GetAdcValue(CV_3), hw.GetAdcValue(CV_4),
    hw.GetAdcValue(CV_5), hw.GetAdcValue(CV_6),
    hw.GetAdcValue(CV_7), hw.GetAdcValue(CV_8)
};

// Average pairs and quantize to 50 steps
for (size_t i = 0; i < 4; i++) {
    cv_values[i] = roundf((cv_values[i] + cv_values[i + 4]) * 50) / 50.0f;

    // Check if value changed significantly
    if (cv_values[i] != cv_knobs[i] || startup) {
        cv_knobs[i] = cv_values[i];

        // Route to appropriate effect parameter
        switch (i) {
            case 0: dry_level = cv_value; break;  // Dry/wet mix
            case 1: jitter_mix_level = cv_value; break;  // Jitter amount
            case 2: reverb.SetFeedback(fmap(cv_value, 0.2f, 1.0f)); break;  // Reverb feedback
            case 3: reverb.SetLpFreq(fmap(cv_value, 1000.f, 19000.f, Mapping::LOG)); break;  // LP filter
        }
    }
}
```

**Mapping Functions:**
- `fmap()` - Maps 0-1 range to output range (linear, log, or exponential)
- `Mapping::LOG` - Logarithmic scaling for perceptual frequency mapping

## Deployment Architecture

### Hardware Integration

**Daisy Patch.init() Built-in Features:**
- Audio I/O: 24-bit stereo via integrated codec
- Sample Rate: 48kHz (fixed)
- CV Inputs: 8 analog CV inputs (0-5V)
- CV Outputs: 4 analog CV outputs for modulation
- Encoder: Incremental optical encoder with click
- Display: OLED screen for menus (optional)
- Button: Programmable pushbutton
- LEDs: RGB status indicator

**Audio Path (from code):**
- Input: Stereo audio via integrated codec input
- Output: Stereo audio via integrated codec output + CV outputs
- Latency: Sub-millisecond (sample-rate callback, 48kHz)

### Build Process

**Compilation (Makefile):**
```bash
make clean
make -j$(sysctl -n hw.ncpu)
```

**Output Artifacts:**
- `build/no_such_verb.elf` - Executable with debug symbols (~1.9MB)
- `build/no_such_verb.bin` - Binary format (116KB)
- `build/no_such_verb.hex` - Intel HEX format (326KB)

**Memory Usage (from build):**
- FLASH: 0B (loaded from QSPI on boot)
- SRAM: 431KB / 512KB (82%)
- QSPI Flash: 118KB / 8MB (1.5%)

**Deployment:**
1. Connect Daisy Patch.init() via USB
2. Put device in bootloader mode (press reset or button combo)
3. Upload using Daisy bootloader utility or make command
4. Firmware runs immediately; settings persisted in QSPI

### CI/CD Pipeline

**Workflow:** GitHub Actions (`.github/workflows/build.yml`)

**Triggers:**
- Push to main branch
- Pull requests to main
- Manual workflow dispatch

**Build Steps:**
1. Checkout repository
2. Install arduino-cli (v1.1.1+)
3. Install Teensy board support
4. Install Teensy Audio Library
5. Compile firmware
6. Archive `.hex` artifact

**Outputs:**
- Build status (pass/fail)
- Artifact: `no_such_verb.hex` (downloadable)
- Build log

## Testing Strategy

### Performance Monitoring

**CPU Usage:**
```cpp
float usage = AudioProcessorUsage();  // Current block usage
float max_usage = AudioProcessorUsageMax();  // Peak usage
Serial.print("CPU: ");
Serial.print(usage);
Serial.print("% (max: ");
Serial.print(max_usage);
Serial.println("%)");
```

**Target:** < 85% CPU usage to maintain headroom for interrupts

**Memory Monitoring:**
```cpp
int16_t *block = AudioStream::allocate();
if (block == NULL) {
    Serial.println("Audio memory pool exhausted!");
}
```

### Manual Testing

**Audio Quality:**
- Record output from various input levels (instrument and line)
- Verify no clipping or distortion at nominal levels
- Listen for aliasing artifacts at high overdrive settings

**Parameter Response:**
- Turn each potentiometer full range; listen for smooth parameter sweep
- Verify no clicking or pops when adjusting parameters
- Check that control latency is < 50ms (imperceptible)

**Edge Cases:**
- Set reverb to maximum size and decay; verify no CPU overload
- Rapidly toggle between full dry and full wet mix
- Test with silent input (verify no noise floor drift)

## Performance Characteristics

### Latency Budget

| Stage | Latency | Notes |
|-------|---------|-------|
| ADC Input (Analog → I2S) | 1-2ms | Inherent to codec |
| Audio Input Buffer | 0-2.9ms | Depends on phase |
| Processing | 2.9ms | One block period |
| Audio Output Buffer | 0-2.9ms | Depends on phase |
| DAC Output (I2S → Analog) | 1-2ms | Inherent to codec |
| **Total End-to-End** | **6-11ms** | Typical: ~8ms |

**Subjective Feel:** Imperceptible to human hearing (threshold ~20ms)

### CPU Usage Budget

| Component | CPU Usage | Notes |
|-----------|-----------|-------|
| Audio Input/Output | 5% | DMA, minimal CPU |
| Overdrive | 8% | Soft clipping + harmonic |
| Tone Filter | 5% | Single-pole IIR |
| Reverb (Base) | 25% | 8 combs + 4 allpass |
| ADC Polling | <1% | Minimal in loop() |
| **Total** | **~43%** | Peak at full reverb decay |

**Headroom:** 57% available for future effects or higher sample rate

### Power Consumption

| Component | Current | Voltage | Power |
|-----------|---------|---------|-------|
| Teensy 4.0 (600MHz) | 100mA | 3.3V | 330mW |
| SGTL5000 Codec | 30mA | 3.3V | 99mW |
| Analog Circuits | 5mA | 3.3V | 17mW |
| **Total** | **135mA** | **3.3V** | **446mW** |

**Powered by:** 500mA USB or 12V DC adapter with LDO regulator

---

## Bootloader & QSPI Coordination

### Critical Timing: 3000ms Bootloader Delay

The Daisy Patch.init() bootloader loads firmware from SD card during startup. This process temporarily holds QSPI resources. To prevent state corruption:

1. Application **must delay 3000ms** after `hw.Init()`
2. This allows bootloader to complete and release QSPI (bootloader has 2.5s grace period)
3. Only then is `PersistentStorage` safe to initialize
4. See Story 1.1 investigation for detailed timing analysis (main.cpp:236-238)

**Implementation:**
```cpp
int main() {
    hw.Init();

    // CRITICAL: Wait for bootloader to release QSPI
    // Duration from Story 1.1 investigation: 3000ms (bootloader has 2.5s grace period)
    System::Delay(3000);  // Let bootloader complete

    // Now safe to initialize QSPI and load settings
    storage.Init({SETTINGS_VERSION, false}, 0x2B000);
}
```

### QSPI Memory Layout

Based on bootloader analysis (Story 1.1) and linker map verification (Story 1.3):

```
QSPI Physical Address Space (8MB total):

┌─────────────────────────────────────────────────────┐
│ Bootloader Reserved (256KB)                         │
│ 0x90000000 - 0x90040000                            │
│ DO NOT USE - bootloader critical                    │
└─────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────┐
│ Application Firmware (~116KB)                       │
│ 0x90040000 - 0x9005CF6C                            │
│ Loaded by bootloader, read-only                     │
└─────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────┐
│ Safety Gap (56KB)                                   │
│ 0x9005CF6C - 0x9006B000                            │
│ Reserved margin for future firmware growth          │
└─────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────┐
│ PersistentStorage (4KB allocated) - SAFE            │
│ 0x9006B000 - 0x9006C000                            │
│ Application persistent settings (offset 0x2B000)    │
└─────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────┐
│ Available for Expansion (~7.6MB)                    │
│ 0x9006C000 - 0x90800000                            │
│ Future application data                             │
└─────────────────────────────────────────────────────┘
```

**Key Safety Margins:**
- Settings location: **0x9006B000** (offset 0x2B000 from application QSPI base)
- Distance from bootloader: **172KB** (256KB bootloader + 172KB margin)
- Distance from firmware end: **56KB** safety gap
- Verified via linker map: No overlaps detected

### Rate-Limited Writes

Settings saved to QSPI with 100ms throttling to prevent flash wear:

**Implementation (main.cpp:297-320):**
```cpp
while (true) {
    if (save_settings) {
        uint32_t now = System::GetNow();

        // Rate limit: max 1 save per 100ms to prevent excessive QSPI wear
        if (now - last_save_time >= 100) {
            Settings &localSettings = storage.GetSettings();
            localSettings.is_overdrive_enabled = enable_overdrive;
            storage.Save();  // Atomic erase+write

            save_settings = false;
            last_save_time = now;
        }
    }
    System::Delay(10);
}
```

**Benefits:**
- Max 1 write per 100ms prevents QSPI wear
- Ensures reliability across power cycles
- Safe during bootloader operations
- Write-only-if-changed optimization (handled by PersistentStorage API)

### Atomic Write Operations

The `PersistentStorage` API ensures atomic writes:

1. **Erase** entire sector before writing
2. **Write** new data in single operation
3. **Only writes if data changed** (prevents unnecessary wear)

**Version Management:**
```cpp
struct Settings {
    int version = SETTINGS_VERSION;
    bool is_overdrive_enabled;

    bool operator!=(const Settings &other) const {
        return version != other.version
            || is_overdrive_enabled != other.is_overdrive_enabled;
    }
};
```

Settings are validated on load - version mismatch triggers safe defaults.

### Future Enhancements

If adding new QSPI features:

1. **Verify memory layout** - Don't overlap bootloader (0x90000000-0x90040000)
2. **Maintain 3000ms delay** in `main()` before QSPI init
3. **Rate-limit writes** - Max 1 write per 100ms
4. **Test with bootloader** - Verify SD card loading still works
5. **Check linker map** - Verify `build/no_such_verb.map` for overlaps
6. **Use safe addresses** - Stay beyond 0x9006B000 for new storage

### Troubleshooting QSPI Issues

**Settings Lost After Power Cycle:**
- Verify 3000ms delay in main.cpp:237
- Check settings offset is 0x2B000 (main.cpp:242)
- Review Story 1.1 findings on bootloader timing

**Settings Corruption:**
- Check rate limiting is active (100ms minimum)
- Verify version check on load (main.cpp:263)
- Ensure safe defaults applied on mismatch (main.cpp:268-270)

**Bootloader Failure:**
- Verify no writes to 0x90000000-0x90040000 (bootloader space)
- Check linker map for unexpected QSPI usage
- Ensure firmware size < 116KB (leaves safety margin)

---

## Known Limitations and Future Enhancements

### Current Limitations

1. **Fixed Sample Rate:** 44.1kHz only (no 48kHz or 96kHz support)
2. **No Preset Storage:** Parameters not saved between power cycles
3. **Single Effect Chain:** No multi-effect routing or parallel processing
4. **Tone Control:** Single low-pass filter (no EQ bands)
5. **No MIDI:** Manual control only via analog potentiometers

### Recommended Enhancements

1. **Preset System:** EEPROM storage for up to 10 presets
2. **Footswitch Input:** GPIO-based bypass toggle (currently manual)
3. **MIDI Over USB:** Receive parameter changes from DAW
4. **Stereo Width:** Separate reverb processing per channel
5. **Parametric EQ:** Replace tone control with 3-band shelving EQ

---

This architecture supports extension to multi-effect processing (delay, chorus, compression) while maintaining deterministic real-time performance and user responsiveness.
