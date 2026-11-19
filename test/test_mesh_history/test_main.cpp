#include <unity.h>

#include "core/mesh_history.h"
#include "core/mesh_view.h"

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

static constexpr uint32_t kOwn = 0x11223344;

static void test_a_message_survives_a_restart() {
    MeshMessage m;
    m.from = 0xa0ccd3cc;
    m.id = 0x1234;
    m.text = "tab\there\nnew \\ line";
    m.clockMinute = 14 * 60 + 5;
    const std::string line = historyMessageLine(m);
    TEST_ASSERT_EQUAL('\n', line.back());
    TEST_ASSERT_EQUAL(std::string::npos, line.substr(0, line.size() - 1).find('\n'));

    MeshStore store;
    TEST_ASSERT_TRUE(loadHistoryLine(line.substr(0, line.size() - 1), store));
    TEST_ASSERT_EQUAL(1, store.messages().size());
    const MeshMessage& back = store.messages().back();
    TEST_ASSERT_EQUAL_HEX32(0xa0ccd3cc, back.from);
    TEST_ASSERT_EQUAL_HEX32(0x1234, back.id);
    TEST_ASSERT_EQUAL_STRING("tab\there\nnew \\ line", back.text.c_str());
    TEST_ASSERT_EQUAL(14 * 60 + 5, back.clockMinute);
    TEST_ASSERT_TRUE(back.restored);
    TEST_ASSERT_EQUAL(0, store.unread());
}

static void test_a_stored_message_without_time_reads_dashes() {
    MeshMessage m;
    m.from = 1;
    m.id = 2;
    m.text = "old";
    const std::string line = historyMessageLine(m);
    MeshStore store;
    loadHistoryLine(line.substr(0, line.size() - 1), store);
    const std::vector<ChatLine> lines =
        meshLines(store, 600000, 100, [](const std::string& s) { return static_cast<int>(s.size()); });
    TEST_ASSERT_EQUAL_STRING("--:-- !0001", lines[0].text.c_str());
}

static void test_a_later_delivery_line_updates_the_own_message() {
    MeshMessage m;
    m.from = kOwn;
    m.id = 9;
    m.text = "mine";
    m.delivery = Delivery::Sent;
    MeshStore store;
    store.setOwnNode(kOwn);
    std::string line = historyMessageLine(m);
    loadHistoryLine(line.substr(0, line.size() - 1), store);
    line = historyDeliveryLine(9, Delivery::Heard);
    TEST_ASSERT_TRUE(loadHistoryLine(line.substr(0, line.size() - 1), store));
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::Heard), static_cast<int>(store.messages().back().delivery));
}

static void test_an_own_message_still_sent_at_restart_reads_not_heard() {
    MeshMessage m;
    m.from = kOwn;
    m.id = 9;
    m.text = "mine";
    m.delivery = Delivery::Sent;
    MeshStore store;
    store.setOwnNode(kOwn);
    const std::string line = historyMessageLine(m);
    loadHistoryLine(line.substr(0, line.size() - 1), store);
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::NotHeard), static_cast<int>(store.messages().back().delivery));
    TEST_ASSERT_TRUE(store.takeDeliveryChanges().empty());
}

static void test_a_node_survives_a_restart() {
    MeshNode n;
    n.id = 0xa0ccd3cc;
    n.longName = "RESS\tIU4LHV";
    n.shortName = "RESS";
    n.rssi = -97;
    const std::string line = historyNodeLine(n);
    MeshStore store;
    TEST_ASSERT_TRUE(loadHistoryLine(line.substr(0, line.size() - 1), store));
    TEST_ASSERT_EQUAL(1, store.nodes().size());
    TEST_ASSERT_EQUAL_STRING("RESS\tIU4LHV", store.nodes()[0].longName.c_str());
    TEST_ASSERT_EQUAL_STRING("RESS", store.nodes()[0].shortName.c_str());
    TEST_ASSERT_EQUAL(-97, store.nodes()[0].rssi);
    TEST_ASSERT_TRUE(store.nodes()[0].restored);
    store.add(text(0xa0ccd3cc, 1, "hi"), -80, 5000);
    TEST_ASSERT_FALSE(store.nodes()[0].restored);
}

static void test_a_relayed_copy_of_a_stored_message_is_not_listed_again() {
    MeshMessage m;
    m.from = 1;
    m.id = 2;
    m.text = "old";
    const std::string line = historyMessageLine(m);
    MeshStore store;
    loadHistoryLine(line.substr(0, line.size() - 1), store);
    TEST_ASSERT_FALSE(store.add(text(1, 2, "old"), -90, 1000));
    TEST_ASSERT_EQUAL(1, store.messages().size());
}

static void test_lines_that_are_not_records_are_skipped() {
    MeshStore store;
    TEST_ASSERT_FALSE(loadHistoryLine("", store));
    TEST_ASSERT_FALSE(loadHistoryLine("M\t0000", store));
    TEST_ASSERT_FALSE(loadHistoryLine("X\t1\t2", store));
    TEST_ASSERT_FALSE(loadHistoryLine("M\tzz\t1\t-1\t0\thi", store));
    TEST_ASSERT_FALSE(loadHistoryLine("M\t1\t1\t-1\t9\thi", store));
    TEST_ASSERT_EQUAL(0, store.messages().size());
}

static void test_the_store_reports_what_to_save() {
    MeshStore store;
    store.setOwnNode(kOwn);
    const uint32_t nodes = store.nodesChanged();
    store.add(text(1, 1, "a"), -90, 0);
    store.addOwn(2, "b", 0);
    TEST_ASSERT_EQUAL(2, store.added());
    TEST_ASSERT_NOT_EQUAL(nodes, store.nodesChanged());
    store.markNotSent(2);
    const std::vector<std::pair<uint32_t, Delivery>> changes = store.takeDeliveryChanges();
    TEST_ASSERT_EQUAL(1, changes.size());
    TEST_ASSERT_EQUAL_HEX32(2, changes[0].first);
    TEST_ASSERT_EQUAL(static_cast<int>(Delivery::NotSent), static_cast<int>(changes[0].second));
    TEST_ASSERT_TRUE(store.takeDeliveryChanges().empty());
}

static void test_clearing_removes_every_message_and_keeps_the_nodes() {
    MeshStore store;
    store.add(text(1, 1, "a"), -90, 0);
    store.clearMessages();
    TEST_ASSERT_EQUAL(0, store.messages().size());
    TEST_ASSERT_EQUAL(0, store.unread());
    TEST_ASSERT_EQUAL(1, store.nodes().size());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_a_message_survives_a_restart);
    RUN_TEST(test_a_stored_message_without_time_reads_dashes);
    RUN_TEST(test_a_later_delivery_line_updates_the_own_message);
    RUN_TEST(test_an_own_message_still_sent_at_restart_reads_not_heard);
    RUN_TEST(test_a_node_survives_a_restart);
    RUN_TEST(test_a_relayed_copy_of_a_stored_message_is_not_listed_again);
    RUN_TEST(test_lines_that_are_not_records_are_skipped);
    RUN_TEST(test_the_store_reports_what_to_save);
    RUN_TEST(test_clearing_removes_every_message_and_keeps_the_nodes);
    return UNITY_END();
}
