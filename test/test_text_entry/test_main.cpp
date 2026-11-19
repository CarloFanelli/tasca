#include <unity.h>

#include "core/text_entry.h"

void setUp() {}
void tearDown() {}

constexpr KeyPos kA{2, 2};
constexpr KeyPos kE{1, 3};
constexpr KeyPos kFn{2, 0};
constexpr KeyPos kDel{0, 13};
constexpr KeyPos kEnter{2, 13};

static void press(TextEntry& entry, std::vector<KeyPos> keys) {
    entry.update(keys);
    entry.update({});
}

static void test_keys_type_their_legends_and_del_deletes() {
    TextEntry entry(10, 10, {kEnter});
    entry.start({});
    press(entry, {kA});
    press(entry, {kE});
    TEST_ASSERT_EQUAL_STRING("ae", entry.text().c_str());
    press(entry, {kDel});
    TEST_ASSERT_EQUAL_STRING("a", entry.text().c_str());
}

static void test_fn_vowel_types_a_grave_accent() {
    TextEntry entry(10, 10, {kEnter});
    entry.start({});
    entry.update({kFn});
    entry.update({kFn, kE});
    entry.update({});
    TEST_ASSERT_EQUAL_STRING("\xC3\xA8", entry.text().c_str());
}

static void test_reserved_keys_type_nothing() {
    TextEntry entry(10, 10, {kEnter});
    entry.start({});
    press(entry, {kA});
    TEST_ASSERT_FALSE(entry.update({kEnter}));
    TEST_ASSERT_EQUAL_STRING("a", entry.text().c_str());
}

static void test_keys_past_the_limit_type_nothing_and_accents_count_two_bytes() {
    TextEntry entry(3, 10, {kEnter});
    entry.start({});
    press(entry, {kA});
    press(entry, {kA});
    entry.update({kFn});
    entry.update({kFn, kE});
    entry.update({});
    TEST_ASSERT_EQUAL_STRING("aa", entry.text().c_str());
    press(entry, {kA});
    TEST_ASSERT_EQUAL_STRING("aaa", entry.text().c_str());
    press(entry, {kA});
    TEST_ASSERT_EQUAL_STRING("aaa", entry.text().c_str());
}

static void test_keys_held_at_start_are_not_typed() {
    TextEntry entry(10, 10, {kEnter});
    entry.start({kA});
    entry.update({kA});
    TEST_ASSERT_EQUAL_STRING("", entry.text().c_str());
}

static void test_keys_past_the_character_limit_type_nothing() {
    TextEntry entry(10, 2, {kEnter});
    entry.start({});
    press(entry, {kA});
    entry.update({kFn});
    entry.update({kFn, kE});
    entry.update({});
    press(entry, {kA});
    TEST_ASSERT_EQUAL_STRING("a\xC3\xA8", entry.text().c_str());
}

static void test_editing_starts_from_the_text_given() {
    TextEntry entry(10, 10, {kEnter});
    entry.start({}, "ab");
    press(entry, {kDel});
    press(entry, {kE});
    TEST_ASSERT_EQUAL_STRING("ae", entry.text().c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_keys_type_their_legends_and_del_deletes);
    RUN_TEST(test_fn_vowel_types_a_grave_accent);
    RUN_TEST(test_reserved_keys_type_nothing);
    RUN_TEST(test_keys_past_the_limit_type_nothing_and_accents_count_two_bytes);
    RUN_TEST(test_keys_held_at_start_are_not_typed);
    RUN_TEST(test_keys_past_the_character_limit_type_nothing);
    RUN_TEST(test_editing_starts_from_the_text_given);
    return UNITY_END();
}
