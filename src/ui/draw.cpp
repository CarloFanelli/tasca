#include "ui/draw.h"

#include <M5Cardputer.h>

#include "app.h"

namespace ui {

TextWidth displayWidth() {
    return [](const std::string& text) { return M5Cardputer.Display.textWidth(text.c_str()); };
}

std::string withSender(bool group, const std::string& sender, const std::string& text) {
    return group ? sender + ": " + text : text;
}

const char* clearPromptText(const std::string& chat) {
    if (app::clearPrompt.chat() != chat) {
        return nullptr;
    }
    if (app::clearPrompt.shown()) {
        return "Press Del again to clear";
    }
    return app::clearPrompt.noticeShown() ? "Not cleared on the iPhone" : nullptr;
}

}  // namespace ui
