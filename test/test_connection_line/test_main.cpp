#include <unity.h>

#include "core/connection_line.h"

void setUp() {}
void tearDown() {}

static void test_no_bond_and_no_iphone_asks_to_connect() {
    TEST_ASSERT_EQUAL_STRING("Connect your iPhone", connectionLine(false, false, false, false));
}

static void test_bonded_iphone_is_awaited() {
    TEST_ASSERT_EQUAL_STRING("Connecting…", connectionLine(true, false, false, false));
}

static void test_iphone_connected_but_not_secured_is_connecting() {
    TEST_ASSERT_EQUAL_STRING("Connecting…", connectionLine(false, true, false, false));
    TEST_ASSERT_EQUAL_STRING("Connecting…", connectionLine(true, true, false, false));
}

static void test_failed_pairing_asks_to_forget_tasca() {
    TEST_ASSERT_EQUAL_STRING("Forget Tasca on the iPhone", connectionLine(false, false, true, false));
    TEST_ASSERT_EQUAL_STRING("Forget Tasca on the iPhone", connectionLine(true, false, true, false));
}

static void test_out_of_range_replaces_connecting() {
    TEST_ASSERT_EQUAL_STRING("iPhone out of range", connectionLine(true, false, false, true));
    TEST_ASSERT_EQUAL_STRING("Forget Tasca on the iPhone", connectionLine(true, false, true, true));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_no_bond_and_no_iphone_asks_to_connect);
    RUN_TEST(test_bonded_iphone_is_awaited);
    RUN_TEST(test_iphone_connected_but_not_secured_is_connecting);
    RUN_TEST(test_failed_pairing_asks_to_forget_tasca);
    RUN_TEST(test_out_of_range_replaces_connecting);
    return UNITY_END();
}
