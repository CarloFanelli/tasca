#include <unity.h>

#include "core/mesh_name.h"

void setUp() {}
void tearDown() {}

static void test_no_name_chosen_gives_tasca_and_4_node_id_digits() {
    TEST_ASSERT_EQUAL_STRING("Tasca9f8d", meshLongName("", 0x84969f8d).c_str());
    TEST_ASSERT_EQUAL_STRING("Carlo", meshLongName("Carlo", 0x84969f8d).c_str());
}

static void test_short_name_is_the_first_4_characters() {
    TEST_ASSERT_EQUAL_STRING("Carl", meshShortName("Carlo").c_str());
    TEST_ASSERT_EQUAL_STRING("Ed", meshShortName("Ed").c_str());
}

// Meshtastic nodes refuse an announcement whose short name passes 4 bytes or long name 39.
static void test_names_stay_within_meshtastic_byte_limits() {
    TEST_ASSERT_EQUAL_STRING("N\xC3\xA8", meshShortName("N\xC3\xA8\xC3\xA8\xC3\xA8").c_str());
    std::string accented;
    for (int i = 0; i < 24; i++) {
        accented += "\xC3\xA8";
    }
    TEST_ASSERT_EQUAL(38, meshLongName(accented, 1).size());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_no_name_chosen_gives_tasca_and_4_node_id_digits);
    RUN_TEST(test_short_name_is_the_first_4_characters);
    RUN_TEST(test_names_stay_within_meshtastic_byte_limits);
    return UNITY_END();
}
