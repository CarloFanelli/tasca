#include <unity.h>

#include "core/out_of_range.h"

void setUp() {}
void tearDown() {}

static void test_nothing_is_due_before_a_start() {
    OutOfRangeAlert alert;
    TEST_ASSERT_FALSE(alert.active());
    TEST_ASSERT_FALSE(alert.due(0));
}

static void test_due_at_start_then_every_5_s() {
    OutOfRangeAlert alert;
    alert.start(1000);
    TEST_ASSERT_TRUE(alert.active());
    TEST_ASSERT_TRUE(alert.due(1000));
    TEST_ASSERT_FALSE(alert.due(5999));
    TEST_ASSERT_TRUE(alert.due(6000));
    TEST_ASSERT_FALSE(alert.due(6000));
}

static void test_beeps_end_after_2_minutes_but_the_alert_stays() {
    OutOfRangeAlert alert;
    alert.start(0);
    int beeps = 0;
    for (uint32_t ms = 0; ms <= 200000; ms += 100) {
        beeps += alert.due(ms) ? 1 : 0;
    }
    TEST_ASSERT_EQUAL(24, beeps);
    TEST_ASSERT_TRUE(alert.active());
}

static void test_silence_ends_the_beeps_only_once() {
    OutOfRangeAlert alert;
    alert.start(0);
    alert.due(0);
    TEST_ASSERT_TRUE(alert.silence());
    TEST_ASSERT_FALSE(alert.silence());
    TEST_ASSERT_FALSE(alert.due(5000));
    TEST_ASSERT_TRUE(alert.active());
}

static void test_stop_ends_the_alert() {
    OutOfRangeAlert alert;
    alert.start(0);
    alert.stop();
    TEST_ASSERT_FALSE(alert.active());
    TEST_ASSERT_FALSE(alert.due(5000));
}

static void test_survives_clock_wrap() {
    OutOfRangeAlert alert;
    alert.start(0xFFFFF000u);
    TEST_ASSERT_TRUE(alert.due(0xFFFFF000u));
    TEST_ASSERT_TRUE(alert.due(0xFFFFF000u + 5000));
    TEST_ASSERT_FALSE(alert.due(0xFFFFF000u + 9999));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_nothing_is_due_before_a_start);
    RUN_TEST(test_due_at_start_then_every_5_s);
    RUN_TEST(test_beeps_end_after_2_minutes_but_the_alert_stays);
    RUN_TEST(test_silence_ends_the_beeps_only_once);
    RUN_TEST(test_stop_ends_the_alert);
    RUN_TEST(test_survives_clock_wrap);
    return UNITY_END();
}
