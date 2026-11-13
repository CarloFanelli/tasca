#include <unity.h>

#include <string>

#include "core/screen_stack.h"

void setUp() {}
void tearDown() {}

namespace {

// Records the hooks called on it, e.g. "shown covered ".
class FakeScreen : public Screen {
public:
    void handleKeys(const std::vector<KeyPos>&, const std::vector<KeyEvent>&) override {}
    void draw() override {}
    void onShown() override { calls += "shown "; }
    void onCovered() override { calls += "covered "; }

    std::string calls;
};

}  // namespace

static void test_bottom_is_on_top_at_start() {
    FakeScreen inbox;
    ScreenStack stack(&inbox);
    TEST_ASSERT_EQUAL_PTR(&inbox, stack.top());
}

static void test_open_covers_the_top_and_shows_the_new_one() {
    FakeScreen inbox, settings;
    ScreenStack stack(&inbox);
    TEST_ASSERT_TRUE(stack.open(&settings));
    TEST_ASSERT_EQUAL_PTR(&settings, stack.top());
    TEST_ASSERT_EQUAL_STRING("covered ", inbox.calls.c_str());
    TEST_ASSERT_EQUAL_STRING("shown ", settings.calls.c_str());
}

static void test_close_returns_to_the_screen_below() {
    FakeScreen inbox, chat, settings;
    ScreenStack stack(&inbox);
    stack.open(&chat);
    stack.open(&settings);
    TEST_ASSERT_TRUE(stack.close());
    TEST_ASSERT_EQUAL_PTR(&chat, stack.top());
    TEST_ASSERT_EQUAL_STRING("shown covered shown ", chat.calls.c_str());
    TEST_ASSERT_FALSE(stack.isOpen(&settings));
}

static void test_bottom_is_never_closed() {
    FakeScreen inbox;
    ScreenStack stack(&inbox);
    TEST_ASSERT_FALSE(stack.close());
    TEST_ASSERT_FALSE(stack.remove(&inbox));
    TEST_ASSERT_EQUAL_PTR(&inbox, stack.top());
    TEST_ASSERT_EQUAL_STRING("", inbox.calls.c_str());
}

static void test_open_screen_is_not_opened_again() {
    FakeScreen inbox, chat, settings;
    ScreenStack stack(&inbox);
    stack.open(&chat);
    stack.open(&settings);
    TEST_ASSERT_FALSE(stack.open(&settings));
    TEST_ASSERT_FALSE(stack.open(&chat));
    TEST_ASSERT_EQUAL_PTR(&settings, stack.top());
    TEST_ASSERT_EQUAL_STRING("shown ", settings.calls.c_str());
}

static void test_remove_covered_screen_keeps_the_top() {
    FakeScreen inbox, chat, settings;
    ScreenStack stack(&inbox);
    stack.open(&chat);
    stack.open(&settings);
    TEST_ASSERT_TRUE(stack.remove(&chat));
    TEST_ASSERT_EQUAL_PTR(&settings, stack.top());
    TEST_ASSERT_EQUAL_STRING("shown ", settings.calls.c_str());
    TEST_ASSERT_TRUE(stack.close());
    TEST_ASSERT_EQUAL_PTR(&inbox, stack.top());
}

static void test_remove_top_screen_shows_the_one_below() {
    FakeScreen inbox, chat;
    ScreenStack stack(&inbox);
    stack.open(&chat);
    TEST_ASSERT_TRUE(stack.remove(&chat));
    TEST_ASSERT_EQUAL_PTR(&inbox, stack.top());
    TEST_ASSERT_EQUAL_STRING("covered shown ", inbox.calls.c_str());
}

static void test_remove_screen_not_open_does_nothing() {
    FakeScreen inbox, chat;
    ScreenStack stack(&inbox);
    TEST_ASSERT_FALSE(stack.remove(&chat));
    TEST_ASSERT_EQUAL_PTR(&inbox, stack.top());
}

static void test_open_is_refused_when_the_stack_is_full() {
    FakeScreen screens[ScreenStack::kMaxScreens + 1];
    ScreenStack stack(&screens[0]);
    for (size_t i = 1; i < ScreenStack::kMaxScreens; i++) {
        TEST_ASSERT_TRUE(stack.open(&screens[i]));
    }
    TEST_ASSERT_FALSE(stack.open(&screens[ScreenStack::kMaxScreens]));
    TEST_ASSERT_EQUAL_PTR(&screens[ScreenStack::kMaxScreens - 1], stack.top());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bottom_is_on_top_at_start);
    RUN_TEST(test_open_covers_the_top_and_shows_the_new_one);
    RUN_TEST(test_close_returns_to_the_screen_below);
    RUN_TEST(test_bottom_is_never_closed);
    RUN_TEST(test_open_screen_is_not_opened_again);
    RUN_TEST(test_remove_covered_screen_keeps_the_top);
    RUN_TEST(test_remove_top_screen_shows_the_one_below);
    RUN_TEST(test_remove_screen_not_open_does_nothing);
    RUN_TEST(test_open_is_refused_when_the_stack_is_full);
    return UNITY_END();
}
