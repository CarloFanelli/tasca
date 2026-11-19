#include "screens/reading_screen.h"

#include <Arduino.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "log.h"
#include "screens/settings_screen.h"
#include "siri.h"

void ReadingScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    if (app::pairingResetCombo.update(pressed, millis()) && ble::resetPairing()) {
        app::restartAfterPairingReset();
    }
    for (const KeyEvent& event : events) {
        char text[kKeyTextCapacity];
        formatKeyEvent(text, sizeof(text), event);
        logMessage(LogLevel::Debug, "KEY", "%s", text);
        releaseSiri(event);
        if (event.kind != KeyEventKind::Down) {
            continue;
        }
        // The Mesh screen and Node list stay on top when the iPhone is lost: a key there silences the alert too.
        app::silenceOutOfRangeAlert();
        // A key that turns the screen on does nothing else.
        if (app::wakeScreen()) {
            continue;
        }
        // Any key but a plain Del removes the clear prompt, then does what it does.
        if (!(app::isKey(event.key, app::kDelKey) && event.modifiers == 0)) {
            app::screenDirty |= app::clearPrompt.cancel();
        }
        if (closeSiriOnBack(event)) {
            continue;
        }
        // Fn+Enter starts Siri; the screen stays as it is, since the request is spoken.
        if (app::isKey(event.key, app::kOpenKey) && (event.modifiers & kModFn) != 0) {
            app::screenDirty |= settingsScreen.cancelPrompt();
            requestSiri();
            continue;
        }
        // Fn is otherwise ignored: Fn+; and Fn+. are the arrow legends of the same keys, Fn+, and Fn+/ likewise.
        onKeyDown(event);
    }
}

void ReadingScreen::update(uint32_t nowMs) {
    if (ancs::takeNotCleared()) {
        app::clearPrompt.showNotice(nowMs);
        app::screenDirty = true;
    }
    if (app::clearPrompt.expire(nowMs)) {
        app::screenDirty = true;
    }
}
