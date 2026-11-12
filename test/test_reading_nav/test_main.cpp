#include <unity.h>

#include <string>
#include <vector>

#include "core/reading_nav.h"

namespace {

std::vector<ChatSummary> inbox(std::vector<std::string> names) {
    std::vector<ChatSummary> chats;
    for (const std::string& n : names) {
        ChatSummary c;
        c.chat = n;
        chats.push_back(c);
    }
    return chats;
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_selection_starts_at_the_top() {
    ReadingNav nav;
    nav.sync(inbox({"Anna", "Bruno"}));
    TEST_ASSERT_EQUAL(0, nav.selected());
}

static void test_selection_moves_and_stops_at_the_ends() {
    ReadingNav nav;
    nav.sync(inbox({"Anna", "Bruno", "Carla"}));
    TEST_ASSERT_FALSE(nav.up());
    TEST_ASSERT_TRUE(nav.down());
    TEST_ASSERT_TRUE(nav.down());
    TEST_ASSERT_FALSE(nav.down());
    TEST_ASSERT_EQUAL(2, nav.selected());
    TEST_ASSERT_TRUE(nav.up());
    TEST_ASSERT_EQUAL(1, nav.selected());
}

static void test_empty_inbox_has_nothing_to_move() {
    ReadingNav nav;
    nav.sync({});
    TEST_ASSERT_FALSE(nav.up());
    TEST_ASSERT_FALSE(nav.down());
    TEST_ASSERT_EQUAL(0, nav.selected());
    TEST_ASSERT_EQUAL(0, nav.firstVisible(3));
}

static void test_window_scrolls_just_enough_to_show_the_selection() {
    ReadingNav nav;
    nav.sync(inbox({"a", "b", "c", "d", "e"}));
    TEST_ASSERT_EQUAL(0, nav.firstVisible(3));
    nav.down();
    nav.down();
    TEST_ASSERT_EQUAL(0, nav.firstVisible(3));
    nav.down();
    TEST_ASSERT_EQUAL(1, nav.firstVisible(3));
    nav.down();
    TEST_ASSERT_EQUAL(2, nav.firstVisible(3));
    nav.up();
    nav.up();
    TEST_ASSERT_EQUAL(2, nav.firstVisible(3));
    nav.up();
    TEST_ASSERT_EQUAL(1, nav.firstVisible(3));
}

static void test_selected_chat_is_followed_when_it_moves() {
    ReadingNav nav;
    nav.sync(inbox({"Anna", "Bruno", "Carla"}));
    nav.down();
    nav.sync(inbox({"Bruno", "Anna", "Carla"}));
    TEST_ASSERT_EQUAL(0, nav.selected());
}

static void test_vanished_chat_keeps_the_position_clamped() {
    ReadingNav nav;
    nav.sync(inbox({"Anna", "Bruno", "Carla"}));
    nav.down();
    nav.sync(inbox({"Anna", "Carla"}));
    TEST_ASSERT_EQUAL(1, nav.selected());
    nav.down();
    nav.sync(inbox({"Anna"}));
    TEST_ASSERT_EQUAL(0, nav.selected());
}

static void test_shrunk_list_leaves_no_empty_rows_at_the_bottom() {
    ReadingNav nav;
    nav.sync(inbox({"a", "b", "c", "d", "e"}));
    for (int i = 0; i < 4; i++) nav.down();
    TEST_ASSERT_EQUAL(2, nav.firstVisible(3));
    nav.sync(inbox({"a", "b", "c", "e"}));
    TEST_ASSERT_EQUAL(1, nav.firstVisible(3));
}

static void test_open_chat_needs_a_chat() {
    ReadingNav nav;
    nav.sync({});
    TEST_ASSERT_FALSE(nav.openChat());
    TEST_ASSERT_FALSE(nav.inChat());
}

static void test_chat_opens_at_its_end_and_closes_to_the_same_selection() {
    ReadingNav nav;
    nav.sync(inbox({"Anna", "Bruno"}));
    nav.down();
    TEST_ASSERT_TRUE(nav.openChat());
    TEST_ASSERT_TRUE(nav.inChat());
    TEST_ASSERT_EQUAL_STRING("Bruno", nav.openedChat().c_str());
    nav.setChatLines(10, 4, 8);
    TEST_ASSERT_EQUAL(6, nav.chatTop());
    nav.closeChat();
    TEST_ASSERT_FALSE(nav.inChat());
    TEST_ASSERT_EQUAL(1, nav.selected());
}

static void test_chat_scroll_stops_at_top_and_bottom() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(5, 3, 4);
    TEST_ASSERT_FALSE(nav.scrollDown());
    TEST_ASSERT_TRUE(nav.scrollUp());
    TEST_ASSERT_TRUE(nav.scrollUp());
    TEST_ASSERT_FALSE(nav.scrollUp());
    TEST_ASSERT_EQUAL(0, nav.chatTop());
    TEST_ASSERT_TRUE(nav.scrollDown());
    TEST_ASSERT_EQUAL(1, nav.chatTop());
}

static void test_short_chat_does_not_scroll() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(2, 6, 0);
    TEST_ASSERT_EQUAL(0, nav.chatTop());
    TEST_ASSERT_FALSE(nav.scrollUp());
    TEST_ASSERT_FALSE(nav.scrollDown());
}

static void test_chat_at_its_end_follows_new_lines() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(5, 3, 4);
    nav.setChatLines(7, 3, 5);
    TEST_ASSERT_EQUAL(4, nav.chatTop());
}

static void test_scrolled_back_chat_keeps_its_place_on_new_lines() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(5, 3, 4);
    nav.scrollUp();
    nav.setChatLines(7, 3, 5);
    TEST_ASSERT_EQUAL(1, nav.chatTop());
}

static void test_chat_scroll_is_clamped_when_lines_are_dropped() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(10, 3, 9);
    nav.scrollUp();
    nav.setChatLines(4, 3, 3);
    TEST_ASSERT_EQUAL(1, nav.chatTop());
}

static void test_long_newest_message_opens_at_its_start() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(20, 6, 8);
    TEST_ASSERT_EQUAL(8, nav.chatTop());
}

static void test_long_new_message_shows_its_start_when_following() {
    ReadingNav nav;
    nav.sync(inbox({"Anna"}));
    nav.openChat();
    nav.setChatLines(6, 3, 5);
    nav.setChatLines(16, 3, 6);
    TEST_ASSERT_EQUAL(6, nav.chatTop());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_selection_starts_at_the_top);
    RUN_TEST(test_selection_moves_and_stops_at_the_ends);
    RUN_TEST(test_empty_inbox_has_nothing_to_move);
    RUN_TEST(test_window_scrolls_just_enough_to_show_the_selection);
    RUN_TEST(test_selected_chat_is_followed_when_it_moves);
    RUN_TEST(test_vanished_chat_keeps_the_position_clamped);
    RUN_TEST(test_shrunk_list_leaves_no_empty_rows_at_the_bottom);
    RUN_TEST(test_open_chat_needs_a_chat);
    RUN_TEST(test_chat_opens_at_its_end_and_closes_to_the_same_selection);
    RUN_TEST(test_chat_scroll_stops_at_top_and_bottom);
    RUN_TEST(test_short_chat_does_not_scroll);
    RUN_TEST(test_chat_at_its_end_follows_new_lines);
    RUN_TEST(test_scrolled_back_chat_keeps_its_place_on_new_lines);
    RUN_TEST(test_chat_scroll_is_clamped_when_lines_are_dropped);
    RUN_TEST(test_long_newest_message_opens_at_its_start);
    RUN_TEST(test_long_new_message_shows_its_start_when_following);
    return UNITY_END();
}
