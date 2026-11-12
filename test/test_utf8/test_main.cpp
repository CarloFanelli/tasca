#include <unity.h>

#include <string>

#include "core/utf8.h"

void setUp() {}
void tearDown() {}

static void test_continuation_bytes_are_recognized() {
    TEST_ASSERT_FALSE(isUtf8Continuation('a'));
    TEST_ASSERT_FALSE(isUtf8Continuation('\xC3'));
    TEST_ASSERT_TRUE(isUtf8Continuation('\xA9'));
}

static void test_complete_text_is_unchanged() {
    std::string text = "perch\xC3\xA9 \xF0\x9F\x8D\xBB";
    trimIncompleteUtf8(text);
    TEST_ASSERT_EQUAL_STRING("perch\xC3\xA9 \xF0\x9F\x8D\xBB", text.c_str());
}

static void test_partial_last_character_is_removed() {
    std::string twoOfFour = "ok \xF0\x9F";
    trimIncompleteUtf8(twoOfFour);
    TEST_ASSERT_EQUAL_STRING("ok ", twoOfFour.c_str());
    std::string leadOnly = "caff\xC3";
    trimIncompleteUtf8(leadOnly);
    TEST_ASSERT_EQUAL_STRING("caff", leadOnly.c_str());
}

static void test_empty_text_stays_empty() {
    std::string text;
    trimIncompleteUtf8(text);
    TEST_ASSERT_EQUAL_STRING("", text.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_continuation_bytes_are_recognized);
    RUN_TEST(test_complete_text_is_unchanged);
    RUN_TEST(test_partial_last_character_is_removed);
    RUN_TEST(test_empty_text_stays_empty);
    return UNITY_END();
}
