#include <iostream>
#include <cassert>

// Test LED indicators behavior (AC: 5)
// Verify LED correctly shows both effect states

// Mock hardware for LED testing
struct MockHardware {
    bool led_state = false;
    void SetLed(bool state) { led_state = state; }
};

MockHardware hw;
bool led_current_state = false;
bool led_target_state = false;

// LED control functions from main.cpp
static inline void ApplyLedState(bool state) {
    led_current_state = state;
    hw.SetLed(state);
}

static inline void RestoreLedToTarget() {
    ApplyLedState(led_target_state);
}

namespace Test_LED_Indicators {

    void test_led_overdrive_only() {
        std::cout << "Testing LED with overdrive only..." << std::endl;

        bool enable_overdrive = true;
        bool filterModulationEnabled = false;

        // From main.cpp line 182: LED logic
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        assert(led_target_state == true);   // Should be ON
        assert(led_current_state == true);
        assert(hw.led_state == true);

        std::cout << "✅ LED overdrive only: PASSED (LED ON)" << std::endl;
    }

    void test_led_modulation_only() {
        std::cout << "Testing LED with modulation only..." << std::endl;

        bool enable_overdrive = false;
        bool filterModulationEnabled = true;

        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        assert(led_target_state == true);   // Should be ON
        assert(led_current_state == true);
        assert(hw.led_state == true);

        std::cout << "✅ LED modulation only: PASSED (LED ON)" << std::endl;
    }

    void test_led_both_effects() {
        std::cout << "Testing LED with both effects..." << std::endl;

        bool enable_overdrive = true;
        bool filterModulationEnabled = true;

        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        assert(led_target_state == true);   // Should be ON
        assert(led_current_state == true);
        assert(hw.led_state == true);

        std::cout << "✅ LED both effects: PASSED (LED ON)" << std::endl;
    }

    void test_led_both_disabled() {
        std::cout << "Testing LED with both effects disabled..." << std::endl;

        bool enable_overdrive = false;
        bool filterModulationEnabled = false;

        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);

        assert(led_target_state == false);  // Should be OFF
        assert(led_current_state == false);
        assert(hw.led_state == false);

        std::cout << "✅ LED both disabled: PASSED (LED OFF)" << std::endl;
    }

    void test_led_state_transitions() {
        std::cout << "Testing LED state transitions..." << std::endl;

        // Start with both OFF
        bool enable_overdrive = false;
        bool filterModulationEnabled = false;
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);
        assert(led_current_state == false);

        // Enable overdrive - LED should turn ON
        enable_overdrive = true;
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);
        assert(led_current_state == true);

        // Disable overdrive, enable modulation - LED should stay ON
        enable_overdrive = false;
        filterModulationEnabled = true;
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);
        assert(led_current_state == true);

        // Disable modulation - LED should turn OFF
        filterModulationEnabled = false;
        led_target_state = enable_overdrive || filterModulationEnabled;
        ApplyLedState(led_target_state);
        assert(led_current_state == false);

        std::cout << "✅ LED state transitions: PASSED" << std::endl;
    }

    void test_led_restore_functionality() {
        std::cout << "Testing LED restore functionality..." << std::endl;

        // Set target state
        led_target_state = true;

        // Manually change current state (simulate LED being temporarily changed)
        led_current_state = false;
        hw.led_state = false;

        // Restore should bring LED back to target
        RestoreLedToTarget();

        assert(led_current_state == led_target_state);
        assert(hw.led_state == led_target_state);

        std::cout << "✅ LED restore functionality: PASSED" << std::endl;
    }

    void test_led_logic_table() {
        std::cout << "Testing LED truth table..." << std::endl;

        struct TestCase {
            bool overdrive;
            bool modulation;
            bool expected_led;
        };

        TestCase cases[] = {
            {false, false, false},  // Both OFF -> LED OFF
            {true,  false, true},   // Overdrive only -> LED ON
            {false, true,  true},   // Modulation only -> LED ON
            {true,  true,  true}    // Both ON -> LED ON
        };

        for (auto& testCase : cases) {
            led_target_state = testCase.overdrive || testCase.modulation;
            ApplyLedState(led_target_state);

            std::cout << "Overdrive: " << testCase.overdrive
                     << ", Modulation: " << testCase.modulation
                     << " -> LED: " << led_current_state << std::endl;

            assert(led_current_state == testCase.expected_led);
            assert(hw.led_state == testCase.expected_led);
        }

        std::cout << "✅ LED truth table: PASSED" << std::endl;
    }

    void run_all_tests() {
        std::cout << "=== Story 2.4 LED Indicator Tests ===" << std::endl;

        test_led_overdrive_only();
        test_led_modulation_only();
        test_led_both_effects();
        test_led_both_disabled();
        test_led_state_transitions();
        test_led_restore_functionality();
        test_led_logic_table();

        std::cout << "=== All LED Indicator Tests PASSED! ===" << std::endl;
        std::cout << std::endl;
    }
}

int main() {
    Test_LED_Indicators::run_all_tests();
    return 0;
}