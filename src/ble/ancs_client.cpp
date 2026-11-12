#include "ble/ancs_client.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <string.h>

#include <atomic>
#include <deque>

#include "core/ancs_request.h"
#include "core/chat_key.h"
#include "log.h"

namespace ancs {

namespace {

const NimBLEUUID kServiceUuid("7905F431-B5CE-4E99-A40F-4B1E122D00D0");
const NimBLEUUID kNotificationSourceUuid("9FBF120D-6301-42D9-8C58-25E699A21DBD");
const NimBLEUUID kDataSourceUuid("22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB");
const NimBLEUUID kControlPointUuid("69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9");

constexpr uint16_t kNoConnection = BLE_HS_CONN_HANDLE_NONE;

template <size_t N>
struct Packet {
    uint16_t length;  // as received, possibly more than N
    uint8_t data[N];
};
using NsPacket = Packet<8>;
using DsPacket = Packet<256>;

// Deep enough for the PreExisting flood that follows the Notification Source subscription.
constexpr UBaseType_t kNsQueueDepth = 128;
constexpr UBaseType_t kDsQueueDepth = 16;
// Well above the ~110 notifications sent at connection; each waiting request takes 8 bytes.
constexpr size_t kMaxPendingRequests = 512;
constexpr uint32_t kResponseTimeoutMs = 2000;

// Apps whose notifications are kept; every other app is only counted.
const char* const kAcceptedApps[] = {"net.whatsapp.WhatsApp"};

QueueHandle_t nsQueue = nullptr;
QueueHandle_t dsQueue = nullptr;
std::atomic<uint32_t> droppedNs{0};
std::atomic<uint32_t> droppedDs{0};

uint16_t pendingSetup = kNoConnection;
uint16_t activeConnection = kNoConnection;
State currentState = State::Idle;
MessageStore store;

struct Request {
    uint32_t uid;
    bool preExisting;
};

// Attribute requests waiting in arrival order, at most one in flight.
NimBLERemoteCharacteristic* controlPoint = nullptr;
std::deque<Request> pendingRequests;
uint32_t droppedRequests = 0;
Request inFlight{};
uint32_t inFlightSinceMs = 0;
AncsResponseDecoder response;
uint32_t otherAppNotifications = 0;

// Runs on the BLE host task: only copies the packet for loop().
template <size_t N>
void enqueue(QueueHandle_t queue, std::atomic<uint32_t>& dropped, const uint8_t* data, size_t length) {
    Packet<N> packet;
    packet.length = static_cast<uint16_t>(length);
    memcpy(packet.data, data, length < N ? length : N);
    if (xQueueSend(queue, &packet, 0) != pdTRUE) {
        dropped++;
    }
}

void onNotificationSource(NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
    enqueue<sizeof(NsPacket::data)>(nsQueue, droppedNs, data, length);
}

void onDataSource(NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
    enqueue<sizeof(DsPacket::data)>(dsQueue, droppedDs, data, length);
}

bool isAccepted(const std::string& app) {
    for (const char* accepted : kAcceptedApps) {
        if (app == accepted) {
            return true;
        }
    }
    return false;
}

void logCounts() {
    logMessage(LogLevel::Info, "MSG", "messages=%u unread=%u", static_cast<unsigned>(store.size()),
               static_cast<unsigned>(store.unread()));
}

// Returns true when the stored Messages changed.
bool handleAttributes(const AncsAttributes& attributes) {
    if (!isAccepted(attributes.app)) {
        otherAppNotifications++;
        logMessage(LogLevel::Debug, "ANCS", "notification from another app (%u so far)",
                   static_cast<unsigned>(otherAppNotifications));
        return false;
    }
    const ChatKey key = chatKeyOf(attributes.title, attributes.subtitle);
    const MessageStore::Result result =
        store.receive(inFlight.uid, inFlight.preExisting, key, attributes.message, attributes.date);
    if (result == MessageStore::Result::Linked) {
        logMessage(LogLevel::Debug, "MSG", "already stored: uid=%u", static_cast<unsigned>(inFlight.uid));
        return false;
    }
    // Message content stays at Debug level: a build with a higher minimum level never logs it.
    logMessage(LogLevel::Debug, "MSG", "%s chat=\"%s\" from=\"%s\" text=\"%s\"",
               result == MessageStore::Result::Added ? "new" : "updated", key.chat.c_str(), key.sender.c_str(),
               attributes.message.c_str());
    logCounts();
    return true;
}

bool setState(State next) {
    if (next == currentState) {
        return false;
    }
    currentState = next;
    return true;
}

// Blocking discovery and subscriptions: loop() only, never from a NimBLE callback.
bool setUp(uint16_t connHandle) {
    NimBLEClient* client = NimBLEDevice::getServer()->getClient(connHandle);
    if (client == nullptr) {
        logMessage(LogLevel::Error, "ANCS", "no GATT client for connection %u", connHandle);
        return false;
    }
    NimBLERemoteService* service = client->getService(kServiceUuid);
    if (!client->isConnected()) {
        logMessage(LogLevel::Info, "ANCS", "setup interrupted: disconnected");
        return false;
    }
    if (service == nullptr) {
        logMessage(LogLevel::Warn, "ANCS", "ANCS not found");
        return false;
    }
    NimBLERemoteCharacteristic* dataSource = service->getCharacteristic(kDataSourceUuid);
    NimBLERemoteCharacteristic* notificationSource = service->getCharacteristic(kNotificationSourceUuid);
    NimBLERemoteCharacteristic* controlPointCharacteristic = service->getCharacteristic(kControlPointUuid);
    // subscribe() reports success when the CCCD is missing: check it, or a silent link would look active.
    const NimBLEUUID cccdUuid(static_cast<uint16_t>(0x2902));
    if (dataSource == nullptr || notificationSource == nullptr || controlPointCharacteristic == nullptr ||
        dataSource->getDescriptor(cccdUuid) == nullptr || notificationSource->getDescriptor(cccdUuid) == nullptr) {
        logMessage(LogLevel::Warn, "ANCS", "ANCS not found: characteristics missing");
        return false;
    }
    // Data Source first: the responses to attribute requests must never be missed.
    const bool subscribed = dataSource->subscribe(true, onDataSource, true) &&
                            notificationSource->subscribe(true, onNotificationSource, true);
    if (!client->isConnected()) {
        logMessage(LogLevel::Info, "ANCS", "setup interrupted: disconnected");
        return false;
    }
    if (!subscribed) {
        logMessage(LogLevel::Warn, "ANCS", "subscription refused (error %d): notification sharing not allowed?",
                   client->getLastError());
        return setState(State::Denied);
    }
    logMessage(LogLevel::Info, "ANCS", "subscribed: data source, notification source");
    controlPoint = controlPointCharacteristic;
    return setState(State::Active);
}

// Returns true when the stored Messages changed.
bool handleEvent(const NsPacket& packet) {
    AncsEvent event;
    // Packets of a connection already gone carry UIDs the next connection reuses for other notifications.
    if (controlPoint == nullptr || !decodeAncsEvent(packet.data, packet.length, &event)) {
        return false;
    }
    if (event.kind == AncsEvent::Kind::Removed) {
        if (!store.markRead(event.uid)) {
            return false;
        }
        logMessage(LogLevel::Debug, "MSG", "read: uid=%u", static_cast<unsigned>(event.uid));
        logCounts();
        return true;
    }
    if (pendingRequests.size() >= kMaxPendingRequests) {
        droppedRequests++;
        return false;
    }
    pendingRequests.push_back({event.uid, event.preExisting});
    return false;
}

// Sends the next request when none is in flight. Blocking GATT write: loop() only.
void sendNextRequest() {
    if (response.active() && millis() - inFlightSinceMs >= kResponseTimeoutMs) {
        logMessage(LogLevel::Warn, "ANCS", "no response for uid=%u within %u ms", static_cast<unsigned>(inFlight.uid),
                   static_cast<unsigned>(kResponseTimeoutMs));
        response.clear();
    }
    while (!response.active() && controlPoint != nullptr && !pendingRequests.empty()) {
        const Request next = pendingRequests.front();
        const uint32_t uid = next.uid;
        pendingRequests.pop_front();
        uint8_t request[kAncsRequestSize];
        const size_t size = encodeGetNotificationAttributes(request, uid);
        if (!controlPoint->writeValue(request, size, true)) {
            // An ANCS error (e.g. 0xA2, notification already gone) arrives as a failed write, with no Data Source answer.
            logMessage(LogLevel::Warn, "ANCS", "request for uid=%u refused (error 0x%x)", static_cast<unsigned>(uid),
                       controlPoint->getClient()->getLastError());
            continue;
        }
        inFlight = next;
        inFlightSinceMs = millis();
        response.start(uid);
    }
}

void logDropped(const char* channel, std::atomic<uint32_t>& dropped) {
    const uint32_t count = dropped.exchange(0);
    if (count > 0) {
        logMessage(LogLevel::Warn, "ANCS", "%s queue full: %u packets dropped", channel, static_cast<unsigned>(count));
    }
}

}  // namespace

void begin() {
    nsQueue = xQueueCreate(kNsQueueDepth, sizeof(NsPacket));
    dsQueue = xQueueCreate(kDsQueueDepth, sizeof(DsPacket));
}

void onSecured(uint16_t connHandle) {
    // iOS repeats the encryption event: set up once per connection (getClient() drops the discovered services).
    if (connHandle != activeConnection) {
        activeConnection = connHandle;
        pendingSetup = connHandle;
    }
}

void onDisconnected() {
    activeConnection = kNoConnection;
    pendingSetup = kNoConnection;
    currentState = State::Idle;
    controlPoint = nullptr;
    const size_t dropped = pendingRequests.size() + (response.active() ? 1 : 0);
    if (dropped > 0) {
        logMessage(LogLevel::Info, "ANCS", "disconnected: %u attribute requests dropped", static_cast<unsigned>(dropped));
    }
    pendingRequests.clear();
    response.clear();
    // UIDs restart at the next connection: Messages are matched again by content then.
    store.unlinkAll();
}

bool poll() {
    bool changed = false;
    if (pendingSetup != kNoConnection) {
        const uint16_t connHandle = pendingSetup;
        pendingSetup = kNoConnection;
        logMessage(LogLevel::Info, "ANCS", "discovering");
        changed |= setUp(connHandle);
    }
    NsPacket ns;
    while (xQueueReceive(nsQueue, &ns, 0) == pdTRUE) {
        changed |= handleEvent(ns);
    }
    if (droppedRequests > 0) {
        logMessage(LogLevel::Warn, "ANCS", "request queue full: %u attribute requests dropped",
                   static_cast<unsigned>(droppedRequests));
        droppedRequests = 0;
    }
    // A lost Data Source packet leaves a hole in the response being decoded: drop that response.
    if (droppedDs > 0 && response.active()) {
        logMessage(LogLevel::Warn, "ANCS", "response for uid=%u lost a packet: discarded",
                   static_cast<unsigned>(inFlight.uid));
        response.clear();
    }
    DsPacket ds;
    while (xQueueReceive(dsQueue, &ds, 0) == pdTRUE) {
        if (ds.length > sizeof(ds.data)) {
            logMessage(LogLevel::Warn, "ANCS", "DS packet of %u bytes cut to %u", static_cast<unsigned>(ds.length),
                       static_cast<unsigned>(sizeof(ds.data)));
        }
        const size_t stored = ds.length < sizeof(ds.data) ? ds.length : sizeof(ds.data);
        switch (response.feed(ds.data, stored)) {
            case AncsResponseDecoder::Result::Complete:
                changed |= handleAttributes(response.attributes());
                break;
            case AncsResponseDecoder::Result::Malformed:
                logMessage(LogLevel::Warn, "ANCS", "malformed response for uid=%u: discarded",
                           static_cast<unsigned>(inFlight.uid));
                break;
            case AncsResponseDecoder::Result::Ignored:
            case AncsResponseDecoder::Result::Progress:
                break;
        }
    }
    logDropped("NS", droppedNs);
    logDropped("DS", droppedDs);
    sendNextRequest();
    return changed;
}

State state() {
    return currentState;
}

const MessageStore& messages() {
    return store;
}

}  // namespace ancs
