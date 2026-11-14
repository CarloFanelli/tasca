#include "ble/ams_client.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <string.h>

#include <atomic>

#include "core/utf8.h"
#include "log.h"

namespace ams {

namespace {

const NimBLEUUID kServiceUuid("89D3502B-0F36-433A-8EF4-C502AD55F8DC");
const NimBLEUUID kRemoteCommandUuid("9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2");
const NimBLEUUID kEntityUpdateUuid("2F7CABCE-808D-411F-9A0C-BB92BA96C102");
const NimBLEUUID kEntityAttributeUuid("C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7");

// Header, then a value cut to the length the Cardputer keeps.
struct UpdatePacket {
    uint16_t length;
    uint8_t data[3 + kAmsMaxValue];
};
// A track change sends a handful of updates at once.
constexpr UBaseType_t kQueueDepth = 16;

QueueHandle_t updates = nullptr;
std::atomic<uint32_t> droppedUpdates{0};
NimBLERemoteCharacteristic* entityUpdate = nullptr;
NimBLERemoteCharacteristic* entityAttribute = nullptr;
NimBLERemoteCharacteristic* remoteCommand = nullptr;
// Every command counts as offered until the iPhone lists them.
constexpr uint32_t kAllCommands = 0xFFFFFFFF;
std::atomic<uint32_t> offeredCommands{kAllCommands};
std::atomic<bool> commandsChanged{false};
NowPlaying current;
// Set when AMS appears or goes: the Music screen then changes.
bool availabilityChanged = false;

// Runs on the BLE host task: only copies the packet for loop().
void onEntityUpdate(NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
    UpdatePacket packet;
    packet.length = static_cast<uint16_t>(length < sizeof(packet.data) ? length : sizeof(packet.data));
    memcpy(packet.data, data, packet.length);
    if (xQueueSend(updates, &packet, 0) != pdTRUE) {
        droppedUpdates++;
    }
}

// Runs on the BLE host task: only keeps the list for loop().
void onRemoteCommand(NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
    offeredCommands = decodeAmsCommands(data, length);
    commandsChanged = true;
}

bool subscribe(const uint8_t* attributes, size_t length) {
    return entityUpdate->writeValue(attributes, length, true);
}

void setUnavailable() {
    if (entityUpdate != nullptr) {
        availabilityChanged = true;
    }
    entityUpdate = nullptr;
    entityAttribute = nullptr;
    remoteCommand = nullptr;
    offeredCommands = kAllCommands;
    current.clear();
    xQueueReset(updates);
}

// The whole value of an attribute the iPhone cut, at most kAmsMaxValue bytes. Blocking: loop() only.
void readWhole(AmsUpdate& update) {
    const uint8_t request[] = {update.entity, update.attribute};
    if (!entityAttribute->writeValue(request, sizeof(request), entityAttribute->canWrite())) {
        logMessage(LogLevel::Warn, "AMS", "read of entity=%u attribute=%u refused (error 0x%x)", update.entity,
                   update.attribute, entityAttribute->getClient()->getLastError());
        return;
    }
    const NimBLEAttValue value = entityAttribute->readValue();
    update.value.assign(reinterpret_cast<const char*>(value.data()),
                        value.length() < kAmsMaxValue ? value.length() : kAmsMaxValue);
    if (value.length() > kAmsMaxValue) {
        trimIncompleteUtf8(update.value);
    }
}

}  // namespace

void begin() {
    updates = xQueueCreate(kQueueDepth, sizeof(UpdatePacket));
}

void setUp(NimBLEClient* client) {
    setUnavailable();
    NimBLERemoteService* service = client->getService(kServiceUuid);
    if (!client->isConnected()) {
        logMessage(LogLevel::Info, "AMS", "setup interrupted: disconnected");
        return;
    }
    if (service == nullptr) {
        logMessage(LogLevel::Info, "AMS", "AMS not found");
        return;
    }
    NimBLERemoteCharacteristic* update = service->getCharacteristic(kEntityUpdateUuid);
    NimBLERemoteCharacteristic* attribute = service->getCharacteristic(kEntityAttributeUuid);
    NimBLERemoteCharacteristic* command = service->getCharacteristic(kRemoteCommandUuid);
    // subscribe() reports success when the CCCD is missing: check it, as for ANCS.
    const NimBLEUUID cccdUuid(static_cast<uint16_t>(0x2902));
    if (update == nullptr || attribute == nullptr || command == nullptr || update->getDescriptor(cccdUuid) == nullptr) {
        logMessage(LogLevel::Warn, "AMS", "AMS not found: characteristics missing");
        return;
    }
    // Subscribed before any attribute is asked for: no update is missed.
    if (!update->subscribe(true, onEntityUpdate, true)) {
        logMessage(LogLevel::Warn, "AMS", "subscription refused (error %d)", client->getLastError());
        return;
    }
    entityUpdate = update;
    entityAttribute = attribute;
    remoteCommand = command;
    // Without the list every key stays usable: iOS refuses what the player does not offer.
    if (command->getDescriptor(cccdUuid) == nullptr || !command->subscribe(true, onRemoteCommand, true)) {
        logMessage(LogLevel::Info, "AMS", "offered commands not followed");
    }
    const uint8_t player[] = {kAmsEntityPlayer, kAmsPlayerName, kAmsPlayerPlaybackInfo, kAmsPlayerVolume};
    const uint8_t track[] = {kAmsEntityTrack, kAmsTrackArtist, kAmsTrackTitle, kAmsTrackDuration};
    if (!subscribe(player, sizeof(player)) || !subscribe(track, sizeof(track))) {
        logMessage(LogLevel::Warn, "AMS", "attributes refused (error 0x%x)", client->getLastError());
        setUnavailable();
        return;
    }
    availabilityChanged = true;
    logMessage(LogLevel::Info, "AMS", "subscribed: player, track");
}

void onServicesChanged(NimBLEClient* client) {
    setUnavailable();
    // Only AMS is forgotten: the ANCS characteristics in use stay valid.
    client->deleteService(kServiceUuid);
    setUp(client);
}

void onDisconnected() {
    setUnavailable();
}

bool poll() {
    bool changed = availabilityChanged || commandsChanged.exchange(false);
    availabilityChanged = false;
    UpdatePacket packet;
    while (xQueueReceive(updates, &packet, 0) == pdTRUE) {
        AmsUpdate update;
        // A packet of a subscription already gone belongs to no Music screen.
        if (entityUpdate == nullptr || !decodeAmsUpdate(packet.data, packet.length, &update)) {
            continue;
        }
        // Lengths only: titles and artists never reach the log.
        logMessage(LogLevel::Debug, "AMS", "update entity=%u attribute=%u bytes=%u%s", update.entity,
                   update.attribute, static_cast<unsigned>(update.value.size()), update.truncated ? " cut" : "");
        if (update.truncated) {
            readWhole(update);
        }
        changed |= current.apply(update, millis());
    }
    const uint32_t dropped = droppedUpdates.exchange(0);
    if (dropped > 0) {
        logMessage(LogLevel::Warn, "AMS", "update queue full: %u updates dropped", static_cast<unsigned>(dropped));
    }
    return changed;
}

bool available() {
    return entityUpdate != nullptr;
}

const NowPlaying& nowPlaying() {
    return current;
}

bool offered(AmsCommand command) {
    return offers(offeredCommands, command);
}

void send(AmsCommand command) {
    const uint8_t id = static_cast<uint8_t>(command);
    if (remoteCommand == nullptr) {
        logMessage(LogLevel::Warn, "AMS", "command %u not sent: AMS not available", id);
        return;
    }
    if (!remoteCommand->writeValue(&id, sizeof(id), remoteCommand->canWrite())) {
        logMessage(LogLevel::Warn, "AMS", "command %u refused (error 0x%x)", id,
                   remoteCommand->getClient()->getLastError());
        return;
    }
    logMessage(LogLevel::Debug, "AMS", "command %u sent", id);
}

}  // namespace ams
