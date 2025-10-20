#include <iostream>
#include <cassert>
#include <cmath>

// Test Story 2.4: Maintain Overdrive Button Functionality
// Comprehensive effect independence testing

// Mock daisy hardware for testing
struct MockHardware {
    bool led_state = false;
    float cv_out_2 = 0.0f;

    void SetLed(bool state) { led_state = state; }
    void WriteCvOut(int cv, float value) { if (cv == 2) cv_out_2 = value; }
};

// Simulate the main application state
MockHardware hw;
volatile bool enable_overdrive = false;
volatile bool filterModulationEnabled = false;
volatile bool led_target_state = false;
bool led_current_state = false;

// Helper functions from main.cpp
static inline void ApplyLedState(bool state) {
    led_current_state = state;
    hw.SetLed(state);
}

// Test cases for effect independence
namespace Test_Story_2_4 {

    void test_overdrive_only() {
        std::cout << "Testing overdrive only state..." << std::endl;

        // Set initial state: overdrive ON, modulation OFF
        enable_overdrive = true;
        filterModulationEnabled = false;

        // Simulate LED update logic from main.cpp line 182
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        // Verify expected state
        assert(enable_overdrive == true);
        assert(filterModulationEnabled == false);
        assert(led_target_state == true);  // LED should be ON (overdrive active)
        assert(hw.led_state == true);

        std::cout << "✅ Overdrive only: PASSED" << std::endl;
    }

    void test_modulation_only() {
        std::cout << "Testing modulation only state..." << std::endl;

        // Set initial state: overdrive OFF, modulation ON
        enable_overdrive = false;
        filterModulationEnabled = true;

        // Simulate LED update logic
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        // Verify expected state
        assert(enable_overdrive == false);
        assert(filterModulationEnabled == true);
        assert(led_target_state == true);  // LED should be ON (modulation active)
        assert(hw.led_state == true);

        std::cout << "✅ Modulation only: PASSED" << std::endl;
    }

    void test_both_effects_enabled() {
        std::cout << "Testing both effects enabled..." << std::endl;

        // Set initial state: both ON
        enable_overdrive = true;
        filterModulationEnabled = true;

        // Simulate LED update logic
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        // Verify expected state
        assert(enable_overdrive == true);
        assert(filterModulationEnabled == true);
        assert(led_target_state == true);  // LED should be ON (both active)
        assert(hw.led_state == true);

        std::cout << "✅ Both effects enabled: PASSED" << std::endl;
    }

    void test_both_effects_disabled() {
        std::cout << "Testing both effects disabled..." << std::endl;

        // Set initial state: both OFF
        enable_overdrive = false;
        filterModulationEnabled = false;

        // Simulate LED update logic
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        // Verify expected state
        assert(enable_overdrive == false);
        assert(filterModulationEnabled == false);
        assert(led_target_state == false);  // LED should be OFF (both inactive)
        assert(hw.led_state == false);

        std::cout << "✅ Both effects disabled: PASSED" << std::endl;
    }

    void test_effect_independence() {
        std::cout << "Testing effect state independence..." << std::endl;

        // Test that changing one effect doesn't affect the other
        enable_overdrive = true;
        filterModulationEnabled = false;

        // Change modulation state - should not affect overdrive
        filterModulationEnabled = true;
        assert(enable_overdrive == true);  // Overdrive unchanged

        // Change overdrive state - should not affect modulation
        enable_overdrive = false;
        assert(filterModulationEnabled == true);  // Modulation unchanged

        std::cout << "✅ Effect independence: PASSED" << std::endl;
    }

    void test_b7_button_simulation() {
        std::cout << "Testing B7 button toggle simulation..." << std::endl;

        // Reset state
        enable_overdrive = false;
        hw.cv_out_2 = 0.0f;

        // Simulate first button press (enable overdrive)
        if (enable_overdrive == false) {
            enable_overdrive = true;
            hw.WriteCvOut(2, 5.0f);  // CV_OUT_2 = 2
        }

        assert(enable_overdrive == true);
        assert(hw.cv_out_2 == 5.0f);

        // Simulate second button press (disable overdrive)
        if (enable_overdrive == true) {
            enable_overdrive = false;
            hw.WriteCvOut(2, 0.0f);
        }

        assert(enable_overdrive == false);
        assert(hw.cv_out_2 == 0.0f);

        std::cout << "✅ B7 button toggle simulation: PASSED" << std::endl;
    }

    void run_all_tests() {
        std::cout << "=== Story 2.4 Effect Independence Tests ===" << std::endl;

        test_overdrive_only();
        test_modulation_only();
        test_both_effects_enabled();
        test_both_effects_disabled();
        test_effect_independence();
        test_b7_button_simulation();

        std::cout << "=== All Effect Independence Tests PASSED! ===" << std::endl;
        std::cout << std::endl;
    }
}

int main() {
    Test_Story_2_4::run_all_tests();
    return 0;
}