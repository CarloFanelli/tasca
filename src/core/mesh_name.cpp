#include "core/mesh_name.h"

#include <stdio.h>

#include "core/utf8.h"

namespace {

constexpr size_t kShortNameBytes = 4;
constexpr size_t kShortNameChars = 4;

// The longest start of text within maxBytes and maxChars, in whole characters.
std::string firstOf(const std::string& text, size_t maxBytes, size_t maxChars) {
    size_t end = 0;
    size_t chars = 0;
    while (end < text.size() && chars < maxChars) {
        size_t next = end + 1;
        while (next < text.size() && isUtf8Continuation(text[next])) {
            next++;
        }
        if (next > maxBytes) {
            break;
        }
        end = next;
        chars++;
    }
    return text.substr(0, end);
}

}  // namespace

std::string meshLongName(const std::string& chosen, uint32_t node) {
    if (chosen.empty()) {
        char name[16];
        snprintf(name, sizeof(name), "Tasca%04x", static_cast<unsigned>(node & 0xFFFF));
        return name;
    }
    return firstOf(chosen, kMeshNameBytes, kMeshNameBytes);
}

std::string meshShortName(const std::string& longName) {
    return firstOf(longName, kShortNameBytes, kShortNameChars);
}
