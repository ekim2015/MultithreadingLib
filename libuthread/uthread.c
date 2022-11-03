#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef enum
{
	Running = 0,
	Ready = 1,
	Blocked = 2,
	Exited = 3,
} state;

struct uthread_tcb
{
	uthread_ctx_t ctx;
	void *stack;
	state state;
};

/* Create global queues for ready thread */
queue_t ready_queue;

struct uthread_tcb *current_thread;

struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

void uthread_yield(void)
{
	struct uthread_tcb *curr = uthread_current();
	struct uthread_tcb *next;

	/* Preempt Disable */
	preempt_disable();

	/* Pick New Thread to Run */
	queue_dequeue(ready_queue, (void **)&next);
	while (next->state != Ready)
	{
		queue_enqueue(ready_queue, next);
		queue_dequeue(ready_queue, (void **)&next);
	}
	next->state = Running;

	/* Save Current Thread's state if it is Running */
	if (curr->state == Running)
	{
		curr->state = Ready;
		queue_enqueue(ready_queue, curr);
	}

	/* Reset New Current Thread */
	current_thread = next;

	/* Context Switch */
	uthread_ctx_switch(&curr->ctx, &next->ctx);

	/* Enable preemption */
	preempt_enable();
}

void uthread_exit(void)
{
	struct uthread_tcb *curr = uthread_current();

	curr->state = Exited;

	/* Destroy stack of current thread */
	uthread_ctx_destroy_stack(curr->stack);

	/* We use uthread_yield because code is roughly the same */
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{
	int init_value;
	int enqueue_value;

	/* Disable Preemption */
	preempt_disable();

	/* Create thread */
	struct uthread_tcb *thread = malloc(sizeof(struct uthread_tcb));
	if (!thread)
	{
		return -1;
	}

	/* Initialize Thread */
	/* We initialize the thread's stack first because it is a parameter in creating the context */
	thread->stack = uthread_ctx_alloc_stack();
	thread->state = Ready;
	init_value = uthread_ctx_init(&thread->ctx, thread->stack, func, arg);

	if (init_value != 0)
	{
		return -1;
	}

	/* Push thread in ready queue */
	enqueue_value = queue_enqueue(ready_queue, thread);
	if (enqueue_value != 0)
	{
		return -1;
	}

	/* Enable Preemption */
	preempt_enable();

	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	int create_value;

	/* Preempt Start */
	preempt_start(preempt);

	/* Create global queues */
	ready_queue = queue_create();

	/* Disable Preempt */
	preempt_disable();

	/* Initialize Current Thread */
	current_thread = malloc(sizeof(struct uthread_tcb));

	/* Register Application as idle Thread */
	struct uthread_tcb *idle = malloc(sizeof(struct uthread_tcb));
	if (!idle || !ready_queue)
	{
		return -1;
	}

	/* Set Idle Thread state to Ready */
	idle->state = Running;

	/* Set Idle as Current Thread */
	current_thread = idle;

	/* Enable Preempt */
	preempt_enable();

	/* Disable Preempt */
	preempt_disable();

	/* Creates New Initial Thread */
	create_value = uthread_create(func, arg);

	if (create_value == -1)
	{
		return -1;
	}

	/* Enable Preempt */
	preempt_enable();

	/* Check for Ready Threads */
	/* If there is nothing left in the ready queue, it should return 0, but should yield when there are still elements in the ready queue */
	while (queue_length(ready_queue) >= 1)
	{
		/* Yield if there are still available threads left */
		uthread_yield();
	}

	/* Free the queue memory */
	queue_destroy(ready_queue);

	/* Enable Preempt */
	preempt_enable();

	/* Preempt Stop */
	preempt_stop();

	return 0;
}

void uthread_block(void)
{
	/* Change current state to blocked */
	current_thread->state = Blocked;
	uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* Disable preempt */
	preempt_disable();

	/* Check if the current state is blocked */
	if (uthread->state == Blocked)
	{
		uthread->state = Ready;
	}
	queue_enqueue(ready_queue, uthread);

	/* Enable Preempt */
	preempt_enable();
}