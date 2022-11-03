#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                 \
	do                                      \
	{                                       \
		printf("ASSERT: " #assert " ... "); \
		if (assert)                         \
		{                                   \
			printf("PASS\n");               \
		}                                   \
		else                                \
		{                                   \
			printf("FAIL\n");               \
			exit(1);                        \
		}                                   \
	} while (0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void **)&ptr);
	TEST_ASSERT(ptr == &data);
}

/* Iterate Queue */
static void iterator_inc(queue_t q, void *data)
{
	int *a = (int *)data;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += 1;
}

void test_iterator(void)
{
	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	size_t i;

	fprintf(stderr, "*** TEST test_iterator ***\n");

	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Increment every item of the queue, delete item '42' */
	queue_iterate(q, iterator_inc);

	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(queue_length(q) == 9);
}

/* Enqueue item into a Null queue */
void test_enqueue_null(void)
{
	queue_t q = NULL;
	int data = 6;

	int return_value;

	fprintf(stderr, "*** TEST test_enqueue_null ***\n");

	return_value = queue_enqueue(q, &data);
	TEST_ASSERT(return_value == -1);
}

/* Helper function for checking if delete works */
static void delete_helper(queue_t q, void *data)
{
	int *num_check = (int *)data;

	if (*num_check == 4)
	{
		queue_delete(q, data);
	}
	else
	{
		;
	}
}

/* Delete one item */
void test_delete_one_data(void)
{
	queue_t q;
	int return_value;
	int data[] = {1, 3, 4, 5, 6, 10};

	fprintf(stderr, "*** TEST test_delete_one_data ***\n");

	q = queue_create();

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		queue_enqueue(q, &data[i]);
	}

	return_value = queue_delete(q, &data[4]);

	TEST_ASSERT(return_value == 0);
	TEST_ASSERT(queue_length(q) == 5);
}

/* Check if able to delete multiple nodes in the middle of queue successfully */
void test_delete_multiple_data_middle(void)
{
	queue_t q;
	int data[] = {3, 4, 5, 6, 10, 7, 4, 9};

	fprintf(stderr, "*** TEST test_delete_multiple_dat_middle ***\n");

	q = queue_create();

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		queue_enqueue(q, &data[i]);
	}

	queue_iterate(q, delete_helper);

	TEST_ASSERT(queue_length(q) == 6);
}

/* Check if able to delete multiple nodes with data at the front of the queue */
void test_delete_multiple_data_front(void)
{
	queue_t q;
	int data[] = {4, 4, 5, 6, 10, 7, 4, 3};

	fprintf(stderr, "*** TEST test_delete_multiple_data_front ***\n");

	q = queue_create();

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		queue_enqueue(q, &data[i]);
	}

	queue_iterate(q, delete_helper);

	TEST_ASSERT(queue_length(q) == 5);
}

/* Check if able to delete multiple nodes with data at the rear of the queue */
void test_delete_multiple_data_rear(void)
{
	queue_t q;
	int data[] = {6, 4, 5, 6, 10, 7, 4, 4};

	fprintf(stderr, "*** TEST test_delete_multiple_data_rear ***\n");

	q = queue_create();

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		queue_enqueue(q, &data[i]);
	}

	queue_iterate(q, delete_helper);

	TEST_ASSERT(queue_length(q) == 5);
}

/* Delete an item that doesn't exit in the queue */
void test_delete_not_in_queue(void)
{
	queue_t q;
	int data[] = {1, 6, 5, 3, 7};
	int return_value;
	int num_check = 4;

	q = queue_create();

	fprintf(stderr, "*** TEST test_delete_not_in_queue ***\n");

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		queue_enqueue(q, &data[i]);
	}

	return_value = queue_delete(q, &num_check);
	TEST_ASSERT(return_value == -1);
}

/* Dequeue an empty queue*/
void dequeue_queue_empty(void)
{
	queue_t q;
	int *ptr, result;

	fprintf(stderr, "*** TEST dequeue_queue_empty ***\n");

	q = queue_create();
	result = queue_dequeue(q, (void **)&ptr);
	TEST_ASSERT(result == -1);
}

/* Delete a NULL item from queue */
void delete_invalid_item(void)
{
	queue_t q;
	int *num = NULL;
	int result;
	int data[] = {1, 2, 3, 4, 5};

	fprintf(stderr, "*** TEST delete_invalid_item ***\n");

	q = queue_create();
	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	result = queue_delete(q,num);
	TEST_ASSERT(result == -1);
}

/* Destroy a non-empty queue */
void destroy_nonempty_queue(void)
{
	queue_t q;
	int result;
	int data[] = {1, 2, 3, 4, 5};

	fprintf(stderr, "*** TEST destroy_nonempty_queue ***\n");

	q = queue_create();
	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	result = queue_destroy(q);
	TEST_ASSERT(result == -1);
}

/* Destroy a null queue */
void destroy_null_queue(void)
{
	int result;
	queue_t q = NULL;

	fprintf(stderr, "*** TEST destroy_null_queue ***\n");

	result = queue_destroy(q);
	TEST_ASSERT(result == -1);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_iterator();
	test_enqueue_null();
	test_delete_one_data();
	test_delete_multiple_data_middle();
	test_delete_multiple_data_front();
	test_delete_multiple_data_rear();
	test_delete_not_in_queue();
	dequeue_queue_empty();
	delete_invalid_item();
	destroy_nonempty_queue();
	destroy_null_queue();

	return 0;
}
