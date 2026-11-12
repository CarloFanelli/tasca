#include <unity.h>

#include <string.h>

#include <string>

#include "core/message_store.h"

namespace {

constexpr const char* kDate = "20261112T094800";

MessageStore::Result receive(MessageStore& store, uint32_t uid, const std::string& text, bool preExisting = false) {
    return store.receive(uid, preExisting, {"Anna", "Anna"}, text, kDate);
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_new_message_is_unread_and_complete() {
    MessageStore store;
    TEST_ASSERT_TRUE(store.receive(1, false, {"Calcetto", "Luca"}, "Ci sei stasera?", kDate) ==
                     MessageStore::Result::Added);
    TEST_ASSERT_EQUAL(1, store.size());
    TEST_ASSERT_EQUAL(1, store.unread());
    const Message& m = store.messages().back();
    TEST_ASSERT_EQUAL_STRING("Calcetto", m.chat.c_str());
    TEST_ASSERT_EQUAL_STRING("Luca", m.sender.c_str());
    TEST_ASSERT_EQUAL_STRING("Ci sei stasera?", m.text.c_str());
    TEST_ASSERT_EQUAL_STRING(kDate, m.date.c_str());
    TEST_ASSERT_FALSE(m.read);
}

static void test_long_text_is_kept_whole() {
    MessageStore store;
    receive(store, 1, std::string(512, 'x'));
    TEST_ASSERT_EQUAL(512, store.messages().back().text.size());
}

static void test_message_201_drops_the_oldest() {
    MessageStore store;
    for (uint32_t i = 0; i < 201; i++) receive(store, i, std::to_string(i));
    TEST_ASSERT_EQUAL(kMaxMessages, store.size());
    TEST_ASSERT_EQUAL_STRING("1", store.messages().front().text.c_str());
    TEST_ASSERT_EQUAL_STRING("200", store.messages().back().text.c_str());
    TEST_ASSERT_EQUAL(kMaxMessages, store.unread());
}

static void test_memory_counts_each_message_and_its_strings() {
    MessageStore store;
    receive(store, 1, "ciao");
    TEST_ASSERT_EQUAL(sizeof(Message) + strlen("Anna") * 2 + strlen("ciao") + strlen(kDate), store.memoryBytes());
}

static void test_memory_over_the_limit_drops_the_oldest_first() {
    MessageStore store;
    for (uint32_t i = 0; i < 120; i++) receive(store, i, std::string(500, 'a' + i % 26));
    TEST_ASSERT_TRUE(store.size() < 120);
    TEST_ASSERT_TRUE(store.memoryBytes() <= kMaxStoreBytes);
    TEST_ASSERT_EQUAL('a' + 119 % 26, store.messages().back().text[0]);
}

static void test_modified_notification_updates_its_message() {
    MessageStore store;
    receive(store, 7, "first");
    store.markRead(7);
    TEST_ASSERT_TRUE(receive(store, 7, "edited") == MessageStore::Result::Updated);
    TEST_ASSERT_EQUAL(1, store.size());
    TEST_ASSERT_EQUAL_STRING("edited", store.messages().back().text.c_str());
    TEST_ASSERT_TRUE(store.messages().back().read);
}

static void test_update_on_a_full_store_drops_nothing() {
    MessageStore store;
    for (uint32_t i = 0; i < kMaxMessages; i++) receive(store, i, "x");
    receive(store, 0, "y");
    TEST_ASSERT_EQUAL(kMaxMessages, store.size());
    TEST_ASSERT_EQUAL_STRING("y", store.messages().front().text.c_str());
}

static void test_removed_notification_marks_its_message_read() {
    MessageStore store;
    receive(store, 3, "ciao");
    TEST_ASSERT_TRUE(store.markRead(3));
    TEST_ASSERT_EQUAL(0, store.unread());
    TEST_ASSERT_FALSE(store.markRead(3));  // already read
    TEST_ASSERT_FALSE(store.markRead(99));  // never stored
}

static void test_preexisting_copy_after_reconnection_links_the_stored_message() {
    MessageStore store;
    receive(store, 5, "ci sei?");
    store.unlinkAll();
    TEST_ASSERT_FALSE(store.markRead(5));  // UIDs of the old connection mean nothing now
    TEST_ASSERT_TRUE(receive(store, 0, "ci sei?", true) == MessageStore::Result::Linked);
    TEST_ASSERT_EQUAL(1, store.size());
    TEST_ASSERT_TRUE(store.markRead(0));
    TEST_ASSERT_EQUAL(0, store.unread());
}

static void test_identical_live_messages_are_both_stored() {
    MessageStore store;
    receive(store, 1, "ok");
    TEST_ASSERT_TRUE(receive(store, 2, "ok") == MessageStore::Result::Added);
    TEST_ASSERT_EQUAL(2, store.size());
}

static void test_two_identical_preexisting_copies_keep_two_messages() {
    MessageStore store;
    receive(store, 1, "ok");
    receive(store, 2, "ok");
    store.unlinkAll();
    TEST_ASSERT_TRUE(receive(store, 10, "ok", true) == MessageStore::Result::Linked);
    TEST_ASSERT_TRUE(receive(store, 11, "ok", true) == MessageStore::Result::Linked);
    TEST_ASSERT_TRUE(receive(store, 12, "ok", true) == MessageStore::Result::Added);
    TEST_ASSERT_EQUAL(3, store.size());
}

static void test_preexisting_with_other_content_is_added() {
    MessageStore store;
    receive(store, 1, "ok");
    store.unlinkAll();
    TEST_ASSERT_TRUE(receive(store, 0, "ok!", true) == MessageStore::Result::Added);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_new_message_is_unread_and_complete);
    RUN_TEST(test_long_text_is_kept_whole);
    RUN_TEST(test_message_201_drops_the_oldest);
    RUN_TEST(test_memory_counts_each_message_and_its_strings);
    RUN_TEST(test_memory_over_the_limit_drops_the_oldest_first);
    RUN_TEST(test_modified_notification_updates_its_message);
    RUN_TEST(test_update_on_a_full_store_drops_nothing);
    RUN_TEST(test_removed_notification_marks_its_message_read);
    RUN_TEST(test_preexisting_copy_after_reconnection_links_the_stored_message);
    RUN_TEST(test_identical_live_messages_are_both_stored);
    RUN_TEST(test_two_identical_preexisting_copies_keep_two_messages);
    RUN_TEST(test_preexisting_with_other_content_is_added);
    return UNITY_END();
}
