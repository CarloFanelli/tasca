#include "core/incoming_call.h"

std::string callerName(const Call& call) {
    if (!call.title.empty()) {
        return call.title;
    }
    return call.titleDone ? "Unknown caller" : "";
}

bool canAct(const Call& call, bool positive) {
    return !call.actionSent && (positive ? call.positiveAction : call.negativeAction);
}

void CallTracker::ring(uint32_t uid, bool positiveAction, bool negativeAction) {
    Call* call = find(uid);
    if (call == nullptr) {
        calls_.push_back({uid, positiveAction, negativeAction, "", false, false, false});
        return;
    }
    call->positiveAction = positiveAction;
    call->negativeAction = negativeAction;
}

bool CallTracker::setTitle(uint32_t uid, const std::string& title) {
    Call* call = find(uid);
    if (call == nullptr) {
        return false;
    }
    call->title = title;
    call->titleDone = true;
    return true;
}

bool CallTracker::titleFailed(uint32_t uid) {
    Call* call = find(uid);
    if (call == nullptr) {
        return false;
    }
    call->titleDone = true;
    return true;
}

bool CallTracker::actionSent(uint32_t uid) {
    Call* call = find(uid);
    if (call == nullptr) {
        return false;
    }
    call->actionSent = true;
    return true;
}

bool CallTracker::actionFailed(uint32_t uid) {
    Call* call = find(uid);
    if (call == nullptr) {
        return false;
    }
    call->actionFailed = true;
    return true;
}

bool CallTracker::end(uint32_t uid) {
    for (auto it = calls_.begin(); it != calls_.end(); ++it) {
        if (it->uid == uid) {
            calls_.erase(it);
            return true;
        }
    }
    return false;
}

bool CallTracker::isCall(uint32_t uid) const {
    for (const Call& call : calls_) {
        if (call.uid == uid) {
            return true;
        }
    }
    return false;
}

Call* CallTracker::find(uint32_t uid) {
    for (Call& call : calls_) {
        if (call.uid == uid) {
            return &call;
        }
    }
    return nullptr;
}

bool RingTimer::due(uint32_t nowMs) {
    if (!active_ || static_cast<int32_t>(nowMs - nextMs_) < 0) {
        return false;
    }
    nextMs_ = nowMs + kPeriodMs;
    return true;
}
