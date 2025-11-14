#ifndef TEST_BASE_H
#define TEST_BASE_H

void test_register(const char *group, const char *name, int (*fn)(void));
void before_register(const char *group, const char *name, int (*fn)(void));

#define TEST(test_id) \
static int _test_## test_id(void); \
__attribute__((constructor)) static void _testregister_## test_id(void) \
{ \
	test_register(__FILE__, #test_id, _test_## test_id); \
} \
static int _test_## test_id(void)

#define BEFORE() \
static int _before_all(void); \
__attribute__((constructor)) static void _beforeregister(void) \
{ \
	before_register(__FILE__, "BEFORE", _before_all); \
} \
static int _before_all(void)

#endif // TEST_BASE_H
