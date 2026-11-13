#pragma once

#include <stdint.h>

#include <string>
#include <vector>

// One iPhone call ringing, from its incoming-call notification.
struct Call {
    uint32_t uid;
    bool positiveAction;
    bool negativeAction;
    // The notification title: the caller's name, or the number when unknown.
    std::string title;
    // The title arrived, or its request failed and will not arrive.
    bool titleDone = false;
    // Answer or decline sent: no other action until the call ends.
    bool actionSent = false;
    // The iPhone refused the action.
    bool actionFailed = false;
};

// True when the positive (answer) or negative (decline) action of call can be sent now.
bool canAct(const Call& call, bool positive);

// The caller as shown: the title; "Unknown caller" when it is empty or never arrived; empty while
// it is still awaited.
std::string callerName(const Call& call);

// The calls ringing on the iPhone, newest last. Several ring at once with call waiting.
class CallTracker {
public:
    // An incoming-call notification, live or already ringing at connection; a repeated uid updates
    // its actions.
    void ring(uint32_t uid, bool positiveAction, bool negativeAction);
    // The attributes of call uid arrived; false when it rings no more.
    bool setTitle(uint32_t uid, const std::string& title);
    // The attributes of call uid will not arrive; false when it rings no more.
    bool titleFailed(uint32_t uid);
    // An action of call uid was sent; false when it rings no more.
    bool actionSent(uint32_t uid);
    // The iPhone refused the action of call uid; false when it rings no more.
    bool actionFailed(uint32_t uid);
    // Call uid stopped ringing (answered, declined, ended); false when it was not ringing.
    bool end(uint32_t uid);
    // The connection ended: no call rings any more.
    void clear() { calls_.clear(); }

    bool isCall(uint32_t uid) const;
    // The newest call ringing; nullptr when none.
    const Call* shown() const { return calls_.empty() ? nullptr : &calls_.back(); }

private:
    Call* find(uint32_t uid);

    std::vector<Call> calls_;
};

// The ring of the Call screen: due when started, then every kPeriodMs. Times come from a
// millisecond clock that may wrap around.
class RingTimer {
public:
    static constexpr uint32_t kPeriodMs = 2000;

    void start(uint32_t nowMs) {
        active_ = true;
        nextMs_ = nowMs;
    }
    void stop() { active_ = false; }
    // True when a ring is due at nowMs; the next one is a full period later.
    bool due(uint32_t nowMs);

private:
    bool active_ = false;
    uint32_t nextMs_ = 0;
};
