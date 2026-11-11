#pragma once

#include <stddef.h>

enum class LogLevel { Debug, Info, Warn, Error };

constexpr size_t kLogMaxTagChars = 6;
constexpr size_t kLogMaxMessageChars = 256;
// "[L][TAG] " prefix + message + terminator.
constexpr size_t kLogLineCapacity = 3 + 2 + kLogMaxTagChars + 1 + kLogMaxMessageChars + 1;

// Writes "[L][TAG] message" into out, always NUL-terminated and never beyond outSize.
// Tags are cut to kLogMaxTagChars; messages longer than kLogMaxMessageChars end with "..."
// and are never cut inside a UTF-8 sequence. An empty message yields "[L][TAG]".
void formatLogLine(char* out, size_t outSize, LogLevel level, const char* tag, const char* message);

bool logLevelEnabled(LogLevel level, LogLevel minimum);
