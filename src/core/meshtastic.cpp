#include "core/meshtastic.h"

#include <stdio.h>

#include "core/aes128.h"

namespace {

constexpr size_t kHeaderSize = 16;
constexpr uint8_t kHops = 3;
constexpr uint32_t kRoleClientMute = 1;
// The public channel's key ("AQ==" in the Meshtastic apps) and the radio preset that names it.
constexpr uint8_t kPublicKey[16] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                    0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
constexpr char kPublicChannelName[] = "MediumFast";

constexpr uint8_t channelHash() {
    uint8_t hash = 0;
    for (size_t i = 0; i + 1 < sizeof(kPublicChannelName); i++) {
        hash ^= static_cast<uint8_t>(kPublicChannelName[i]);
    }
    for (uint8_t b : kPublicKey) {
        hash ^= b;
    }
    return hash;
}

void writeLe32(std::vector<uint8_t>& out, uint32_t value) {
    for (int i = 0; i < 4; i++) {
        out.push_back(static_cast<uint8_t>(value >> (8 * i)));
    }
}

void writeVarint(std::vector<uint8_t>& out, uint64_t value) {
    while (value >= 0x80) {
        out.push_back(static_cast<uint8_t>(value | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<uint8_t>(value));
}

void writeBytes(std::vector<uint8_t>& out, uint8_t tag, const std::string& bytes) {
    out.push_back(tag);
    writeVarint(out, bytes.size());
    out.insert(out.end(), bytes.begin(), bytes.end());
}

uint32_t readLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | static_cast<uint32_t>(p[1]) << 8 | static_cast<uint32_t>(p[2]) << 16 |
           static_cast<uint32_t>(p[3]) << 24;
}

int hexDigit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

// The protobuf wire format, only as far as Data and User need it.
class ProtoReader {
public:
    explicit ProtoReader(const std::string& buffer) : data_(buffer) {}

    // Next field: its number, and its value as a number or as bytes; false at the end or on a malformed field.
    bool next(uint32_t& field, uint64_t& number, std::string& bytes) {
        uint64_t tag = 0;
        if (pos_ >= data_.size() || !varint(tag)) {
            return false;
        }
        field = static_cast<uint32_t>(tag >> 3);
        switch (tag & 7) {
            case 0:
                return varint(number);
            case 1:
                return skip(8);
            case 2: {
                uint64_t length = 0;
                if (!varint(length) || length > data_.size() - pos_) {
                    return false;
                }
                bytes = data_.substr(pos_, static_cast<size_t>(length));
                pos_ += static_cast<size_t>(length);
                return true;
            }
            case 5:
                return skip(4);
            default:
                return false;
        }
    }

    bool atEnd() const { return pos_ == data_.size(); }

private:
    bool varint(uint64_t& value) {
        value = 0;
        for (int shift = 0; shift < 64 && pos_ < data_.size(); shift += 7) {
            const uint8_t b = static_cast<uint8_t>(data_[pos_++]);
            value |= static_cast<uint64_t>(b & 0x7f) << shift;
            if ((b & 0x80) == 0) {
                return true;
            }
        }
        return false;
    }

    bool skip(size_t count) {
        if (count > data_.size() - pos_) {
            return false;
        }
        pos_ += count;
        return true;
    }

    const std::string& data_;
    size_t pos_ = 0;
};

constexpr uint8_t kChannelHash = channelHash();

// A broadcast packet from from with a Data message of port carrying payload, encrypted.
std::vector<uint8_t> encodeMeshData(uint32_t from, uint32_t id, uint32_t port, const std::string& payload) {
    std::vector<uint8_t> packet;
    writeLe32(packet, kMeshBroadcast);
    writeLe32(packet, from);
    writeLe32(packet, id);
    // Hop limit and hop start; the next hop is anyone, and the last relay is the sender itself.
    packet.push_back(static_cast<uint8_t>(kHops | kHops << 5));
    packet.push_back(kChannelHash);
    packet.push_back(0);
    packet.push_back(static_cast<uint8_t>(from & 0xff));
    // Data: portnum (field 1), payload (field 2).
    packet.push_back(0x08);
    writeVarint(packet, port);
    writeBytes(packet, 0x12, payload);
    meshCrypt(from, id, &packet[kHeaderSize], packet.size() - kHeaderSize);
    return packet;
}

}  // namespace

const uint8_t kPublicChannelHash = kChannelHash;

std::string toHex(const std::vector<uint8_t>& bytes) {
    static const char kDigits[] = "0123456789ABCDEF";
    std::string hex;
    hex.reserve(bytes.size() * 2);
    for (uint8_t b : bytes) {
        hex += kDigits[b >> 4];
        hex += kDigits[b & 0x0f];
    }
    return hex;
}

bool parseHex(const std::string& hex, std::vector<uint8_t>& bytes) {
    if (hex.size() % 2 != 0) {
        return false;
    }
    bytes.clear();
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        const int high = hexDigit(hex[i]);
        const int low = hexDigit(hex[i + 1]);
        if (high < 0 || low < 0) {
            return false;
        }
        bytes.push_back(static_cast<uint8_t>(high << 4 | low));
    }
    return true;
}

void meshCrypt(uint32_t from, uint32_t id, uint8_t* data, size_t length) {
    // The counter block: the packet ID as 64 bits, then the sender, little-endian, then zeros.
    uint8_t counter[16] = {};
    for (int i = 0; i < 4; i++) {
        counter[i] = static_cast<uint8_t>(id >> (8 * i));
        counter[8 + i] = static_cast<uint8_t>(from >> (8 * i));
    }
    static const Aes128 aes(kPublicKey);
    aes.ctr(counter, data, length);
}

bool decodeMeshPacket(const std::vector<uint8_t>& bytes, MeshPacket& packet) {
    if (bytes.size() < kHeaderSize || bytes[13] != kPublicChannelHash) {
        return false;
    }
    packet.to = readLe32(&bytes[0]);
    packet.from = readLe32(&bytes[4]);
    packet.id = readLe32(&bytes[8]);
    packet.hopLimit = bytes[12] & 0x07;
    packet.hopStart = (bytes[12] >> 5) & 0x07;
    std::string plain(bytes.begin() + kHeaderSize, bytes.end());
    meshCrypt(packet.from, packet.id, reinterpret_cast<uint8_t*>(&plain[0]), plain.size());

    ProtoReader reader(plain);
    uint32_t field = 0;
    uint64_t number = 0;
    std::string value;
    bool hasPort = false;
    packet.payload.clear();
    while (reader.next(field, number, value)) {
        if (field == 1) {
            packet.port = static_cast<uint32_t>(number);
            hasPort = true;
        } else if (field == 2) {
            packet.payload = value;
        }
    }
    return hasPort && reader.atEnd();
}

std::vector<uint8_t> encodeMeshText(uint32_t from, uint32_t id, const std::string& text) {
    return encodeMeshData(from, id, kPortText, text);
}

std::vector<uint8_t> encodeMeshUser(uint32_t from, uint32_t id, const MeshUser& user) {
    char nodeId[10];
    snprintf(nodeId, sizeof(nodeId), "!%08x", static_cast<unsigned>(from));
    // User: id (1), long_name (2), short_name (3), role (7) CLIENT_MUTE.
    std::vector<uint8_t> payload;
    writeBytes(payload, 0x0a, nodeId);
    writeBytes(payload, 0x12, user.longName);
    writeBytes(payload, 0x1a, user.shortName);
    payload.push_back(0x38);
    writeVarint(payload, kRoleClientMute);
    return encodeMeshData(from, id, kPortNodeInfo, std::string(payload.begin(), payload.end()));
}

bool decodeMeshUser(const std::string& payload, MeshUser& user) {
    ProtoReader reader(payload);
    uint32_t field = 0;
    uint64_t number = 0;
    std::string value;
    user = MeshUser{};
    while (reader.next(field, number, value)) {
        if (field == 2) {
            user.longName = value;
        } else if (field == 3) {
            user.shortName = value;
        }
    }
    return reader.atEnd();
}
