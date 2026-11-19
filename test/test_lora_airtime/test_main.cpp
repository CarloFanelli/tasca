#include <unity.h>

#include "core/lora_airtime.h"

void setUp() {}
void tearDown() {}

// Semtech's formula for SF9, 250 kHz, CR 4/5, preamble 16, explicit header, CRC on.
static void test_time_on_air_of_medium_fast_packets() {
    TEST_ASSERT_UINT32_WITHIN(1, 99, loraAirtimeMs(16));
    TEST_ASSERT_UINT32_WITHIN(1, 570, loraAirtimeMs(221));
}

static void test_the_budget_allows_10_percent_of_an_hour() {
    AirtimeBudget budget;
    for (int i = 0; i < 360; i++) {
        TEST_ASSERT_TRUE(budget.allows(i * 1000, 1000));
        budget.spend(i * 1000, 1000);
    }
    TEST_ASSERT_FALSE(budget.allows(361000, 1));
}

static void test_time_spent_over_an_hour_ago_is_available_again() {
    AirtimeBudget budget;
    budget.spend(0, 360000);
    TEST_ASSERT_FALSE(budget.allows(3599999, 500));
    TEST_ASSERT_TRUE(budget.allows(3600000, 500));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_time_on_air_of_medium_fast_packets);
    RUN_TEST(test_the_budget_allows_10_percent_of_an_hour);
    RUN_TEST(test_time_spent_over_an_hour_ago_is_available_again);
    return UNITY_END();
}
