#include <unity.h>

#include "core/chat_key.h"

namespace {

const char* const kLrm = "\xE2\x80\x8E";  // U+200E LEFT-TO-RIGHT MARK
const char* const kRlo = "\xE2\x80\xAE";  // U+202E RIGHT-TO-LEFT OVERRIDE

}  // namespace

void setUp() {}
void tearDown() {}

static void test_single_chat_uses_the_title_for_chat_and_sender() {
    const ChatKey key = chatKeyOf("Anna Rossi", "");
    TEST_ASSERT_EQUAL_STRING("Anna Rossi", key.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Anna Rossi", key.sender.c_str());
}

static void test_group_uses_subtitle_as_chat_and_title_as_sender() {
    const ChatKey key = chatKeyOf("Luca", "Calcetto \xF0\x9F\x8D\xBB");
    TEST_ASSERT_EQUAL_STRING("Calcetto \xF0\x9F\x8D\xBB", key.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Luca", key.sender.c_str());
}

static void test_direction_marks_are_trimmed_at_both_ends() {
    const ChatKey key = chatKeyOf(std::string(kLrm) + "Meta AI" + kLrm, std::string(kRlo) + kLrm);
    TEST_ASSERT_EQUAL_STRING("Meta AI", key.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Meta AI", key.sender.c_str());
}

static void test_direction_mark_inside_a_name_is_kept() {
    const std::string inside = std::string("Anna") + kLrm + "Rossi";
    TEST_ASSERT_EQUAL_STRING(inside.c_str(), chatKeyOf(inside, "").chat.c_str());
}

static void test_empty_names_become_a_question_mark() {
    const ChatKey none = chatKeyOf("", "");
    TEST_ASSERT_EQUAL_STRING("?", none.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("?", none.sender.c_str());
    const ChatKey onlyGroup = chatKeyOf(kLrm, "Famiglia");
    TEST_ASSERT_EQUAL_STRING("Famiglia", onlyGroup.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("?", onlyGroup.sender.c_str());
}

static void test_group_named_question_mark_stays_a_group() {
    const ChatKey key = chatKeyOf("Luca", "?");
    TEST_ASSERT_EQUAL_STRING("?", key.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Luca", key.sender.c_str());
}

static void test_group_is_a_sender_other_than_the_chat() {
    TEST_ASSERT_TRUE(isGroup("Calcetto", "Luca"));
    TEST_ASSERT_FALSE(isGroup("Anna", "Anna"));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_single_chat_uses_the_title_for_chat_and_sender);
    RUN_TEST(test_group_uses_subtitle_as_chat_and_title_as_sender);
    RUN_TEST(test_direction_marks_are_trimmed_at_both_ends);
    RUN_TEST(test_direction_mark_inside_a_name_is_kept);
    RUN_TEST(test_empty_names_become_a_question_mark);
    RUN_TEST(test_group_named_question_mark_stays_a_group);
    RUN_TEST(test_group_is_a_sender_other_than_the_chat);
    return UNITY_END();
}
