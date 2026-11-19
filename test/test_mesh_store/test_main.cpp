#include <unity.h>

#include "core/mesh_store.h"
#include "core/mesh_view.h"
#include "core/wall_clock.h"

void setUp() {}
void tearDown() {}

static MeshPacket text(uint32_t from, uint32_t id, const char* body) {
    MeshPacket p;
    p.to = kMeshBroadcast;
    p.from = from;
    p.id = id;
    p.port = kPortText;
    p.payload = body;
    return p;
}

// User{id 1, long_name 2, short_name 3}, as a node announces itself.
static MeshPacket announcement(uint32_t from, uint32_t id, const std::string& longName, const std::string& shortName) {
    MeshPacket p;
    p.to = kMeshBroadcast;
    p.from = from;
    p.id = id;
    p.port = kPortNodeInfo;
    p.payload = std::string("\x12") + static_cast<char>(longName.size()) + longName + "\x1a" +
                static_cast<char>(shortName.size()) + shortName;
    return p;
}

static void test_a_text_is_listed_and_unread() {
    MeshStore store;
    TEST_ASSERT_TRUE(store.add(text(0xa0ccd3cc, 1, "ciao"), -90, 1000));
    TEST_ASSERT_EQUAL(1, store.messages().size());
    TEST_ASSERT_EQUAL_STRING("ciao", store.messages().back().text.c_str());
    TEST_ASSERT_EQUAL_UINT32(1000, store.messages().back().arrivedMs);
    TEST_ASSERT_EQUAL(1, store.unread());
}

static void test_the_same_packet_relayed_is_listed_once() {
    MeshStore store;
    store.add(text(0xa0ccd3cc, 7, "ciao"), -90, 1000);
    TEST_ASSERT_FALSE(store.add(text(0xa0ccd3cc, 7, "ciao"), -80, 2000));
    TEST_ASSERT_EQUAL(1, store.messages().size());
    TEST_ASSERT_EQUAL(1, store.unread());
}

static void test_marking_read_clears_the_count() {
    MeshStore store;
    store.add(text(1, 1, "a"), -90, 0);
    store.add(text(1, 2, "b"), -90, 0);
    TEST_ASSERT_TRUE(store.markAllRead());
    TEST_ASSERT_EQUAL(0, store.unread());
    TEST_ASSERT_FALSE(store.markAllRead());
}

static void test_an_unannounced_sender_is_named_by_its_id() {
    MeshStore store;
    TEST_ASSERT_EQUAL_STRING("!d3cc", store.shortNameOf(0xa0ccd3cc).c_str());
}

static void test_an_announcement_names_the_sender_and_is_not_listed() {
    MeshStore store;
    store.add(text(0xa0ccd3cc, 1, "ciao"), -90, 0);
    TEST_ASSERT_TRUE(store.add(announcement(0xa0ccd3cc, 2, "RESS IU4LHV", "RESS"), -90, 0));
    TEST_ASSERT_EQUAL(1, store.messages().size());
    TEST_ASSERT_EQUAL_STRING("RESS", store.shortNameOf(0xa0ccd3cc).c_str());
}

static void test_direct_messages_and_other_ports_are_not_listed() {
    MeshStore store;
    MeshPacket direct = text(1, 1, "secret");
    direct.to = 0x12345678;
    TEST_ASSERT_FALSE(store.add(direct, -90, 0));
    MeshPacket position = text(1, 2, "");
    position.port = 3;
    store.add(position, -90, 0);
    TEST_ASSERT_EQUAL(0, store.messages().size());
}

static void test_only_the_last_500_messages_are_kept() {
    MeshStore store;
    for (uint32_t i = 1; i <= 501; i++) {
        store.add(text(1, i, "m"), -90, i);
    }
    TEST_ASSERT_EQUAL(500, store.messages().size());
    TEST_ASSERT_EQUAL_UINT32(2, store.messages().front().id);
}

static void test_elapsed_time_labels() {
    TEST_ASSERT_EQUAL_STRING("now", elapsedLabel(59 * 1000).c_str());
    TEST_ASSERT_EQUAL_STRING("5m", elapsedLabel(5 * 60 * 1000 + 30 * 1000).c_str());
    TEST_ASSERT_EQUAL_STRING("2h", elapsedLabel(2 * 3600 * 1000 + 59 * 60 * 1000).c_str());
    TEST_ASSERT_EQUAL_STRING("3d", elapsedLabel(3u * 24 * 3600 * 1000).c_str());
}

static void test_lines_show_elapsed_time_sender_and_wrapped_text() {
    MeshStore store;
    store.add(announcement(0xa0ccd3cc, 1, "RESS IU4LHV", "RESS"), -90, 0);
    store.add(text(0xa0ccd3cc, 2, "ciao a tutti"), -90, 0);
    // One pixel per byte: the text wraps at 8.
    const std::vector<ChatLine> lines =
        meshLines(store, 5 * 60 * 1000, 8, [](const std::string& s) { return static_cast<int>(s.size()); });
    TEST_ASSERT_EQUAL(3, lines.size());
    TEST_ASSERT_TRUE(lines[0].header);
    TEST_ASSERT_EQUAL_STRING("5m RESS", lines[0].text.c_str());
    TEST_ASSERT_EQUAL_STRING("ciao a", lines[1].text.c_str());
    TEST_ASSERT_EQUAL_STRING("tutti", lines[2].text.c_str());
}

static constexpr uint32_t kOwn = 0x11223344;

static void test_an_own_message_reads_sent_and_is_not_unread() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hello", 1000);
    TEST_ASSERT_EQUAL(1, store.messages().size());
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::Sent), static_cast<int>(store.messages().back().delivery));
    TEST_ASSERT_EQUAL(0, store.unread());
}

static void test_an_own_message_relayed_back_is_heard_and_not_listed_again() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hello", 1000);
    TEST_ASSERT_TRUE(store.add(text(kOwn, 9, "hello"), -70, 2000));
    TEST_ASSERT_EQUAL(1, store.messages().size());
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::Heard), static_cast<int>(store.messages().back().delivery));
    TEST_ASSERT_FALSE(store.add(text(kOwn, 9, "hello"), -70, 2500));
}

static void test_an_own_message_unheard_for_30_s_is_not_heard() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hello", 1000);
    TEST_ASSERT_FALSE(store.expire(30999));
    TEST_ASSERT_TRUE(store.expire(31000));
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::NotHeard), static_cast<int>(store.messages().back().delivery));
    TEST_ASSERT_FALSE(store.expire(40000));
}

static void test_a_failed_send_reads_not_sent() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hello", 1000);
    TEST_ASSERT_TRUE(store.markNotSent(9));
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::NotSent), static_cast<int>(store.messages().back().delivery));
    TEST_ASSERT_FALSE(store.expire(60000));
}

static void test_an_own_message_header_carries_its_mark() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hi", 0);
    const std::vector<ChatLine> lines =
        meshLines(store, 0, 100, [](const std::string& s) { return static_cast<int>(s.size()); });
    TEST_ASSERT_EQUAL_STRING("now !3344 \xC2\xB7 sent", lines[0].text.c_str());
}

static void test_own_messages_carry_the_users_short_name() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.setOwnShortName("Carl");
    TEST_ASSERT_EQUAL_STRING("Carl", store.shortNameOf(kOwn).c_str());
}

static void test_received_counts_texts_from_other_nodes_only() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.add(text(1, 1, "a"), -90, 0);
    store.add(text(1, 1, "a"), -90, 0);
    store.add(announcement(1, 2, "Node", "N"), -90, 0);
    store.addOwn(3, "mine", 0);
    TEST_ASSERT_EQUAL(1, store.received());
}

static void test_nodes_are_listed_newest_first_with_their_signal() {
    MeshStore store;
    store.add(announcement(1, 1, "First", "F"), -90, 1000);
    MeshPacket position = text(2, 2, "");
    position.port = 3;
    TEST_ASSERT_TRUE(store.add(position, -80, 2000));
    store.add(text(1, 3, "again"), -70, 3000);
    TEST_ASSERT_EQUAL(2, store.nodes().size());
    TEST_ASSERT_EQUAL_HEX32(1, store.nodes()[0].id);
    TEST_ASSERT_EQUAL_STRING("First", store.nodes()[0].longName.c_str());
    TEST_ASSERT_EQUAL(-70, store.nodes()[0].rssi);
    TEST_ASSERT_EQUAL_UINT32(3000, store.nodes()[0].lastHeardMs);
    TEST_ASSERT_EQUAL_HEX32(2, store.nodes()[1].id);
    TEST_ASSERT_EQUAL_STRING("", store.nodes()[1].longName.c_str());
}

static void test_own_packets_heard_back_are_not_a_node() {
    MeshStore store;
    store.setOwnNode(kOwn);
    store.addOwn(9, "hello", 0);
    store.add(text(kOwn, 9, "hello"), -70, 100);
    TEST_ASSERT_EQUAL(0, store.nodes().size());
}

static const uint8_t kAt1405[] = {0xEA, 0x07, 11, 19, 14, 5, 0, 4, 0, 0};

static void test_messages_arriving_with_the_clock_known_show_their_time() {
    MeshStore store;
    WallClock clock;
    store.setClock(&clock);
    store.setOwnNode(kOwn);
    store.add(text(1, 1, "before"), -90, 0);
    clock.setFromCurrentTime(kAt1405, sizeof(kAt1405), 1000);
    store.add(text(1, 2, "after"), -90, 1000);
    store.addOwn(3, "mine", 61000);
    TEST_ASSERT_EQUAL(-1, store.messages()[0].clockMinute);
    TEST_ASSERT_EQUAL(14 * 60 + 5, store.messages()[1].clockMinute);
    const std::vector<ChatLine> lines =
        meshLines(store, 10 * 60 * 1000, 100, [](const std::string& s) { return static_cast<int>(s.size()); });
    TEST_ASSERT_EQUAL_STRING("10m !0001", lines[0].text.c_str());
    TEST_ASSERT_EQUAL_STRING("14:05 !0001", lines[2].text.c_str());
    TEST_ASSERT_EQUAL_STRING("14:06 !3344 \xC2\xB7 sent", lines[4].text.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_a_text_is_listed_and_unread);
    RUN_TEST(test_the_same_packet_relayed_is_listed_once);
    RUN_TEST(test_marking_read_clears_the_count);
    RUN_TEST(test_an_unannounced_sender_is_named_by_its_id);
    RUN_TEST(test_an_announcement_names_the_sender_and_is_not_listed);
    RUN_TEST(test_direct_messages_and_other_ports_are_not_listed);
    RUN_TEST(test_only_the_last_500_messages_are_kept);
    RUN_TEST(test_elapsed_time_labels);
    RUN_TEST(test_lines_show_elapsed_time_sender_and_wrapped_text);
    RUN_TEST(test_an_own_message_reads_sent_and_is_not_unread);
    RUN_TEST(test_an_own_message_relayed_back_is_heard_and_not_listed_again);
    RUN_TEST(test_an_own_message_unheard_for_30_s_is_not_heard);
    RUN_TEST(test_a_failed_send_reads_not_sent);
    RUN_TEST(test_an_own_message_header_carries_its_mark);
    RUN_TEST(test_own_messages_carry_the_users_short_name);
    RUN_TEST(test_received_counts_texts_from_other_nodes_only);
    RUN_TEST(test_nodes_are_listed_newest_first_with_their_signal);
    RUN_TEST(test_own_packets_heard_back_are_not_a_node);
    RUN_TEST(test_messages_arriving_with_the_clock_known_show_their_time);
    return UNITY_END();
}
