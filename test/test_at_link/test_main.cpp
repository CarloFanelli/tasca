#include <unity.h>

#include <string>
#include <vector>

#include "core/at_link.h"

void setUp() {}
void tearDown() {}

static const std::vector<AtStep> kSetup = {{"AT+NWM=0", 1000}, {"AT+PRECV=65535", 0}};

// Answers every command the link sends with reply, at time t; returns the commands sent.
static std::vector<std::string> run(AtLink& link, uint32_t from, uint32_t to, const char* reply) {
    std::vector<std::string> sent;
    for (uint32_t t = from; t <= to; t += 10) {
        const char* command = link.due(t);
        if (command != nullptr) {
            sent.push_back(command);
            if (reply != nullptr) {
                link.onLine(reply, t);
            }
        }
    }
    return sent;
}

static void test_probes_while_absent_and_stays_absent_without_answer() {
    AtLink link(kSetup);
    const std::vector<std::string> sent = run(link, 0, 2990, nullptr);
    TEST_ASSERT_EQUAL(3, sent.size());
    TEST_ASSERT_EQUAL_STRING("AT", sent[0].c_str());
    TEST_ASSERT_FALSE(link.present());
}

static void test_an_answer_runs_the_setup_then_listens() {
    AtLink link(kSetup);
    const std::vector<std::string> sent = run(link, 0, 1100, "OK");
    TEST_ASSERT_EQUAL(3, sent.size());
    TEST_ASSERT_EQUAL_STRING("AT+NWM=0", sent[1].c_str());
    TEST_ASSERT_EQUAL_STRING("AT+PRECV=65535", sent[2].c_str());
    TEST_ASSERT_TRUE(link.present());
}

// The unit restarts after a mode change: the next command waits for it.
static void test_a_setup_step_waits_for_the_unit_to_settle() {
    AtLink link(kSetup);
    TEST_ASSERT_EQUAL_STRING("AT", link.due(0));
    link.onLine("OK", 5);
    TEST_ASSERT_EQUAL_STRING("AT+NWM=0", link.due(10));
    link.onLine("OK", 20);
    link.onLine("Current Work Mode: LoRa P2P.", 30);
    TEST_ASSERT_NULL(link.due(500));
    TEST_ASSERT_NULL(link.due(1000));
    TEST_ASSERT_EQUAL_STRING("AT+PRECV=65535", link.due(1020));
}

static void test_a_refused_setup_command_means_absent() {
    AtLink link(kSetup);
    TEST_ASSERT_EQUAL_STRING("AT", link.due(0));
    link.onLine("OK", 5);
    TEST_ASSERT_EQUAL_STRING("AT+NWM=0", link.due(10));
    link.onLine("AT_PARAM_ERROR", 15);
    TEST_ASSERT_FALSE(link.present());
    TEST_ASSERT_NULL(link.due(20));
}

static void test_a_unit_that_stops_answering_is_absent_within_5_s() {
    AtLink link(kSetup);
    run(link, 0, 1100, "OK");
    TEST_ASSERT_TRUE(link.present());
    run(link, 1110, 6100, nullptr);
    TEST_ASSERT_FALSE(link.present());
}

static void test_a_listening_unit_that_answers_stays_present() {
    AtLink link(kSetup);
    run(link, 0, 1100, "OK");
    run(link, 1110, 20000, "OK");
    TEST_ASSERT_TRUE(link.present());
}

static void test_a_received_packet_is_taken_once() {
    AtLink link(kSetup);
    run(link, 0, 1100, "OK");
    link.onLine("+EVT:RXP2P:-104:-14:FFFF0102", 1200);
    LoraPacket packet;
    TEST_ASSERT_TRUE(link.takePacket(packet));
    TEST_ASSERT_EQUAL(-104, packet.rssi);
    TEST_ASSERT_EQUAL(-14, packet.snr);
    TEST_ASSERT_EQUAL_STRING("FFFF0102", packet.hex.c_str());
    TEST_ASSERT_FALSE(link.takePacket(packet));
}

static void test_lines_end_at_cr_or_lf_and_overlong_ones_are_dropped() {
    LineSplitter splitter(8);
    std::string line;
    std::vector<std::string> lines;
    for (char c : std::string("OK\r\n0123456789\nAB\n")) {
        if (splitter.add(c, line)) {
            lines.push_back(line);
        }
    }
    TEST_ASSERT_EQUAL(2, lines.size());
    TEST_ASSERT_EQUAL_STRING("OK", lines[0].c_str());
    TEST_ASSERT_EQUAL_STRING("AB", lines[1].c_str());
}

// Listening must stop before the unit accepts a packet, and resume once it went out.
static void test_a_send_stops_listening_transmits_then_listens_again() {
    AtLink link(kSetup);
    run(link, 0, 1100, "OK");
    TEST_ASSERT_TRUE(link.send("0102"));
    TEST_ASSERT_FALSE(link.send("0304"));
    TEST_ASSERT_EQUAL_STRING("AT+PRECV=0", link.due(1200));
    link.onLine("OK", 1210);
    TEST_ASSERT_EQUAL_STRING("AT+PSEND=0102", link.due(1220));
    link.onLine("OK", 1230);
    TEST_ASSERT_NULL(link.due(1600));
    link.onLine("+EVT:TXP2P DONE", 1700);
    TEST_ASSERT_EQUAL_STRING("AT+PRECV=65535", link.due(1710));
    link.onLine("OK", 1720);
    TEST_ASSERT_EQUAL(static_cast<int>(SendResult::Sent), static_cast<int>(link.takeSendResult()));
    TEST_ASSERT_EQUAL(static_cast<int>(SendResult::None), static_cast<int>(link.takeSendResult()));
    TEST_ASSERT_TRUE(link.present());
}

static void test_a_send_without_the_unit_is_refused() {
    AtLink link(kSetup);
    TEST_ASSERT_FALSE(link.send("0102"));
}

static void test_a_unit_gone_while_sending_fails_the_send() {
    AtLink link(kSetup);
    run(link, 0, 1100, "OK");
    link.send("0102");
    TEST_ASSERT_EQUAL_STRING("AT+PRECV=0", link.due(1200));
    link.onLine("OK", 1210);
    TEST_ASSERT_EQUAL_STRING("AT+PSEND=0102", link.due(1220));
    link.onLine("OK", 1230);
    run(link, 1240, 6000, nullptr);
    TEST_ASSERT_EQUAL(static_cast<int>(SendResult::Failed), static_cast<int>(link.takeSendResult()));
    TEST_ASSERT_FALSE(link.present());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_probes_while_absent_and_stays_absent_without_answer);
    RUN_TEST(test_an_answer_runs_the_setup_then_listens);
    RUN_TEST(test_a_setup_step_waits_for_the_unit_to_settle);
    RUN_TEST(test_a_refused_setup_command_means_absent);
    RUN_TEST(test_a_unit_that_stops_answering_is_absent_within_5_s);
    RUN_TEST(test_a_listening_unit_that_answers_stays_present);
    RUN_TEST(test_a_received_packet_is_taken_once);
    RUN_TEST(test_lines_end_at_cr_or_lf_and_overlong_ones_are_dropped);
    RUN_TEST(test_a_send_stops_listening_transmits_then_listens_again);
    RUN_TEST(test_a_send_without_the_unit_is_refused);
    RUN_TEST(test_a_unit_gone_while_sending_fails_the_send);
    return UNITY_END();
}
