#include <string.h>
#include <unity.h>

#include <string>
#include <vector>

#include "core/ancs_request.h"

using Bytes = std::vector<uint8_t>;

static const uint32_t kUid = 0x0403026d;

// CommandID 0, UID little-endian, then the requested tuples: AppIdentifier, Title, Subtitle, Message, Date.
static Bytes header(uint32_t uid) {
    return {0x00, static_cast<uint8_t>(uid), static_cast<uint8_t>(uid >> 8), static_cast<uint8_t>(uid >> 16),
            static_cast<uint8_t>(uid >> 24)};
}

static void appendTuple(Bytes& out, uint8_t id, const char* value) {
    const size_t length = strlen(value);
    out.push_back(id);
    out.push_back(static_cast<uint8_t>(length));
    out.push_back(static_cast<uint8_t>(length >> 8));
    out.insert(out.end(), value, value + length);
}

static Bytes fullResponse(uint32_t uid) {
    Bytes out = header(uid);
    appendTuple(out, 0, "net.whatsapp.WhatsApp");
    appendTuple(out, 1, "Anna");
    appendTuple(out, 2, "");
    appendTuple(out, 3, "Ciao!");
    appendTuple(out, 5, "20261112T094800");
    return out;
}

static AncsResponseDecoder::Result feed(AncsResponseDecoder& decoder, const Bytes& packet) {
    return decoder.feed(packet.data(), packet.size());
}

void setUp() {}
void tearDown() {}

static void test_request_asks_the_five_attributes_with_their_limits() {
    uint8_t out[kAncsRequestSize];
    TEST_ASSERT_EQUAL(16, encodeGetNotificationAttributes(out, kUid));
    const uint8_t expected[] = {0x00, 0x6d, 0x02, 0x03, 0x04, 0x00, 0x01, 0x40, 0x00,
                                0x02, 0x40, 0x00, 0x03, 0x00, 0x02, 0x05};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, out, sizeof(expected));
}

static void test_events_are_decoded_with_kind_uid_and_preexisting_flag() {
    const uint8_t added[] = {0x00, 0x14, 0x04, 0x01, 0x6d, 0x02, 0x03, 0x04};  // flags: PreExisting + Important
    const uint8_t modified[] = {0x01, 0x00, 0x04, 0x01, 0x6d, 0x02, 0x03, 0x04};
    const uint8_t removed[] = {0x02, 0x00, 0x04, 0x01, 0x6d, 0x02, 0x03, 0x04};
    AncsEvent event;
    TEST_ASSERT_TRUE(decodeAncsEvent(added, sizeof(added), &event));
    TEST_ASSERT_TRUE(event.kind == AncsEvent::Kind::Added);
    TEST_ASSERT_TRUE(event.preExisting);
    TEST_ASSERT_EQUAL_HEX32(kUid, event.uid);
    TEST_ASSERT_TRUE(decodeAncsEvent(modified, sizeof(modified), &event));
    TEST_ASSERT_TRUE(event.kind == AncsEvent::Kind::Modified);
    TEST_ASSERT_FALSE(event.preExisting);
    TEST_ASSERT_TRUE(decodeAncsEvent(removed, sizeof(removed), &event));
    TEST_ASSERT_TRUE(event.kind == AncsEvent::Kind::Removed);
}

static void test_event_category_is_decoded() {
    const uint8_t social[] = {0x00, 0x00, 0x04, 0x01, 0x6d, 0x02, 0x03, 0x04};
    const uint8_t email[] = {0x00, 0x00, 0x06, 0x01, 0x6d, 0x02, 0x03, 0x04};
    AncsEvent event;
    TEST_ASSERT_TRUE(decodeAncsEvent(social, sizeof(social), &event));
    TEST_ASSERT_EQUAL_UINT8(4, event.category);
    TEST_ASSERT_TRUE(decodeAncsEvent(email, sizeof(email), &event));
    TEST_ASSERT_EQUAL_UINT8(6, event.category);
}

static void test_short_or_unknown_event_is_rejected() {
    const uint8_t truncated[] = {0x00, 0x10, 0x04, 0x01, 0x6d};
    const uint8_t unknown[] = {0x07, 0x00, 0x04, 0x01, 0x6d, 0x02, 0x03, 0x04};
    AncsEvent event;
    TEST_ASSERT_FALSE(decodeAncsEvent(truncated, sizeof(truncated), &event));
    TEST_ASSERT_FALSE(decodeAncsEvent(unknown, sizeof(unknown), &event));
}

static void test_response_in_one_packet_completes() {
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Complete, feed(decoder, fullResponse(kUid)));
    TEST_ASSERT_FALSE(decoder.active());
}

static void test_response_split_inside_a_length_field_completes_on_the_last_fragment() {
    const Bytes response = fullResponse(kUid);
    // header (5) + AppIdentifier id + first length byte: the cut falls inside the 2-byte length.
    const size_t cuts[] = {7, 20, 40};
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    size_t from = 0;
    for (size_t cut : cuts) {
        TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Progress,
                          feed(decoder, Bytes(response.begin() + from, response.begin() + cut)));
        from = cut;
    }
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Complete, feed(decoder, Bytes(response.begin() + from, response.end())));
}

static void test_all_attributes_empty_completes() {
    Bytes response = header(kUid);
    for (uint8_t id : {0, 1, 2, 3, 5}) appendTuple(response, id, "");
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Complete, feed(decoder, response));
}

static void test_response_for_another_uid_is_ignored() {
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Ignored, feed(decoder, fullResponse(kUid + 1)));
    TEST_ASSERT_TRUE(decoder.active());
}

static void test_late_fragment_of_a_timed_out_response_is_not_counted() {
    const Bytes old = fullResponse(kUid);
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    feed(decoder, Bytes(old.begin(), old.begin() + 20));
    // Timeout: the next request starts while the rest of the old response is still on its way.
    decoder.start(kUid + 1);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Ignored, feed(decoder, Bytes(old.begin() + 20, old.end())));
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Complete, feed(decoder, fullResponse(kUid + 1)));
}

static void test_packet_with_another_command_is_ignored() {
    Bytes response = fullResponse(kUid);
    response[0] = 0x01;
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Ignored, feed(decoder, response));
}

static void test_packet_with_no_request_in_flight_is_ignored() {
    AncsResponseDecoder decoder;
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Ignored, feed(decoder, fullResponse(kUid)));
    decoder.start(kUid);
    decoder.clear();
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Ignored, feed(decoder, fullResponse(kUid)));
}

static void test_values_are_decoded_across_packets() {
    Bytes response = header(kUid);
    appendTuple(response, 0, "net.whatsapp.WhatsApp");
    appendTuple(response, 1, "Luca Bianchi");
    appendTuple(response, 2, "Calcetto \xF0\x9F\x8D\xBB");
    appendTuple(response, 3, "Perch\xC3\xA9 no? Ci vediamo alle 8");
    appendTuple(response, 5, "20261112T094800");
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    for (size_t from = 0; from < response.size(); from += 7) {
        const size_t to = from + 7 < response.size() ? from + 7 : response.size();
        feed(decoder, Bytes(response.begin() + from, response.begin() + to));
    }
    const AncsAttributes& a = decoder.attributes();
    TEST_ASSERT_EQUAL_STRING("net.whatsapp.WhatsApp", a.app.c_str());
    TEST_ASSERT_EQUAL_STRING("Luca Bianchi", a.title.c_str());
    TEST_ASSERT_EQUAL_STRING("Calcetto \xF0\x9F\x8D\xBB", a.subtitle.c_str());
    TEST_ASSERT_EQUAL_STRING("Perch\xC3\xA9 no? Ci vediamo alle 8", a.message.c_str());
    TEST_ASSERT_EQUAL_STRING("20261112T094800", a.date.c_str());
}

static void test_empty_values_are_decoded_as_empty() {
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    feed(decoder, fullResponse(kUid));
    TEST_ASSERT_EQUAL_STRING("Anna", decoder.attributes().title.c_str());
    TEST_ASSERT_EQUAL_STRING("", decoder.attributes().subtitle.c_str());
}

static void test_value_longer_than_requested_is_malformed() {
    Bytes response = header(kUid);
    appendTuple(response, 0, "net.whatsapp.WhatsApp");
    appendTuple(response, 1, std::string(65, 'a').c_str());
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Malformed, feed(decoder, response));
    TEST_ASSERT_FALSE(decoder.active());
}

static void test_unrequested_attribute_is_malformed() {
    Bytes response = header(kUid);
    appendTuple(response, 4, "12");  // MessageSize, not requested
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Malformed, feed(decoder, response));
}

static void test_bytes_after_the_last_attribute_are_malformed() {
    Bytes response = fullResponse(kUid);
    response.push_back(0x07);
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Malformed, feed(decoder, response));
}

static void test_character_cut_by_the_length_limit_is_dropped() {
    Bytes response = header(kUid);
    appendTuple(response, 0, "net.whatsapp.WhatsApp");
    appendTuple(response, 1, "Anna");
    appendTuple(response, 2, "");
    appendTuple(response, 3, "ok \xF0\x9F\x8D");  // beer emoji missing its last byte
    appendTuple(response, 5, "20261112T094800");
    AncsResponseDecoder decoder;
    decoder.start(kUid);
    TEST_ASSERT_EQUAL(AncsResponseDecoder::Result::Complete, feed(decoder, response));
    TEST_ASSERT_EQUAL_STRING("ok ", decoder.attributes().message.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_request_asks_the_five_attributes_with_their_limits);
    RUN_TEST(test_events_are_decoded_with_kind_uid_and_preexisting_flag);
    RUN_TEST(test_event_category_is_decoded);
    RUN_TEST(test_short_or_unknown_event_is_rejected);
    RUN_TEST(test_response_in_one_packet_completes);
    RUN_TEST(test_response_split_inside_a_length_field_completes_on_the_last_fragment);
    RUN_TEST(test_all_attributes_empty_completes);
    RUN_TEST(test_response_for_another_uid_is_ignored);
    RUN_TEST(test_late_fragment_of_a_timed_out_response_is_not_counted);
    RUN_TEST(test_packet_with_another_command_is_ignored);
    RUN_TEST(test_packet_with_no_request_in_flight_is_ignored);
    RUN_TEST(test_values_are_decoded_across_packets);
    RUN_TEST(test_empty_values_are_decoded_as_empty);
    RUN_TEST(test_value_longer_than_requested_is_malformed);
    RUN_TEST(test_unrequested_attribute_is_malformed);
    RUN_TEST(test_bytes_after_the_last_attribute_are_malformed);
    RUN_TEST(test_character_cut_by_the_length_limit_is_dropped);
    return UNITY_END();
}
