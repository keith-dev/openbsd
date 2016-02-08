/*	$OpenBSD: pcnfsd_misc.c,v 1.5 2002/05/26 09:25:22 deraadt Exp $	*/
/*	$NetBSD: pcnfsd_misc.c,v 1.2 1995/07/25 22:20:42 gwr Exp $	*/

/* RE_SID: @(%)/usr/dosnfs/shades_SCCS/unix/pcnfsd/v2/src/SCCS/s.pcnfsd_misc.c 1.5 92/01/24 19:59:13 SMI */
/*
 *=====================================================================
 * Copyright (c) 1986,1987,1988,1989,1990,1991 by Sun Microsystems, Inc.
 *	@(#)pcnfsd_misc.c	1.5	1/24/92
 *=====================================================================
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <utmp.h>

#include "pcnfsd.h"

#define	zchar		0x5b

char            tempstr[256];
extern char	sp_name[1024]; /* in pcnfsd_print.c */

void
scramble(s1, s2)
	char *s1, *s2;
{
	while (*s1) {
		*s2++ = (*s1 ^ zchar) & 0x7f;
		s1++;
	}
	*s2 = 0;
}

struct passwd *
get_password(user)
	char *user;
{
	struct passwd *pwd;
	static struct passwd lpwd;
	char *pass, *ushell;
	int ok = 0;

	if ((pwd = getpwnam(user)) == NULL)
		return (NULL);

	pass = pwd->pw_passwd;

	lpwd = *pwd;
	lpwd.pw_passwd = pass;

	/*
	 * ensure that the shell ends in "sh" - probobly not worth it.
	 */
	ushell = lpwd.pw_shell;
	if (strlen(ushell) < 2)
		return ((struct passwd *)NULL);
	ushell += strlen(ushell) - 2;
	if (strcmp(ushell, "sh"))
		return ((struct passwd *)NULL);

	return (&lpwd);
}

/*
 *---------------------------------------------------------------------
 *                      Print support procedures 
 *---------------------------------------------------------------------
 */

char           *
mapfont(f, i, b)
	char f, i, b;
{
	static char fontname[64];

	fontname[0] = 0;	/* clear it out */

	switch (f) {
	case 'c':
		(void)strcpy(fontname, "Courier");
		break;
	case 'h':
		(void)strcpy(fontname, "Helvetica");
		break;
	case 't':
		(void)strcpy(fontname, "Times");
		break;
	default:
		(void)strcpy(fontname, "Times-Roman");
		goto finis ;
	}
	if (i != 'o' && b != 'b') {	/* no bold or oblique */
		if (f == 't')	/* special case Times */
			(void)strcat(fontname, "-Roman");
		goto finis;
	}
	(void)strcat(fontname, "-");
	if (b == 'b')
		(void)strcat(fontname, "Bold");
	if (i == 'o')		/* o-blique */
		(void)strcat(fontname, f == 't' ? "Italic" : "Oblique");

finis:
	return (&fontname[0]);
}

void
wlogin(name, req)
	char *name;
	struct svc_req *req;
{
	struct sockaddr_in *who;
	struct hostent *hp;
	char *host;
	struct utmp ut;
	int fd;

	/* Get network address of client. */
	who = &req->rq_xprt->xp_raddr;

	/* Get name of connected client */
	hp = gethostbyaddr((char *)&who->sin_addr, sizeof(struct in_addr),
			   who->sin_family);

	if (hp && (strlen(hp->h_name) <= sizeof(ut.ut_host))) {
		host = hp->h_name;
	} else {
		host = inet_ntoa(who->sin_addr);
	}

	(void)strcpy(ut.ut_line, "PC-NFS");
	(void)strncpy(ut.ut_name, name, sizeof(ut.ut_name));
	(void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
	(void)time(&ut.ut_time);

	if ((fd = open(_PATH_WTMP, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)&ut, sizeof(struct utmp));
		(void)close(fd);
	}
}

/*
 *---------------------------------------------------------------------
 *                      Run-process-as-user procedures 
 *---------------------------------------------------------------------
 */


#define	READER_FD	0
#define	WRITER_FD	1

static pid_t    child_pid;

static char     cached_user[64] = "";
static uid_t    cached_uid;
static gid_t    cached_gid;

static	struct sigaction old_action;
static	struct sigaction new_action;
static	struct itimerval timer;

int interrupted = 0;
static	FILE *pipe_handle;

static void
myhandler()
{
	interrupted = 1;
	(void)fclose(pipe_handle);
	kill(child_pid, SIGKILL);
}

void
start_watchdog(n)
	int n;
{
	/*
	 * Setup SIGALRM handler, force interrupt of ongoing syscall
	 */

	new_action.sa_handler = myhandler;
	sigemptyset(&(new_action.sa_mask));
	new_action.sa_flags = 0;
#ifdef SA_INTERRUPT
	new_action.sa_flags |= SA_INTERRUPT;
#endif
	sigaction(SIGALRM, &new_action, &old_action);

	/*
	 * Set interval timer for n seconds
	 */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = n;
	timer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	interrupted = 0;
}

void stop_watchdog()
{
	/*
	 * Cancel timer
	 */

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	/*
 	 * restore old signal handling
	 */
	sigaction(SIGALRM, &old_action, NULL);
}



FILE *
su_popen(user, cmd, maxtime)
	char           *user;
	char           *cmd;
	int		maxtime;
{
	int             p[2];
	int             parent_fd, child_fd;
	pid_t		pid;
	struct passwd *pw;

	if (strcmp(cached_user, user)) {
		pw = getpwnam(user);
		if (!pw)
			pw = getpwnam("nobody");
		if (pw) {
			cached_uid = pw->pw_uid;
			cached_gid = pw->pw_gid;
			strcpy(cached_user, user);
		} else {
			cached_uid = (uid_t) (-2);
			cached_gid = (gid_t) (-2);
			cached_user[0] = '\0';
		}
	}
	if (pipe(p) < 0) {
		msg_out("rpc.pcnfsd: unable to create pipe in su_popen");
		return (NULL);
	}
	parent_fd = p[READER_FD];
	child_fd = p[WRITER_FD];
	if ((pid = fork()) == 0) {
		int             i;

		for (i = 0; i < 10; i++)
			if (i != child_fd)
				(void) close(i);
		if (child_fd != 1) {
			(void) dup2(child_fd, 1);
			(void) close(child_fd);
		}
		dup2(1, 2);	/* let's get stderr as well */

		(void) setgid(cached_gid);
		(void) setuid(cached_uid);

		(void) execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
		_exit(255);
	}
	if (pid == -1) {
		msg_out("rpc.pcnfsd: fork failed");
		close(parent_fd);
		close(child_fd);
		return (NULL);
	}
	child_pid = pid;
	close(child_fd);
	start_watchdog(maxtime);
	pipe_handle = fdopen(parent_fd, "r");
	return (pipe_handle);
}

int
su_pclose(ptr)
	FILE           *ptr;
{
	int             status;
	pid_t		pid;

	stop_watchdog();

	fclose(ptr);
	if (child_pid == -1)
		return (-1);
	while ((pid = wait(&status)) != child_pid && pid != -1)
		;
	return (pid == -1 ? -1 : status);
}

/*
 * strembedded - returns true if s1 is embedded (in any case) in s2
 */
int
strembedded(s1, s2)
	char *s1, *s2;
{
	while (*s2) {
		if (!strcasecmp(s1, s2))
			return (1);
		s2++;
	}
	return (0);
}
