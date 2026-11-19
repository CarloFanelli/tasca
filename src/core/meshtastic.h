#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

// Meshtastic packets on the public channel: a 16-byte header in clear, then a protobuf Data message
// encrypted with AES-128-CTR under the channel key that Meshtastic publishes.

constexpr uint32_t kMeshBroadcast = 0xFFFFFFFF;
constexpr uint32_t kPortText = 1;
constexpr uint32_t kPortNodeInfo = 4;

// The channel byte of the header: the XOR of the channel name's bytes and of the key's.
extern const uint8_t kPublicChannelHash;

struct MeshPacket {
    uint32_t to = 0;
    uint32_t from = 0;
    uint32_t id = 0;
    uint8_t hopLimit = 0;
    uint8_t hopStart = 0;
    uint32_t port = 0;
    // The Data message's payload, its meaning set by port.
    std::string payload;
};

// A node's announcement.
struct MeshUser {
    std::string longName;
    std::string shortName;
};

// Two upper-case hex digits per byte.
std::string toHex(const std::vector<uint8_t>& bytes);
// Hex text, two digits per byte, either case; false on an odd length or another character.
bool parseHex(const std::string& hex, std::vector<uint8_t>& bytes);

// Encrypts or decrypts, in place, the part after the header of a packet from from with id.
void meshCrypt(uint32_t from, uint32_t id, uint8_t* data, size_t length);

// False when the packet is shorter than its header, belongs to another channel, or does not decrypt to
// a Data message.
bool decodeMeshPacket(const std::vector<uint8_t>& bytes, MeshPacket& packet);

// The announcement of a node's names to every node on the public channel, as a client that does not relay.
std::vector<uint8_t> encodeMeshUser(uint32_t from, uint32_t id, const MeshUser& user);

// A text to every node on the public channel, from from with packet ID id, ready for the radio: up to 3 hops.
std::vector<uint8_t> encodeMeshText(uint32_t from, uint32_t id, const std::string& text);

// The names of a node announcement's payload; false when it is not a User message.
bool decodeMeshUser(const std::string& payload, MeshUser& user);
