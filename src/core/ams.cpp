#include "core/ams.h"

#include <stdio.h>

#include "core/utf8.h"

namespace {

constexpr size_t kHeaderSize = 3;
constexpr uint8_t kFlagTruncated = 1 << 0;

template <typename T>
bool set(T& field, const T& value) {
    if (field == value) {
        return false;
    }
    field = value;
    return true;
}

// A decimal such as "-1.5" or "42.25"; parsing stops at the first other character, left in *end.
// Not strtof: it would pull the C library's float parser into the firmware.
float parseDecimal(const char* text, const char** end) {
    const bool negative = *text == '-';
    if (negative) {
        text++;
    }
    float value = 0;
    while (*text >= '0' && *text <= '9') {
        value = value * 10 + static_cast<float>(*text++ - '0');
    }
    if (*text == '.') {
        text++;
        float scale = 0.1f;
        while (*text >= '0' && *text <= '9') {
            value += static_cast<float>(*text++ - '0') * scale;
            scale /= 10;
        }
    }
    if (end != nullptr) {
        *end = text;
    }
    return negative ? -value : value;
}

// "state,rate,elapsed"; empty when nothing plays.
void applyPlaybackInfo(const std::string& value, PlaybackState& state, float& rate, float& elapsed) {
    const char* end = nullptr;
    const float stateValue = parseDecimal(value.c_str(), &end);
    float nextRate = 0;
    float nextElapsed = 0;
    if (*end == ',') {
        nextRate = parseDecimal(end + 1, &end);
        if (*end == ',') {
            nextElapsed = parseDecimal(end + 1, &end);
        }
    }
    state = stateValue >= 0 && stateValue <= 3 ? static_cast<PlaybackState>(static_cast<int>(stateValue))
                                               : PlaybackState::Paused;
    rate = nextRate;
    elapsed = nextElapsed < 0 ? 0 : nextElapsed;
}

}  // namespace

uint32_t decodeAmsCommands(const uint8_t* data, size_t length) {
    uint32_t commands = 0;
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 32) {
            commands |= 1u << data[i];
        }
    }
    return commands;
}

bool decodeAmsUpdate(const uint8_t* data, size_t length, AmsUpdate* update) {
    if (length < kHeaderSize) {
        return false;
    }
    update->entity = data[0];
    update->attribute = data[1];
    update->truncated = (data[2] & kFlagTruncated) != 0;
    const size_t valueLength = length - kHeaderSize;
    update->value.assign(reinterpret_cast<const char*>(data + kHeaderSize),
                         valueLength < kAmsMaxValue ? valueLength : kAmsMaxValue);
    // The BLE client already keeps only kAmsMaxValue bytes: a value of that length may end inside a character.
    // A whole value never does, so trimming it changes nothing.
    if (valueLength >= kAmsMaxValue) {
        trimIncompleteUtf8(update->value);
    }
    return true;
}

std::string formatPlayTime(uint32_t seconds) {
    char text[16];
    if (seconds >= 3600) {
        snprintf(text, sizeof(text), "%u:%02u:%02u", static_cast<unsigned>(seconds / 3600),
                 static_cast<unsigned>(seconds / 60 % 60), static_cast<unsigned>(seconds % 60));
    } else {
        snprintf(text, sizeof(text), "%u:%02u", static_cast<unsigned>(seconds / 60), static_cast<unsigned>(seconds % 60));
    }
    return text;
}

bool NowPlaying::apply(const AmsUpdate& update, uint32_t nowMs) {
    if (update.entity == kAmsEntityPlayer) {
        switch (update.attribute) {
            case kAmsPlayerName:
                return set(player_, update.value);
            case kAmsPlayerPlaybackInfo:
                // Every update carries a new elapsed time: the progress shown changes.
                updateMs_ = nowMs;
                applyPlaybackInfo(update.value, state_, rate_, elapsedAtUpdate_);
                return true;
            case kAmsPlayerVolume:
                // A fraction from 0 to 1.
                return set(volumePercent_, update.value.empty()
                                               ? -1
                                               : static_cast<int>(parseDecimal(update.value.c_str(), nullptr) * 100 + 0.5f));
        }
    } else if (update.entity == kAmsEntityTrack) {
        switch (update.attribute) {
            case kAmsTrackArtist:
                return set(artist_, update.value);
            case kAmsTrackTitle:
                return set(title_, update.value);
            case kAmsTrackDuration: {
                const float seconds = parseDecimal(update.value.c_str(), nullptr);
                return set(durationS_, seconds > 0 ? static_cast<uint32_t>(seconds) : 0u);
            }
        }
    }
    return false;
}

void NowPlaying::clear() {
    *this = NowPlaying();
}

uint32_t NowPlaying::elapsedS(uint32_t nowMs) const {
    float elapsed = elapsedAtUpdate_ + rate_ * static_cast<float>(nowMs - updateMs_) / 1000.0f;
    if (elapsed < 0) {
        elapsed = 0;
    }
    const uint32_t seconds = static_cast<uint32_t>(elapsed);
    return durationS_ > 0 && seconds > durationS_ ? durationS_ : seconds;
}
