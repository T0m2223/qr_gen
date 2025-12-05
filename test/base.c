#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test/base.h>

typedef struct test_node
{
	const char *name;
	int is_preparation;
	struct test_result (*fn)(void);
	struct test_result res;
	struct test_node *next;
} test_node;

typedef struct group_node
{
	const char *name;
	test_node *tests;
	size_t count;
	struct group_node *next;
} group_node;

static group_node *group_head = NULL;
static size_t biggest_group_name = 0;

test_node *
append_test(test_node **head, const char *name, struct test_result (*fn)(void))
{
	test_node *new_test, *current;

	new_test = test_malloc(sizeof(test_node));
	new_test->name = name;
	new_test->is_preparation = 0;
	new_test->fn = fn;
	new_test->res = TEST_SUCCESS;
	new_test->next = NULL;

	if (!*head)
	{
		*head = new_test;
	}
	else
	{
		for (current = *head; current->next; current = current->next);
		current->next = new_test;
	}

	return new_test;
}

test_node *
prepend_preparation(test_node **head, const char *name, struct test_result (*fn)(void))
{
	test_node *new_test;

	new_test = test_malloc(sizeof(test_node));
	new_test->name = name;
	new_test->is_preparation = 1;
	new_test->fn = fn;
	new_test->res = TEST_SUCCESS;
	new_test->next = *head;

	*head = new_test;

	return new_test;
}

group_node *
append_group(group_node **head, const char *name)
{
	group_node *current;

	group_node *new_group = test_malloc(sizeof(group_node));
	new_group->name = name;
	new_group->tests = NULL;
	new_group->count = 0;
	new_group->next = NULL;

	if (!*head)
	{
		*head = new_group;
	}
	else
	{
		for (current = *head; current->next; current = current->next);
		current->next = new_group;
	}

	if (strlen(new_group->name) > biggest_group_name)
		biggest_group_name = strlen(new_group->name);

	return new_group;
}

group_node *
group_exists(group_node **head, const char *name)
{
	group_node *current;

	for (current = *head; current; current = current->next)
		if (!strcmp(current->name, name))
			return current;

	return NULL;
}

void
test_register_case(const char *group_name, const char *test_name, struct test_result (*fn)(void))
{
	group_node *group;

	if (!(group = group_exists(&group_head, group_name)))
		group = append_group(&group_head, group_name);

	append_test(&group->tests, test_name, fn);
	++group->count;
}

void
test_register_setup(const char *group_name, const char *test_name, struct test_result (*fn)(void))
{
	group_node *group;

	if (!(group = group_exists(&group_head, group_name)))
		group = append_group(&group_head, group_name);

	prepend_preparation(&group->tests, test_name, fn);
}

typedef struct memory_node
{
	void *ptr;
	struct memory_node *next;
} memory_node;

memory_node *memory_head = NULL;

void *
test_malloc(size_t size)
{
	void *ptr = malloc(size);
	memory_node *new_memory = malloc(sizeof(memory_node));
	new_memory->ptr = ptr;
	new_memory->next = memory_head;
	memory_head = new_memory;
	return ptr;
}

void
test_free(void)
{
	memory_node *current, *next;

	for (current = memory_head; current; current = next)
	{
		free(current->ptr);
		next = current->next;
		free(current);
	}

	memory_head = NULL;
}

#define BAR_WIDTH 40

size_t
run_test_group(group_node *group)
{
	size_t total = 0, failures = 0, i;

	printf("Test group %s:\n", group->name);
	for (test_node *current_test = group->tests; current_test; current_test = current_test->next)
	{
		if (current_test->is_preparation)
		{
			current_test->res = current_test->fn();
			continue;
		}

		printf("  => %s (%zu/%zu)", current_test->name, total + 1, group->count);
		fflush(stdout);
		++total;
		current_test->res = current_test->fn();
		printf("\r\x1b[K");

		if (!current_test->res.failed) continue;
		++failures;
		printf("  => %s:%zu [%s] failed:\n    => %s\n", current_test->name, current_test->res.line, current_test->name, current_test->res.message);
	}

	for (i = 0; i < (2 * failures) + 1; ++i)
		printf("\x1b[K\x1b[A");

	printf("Test group %s%-*s %3zu test(s) ran; %3zu failed. ", group->name, ((int) biggest_group_name) - ((int) strlen(group->name)) + 1, ":", total, failures);

	printf("[\x1b[32m");
	for (i = 0; i < ((total - failures) / (float) total) * BAR_WIDTH; ++i)
		printf("-");
	printf("\x1b[31m");
	for (; i < BAR_WIDTH; ++i)
		printf("-");
	printf("\x1b[0m]\n");

	return failures;
}

int
main(void)
{
	group_node *current_group;
	test_node *current_test;
	size_t total_failures = 0, total_tests = 0;

	for (current_group = group_head; current_group; current_group = current_group->next)
		total_tests += current_group->count;
	for (current_group = group_head; current_group; current_group = current_group->next)
		total_failures += run_test_group(current_group);

	printf("\nSummary: %zu test(s) ran; %zu failed.\n", total_tests, total_failures);
	for (current_group = group_head; current_group; current_group = current_group->next)
	{
		for (current_test = current_group->tests; current_test; current_test = current_test->next)
		{
			if (!current_test->res.failed) continue;
			printf("  => %s:%zu [%s] failed:\n    => %s\n", current_group->name, current_test->res.line, current_test->name, current_test->res.message);
		}
	}

	test_free();
	return total_failures;
}
