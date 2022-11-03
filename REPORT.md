# Threading library

## Summary

The threading library is a user-level thread library which can create and
schedule threads. The library is able to support multiple threads.

## Implementation

The implementation of the library can be divided into 4 different phases:

1. Queue data structure implementation
2. Thread scheduler
3. Semaphore and thread block implementation
4. Preemption

## Queue

The queue was implemented using a doubly linked list implementation. The queue
struct consisted of a front and a rear, which was important for enqueuing and
dequeuing. Furthermore, the queue had an additional integer parameter of length.
When creating the queue, memory would be allocated for a queue struct object.
Once this queue was created, it could be manipulated. A linked list node struct
was also implemented, possessing pointers which pointed to the previous and next
node, as well as data.

Two O(1) operations were implemented for the queue, queue_enqueue() and
queue_dequeue(). **queue_enqueue()** works by initializing a node, info_node,
with NULL parameters. If the queue is empty, both the queue's front and rear
would be set to this node to fill the queue with its first element. Otherwise,
elements are queued at the rear, and the rear's pointers are updated.
**queue_dequeue()** works by setting the queue's front to the next node and
decreasing the queue's length by 1.

Two extra operations were also implemented, queue_iterate() and queue_delete().
**queue_iterate()** was implemented by running a function on the current node,
and then setting the current node to the next node. The current node is
initially the queue's front, and then it would set the current node to the next
node until there were no more next nodes. **queue_delete()** would iterate
through a while loop through nodes until it encountered a node with the same
data pointer. Once this was done, the node would be deleted by setting its data
to NULL, resetting the pointers for the previous node and the next node.

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

    /* Decrement Queue Length*/
    queue->len--;

## Thread scheduling

A thread's TCB struct stores the thread's state, context, and stack pointer. The
scheduling was done in **uthread_run()**. A ready queue was initially created,
for the purpose of storing nodes that were either ready or blocked. An idle
thread is registered and set to the ready state, until the initial thread is
created.

When the initial thread is created using **uthread_create()**, memory is
allocated for the thread. From there, the thread's stack is initialized using
uthread_ctx_alloc_stack(), the state is set to Ready, and the context is
initialized. This initial thread proceeds to be enqueued into the ready queue,
where it will later be elected to run.

Much of the project revolved around our yielding function, because we only used
one queue for both blocked and ready threads. **uthread_yield()** had two
pointers, curr and new, pointed to the current thread and the next thread. From
there, a new thread is selected through dequeuing the oldest thread. If the next
thread's state was either blocked or exited, it would simply push it back into
the rear of the queue and the next oldest thread would be checked to see if it
was in the Ready state. The current thread, represented by the curr pointer,
would be enqueued back into the queue and have its state set to Ready. Because
the current thread represents the currently running thread, we reset the current
thread's pointer to this newly dequeued thread. We then call
uthread_ctx_switch() to switch from the previously running thread's context to
the new one.

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

**uthread_exit()** simply sets the state of the current thread to Exited and
destroys its stack. After that, it will yield to the next thread because it is
done running. As uthread_yield() will only dequeue ready threads, exited threads
will never run again.

**uthread_block()** simply sets the thread's state to blocked and yields to the
next ready thread. Blocked threads can become ready again through
**uthread_unblock()**, which will set the thread's state to Ready and reenqueue
it into the ready queue.

## Semaphore implementation

The semaphore struct contains a waiting threads queue and an internal count. A
semaphore struct object can be initialized using **sem_create()**, which
allocates memory for a semaphore object, creates the internal queue, and sets
the count to the initial count.

**sem_down()** will check whether resources are available. If they are, the
internal count is simply decremented. Otherwise, the current thread is blocked
by calling **uthread_block()** and enqueuing it into a list of waiting threads.

**sem_up()** will interact with this queue of blocked threads. When releasing a
resource, it will check the queue for blocked threads. If there are still
blocked threads looking for resources, it will dequeue the oldest blocked thread
and call uthread_unblock(), allowing the thread to be scheduled again.
Otherwise, it will simply increment the resource count.

## Preemption

Preemption dealt with changing the signal mask to ignore the SIGVTALRM signal.
Global objects included a sigaction object, a signal set object, and a virtual
timer object.

For **preempt_enable()**, the program adds the SIGVTALRM signal to the signal
set, which tells what signals will be affected by the handler. The signal mask
is then changed to block these alarm signals, preventing it from interrupting
important functions from taking place. **preempt_disable()** merely reverses
this, unblocking these signals and allowing interrupts to take place again.

**preempt_run()** checks if the preempt boolean is set to true. If it is, a
signal handler is set up. The signal_handler function calls uthread_yield() to
schedule another thread. A virtual timer object timer is used, as it is
associated with the SIGVTALRM. To get seconds from the frequency, the reciprocal
of the frequency was taken, as it is in cycles per second. It was then converted
to microseconds. The it_interval was set equal to the it_value, because the
period between successive timer interrupts and the period between now and the
first timer interrupt should be the same. **preempt_stop()** stops the alarm by
setting the timer values to 0. The signal handler was then restored to its
default action, SIG_DFL, and sigaction() was called once more to restore the
signal action.

## Citations

1. Porquet's syscall lecture (for signals)
2. https://man7.org/linux/man-pages/man2/sigaction.2.html
