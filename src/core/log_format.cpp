#include "core/log_format.h"

#include <string.h>

#include "core/utf8.h"

namespace {

char levelLetter(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return 'D';
        case LogLevel::Info:  return 'I';
        case LogLevel::Warn:  return 'W';
        case LogLevel::Error: return 'E';
    }
    return '?';
}

class LineWriter {
public:
    LineWriter(char* out, size_t size) : out_(out), size_(size) {}

    void put(char c) {
        if (length_ + 1 < size_) {
            out_[length_++] = c;
        }
    }

    void put(const char* text, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            put(text[i]);
        }
    }

    void terminate() {
        if (size_ > 0) {
            out_[length_] = '\0';
        }
    }

private:
    char* out_;
    size_t size_;
    size_t length_ = 0;
};

}  // namespace

void formatLogLine(char* out, size_t outSize, LogLevel level, const char* tag, const char* message) {
    LineWriter line(out, outSize);
    const char* safeTag = tag ? tag : "";
    const char* safeMessage = message ? message : "";

    line.put('[');
    line.put(levelLetter(level));
    line.put(']');
    line.put('[');
    line.put(safeTag, strnlen(safeTag, kLogMaxTagChars));
    line.put(']');

    size_t messageLength = strnlen(safeMessage, kLogMaxMessageChars + 1);
    if (messageLength > 0) {
        line.put(' ');
        if (messageLength <= kLogMaxMessageChars) {
            line.put(safeMessage, messageLength);
        } else {
            size_t keep = kLogMaxMessageChars - 3;
            while (keep > 0 && isUtf8Continuation(safeMessage[keep])) {
                --keep;
            }
            line.put(safeMessage, keep);
            line.put("...", 3);
        }
    }
    line.terminate();
}

bool logLevelEnabled(LogLevel level, LogLevel minimum) {
    return static_cast<int>(level) >= static_cast<int>(minimum);
}
