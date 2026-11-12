#include <unity.h>

#include <vector>

#include "core/keymap.h"

namespace {

constexpr KeyPos kFn{2, 0};
constexpr KeyPos kShift{2, 1};
constexpr KeyPos kCtrl{3, 0};
constexpr KeyPos kOpt{3, 1};
constexpr KeyPos kAlt{3, 2};
constexpr KeyPos kA{2, 2};
constexpr KeyPos kS{2, 3};
constexpr KeyPos kD{2, 4};
constexpr KeyPos kF{2, 5};
constexpr KeyPos kG{2, 6};
constexpr KeyPos kH{2, 7};
constexpr KeyPos kJ{2, 8};
constexpr KeyPos kSemicolon{2, 11};
constexpr KeyPos kComma{3, 10};
constexpr KeyPos kPeriod{3, 11};
constexpr KeyPos kSlash{3, 12};
constexpr KeyPos kGrave{0, 0};
constexpr KeyPos kDel{0, 13};
constexpr KeyPos kEnter{2, 13};
constexpr KeyPos kBackslash{1, 13};
constexpr KeyPos kE{1, 3};
constexpr KeyPos kI{1, 8};
constexpr KeyPos kO{1, 9};
constexpr KeyPos kU{1, 7};
constexpr KeyPos kSpace{3, 13};

KeyboardReport makeReport(uint8_t modifiers, std::vector<uint8_t> keys) {
    KeyboardReport report{};
    report[0] = modifiers;
    for (size_t i = 0; i < keys.size(); i++) report[2 + i] = keys[i];
    return report;
}

// Option + dead key, release, (Shift +) vowel, release.
void assertAccent(uint8_t deadKey, uint8_t vowel, bool shift, const std::vector<KeyboardReport>& reports) {
    TEST_ASSERT_TRUE(reports.size() >= 4);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(makeReport(0x04, {deadKey}).data(), reports[0].data(), kKeyboardReportSize);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(makeReport(0, {}).data(), reports[1].data(), kKeyboardReportSize);
    const uint8_t mods = shift ? 0x02 : 0;
    TEST_ASSERT_EQUAL_HEX8_ARRAY(makeReport(mods, {vowel}).data(), reports[2].data(), kKeyboardReportSize);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(makeReport(mods, {}).data(), reports[3].data(), kKeyboardReportSize);
}

void assertReport(uint8_t modifiers, std::vector<uint8_t> keys, const KeyboardReport& report) {
    uint8_t expected[kKeyboardReportSize] = {modifiers, 0};
    for (size_t i = 0; i < keys.size(); i++) expected[2 + i] = keys[i];
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, report.data(), kKeyboardReportSize);
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_letters_and_enter_are_sent_by_position() {
    KeyboardMapper mapper;
    TEST_ASSERT_FALSE(mapper.update({kA}).empty());
    assertReport(0, {0x04}, mapper.report());
    TEST_ASSERT_FALSE(mapper.update({kEnter}).empty());
    assertReport(0, {0x28}, mapper.report());
    TEST_ASSERT_FALSE(mapper.update({kBackslash}).empty());
    assertReport(0, {0x31}, mapper.report());
}

static void test_unchanged_keys_produce_no_new_report() {
    KeyboardMapper mapper;
    TEST_ASSERT_FALSE(mapper.update({kA}).empty());
    TEST_ASSERT_TRUE(mapper.update({kA}).empty());
    TEST_ASSERT_FALSE(mapper.update({}).empty());
    assertReport(0, {}, mapper.report());
}

static void test_modifiers_map_to_shift_control_option_and_command() {
    KeyboardMapper mapper;
    mapper.update({kShift, kA});
    assertReport(0x02, {0x04}, mapper.report());
    mapper.update({kCtrl, kOpt, kAlt, kA});
    assertReport(0x01 | 0x04 | 0x08, {0x04}, mapper.report());
}

static void test_modifier_alone_is_sent() {
    KeyboardMapper mapper;
    TEST_ASSERT_FALSE(mapper.update({kShift}).empty());
    assertReport(0x02, {}, mapper.report());
}

static void test_fn_layer_sends_arrows_and_escape() {
    KeyboardMapper mapper;
    mapper.update({kFn, kSemicolon});
    assertReport(0, {0x52}, mapper.report());
    mapper.update({kFn, kComma});
    assertReport(0, {0x50}, mapper.report());
    mapper.update({kFn, kPeriod});
    assertReport(0, {0x51}, mapper.report());
    mapper.update({kFn, kSlash});
    assertReport(0, {0x4F}, mapper.report());
    mapper.update({kFn, kGrave});
    assertReport(0, {0x29}, mapper.report());
}

static void test_fn_keeps_the_other_modifiers() {
    KeyboardMapper mapper;
    mapper.update({kFn, kShift, kSemicolon});
    assertReport(0x02, {0x52}, mapper.report());
}

static void test_del_is_backspace_and_fn_del_sends_nothing() {
    KeyboardMapper mapper;
    mapper.update({kDel});
    assertReport(0, {0x2A}, mapper.report());
    mapper.update({kFn, kDel});
    assertReport(0, {}, mapper.report());
}

static void test_other_fn_combinations_send_nothing() {
    KeyboardMapper mapper;
    TEST_ASSERT_TRUE(mapper.update({kFn, kS}).empty());
    assertReport(0, {}, mapper.report());
}

static void test_seventh_key_waits_for_a_free_slot() {
    KeyboardMapper mapper;
    mapper.update({kA, kS, kD, kF, kG, kH});
    TEST_ASSERT_TRUE(mapper.update({kA, kS, kD, kF, kG, kH, kJ}).empty());
    assertReport(0, {0x04, 0x16, 0x07, 0x09, 0x0A, 0x0B}, mapper.report());
    mapper.update({kS, kD, kF, kG, kH, kJ});
    assertReport(0, {0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D}, mapper.report());
}

static void test_keys_held_at_reset_are_ignored_until_released() {
    KeyboardMapper mapper;
    mapper.reset({kA});
    TEST_ASSERT_TRUE(mapper.update({kA}).empty());
    TEST_ASSERT_FALSE(mapper.update({kA, kS}).empty());
    assertReport(0, {0x16}, mapper.report());
    mapper.update({kS});
    TEST_ASSERT_FALSE(mapper.update({kA, kS}).empty());
    assertReport(0, {0x16, 0x04}, mapper.report());
}

static void test_reset_clears_the_report() {
    KeyboardMapper mapper;
    mapper.update({kShift, kA});
    mapper.reset({});
    assertReport(0, {}, mapper.report());
}

static void test_key_presses_are_counted_modifiers_excluded() {
    KeyboardMapper mapper;
    mapper.update({kShift});
    mapper.update({kShift, kA});
    mapper.update({kShift});
    mapper.update({kS});
    mapper.update({kS, kD});
    TEST_ASSERT_EQUAL(3, mapper.keyPresses());
    mapper.reset({});
    mapper.update({kA});
    TEST_ASSERT_EQUAL(4, mapper.keyPresses());
}

static void test_fn_vowels_type_grave_accents() {
    KeyboardMapper mapper;
    assertAccent(0x35, 0x04, false, mapper.update({kFn, kA}));
    mapper.update({});
    assertAccent(0x35, 0x08, false, mapper.update({kFn, kE}));
    mapper.update({});
    assertAccent(0x35, 0x0C, false, mapper.update({kFn, kI}));
    mapper.update({});
    assertAccent(0x35, 0x12, false, mapper.update({kFn, kO}));
    mapper.update({});
    assertAccent(0x35, 0x18, false, mapper.update({kFn, kU}));
}

static void test_ctrl_e_types_acute_e() {
    KeyboardMapper mapper;
    mapper.update({kCtrl});
    const std::vector<KeyboardReport> reports = mapper.update({kCtrl, kE});
    assertAccent(0x08, 0x08, false, reports);
    // Ctrl is still held: the last report brings it back, without the e.
    assertReport(0x01, {}, reports.back());
}

static void test_shift_gives_uppercase_accents() {
    KeyboardMapper mapper;
    assertAccent(0x35, 0x08, true, mapper.update({kFn, kShift, kE}));
    mapper.update({});
    assertAccent(0x08, 0x08, true, mapper.update({kCtrl, kShift, kE}));
}

static void test_held_accent_types_once() {
    KeyboardMapper mapper;
    mapper.update({kFn, kE});
    TEST_ASSERT_TRUE(mapper.update({kFn, kE}).empty());
    assertReport(0, {}, mapper.report());
}

static void test_accent_releases_other_held_keys_until_pressed_again() {
    KeyboardMapper mapper;
    mapper.update({kS});
    const std::vector<KeyboardReport> reports = mapper.update({kS, kCtrl, kE});
    // Pressing s again after the sequence would type a second s: it stays released.
    assertReport(0x01, {}, reports.back());
    TEST_ASSERT_TRUE(mapper.update({kS, kCtrl, kE}).empty());
    mapper.update({});
    mapper.update({kS});
    assertReport(0, {0x16}, mapper.report());
}

static void test_fn_ctrl_e_sends_nothing() {
    KeyboardMapper mapper;
    const std::vector<KeyboardReport> reports = mapper.update({kFn, kCtrl, kE});
    TEST_ASSERT_EQUAL(1, reports.size());
    assertReport(0x01, {}, reports.back());
}

static void test_accents_are_counted_as_key_presses() {
    KeyboardMapper mapper;
    mapper.update({kFn, kE});
    TEST_ASSERT_EQUAL(1, mapper.keyPresses());
}

static void test_fn_space_requests_eject_once_per_press() {
    KeyboardMapper mapper;
    TEST_ASSERT_TRUE(mapper.update({kFn, kSpace}).empty());
    TEST_ASSERT_TRUE(mapper.takeEject());
    TEST_ASSERT_FALSE(mapper.takeEject());
    mapper.update({kFn, kSpace});
    TEST_ASSERT_FALSE(mapper.takeEject());
    mapper.update({kFn});
    mapper.update({kFn, kSpace});
    TEST_ASSERT_TRUE(mapper.takeEject());
    assertReport(0, {}, mapper.report());
}

static void test_space_without_fn_is_no_eject() {
    KeyboardMapper mapper;
    mapper.update({kSpace});
    TEST_ASSERT_FALSE(mapper.takeEject());
    assertReport(0, {0x2C}, mapper.report());
}

static void test_fn_enter_requests_siri_once_per_press_and_types_nothing() {
    KeyboardMapper mapper;
    TEST_ASSERT_TRUE(mapper.update({kFn, kEnter}).empty());
    TEST_ASSERT_TRUE(mapper.takeSiri());
    TEST_ASSERT_FALSE(mapper.takeSiri());
    mapper.update({kFn, kEnter});
    TEST_ASSERT_FALSE(mapper.takeSiri());
    mapper.update({kFn});
    mapper.update({kFn, kEnter});
    TEST_ASSERT_TRUE(mapper.takeSiri());
    TEST_ASSERT_FALSE(mapper.takeEject());
    assertReport(0, {}, mapper.report());
}

static void test_enter_without_fn_is_no_siri() {
    KeyboardMapper mapper;
    mapper.update({kEnter});
    TEST_ASSERT_FALSE(mapper.takeSiri());
    assertReport(0, {0x28}, mapper.report());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_letters_and_enter_are_sent_by_position);
    RUN_TEST(test_unchanged_keys_produce_no_new_report);
    RUN_TEST(test_modifiers_map_to_shift_control_option_and_command);
    RUN_TEST(test_modifier_alone_is_sent);
    RUN_TEST(test_fn_layer_sends_arrows_and_escape);
    RUN_TEST(test_fn_keeps_the_other_modifiers);
    RUN_TEST(test_del_is_backspace_and_fn_del_sends_nothing);
    RUN_TEST(test_other_fn_combinations_send_nothing);
    RUN_TEST(test_seventh_key_waits_for_a_free_slot);
    RUN_TEST(test_keys_held_at_reset_are_ignored_until_released);
    RUN_TEST(test_reset_clears_the_report);
    RUN_TEST(test_key_presses_are_counted_modifiers_excluded);
    RUN_TEST(test_fn_vowels_type_grave_accents);
    RUN_TEST(test_ctrl_e_types_acute_e);
    RUN_TEST(test_shift_gives_uppercase_accents);
    RUN_TEST(test_held_accent_types_once);
    RUN_TEST(test_accent_releases_other_held_keys_until_pressed_again);
    RUN_TEST(test_fn_ctrl_e_sends_nothing);
    RUN_TEST(test_accents_are_counted_as_key_presses);
    RUN_TEST(test_fn_space_requests_eject_once_per_press);
    RUN_TEST(test_space_without_fn_is_no_eject);
    RUN_TEST(test_fn_enter_requests_siri_once_per_press_and_types_nothing);
    RUN_TEST(test_enter_without_fn_is_no_siri);
    return UNITY_END();
}
