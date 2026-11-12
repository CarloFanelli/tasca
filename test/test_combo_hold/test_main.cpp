#include <vector>
#include <unity.h>

#include "core/combo_hold.h"

namespace {

constexpr KeyPos kFn{2, 0};
constexpr KeyPos kDel{0, 13};
constexpr KeyPos kA{2, 2};
constexpr uint32_t kHoldMs = 3000;

}  // namespace

void setUp() {}
void tearDown() {}

static void test_released_before_the_hold_time_never_fires() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 1000));
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 3999));
    TEST_ASSERT_FALSE(combo.update({kFn}, 4000));
    TEST_ASSERT_FALSE(combo.update({}, 9000));
}

static void test_fires_once_when_held_exactly_the_hold_time() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 1000));
    TEST_ASSERT_TRUE(combo.update({kFn, kDel}, 4000));
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 9000));
}

static void test_fires_again_after_release_and_a_new_hold() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    combo.update({kFn, kDel}, 0);
    TEST_ASSERT_TRUE(combo.update({kFn, kDel}, 3000));
    combo.update({}, 3500);
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 4000));
    TEST_ASSERT_TRUE(combo.update({kFn, kDel}, 7000));
}

static void test_other_keys_held_meanwhile_do_not_matter() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    combo.update({kFn, kDel}, 0);
    TEST_ASSERT_FALSE(combo.update({kFn, kDel, kA}, 1500));
    TEST_ASSERT_TRUE(combo.update({kA, kDel, kFn}, 3000));
}

static void test_one_key_alone_never_fires() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    combo.update({kDel}, 0);
    TEST_ASSERT_FALSE(combo.update({kDel}, 10000));
}

static void test_hold_time_counts_from_when_both_keys_are_down() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    combo.update({kFn}, 0);
    combo.update({kFn, kDel}, 2000);
    TEST_ASSERT_FALSE(combo.update({kFn, kDel}, 4000));
    TEST_ASSERT_TRUE(combo.update({kFn, kDel}, 5000));
}

static void test_millis_wraparound() {
    ComboHold combo({kFn, kDel}, kHoldMs);
    combo.update({kFn, kDel}, 0xFFFFFFFFu - 1000);
    TEST_ASSERT_TRUE(combo.update({kFn, kDel}, 2000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_released_before_the_hold_time_never_fires);
    RUN_TEST(test_fires_once_when_held_exactly_the_hold_time);
    RUN_TEST(test_fires_again_after_release_and_a_new_hold);
    RUN_TEST(test_other_keys_held_meanwhile_do_not_matter);
    RUN_TEST(test_one_key_alone_never_fires);
    RUN_TEST(test_hold_time_counts_from_when_both_keys_are_down);
    RUN_TEST(test_millis_wraparound);
    return UNITY_END();
}
