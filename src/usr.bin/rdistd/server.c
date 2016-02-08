/*	$OpenBSD: server.c,v 1.16 2003/07/07 14:39:26 mpech Exp $	*/
 * 3. Neither the name of the University nor the names of its contributors
#include "defs.h"

static char RCSid[] __attribute__((__unused__)) =
"$From: server.c,v 1.10 1999/08/04 15:57:33 christos Exp $";
static char RCSid[] __attribute__((__unused__)) =
"$OpenBSD: server.c,v 1.16 2003/07/07 14:39:26 mpech Exp $";
static char sccsid[] __attribute__((__unused__)) =
"@(#)server.c	5.3 (Berkeley) 6/7/86";
static char copyright[] __attribute__((__unused__)) =
static int cattarget(char *);
static int setownership(char *, int, UID_T, GID_T, int);
static int setfilemode(char *, int, int, int);
static int fchog(int, char *, char *, char *, int);
static int removefile(struct stat *, int);
static void doclean(char *);
static void clean(char *);
static void dospecial(char *);
static void docmdspecial(void);
static void query(char *);
static int chkparent(char *, opt_t);
static char *savetarget(char *, opt_t);
static void recvfile(char *, opt_t, int, char *, char *, time_t, time_t, off_t);
static void recvdir(opt_t, int, char *, char *);
static void recvlink(char *, opt_t, int, off_t);
static void hardlink(char *);
static void setconfig(char *);
static void recvit(char *, int);
static void dochmog(char *);
static void settarget(char *, int);

static int
cattarget(char *string)
	(void) snprintf(ptarget, sizeof(target) - (ptarget - target),
			"/%s", string);
static int
setownership(char *file, int fd, UID_T uid, GID_T gid, int link)
	if (getuid() != 0) 
		uid = -1;

	/*
	 * If we are dealing with a symlink, only try to change it if
	 * we have lchown, if we don't leave it alone.
	 */
#if	defined(HAVE_LCHOWN)
	if (link)
		status = lchown(file, (CHOWN_UID_T) uid, 
				(CHOWN_GID_T) gid);
#else
	if (link)
		return 0;
	if (fd != -1 && !link)
		status = fchown(fd, (CHOWN_UID_T) uid, 
				(CHOWN_GID_T) gid);
	if (status < 0 && !link)
		status = chown(file, (CHOWN_UID_T) uid, 
			       (CHOWN_GID_T) gid);
	if (status < 0) {
		if (uid == (UID_T) -1)
				target, gid, SYSERR);
		else
			message(MT_NOTICE, "%s: chown %d.%d failed: %s", 
				target, uid, gid, SYSERR);
		return(-1);
static int
setfilemode(char *file, int fd, int mode, int link)
	/*
	 * If we are dealing with a symlink, only try to change it if
	 * we have lchown, if we don't leave it alone.
	 */
#if	defined(HAVE_LCHMOD)
	if (link)
		status = lchmod(file, mode);
#else
	if (link)
		return 0;
#endif

	if (fd != -1 && !link)
	if (status < 0 && !link)
static int
fchog(int fd, char *file, char *owner, char *group, int mode)
	static struct group *gr = NULL;
	struct stat st;
		} else if (pw == NULL || strcmp(owner, pw->pw_name) != 0) {
			} else
				uid = pw->pw_uid;
			uid = pw->pw_uid;
			primegid = pw->pw_gid;
	if (gr == NULL || strcmp(group, gr->gr_name) != 0) {
		if ((*group == ':' && 
		     (getgrgid(gid = atoi(group + 1)) == NULL))
		    || ((gr = (struct group *)getgrnam(group)) == NULL)) {
		} else
			gid = gr->gr_gid;
	} else
		gid = gr->gr_gid;
				if (strcmp(locuser, gr->gr_mem[i]) == 0)
	if (stat(file, &st) == -1) {
		error("%s: Stat failed %s", file, SYSERR);
		return -1;
	}
	if (setownership(file, fd, uid, gid, S_ISLNK(st.st_mode)) < 0) {
	(void) setfilemode(file, fd, mode, S_ISLNK(st.st_mode));
static int
removefile(struct stat *statb, int silent)
	case S_IFCHR:
	case S_IFBLK:
#ifdef S_IFSOCK
	case S_IFSOCK:
#endif
#ifdef S_IFIFO
	case S_IFIFO:
#endif
				if (!silent)
					message(MT_REMOTE|MT_NOTICE, 
						"%s: unlink failed: %s",
						target, SYSERR);
	while ((dp = readdir(d)) != NULL) {
			if (!silent)
				message(MT_REMOTE|MT_WARNING, 
					"%s/%s: Name too long", 
					target, dp->d_name);
		while ((*ptarget++ = *cp++) != '\0')
			continue;
			if (!silent)
				message(MT_REMOTE|MT_WARNING,
					"%s: lstat failed: %s", 
					target, SYSERR);
		if (removefile(&stb, 0) < 0)
#if NEWWAY
	if (!silent)
		message(MT_CHANGE|MT_REMOTE, "%s: removed", target);
#else
#endif
static void
doclean(char *cp)
	char targ[MAXPATHLEN*4];
	while ((dp = readdir(d)) != NULL) {
		while ((*ptarget++ = *cp++) != '\0')
			continue;
		ENCODE(targ, dp->d_name);
		(void) sendcmd(CC_QUERY, "%s", targ);
			(void) removefile(&stb, 0);
static void
clean(char *cp)
static void
dospecial(char *xcmd)
	char cmd[BUFSIZ];
	if (DECODE(cmd, xcmd) == -1) {
		error("dospecial: Cannot decode command.");
		return;
	}
static void
docmdspecial(void)
	size_t len;
				(void) snprintf(env, len, "export %s;%s=%s", 
				len = strlen(env) + 1 + strlen(cp) + 1;
				env = (char *) xrealloc(env, len);
				(void) strlcat(env, ":", len);
				(void) strlcat(env, cp, len);
				len = strlen(env) + 1 + strlen(cp) + 1;
				env = (char *) xrealloc(env, len);
				(void) strlcat(env, ";", len);
				(void) strlcat(env, cp, len);
static void
query(char *xname)
	char name[MAXPATHLEN];

	if (DECODE(name, xname) == -1) {
		error("query: Cannot decode filename");
		return;
	}
#ifdef notyet
	case S_IFCHR:
	case S_IFBLK:
#ifdef S_IFSOCK
	case S_IFSOCK:
#endif
#ifdef S_IFIFO
	case S_IFIFO:
#endif
#endif
			       (long) stb.st_size, stb.st_mtime,
static int
chkparent(char *name, opt_t opts)
static char *
savetarget(char *file, opt_t opts)
	if (IS_ON(opts, DO_HISTORY)) {
		int i;
		struct stat st;
		/*
		 * There is a race here, but the worst that can happen
		 * is to lose a version of the file
		 */
		for (i = 1; i < 1000; i++) {
			(void) snprintf(savefile, sizeof(savefile),
					"%s;%.3d", file, i);
			if (stat(savefile, &st) == -1 && errno == ENOENT)
				break;
		}
		if (i == 1000) {
			message(MT_NOTICE, 
			    "%s: More than 1000 versions for %s; reusing 1\n",
				savefile, SYSERR);
			i = 1;
			(void) snprintf(savefile, sizeof(savefile),
					"%s;%.3d", file, i);
		}
	}
	else {
		(void) snprintf(savefile, sizeof(savefile), "%s%s",
				file, SAVE_SUFFIX);

		if (unlink(savefile) != 0 && errno != ENOENT) {
			message(MT_NOTICE, "%s: remove failed: %s",
				savefile, SYSERR);
			return(NULL);
		}
static void
recvfile(char *new, opt_t opts, int mode, char *owner, char *group,
	 time_t mtime, time_t atime, off_t size)
	int f, wrerr, olderrno;
		if (wrerr == 0 && xwrite(f, buf, amt) != amt) {
			(void) fclose(f1);
		if ((savefile = savetarget(target, opts)) == NULL) {
		removefile(&statbuff, 0);
		static char fmt[] = "%s -> %s: rename failed: %s";
		struct stat stb;
		switch (errno) {
		case ETXTBSY:
			if ((savefile = savetarget(target, opts)) != NULL) {
					error(fmt, new, target, SYSERR);
						error(fmt,
						      savefile, target, SYSERR);
					(void) unlink(new);
				/*
				 * XXX: We should remove the savefile here.
				 *	But we are nice to nfs clients and
				 *	we keep it.
				 */
			break;
		case EISDIR:
			/*
			 * See if target is a directory and remove it if it is
			 */
			if (lstat(target, &stb) == 0) {
				if (S_ISDIR(stb.st_mode)) {
					char *optarget = ptarget;
					for (ptarget = target; *ptarget;
						ptarget++);
					/* If we failed to remove, we'll catch
					   it later */
					(void) removefile(&stb, 1);
					ptarget = optarget;
				}
			}
			if (rename(new, target) >= 0)
				break;
			/*FALLTHROUGH*/

		default:
			error(fmt, new, target, SYSERR);
			break;
static void
recvdir(opt_t opts, int mode, char *owner, char *group)
				if ((cp = getusername(stb.st_uid, target, o))
				    != NULL)
						(void) strlcpy(lowner, cp,
						    sizeof(lowner));
				if ((cp = getgroupname(stb.st_gid, target, o))
				    != NULL)
						(void) strlcpy(lgroup, cp,
						    sizeof(lgroup));
					(void) strlcpy(lowner, cp,
					    sizeof(lowner));
					(void) strlcpy(lgroup, cp,
					    sizeof(lgroup));
				"%s: need to chown from %s:%s to %s:%s",
					       "%s: chown from %s:%s to %s:%s",
static void
recvlink(char *new, opt_t opts, int mode, off_t size)
	char tbuf[MAXPATHLEN];
	int uptodate;
	uptodate = 0;
	if ((i = readlink(target, tbuf, sizeof(tbuf)-1)) != -1) {
		tbuf[i] = '\0';
		if (i == size && strncmp(buf, tbuf, (int) size) == 0)
			uptodate = 1;
	}
	mode &= 0777;

	if (IS_ON(opts, DO_VERIFY) || uptodate) {
		if (uptodate)
			message(MT_REMOTE|MT_INFO, "");
		else
			message(MT_REMOTE|MT_INFO, "%s: need to update",
				target);
		if (IS_ON(opts, DO_COMPARE))
			return;
		(void) sendcmd(C_END, NULL);
		(void) response();
		return;
	}

			if (removefile(&stb, 0) < 0) {
	message(MT_REMOTE|MT_CHANGE, "%s: updated", target);
static void
hardlink(char *cmd)
	char *xoldname, *xnewname;
	char oldname[BUFSIZ], newname[BUFSIZ];
	xoldname = strtok(cp, " ");
	if (xoldname == NULL) {
	if (DECODE(oldname, xoldname) == -1) {
		error("hardlink: Cannot decode oldname");
		return;
	}

	xnewname = strtok(NULL, " ");
	if (xnewname == NULL) {
	if (DECODE(newname, xnewname) == -1) {
		error("hardlink: Cannot decode newname");
		return;
	}

	if (exptilde(expbuf, oldname, sizeof(expbuf)) == NULL) {
	if (link(expbuf, target) < 0) {
static void
setconfig(char *cmd)
			message(MT_SYSLOG, "startup for %s", fromhost);
		if (!isdigit((unsigned char)*cp)) {
		if (!isdigit((unsigned char)*cp)) {
		if ((estr = msgparseopts(cp, TRUE)) != NULL) {
	case SC_DEFOWNER:
		(void) strlcpy(defowner, cp, sizeof(defowner));
		break;

	case SC_DEFGROUP:
		(void) strlcpy(defgroup, cp, sizeof(defgroup));
		break;

static void
recvit(char *cmd, int type)
	char fileb[MAXPATHLEN];
	 * Get file name. Can't use strtok() since there could
	if (DECODE(fileb, group + strlen(group) + 1) == -1) {
		error("recvit: Cannot decode file name");
		return;
	}

	if (fileb[0] == '\0') {
	file = fileb;
		if ((size_t) catname >= sizeof(sptarget)) {
			while ((*ptarget++ = *file++) != '\0')
			    continue;
			(void) strlcpy(new, tempname, sizeof(new));
			(void) snprintf(new, sizeof(new), "/%s", tempname);
			(void) snprintf(new, sizeof(new), "%s/%s", target,
					tempname);
/*
 * Chmog something
 */
static void
dochmog(char *cmd)
{
	int mode;
	opt_t opts;
	char *owner, *group, *file;
	char *cp = cmd;
	char fileb[MAXPATHLEN];

	/*
	 * Get rdist option flags
	 */
	opts = strtol(cp, &cp, 8);
	if (*cp++ != ' ') {
		error("dochmog: options not delimited");
		return;
	}

	/*
	 * Get file mode
	 */
	mode = strtol(cp, &cp, 8);
	if (*cp++ != ' ') {
		error("dochmog: mode not delimited");
		return;
	}

	/*
	 * Get file owner name
	 */
	owner = strtok(cp, " ");
	if (owner == NULL) {
		error("dochmog: owner name not delimited");
		return;
	}

	/*
	 * Get file group name
	 */
	group = strtok(NULL, " ");
	if (group == NULL) {
		error("dochmog: group name not delimited");
		return;
	}

	/*
	 * Get file name. Can't use strtok() since there could
	 * be white space in the file name.
	 */
	if (DECODE(fileb, group + strlen(group) + 1) == -1) {
		error("dochmog: Cannot decode file name");
		return;
	}

	if (fileb[0] == '\0') {
		error("dochmog: no file name");
		return;
	}
	file = fileb;

	debugmsg(DM_MISC,
		 "dochmog: opts = %04o mode = %04o", opts, mode);
	debugmsg(DM_MISC,
	         "dochmog: owner = '%s' group = '%s' file = '%s' catname = %d",
		 owner, group, file, catname);

	if (catname && cattarget(file) < 0) {
		error("Cannot set newname target.");
		return;
	}

	(void) fchog(-1, target, owner, group, mode);

	ack();
}

static void
settarget(char *cmd, int isdir)
	char file[BUFSIZ];
	if (DECODE(file, cp) == -1) {
		error("settarget: Cannot decode target name");
		return;
	}

	if (exptilde(target, cp, sizeof(target)) == NULL)
void
cleanup(int dummy)
void
server(void)
	if (setjmp(finish_jmpbuf))
	(void) strlcpy(tempname, _RDIST_TMP, sizeof(tempname));
	if (cmdbuf[0] != S_VERSION || !isdigit((unsigned char)cmdbuf[1])) {
		if (n == -1)		/* EOF */
	        case C_CHMOG:  		/* Set owner, group, mode */
			dochmog(cp);