#include <unity.h>

#include <string>
#include <vector>

#include "core/display_text.h"
#include "core/utf8.h"

namespace {

const std::string kBox = kMissingGlyph;
const std::string kDots = kEllipsis;

// Every character 10 pixels wide.
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

static void test_latin1_accents_are_kept() {
    TEST_ASSERT_EQUAL_STRING("perch\xC3\xA9 citt\xC3\xA0 \xC3\xB2", displayable("perch\xC3\xA9 citt\xC3\xA0 \xC3\xB2", true).c_str());
}

static void test_single_emoji_becomes_one_box() {
    TEST_ASSERT_EQUAL_STRING(("ok " + kBox).c_str(), displayable("ok \xF0\x9F\x8D\xBB", true).c_str());
}

static void test_emoji_with_variation_selector_becomes_one_box() {
    // U+2764 U+FE0F red heart
    TEST_ASSERT_EQUAL_STRING(kBox.c_str(), displayable("\xE2\x9D\xA4\xEF\xB8\x8F", true).c_str());
}

static void test_skin_tone_and_zwj_sequence_become_one_box() {
    // U+1F44D U+1F3FD thumbs up, medium skin
    TEST_ASSERT_EQUAL_STRING(kBox.c_str(), displayable("\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD", true).c_str());
    // man ZWJ woman ZWJ girl
    TEST_ASSERT_EQUAL_STRING(
        kBox.c_str(),
        displayable("\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7", true).c_str());
}

static void test_flag_becomes_one_box_and_two_flags_two() {
    const std::string italy = "\xF0\x9F\x87\xAE\xF0\x9F\x87\xB9";
    TEST_ASSERT_EQUAL_STRING(kBox.c_str(), displayable(italy, true).c_str());
    TEST_ASSERT_EQUAL_STRING((kBox + kBox).c_str(), displayable(italy + italy, true).c_str());
}

static void test_non_latin_script_becomes_boxes() {
    // Cyrillic "да"
    TEST_ASSERT_EQUAL_STRING((kBox + kBox).c_str(), displayable("\xD0\xB4\xD0\xB0", true).c_str());
}

static void test_invisible_marks_and_invalid_bytes_are_dropped() {
    TEST_ASSERT_EQUAL_STRING("Meta AI", displayable("\xE2\x80\x8EMeta AI", true).c_str());
    TEST_ASSERT_EQUAL_STRING("ab", displayable("a\xFF" "b", true).c_str());
    TEST_ASSERT_EQUAL_STRING("a", displayable("a\xF0\x9F", true).c_str());
}

static void test_line_breaks_flatten_only_on_request() {
    TEST_ASSERT_EQUAL_STRING("a b c", displayable("a\nb\tc", true).c_str());
    TEST_ASSERT_EQUAL_STRING("a\nb", displayable("a\nb", false).c_str());
}

static void test_text_that_fits_is_unchanged() {
    TEST_ASSERT_EQUAL_STRING("Anna", fitWidth("Anna", 40, charWidth).c_str());
}

static void test_long_text_is_cut_with_ellipsis() {
    TEST_ASSERT_EQUAL_STRING(("Calc" + kDots).c_str(), fitWidth("Calcetto", 50, charWidth).c_str());
}

static void test_cut_never_splits_a_character() {
    TEST_ASSERT_EQUAL_STRING(("c\xC3\xA0" + kDots).c_str(), fitWidth("c\xC3\xA0" "ff\xC3\xA8", 30, charWidth).c_str());
    TEST_ASSERT_EQUAL_STRING(("c" + kBox + kDots).c_str(), fitWidth("c" + kBox + "xyz", 30, charWidth).c_str());
}

static void test_too_narrow_gives_ellipsis_or_nothing() {
    TEST_ASSERT_EQUAL_STRING(kDots.c_str(), fitWidth("Anna", 10, charWidth).c_str());
    TEST_ASSERT_EQUAL_STRING("", fitWidth("Anna", 5, charWidth).c_str());
}

static void assertLines(const std::vector<std::string>& expected, const std::vector<std::string>& actual) {
    TEST_ASSERT_EQUAL_UINT(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), actual[i].c_str());
    }
}

static void test_short_text_is_one_line() {
    assertLines({"ci sei?"}, wrapText("ci sei?", 100, charWidth));
}

static void test_text_wraps_between_words() {
    assertLines({"ci vediamo", "alle otto"}, wrapText("ci vediamo alle otto", 100, charWidth));
}

static void test_word_longer_than_a_line_is_broken() {
    assertLines({"ok", "abcde", "fgh"}, wrapText("ok abcdefgh", 50, charWidth));
}

static void test_line_breaks_are_kept() {
    assertLines({"uno", "", "due"}, wrapText("uno\n\ndue", 100, charWidth));
}

static void test_wrap_never_splits_a_character() {
    assertLines({"\xC3\xA0\xC3\xA8", "\xC3\xAC"}, wrapText("\xC3\xA0\xC3\xA8\xC3\xAC", 20, charWidth));
}

static void test_empty_text_is_one_empty_line() {
    assertLines({""}, wrapText("", 100, charWidth));
}

static void test_the_end_of_a_long_line_is_kept_whole_characters() {
    TEST_ASSERT_EQUAL_STRING("ciao", lastFitting("ciao", 40, charWidth).c_str());
    TEST_ASSERT_EQUAL_STRING("\xC3\xA8 ok", lastFitting("perch\xC3\xA8 ok", 40, charWidth).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_latin1_accents_are_kept);
    RUN_TEST(test_single_emoji_becomes_one_box);
    RUN_TEST(test_emoji_with_variation_selector_becomes_one_box);
    RUN_TEST(test_skin_tone_and_zwj_sequence_become_one_box);
    RUN_TEST(test_flag_becomes_one_box_and_two_flags_two);
    RUN_TEST(test_non_latin_script_becomes_boxes);
    RUN_TEST(test_invisible_marks_and_invalid_bytes_are_dropped);
    RUN_TEST(test_line_breaks_flatten_only_on_request);
    RUN_TEST(test_text_that_fits_is_unchanged);
    RUN_TEST(test_the_end_of_a_long_line_is_kept_whole_characters);
    RUN_TEST(test_long_text_is_cut_with_ellipsis);
    RUN_TEST(test_cut_never_splits_a_character);
    RUN_TEST(test_too_narrow_gives_ellipsis_or_nothing);
    RUN_TEST(test_short_text_is_one_line);
    RUN_TEST(test_text_wraps_between_words);
    RUN_TEST(test_word_longer_than_a_line_is_broken);
    RUN_TEST(test_line_breaks_are_kept);
    RUN_TEST(test_wrap_never_splits_a_character);
    RUN_TEST(test_empty_text_is_one_empty_line);
    return UNITY_END();
}
