#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"
#include "uthread.h"

struct semaphore
{
	queue_t waiting_threads;
	size_t sem_count;
};

sem_t sem_create(size_t count)
{
	sem_t semaphore = malloc(sizeof(struct semaphore));

	if (!semaphore)
	{
		return NULL;
	}

	semaphore->waiting_threads = queue_create();
	semaphore->sem_count = count;
	return semaphore;
}

int sem_destroy(sem_t sem)
{
	if (!sem || queue_length(sem->waiting_threads) < 0)
	{
		return -1;
	}

	free(sem);
	return 0;
}

/* Block the thread and then enqueue into the waiting threads queue */
int sem_down(sem_t sem)
{
	if (!sem)
	{
		return -1;
	}

	struct uthread_tcb *current_thread = uthread_current();

	/* Check if there are still resources available */
	if (sem->sem_count != 0)
	{
		sem->sem_count--;
	}
	else if (sem->sem_count == 0)
	{
		/* No more resource left, add to waiting queue for resource */
		queue_enqueue(sem->waiting_threads, current_thread);
		uthread_block();
	}
	return 0;
}

/* Release waiting threads if any or release resource */
int sem_up(sem_t sem)
{
	struct uthread_tcb *thread;

	if (!sem)
	{
		return -1;
	}

	/* Check if sem count was at 0 */
	if (sem->sem_count == 0)
	{
		/* Check if there are threads waiting for resource */
		if (queue_length(sem->waiting_threads) != 0)
		{
			/* Get oldest item in the queue */
			queue_dequeue(sem->waiting_threads, (void **)&thread);
			uthread_unblock(thread);
		}
		else
		{
			/* if there are no threads waiting, increase sem count */
			sem->sem_count++;
		}
	}
	else
	{
		sem->sem_count++;
	}

	return 0;
}
