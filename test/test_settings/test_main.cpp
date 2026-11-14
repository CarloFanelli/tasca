#include <unity.h>

#include "core/settings.h"
#include "core/settings_nav.h"

void setUp() {}
void tearDown() {}

static void test_defaults_are_60_percent_sound_on_30_s_wake_on() {
    Settings settings;
    TEST_ASSERT_EQUAL_STRING("60 %", settings.label(Setting::Brightness).c_str());
    TEST_ASSERT_EQUAL_STRING("On", settings.label(Setting::Sound).c_str());
    TEST_ASSERT_EQUAL_STRING("30 s", settings.label(Setting::DisplayTimeout).c_str());
    TEST_ASSERT_EQUAL_STRING("On", settings.label(Setting::WakeOnMessage).c_str());
    TEST_ASSERT_EQUAL(60, settings.brightnessPercent());
}

static void test_brightness_steps_by_20_between_20_and_100() {
    Settings settings;
    TEST_ASSERT_TRUE(settings.step(Setting::Brightness, -1));
    TEST_ASSERT_TRUE(settings.step(Setting::Brightness, -1));
    TEST_ASSERT_EQUAL(20, settings.brightnessPercent());
    TEST_ASSERT_FALSE(settings.step(Setting::Brightness, -1));
    TEST_ASSERT_EQUAL(20, settings.brightnessPercent());
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_TRUE(settings.step(Setting::Brightness, 1));
    }
    TEST_ASSERT_EQUAL_STRING("100 %", settings.label(Setting::Brightness).c_str());
    TEST_ASSERT_FALSE(settings.step(Setting::Brightness, 1));
}

static void test_display_timeout_offers_six_values_ending_with_never() {
    Settings settings;
    TEST_ASSERT_TRUE(settings.step(Setting::DisplayTimeout, -1));
    TEST_ASSERT_EQUAL_STRING("15 s", settings.label(Setting::DisplayTimeout).c_str());
    TEST_ASSERT_FALSE(settings.step(Setting::DisplayTimeout, -1));
    const char* expected[] = {"30 s", "1 min", "2 min", "5 min", "Never"};
    for (const char* label : expected) {
        TEST_ASSERT_TRUE(settings.step(Setting::DisplayTimeout, 1));
        TEST_ASSERT_EQUAL_STRING(label, settings.label(Setting::DisplayTimeout).c_str());
    }
    TEST_ASSERT_FALSE(settings.step(Setting::DisplayTimeout, 1));
}

static void test_on_off_settings_stop_at_both_ends() {
    Settings settings;
    TEST_ASSERT_FALSE(settings.step(Setting::Sound, 1));
    TEST_ASSERT_TRUE(settings.step(Setting::Sound, -1));
    TEST_ASSERT_EQUAL_STRING("Off", settings.label(Setting::Sound).c_str());
    TEST_ASSERT_FALSE(settings.step(Setting::Sound, -1));
}

static void test_values_read_by_the_firmware() {
    Settings settings;
    TEST_ASSERT_TRUE(settings.sound());
    TEST_ASSERT_TRUE(settings.wakeOnMessage());
    TEST_ASSERT_EQUAL_UINT32(30000, settings.displayTimeoutMs());
    settings.step(Setting::Sound, -1);
    settings.step(Setting::WakeOnMessage, -1);
    for (int i = 0; i < 4; i++) {
        settings.step(Setting::DisplayTimeout, 1);
    }
    TEST_ASSERT_FALSE(settings.sound());
    TEST_ASSERT_FALSE(settings.wakeOnMessage());
    TEST_ASSERT_EQUAL_UINT32(0, settings.displayTimeoutMs());
}

static void test_stored_value_is_restored() {
    Settings settings;
    settings.step(Setting::Brightness, 1);
    Settings restored;
    TEST_ASSERT_TRUE(restored.setValueIndex(Setting::Brightness, settings.valueIndex(Setting::Brightness)));
    TEST_ASSERT_EQUAL(80, restored.brightnessPercent());
}

static void test_stored_value_out_of_range_keeps_the_default() {
    Settings settings;
    TEST_ASSERT_FALSE(settings.setValueIndex(Setting::Brightness, 5));
    TEST_ASSERT_FALSE(settings.setValueIndex(Setting::Sound, 2));
    TEST_ASSERT_EQUAL(60, settings.brightnessPercent());
    TEST_ASSERT_EQUAL_STRING("On", settings.label(Setting::Sound).c_str());
}

namespace {

constexpr uint32_t kConfirmMs = 3000;

// Two settings, then one action.
SettingsNav onAction() {
    SettingsNav nav(2, 1, kConfirmMs);
    nav.down();
    nav.down();
    return nav;
}

}  // namespace

static void test_selection_moves_and_stops_at_the_ends() {
    SettingsNav nav(2, 2, kConfirmMs);
    TEST_ASSERT_EQUAL(0, nav.selected());
    TEST_ASSERT_FALSE(nav.up());
    TEST_ASSERT_TRUE(nav.down());
    TEST_ASSERT_TRUE(nav.down());
    TEST_ASSERT_TRUE(nav.down());
    TEST_ASSERT_FALSE(nav.down());
    TEST_ASSERT_EQUAL(3, nav.selected());
    TEST_ASSERT_TRUE(nav.up());
    TEST_ASSERT_EQUAL(2, nav.selected());
}

static void test_actions_follow_the_settings() {
    SettingsNav nav(2, 1, kConfirmMs);
    TEST_ASSERT_FALSE(nav.onAction());
    nav.down();
    TEST_ASSERT_FALSE(nav.onAction());
    nav.down();
    TEST_ASSERT_TRUE(nav.onAction());
}

static void test_action_runs_only_on_the_second_enter() {
    SettingsNav nav = onAction();
    TEST_ASSERT_FALSE(nav.enter(100));
    TEST_ASSERT_TRUE(nav.confirming());
    TEST_ASSERT_TRUE(nav.enter(200));
    TEST_ASSERT_FALSE(nav.confirming());
}

static void test_enter_on_a_setting_shows_no_prompt() {
    SettingsNav nav(2, 1, kConfirmMs);
    TEST_ASSERT_FALSE(nav.enter(100));
    TEST_ASSERT_FALSE(nav.confirming());
    TEST_ASSERT_FALSE(nav.enter(200));
}

static void test_cancelled_prompt_needs_two_enters_again() {
    SettingsNav nav = onAction();
    nav.enter(100);
    TEST_ASSERT_TRUE(nav.cancel());
    TEST_ASSERT_FALSE(nav.confirming());
    TEST_ASSERT_FALSE(nav.cancel());
    TEST_ASSERT_FALSE(nav.enter(200));
    TEST_ASSERT_TRUE(nav.confirming());
}

static void test_prompt_expires_after_3_s() {
    SettingsNav nav = onAction();
    nav.enter(1000);
    TEST_ASSERT_FALSE(nav.expire(3999));
    TEST_ASSERT_TRUE(nav.confirming());
    TEST_ASSERT_TRUE(nav.expire(4000));
    TEST_ASSERT_FALSE(nav.confirming());
    TEST_ASSERT_FALSE(nav.expire(5000));
}

static void test_prompt_expiry_survives_clock_wrap() {
    SettingsNav nav = onAction();
    nav.enter(0xFFFFFF00u);
    TEST_ASSERT_FALSE(nav.expire(0x00000010u));
    TEST_ASSERT_TRUE(nav.expire(0xFFFFFF00u + kConfirmMs));
}

static void test_reset_selects_the_first_row_without_prompt() {
    SettingsNav nav(1, 1, 3000);
    nav.down();
    nav.enter(0);
    nav.reset();
    TEST_ASSERT_EQUAL(0, nav.selected());
    TEST_ASSERT_FALSE(nav.confirming());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_defaults_are_60_percent_sound_on_30_s_wake_on);
    RUN_TEST(test_brightness_steps_by_20_between_20_and_100);
    RUN_TEST(test_display_timeout_offers_six_values_ending_with_never);
    RUN_TEST(test_on_off_settings_stop_at_both_ends);
    RUN_TEST(test_values_read_by_the_firmware);
    RUN_TEST(test_stored_value_is_restored);
    RUN_TEST(test_stored_value_out_of_range_keeps_the_default);
    RUN_TEST(test_selection_moves_and_stops_at_the_ends);
    RUN_TEST(test_actions_follow_the_settings);
    RUN_TEST(test_action_runs_only_on_the_second_enter);
    RUN_TEST(test_enter_on_a_setting_shows_no_prompt);
    RUN_TEST(test_cancelled_prompt_needs_two_enters_again);
    RUN_TEST(test_prompt_expires_after_3_s);
    RUN_TEST(test_prompt_expiry_survives_clock_wrap);
    RUN_TEST(test_reset_selects_the_first_row_without_prompt);
    return UNITY_END();
}
