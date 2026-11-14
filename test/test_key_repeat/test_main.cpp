#include <unity.h>

#include "core/key_repeat.h"

void setUp() {}
void tearDown() {}

static const KeyPos kKey{2, 11};
static const KeyPos kOther{3, 11};

static void test_nothing_is_due_before_a_start() {
    KeyRepeat repeat(200);
    TEST_ASSERT_FALSE(repeat.update({kKey}, 1000));
}

static void test_due_every_interval_while_held() {
    KeyRepeat repeat(200);
    repeat.start(kKey, 0);
    TEST_ASSERT_FALSE(repeat.update({kKey}, 199));
    TEST_ASSERT_TRUE(repeat.update({kKey}, 200));
    TEST_ASSERT_FALSE(repeat.update({kKey}, 350));
    TEST_ASSERT_TRUE(repeat.update({kKey}, 400));
}

static void test_release_stops_until_the_next_start() {
    KeyRepeat repeat(200);
    repeat.start(kKey, 0);
    TEST_ASSERT_FALSE(repeat.update({kOther}, 100));
    TEST_ASSERT_FALSE(repeat.update({kKey}, 300));
    repeat.start(kKey, 300);
    TEST_ASSERT_TRUE(repeat.update({kKey}, 500));
}

static void test_survives_clock_wrap() {
    KeyRepeat repeat(200);
    repeat.start(kKey, 0xFFFFFFF0u);
    TEST_ASSERT_TRUE(repeat.update({kKey}, 0xFFFFFFF0u + 200));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_nothing_is_due_before_a_start);
    RUN_TEST(test_due_every_interval_while_held);
    RUN_TEST(test_release_stops_until_the_next_start);
    RUN_TEST(test_survives_clock_wrap);
    return UNITY_END();
}
