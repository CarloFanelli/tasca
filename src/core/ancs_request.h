#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// Get Notification Attributes for AppIdentifier, Title (64), Subtitle (64), Message (512), Date.
constexpr size_t kAncsRequestSize = 16;
constexpr uint8_t kAncsRequestedAttributes = 5;

// Writes the Control Point command for notification `uid` into out; returns its size.
size_t encodeGetNotificationAttributes(uint8_t* out, uint32_t uid);

// Perform Notification Action: answers (positive) or declines (negative) a call.
constexpr size_t kAncsActionSize = 6;

// Writes the Control Point command for the positive or negative action of notification `uid` into out;
// returns its size.
size_t encodePerformNotificationAction(uint8_t* out, uint32_t uid, bool positive);

// One Notification Source event.
struct AncsEvent {
    enum class Kind { Added, Modified, Removed } kind;
    bool preExisting;  // already on the iPhone when the subscription started
    bool positiveAction;  // the iPhone can perform its positive action (answer a call)
    bool negativeAction;  // the iPhone can perform its negative action (decline a call)
    uint8_t category;  // CategoryID of the ANCS specification, e.g. 4 Social
    uint32_t uid;
};

// False for a packet too short or with an unknown event id.
bool decodeAncsEvent(const uint8_t* data, size_t length, AncsEvent* event);

// The attributes of one notification, as UTF-8 text.
struct AncsAttributes {
    std::string app;
    std::string title;
    std::string subtitle;
    std::string message;
    std::string date;  // yyyyMMdd'T'HHmmSS, the iPhone's local time
};

// Decodes one Get Notification Attributes response over its Data Source fragments, until the
// requested tuples have all arrived.
class AncsResponseDecoder {
public:
    // Malformed: an attribute that was not requested, a value over its requested maximum, or
    // bytes after the last attribute. The decoder is then inactive.
    enum class Result { Ignored, Progress, Complete, Malformed };

    void start(uint32_t uid);
    void clear();
    bool active() const { return active_; }
    Result feed(const uint8_t* data, size_t length);

    // Complete once feed() returned Complete; text cut by a length limit holds whole characters only.
    const AncsAttributes& attributes() const { return attributes_; }

private:
    enum class Field { AttributeId, LengthLow, LengthHigh, Value };

    Result fail();
    std::string* valueOf(uint8_t attributeId);

    bool active_ = false;
    bool headerSeen_ = false;
    uint32_t uid_ = 0;
    Field field_ = Field::AttributeId;
    uint8_t attributeId_ = 0;
    uint16_t remaining_ = 0;
    uint8_t tuplesDone_ = 0;
    AncsAttributes attributes_;
};
