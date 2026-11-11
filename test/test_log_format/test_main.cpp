#include <string.h>
#include <string>
#include <unity.h>

#include "core/log_format.h"

static std::string format(LogLevel level, const char* tag, const char* message) {
    char buffer[kLogLineCapacity];
    formatLogLine(buffer, sizeof(buffer), level, tag, message);
    return std::string(buffer);
}

void setUp() {}
void tearDown() {}

static void test_each_level_has_its_letter() {
    TEST_ASSERT_EQUAL_STRING("[D][SYS] boot", format(LogLevel::Debug, "SYS", "boot").c_str());
    TEST_ASSERT_EQUAL_STRING("[I][SYS] boot", format(LogLevel::Info, "SYS", "boot").c_str());
    TEST_ASSERT_EQUAL_STRING("[W][SYS] boot", format(LogLevel::Warn, "SYS", "boot").c_str());
    TEST_ASSERT_EQUAL_STRING("[E][SYS] boot", format(LogLevel::Error, "SYS", "boot").c_str());
}

static void test_empty_message_keeps_level_and_tag() {
    TEST_ASSERT_EQUAL_STRING("[I][KEY]", format(LogLevel::Info, "KEY", "").c_str());
}

static void test_tag_longer_than_six_chars_is_truncated() {
    TEST_ASSERT_EQUAL_STRING("[I][KEYBOA] x", format(LogLevel::Info, "KEYBOARD", "x").c_str());
}

static void test_tag_of_exactly_six_chars_is_kept() {
    TEST_ASSERT_EQUAL_STRING("[I][ABCDEF] x", format(LogLevel::Info, "ABCDEF", "x").c_str());
}

static void test_message_of_exactly_max_length_is_kept() {
    std::string message(kLogMaxMessageChars, 'a');
    std::string line = format(LogLevel::Info, "SYS", message.c_str());
    TEST_ASSERT_EQUAL_STRING(("[I][SYS] " + message).c_str(), line.c_str());
}

static void test_long_message_is_truncated_with_ellipsis() {
    std::string message(kLogMaxMessageChars + 1, 'a');
    std::string line = format(LogLevel::Info, "SYS", message.c_str());
    std::string expected = "[I][SYS] " + std::string(kLogMaxMessageChars - 3, 'a') + "...";
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), line.c_str());
}

static void test_truncation_does_not_split_utf8_characters() {
    // 252 ASCII chars followed by "è" (2 bytes): the cut at 253 would fall inside it.
    std::string message = std::string(kLogMaxMessageChars - 4, 'a') + "\xC3\xA8" + std::string(10, 'b');
    std::string line = format(LogLevel::Info, "SYS", message.c_str());
    std::string expected = "[I][SYS] " + std::string(kLogMaxMessageChars - 4, 'a') + "...";
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), line.c_str());
}

static void test_small_output_buffer_is_never_overrun() {
    char buffer[8];
    memset(buffer, 'x', sizeof(buffer));
    formatLogLine(buffer, sizeof(buffer), LogLevel::Info, "SYS", "boot");
    TEST_ASSERT_EQUAL_STRING("[I][SYS", buffer);
}

static void test_level_threshold() {
    TEST_ASSERT_TRUE(logLevelEnabled(LogLevel::Debug, LogLevel::Debug));
    TEST_ASSERT_TRUE(logLevelEnabled(LogLevel::Error, LogLevel::Warn));
    TEST_ASSERT_FALSE(logLevelEnabled(LogLevel::Info, LogLevel::Error));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_each_level_has_its_letter);
    RUN_TEST(test_empty_message_keeps_level_and_tag);
    RUN_TEST(test_tag_longer_than_six_chars_is_truncated);
    RUN_TEST(test_tag_of_exactly_six_chars_is_kept);
    RUN_TEST(test_message_of_exactly_max_length_is_kept);
    RUN_TEST(test_long_message_is_truncated_with_ellipsis);
    RUN_TEST(test_truncation_does_not_split_utf8_characters);
    RUN_TEST(test_small_output_buffer_is_never_overrun);
    RUN_TEST(test_level_threshold);
    return UNITY_END();
}
