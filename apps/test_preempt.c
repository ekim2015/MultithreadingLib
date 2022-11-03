#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

/* Should print thread2 and thread3, then get stuck on an infinite loop */
/* Test checks what happens if there are multiple threads involved */

void thread3(void *arg)
{
    (void)arg;
    printf("thread3\n");
}

void thread2(void *arg)
{
    (void)arg;
    printf("thread2\n");

    uthread_create(thread3, arg);

    /* Hog the resource for the second thread */
    while (1)
    {
        ;
    }
}

void thread1(void *arg)
{
    (void)arg;
    uthread_create(thread2, arg);
    printf("thread1\n");

    /* Hog the resource for the first thread */
    while (1)
    {
        ;
    }
}

int main(void)
{
    uthread_run(true, thread1, NULL);

    return 0;
}