#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue
{
	int len;
	struct node *front;
	struct node *rear;
};

struct node
{
	void *data;
	struct node *next;
	struct node *prev;
} node;

queue_t queue_create(void)
{
	// malloc queue
	queue_t queue;
	queue = malloc(sizeof(queue_t));

	if (!queue)
	{
		return NULL;
	}
	else
	{
		queue->len = 0;
		queue->front = NULL;
		queue->rear = NULL;
		return queue;
	}
}

int queue_destroy(queue_t queue)
{
	if (!queue || queue->len > 0)
	{
		return -1;
	}

	/* Free from memory */
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	/* Allocate memory for a new node */
	/* The newest node will always be the rear */
	struct node *info_node;

	info_node = malloc(sizeof(struct node));

	if (!queue || !data || !info_node)
	{
		return -1;
	}

	info_node->prev = NULL;
	info_node->next = NULL;
	info_node->data = data;

	/* 2 cases:
	1. If the queue is being enqueued into for the first time, then both the front and the rear should be set to this new node
	2. If the queue already has elements, the queue's rear will be set to the new element */
	if (queue->len == 0)
	{
		queue->front = info_node;
		queue->rear = info_node;
	}
	else
	{
		/* Pointer to the previous node */
		info_node->prev = queue->rear;
		queue->rear->next = info_node;
		queue->rear = info_node;
	}
	queue->len++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (!queue || !data || queue->len == 0)
	{
		return -1;
	}

	/* Point to the data in the queue head */
	*data = queue->front->data;

	/* 2 cases:
	1. There is only one item left
	2. There is more than 1 item left
	We set the front to the linked list element to the next element
	*/
	if (queue->len == 1)
	{
		queue->front = NULL;
		queue->rear = NULL;
	}
	else
	{
		queue->front = queue->front->next;
	}
	queue->len--;

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (!queue || !data)
	{
		return -1;
	}

	/* We will look for the data, pointing to next, until we can find the data */
	struct node *currentNode;
	int retval = 0; // Default value of retval set to 0, will only need to change if we reach the end with nothing
	currentNode = malloc(sizeof(struct node));
	currentNode = queue->front;

	/* Iterate through queue to find data */
	while (currentNode->data != data)
	{
		currentNode = currentNode->next;
		if (currentNode == queue->rear)
		{
			retval = -1;
			break;
		}
	}

	/* Set data to null */
	currentNode->data = NULL;

	/* If element is at the front with no previous, then set the queue's front to the node's next node */
	if (currentNode->prev == NULL)
	{
		queue->front = currentNode->next;
		currentNode->next->prev = NULL;
	}
	else if (currentNode->next == NULL)
	{
		queue->rear = currentNode->prev;
		currentNode->prev->next = NULL;
	}
	else
	{
		/* Reset pointers for the rest of the nodes */
		currentNode->next->prev = currentNode->prev;
		currentNode->prev->next = currentNode->next;
	}

	/* Decrement Queue Length */
	queue->len--;

	return retval;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	struct node *currentNode;
	currentNode = malloc(sizeof(struct node));

	if (!queue || !func)
	{
		return -1;
	}

	/* Iterate beginning from head */
	currentNode = queue->front;
	while (currentNode != NULL)
	{
		func(queue, currentNode->data);
		currentNode = currentNode->next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (!queue)
	{
		return -1;
	}

	return queue->len;
}
