#include <unity.h>

#include "core/meshtastic.h"

void setUp() {}
void tearDown() {}

// A node announcement heard in Leno on the public MediumFast channel, as the LoRa unit reported it.
static const char* kAnnouncement =
    "FFFFFFFFCCD3CCA0246F37D3A11F0068AB5AAE3063E75DC116C9B6B8AECF905682EC2AACD844FEC047265B0DE8323F6565BB712D756F"
    "BCBD07072B8B2BD4899320AAF28A0937D1E2384C0C552D8B54D2D616004A9EA4552C8A394A569B1F239113DBEE759DFF10F6FD64AC";

static std::vector<uint8_t> bytesOf(const char* hex) {
    std::vector<uint8_t> bytes;
    TEST_ASSERT_TRUE(parseHex(hex, bytes));
    return bytes;
}

static void test_public_channel_hash_is_that_of_medium_fast() {
    TEST_ASSERT_EQUAL_HEX8(0x1f, kPublicChannelHash);
}

static void test_reads_the_header_in_clear() {
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(bytesOf(kAnnouncement), packet));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, packet.to);
    TEST_ASSERT_EQUAL_HEX32(0xa0ccd3cc, packet.from);
    TEST_ASSERT_EQUAL_HEX32(0xd3376f24, packet.id);
    TEST_ASSERT_EQUAL_UINT8(1, packet.hopLimit);
    TEST_ASSERT_EQUAL_UINT8(5, packet.hopStart);
}

static void test_decrypts_a_public_node_announcement() {
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(bytesOf(kAnnouncement), packet));
    TEST_ASSERT_EQUAL_UINT32(kPortNodeInfo, packet.port);
    MeshUser user;
    TEST_ASSERT_TRUE(decodeMeshUser(packet.payload, user));
    TEST_ASSERT_EQUAL_STRING("RESS IU4LHV \xF0\x9F\x87\xAE\xF0\x9F\x87\xB9", user.longName.c_str());
    TEST_ASSERT_EQUAL_STRING("RESS", user.shortName.c_str());
}

// A text from !12345678 with ID 0x0000abcd: Data{portnum 1, payload "ciao"} encrypted as a node would.
static void test_decrypts_a_text_message() {
    std::vector<uint8_t> bytes = {0xff, 0xff, 0xff, 0xff, 0x78, 0x56, 0x34, 0x12, 0xcd, 0xab, 0x00, 0x00,
                                  0x63, kPublicChannelHash, 0x00, 0x00};
    uint8_t data[] = {0x08, 0x01, 0x12, 0x04, 'c', 'i', 'a', 'o'};
    meshCrypt(0x12345678, 0xabcd, data, sizeof(data));
    bytes.insert(bytes.end(), data, data + sizeof(data));
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(bytes, packet));
    TEST_ASSERT_EQUAL_UINT32(kPortText, packet.port);
    TEST_ASSERT_EQUAL_STRING("ciao", packet.payload.c_str());
    TEST_ASSERT_EQUAL_UINT8(3, packet.hopLimit);
    TEST_ASSERT_EQUAL_UINT8(3, packet.hopStart);
}

static void test_another_channel_is_not_decrypted() {
    std::vector<uint8_t> bytes = bytesOf(kAnnouncement);
    bytes[13] = 0x08;
    MeshPacket packet;
    TEST_ASSERT_FALSE(decodeMeshPacket(bytes, packet));
}

static void test_a_packet_shorter_than_its_header_is_refused() {
    std::vector<uint8_t> bytes(15, 0);
    MeshPacket packet;
    TEST_ASSERT_FALSE(decodeMeshPacket(bytes, packet));
}

static void test_bad_hex_is_refused() {
    std::vector<uint8_t> bytes;
    TEST_ASSERT_FALSE(parseHex("ABC", bytes));
    TEST_ASSERT_FALSE(parseHex("ZZ", bytes));
}

static void test_an_encoded_text_decodes_back() {
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(encodeMeshText(0xa0b1c2d3, 0x1234, "ciao è"), packet));
    TEST_ASSERT_EQUAL_HEX32(kMeshBroadcast, packet.to);
    TEST_ASSERT_EQUAL_HEX32(0xa0b1c2d3, packet.from);
    TEST_ASSERT_EQUAL_HEX32(0x1234, packet.id);
    TEST_ASSERT_EQUAL_UINT8(3, packet.hopLimit);
    TEST_ASSERT_EQUAL_UINT8(3, packet.hopStart);
    TEST_ASSERT_EQUAL_UINT32(kPortText, packet.port);
    TEST_ASSERT_EQUAL_STRING("ciao è", packet.payload.c_str());
}

static void test_a_200_byte_text_fits_one_packet() {
    const std::vector<uint8_t> bytes = encodeMeshText(1, 2, std::string(200, 'x'));
    TEST_ASSERT_EQUAL(16 + 2 + 3 + 200, bytes.size());
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(bytes, packet));
    TEST_ASSERT_EQUAL(200, packet.payload.size());
}

static void test_hex_is_two_upper_case_digits_per_byte() {
    TEST_ASSERT_EQUAL_STRING("00FFA1", toHex({0x00, 0xff, 0xa1}).c_str());
}

static void test_an_encoded_announcement_decodes_back() {
    MeshPacket packet;
    TEST_ASSERT_TRUE(decodeMeshPacket(encodeMeshUser(0x84969f8d, 7, {"Carlo", "Carl"}), packet));
    TEST_ASSERT_EQUAL_UINT32(kPortNodeInfo, packet.port);
    MeshUser user;
    TEST_ASSERT_TRUE(decodeMeshUser(packet.payload, user));
    TEST_ASSERT_EQUAL_STRING("Carlo", user.longName.c_str());
    TEST_ASSERT_EQUAL_STRING("Carl", user.shortName.c_str());
    // User.id, field 1, is the node ID as Meshtastic writes it.
    TEST_ASSERT_EQUAL_STRING("\x0a\x09!84969f8d", packet.payload.substr(0, 11).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_public_channel_hash_is_that_of_medium_fast);
    RUN_TEST(test_reads_the_header_in_clear);
    RUN_TEST(test_decrypts_a_public_node_announcement);
    RUN_TEST(test_decrypts_a_text_message);
    RUN_TEST(test_another_channel_is_not_decrypted);
    RUN_TEST(test_a_packet_shorter_than_its_header_is_refused);
    RUN_TEST(test_bad_hex_is_refused);
    RUN_TEST(test_an_encoded_text_decodes_back);
    RUN_TEST(test_a_200_byte_text_fits_one_packet);
    RUN_TEST(test_hex_is_two_upper_case_digits_per_byte);
    RUN_TEST(test_an_encoded_announcement_decodes_back);
    return UNITY_END();
}
