#include "core/ancs_request.h"

#include <initializer_list>

#include "core/utf8.h"

namespace {

constexpr uint8_t kCommandGetNotificationAttributes = 0;
constexpr uint8_t kEventAdded = 0;
constexpr uint8_t kEventModified = 1;
constexpr uint8_t kEventRemoved = 2;
constexpr uint8_t kEventFlagPreExisting = 1 << 2;
constexpr size_t kEventSize = 8;
constexpr size_t kResponseHeaderSize = 5;

constexpr uint8_t kAttributeApp = 0;
constexpr uint8_t kAttributeTitle = 1;
constexpr uint8_t kAttributeSubtitle = 2;
constexpr uint8_t kAttributeMessage = 3;
constexpr uint8_t kAttributeDate = 5;
constexpr uint16_t kMaxTitle = 64;
constexpr uint16_t kMaxMessage = 512;
// Neither has a requested maximum: bounds well above a bundle id and the 15-character date.
constexpr uint16_t kMaxApp = 128;
constexpr uint16_t kMaxDate = 32;

uint16_t maxLengthOf(uint8_t attributeId) {
    switch (attributeId) {
        case kAttributeApp:      return kMaxApp;
        case kAttributeTitle:    return kMaxTitle;
        case kAttributeSubtitle: return kMaxTitle;
        case kAttributeMessage:  return kMaxMessage;
        default:                 return kMaxDate;
    }
}

void putUint32(uint8_t* out, uint32_t value) {
    for (int i = 0; i < 4; i++) {
        out[i] = static_cast<uint8_t>(value >> (8 * i));
    }
}

uint32_t getUint32(const uint8_t* in) {
    return in[0] | (in[1] << 8) | (in[2] << 16) | (static_cast<uint32_t>(in[3]) << 24);
}

}  // namespace

size_t encodeGetNotificationAttributes(uint8_t* out, uint32_t uid) {
    out[0] = kCommandGetNotificationAttributes;
    putUint32(out + 1, uid);
    const uint8_t attributes[] = {
        kAttributeApp,
        kAttributeTitle, kMaxTitle & 0xFF, kMaxTitle >> 8,
        kAttributeSubtitle, kMaxTitle & 0xFF, kMaxTitle >> 8,
        kAttributeMessage, kMaxMessage & 0xFF, kMaxMessage >> 8,
        kAttributeDate,
    };
    for (size_t i = 0; i < sizeof(attributes); i++) {
        out[5 + i] = attributes[i];
    }
    return kAncsRequestSize;
}

bool decodeAncsEvent(const uint8_t* data, size_t length, AncsEvent* event) {
    if (length < kEventSize) {
        return false;
    }
    switch (data[0]) {
        case kEventAdded:    event->kind = AncsEvent::Kind::Added; break;
        case kEventModified: event->kind = AncsEvent::Kind::Modified; break;
        case kEventRemoved:  event->kind = AncsEvent::Kind::Removed; break;
        default:             return false;
    }
    event->preExisting = (data[1] & kEventFlagPreExisting) != 0;
    event->category = data[2];
    event->uid = getUint32(data + 4);
    return true;
}

void AncsResponseDecoder::start(uint32_t uid) {
    *this = AncsResponseDecoder();
    active_ = true;
    uid_ = uid;
}

void AncsResponseDecoder::clear() {
    *this = AncsResponseDecoder();
}

AncsResponseDecoder::Result AncsResponseDecoder::fail() {
    clear();
    return Result::Malformed;
}

std::string* AncsResponseDecoder::valueOf(uint8_t attributeId) {
    switch (attributeId) {
        case kAttributeApp:      return &attributes_.app;
        case kAttributeTitle:    return &attributes_.title;
        case kAttributeSubtitle: return &attributes_.subtitle;
        case kAttributeMessage:  return &attributes_.message;
        case kAttributeDate:     return &attributes_.date;
    }
    return nullptr;
}

AncsResponseDecoder::Result AncsResponseDecoder::feed(const uint8_t* data, size_t length) {
    if (!active_) {
        return Result::Ignored;
    }
    size_t i = 0;
    if (!headerSeen_) {
        // Only a packet opening the response to the request in flight counts: anything else is a
        // stale fragment of an earlier, timed-out response.
        if (length < kResponseHeaderSize || data[0] != kCommandGetNotificationAttributes ||
            getUint32(data + 1) != uid_) {
            return Result::Ignored;
        }
        headerSeen_ = true;
        i = kResponseHeaderSize;
    }
    while (i < length) {
        switch (field_) {
            case Field::AttributeId:
                if (tuplesDone_ == kAncsRequestedAttributes || valueOf(data[i]) == nullptr) {
                    return fail();
                }
                attributeId_ = data[i++];
                field_ = Field::LengthLow;
                break;
            case Field::LengthLow:
                remaining_ = data[i++];
                field_ = Field::LengthHigh;
                break;
            case Field::LengthHigh:
                remaining_ |= static_cast<uint16_t>(data[i++] << 8);
                if (remaining_ > maxLengthOf(attributeId_)) {
                    return fail();
                }
                field_ = Field::Value;
                break;
            case Field::Value: {
                const size_t take = remaining_ < length - i ? remaining_ : length - i;
                valueOf(attributeId_)->append(reinterpret_cast<const char*>(data + i), take);
                remaining_ -= static_cast<uint16_t>(take);
                i += take;
                break;
            }
        }
        if (field_ == Field::Value && remaining_ == 0) {
            field_ = Field::AttributeId;
            ++tuplesDone_;
        }
    }
    if (tuplesDone_ == kAncsRequestedAttributes) {
        active_ = false;
        for (std::string* text : {&attributes_.title, &attributes_.subtitle, &attributes_.message}) {
            trimIncompleteUtf8(*text);
        }
        return Result::Complete;
    }
    return Result::Progress;
}
