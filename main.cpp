#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "lib/daisy_midi.h"
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
bool enable_overdrive = false;
bool save_settings = false;
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

    bool operator!=(const Settings &other) const
    {
        return version != other.version
            || is_overdrive_enabled != other.is_overdrive_enabled;
    }
};

PersistentStorage<Settings> storage(hw.qspi);

struct KnobOnePoleFilter {
    float tmp = 0.0f;

    float Process(const float &x) {
        tmp = 0.0005f * x + 0.9995f * tmp;
        return tmp;
    }
};

KnobOnePoleFilter overdrive_filter_l;
KnobOnePoleFilter overdrive_filter_r;

// ReSharper disable once CppParameterMayBeConst
void AudioCallback(const AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   const size_t size) {
    button.Debounce();
    toggle.Debounce();
    hw.ProcessAllControls();

    bool toggle_state = toggle.Pressed();

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

        // Overdrive pre-reverb if toggle is on
        if (toggle_state) {
            audio_in_l[i] = overdrive_l.Process(audio_in_l[i]);
            audio_in_r[i] = overdrive_r.Process(audio_in_r[i]);
        }

        // Process input through high-pass filters
        hp_filter_l.Process(audio_in_l[i]);
        hp_filter_r.Process(audio_in_r[i]);
        audio_in_l[i] = hp_filter_l.High(); // Use high-pass output
        audio_in_r[i] = hp_filter_r.High(); // Use high-pass output

        const float noise_l_out = noise_l.Process(audio_in_l[i]) * NOISE_FACTOR * jitter_mix_level;
        const float noise_r_out = noise_r.Process(audio_in_r[i]) * NOISE_FACTOR * jitter_mix_level;

        reverb.Process((audio_in_l[i] + noise_l_out) * wet_level, (audio_in_r[i] + noise_r_out) * wet_level,
                       &audio_in_l[i], &audio_in_r[i]);

        const float jitter_out = jitter.Process();

        audio_out_l[i] = dry_l + audio_in_l[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);
        audio_out_r[i] = dry_r + audio_in_r[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);

        // Overdrive post-reverb if toggle is off
        if (!toggle_state) {
            audio_out_l[i] = overdrive_l.Process(audio_out_l[i]);
            audio_out_r[i] = overdrive_r.Process(audio_out_r[i]);
        }
    }

    limiter.ProcessBlock(audio_out_l, size, 1.1f);
    limiter.ProcessBlock(audio_out_r, size, 1.1f);

    for (size_t i = 0; i < size; i++) {
        out[0][i] = audio_out_l[i];
        out[1][i] = audio_out_r[i];
    }

    midi.sysex_send_buffer();
}

int main() {
    hw.Init();

    // Restore settings
    storage.Init({SETTINGS_VERSION, false}, 0x2B000);
    const Settings &settings = storage.GetSettings();
    // Only load if we have a settings version match
    if (settings.version == SETTINGS_VERSION) {
        if (settings.is_overdrive_enabled) {
            enable_overdrive = true;
        } else {
            enable_overdrive = false;
        }
        hw.WriteCvOut(CV_OUT_2, enable_overdrive ? 5.0f : 0.0f);
    }


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

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        if (save_settings) {
            Settings &localSettings = storage.GetSettings();
            localSettings.is_overdrive_enabled = enable_overdrive;
            storage.Save();
            save_settings = false;
        }
        System::Delay(250);
    }
}
