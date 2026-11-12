#include <unity.h>

#include "core/screen_timer.h"

void setUp() {}
void tearDown() {}

static void test_screen_turns_off_once_after_the_timeout() {
    ScreenTimer timer;
    timer.setTimeout(30000);
    timer.activity(1000);
    TEST_ASSERT_FALSE(timer.expire(30999));
    TEST_ASSERT_FALSE(timer.off());
    TEST_ASSERT_TRUE(timer.expire(31000));
    TEST_ASSERT_TRUE(timer.off());
    TEST_ASSERT_FALSE(timer.expire(40000));
}

static void test_activity_restarts_the_timeout() {
    ScreenTimer timer;
    timer.setTimeout(15000);
    timer.activity(0);
    TEST_ASSERT_FALSE(timer.activity(10000));
    TEST_ASSERT_FALSE(timer.expire(24999));
    TEST_ASSERT_TRUE(timer.expire(25000));
}

static void test_activity_turns_the_screen_back_on() {
    ScreenTimer timer;
    timer.setTimeout(15000);
    timer.activity(0);
    timer.expire(15000);
    TEST_ASSERT_TRUE(timer.activity(20000));
    TEST_ASSERT_FALSE(timer.off());
    TEST_ASSERT_FALSE(timer.expire(34999));
    TEST_ASSERT_TRUE(timer.expire(35000));
}

static void test_never_keeps_the_screen_on() {
    ScreenTimer timer;
    timer.setTimeout(0);
    timer.activity(0);
    TEST_ASSERT_FALSE(timer.expire(0xF0000000u));
    TEST_ASSERT_FALSE(timer.off());
}

static void test_timeout_changed_counts_from_the_last_activity() {
    ScreenTimer timer;
    timer.setTimeout(300000);
    timer.activity(0);
    timer.setTimeout(15000);
    TEST_ASSERT_TRUE(timer.expire(15000));
}

static void test_timeout_survives_clock_wrap() {
    ScreenTimer timer;
    timer.setTimeout(15000);
    timer.activity(0xFFFFF000u);
    TEST_ASSERT_FALSE(timer.expire(0x00000100u));
    TEST_ASSERT_TRUE(timer.expire(0xFFFFF000u + 15000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_screen_turns_off_once_after_the_timeout);
    RUN_TEST(test_activity_restarts_the_timeout);
    RUN_TEST(test_activity_turns_the_screen_back_on);
    RUN_TEST(test_never_keeps_the_screen_on);
    RUN_TEST(test_timeout_changed_counts_from_the_last_activity);
    RUN_TEST(test_timeout_survives_clock_wrap);
    return UNITY_END();
}
