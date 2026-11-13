#include <unity.h>

#include "core/siri_press.h"

void setUp() {}
void tearDown() {}

static void test_start_asks_for_the_wake_tap_first() {
    SiriPress press;
    TEST_ASSERT_TRUE(press.start(1000));
    TEST_ASSERT_TRUE(press.active());
    TEST_ASSERT_EQUAL(SiriAction::None, press.update(1000 + SiriPress::kWakeMs - 1, true));
}

static void test_hold_starts_after_the_wake_delay() {
    SiriPress press;
    press.start(1000);
    TEST_ASSERT_EQUAL(SiriAction::StartHold, press.update(1000 + SiriPress::kWakeMs, true));
    TEST_ASSERT_EQUAL(SiriAction::None, press.update(1000 + SiriPress::kWakeMs + 10, true));
}

static void test_hold_ends_when_enter_is_released() {
    SiriPress press;
    press.start(0);
    press.update(SiriPress::kWakeMs, true);
    const uint32_t later = SiriPress::kWakeMs + SiriPress::kMinHoldMs + 500;
    TEST_ASSERT_EQUAL(SiriAction::None, press.update(later, true));
    TEST_ASSERT_EQUAL(SiriAction::EndHold, press.update(later + 1, false));
    TEST_ASSERT_FALSE(press.active());
}

static void test_short_press_still_holds_for_the_minimum() {
    SiriPress press;
    press.start(0);
    // Enter released before the hold started: the hold still starts and lasts the minimum.
    TEST_ASSERT_EQUAL(SiriAction::None, press.update(50, false));
    TEST_ASSERT_EQUAL(SiriAction::StartHold, press.update(SiriPress::kWakeMs, false));
    TEST_ASSERT_EQUAL(SiriAction::None,
                      press.update(SiriPress::kWakeMs + SiriPress::kMinHoldMs - 1, false));
    TEST_ASSERT_EQUAL(SiriAction::EndHold, press.update(SiriPress::kWakeMs + SiriPress::kMinHoldMs, false));
}

static void test_start_is_refused_while_a_press_is_active() {
    SiriPress press;
    TEST_ASSERT_TRUE(press.start(0));
    TEST_ASSERT_FALSE(press.start(10));
}

static void test_cancel_ends_an_active_hold() {
    SiriPress press;
    press.start(0);
    press.update(SiriPress::kWakeMs, true);
    TEST_ASSERT_TRUE(press.cancel());
    TEST_ASSERT_FALSE(press.active());
    TEST_ASSERT_FALSE(press.cancel());
}

static void test_clock_wrap() {
    SiriPress press;
    const uint32_t start = 0xFFFFFFFFu - 100;
    press.start(start);
    TEST_ASSERT_EQUAL(SiriAction::StartHold, press.update(start + SiriPress::kWakeMs, true));
}

static void test_back_right_after_siri_closes_it_once() {
    SiriCloseWindow window;
    window.open(1000);
    TEST_ASSERT_TRUE(window.keyDown(2000, true));
    TEST_ASSERT_FALSE(window.keyDown(2100, true));
}

static void test_another_key_first_ends_the_window() {
    SiriCloseWindow window;
    window.open(0);
    TEST_ASSERT_FALSE(window.keyDown(100, false));
    TEST_ASSERT_FALSE(window.keyDown(200, true));
}

static void test_back_after_the_window_is_its_usual_key() {
    SiriCloseWindow window;
    window.open(0);
    TEST_ASSERT_FALSE(window.keyDown(SiriCloseWindow::kOpenMs, true));
}

static void test_back_without_siri_is_its_usual_key() {
    SiriCloseWindow window;
    TEST_ASSERT_FALSE(window.keyDown(0, true));
}

static void test_close_window_survives_clock_wrap() {
    SiriCloseWindow window;
    window.open(0xFFFFFFFFu - 10);
    TEST_ASSERT_TRUE(window.keyDown(20, true));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_start_asks_for_the_wake_tap_first);
    RUN_TEST(test_hold_starts_after_the_wake_delay);
    RUN_TEST(test_hold_ends_when_enter_is_released);
    RUN_TEST(test_short_press_still_holds_for_the_minimum);
    RUN_TEST(test_start_is_refused_while_a_press_is_active);
    RUN_TEST(test_cancel_ends_an_active_hold);
    RUN_TEST(test_clock_wrap);
    RUN_TEST(test_back_right_after_siri_closes_it_once);
    RUN_TEST(test_another_key_first_ends_the_window);
    RUN_TEST(test_back_after_the_window_is_its_usual_key);
    RUN_TEST(test_back_without_siri_is_its_usual_key);
    RUN_TEST(test_close_window_survives_clock_wrap);
    return UNITY_END();
}
