#include <unity.h>

#include "core/reconnect_import.h"

void setUp() {}
void tearDown() {}

static void test_not_started_is_never_done() {
    ReconnectImport import;
    TEST_ASSERT_FALSE(import.finished(100000, 0));
}

static void test_done_once_requests_answered_and_events_quiet() {
    ReconnectImport import;
    import.start(1000);
    import.onPreExistingEvent(1200);
    TEST_ASSERT_FALSE(import.finished(2000, 3));
    TEST_ASSERT_FALSE(import.finished(3000, 0));
    TEST_ASSERT_TRUE(import.finished(3200, 0));
    TEST_ASSERT_FALSE(import.finished(9000, 0));
}

static void test_no_preexisting_notification_finishes_after_the_quiet_time() {
    ReconnectImport import;
    import.start(1000);
    TEST_ASSERT_FALSE(import.finished(2999, 0));
    TEST_ASSERT_TRUE(import.finished(3000, 0));
}

static void test_failure_finishes_without_cleanup() {
    ReconnectImport import;
    import.start(1000);
    import.onFailure();
    TEST_ASSERT_TRUE(import.finished(5000, 0));
    TEST_ASSERT_FALSE(import.complete());
}

static void test_clean_import_is_complete() {
    ReconnectImport import;
    import.start(1000);
    TEST_ASSERT_TRUE(import.finished(5000, 0));
    TEST_ASSERT_TRUE(import.complete());
}

static void test_cancel_stops_the_import() {
    ReconnectImport import;
    import.start(1000);
    import.cancel();
    TEST_ASSERT_FALSE(import.finished(5000, 0));
}

static void test_restart_clears_a_previous_failure() {
    ReconnectImport import;
    import.start(1000);
    import.onFailure();
    import.cancel();
    import.start(10000);
    TEST_ASSERT_TRUE(import.finished(13000, 0));
    TEST_ASSERT_TRUE(import.complete());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_not_started_is_never_done);
    RUN_TEST(test_done_once_requests_answered_and_events_quiet);
    RUN_TEST(test_no_preexisting_notification_finishes_after_the_quiet_time);
    RUN_TEST(test_failure_finishes_without_cleanup);
    RUN_TEST(test_clean_import_is_complete);
    RUN_TEST(test_cancel_stops_the_import);
    RUN_TEST(test_restart_clears_a_previous_failure);
    return UNITY_END();
}
