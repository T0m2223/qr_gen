#include <test/base.h>

/**
 * Basic sanity check test to verify the test framework is working.
 * This is a simple test that verifies basic arithmetic works as expected.
 */
TEST(sanity_addition)
{
	assert_eq(3 + 4, 7, "Basic addition failed");
	return TEST_SUCCESS;
}
