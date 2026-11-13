#include <unity.h>

#include <stdint.h>

#include "core/clear_prompt.h"

void setUp() {}
void tearDown() {}

static void test_first_del_shows_the_prompt_for_the_chat() {
    ClearPrompt prompt(3000);
    TEST_ASSERT_FALSE(prompt.del("Anna", 100));
    TEST_ASSERT_TRUE(prompt.shown());
    TEST_ASSERT_EQUAL_STRING("Anna", prompt.chat().c_str());
}

static void test_second_del_within_the_wait_clears_and_removes_the_prompt() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    TEST_ASSERT_TRUE(prompt.del("Anna", 3099));
    TEST_ASSERT_FALSE(prompt.shown());
}

static void test_del_on_another_chat_shows_the_prompt_for_it() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    TEST_ASSERT_FALSE(prompt.del("Calcetto", 200));
    TEST_ASSERT_TRUE(prompt.shown());
    TEST_ASSERT_EQUAL_STRING("Calcetto", prompt.chat().c_str());
}

static void test_cancel_removes_the_prompt_and_clears_nothing() {
    ClearPrompt prompt(3000);
    TEST_ASSERT_FALSE(prompt.cancel());
    prompt.del("Anna", 100);
    TEST_ASSERT_TRUE(prompt.cancel());
    TEST_ASSERT_FALSE(prompt.shown());
    TEST_ASSERT_FALSE(prompt.del("Anna", 200));
}

static void test_prompt_expires_after_the_wait() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    TEST_ASSERT_FALSE(prompt.expire(3099));
    TEST_ASSERT_TRUE(prompt.expire(3100));
    TEST_ASSERT_FALSE(prompt.shown());
    TEST_ASSERT_FALSE(prompt.del("Anna", 3200));
}

static void test_expiry_survives_the_clock_wrapping_around() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", UINT32_MAX - 1000);
    TEST_ASSERT_FALSE(prompt.expire(1000));
    TEST_ASSERT_TRUE(prompt.expire(2000));
}

static void test_notice_shows_for_the_cleared_chat_then_expires() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    prompt.del("Anna", 200);
    TEST_ASSERT_FALSE(prompt.noticeShown());
    prompt.showNotice(500);
    TEST_ASSERT_TRUE(prompt.noticeShown());
    TEST_ASSERT_FALSE(prompt.shown());
    TEST_ASSERT_EQUAL_STRING("Anna", prompt.chat().c_str());
    TEST_ASSERT_FALSE(prompt.expire(3499));
    TEST_ASSERT_TRUE(prompt.expire(3500));
    TEST_ASSERT_FALSE(prompt.noticeShown());
}

static void test_a_key_removes_the_notice() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    prompt.del("Anna", 200);
    prompt.showNotice(500);
    TEST_ASSERT_TRUE(prompt.cancel());
    TEST_ASSERT_FALSE(prompt.noticeShown());
}

static void test_del_after_the_notice_shows_the_prompt_again() {
    ClearPrompt prompt(3000);
    prompt.del("Anna", 100);
    prompt.del("Anna", 200);
    prompt.showNotice(500);
    TEST_ASSERT_FALSE(prompt.del("Anna", 600));
    TEST_ASSERT_TRUE(prompt.shown());
    TEST_ASSERT_FALSE(prompt.noticeShown());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_first_del_shows_the_prompt_for_the_chat);
    RUN_TEST(test_second_del_within_the_wait_clears_and_removes_the_prompt);
    RUN_TEST(test_del_on_another_chat_shows_the_prompt_for_it);
    RUN_TEST(test_cancel_removes_the_prompt_and_clears_nothing);
    RUN_TEST(test_prompt_expires_after_the_wait);
    RUN_TEST(test_expiry_survives_the_clock_wrapping_around);
    RUN_TEST(test_notice_shows_for_the_cleared_chat_then_expires);
    RUN_TEST(test_a_key_removes_the_notice);
    RUN_TEST(test_del_after_the_notice_shows_the_prompt_again);
    return UNITY_END();
}
