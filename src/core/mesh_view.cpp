#include "core/mesh_view.h"

namespace {

// The mark after the user's own messages; nullptr for received ones.
const char* markOf(Delivery delivery) {
    switch (delivery) {
        case Delivery::Sent:
            return "sent";
        case Delivery::Heard:
            return "heard";
        case Delivery::NotHeard:
            return "not heard";
        case Delivery::NotSent:
            return "not sent";
        case Delivery::Received:
            break;
    }
    return nullptr;
}

}  // namespace

std::string elapsedLabel(uint32_t elapsedMs) {
    const uint32_t minutes = elapsedMs / 60000;
    if (minutes == 0) {
        return "now";
    }
    if (minutes < 60) {
        return std::to_string(minutes) + "m";
    }
    if (minutes < 24 * 60) {
        return std::to_string(minutes / 60) + "h";
    }
    return std::to_string(minutes / (24 * 60)) + "d";
}

std::vector<ChatLine> meshLines(const MeshStore& store, uint32_t nowMs, int maxWidth, const TextWidth& width) {
    std::vector<ChatLine> lines;
    for (const MeshMessage& m : store.messages()) {
        const std::string time = m.clockMinute >= 0 ? clockLabel(m.clockMinute)
                                 : m.restored       ? "--:--"
                                                    : elapsedLabel(nowMs - m.arrivedMs);
        std::string header = time + " " + displayable(store.shortNameOf(m.from), true);
        if (const char* mark = markOf(m.delivery)) {
            header += std::string(" \xC2\xB7 ") + mark;
        }
        lines.push_back({fitWidth(header, maxWidth, width), true});
        for (std::string& line : wrapText(displayable(m.text, false), maxWidth, width)) {
            lines.push_back({std::move(line), false});
        }
    }
    return lines;
}
