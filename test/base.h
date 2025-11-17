#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct test_result
{
	int failed;
	const char *message;
	size_t line;
};
void test_register(const char *group, const char *name, struct test_result (*fn)(void));
void before_register(const char *group, const char *name, struct test_result (*fn)(void));

#define TEST(test_id) \
static struct test_result __test_## test_id(void); \
__attribute__((constructor)) static void __testregister_## test_id(void) \
{ \
	test_register(__FILE__, #test_id, __test_## test_id); \
} \
static struct test_result __test_## test_id(void)

#define BEFORE() \
static struct test_result __before_all(void); \
__attribute__((constructor)) static void __beforeregister(void) \
{ \
	before_register(__FILE__, "BEFORE", __before_all); \
} \
static struct test_result __before_all(void)

void *test_malloc(size_t size);

#define INT_MAX_CHARS 11

#define TEST_SUCCESS (struct test_result) {0, NULL, 0}
#define TEST_FAILURE(message) (struct test_result) {1, message, __LINE__}

#define assert_base(lhs, rhs, message, operator) \
	do { \
		int __assert_lhs = (lhs); \
		int __assert_rhs = (rhs); \
		if (!(__assert_lhs operator __assert_rhs)) \
		{ \
			size_t __assert_length = \
				strlen(message) + \
				strlen(": %d " #operator " %d") + (2 * INT_MAX_CHARS) + 1; \
			char *__assert_message = test_malloc(__assert_length); \
			if (__assert_message == NULL) return TEST_FAILURE("assert: test_malloc failed"); \
			snprintf(__assert_message, __assert_length, "%s: %d " #operator " %d", message, __assert_lhs, __assert_rhs); \
			return TEST_FAILURE(__assert_message); \
		} \
	} while (0)

#define assert_eq(lhs, rhs, message) assert_base(lhs, rhs, message, ==)
#define assert_neq(lhs, rhs, message) assert_base(lhs, rhs, message, !=)
#define assert_lt(lhs, rhs, message) assert_base(lhs, rhs, message, <)
#define assert_gt(lhs, rhs, message) assert_base(lhs, rhs, message, >)
#define assert_lte(lhs, rhs, message) assert_base(lhs, rhs, message, <=)
#define assert_gte(lhs, rhs, message) assert_base(lhs, rhs, message, >=)

#endif // TEST_BASE_H
