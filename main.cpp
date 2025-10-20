#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "lib/daisy_midi.h"
#include "sd_settings.h"
#include <random>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#define AUDIO_BLOCK_SIZE 32
#define MIN_OVERDRIVE 0.10f
#define MAX_OVERDRIVE 0.4f
#define NOISE_FACTOR 0.003f
#define SETTINGS_VERSION 1

DaisyPatchSM hw;
DaisyMidi midi;
Switch button;
Switch toggle;

SquareNoise noise_l;
SquareNoise noise_r;
Overdrive overdrive_l;
Overdrive overdrive_r;
Limiter limiter;
Jitter jitter;
ReverbSc reverb;
Svf hp_filter_l;
Svf hp_filter_r;

bool startup = true;
bool button_pressed = false;
bool toggle_pressed = false;
volatile bool enable_overdrive = false;
volatile bool filterModulationEnabled = false;
volatile bool save_settings = false;
volatile bool led_target_state = false;
bool led_current_state = false;
uint32_t last_save_time = 0;
float dry_level = 0.0f;
float wet_level = 0.0f;
float jitter_mix_level = 0.0f;
float cv_knobs[4];

std::random_device random_device;
std::mt19937 mt(random_device());
std::uniform_real_distribution<float> rd(0.0f, 1.0f);

struct Settings
{
    int version = SETTINGS_VERSION;
    bool is_overdrive_enabled = false;
    bool is_filter_modulation_enabled = false;

    bool operator!=(const Settings &other) const
    {
        return version != other.version
            || is_overdrive_enabled != other.is_overdrive_enabled
            || is_filter_modulation_enabled != other.is_filter_modulation_enabled;
    }
};

// SDSettings storage - moved from AXI SRAM to eliminate DMA conflicts
// WAS: __attribute__((section(".sram1_bss"))) which caused mount corruption due to NOLOAD
SDSettings<Settings> storage(hw);  // Now in regular RAM

struct KnobOnePoleFilter {
    float tmp = 0.0f;

    float Process(const float &x) {
        tmp = 0.0005f * x + 0.9995f * tmp;
        return tmp;
    }
};

KnobOnePoleFilter overdrive_filter_l;
KnobOnePoleFilter overdrive_filter_r;

static inline void ApplyLedState(bool state) {
    led_current_state = state;
    hw.SetLed(state);
}

static inline void RestoreLedToTarget() {
    ApplyLedState(led_target_state);
}

static void BlinkLedFeedback(int flashes, uint32_t on_ms, uint32_t off_ms) {
    const bool base_state = led_target_state;
    for(int i = 0; i < flashes; i++) {
        ApplyLedState(!base_state);
        System::Delay(on_ms);
        ApplyLedState(base_state);
        System::Delay(off_ms);
    }
}

// ReSharper disable once CppParameterMayBeConst
void AudioCallback(const AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   const size_t size) {
    button.Debounce();
    toggle.Debounce();
    hw.ProcessAllControls();

    float cv_values[8] = {
        hw.GetAdcValue(CV_1), hw.GetAdcValue(CV_2),
        hw.GetAdcValue(CV_3), hw.GetAdcValue(CV_4),
        hw.GetAdcValue(CV_5), hw.GetAdcValue(CV_6),
        hw.GetAdcValue(CV_7), hw.GetAdcValue(CV_8)
    };
    for (size_t i = 0; i < 4; i++) {
        cv_values[i] = roundf((cv_values[i] + cv_values[i + 4]) * 50) / 50.0f;
        if (cv_values[i] != cv_knobs[i] || startup) {
            cv_knobs[i] = cv_values[i];
            const float cv_value = cv_values[i];
            switch (i) {
                case 0:
                    // Reverb send level
                    dry_level = cv_value;
                    wet_level = 1.0f - dry_level;
                    break;
                case 1:
                    // Jitter amount
                    jitter_mix_level = cv_value;
                    break;
                case 2:
                    // Reverb feedback amount
                    reverb.SetFeedback(fmap(cv_value, 0.2f, 1.00f));

                    // Set high-pass filter cutoff frequency based on reverb time
                    {
                        const float cutoff_freq = fmap(cv_value, 20.0f, 500.0f, Mapping::LOG);
                        hp_filter_l.SetFreq(cutoff_freq);
                        hp_filter_r.SetFreq(cutoff_freq);
                    }
                    break;
                case 3:
                    reverb.SetLpFreq(fmap(cv_value, 1000.f, 19000.f, Mapping::LOG));
                    break;
                // ReSharper disable once CppDFAUnreachableCode
                default: break;
            }
        }
    }
    if (startup) {
        midi.sysex_printf_buffer("Calculation order: POST-reverb (fixed)\n");
        midi.sysex_send_buffer();
        startup = false;
    }

    if (button.Pressed() && !button_pressed) {
        midi.sysex_printf_buffer("Button Pressed\n");
        button_pressed = true;

        if (enable_overdrive == false) {
            enable_overdrive = true;
            hw.WriteCvOut(CV_OUT_2, 5.0f);
            save_settings = true;
        } else {
            enable_overdrive = false;
            hw.WriteCvOut(CV_OUT_2, 0.0f);
            save_settings = true;
        }
    } else if (!button.Pressed()) {
        button_pressed = false;
    }

    // Toggle switch (B8) handler for filter modulation enable/disable
    if (toggle.Pressed() && !toggle_pressed) {
        midi.sysex_printf_buffer("Toggle Pressed - Filter Modulation: %s\n",
                                filterModulationEnabled ? "OFF" : "ON");
        toggle_pressed = true;

        filterModulationEnabled = !filterModulationEnabled;
        save_settings = true;
    } else if (!toggle.Pressed()) {
        toggle_pressed = false;
    }

    // LED state: ON when overdrive OR filter modulation enabled, OFF when both disabled
    led_target_state = enable_overdrive || filterModulationEnabled;

    float audio_in_l[size];
    float audio_in_r[size];

    float audio_out_l[size];
    float audio_out_r[size];

    for (size_t i = 0; i < size; i++) {
        audio_in_l[i] = in[0][i];
        audio_in_r[i] = in[1][i];
    }

    limiter.ProcessBlock(audio_in_l, size, 0.9f);
    limiter.ProcessBlock(audio_in_r, size, 0.9f);

    overdrive_l.SetDrive(overdrive_filter_l.Process(enable_overdrive
                                                        ? fmap(jitter_mix_level, MIN_OVERDRIVE, MAX_OVERDRIVE)
                                                        : MIN_OVERDRIVE));
    overdrive_r.SetDrive(overdrive_filter_r.Process(enable_overdrive
                                                        ? fmap(jitter_mix_level, MIN_OVERDRIVE, MAX_OVERDRIVE)
                                                        : MIN_OVERDRIVE));

    for (size_t i = 0; i < size; i++) {
        // Dry signal
        const float dry_l = audio_in_l[i] * dry_level;
        const float dry_r = audio_in_r[i] * dry_level;

        // Process input through high-pass filters
        hp_filter_l.Process(audio_in_l[i]);
        hp_filter_r.Process(audio_in_r[i]);
        audio_in_l[i] = hp_filter_l.High(); // Use high-pass output
        audio_in_r[i] = hp_filter_r.High(); // Use high-pass output

        const float noise_l_out = filterModulationEnabled
            ? noise_l.Process(audio_in_l[i]) * NOISE_FACTOR * jitter_mix_level
            : 0.0f;
        const float noise_r_out = filterModulationEnabled
            ? noise_r.Process(audio_in_r[i]) * NOISE_FACTOR * jitter_mix_level
            : 0.0f;

        reverb.Process((audio_in_l[i] + noise_l_out) * wet_level, (audio_in_r[i] + noise_r_out) * wet_level,
                       &audio_in_l[i], &audio_in_r[i]);

        const float jitter_out = filterModulationEnabled ? jitter.Process() : 0.0f;

        if (filterModulationEnabled) {
            audio_out_l[i] = dry_l + audio_in_l[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);
            audio_out_r[i] = dry_r + audio_in_r[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);
        } else {
            // When modulation disabled, clean signal path without jitter
            audio_out_l[i] = dry_l + audio_in_l[i];
            audio_out_r[i] = dry_r + audio_in_r[i];
        }

        // Calculation order fixed: Overdrive always applied post-reverb (optimal default)
        audio_out_l[i] = overdrive_l.Process(audio_out_l[i]);
        audio_out_r[i] = overdrive_r.Process(audio_out_r[i]);
    }

    limiter.ProcessBlock(audio_out_l, size, 1.1f);
    limiter.ProcessBlock(audio_out_r, size, 1.1f);

    for (size_t i = 0; i < size; i++) {
        out[0][i] = audio_out_l[i];
        out[1][i] = audio_out_r[i];
    }

    midi.sysex_send_buffer();
}

// ============================================================================
// QSPI Memory Layout (Verified Safe - Story 1.3)
// ============================================================================
// Based on bootloader analysis (Story 1.1) and linker map verification (Story 1.3):
//
// QSPI Physical Address Space (8MB total):
//   0x90000000 - 0x90040000: Bootloader Reserved (256KB) - DO NOT USE
//   0x90040000 - 0x9005CF6C: Application Firmware (~116KB)
//   0x9005CF6C - 0x9006B000: Safety Gap (56KB)
//   0x9006B000 - 0x9006C000: PersistentStorage (4KB allocated)
//   0x9006C000 - 0x90800000: Available for expansion (~7.6MB)
//
// Settings Location: 0x2B000 offset = 0x9006B000 absolute address
// Safety Verified: Settings are 172KB beyond bootloader space (256KB + 172KB)
//                  Settings are 56KB beyond firmware end (acceptable margin)
//
// Build verification (Story 1.3):
//   Linker map checked: No overlaps detected
//   Bootloader space protected: 0x90000000-0x90040000 untouched
//   Firmware size: 115KB (well within available space)
// ============================================================================

int main() {
    // Initialize core hardware
    hw.Init();

    // CRITICAL: Wait for bootloader to release SD card and QSPI
    // Duration from Story 1.1 investigation: 3000ms (bootloader has 2.5s grace period)
    System::Delay(3000);  // Let bootloader complete

    // Ensure SD card mount is preserved during settings operations

    // Additional delay to ensure SD card is fully ready
    System::Delay(500);

    // Initialize SD card settings storage
    // Using SD card because program is too large (549KB) for BOOT_SRAM mode (480KB limit)
    // BOOT_QSPI mode prevents QSPI writes (hardware limitation)
    auto storage_result = storage.Init({SETTINGS_VERSION, false, false}, "nsv_settings.bin");

    if (storage_result == SDSettings<Settings>::Result::OK)
    {
        Settings& loaded_settings = storage.GetSettings();
        if (loaded_settings.version == SETTINGS_VERSION) {
            // Settings loaded successfully with matching version
            enable_overdrive = loaded_settings.is_overdrive_enabled;
            filterModulationEnabled = loaded_settings.is_filter_modulation_enabled;
            hw.WriteCvOut(CV_OUT_2, enable_overdrive ? 5.0f : 0.0f);
        } else {
            // Version mismatch: use safe defaults
            enable_overdrive = false;
            filterModulationEnabled = false;
            hw.WriteCvOut(CV_OUT_2, 0.0f);
        }
        // Queue a save so defaults or loaded settings always persist to disk
        save_settings = true;
    }
    else
    {
        // SD card not available - use defaults (settings won't persist)
        enable_overdrive = false;
        filterModulationEnabled = false;
        hw.WriteCvOut(CV_OUT_2, 0.0f);
    }

    // LED state: ON when overdrive OR filter modulation enabled, OFF when both disabled
    led_target_state = enable_overdrive || filterModulationEnabled;
    if (storage_result != SDSettings<Settings>::Result::OK) {
        BlinkLedFeedback(4, 400, 200);  // SD init error
        RestoreLedToTarget();
    }

    // Force LED update to match target state
    ApplyLedState(led_target_state);

    button.Init(DaisyPatchSM::B7);
    toggle.Init(DaisyPatchSM::B8);

    limiter.Init();
    overdrive_l.Init();
    overdrive_r.Init();
    noise_l.Init(hw.AudioSampleRate());
    noise_r.Init(hw.AudioSampleRate());
    jitter.Init(hw.AudioSampleRate());
    jitter.SetAmp(1);
    jitter.SetCpsMin(1);
    jitter.SetCpsMax(25);
    reverb.Init(hw.AudioSampleRate());
    midi.Init();

    hp_filter_l.Init(hw.AudioSampleRate());
    hp_filter_r.Init(hw.AudioSampleRate());

    hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
    hw.StartAudio(AudioCallback);

    // Flash LED on startup to indicate we reached main loop
    for(int i = 0; i < 5; i++) {
        ApplyLedState(true);
        System::Delay(120);
        ApplyLedState(false);
        System::Delay(120);
    }
    RestoreLedToTarget();

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        if (save_settings) {
            const uint32_t now = System::GetNow();

            // Rate limit: max 1 save per 100ms to prevent excessive SD card wear
            if (now - last_save_time >= 100) {
                Settings &localSettings = storage.GetSettings();
                localSettings.is_overdrive_enabled = enable_overdrive;
                localSettings.is_filter_modulation_enabled = filterModulationEnabled;

                // Save settings to SD card
                auto save_result = storage.Save();

                if (save_result == SDSettings<Settings>::Result::OK) {
                    // Save succeeded - 2 quick flashes
                    BlinkLedFeedback(2, 100, 100);
                } else {
                    if (save_result == SDSettings<Settings>::Result::ERR_NO_SD_CARD) {
                        // SD card not mounted - 4 slow flashes
                        BlinkLedFeedback(4, 200, 200);
                    } else if (save_result == SDSettings<Settings>::Result::ERR_FILE_ERROR) {
                        // File write failure - 3 medium flashes
                        BlinkLedFeedback(3, 150, 150);
                    } else {
                        // Mount failure - 5 very slow flashes
                        BlinkLedFeedback(5, 250, 250);
                    }
                }
                RestoreLedToTarget();

                save_settings = false;
                last_save_time = now;
            }
        }
        if (led_current_state != led_target_state) {
            RestoreLedToTarget();
        }
        System::Delay(10);
    }
}
