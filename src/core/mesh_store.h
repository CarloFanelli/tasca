#pragma once

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "core/meshtastic.h"
#include "core/wall_clock.h"

// What became of a message: received from another node, or the user's own, sent and then heard by a node
// rebroadcasting it, not heard within 30 s, or not sent at all.
enum class Delivery { Received, Sent, Heard, NotHeard, NotSent };

// A text message of the public channel.
struct MeshMessage {
    uint32_t from = 0;
    uint32_t id = 0;
    std::string text;
    // millis() when it arrived or was sent.
    uint32_t arrivedMs = 0;
    // The minute of the day it arrived, from the iPhone's clock; -1 when the clock was not known.
    int clockMinute = -1;
    Delivery delivery = Delivery::Received;
    // Loaded from the microSD card at startup: its age is not known.
    bool restored = false;
};

// A node heard on the public channel.
struct MeshNode {
    uint32_t id = 0;
    std::string longName;
    std::string shortName;
    int rssi = 0;
    uint32_t lastHeardMs = 0;
    // Loaded from the microSD card and not heard since startup: lastHeardMs means nothing.
    bool restored = false;
};

// The public channel's text messages, oldest first, and the nodes heard. loop() only.
class MeshStore {
public:
    // The Cardputer's node ID: its own packets heard back mark its messages heard.
    void setOwnNode(uint32_t node) { ownNode_ = node; }
    // The clock that stamps arriving messages with their time of day; nullptr for none.
    void setClock(const WallClock* clock) { clock_ = clock; }
    // The short name the user's own messages show.
    void setOwnShortName(const std::string& name) { ownShortName_ = name; }
    // A decoded packet heard at rssi; true when it was new: a message, a mark, or a node heard.
    bool add(const MeshPacket& packet, int rssi, uint32_t nowMs);
    // The user's message with packet ID id, just given to the radio: it reads "sent".
    void addOwn(uint32_t id, const std::string& text, uint32_t nowMs);
    // The radio could not send the user's message id; true when it was listed.
    bool markNotSent(uint32_t id);
    // The user's messages sent 30 s ago or more and not heard read "not heard"; true when one changed.
    bool expire(uint32_t nowMs);
    const std::deque<MeshMessage>& messages() const { return messages_; }
    size_t unread() const { return unread_; }
    // Texts received from other nodes since startup: one more means a new arrival.
    size_t received() const { return received_; }
    // True when there was something unread.
    bool markAllRead();
    // The node's announced short name, or "!" and the last 4 hex digits of its ID.
    std::string shortNameOf(uint32_t node) const;
    // The nodes heard, the most recently heard first.
    const std::vector<MeshNode>& nodes() const { return nodes_; }
    // Removes every message; the nodes stay.
    void clearMessages();

    // For the microSD card. Messages listed since startup, received and own: one more means one to save.
    size_t added() const { return added_; }
    // The user's messages whose mark changed since the last call, with their new mark.
    std::vector<std::pair<uint32_t, Delivery>> takeDeliveryChanges();
    // Changes whenever a node is heard or renamed.
    uint32_t nodesChanged() const { return nodesChanged_; }
    // A message saved before the restart, appended as read; an own message still "sent" can no longer be heard.
    void restoreMessage(MeshMessage message);
    // A later mark of a saved own message.
    void restoreDelivery(uint32_t id, Delivery delivery);
    // A node saved before the restart, appended after those already known.
    void restoreNode(MeshNode node);

private:
    // The time of day at nowMs for a new message, -1 without a known clock.
    int clockMinute(uint32_t nowMs) const;
    MeshMessage* ownMessage(uint32_t id);
    void setDelivery(MeshMessage& own, Delivery delivery);
    // Appends message, the oldest forgotten past the limit.
    void append(MeshMessage message);
    // The node of id, moved first, or created first when new; the least recently heard is forgotten past the limit.
    MeshNode& heard(uint32_t id);
    bool seen(uint32_t from, uint32_t id);

    std::deque<MeshMessage> messages_;
    std::vector<MeshNode> nodes_;
    // Packets already handled, by sender and ID: relays repeat them.
    std::deque<uint64_t> seen_;
    size_t unread_ = 0;
    uint32_t ownNode_ = 0;
    std::string ownShortName_;
    size_t received_ = 0;
    const WallClock* clock_ = nullptr;
    size_t added_ = 0;
    std::vector<std::pair<uint32_t, Delivery>> deliveryChanges_;
    uint32_t nodesChanged_ = 0;
};
