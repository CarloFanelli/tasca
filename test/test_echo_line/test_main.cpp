#include <unity.h>

#include <initializer_list>
#include <string>

#include "core/echo_line.h"

namespace {

constexpr uint8_t kShift = 0x02;
constexpr uint8_t kCtrl = 0x01;
constexpr uint8_t kOption = 0x04;
constexpr uint8_t kCommand = 0x08;

KeyboardReport report(uint8_t modifiers, std::initializer_list<uint8_t> keys = {}) {
    KeyboardReport r{};
    r[0] = modifiers;
    size_t slot = 2;
    for (uint8_t key : keys) r[slot++] = key;
    return r;
}

// Presses and releases one key with the given modifiers.
void tap(EchoLine& echo, uint8_t modifiers, uint8_t usage) {
    echo.apply(report(modifiers, {usage}));
    echo.apply(report(modifiers));
    echo.apply(report(0));
}

// The report sequence the keyboard mapper sends for an accented letter.
void accent(EchoLine& echo, uint8_t deadKey, uint8_t vowel, bool shift) {
    echo.apply(report(kOption, {deadKey}));
    echo.apply(report(0));
    echo.apply(report(shift ? kShift : 0, {vowel}));
    echo.apply(report(shift ? kShift : 0));
    echo.apply(report(0));
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_keycap_legends_and_shift_characters() {
    EchoLine echo;
    tap(echo, 0, 0x04);        // a
    tap(echo, kShift, 0x04);   // A
    tap(echo, 0, 0x1E);        // 1
    tap(echo, kShift, 0x1F);   // @
    tap(echo, 0, 0x2C);        // space
    tap(echo, 0, 0x33);        // ;
    tap(echo, kShift, 0x34);   // "
    tap(echo, 0, 0x35);        // `
    tap(echo, kShift, 0x38);   // ?
    TEST_ASSERT_EQUAL_STRING("aA1@ ;\"`?", echo.text().c_str());
}

static void test_held_key_echoes_once() {
    EchoLine echo;
    TEST_ASSERT_TRUE(echo.apply(report(0, {0x04})));
    TEST_ASSERT_FALSE(echo.apply(report(0, {0x04})));
    TEST_ASSERT_TRUE(echo.apply(report(0, {0x04, 0x05})));
    TEST_ASSERT_EQUAL_STRING("ab", echo.text().c_str());
}

static void test_grave_and_acute_accents() {
    EchoLine echo;
    accent(echo, 0x35, 0x04, false);
    accent(echo, 0x35, 0x08, false);
    accent(echo, 0x35, 0x0C, false);
    accent(echo, 0x35, 0x12, false);
    accent(echo, 0x35, 0x18, false);
    accent(echo, 0x08, 0x08, false);
    accent(echo, 0x35, 0x08, true);
    accent(echo, 0x08, 0x08, true);
    TEST_ASSERT_EQUAL_STRING("àèìòùéÈÉ", echo.text().c_str());
}

static void test_typing_perche_pero_cosi() {
    EchoLine echo;
    for (uint8_t usage : {0x13, 0x08, 0x15, 0x06, 0x0B}) tap(echo, 0, usage);  // perch
    accent(echo, 0x08, 0x08, false);                                          // é
    tap(echo, 0, 0x2C);
    for (uint8_t usage : {0x13, 0x08, 0x15}) tap(echo, 0, usage);  // per
    accent(echo, 0x35, 0x12, false);                              // ò
    tap(echo, 0, 0x2C);
    for (uint8_t usage : {0x06, 0x12, 0x16}) tap(echo, 0, usage);  // cos
    accent(echo, 0x35, 0x0C, false);                              // ì
    TEST_ASSERT_EQUAL_STRING("perché però così", echo.text().c_str());
}

static void test_backspace_removes_last_character() {
    EchoLine echo;
    tap(echo, 0, 0x04);
    accent(echo, 0x35, 0x08, false);
    tap(echo, 0, 0x2A);
    TEST_ASSERT_EQUAL_STRING("a", echo.text().c_str());
    tap(echo, 0, 0x2A);
    TEST_ASSERT_EQUAL_STRING("", echo.text().c_str());
    TEST_ASSERT_FALSE(echo.apply(report(0, {0x2A})));  // empty line stays empty
    TEST_ASSERT_EQUAL_STRING("", echo.text().c_str());
}

static void test_enter_starts_an_empty_line() {
    EchoLine echo;
    tap(echo, 0, 0x04);
    tap(echo, 0, 0x28);
    TEST_ASSERT_EQUAL_STRING("", echo.text().c_str());
}

static void test_ignored_keys_leave_the_line_unchanged() {
    EchoLine echo;
    tap(echo, 0, 0x04);
    for (uint8_t usage : {0x52, 0x50, 0x51, 0x4F, 0x29, 0x2B}) tap(echo, 0, usage);  // arrows, Esc, Tab
    tap(echo, kCtrl, 0x05);
    tap(echo, kCommand, 0x06);
    TEST_ASSERT_FALSE(echo.apply(report(kShift)));  // modifier alone
    TEST_ASSERT_EQUAL_STRING("a", echo.text().c_str());
}

static void test_other_option_characters_show_a_placeholder() {
    EchoLine echo;
    tap(echo, kOption, 0x16);  // Opt+s
    TEST_ASSERT_EQUAL_STRING("?", echo.text().c_str());
}

static void test_dead_key_before_a_non_vowel_shows_a_placeholder() {
    EchoLine echo;
    tap(echo, kOption, 0x35);
    TEST_ASSERT_EQUAL_STRING("", echo.text().c_str());
    tap(echo, 0, 0x05);
    TEST_ASSERT_EQUAL_STRING("?b", echo.text().c_str());
}

static void test_long_line_keeps_its_end() {
    EchoLine echo;
    for (size_t i = 0; i < kEchoMaxBytes; i++) tap(echo, 0, 0x04);
    accent(echo, 0x35, 0x04, false);
    TEST_ASSERT_TRUE(echo.text().size() <= kEchoMaxBytes);
    TEST_ASSERT_EQUAL_STRING("aà", echo.text().substr(echo.text().size() - 3).c_str());
}

static void test_clear_empties_the_line_and_forgets_dead_keys() {
    EchoLine echo;
    tap(echo, 0, 0x04);
    tap(echo, kOption, 0x35);
    echo.clear();
    TEST_ASSERT_EQUAL_STRING("", echo.text().c_str());
    tap(echo, 0, 0x08);
    TEST_ASSERT_EQUAL_STRING("e", echo.text().c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_keycap_legends_and_shift_characters);
    RUN_TEST(test_held_key_echoes_once);
    RUN_TEST(test_grave_and_acute_accents);
    RUN_TEST(test_typing_perche_pero_cosi);
    RUN_TEST(test_backspace_removes_last_character);
    RUN_TEST(test_enter_starts_an_empty_line);
    RUN_TEST(test_ignored_keys_leave_the_line_unchanged);
    RUN_TEST(test_other_option_characters_show_a_placeholder);
    RUN_TEST(test_dead_key_before_a_non_vowel_shows_a_placeholder);
    RUN_TEST(test_long_line_keeps_its_end);
    RUN_TEST(test_clear_empties_the_line_and_forgets_dead_keys);
    return UNITY_END();
}
