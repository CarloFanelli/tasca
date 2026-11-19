#include "core/at_link.h"

#include <stdlib.h>

namespace {

// Absent: a probe each second. Listening: a probe after 1.5 s without a line, so a unit unplugged is
// missed twice within 5 s.
constexpr uint32_t kAbsentProbeMs = 1000;
constexpr uint32_t kListeningProbeMs = 1500;
constexpr uint32_t kAnswerMs = 500;
constexpr int kMissedLimit = 2;
constexpr size_t kMaxPackets = 8;
constexpr char kProbe[] = "AT";
constexpr char kReceived[] = "+EVT:RXP2P:";
constexpr char kSent[] = "+EVT:TXP2P DONE";
// The longest packet takes about 0.6 s on air.
constexpr uint32_t kSendMs = 3000;
constexpr char kStopListening[] = "AT+PRECV=0";
constexpr char kListen[] = "AT+PRECV=65535";

// "+EVT:RXP2P:<rssi>:<snr>:<hex>".
bool parsePacket(const std::string& line, LoraPacket& packet) {
    if (line.compare(0, sizeof(kReceived) - 1, kReceived) != 0) {
        return false;
    }
    const char* p = line.c_str() + sizeof(kReceived) - 1;
    char* end = nullptr;
    packet.rssi = static_cast<int>(strtol(p, &end, 10));
    if (end == p || *end != ':') {
        return false;
    }
    p = end + 1;
    packet.snr = static_cast<int>(strtol(p, &end, 10));
    if (end == p || *end != ':') {
        return false;
    }
    packet.hex = end + 1;
    return true;
}

}  // namespace

bool LineSplitter::add(char c, std::string& line) {
    if (c != '\r' && c != '\n') {
        if (pending_.size() < maxLength_) {
            pending_ += c;
        } else {
            overlong_ = true;
        }
        return false;
    }
    const bool complete = !pending_.empty() && !overlong_;
    if (complete) {
        line = pending_;
    }
    pending_.clear();
    overlong_ = false;
    return complete;
}

const char* AtLink::due(uint32_t nowMs) {
    if (tx_ == Tx::AwaitDone) {
        if (nowMs - sentMs_ >= kSendMs) {
            becomeAbsent(nowMs);
        }
        return nullptr;
    }
    if (awaiting_) {
        if (nowMs - sentMs_ < kAnswerMs) {
            return nullptr;
        }
        awaiting_ = false;
        if (state_ == State::SettingUp || tx_ != Tx::Idle ||
            (state_ == State::Listening && ++missed_ >= kMissedLimit)) {
            becomeAbsent(nowMs);
            return nullptr;
        }
    }
    const char* command = nullptr;
    switch (state_) {
        case State::Absent:
            if (!probed_ || nowMs - lastMs_ >= kAbsentProbeMs) {
                probed_ = true;
                lastMs_ = nowMs;
                command = kProbe;
            }
            break;
        case State::SettingUp:
            if (step_ == 0 || nowMs - lastMs_ >= setup_[step_ - 1].settleMs) {
                command = setup_[step_].command.c_str();
            }
            break;
        case State::Listening:
            if (tx_ == Tx::StopListening) {
                command = kStopListening;
            } else if (tx_ == Tx::Send) {
                command = txCommand_.c_str();
            } else if (tx_ == Tx::Listen) {
                command = kListen;
            } else if (nowMs - lastMs_ >= kListeningProbeMs) {
                command = kProbe;
            }
            break;
    }
    if (command != nullptr) {
        awaiting_ = true;
        sentMs_ = nowMs;
    }
    return command;
}

void AtLink::onLine(const std::string& line, uint32_t nowMs) {
    LoraPacket packet;
    if (state_ == State::Listening) {
        lastMs_ = nowMs;
        if (parsePacket(line, packet)) {
            if (packets_.size() == kMaxPackets) {
                packets_.pop_front();
            }
            packets_.push_back(std::move(packet));
            return;
        }
    }
    if (tx_ == Tx::AwaitDone && line == kSent) {
        tx_ = Tx::Listen;
        return;
    }
    if (!awaiting_) {
        return;
    }
    if (line == "OK") {
        awaiting_ = false;
        missed_ = 0;
        lastMs_ = nowMs;
        if (tx_ == Tx::StopListening) {
            tx_ = Tx::Send;
        } else if (tx_ == Tx::Send) {
            tx_ = Tx::AwaitDone;
            sentMs_ = nowMs;
        } else if (tx_ == Tx::Listen) {
            tx_ = Tx::Idle;
            sendResult_ = SendResult::Sent;
        } else if (state_ == State::Absent) {
            state_ = State::SettingUp;
            step_ = 0;
        } else if (state_ == State::SettingUp && ++step_ == setup_.size()) {
            state_ = State::Listening;
        }
    } else if (line.compare(0, 3, "AT_") == 0 && (state_ == State::SettingUp || tx_ != Tx::Idle)) {
        // RUI answers a refused command with AT_ERROR, AT_PARAM_ERROR, AT_BUSY_ERROR and the like.
        awaiting_ = false;
        becomeAbsent(nowMs);
    }
}

bool AtLink::takePacket(LoraPacket& packet) {
    if (packets_.empty()) {
        return false;
    }
    packet = std::move(packets_.front());
    packets_.pop_front();
    return true;
}

bool AtLink::send(const std::string& hex) {
    if (state_ != State::Listening || tx_ != Tx::Idle) {
        return false;
    }
    txCommand_ = "AT+PSEND=" + hex;
    tx_ = Tx::StopListening;
    return true;
}

SendResult AtLink::takeSendResult() {
    const SendResult result = sendResult_;
    sendResult_ = SendResult::None;
    return result;
}

// A unit that fails mid-send is found and set up again, which resumes listening.
void AtLink::becomeAbsent(uint32_t nowMs) {
    if (tx_ != Tx::Idle) {
        tx_ = Tx::Idle;
        sendResult_ = SendResult::Failed;
    }
    state_ = State::Absent;
    missed_ = 0;
    probed_ = true;
    lastMs_ = nowMs;
    packets_.clear();
}
