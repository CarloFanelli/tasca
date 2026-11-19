#include "ble/cts_client.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "log.h"

namespace cts {

namespace {

const NimBLEUUID kServiceUuid(static_cast<uint16_t>(0x1805));
const NimBLEUUID kCurrentTimeUuid(static_cast<uint16_t>(0x2A2B));

WallClock wallClock;

}  // namespace

void setUp(NimBLEClient* client) {
    NimBLERemoteService* service = client->getService(kServiceUuid);
    NimBLERemoteCharacteristic* currentTime =
        service == nullptr ? nullptr : service->getCharacteristic(kCurrentTimeUuid);
    if (currentTime == nullptr || !currentTime->canRead()) {
        logMessage(LogLevel::Info, "BLE", "iPhone time not offered");
        return;
    }
    const NimBLEAttValue value = currentTime->readValue();
    if (!wallClock.setFromCurrentTime(value.data(), value.length(), millis())) {
        logMessage(LogLevel::Warn, "BLE", "iPhone time not read (%u bytes)", static_cast<unsigned>(value.length()));
        return;
    }
    const int minute = wallClock.minuteOfDay(millis());
    logMessage(LogLevel::Info, "BLE", "iPhone time %s", clockLabel(minute).c_str());
}

const WallClock& clock() {
    return wallClock;
}

}  // namespace cts
