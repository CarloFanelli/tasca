#include "core/text_entry.h"

#include <algorithm>

#include "core/utf8.h"

namespace {

size_t characters(const std::string& text) {
    return static_cast<size_t>(
        std::count_if(text.begin(), text.end(), [](char c) { return !isUtf8Continuation(c); }));
}

}  // namespace

void TextEntry::start(const std::vector<KeyPos>& pressed, const std::string& text) {
    mapper_.reset(pressed);
    echo_.clear(text);
}

bool TextEntry::update(const std::vector<KeyPos>& pressed) {
    std::vector<KeyPos> typing = pressed;
    typing.erase(std::remove_if(typing.begin(), typing.end(),
                                [this](KeyPos key) { return containsKey(reserved_, key); }),
                 typing.end());
    const std::string before = echo_.text();
    for (const KeyboardReport& report : mapper_.update(typing)) {
        echo_.apply(report);
        while (echo_.text().size() > maxBytes_ || characters(echo_.text()) > maxChars_) {
            echo_.removeLastCharacter();
        }
    }
    return echo_.text() != before;
}
