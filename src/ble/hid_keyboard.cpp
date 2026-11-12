#include "ble/hid_keyboard.h"

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include <atomic>

namespace ble {

namespace {

constexpr uint8_t kKeyboardReportId = 1;
constexpr uint8_t kConsumerReportId = 2;

uint8_t kReportMap[] = {
    // Keyboard: modifiers, reserved byte, 6 key codes; LED output report.
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x85, kKeyboardReportId,
    0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
    0x95, 0x01, 0x75, 0x08, 0x81, 0x01,
    0x95, 0x05, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02,
    0x95, 0x01, 0x75, 0x03, 0x91, 0x01,
    0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00, 0x29, 0x65, 0x81, 0x00,
    0xC0,
    // Consumer control: one 16-bit usage.
    0x05, 0x0C, 0x09, 0x01, 0xA1, 0x01, 0x85, kConsumerReportId,
    0x15, 0x00, 0x26, 0xFF, 0x03, 0x19, 0x00, 0x2A, 0xFF, 0x03, 0x75, 0x10, 0x95, 0x01, 0x81, 0x00,
    0xC0,
};

NimBLECharacteristic* keyboardInput = nullptr;
NimBLECharacteristic* consumerInput = nullptr;
std::atomic<bool> keyboardSubscribed{false};

// Runs on the BLE host task. NimBLE also calls it when it restores a bonded peer's
// subscription at reconnection and when the link drops.
class KeyboardInputCallbacks : public NimBLECharacteristicCallbacks {
    void onSubscribe(NimBLECharacteristic*, NimBLEConnInfo&, uint16_t subValue) override {
        keyboardSubscribed = (subValue & 0x01) != 0;
    }
};

KeyboardInputCallbacks keyboardInputCallbacks;

}  // namespace

void createHidServices(NimBLEServer* server, uint8_t batteryLevel) {
    static NimBLEHIDDevice hid(server);
    hid.setManufacturer("Tasca");
    hid.setPnp(0x02, 0x303A, 0x4001, 0x0100);
    hid.setHidInfo(0x00, 0x02);
    hid.setReportMap(kReportMap, sizeof(kReportMap));
    keyboardInput = hid.getInputReport(kKeyboardReportId);
    keyboardInput->setCallbacks(&keyboardInputCallbacks);
    hid.getOutputReport(kKeyboardReportId);
    consumerInput = hid.getInputReport(kConsumerReportId);
    hid.setBatteryLevel(batteryLevel);
}

bool keyboardReportsEnabled() {
    return keyboardSubscribed;
}

bool sendKeyboardReport(const KeyboardReport& report) {
    // notify() does not check the subscription: callers send only when keyboardReportsEnabled().
    return keyboardInput->notify(report.data(), report.size());
}

bool sendConsumerUsage(uint16_t usage) {
    const uint8_t press[] = {static_cast<uint8_t>(usage & 0xFF), static_cast<uint8_t>(usage >> 8)};
    const uint8_t release[] = {0, 0};
    const bool pressed = consumerInput->notify(press, sizeof(press));
    return consumerInput->notify(release, sizeof(release)) && pressed;
}

}  // namespace ble
