#include <unity.h>

#include <string>

#include "core/chat_view.h"
#include "core/utf8.h"

namespace {

int charWidth(const std::string& text) {
    int count = 0;
    for (char c : text) {
        count += isUtf8Continuation(c) ? 0 : 1;
    }
    return count * 10;
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_time_is_taken_from_the_date() {
    TEST_ASSERT_EQUAL_STRING("09:48", timeOf("20261112T094800").c_str());
    TEST_ASSERT_EQUAL_STRING("--:--", timeOf("2026").c_str());
}

static void test_single_chat_lines_have_time_headers_and_wrapped_text() {
    MessageStore store;
    store.receive(1, false, {"Anna", "Anna"}, "ci vediamo alle otto", "20261112T090000");
    const auto lines = chatLines(store.messages(), "Anna", 100, charWidth);
    TEST_ASSERT_EQUAL(3, lines.size());
    TEST_ASSERT_TRUE(lines[0].header);
    TEST_ASSERT_EQUAL_STRING("09:00", lines[0].text.c_str());
    TEST_ASSERT_FALSE(lines[1].header);
    TEST_ASSERT_EQUAL_STRING("ci vediamo", lines[1].text.c_str());
    TEST_ASSERT_EQUAL_STRING("alle otto", lines[2].text.c_str());
}

static void test_group_headers_carry_the_sender() {
    MessageStore store;
    store.receive(1, false, {"Calcetto", "Luca"}, "chi viene?", "20261112T090000");
    const auto lines = chatLines(store.messages(), "Calcetto", 200, charWidth);
    TEST_ASSERT_EQUAL_STRING("09:00 Luca", lines[0].text.c_str());
}

static void test_only_that_chat_oldest_first_by_date() {
    MessageStore store;
    store.receive(1, false, {"Anna", "Anna"}, "secondo", "20261112T100000");
    store.receive(2, false, {"Bruno", "Bruno"}, "altro", "20261112T093000");
    store.receive(3, false, {"Anna", "Anna"}, "primo", "20261112T090000");
    const auto lines = chatLines(store.messages(), "Anna", 200, charWidth);
    TEST_ASSERT_EQUAL(4, lines.size());
    TEST_ASSERT_EQUAL_STRING("primo", lines[1].text.c_str());
    TEST_ASSERT_EQUAL_STRING("secondo", lines[3].text.c_str());
}

static void test_emoji_in_text_and_sender_show_as_boxes() {
    MessageStore store;
    store.receive(1, false, {"Calcetto", "Luca \xF0\x9F\x8E\xB8"}, "ok \xF0\x9F\x91\x8D", "20261112T090000");
    const auto lines = chatLines(store.messages(), "Calcetto", 200, charWidth);
    TEST_ASSERT_EQUAL_STRING((std::string("09:00 Luca ") + kMissingGlyph).c_str(), lines[0].text.c_str());
    TEST_ASSERT_EQUAL_STRING((std::string("ok ") + kMissingGlyph).c_str(), lines[1].text.c_str());
}

static void test_unknown_chat_has_no_lines() {
    MessageStore store;
    TEST_ASSERT_EQUAL(0, chatLines(store.messages(), "Anna", 100, charWidth).size());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_time_is_taken_from_the_date);
    RUN_TEST(test_single_chat_lines_have_time_headers_and_wrapped_text);
    RUN_TEST(test_group_headers_carry_the_sender);
    RUN_TEST(test_only_that_chat_oldest_first_by_date);
    RUN_TEST(test_emoji_in_text_and_sender_show_as_boxes);
    RUN_TEST(test_unknown_chat_has_no_lines);
    return UNITY_END();
}
