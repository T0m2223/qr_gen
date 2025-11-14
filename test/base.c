#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test/base.h>

typedef struct test_node
{
	const char *name;
	int (*fn)(void);
	struct test_node *next;
	int res;
	int is_preparation;
} test_node;

typedef struct group_node
{
	const char *name;
	test_node *tests;
	size_t count;
	struct group_node *next;
} group_node;

static group_node *group_head;
static size_t biggest_group_name = 0;

test_node *
append_test(test_node **head, const char *name, int (*fn)(void))
{
	test_node *new_test, *current;

	new_test = malloc(sizeof(test_node));
	new_test->name = name;
	new_test->fn = fn;
	new_test->next = NULL;
	new_test->res = 0;
	new_test->is_preparation = 0;

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
prepend_preparation(test_node **head, const char *name, int (*fn)(void))
{
	test_node *new_test;

	new_test = malloc(sizeof(test_node));
	new_test->name = name;
	new_test->fn = fn;
	new_test->next = *head;
	new_test->res = 0;
	new_test->is_preparation = 1;

	*head = new_test;

	return new_test;
}

group_node *
append_group(group_node **head, const char *name)
{
	group_node *current;

	group_node *new_group = malloc(sizeof(group_node));
	new_group->name = name;
	new_group->next = NULL;
	new_group->tests = NULL;
	new_group->count = 0;

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
test_register(const char *group_name, const char *test_name, int (*fn)(void))
{
	group_node *group;

	if (!(group = group_exists(&group_head, group_name)))
		group = append_group(&group_head, group_name);

	append_test(&group->tests, test_name, fn);
	++group->count;
}

void
before_register(const char *group_name, const char *test_name, int (*fn)(void))
{
	group_node *group;

	if (!(group = group_exists(&group_head, group_name)))
		group = append_group(&group_head, group_name);

	prepend_preparation(&group->tests, test_name, fn);
}

#define BAR_WIDTH 40

size_t
run_test_group(group_node *group)
{
	size_t total, failures, i;
	total = failures = 0;
	printf("Test group %s:\n", group->name);
	for (test_node *current_test = group->tests; current_test; current_test = current_test->next)
	{
		if (current_test->is_preparation)
		{
			current_test->res = current_test->fn();
			continue;
		}

		printf("=> '%s' (%zu/%zu)", current_test->name, total, group->count);
		fflush(stdout);
		++total;
		current_test->res = current_test->fn();
		printf("\r\x1b[K");

		if (!current_test->res) continue;
		++failures;
		printf("  => '%s' failed: %d\n", current_test->name, current_test->res);
	}

	for (i = 0; i < failures + 1; ++i)
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
	size_t total_failures = 0;

	for (current_group = group_head; current_group; current_group = current_group->next)
		total_failures += run_test_group(current_group);
	for (current_group = group_head; current_group; current_group = current_group->next)
	{
		for (current_test = current_group->tests; current_test; current_test = current_test->next)
		{
			if (!current_test->res)
				continue;

			printf("  => '%s [%s]' failed: %d\n", current_group->name, current_test->name, current_test->res);
		}
	}

	return total_failures;
}
