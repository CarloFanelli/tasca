#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// Apple Media Service: the entities and attributes the Music screen shows.
constexpr uint8_t kAmsEntityPlayer = 0;
constexpr uint8_t kAmsEntityTrack = 2;
constexpr uint8_t kAmsPlayerName = 0;
constexpr uint8_t kAmsPlayerPlaybackInfo = 1;
constexpr uint8_t kAmsPlayerVolume = 2;
constexpr uint8_t kAmsTrackArtist = 0;
constexpr uint8_t kAmsTrackTitle = 2;
constexpr uint8_t kAmsTrackDuration = 3;
// Longest value kept, in bytes: a title or an artist past it is cut.
constexpr size_t kAmsMaxValue = 128;

// Remote Command IDs the Music screen sends.
enum class AmsCommand : uint8_t {
    Play = 0,
    TogglePlayPause = 2,
    NextTrack = 3,
    PreviousTrack = 4,
    VolumeUp = 5,
    VolumeDown = 6,
};

// The commands a Remote Command notification lists, one bit per command ID.
uint32_t decodeAmsCommands(const uint8_t* data, size_t length);
inline bool offers(uint32_t commands, AmsCommand command) {
    return (commands & (1u << static_cast<uint8_t>(command))) != 0;
}

// One Entity Update notification.
struct AmsUpdate {
    uint8_t entity;
    uint8_t attribute;
    bool truncated;  // the iPhone cut the value: the whole one is read through Entity Attribute
    std::string value;
};

// False for a packet shorter than its header. The value keeps at most kAmsMaxValue bytes, whole characters only.
bool decodeAmsUpdate(const uint8_t* data, size_t length, AmsUpdate* update);

// Seconds as m:ss, or h:mm:ss from one hour.
std::string formatPlayTime(uint32_t seconds);

enum class PlaybackState : uint8_t { Paused, Playing, Rewinding, FastForwarding };

// What the iPhone is playing, from its Entity Updates.
class NowPlaying {
public:
    // nowMs: a millisecond clock that may wrap around. True when something shown changed.
    bool apply(const AmsUpdate& update, uint32_t nowMs);
    void clear();

    const std::string& player() const { return player_; }
    const std::string& title() const { return title_; }
    const std::string& artist() const { return artist_; }
    PlaybackState state() const { return state_; }
    // 0 to 100; negative when unknown.
    int volumePercent() const { return volumePercent_; }
    // 0 when unknown, e.g. live radio.
    uint32_t durationS() const { return durationS_; }
    // The elapsed time of the last update, advanced at its playback rate; never past a known duration.
    uint32_t elapsedS(uint32_t nowMs) const;

private:
    std::string player_;
    std::string title_;
    std::string artist_;
    PlaybackState state_ = PlaybackState::Paused;
    float rate_ = 0;
    float elapsedAtUpdate_ = 0;
    uint32_t updateMs_ = 0;
    int volumePercent_ = -1;
    uint32_t durationS_ = 0;
};
