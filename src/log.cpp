#include "log.h"

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef TASCA_LOG_MIN_LEVEL
#define TASCA_LOG_MIN_LEVEL 0
#endif

namespace {
constexpr LogLevel kMinimumLevel = static_cast<LogLevel>(TASCA_LOG_MIN_LEVEL);
}

void logMessage(LogLevel level, const char* tag, const char* format, ...) {
    if (!logLevelEnabled(level, kMinimumLevel)) {
        return;
    }
    // One byte beyond the maximum lets the formatter see that the message overflowed.
    char message[kLogMaxMessageChars + 2];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    char line[kLogLineCapacity];
    formatLogLine(line, sizeof(line), level, tag, message);
    Serial.println(line);
}
