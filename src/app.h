#pragma once

#include <stdint.h>

#include <vector>

#include "core/clear_prompt.h"
#include "core/combo_hold.h"
#include "core/inbox.h"
#include "core/key_events.h"
#include "core/keymap.h"
#include "core/reading_nav.h"
#include "core/screen_stack.h"
#include "core/screen_timer.h"
#include "core/settings.h"

// State and services shared by the screens. loop() only.
namespace app {

constexpr KeyPos kUpKey{2, 11};       // ;
constexpr KeyPos kDownKey{3, 11};     // .
constexpr KeyPos kOpenKey{2, 13};     // Enter
constexpr KeyPos kBackKey{0, 0};      // `
constexpr KeyPos kSettingsKey{2, 3};  // s, with Opt
constexpr KeyPos kPrevKey{3, 10};     // ,
constexpr KeyPos kNextKey{3, 12};     // /
constexpr KeyPos kDelKey{0, 13};
constexpr KeyPos kFnKey{2, 0};

extern bool displayReady;
// The iPhone accepts keys: Keyboard mode sends them, Siri needs them.
extern bool keyboardReady;
// Fn + Del held for 3 s: forget the bonded iPhone.
extern ComboHold pairingResetCombo;

// Reading mode: Del twice clears a Chat on the iPhone.
extern ClearPrompt clearPrompt;
// Reading mode: the Inbox built from the stored Messages and the user's place in it.
extern std::vector<ChatSummary> chats;
extern ReadingNav nav;
extern Settings settings;
// Battery percent, negative when unknown.
extern int32_t batteryLevel;
// Redraw only what changed, once per loop.
extern bool statusDirty;
extern bool screenDirty;
// Keyboard mode: only the area above the mode line changed.
extern bool contextDirty;
// While the screen is off nothing is drawn: the dirty flags stay set until it is on again.
extern ScreenTimer screenTimer;
// The Inbox at the bottom, then the screens opened over it.
extern ScreenStack screens;

bool isKey(KeyPos a, KeyPos b);
bool canDraw();
// A key, G0 or live Message: the display timeout restarts. True when this turned the screen on.
bool wakeScreen();
void applyBrightness();
void rebuildInbox();
// The stored Messages changed: rebuild the Inbox, and the open Chat screen unless all its Messages are gone.
void onMessagesChanged();
bool sendKeyboardReport(const KeyboardReport& report);
// Short beeps, each heard apart; played by pollBeeps.
void queueBeeps(size_t count);
void pollBeeps();
void restartAfterPairingReset();
// Del on chat in Reading mode: shows the prompt, or clears chat on the iPhone when it is shown.
void delOnChat(const std::string& chat);

}  // namespace app
