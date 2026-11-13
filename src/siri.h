#pragma once

#include "core/key_events.h"

// Fn+Enter starts Siri on the iPhone; ` right after it closes Siri. loop() only.

void requestSiri();
// A key pressed after Fn+Enter: ` closes Siri and does nothing else (true); any key ends the window.
bool closeSiriOnBack(const KeyEvent& event);
void releaseSiri(const KeyEvent& event);
// Starts and ends the held Voice Command of the current Fn+Enter.
void pollSiri();
// Ends the current Fn+Enter at once, releasing the Voice Command if it is held.
void cancelSiri();
