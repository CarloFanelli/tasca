#include "ble/ble_manager.h"

#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <atomic>

#include "ble/ams_client.h"
#include "ble/ancs_client.h"
#include "ble/hid_keyboard.h"
#include "log.h"

namespace ble {

namespace {

constexpr const char* kDeviceName = "Tasca";
constexpr uint16_t kKeyboardAppearance = 0x03C1;
constexpr uint8_t kAdTypeSolicitation128 = 0x15;
constexpr UBaseType_t kEventQueueDepth = 16;

// Link timings in Bluetooth units (interval 1.25 ms, timeout 10 ms), within Apple's rules for
// accessories: interval min >= 15 ms, max >= min + 15 ms, latency <= 30, timeout 2-6 s and above
// max interval * (latency + 1) * 3.
struct Timings {
    uint16_t minInterval;
    uint16_t maxInterval;
    uint16_t latency;
    uint16_t timeout;
};
// 15-30 ms, every event, 2 s.
constexpr Timings kUsualTimings{12, 24, 0, 200};
// 120-150 ms, up to 4 events skipped, 6 s.
constexpr Timings kSlowTimings{96, 120, 4, 600};

const NimBLEUUID kHidServiceUuid(static_cast<uint16_t>(0x1812));
const NimBLEUUID kAncsServiceUuid("7905F431-B5CE-4E99-A40F-4B1E122D00D0");
const NimBLEUUID kGenericAttributeUuid(static_cast<uint16_t>(0x1801));
const NimBLEUUID kServiceChangedUuid(static_cast<uint16_t>(0x2A05));

struct Event {
    enum class Type : uint8_t { Connect, Disconnect, Authentication, TimingsUpdate } type;
    uint16_t connHandle;
    int reason;
    bool encrypted;
    bool bonded;
    ble_addr_t peer;
    // TimingsUpdate: the link timings now in use, in Bluetooth units.
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
};

QueueHandle_t events = nullptr;
std::atomic<uint32_t> droppedEvents{0};
Phase currentPhase = Phase::Advertising;
uint16_t securedConn = BLE_HS_CONN_HANDLE_NONE;
// The connection whose iPhone services are set up, or waiting to be in loop().
uint16_t setUpConn = BLE_HS_CONN_HANDLE_NONE;
uint16_t pendingSetup = BLE_HS_CONN_HANDLE_NONE;
// The iPhone's GATT database as discovered on setUpConn; ANCS and AMS hold its characteristics.
NimBLEClient* gattClient = nullptr;
std::atomic<bool> servicesChanged{false};
// The secured link was lost by signal; taken by loop().
bool linkLost = false;
bool slowRequested = false;
// A request was sent and its outcome is not known yet; updates the iPhone starts itself are only logged.
bool timingsPending = false;

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

    // Called with the resulting timings, whether or not the iPhone granted the request.
    void onConnParamsUpdate(NimBLEConnInfo& info) override {
        enqueue({Event::Type::TimingsUpdate, info.getConnHandle(), 0, false, false, info.getIdAddress().getBase()[0],
                 info.getConnInterval(), info.getConnLatency(), info.getConnTimeout()});
    }
};

ServerCallbacks serverCallbacks;

// Runs on the BLE host task: loop() discovers again.
void onServiceChanged(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool) {
    servicesChanged = true;
}

// Blocking discovery and subscriptions: loop() only.
bool setUpServices(uint16_t connHandle) {
    // Once per connection: getClient() forgets every service discovered so far, ANCS included.
    gattClient = NimBLEDevice::getServer()->getClient(connHandle);
    if (gattClient == nullptr) {
        logMessage(LogLevel::Error, "BLE", "no GATT client for connection %u", connHandle);
        return false;
    }
    const bool changed = ancs::setUp(gattClient);
    ams::setUp(gattClient);
    NimBLERemoteService* service = gattClient->getService(kGenericAttributeUuid);
    NimBLERemoteCharacteristic* serviceChanged =
        service == nullptr ? nullptr : service->getCharacteristic(kServiceChangedUuid);
    if (serviceChanged == nullptr || !serviceChanged->subscribe(false, onServiceChanged, true)) {
        logMessage(LogLevel::Info, "BLE", "service changes not followed");
    }
    return changed;
}

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

// No retry when refused: the next request comes with the next screen change.
void logTimings(const Event& event) {
    logMessage(LogLevel::Info, "BLE", "link timings interval=%u.%02u ms latency=%u timeout=%u ms",
               static_cast<unsigned>(event.interval * 125 / 100), static_cast<unsigned>(event.interval * 125 % 100),
               static_cast<unsigned>(event.latency), static_cast<unsigned>(event.timeout * 10));
    if (!timingsPending) {
        return;
    }
    timingsPending = false;
    const Timings& requested = slowRequested ? kSlowTimings : kUsualTimings;
    if (event.interval < requested.minInterval || event.interval > requested.maxInterval) {
        logMessage(LogLevel::Warn, "BLE", "%s link timings refused by the iPhone", slowRequested ? "slow" : "usual");
    }
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
            ams::onDisconnected();
            // The reason is a host code: an HCI reason arrives offset by BLE_HS_ERR_HCI_BASE.
            linkLost = event.connHandle == securedConn && event.reason == BLE_HS_HCI_ERR(BLE_ERR_CONN_SPVN_TMO);
            securedConn = BLE_HS_CONN_HANDLE_NONE;
            setUpConn = BLE_HS_CONN_HANDLE_NONE;
            pendingSetup = BLE_HS_CONN_HANDLE_NONE;
            gattClient = nullptr;
            servicesChanged = false;
            timingsPending = false;
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
            securedConn = event.connHandle;
            // iOS repeats the encryption event: set up once per connection.
            if (event.connHandle != setUpConn) {
                setUpConn = event.connHandle;
                pendingSetup = event.connHandle;
            }
            return setPhase(Phase::Secured);

        case Event::Type::TimingsUpdate:
            logTimings(event);
            return false;
    }
    return false;
}

}  // namespace

void begin(uint8_t batteryLevel) {
    events = xQueueCreate(kEventQueueDepth, sizeof(Event));
    ancs::begin();
    ams::begin();

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
    if (pendingSetup != BLE_HS_CONN_HANDLE_NONE) {
        const uint16_t connHandle = pendingSetup;
        pendingSetup = BLE_HS_CONN_HANDLE_NONE;
        changed |= setUpServices(connHandle);
    }
    if (servicesChanged.exchange(false) && gattClient != nullptr) {
        logMessage(LogLevel::Info, "BLE", "services changed on the iPhone");
        ams::onServicesChanged(gattClient);
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

void requestTimings(bool slow) {
    slowRequested = slow;
    if (securedConn == BLE_HS_CONN_HANDLE_NONE) {
        return;
    }
    const Timings& t = slow ? kSlowTimings : kUsualTimings;
    NimBLEDevice::getServer()->updateConnParams(securedConn, t.minInterval, t.maxInterval, t.latency, t.timeout);
    timingsPending = true;
    logMessage(LogLevel::Debug, "BLE", "%s link timings requested", slow ? "slow" : "usual");
}

Phase phase() {
    return currentPhase;
}

bool bonded() {
    return NimBLEDevice::getNumBonds() > 0;
}

bool takeLinkLost() {
    const bool lost = linkLost;
    linkLost = false;
    return lost;
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
