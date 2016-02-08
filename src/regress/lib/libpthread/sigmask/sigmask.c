/* $OpenBSD: sigmask.c,v 1.2 2003/07/14 22:01:42 marc Exp $ */
/* PUBLIC DOMAIN July 2003 Marco S Hyman <marc@snafu.org> */

#include <sys/time.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "test.h"

/*
 * Test that masked signals with a default action of terminate process
 * do NOT terminate the process.
 */
int main (int argc, char *argv[])
{
	sigset_t mask;
	int sig;

	/* any two (or more) command line args should cause the program
	   to die */
	if (argc > 2) {
		printf("trigger sigalrm[1] [test should die]\n");
		ualarm(100000, 0);
		CHECKe(sleep(1));
	}

	/* mask sigalrm */
	CHECKe(sigemptyset(&mask));
	CHECKe(sigaddset(&mask, SIGALRM));
	CHECKe(pthread_sigmask(SIG_BLOCK, &mask, NULL));

	/* now trigger sigalrm and wait for it */
	printf("trigger sigalrm[2] [masked, test should not die]\n");
	ualarm(100000, 0);
	CHECKe(sleep(1));

	/* sigwait for sigalrm, it should be pending.   If it is not
	   the test will hang. */
	CHECKe(sigwait(&mask, &sig));
	ASSERT(sig == SIGALRM);

	/* make sure sigwait didn't muck with the mask by triggering
	   sigalrm, again */
	printf("trigger sigalrm[3] after sigwait [masked, test should not die]\n");
	ualarm(100000, 0);
	CHECKe(sleep(1));

	/* any single command line arg will run this code wich unmasks the
	   signal and then makes sure the program terminates when sigalrm
	   is triggered. */
	if (argc > 1) {
		printf("trigger sigalrm[4] [unmasked, test should die]\n");
		CHECKe(pthread_sigmask(SIG_UNBLOCK, &mask, NULL));
		ualarm(100000, 0);
		CHECKe(sleep(1));
	}
	
	SUCCEED;
}