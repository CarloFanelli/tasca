#include "mesh/lora_unit.h"

#include <Arduino.h>
#include <esp_mac.h>

#include <vector>

#include "core/at_link.h"
#include "core/lora_airtime.h"
#include "core/mesh_name.h"
#include "log.h"

namespace lora {

namespace {

HardwareSerial& port = Serial1;
constexpr int kRxPin = 1;
constexpr int kTxPin = 2;
constexpr uint32_t kBaud = 115200;
// A received packet of 255 bytes is a line of about 540 characters.
constexpr size_t kRxBufferSize = 2048;
constexpr size_t kMaxLineLength = 600;
// Lines handled per loop, so a burst does not delay drawing.
constexpr int kLinesPerLoop = 4;
constexpr uint32_t kAnnounceEveryMs = 3 * 3600 * 1000;

// Meshtastic EU_868 MediumFast: 869.525 MHz, SF9, 250 kHz, CR 4/5, preamble 16, sync word 0x2B (24B4 for the
// unit). Receiving is stopped first: a listening unit refuses changes. A mode change restarts the unit, even to
// the mode it is in. The unit's highest power, 22 dBm, stays within the 27 dBm allowed at 869.4-869.65 MHz.
const std::vector<AtStep> kSetup = {
    {"AT+PRECV=0", 0},           {"AT+NWM=0", 1500},       {"AT+RFFREQUENCY=869525000", 0},
    {"AT+SPREADINGFACTOR=9", 0}, {"AT+BANDWIDTH=1", 0},    {"AT+CODINGRATE=0", 0},
    {"AT+PREAMBLELENGTH=16", 0}, {"AT+IQINVER=0", 0},      {"AT+SYNCWORD=24B4", 0},
    {"AT+ENCRY=0", 0},           {"AT+TXOUTPUTPOWER=22", 0}, {"AT+PRECV=65535", 0},
};

AtLink link(kSetup);
LineSplitter splitter(kMaxLineLength);
MeshStore store;
AirtimeBudget airtime;
uint32_t ownNode = 0;
// The packet ID of the user's message on its way; 0 for an announcement.
uint32_t sendingId = 0;
MeshUser names;
// The user's message waiting for the unit to finish the send in flight; ID 0 for none.
std::vector<uint8_t> queuedText;
uint32_t queuedId = 0;
// The names wait to be announced: the unit was found, they changed, or 3 hours passed.
bool announceDue = false;
uint32_t announcedMs = 0;

uint32_t newPacketId() {
    uint32_t id = 0;
    while (id == 0) {
        id = esp_random();
    }
    return id;
}

// The packet goes out when the unit takes it and the hour's radio time allows; false otherwise.
bool transmit(const std::vector<uint8_t>& packet) {
    const uint32_t airtimeMs = loraAirtimeMs(packet.size());
    if (!airtime.allows(millis(), airtimeMs) || !link.send(toHex(packet))) {
        return false;
    }
    airtime.spend(millis(), airtimeMs);
    return true;
}

// The queued message goes out once the unit is free; "not sent" if the unit goes meanwhile.
bool pollQueued() {
    if (queuedId == 0) {
        return false;
    }
    if (!link.present()) {
        logMessage(LogLevel::Warn, "MESH", "not sent %08x: unit gone", static_cast<unsigned>(queuedId));
        const bool changed = store.markNotSent(queuedId);
        queuedId = 0;
        return changed;
    }
    if (transmit(queuedText)) {
        sendingId = queuedId;
        queuedId = 0;
    }
    return false;
}

void pollAnnounce() {
    if (!link.present()) {
        return;
    }
    if (announceDue || millis() - announcedMs >= kAnnounceEveryMs) {
        announceDue = true;
        if (transmit(encodeMeshUser(ownNode, newPacketId(), names))) {
            announceDue = false;
            announcedMs = millis();
            sendingId = 0;
            logMessage(LogLevel::Info, "MESH", "announcing %s", names.shortName.c_str());
        }
    }
}

bool handlePacket(const LoraPacket& received) {
    std::vector<uint8_t> bytes;
    MeshPacket packet;
    if (!parseHex(received.hex, bytes) || !decodeMeshPacket(bytes, packet)) {
        logMessage(LogLevel::Debug, "MESH", "packet not on the public channel (%u bytes)",
                   static_cast<unsigned>(received.hex.size() / 2));
        return false;
    }
    logMessage(LogLevel::Debug, "MESH", "from !%08x port %u rssi %d snr %d", static_cast<unsigned>(packet.from),
               static_cast<unsigned>(packet.port), received.rssi, received.snr);
    return store.add(packet, received.rssi, millis());
}

}  // namespace

void begin() {
    // As Meshtastic does: the node ID is the last 4 bytes of the Bluetooth address.
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    ownNode = static_cast<uint32_t>(mac[2]) << 24 | static_cast<uint32_t>(mac[3]) << 16 |
              static_cast<uint32_t>(mac[4]) << 8 | mac[5];
    store.setOwnNode(ownNode);
    logMessage(LogLevel::Info, "MESH", "node !%08x", static_cast<unsigned>(ownNode));
    port.setRxBufferSize(kRxBufferSize);
    port.begin(kBaud, SERIAL_8N1, kRxPin, kTxPin);
}

bool poll() {
    const bool wasPresent = link.present();
    std::string line;
    for (int lines = 0; lines < kLinesPerLoop && port.available() > 0;) {
        if (splitter.add(static_cast<char>(port.read()), line)) {
            link.onLine(line, millis());
            lines++;
        }
    }
    if (const char* command = link.due(millis())) {
        port.print(command);
        port.print("\r\n");
    }
    bool changed = link.present() != wasPresent;
    if (changed) {
        logMessage(LogLevel::Info, "MESH", link.present() ? "LoRa unit listening" : "LoRa unit gone");
        announceDue = announceDue || link.present();
    }
    LoraPacket received;
    while (link.takePacket(received)) {
        changed |= handlePacket(received);
    }
    const SendResult sent = link.takeSendResult();
    if (sent == SendResult::Sent) {
        logMessage(LogLevel::Info, "MESH", "sent %08x", static_cast<unsigned>(sendingId));
    } else if (sent == SendResult::Failed && sendingId == 0) {
        logMessage(LogLevel::Warn, "MESH", "announcement not sent");
        announceDue = true;
    } else if (sent == SendResult::Failed) {
        logMessage(LogLevel::Warn, "MESH", "not sent %08x", static_cast<unsigned>(sendingId));
        changed |= store.markNotSent(sendingId);
    }
    changed |= store.expire(millis());
    changed |= pollQueued();
    pollAnnounce();
    return changed;
}

void setMeshName(const std::string& chosen) {
    const std::string longName = meshLongName(chosen, ownNode);
    if (longName == names.longName) {
        return;
    }
    names.longName = longName;
    names.shortName = meshShortName(longName);
    store.setOwnShortName(names.shortName);
    announceDue = true;
}

const std::string& meshName() {
    return names.longName;
}

bool sendText(const std::string& text) {
    const uint32_t id = newPacketId();
    const std::vector<uint8_t> packet = encodeMeshText(ownNode, id, text);
    if (!airtime.allows(millis(), loraAirtimeMs(packet.size()))) {
        logMessage(LogLevel::Warn, "MESH", "radio limit reached: message refused");
        return false;
    }
    store.addOwn(id, text, millis());
    if (transmit(packet)) {
        sendingId = id;
    } else if (link.present() && queuedId == 0) {
        // The unit is still busy with another packet, such as the name announcement.
        queuedText = packet;
        queuedId = id;
    } else {
        store.markNotSent(id);
        logMessage(LogLevel::Warn, "MESH", "not sent %08x: unit not ready", static_cast<unsigned>(id));
    }
    return true;
}

bool present() {
    return link.present();
}

MeshStore& messages() {
    return store;
}

}  // namespace lora
