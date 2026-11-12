#include <unity.h>

#include <string>

#include "core/inbox.h"

namespace {

void add(MessageStore& store, uint32_t uid, const char* chat, const char* sender, const char* text, const char* date) {
    store.receive(uid, false, {chat, sender}, text, date);
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_empty_store_gives_empty_inbox() {
    MessageStore store;
    TEST_ASSERT_EQUAL(0, buildInbox(store.messages()).size());
}

static void test_each_chat_appears_once_with_its_newest_message() {
    MessageStore store;
    add(store, 1, "Anna", "Anna", "ciao", "20261112T090000");
    add(store, 2, "Anna", "Anna", "ci sei?", "20261112T090500");
    const auto chats = buildInbox(store.messages());
    TEST_ASSERT_EQUAL(1, chats.size());
    TEST_ASSERT_EQUAL_STRING("Anna", chats[0].chat.c_str());
    TEST_ASSERT_EQUAL_STRING("ci sei?", chats[0].text.c_str());
    TEST_ASSERT_EQUAL(2, chats[0].unread);
    TEST_ASSERT_FALSE(chats[0].group);
}

static void test_chats_are_ordered_newest_first_by_date() {
    MessageStore store;
    add(store, 1, "Anna", "Anna", "a", "20261112T100000");
    add(store, 2, "Calcetto", "Luca", "b", "20261112T090000");
    add(store, 3, "Bruno", "Bruno", "c", "20261112T110000");
    const auto chats = buildInbox(store.messages());
    TEST_ASSERT_EQUAL_STRING("Bruno", chats[0].chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Anna", chats[1].chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Calcetto", chats[2].chat.c_str());
}

static void test_equal_dates_put_the_later_stored_first() {
    MessageStore store;
    add(store, 1, "Anna", "Anna", "a", "20261112T100000");
    add(store, 2, "Bruno", "Bruno", "b", "20261112T100000");
    const auto chats = buildInbox(store.messages());
    TEST_ASSERT_EQUAL_STRING("Bruno", chats[0].chat.c_str());
}

static void test_message_stored_late_with_an_older_date_is_not_the_preview() {
    MessageStore store;
    add(store, 1, "Anna", "Anna", "nuovo", "20261112T100000");
    add(store, 2, "Anna", "Anna", "vecchio", "20261111T100000");
    TEST_ASSERT_EQUAL_STRING("nuovo", buildInbox(store.messages())[0].text.c_str());
}

static void test_group_keeps_sender_of_newest_message() {
    MessageStore store;
    add(store, 1, "Calcetto", "Luca", "chi c'\xC3\xA8?", "20261112T090000");
    add(store, 2, "Calcetto", "Marco", "io", "20261112T090100");
    const auto chats = buildInbox(store.messages());
    TEST_ASSERT_TRUE(chats[0].group);
    TEST_ASSERT_EQUAL_STRING("Marco", chats[0].sender.c_str());
}

static void test_read_messages_are_not_counted() {
    MessageStore store;
    add(store, 1, "Anna", "Anna", "a", "20261112T090000");
    add(store, 2, "Anna", "Anna", "b", "20261112T090100");
    store.markChatRead("Anna");
    TEST_ASSERT_EQUAL(0, buildInbox(store.messages())[0].unread);
    add(store, 3, "Anna", "Anna", "c", "20261112T090200");
    TEST_ASSERT_EQUAL(1, buildInbox(store.messages())[0].unread);
}

static void test_unread_label_caps_at_99() {
    TEST_ASSERT_EQUAL_STRING("0", unreadLabel(0).c_str());
    TEST_ASSERT_EQUAL_STRING("99", unreadLabel(99).c_str());
    TEST_ASSERT_EQUAL_STRING("99+", unreadLabel(100).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_empty_store_gives_empty_inbox);
    RUN_TEST(test_each_chat_appears_once_with_its_newest_message);
    RUN_TEST(test_chats_are_ordered_newest_first_by_date);
    RUN_TEST(test_equal_dates_put_the_later_stored_first);
    RUN_TEST(test_message_stored_late_with_an_older_date_is_not_the_preview);
    RUN_TEST(test_group_keeps_sender_of_newest_message);
    RUN_TEST(test_read_messages_are_not_counted);
    RUN_TEST(test_unread_label_caps_at_99);
    return UNITY_END();
}
