#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "lib/daisy_midi.h"
#include <random>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#define AUDIO_BLOCK_SIZE 32

DaisyPatchSM hw;
DaisyMidi midi;
Switch button;
Switch toggle;

Limiter limiter;
Jitter jitter;
ReverbSc reverb;

bool startup = true;
bool button_pressed = false;
float send_level = 0.0f;
float jitter_mix_level = 0.0f;
float cv_knobs[4];

std::random_device random_device;
std::mt19937 mt(random_device());
std::uniform_real_distribution<float> rd(0.0f, 1.0f);

// ReSharper disable once CppParameterMayBeConst
void AudioCallback(const AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   const size_t size) {
    button.Debounce();
    toggle.Debounce();
    hw.ProcessAllControls();

    bool toggle_state = toggle.Pressed();

    float cv_values[4] = {
        hw.GetAdcValue(CV_1), hw.GetAdcValue(CV_2),
        hw.GetAdcValue(CV_3), hw.GetAdcValue(CV_4)
    };
    for (size_t i = 0; i < 4; i++) {
        cv_values[i] = roundf(cv_values[i] * 50) / 50.0f;
        if (cv_values[i] != cv_knobs[i] || startup) {
            // midi.sysex_printf_buffer("env: %2.3f, CV_%d: %f\n", env.Process(), i +
            // 1,
            //                          cv_values[i]);
            cv_knobs[i] = cv_values[i];
            if (!toggle_state) {
                // toggle is down
                switch (i) {
                    case 0:
                        // Reverb send level
                        send_level = cv_values[i];
                        break;
                    case 1:
                        // Jitter amount
                        jitter_mix_level = cv_values[i];
                        break;
                    case 2:
                        // Reverb feedback amount
                        reverb.SetFeedback(fmap(cv_values[i], 0.2f, 1.00f));
                        break;
                    case 3:
                        reverb.SetLpFreq(fmap(cv_values[i], 1000.f, 19000.f, Mapping::LOG));
                        break;
                    // ReSharper disable once CppDFAUnreachableCode
                    default: break;
                };
            } else {
                // Not sure what to do when toggle is up, maybe nothing, so the
                // knobs won't mess with the sound?
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
    } else if (!button.Pressed()) {
        button_pressed = false;
    }

    float audio_in_l[size];
    float audio_in_r[size];

    for (size_t i = 0; i < size; i++) {
        audio_in_l[i] = in[0][i];
        audio_in_r[i] = in[1][i];
    }

    limiter.ProcessBlock(audio_in_l, size, 0.9f);
    limiter.ProcessBlock(audio_in_r, size, 0.9f);

    for (size_t i = 0; i < size; i++) {
        reverb.Process(audio_in_l[i] * send_level, audio_in_r[i] * send_level, &audio_in_l[i], &audio_in_r[i]);

        const float jitter_out = jitter.Process();

        out[0][i] = audio_in_l[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);
        out[1][i] = audio_in_r[i] * (1 - jitter_mix_level + jitter_out * jitter_mix_level);
    }

    midi.sysex_send_buffer();
}

int main() {
    hw.Init();

    button.Init(DaisyPatchSM::B7);
    toggle.Init(DaisyPatchSM::B8);

    limiter.Init();
    jitter.Init(hw.AudioSampleRate());
    jitter.SetAmp(1);
    jitter.SetCpsMin(1);
    jitter.SetCpsMax(25);
    reverb.Init(hw.AudioSampleRate());
    midi.Init();

    hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
    hw.StartAudio(AudioCallback);

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
    }
}
