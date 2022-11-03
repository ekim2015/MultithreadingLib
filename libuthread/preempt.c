#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction sa;
sigset_t ss;
struct itimerval timer;

/* Pass this as signal handler */
void sig_handler(int dummy)
{
	/* Do nothing with this value, handles the int warning */
	(void)dummy;

	uthread_yield();
}

void preempt_disable(void)
{
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &ss, NULL);
}

void preempt_enable(void)
{
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &ss, NULL);
}

void preempt_start(bool preempt)
{
	if (preempt)
	{
		/* Set up handler */
		sa.sa_handler = sig_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGVTALRM, &sa, NULL);

		/* Configure Timer */
		timer.it_value.tv_sec = 1 / HZ;
		timer.it_value.tv_usec = 1000000 / HZ;
		timer.it_interval = timer.it_value;
		setitimer(ITIMER_VIRTUAL, &timer, NULL);
	}
}

void preempt_stop(void)
{
	/* To stop preemption, we need to stop the timer by setting it_value values to 0 */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);

	/* Proceed to then set the handler back */
	/* SIG_DFL derived from struct sigaction man page */
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGVTALRM, &sa, NULL);
}
