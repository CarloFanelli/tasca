#include "ble/ble_manager.h"

#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <atomic>

#include "ble/ancs_client.h"
#include "ble/hid_keyboard.h"
#include "log.h"

namespace ble {

namespace {

constexpr const char* kDeviceName = "Tasca";
constexpr uint16_t kKeyboardAppearance = 0x03C1;
constexpr uint8_t kAdTypeSolicitation128 = 0x15;
constexpr UBaseType_t kEventQueueDepth = 16;

const NimBLEUUID kHidServiceUuid(static_cast<uint16_t>(0x1812));
const NimBLEUUID kAncsServiceUuid("7905F431-B5CE-4E99-A40F-4B1E122D00D0");

struct Event {
    enum class Type : uint8_t { Connect, Disconnect, Authentication } type;
    uint16_t connHandle;
    int reason;
    bool encrypted;
    bool bonded;
    ble_addr_t peer;
};

QueueHandle_t events = nullptr;
std::atomic<uint32_t> droppedEvents{0};
Phase currentPhase = Phase::Advertising;

// Runs on the BLE host task: only copies the event for loop().
void enqueue(const Event& event) {
    if (xQueueSend(events, &event, 0) != pdTRUE) {
        droppedEvents++;
    }
}

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*, NimBLEConnInfo& info) override {
        enqueue({Event::Type::Connect, info.getConnHandle(), 0, false, false, info.getIdAddress().getBase()[0]});
    }

    void onDisconnect(NimBLEServer*, NimBLEConnInfo& info, int reason) override {
        enqueue({Event::Type::Disconnect, info.getConnHandle(), reason, false, false, info.getIdAddress().getBase()[0]});
    }

    // Also called when encryption fails: the event carries the resulting state.
    void onAuthenticationComplete(NimBLEConnInfo& info) override {
        enqueue({Event::Type::Authentication, info.getConnHandle(), 0, info.isEncrypted(), info.isBonded(),
                 info.getIdAddress().getBase()[0]});
    }
};

ServerCallbacks serverCallbacks;

void startAdvertising() {
    NimBLEAdvertisementData advertisement;
    advertisement.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
    advertisement.setAppearance(kKeyboardAppearance);
    advertisement.setCompleteServices16({kHidServiceUuid});
    uint8_t solicitation[2 + 16] = {sizeof(solicitation) - 1, kAdTypeSolicitation128};
    memcpy(solicitation + 2, kAncsServiceUuid.getValue(), 16);
    if (!advertisement.addData(solicitation, sizeof(solicitation))) {
        logMessage(LogLevel::Error, "BLE", "advertising payload too long");
    }

    NimBLEAdvertisementData scanResponse;
    scanResponse.setName(kDeviceName);

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    if (!advertising->setAdvertisementData(advertisement) || !advertising->setScanResponseData(scanResponse) ||
        !advertising->start()) {
        logMessage(LogLevel::Error, "BLE", "advertising failed to start");
    }
}

bool setPhase(Phase next) {
    if (next == currentPhase) {
        return false;
    }
    currentPhase = next;
    logMessage(LogLevel::Info, "BLE", "phase %s", phaseName(next));
    return true;
}

bool handle(const Event& event) {
    const NimBLEAddress peer(event.peer);
    switch (event.type) {
        case Event::Type::Connect:
            logMessage(LogLevel::Info, "BLE", "connected peer=%s", peer.toString().c_str());
            NimBLEDevice::startSecurity(event.connHandle);
            // A bonded iPhone can re-encrypt before this event is handled: never step back from secured.
            return currentPhase == Phase::Secured ? false : setPhase(Phase::Connected);

        case Event::Type::Disconnect:
            logMessage(LogLevel::Info, "BLE", "disconnected reason=0x%x advertising=%s", event.reason,
                       NimBLEDevice::getAdvertising()->isAdvertising() ? "yes" : "no");
            ancs::onDisconnected();
            // A failed pairing stays visible while advertising, until the next connection.
            return currentPhase == Phase::PairingFailed ? false : setPhase(Phase::Advertising);

        case Event::Type::Authentication:
            if (!event.encrypted) {
                logMessage(LogLevel::Warn, "BLE", "pairing failed peer=%s", peer.toString().c_str());
                return setPhase(Phase::PairingFailed);
            }
            if (NimBLEDevice::getNumBonds() > 1) {
                logMessage(LogLevel::Warn, "BLE", "second device refused peer=%s: reset pairing first",
                           peer.toString().c_str());
                NimBLEDevice::deleteBond(peer);
                NimBLEDevice::getServer()->disconnect(event.connHandle);
                return false;
            }
            logMessage(LogLevel::Info, "BLE", "secured bonded=%s peer=%s", event.bonded ? "yes" : "no",
                       peer.toString().c_str());
            ancs::onSecured(event.connHandle);
            return setPhase(Phase::Secured);
    }
    return false;
}

}  // namespace

void begin(uint8_t batteryLevel) {
    events = xQueueCreate(kEventQueueDepth, sizeof(Event));
    ancs::begin();

    NimBLEDevice::init(kDeviceName);
    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    // Identity keys let the bond survive the iPhone's rotating private address.
    NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);

    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);
    server->advertiseOnDisconnect(true);
    createHidServices(server, batteryLevel);
    server->start();

    startAdvertising();
    logMessage(LogLevel::Info, "BLE", "advertising as %s bonds=%d", kDeviceName, NimBLEDevice::getNumBonds());
}

bool poll() {
    bool changed = false;
    Event event;
    while (xQueueReceive(events, &event, 0) == pdTRUE) {
        changed |= handle(event);
    }
    const uint32_t dropped = droppedEvents.exchange(0);
    if (dropped > 0) {
        logMessage(LogLevel::Warn, "BLE", "event queue full: %u events dropped", static_cast<unsigned>(dropped));
    }
    return changed;
}

bool resetPairing() {
    const int bonds = NimBLEDevice::getNumBonds();
    NimBLEServer* server = NimBLEDevice::getServer();
    for (uint16_t connHandle : server->getPeerDevices()) {
        server->disconnect(connHandle);
    }
    if (bonds == 0) {
        logMessage(LogLevel::Info, "BLE", "pairing reset: no bonds");
        return false;
    }
    if (!NimBLEDevice::deleteAllBonds()) {
        logMessage(LogLevel::Error, "BLE", "pairing reset failed: %d bond(s) left", NimBLEDevice::getNumBonds());
        return false;
    }
    logMessage(LogLevel::Info, "BLE", "pairing reset: %d bond(s) erased", bonds);
    return setPhase(Phase::PairingReset);
}

Phase phase() {
    return currentPhase;
}

const char* phaseName(Phase phase) {
    switch (phase) {
        case Phase::Advertising:   return "advertising";
        case Phase::Connected:     return "connected";
        case Phase::Secured:       return "secured";
        case Phase::PairingFailed: return "pairing failed";
        case Phase::PairingReset:  return "pairing reset";
    }
    return "?";
}

}  // namespace ble
