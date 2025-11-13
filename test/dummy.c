#include <test/base.h>

/**
 * Basic sanity check test to verify the test framework is working.
 * This is a simple test that verifies basic arithmetic works as expected.
 * 
 * @return 0 if the test passes, 1 if it fails
 */
TEST(sanity_addition)
{
    return 3 + 4 == 7 ? 0 : 1;
}
