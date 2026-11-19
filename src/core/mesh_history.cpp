#include "core/mesh_history.h"

#include <stdlib.h>

#include <vector>

namespace {

constexpr char kSeparator = '\t';

std::string escaped(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '\t': out += "\\t"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string unescaped(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] != '\\' || i + 1 == text.size()) {
            out += text[i];
            continue;
        }
        switch (text[++i]) {
            case 't': out += '\t'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            default: out += text[i]; break;
        }
    }
    return out;
}

std::string hex(uint32_t value) {
    static const char kDigits[] = "0123456789abcdef";
    std::string out(8, '0');
    for (int i = 7; i >= 0; i--, value >>= 4) {
        out[i] = kDigits[value & 0xF];
    }
    return out;
}

std::vector<std::string> fields(const std::string& line) {
    std::vector<std::string> out(1);
    for (const char c : line) {
        if (c == kSeparator) {
            out.emplace_back();
        } else {
            out.back() += c;
        }
    }
    return out;
}

bool parseHex32(const std::string& text, uint32_t& value) {
    if (text.empty() || text.size() > 8) {
        return false;
    }
    char* end = nullptr;
    value = static_cast<uint32_t>(strtoul(text.c_str(), &end, 16));
    return *end == '\0';
}

bool parseInt(const std::string& text, long min, long max, long& value) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    value = strtol(text.c_str(), &end, 10);
    return *end == '\0' && value >= min && value <= max;
}

bool parseDelivery(const std::string& text, Delivery& delivery) {
    long value = 0;
    if (!parseInt(text, 0, static_cast<long>(Delivery::NotSent), value)) {
        return false;
    }
    delivery = static_cast<Delivery>(value);
    return true;
}

}  // namespace

std::string historyMessageLine(const MeshMessage& message) {
    return "M\t" + hex(message.from) + kSeparator + hex(message.id) + kSeparator +
           std::to_string(message.clockMinute) + kSeparator + std::to_string(static_cast<int>(message.delivery)) +
           kSeparator + escaped(message.text) + '\n';
}

std::string historyDeliveryLine(uint32_t id, Delivery delivery) {
    return "D\t" + hex(id) + kSeparator + std::to_string(static_cast<int>(delivery)) + '\n';
}

std::string historyNodeLine(const MeshNode& node) {
    return "N\t" + hex(node.id) + kSeparator + std::to_string(node.rssi) + kSeparator + escaped(node.shortName) +
           kSeparator + escaped(node.longName) + '\n';
}

bool loadHistoryLine(const std::string& line, MeshStore& store) {
    const std::vector<std::string> f = fields(line);
    if (f[0] == "M" && f.size() == 6) {
        MeshMessage message;
        long minute = 0;
        if (!parseHex32(f[1], message.from) || !parseHex32(f[2], message.id) ||
            !parseInt(f[3], -1, 24 * 60 - 1, minute) || !parseDelivery(f[4], message.delivery)) {
            return false;
        }
        message.clockMinute = static_cast<int>(minute);
        message.text = unescaped(f[5]);
        store.restoreMessage(std::move(message));
        return true;
    }
    if (f[0] == "D" && f.size() == 3) {
        uint32_t id = 0;
        Delivery delivery = Delivery::Received;
        if (!parseHex32(f[1], id) || !parseDelivery(f[2], delivery)) {
            return false;
        }
        store.restoreDelivery(id, delivery);
        return true;
    }
    if (f[0] == "N" && f.size() == 5) {
        MeshNode node;
        long rssi = 0;
        if (!parseHex32(f[1], node.id) || !parseInt(f[2], -200, 50, rssi)) {
            return false;
        }
        node.rssi = static_cast<int>(rssi);
        node.shortName = unescaped(f[3]);
        node.longName = unescaped(f[4]);
        store.restoreNode(std::move(node));
        return true;
    }
    return false;
}
