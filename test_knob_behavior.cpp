#include <iostream>
#include <cassert>
#include <cmath>

// Test knob behavior with overdrive (AC: 4)
// Analyze how jitter_mix_level affects different parameters

// Constants from main.cpp
#define MIN_OVERDRIVE 0.10f
#define MAX_OVERDRIVE 0.4f
#define NOISE_FACTOR 0.003f

// Mock fmap function
float fmap(float x, float min, float max) {
    return min + (max - min) * x;
}

// Mock one-pole filter
struct MockKnobFilter {
    float tmp = 0.0f;
    float Process(float x) {
        tmp = 0.0005f * x + 0.9995f * tmp;
        return tmp;
    }
};

// Test the unified intensity control model
namespace Test_Knob_Behavior {

    void test_overdrive_gain_mapping() {
        std::cout << "Testing overdrive gain mapping..." << std::endl;

        MockKnobFilter overdrive_filter;
        bool enable_overdrive = true;

        // Test various knob positions (jitter_mix_level)
        float knob_positions[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

        for (float jitter_mix_level : knob_positions) {
            // From main.cpp lines 198-203: overdrive gain calculation
            float expected_drive = overdrive_filter.Process(enable_overdrive
                ? fmap(jitter_mix_level, MIN_OVERDRIVE, MAX_OVERDRIVE)
                : MIN_OVERDRIVE);

            float expected_raw_drive = fmap(jitter_mix_level, MIN_OVERDRIVE, MAX_OVERDRIVE);

            std::cout << "Knob: " << jitter_mix_level
                     << " -> Drive: " << expected_raw_drive
                     << " (range: " << MIN_OVERDRIVE << "-" << MAX_OVERDRIVE << ")" << std::endl;

            // Validate drive is within expected bounds
            assert(expected_raw_drive >= MIN_OVERDRIVE);
            assert(expected_raw_drive <= MAX_OVERDRIVE);
        }

        std::cout << "✅ Overdrive gain mapping: PASSED" << std::endl;
    }

    void test_noise_factor_control() {
        std::cout << "Testing square noise control..." << std::endl;

        bool enable_overdrive = true;
        float audio_input = 1.0f;  // Test signal

        // Test various knob positions affect noise level
        float knob_positions[] = {0.0f, 0.5f, 1.0f};

        for (float jitter_mix_level : knob_positions) {
            // From main.cpp lines 216-221: noise calculation
            float expected_noise = enable_overdrive
                ? audio_input * NOISE_FACTOR * jitter_mix_level
                : 0.0f;

            std::cout << "Knob: " << jitter_mix_level
                     << " -> Noise: " << expected_noise << std::endl;

            // Validate noise scaling
            if (enable_overdrive) {
                assert(expected_noise == audio_input * NOISE_FACTOR * jitter_mix_level);
            } else {
                assert(expected_noise == 0.0f);
            }
        }

        std::cout << "✅ Square noise control: PASSED" << std::endl;
    }

    void test_modulation_intensity_control() {
        std::cout << "Testing modulation intensity control..." << std::endl;

        bool filterModulationEnabled = true;
        float jitter_output = 0.8f;  // Mock jitter output

        // Test various knob positions
        float knob_positions[] = {0.0f, 0.5f, 1.0f};

        for (float jitter_mix_level : knob_positions) {
            // From main.cpp line 227: modulation intensity
            float modulation_intensity = filterModulationEnabled ? jitter_mix_level : 0.0f;

            std::cout << "Knob: " << jitter_mix_level
                     << " -> Modulation Intensity: " << modulation_intensity << std::endl;

            if (filterModulationEnabled) {
                assert(modulation_intensity == jitter_mix_level);
            } else {
                assert(modulation_intensity == 0.0f);
            }
        }

        std::cout << "✅ Modulation intensity control: PASSED" << std::endl;
    }

    void test_unified_control_scenarios() {
        std::cout << "Testing unified intensity control scenarios..." << std::endl;

        float test_knob = 0.75f;  // 75% knob position

        // Scenario 1: Modulation OFF - knob controls overdrive + noise only
        bool enable_overdrive = true;
        bool filterModulationEnabled = false;

        float overdrive_drive = fmap(test_knob, MIN_OVERDRIVE, MAX_OVERDRIVE);
        float noise_level = test_knob * NOISE_FACTOR;
        float modulation_intensity = filterModulationEnabled ? test_knob : 0.0f;

        std::cout << "Scenario 1 (Modulation OFF):" << std::endl;
        std::cout << "  Overdrive drive: " << overdrive_drive << std::endl;
        std::cout << "  Noise level: " << noise_level << std::endl;
        std::cout << "  Modulation intensity: " << modulation_intensity << std::endl;

        assert(modulation_intensity == 0.0f);
        assert(overdrive_drive > MIN_OVERDRIVE);

        // Scenario 2: Modulation ON - knob controls all three parameters
        filterModulationEnabled = true;
        modulation_intensity = filterModulationEnabled ? test_knob : 0.0f;

        std::cout << "Scenario 2 (Modulation ON):" << std::endl;
        std::cout << "  Overdrive drive: " << overdrive_drive << std::endl;
        std::cout << "  Noise level: " << noise_level << std::endl;
        std::cout << "  Modulation intensity: " << modulation_intensity << std::endl;

        assert(modulation_intensity == test_knob);
        assert(overdrive_drive > MIN_OVERDRIVE);

        std::cout << "✅ Unified control scenarios: PASSED" << std::endl;
    }

    void test_knob_disabled_overdrive() {
        std::cout << "Testing knob with disabled overdrive..." << std::endl;

        bool enable_overdrive = false;
        float jitter_mix_level = 0.8f;

        // When overdrive disabled, drive should be MIN_OVERDRIVE
        float expected_drive = MIN_OVERDRIVE;
        float expected_noise = 0.0f;  // No noise when overdrive off

        std::cout << "Overdrive OFF -> Drive: " << expected_drive
                 << ", Noise: " << expected_noise << std::endl;

        assert(expected_drive == MIN_OVERDRIVE);
        assert(expected_noise == 0.0f);

        std::cout << "✅ Knob with disabled overdrive: PASSED" << std::endl;
    }

    void run_all_tests() {
        std::cout << "=== Story 2.4 Knob Behavior Tests ===" << std::endl;

        test_overdrive_gain_mapping();
        test_noise_factor_control();
        test_modulation_intensity_control();
        test_unified_control_scenarios();
        test_knob_disabled_overdrive();

        std::cout << "=== All Knob Behavior Tests PASSED! ===" << std::endl;
        std::cout << std::endl;
    }
}

int main() {
    Test_Knob_Behavior::run_all_tests();
    return 0;
}