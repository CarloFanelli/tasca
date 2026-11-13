#include <unity.h>

#include "core/incoming_call.h"

void setUp() {}
void tearDown() {}

static void test_no_call_rings_at_start() {
    CallTracker calls;
    TEST_ASSERT_NULL(calls.shown());
}

static void test_ringing_call_is_shown_with_its_actions() {
    CallTracker calls;
    calls.ring(7, true, false);
    const Call* call = calls.shown();
    TEST_ASSERT_NOT_NULL(call);
    TEST_ASSERT_EQUAL_UINT32(7, call->uid);
    TEST_ASSERT_TRUE(call->positiveAction);
    TEST_ASSERT_FALSE(call->negativeAction);
    TEST_ASSERT_TRUE(calls.isCall(7));
    TEST_ASSERT_FALSE(calls.isCall(8));
}

static void test_caller_is_empty_until_the_title_arrives() {
    CallTracker calls;
    calls.ring(7, true, true);
    TEST_ASSERT_EQUAL_STRING("", callerName(*calls.shown()).c_str());
    TEST_ASSERT_TRUE(calls.setTitle(7, "Anna"));
    TEST_ASSERT_EQUAL_STRING("Anna", callerName(*calls.shown()).c_str());
}

static void test_empty_or_missing_title_is_unknown_caller() {
    CallTracker calls;
    calls.ring(7, true, true);
    calls.setTitle(7, "");
    TEST_ASSERT_EQUAL_STRING("Unknown caller", callerName(*calls.shown()).c_str());
    calls.ring(8, true, true);
    TEST_ASSERT_TRUE(calls.titleFailed(8));
    TEST_ASSERT_EQUAL_STRING("Unknown caller", callerName(*calls.shown()).c_str());
}

static void test_title_of_a_call_no_longer_ringing_is_ignored() {
    CallTracker calls;
    TEST_ASSERT_FALSE(calls.setTitle(7, "Anna"));
    TEST_ASSERT_FALSE(calls.titleFailed(7));
    TEST_ASSERT_NULL(calls.shown());
}

static void test_newest_call_is_shown_and_the_older_comes_back_when_it_ends() {
    CallTracker calls;
    calls.ring(7, true, true);
    calls.ring(9, true, true);
    TEST_ASSERT_EQUAL_UINT32(9, calls.shown()->uid);
    TEST_ASSERT_TRUE(calls.end(9));
    TEST_ASSERT_EQUAL_UINT32(7, calls.shown()->uid);
    TEST_ASSERT_TRUE(calls.end(7));
    TEST_ASSERT_NULL(calls.shown());
}

static void test_repeated_event_updates_the_call_without_a_second_entry() {
    CallTracker calls;
    calls.ring(7, true, true);
    calls.setTitle(7, "Anna");
    calls.ring(7, false, true);
    TEST_ASSERT_FALSE(calls.shown()->positiveAction);
    TEST_ASSERT_EQUAL_STRING("Anna", callerName(*calls.shown()).c_str());
    TEST_ASSERT_TRUE(calls.end(7));
    TEST_ASSERT_NULL(calls.shown());
}

static void test_end_of_an_unknown_uid_changes_nothing() {
    CallTracker calls;
    calls.ring(7, true, true);
    TEST_ASSERT_FALSE(calls.end(8));
    TEST_ASSERT_EQUAL_UINT32(7, calls.shown()->uid);
}

static void test_clear_ends_every_call() {
    CallTracker calls;
    calls.ring(7, true, true);
    calls.ring(9, true, true);
    calls.clear();
    TEST_ASSERT_NULL(calls.shown());
}

static void test_action_is_allowed_only_when_the_notification_offers_it() {
    CallTracker calls;
    calls.ring(7, true, false);
    TEST_ASSERT_TRUE(canAct(*calls.shown(), true));
    TEST_ASSERT_FALSE(canAct(*calls.shown(), false));
}

static void test_one_action_per_call() {
    CallTracker calls;
    calls.ring(7, true, true);
    TEST_ASSERT_TRUE(calls.actionSent(7));
    TEST_ASSERT_FALSE(canAct(*calls.shown(), true));
    TEST_ASSERT_FALSE(canAct(*calls.shown(), false));
    TEST_ASSERT_FALSE(calls.shown()->actionFailed);
}

static void test_failed_action_is_kept_until_the_call_ends() {
    CallTracker calls;
    calls.ring(7, true, true);
    calls.actionSent(7);
    TEST_ASSERT_TRUE(calls.actionFailed(7));
    TEST_ASSERT_TRUE(calls.shown()->actionFailed);
    TEST_ASSERT_FALSE(canAct(*calls.shown(), false));
    calls.ring(7, true, true);
    TEST_ASSERT_TRUE(calls.shown()->actionFailed);
    TEST_ASSERT_FALSE(calls.actionSent(8));
    TEST_ASSERT_FALSE(calls.actionFailed(8));
}

static void test_ring_is_due_at_start_then_every_period() {
    RingTimer ring;
    TEST_ASSERT_FALSE(ring.due(0));
    ring.start(1000);
    TEST_ASSERT_TRUE(ring.due(1000));
    TEST_ASSERT_FALSE(ring.due(1000 + RingTimer::kPeriodMs - 1));
    TEST_ASSERT_TRUE(ring.due(1000 + RingTimer::kPeriodMs));
    TEST_ASSERT_FALSE(ring.due(1000 + RingTimer::kPeriodMs + 10));
}

static void test_late_check_rings_once_then_waits_a_full_period() {
    RingTimer ring;
    ring.start(0);
    ring.due(0);
    TEST_ASSERT_TRUE(ring.due(5 * RingTimer::kPeriodMs));
    TEST_ASSERT_FALSE(ring.due(5 * RingTimer::kPeriodMs + 1));
}

static void test_stopped_ring_is_never_due_and_wraps_around() {
    RingTimer ring;
    ring.start(0xFFFFFF00);
    TEST_ASSERT_TRUE(ring.due(0xFFFFFF00));
    TEST_ASSERT_TRUE(ring.due(0xFFFFFF00 + RingTimer::kPeriodMs));
    ring.stop();
    TEST_ASSERT_FALSE(ring.due(0xFFFFFF00 + 10 * RingTimer::kPeriodMs));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_no_call_rings_at_start);
    RUN_TEST(test_ringing_call_is_shown_with_its_actions);
    RUN_TEST(test_caller_is_empty_until_the_title_arrives);
    RUN_TEST(test_empty_or_missing_title_is_unknown_caller);
    RUN_TEST(test_title_of_a_call_no_longer_ringing_is_ignored);
    RUN_TEST(test_newest_call_is_shown_and_the_older_comes_back_when_it_ends);
    RUN_TEST(test_repeated_event_updates_the_call_without_a_second_entry);
    RUN_TEST(test_end_of_an_unknown_uid_changes_nothing);
    RUN_TEST(test_clear_ends_every_call);
    RUN_TEST(test_action_is_allowed_only_when_the_notification_offers_it);
    RUN_TEST(test_one_action_per_call);
    RUN_TEST(test_failed_action_is_kept_until_the_call_ends);
    RUN_TEST(test_ring_is_due_at_start_then_every_period);
    RUN_TEST(test_late_check_rings_once_then_waits_a_full_period);
    RUN_TEST(test_stopped_ring_is_never_due_and_wraps_around);
    return UNITY_END();
}
