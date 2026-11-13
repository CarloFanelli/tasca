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
#include "core/incoming_call.h"
#include "core/reconnect_import.h"
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
// Categories whose attributes are requested: WhatsApp messages come as Social (4), seen on an iPhone,
// and calls as IncomingCall (1), from any app. Other categories (missed calls included) are only
// counted, so the import at connection skips most other apps.
constexpr uint8_t kCategoryIncomingCall = 1;
const uint8_t kAcceptedCategories[] = {4, kCategoryIncomingCall};

QueueHandle_t nsQueue = nullptr;
QueueHandle_t dsQueue = nullptr;
std::atomic<uint32_t> droppedNs{0};
std::atomic<uint32_t> droppedDs{0};

uint16_t pendingSetup = kNoConnection;
uint16_t activeConnection = kNoConnection;
State currentState = State::Idle;
MessageStore store;
CallTracker calls;

struct Request {
    uint32_t uid;
    bool preExisting;
    bool call;
};

// Attribute requests waiting in arrival order, at most one in flight.
NimBLERemoteCharacteristic* controlPoint = nullptr;
// UIDs to clear on the iPhone, one negative action each, in any order.
std::vector<uint32_t> pendingClears;
// Messages of the clearing that stay: refused by the iPhone or not linked.
size_t notClearedCount = 0;
bool notClearedReady = false;
std::deque<Request> pendingRequests;
uint32_t droppedRequests = 0;
Request inFlight{};
uint32_t inFlightSinceMs = 0;
AncsResponseDecoder response;
uint32_t otherAppNotifications = 0;
uint32_t otherCategoryNotifications = 0;
ReconnectImport import;

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

bool isAcceptedCategory(uint8_t category) {
    for (uint8_t accepted : kAcceptedCategories) {
        if (category == accepted) {
            return true;
        }
    }
    return false;
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

// The attributes of a request that will get no answer: a call shows without its title.
// Returns true when a ringing call changed.
bool requestFailed(const Request& request) {
    return request.call && calls.titleFailed(request.uid);
}

// Returns true when the stored Messages or the ringing calls changed.
bool handleAttributes(const AncsAttributes& attributes) {
    // A call is never stored, and its caller never logged.
    if (inFlight.call) {
        return calls.setTitle(inFlight.uid, attributes.title);
    }
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
    import.start(millis());
    return setState(State::Active);
}

// Returns true when the stored Messages or the ringing calls changed.
bool handleEvent(const NsPacket& packet) {
    AncsEvent event;
    // Packets of a connection already gone carry UIDs the next connection reuses for other notifications.
    if (controlPoint == nullptr || !decodeAncsEvent(packet.data, packet.length, &event)) {
        return false;
    }
    // Gone from the iPhone (read or cleared there): gone from the Cardputer too.
    if (event.kind == AncsEvent::Kind::Removed) {
        if (calls.end(event.uid)) {
            logMessage(LogLevel::Info, "ANCS", "call ended: uid=%u", static_cast<unsigned>(event.uid));
            return true;
        }
        if (!store.remove(event.uid)) {
            return false;
        }
        logMessage(LogLevel::Debug, "MSG", "removed: uid=%u", static_cast<unsigned>(event.uid));
        logCounts();
        return true;
    }
    if (event.preExisting) {
        import.onPreExistingEvent(millis());
    }
    if (!isAcceptedCategory(event.category)) {
        otherCategoryNotifications++;
        return false;
    }
    if (event.category == kCategoryIncomingCall) {
        const bool added = !calls.isCall(event.uid);
        calls.ring(event.uid, event.positiveAction, event.negativeAction);
        if (!added) {
            return true;
        }
        logMessage(LogLevel::Info, "ANCS", "incoming call: uid=%u", static_cast<unsigned>(event.uid));
        // Ahead of the import's requests: the caller's name is awaited on screen.
        pendingRequests.push_front({event.uid, event.preExisting, true});
        return true;
    }
    if (pendingRequests.size() >= kMaxPendingRequests) {
        droppedRequests++;
        import.onFailure();
        return false;
    }
    pendingRequests.push_back({event.uid, event.preExisting, false});
    return false;
}

// Sends the next request when none is in flight. Blocking GATT write: loop() only.
// One negative action per call, never while an attribute response is being received.
void sendNextClear() {
    if (response.active() || controlPoint == nullptr || pendingClears.empty()) {
        return;
    }
    const uint32_t uid = pendingClears.back();
    pendingClears.pop_back();
    uint8_t command[kAncsActionSize];
    const size_t size = encodePerformNotificationAction(command, uid, false);
    if (controlPoint->writeValue(command, size, true)) {
        logMessage(LogLevel::Debug, "ANCS", "clear sent: uid=%u", static_cast<unsigned>(uid));
    } else {
        logMessage(LogLevel::Warn, "ANCS", "clear of uid=%u refused (error 0x%x)", static_cast<unsigned>(uid),
                   controlPoint->getClient()->getLastError());
        notClearedCount++;
    }
    if (pendingClears.empty()) {
        notClearedReady = notClearedCount > 0;
    }
}

// Returns true when a ringing call changed.
bool sendNextRequest() {
    bool changed = false;
    if (response.active() && millis() - inFlightSinceMs >= kResponseTimeoutMs) {
        logMessage(LogLevel::Warn, "ANCS", "no response for uid=%u within %u ms", static_cast<unsigned>(inFlight.uid),
                   static_cast<unsigned>(kResponseTimeoutMs));
        response.clear();
        import.onFailure();
        changed |= requestFailed(inFlight);
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
            changed |= requestFailed(next);
            continue;
        }
        inFlight = next;
        inFlightSinceMs = millis();
        response.start(uid);
    }
    return changed;
}

void logDropped(const char* channel, std::atomic<uint32_t>& dropped) {
    const uint32_t count = dropped.exchange(0);
    if (count > 0) {
        import.onFailure();
        logMessage(LogLevel::Warn, "ANCS", "%s queue full: %u packets dropped", channel, static_cast<unsigned>(count));
    }
}

// Once the notifications resent at connection are all answered, Messages the iPhone no longer has
// were read or cleared there while disconnected: remove them. Returns true when the store changed.
bool finishImport() {
    size_t waiting = response.active() && inFlight.preExisting ? 1 : 0;
    for (const Request& r : pendingRequests) {
        waiting += r.preExisting ? 1 : 0;
    }
    if (!import.finished(millis(), waiting)) {
        return false;
    }
    if (!import.complete()) {
        logMessage(LogLevel::Warn, "MSG", "import incomplete: no Message removed");
        return false;
    }
    const size_t removed = store.removeUnlinked();
    logMessage(LogLevel::Info, "MSG", "import done: %u gone from the iPhone removed, %u of other categories skipped so far",
               static_cast<unsigned>(removed), static_cast<unsigned>(otherCategoryNotifications));
    if (removed == 0) {
        return false;
    }
    logCounts();
    return true;
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
    // UIDs restart at the next connection: a clear left over would hit another notification.
    if (!pendingClears.empty()) {
        logMessage(LogLevel::Info, "ANCS", "disconnected: %u clears dropped", static_cast<unsigned>(pendingClears.size()));
        pendingClears.clear();
    }
    response.clear();
    import.cancel();
    calls.clear();
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
        import.onFailure();
        changed |= requestFailed(inFlight);
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
                import.onFailure();
                changed |= requestFailed(inFlight);
                break;
            case AncsResponseDecoder::Result::Ignored:
            case AncsResponseDecoder::Result::Progress:
                break;
        }
    }
    logDropped("NS", droppedNs);
    logDropped("DS", droppedDs);
    changed |= sendNextRequest();
    sendNextClear();
    changed |= finishImport();
    return changed;
}

State state() {
    return currentState;
}

const MessageStore& messages() {
    return store;
}

const CallTracker& ringingCalls() {
    return calls;
}

bool actOnCall(bool positive) {
    const Call* call = calls.shown();
    if (call == nullptr || controlPoint == nullptr || !canAct(*call, positive)) {
        return false;
    }
    const uint32_t uid = call->uid;
    calls.actionSent(uid);
    uint8_t command[kAncsActionSize];
    const size_t size = encodePerformNotificationAction(command, uid, positive);
    if (!controlPoint->writeValue(command, size, true)) {
        logMessage(LogLevel::Warn, "ANCS", "%s of call uid=%u refused (error 0x%x)", positive ? "answer" : "decline",
                   static_cast<unsigned>(uid), controlPoint->getClient()->getLastError());
        calls.actionFailed(uid);
        return true;
    }
    logMessage(LogLevel::Info, "ANCS", "%s sent: call uid=%u", positive ? "answer" : "decline",
               static_cast<unsigned>(uid));
    return true;
}

void clearChat(const std::string& chat) {
    // Without ANCS no Message is linked: nothing is sent and every Message stays.
    if (controlPoint != nullptr) {
        pendingClears = store.linkedUids(chat);
    }
    notClearedCount = store.unlinkedCount(chat);
    logMessage(LogLevel::Info, "ANCS", "clearing a chat: %u notifications, %u not linked",
               static_cast<unsigned>(pendingClears.size()), static_cast<unsigned>(notClearedCount));
    // Nothing to send: only Messages that cannot be cleared.
    notClearedReady = pendingClears.empty() && notClearedCount > 0;
}

bool clearing() {
    return !pendingClears.empty();
}

bool takeNotCleared() {
    const bool ready = notClearedReady;
    notClearedReady = false;
    return ready;
}

void clearMessages() {
    const size_t cleared = store.size();
    store.clear();
    logMessage(LogLevel::Info, "MSG", "%u messages cleared", static_cast<unsigned>(cleared));
    logCounts();
}

bool markChatRead(const std::string& chat) {
    if (store.markChatRead(chat) == 0) {
        return false;
    }
    logCounts();
    return true;
}

}  // namespace ancs
