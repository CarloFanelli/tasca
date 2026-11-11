#pragma once

#include "core/log_format.h"

// Prints one "[L][TAG] message" line on serial, unless level is below the build-time
// minimum set with -DTASCA_LOG_MIN_LEVEL (0 = Debug, 1 = Info, 2 = Warn, 3 = Error).
void logMessage(LogLevel level, const char* tag, const char* format, ...)
    __attribute__((format(printf, 3, 4)));
