#include <unity.h>

#include <string>
#include <vector>

#include "core/ams.h"

void setUp() {}
void tearDown() {}

static AmsUpdate update(uint8_t entity, uint8_t attribute, const std::string& value) {
    return {entity, attribute, false, value};
}

static void test_decode_reads_header_and_value() {
    const uint8_t packet[] = {2, 2, 1, 'S', 'o', 'n', 'g'};
    AmsUpdate u;
    TEST_ASSERT_TRUE(decodeAmsUpdate(packet, sizeof(packet), &u));
    TEST_ASSERT_EQUAL(kAmsEntityTrack, u.entity);
    TEST_ASSERT_EQUAL(kAmsTrackTitle, u.attribute);
    TEST_ASSERT_TRUE(u.truncated);
    TEST_ASSERT_EQUAL_STRING("Song", u.value.c_str());
}

static void test_decode_rejects_a_packet_shorter_than_its_header() {
    const uint8_t packet[] = {2, 2};
    AmsUpdate u;
    TEST_ASSERT_FALSE(decodeAmsUpdate(packet, sizeof(packet), &u));
}

static void test_decode_cuts_the_value_at_128_bytes_on_a_whole_character() {
    std::vector<uint8_t> packet = {2, 2, 0};
    packet.insert(packet.end(), 127, 'a');
    packet.push_back(0xC3);  // à, its second byte past the limit
    packet.push_back(0xA0);
    AmsUpdate u;
    TEST_ASSERT_TRUE(decodeAmsUpdate(packet.data(), packet.size(), &u));
    TEST_ASSERT_EQUAL(127, u.value.size());
}

static void test_track_and_player_fields_are_kept() {
    NowPlaying np;
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityTrack, kAmsTrackTitle, "Song"), 0));
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityTrack, kAmsTrackArtist, "Band"), 0));
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityPlayer, kAmsPlayerName, "Spotify"), 0));
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityTrack, kAmsTrackDuration, "215.5"), 0));
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityPlayer, kAmsPlayerVolume, "0.5"), 0));
    TEST_ASSERT_EQUAL_STRING("Song", np.title().c_str());
    TEST_ASSERT_EQUAL_STRING("Band", np.artist().c_str());
    TEST_ASSERT_EQUAL_STRING("Spotify", np.player().c_str());
    TEST_ASSERT_EQUAL(215, np.durationS());
    TEST_ASSERT_EQUAL(50, np.volumePercent());
}

static void test_same_value_again_changes_nothing() {
    NowPlaying np;
    np.apply(update(kAmsEntityTrack, kAmsTrackTitle, "Song"), 0);
    TEST_ASSERT_FALSE(np.apply(update(kAmsEntityTrack, kAmsTrackTitle, "Song"), 0));
}

static void test_unknown_attribute_changes_nothing() {
    NowPlaying np;
    TEST_ASSERT_FALSE(np.apply(update(1, 0, "3"), 0));
    TEST_ASSERT_FALSE(np.apply(update(kAmsEntityTrack, 1, "Album"), 0));
}

static void test_playback_info_sets_state_and_elapsed() {
    NowPlaying np;
    TEST_ASSERT_TRUE(np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "1,1.0,42.5"), 1000));
    TEST_ASSERT_EQUAL(static_cast<int>(PlaybackState::Playing), static_cast<int>(np.state()));
    TEST_ASSERT_EQUAL(42, np.elapsedS(1000));
}

static void test_elapsed_advances_at_the_playback_rate() {
    NowPlaying np;
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "1,1.0,10"), 5000);
    TEST_ASSERT_EQUAL(13, np.elapsedS(8000));
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "3,2.0,10"), 5000);
    TEST_ASSERT_EQUAL(16, np.elapsedS(8000));
}

static void test_paused_elapsed_stays() {
    NowPlaying np;
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "0,0.0,30"), 0);
    TEST_ASSERT_EQUAL(static_cast<int>(PlaybackState::Paused), static_cast<int>(np.state()));
    TEST_ASSERT_EQUAL(30, np.elapsedS(60000));
}

static void test_elapsed_stops_at_the_duration() {
    NowPlaying np;
    np.apply(update(kAmsEntityTrack, kAmsTrackDuration, "100"), 0);
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "1,1.0,95"), 0);
    TEST_ASSERT_EQUAL(100, np.elapsedS(20000));
}

static void test_elapsed_survives_clock_wrap() {
    NowPlaying np;
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, "1,1.0,0"), 0xFFFFF000u);
    TEST_ASSERT_EQUAL(5, np.elapsedS(0xFFFFF000u + 5000));
}

static void test_empty_values_mean_nothing_playing() {
    NowPlaying np;
    np.apply(update(kAmsEntityTrack, kAmsTrackTitle, "Song"), 0);
    np.apply(update(kAmsEntityTrack, kAmsTrackDuration, "200"), 0);
    np.apply(update(kAmsEntityPlayer, kAmsPlayerVolume, "0.3"), 0);
    np.apply(update(kAmsEntityTrack, kAmsTrackTitle, ""), 0);
    np.apply(update(kAmsEntityTrack, kAmsTrackDuration, ""), 0);
    np.apply(update(kAmsEntityPlayer, kAmsPlayerVolume, ""), 0);
    np.apply(update(kAmsEntityPlayer, kAmsPlayerPlaybackInfo, ""), 0);
    TEST_ASSERT_TRUE(np.title().empty());
    TEST_ASSERT_EQUAL(0, np.durationS());
    TEST_ASSERT_EQUAL(-1, np.volumePercent());
    TEST_ASSERT_EQUAL(0, np.elapsedS(1000));
}

static void test_clear_forgets_everything() {
    NowPlaying np;
    np.apply(update(kAmsEntityTrack, kAmsTrackTitle, "Song"), 0);
    np.clear();
    TEST_ASSERT_TRUE(np.title().empty());
    TEST_ASSERT_EQUAL(-1, np.volumePercent());
}

static void test_whole_and_negative_numbers() {
    NowPlaying np;
    np.apply(update(kAmsEntityPlayer, kAmsPlayerVolume, "1"), 0);
    np.apply(update(kAmsEntityTrack, kAmsTrackDuration, "-1"), 0);
    TEST_ASSERT_EQUAL(100, np.volumePercent());
    TEST_ASSERT_EQUAL(0, np.durationS());
}

static void test_commands_list_becomes_a_set() {
    const uint8_t packet[] = {2, 3, 4, 40};
    const uint32_t commands = decodeAmsCommands(packet, sizeof(packet));
    TEST_ASSERT_TRUE(offers(commands, AmsCommand::TogglePlayPause));
    TEST_ASSERT_TRUE(offers(commands, AmsCommand::NextTrack));
    TEST_ASSERT_TRUE(offers(commands, AmsCommand::PreviousTrack));
    TEST_ASSERT_FALSE(offers(commands, AmsCommand::VolumeUp));
    TEST_ASSERT_EQUAL(0, decodeAmsCommands(packet, 0));
}

static void test_play_time_format() {
    TEST_ASSERT_EQUAL_STRING("0:00", formatPlayTime(0).c_str());
    TEST_ASSERT_EQUAL_STRING("3:05", formatPlayTime(185).c_str());
    TEST_ASSERT_EQUAL_STRING("1:00:01", formatPlayTime(3601).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_decode_reads_header_and_value);
    RUN_TEST(test_decode_rejects_a_packet_shorter_than_its_header);
    RUN_TEST(test_decode_cuts_the_value_at_128_bytes_on_a_whole_character);
    RUN_TEST(test_track_and_player_fields_are_kept);
    RUN_TEST(test_same_value_again_changes_nothing);
    RUN_TEST(test_unknown_attribute_changes_nothing);
    RUN_TEST(test_playback_info_sets_state_and_elapsed);
    RUN_TEST(test_elapsed_advances_at_the_playback_rate);
    RUN_TEST(test_paused_elapsed_stays);
    RUN_TEST(test_elapsed_stops_at_the_duration);
    RUN_TEST(test_elapsed_survives_clock_wrap);
    RUN_TEST(test_empty_values_mean_nothing_playing);
    RUN_TEST(test_clear_forgets_everything);
    RUN_TEST(test_whole_and_negative_numbers);
    RUN_TEST(test_commands_list_becomes_a_set);
    RUN_TEST(test_play_time_format);
    return UNITY_END();
}
