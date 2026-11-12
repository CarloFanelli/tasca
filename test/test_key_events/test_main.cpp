#include <string>
#include <vector>
#include <unity.h>

#include "core/key_events.h"

namespace {

// Physical positions on the Cardputer keyboard (row, column).
constexpr KeyPos kA{2, 2};
constexpr KeyPos kS{2, 3};
constexpr KeyPos kSemicolon{2, 11};
constexpr KeyPos kEnter{2, 13};
constexpr KeyPos kDel{0, 13};
constexpr KeyPos kTab{1, 0};
constexpr KeyPos kSpace{3, 13};
constexpr KeyPos kFn{2, 0};
constexpr KeyPos kShift{2, 1};
constexpr KeyPos kCtrl{3, 0};
constexpr KeyPos kOpt{3, 1};
constexpr KeyPos kAlt{3, 2};

std::string label(KeyPos key, uint8_t modifiers) {
    char buffer[kKeyTextCapacity];
    formatKeyLabel(buffer, sizeof(buffer), key, modifiers);
    return buffer;
}

std::string line(const KeyEvent& event) {
    char buffer[kKeyTextCapacity];
    formatKeyEvent(buffer, sizeof(buffer), event);
    return buffer;
}

std::vector<std::string> update(KeyTracker& tracker, std::vector<KeyPos> pressed) {
    std::vector<std::string> lines;
    for (const KeyEvent& event : tracker.update(pressed)) {
        lines.push_back(line(event));
    }
    return lines;
}

void assertLines(const std::vector<std::string>& expected, const std::vector<std::string>& actual) {
    TEST_ASSERT_EQUAL_UINT(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), actual[i].c_str());
    }
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_plain_keys_are_labelled_by_their_character() {
    TEST_ASSERT_EQUAL_STRING("a", label(kA, 0).c_str());
    TEST_ASSERT_EQUAL_STRING(";", label(kSemicolon, 0).c_str());
}

static void test_keys_without_glyph_are_labelled_by_name() {
    TEST_ASSERT_EQUAL_STRING("Enter", label(kEnter, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Del", label(kDel, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Tab", label(kTab, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Space", label(kSpace, 0).c_str());
}

static void test_each_modifier_has_its_name() {
    TEST_ASSERT_EQUAL_STRING("Ctrl", label(kCtrl, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Opt", label(kOpt, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Alt", label(kAlt, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Shift", label(kShift, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("Fn", label(kFn, 0).c_str());
}

static void test_each_modifier_prefixes_the_label() {
    TEST_ASSERT_EQUAL_STRING("Ctrl+a", label(kA, kModCtrl).c_str());
    TEST_ASSERT_EQUAL_STRING("Opt+a", label(kA, kModOpt).c_str());
    TEST_ASSERT_EQUAL_STRING("Alt+a", label(kA, kModAlt).c_str());
    TEST_ASSERT_EQUAL_STRING("Shift+a", label(kA, kModShift).c_str());
    TEST_ASSERT_EQUAL_STRING("Fn+;", label(kSemicolon, kModFn).c_str());
}

static void test_multiple_modifiers_use_a_fixed_order() {
    TEST_ASSERT_EQUAL_STRING("Ctrl+Shift+a", label(kA, kModShift | kModCtrl).c_str());
    TEST_ASSERT_EQUAL_STRING("Ctrl+Opt+Alt+Shift+Fn+a",
                             label(kA, kModFn | kModShift | kModAlt | kModOpt | kModCtrl).c_str());
}

static void test_press_and_release_give_one_down_and_one_up() {
    KeyTracker tracker;
    assertLines({"down 'a' mods=none"}, update(tracker, {kA}));
    assertLines({"up 'a' mods=none"}, update(tracker, {}));
}

static void test_held_key_gives_no_repeat() {
    KeyTracker tracker;
    update(tracker, {kA});
    assertLines({}, update(tracker, {kA}));
    assertLines({}, update(tracker, {kA}));
}

static void test_glyphless_key_event_uses_its_name() {
    KeyTracker tracker;
    assertLines({"down Enter mods=none"}, update(tracker, {kEnter}));
}

static void test_fn_combo_reports_the_held_modifier() {
    KeyTracker tracker;
    assertLines({"down Fn mods=none"}, update(tracker, {kFn}));
    assertLines({"down ';' mods=Fn"}, update(tracker, {kFn, kSemicolon}));
    assertLines({"up ';' mods=Fn"}, update(tracker, {kFn}));
    assertLines({"up Fn mods=none"}, update(tracker, {}));
}

static void test_modifier_released_while_key_held_updates_later_events() {
    KeyTracker tracker;
    update(tracker, {kShift});
    update(tracker, {kShift, kA});
    assertLines({"up Shift mods=none"}, update(tracker, {kA}));
    assertLines({"up 'a' mods=none"}, update(tracker, {}));
}

static void test_two_keys_held_together_get_their_own_events() {
    KeyTracker tracker;
    assertLines({"down 'a' mods=none", "down 's' mods=none"}, update(tracker, {kA, kS}));
    assertLines({"up 'a' mods=none"}, update(tracker, {kS}));
    assertLines({"up 's' mods=none"}, update(tracker, {}));
}

static void test_release_and_press_in_the_same_scan() {
    KeyTracker tracker;
    update(tracker, {kA});
    assertLines({"up 'a' mods=none", "down 's' mods=none"}, update(tracker, {kS}));
}

static void test_last_pressed_follows_the_most_recent_down() {
    KeyTracker tracker;
    TEST_ASSERT_FALSE(tracker.hasLastPressed());
    update(tracker, {kCtrl});
    update(tracker, {kCtrl, kA});
    TEST_ASSERT_TRUE(tracker.hasLastPressed());
    TEST_ASSERT_EQUAL_STRING("Ctrl+a", label(tracker.lastPressed().key, tracker.lastPressed().modifiers).c_str());
    update(tracker, {});
    TEST_ASSERT_EQUAL_STRING("Ctrl+a", label(tracker.lastPressed().key, tracker.lastPressed().modifiers).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_plain_keys_are_labelled_by_their_character);
    RUN_TEST(test_keys_without_glyph_are_labelled_by_name);
    RUN_TEST(test_each_modifier_has_its_name);
    RUN_TEST(test_each_modifier_prefixes_the_label);
    RUN_TEST(test_multiple_modifiers_use_a_fixed_order);
    RUN_TEST(test_press_and_release_give_one_down_and_one_up);
    RUN_TEST(test_held_key_gives_no_repeat);
    RUN_TEST(test_glyphless_key_event_uses_its_name);
    RUN_TEST(test_fn_combo_reports_the_held_modifier);
    RUN_TEST(test_modifier_released_while_key_held_updates_later_events);
    RUN_TEST(test_two_keys_held_together_get_their_own_events);
    RUN_TEST(test_release_and_press_in_the_same_scan);
    RUN_TEST(test_last_pressed_follows_the_most_recent_down);
    return UNITY_END();
}
