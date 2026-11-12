#include "core/utf8.h"

namespace {

// Bytes of the character a lead byte starts; 1 for ASCII and for invalid lead bytes.
size_t sequenceLength(unsigned char lead) {
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 1;
}

}  // namespace

bool isUtf8Continuation(char c) {
    return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
}

void trimIncompleteUtf8(std::string& text) {
    size_t lead = text.size();
    while (lead > 0 && text.size() - lead < 4) {
        lead--;
        if (!isUtf8Continuation(text[lead])) {
            if (text.size() - lead < sequenceLength(static_cast<unsigned char>(text[lead]))) {
                text.erase(lead);
            }
            return;
        }
    }
}
