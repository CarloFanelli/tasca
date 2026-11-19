#include "core/mesh_store.h"

#include <stdio.h>

#include <algorithm>

namespace {

constexpr size_t kMaxMessages = 500;
constexpr size_t kMaxNodes = 100;
constexpr size_t kMaxSeen = 64;
constexpr uint32_t kHeardWithinMs = 30000;

}  // namespace

bool MeshStore::add(const MeshPacket& packet, int rssi, uint32_t nowMs) {
    if (packet.to != kMeshBroadcast || seen(packet.from, packet.id)) {
        return false;
    }
    // A node rebroadcast the user's packet: it was heard.
    if (packet.from == ownNode_) {
        MeshMessage* own = ownMessage(packet.id);
        if (own == nullptr || own->delivery == Delivery::Heard) {
            return false;
        }
        setDelivery(*own, Delivery::Heard);
        return true;
    }
    MeshNode& node = heard(packet.from);
    node.rssi = rssi;
    node.lastHeardMs = nowMs;
    node.restored = false;
    if (packet.port == kPortNodeInfo) {
        MeshUser user;
        if (!decodeMeshUser(packet.payload, user)) {
            return false;
        }
        node.longName = user.longName;
        node.shortName = user.shortName;
        return true;
    }
    if (packet.port != kPortText || packet.payload.empty()) {
        return true;
    }
    append({packet.from, packet.id, packet.payload, nowMs, clockMinute(nowMs), Delivery::Received});
    unread_ = std::min(unread_ + 1, messages_.size());
    received_++;
    added_++;
    return true;
}

void MeshStore::addOwn(uint32_t id, const std::string& text, uint32_t nowMs) {
    append({ownNode_, id, text, nowMs, clockMinute(nowMs), Delivery::Sent});
    added_++;
}

void MeshStore::append(MeshMessage message) {
    if (messages_.size() == kMaxMessages) {
        messages_.pop_front();
    }
    messages_.push_back(std::move(message));
}

bool MeshStore::markNotSent(uint32_t id) {
    MeshMessage* own = ownMessage(id);
    if (own == nullptr) {
        return false;
    }
    setDelivery(*own, Delivery::NotSent);
    return true;
}

bool MeshStore::expire(uint32_t nowMs) {
    bool changed = false;
    for (MeshMessage& m : messages_) {
        if (m.delivery == Delivery::Sent && nowMs - m.arrivedMs >= kHeardWithinMs) {
            setDelivery(m, Delivery::NotHeard);
            changed = true;
        }
    }
    return changed;
}

void MeshStore::setDelivery(MeshMessage& own, Delivery delivery) {
    own.delivery = delivery;
    deliveryChanges_.emplace_back(own.id, delivery);
}

std::vector<std::pair<uint32_t, Delivery>> MeshStore::takeDeliveryChanges() {
    std::vector<std::pair<uint32_t, Delivery>> changes;
    changes.swap(deliveryChanges_);
    return changes;
}

void MeshStore::clearMessages() {
    messages_.clear();
    unread_ = 0;
}

void MeshStore::restoreMessage(MeshMessage message) {
    seen(message.from, message.id);
    message.restored = true;
    if (message.delivery == Delivery::Sent) {
        message.delivery = Delivery::NotHeard;
    }
    append(std::move(message));
}

void MeshStore::restoreDelivery(uint32_t id, Delivery delivery) {
    if (MeshMessage* own = ownMessage(id)) {
        own->delivery = delivery;
    }
}

void MeshStore::restoreNode(MeshNode node) {
    const bool known =
        std::any_of(nodes_.begin(), nodes_.end(), [&node](const MeshNode& n) { return n.id == node.id; });
    if (known || nodes_.size() == kMaxNodes) {
        return;
    }
    node.restored = true;
    nodes_.push_back(std::move(node));
}

int MeshStore::clockMinute(uint32_t nowMs) const {
    return clock_ == nullptr ? -1 : clock_->minuteOfDay(nowMs);
}

MeshMessage* MeshStore::ownMessage(uint32_t id) {
    for (auto it = messages_.rbegin(); it != messages_.rend(); ++it) {
        if (it->from == ownNode_ && it->id == id) {
            return &*it;
        }
    }
    return nullptr;
}

bool MeshStore::markAllRead() {
    const bool had = unread_ != 0;
    unread_ = 0;
    return had;
}

std::string MeshStore::shortNameOf(uint32_t node) const {
    if (node == ownNode_ && !ownShortName_.empty()) {
        return ownShortName_;
    }
    const auto it = std::find_if(nodes_.begin(), nodes_.end(), [node](const MeshNode& n) { return n.id == node; });
    if (it != nodes_.end() && !it->shortName.empty()) {
        return it->shortName;
    }
    char name[8];
    snprintf(name, sizeof(name), "!%04x", static_cast<unsigned>(node & 0xFFFF));
    return name;
}

MeshNode& MeshStore::heard(uint32_t id) {
    nodesChanged_++;
    const auto it = std::find_if(nodes_.begin(), nodes_.end(), [id](const MeshNode& n) { return n.id == id; });
    if (it != nodes_.end()) {
        std::rotate(nodes_.begin(), it, it + 1);
        return nodes_.front();
    }
    if (nodes_.size() == kMaxNodes) {
        nodes_.pop_back();
    }
    MeshNode node;
    node.id = id;
    nodes_.insert(nodes_.begin(), node);
    return nodes_.front();
}

bool MeshStore::seen(uint32_t from, uint32_t id) {
    const uint64_t key = static_cast<uint64_t>(from) << 32 | id;
    if (std::find(seen_.begin(), seen_.end(), key) != seen_.end()) {
        return true;
    }
    if (seen_.size() == kMaxSeen) {
        seen_.pop_front();
    }
    seen_.push_back(key);
    return false;
}
