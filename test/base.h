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
static struct test_result __test_##test_id(void); \
__attribute__((constructor)) static void __testregister_##test_id(void) \
{ \
	test_register(__FILE__, #test_id, __test_##test_id); \
} \
static struct test_result __test_##test_id(void)

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
#define TEST_FAILURE(message) (struct test_result) {1, (message), __LINE__}

#define test_expect_base(lhs, rhs, message, operator) \
	do { \
		int __test_expect_lhs = (lhs); \
		int __test_expect_rhs = (rhs); \
		if (!(__test_expect_lhs operator __test_expect_rhs)) \
		{ \
			size_t __test_expect_length = \
				strlen(message) + \
				strlen(":  " #operator " ") + \
				(2 * INT_MAX_CHARS) + 1; \
			char *__test_expect_message = test_malloc(__test_expect_length); \
			if (__test_expect_message == NULL) return TEST_FAILURE("test_expect: test_malloc failed"); \
			snprintf(__test_expect_message, __test_expect_length, "%s: %d " #operator " %d", message, __test_expect_lhs, __test_expect_rhs); \
			return TEST_FAILURE(__test_expect_message); \
		} \
	} while (0)

#define test_expect_eq(lhs, rhs, message) test_expect_base(lhs, rhs, message, ==)
#define test_expect_ne(lhs, rhs, message) test_expect_base(lhs, rhs, message, !=)
#define test_expect_lt(lhs, rhs, message) test_expect_base(lhs, rhs, message, <)
#define test_expect_gt(lhs, rhs, message) test_expect_base(lhs, rhs, message, >)
#define test_expect_le(lhs, rhs, message) test_expect_base(lhs, rhs, message, <=)
#define test_expect_ge(lhs, rhs, message) test_expect_base(lhs, rhs, message, >=)

#endif // TEST_BASE_H
