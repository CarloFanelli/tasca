#pragma once

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <string>
#include <vector>

// Splits the bytes of a serial line into lines, at CR or LF; empty lines and lines longer than the limit are dropped.
class LineSplitter {
public:
    explicit LineSplitter(size_t maxLength) : maxLength_(maxLength) {}

    // True when c completed a line, then in line.
    bool add(char c, std::string& line);

private:
    size_t maxLength_;
    std::string pending_;
    bool overlong_ = false;
};

// A LoRa packet the unit received: signal strength and noise margin in dB, the bytes in hex.
struct LoraPacket {
    int rssi = 0;
    int snr = 0;
    std::string hex;
};

// How the last packet given to send() went.
enum class SendResult { None, Sent, Failed };

// A setup command and how long the unit needs after its "OK" before the next one, e.g. to restart.
struct AtStep {
    std::string command;
    uint32_t settleMs = 0;
};

// The AT dialogue with the LoRa unit: finds it, sends the setup commands one by one, then checks it keeps
// answering while it listens. One command at a time, each awaiting "OK".
class AtLink {
public:
    explicit AtLink(std::vector<AtStep> setup) : setup_(std::move(setup)) {}

    // The command to write now, or nullptr; a command returned counts as sent at nowMs.
    const char* due(uint32_t nowMs);
    // A line read from the unit.
    void onLine(const std::string& line, uint32_t nowMs);
    // Set up and listening.
    bool present() const { return state_ == State::Listening; }
    // The oldest received packet not taken yet; false when none.
    bool takePacket(LoraPacket& packet);
    // Sends a packet given in hex: listening stops, the unit transmits, listening resumes. False while the unit is
    // absent or another packet is on its way.
    bool send(const std::string& hex);
    // Sent or Failed once per packet, when the unit is listening again or was lost; None meanwhile.
    SendResult takeSendResult();

private:
    enum class State { Absent, SettingUp, Listening };
    enum class Tx { Idle, StopListening, Send, AwaitDone, Listen };

    void becomeAbsent(uint32_t nowMs);

    std::vector<AtStep> setup_;
    State state_ = State::Absent;
    size_t step_ = 0;
    // A command was sent and its answer is awaited.
    bool awaiting_ = false;
    uint32_t sentMs_ = 0;
    // Last probe while absent, last "OK" while setting up, or last line from the unit while listening.
    uint32_t lastMs_ = 0;
    bool probed_ = false;
    int missed_ = 0;
    std::deque<LoraPacket> packets_;
    Tx tx_ = Tx::Idle;
    std::string txCommand_;
    SendResult sendResult_ = SendResult::None;
};
