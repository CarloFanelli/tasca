#include <unity.h>

#include "core/signal_meter.h"

void setUp() {}
void tearDown() {}

static void test_no_reading_means_searching() {
    SignalMeter meter;
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::None), static_cast<int>(meter.level(0)));
    TEST_ASSERT_EQUAL(0, meter.barPercent(0));
    TEST_ASSERT_EQUAL_UINT32(0, beepPeriodMs(SignalLevel::None));
}

static void test_levels_follow_the_signal() {
    SignalMeter far;
    far.add(-85, 0);
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::Far), static_cast<int>(far.level(0)));
    SignalMeter near;
    near.add(-73, 0);
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::Near), static_cast<int>(near.level(0)));
    SignalMeter close;
    close.add(-45, 0);
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::VeryClose), static_cast<int>(close.level(0)));
}

static void test_readings_are_averaged_over_2_s() {
    SignalMeter meter;
    meter.add(-60, 0);
    meter.add(-90, 500);
    // (-60 - 90) / 2 = -75: Near, not Very close nor Far.
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::Near), static_cast<int>(meter.level(500)));
    // The first reading is older than 2 s: only -90 is left.
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::Far), static_cast<int>(meter.level(2001)));
    // None left after 2 s more.
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::None), static_cast<int>(meter.level(2501)));
}

static void test_zero_is_not_a_reading() {
    SignalMeter meter;
    meter.add(0, 0);
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::None), static_cast<int>(meter.level(0)));
}

static void test_bar_grows_closer_and_stays_in_range() {
    SignalMeter weak;
    weak.add(-120, 0);
    TEST_ASSERT_EQUAL(0, weak.barPercent(0));
    SignalMeter mid;
    mid.add(-60, 0);
    TEST_ASSERT_EQUAL(50, mid.barPercent(0));
    SignalMeter strong;
    strong.add(-20, 0);
    TEST_ASSERT_EQUAL(100, strong.barPercent(0));
}

static void test_beep_gets_faster_closer() {
    TEST_ASSERT_EQUAL_UINT32(2000, beepPeriodMs(SignalLevel::Far));
    TEST_ASSERT_EQUAL_UINT32(1000, beepPeriodMs(SignalLevel::Near));
    TEST_ASSERT_EQUAL_UINT32(250, beepPeriodMs(SignalLevel::VeryClose));
}

static void test_more_readings_than_kept_drop_the_oldest() {
    SignalMeter meter;
    for (uint32_t i = 0; i < 40; i++) {
        meter.add(i < 28 ? -90 : -40, i * 10);
    }
    // The 16 kept are 4 at -90 and 12 at -40: Very close. All 40 would average -75, Near.
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::VeryClose), static_cast<int>(meter.level(400)));
}

static void test_survives_clock_wrap() {
    SignalMeter meter;
    meter.add(-45, 0xFFFFFF00u);
    TEST_ASSERT_EQUAL(static_cast<int>(SignalLevel::VeryClose), static_cast<int>(meter.level(0x00000100u)));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_no_reading_means_searching);
    RUN_TEST(test_levels_follow_the_signal);
    RUN_TEST(test_readings_are_averaged_over_2_s);
    RUN_TEST(test_zero_is_not_a_reading);
    RUN_TEST(test_bar_grows_closer_and_stays_in_range);
    RUN_TEST(test_beep_gets_faster_closer);
    RUN_TEST(test_more_readings_than_kept_drop_the_oldest);
    RUN_TEST(test_survives_clock_wrap);
    return UNITY_END();
}
