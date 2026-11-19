#include <unity.h>

#include "core/wall_clock.h"

void setUp() {}
void tearDown() {}

// Current Time: year (LE), month, day, hours, minutes, seconds, day of week, fractions, adjust reason.
static const uint8_t kAt235930[] = {0xEA, 0x07, 11, 19, 23, 59, 30, 4, 0, 0};

static void test_an_unset_clock_is_unknown() {
    WallClock clock;
    TEST_ASSERT_FALSE(clock.known());
    TEST_ASSERT_EQUAL(-1, clock.minuteOfDay(1000));
}

static void test_the_iphone_time_sets_the_clock() {
    WallClock clock;
    TEST_ASSERT_TRUE(clock.setFromCurrentTime(kAt235930, sizeof(kAt235930), 1000));
    TEST_ASSERT_TRUE(clock.known());
    TEST_ASSERT_EQUAL(23 * 60 + 59, clock.minuteOfDay(1000));
}

static void test_the_clock_keeps_running_past_midnight() {
    WallClock clock;
    clock.setFromCurrentTime(kAt235930, sizeof(kAt235930), 1000);
    TEST_ASSERT_EQUAL(0, clock.minuteOfDay(1000 + 30 * 1000));
    TEST_ASSERT_EQUAL(61, clock.minuteOfDay(1000 + 30 * 1000 + 61 * 60 * 1000));
}

static void test_the_clock_survives_the_millis_wrap() {
    WallClock clock;
    clock.setFromCurrentTime(kAt235930, sizeof(kAt235930), 0xFFFFFFFFu - 9999);
    TEST_ASSERT_EQUAL(0, clock.minuteOfDay(20000));
}

static void test_a_short_or_invalid_time_is_refused() {
    WallClock clock;
    TEST_ASSERT_FALSE(clock.setFromCurrentTime(kAt235930, 6, 0));
    const uint8_t badHour[] = {0xEA, 0x07, 11, 19, 24, 0, 0, 4, 0, 0};
    TEST_ASSERT_FALSE(clock.setFromCurrentTime(badHour, sizeof(badHour), 0));
    TEST_ASSERT_FALSE(clock.known());
}

static void test_minutes_read_as_hh_mm() {
    TEST_ASSERT_EQUAL_STRING("00:00", clockLabel(0).c_str());
    TEST_ASSERT_EQUAL_STRING("09:05", clockLabel(9 * 60 + 5).c_str());
    TEST_ASSERT_EQUAL_STRING("23:59", clockLabel(23 * 60 + 59).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_an_unset_clock_is_unknown);
    RUN_TEST(test_the_iphone_time_sets_the_clock);
    RUN_TEST(test_the_clock_keeps_running_past_midnight);
    RUN_TEST(test_the_clock_survives_the_millis_wrap);
    RUN_TEST(test_a_short_or_invalid_time_is_refused);
    RUN_TEST(test_minutes_read_as_hh_mm);
    return UNITY_END();
}
