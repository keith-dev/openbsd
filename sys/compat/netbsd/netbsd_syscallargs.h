/*	$OpenBSD: netbsd_syscallargs.h,v 1.23 2004/01/14 05:26:56 tedu Exp $	*/

/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from;	OpenBSD: syscalls.master,v 1.21 2004/01/14 05:23:25 tedu Exp 
 */

#ifdef	syscallarg
#undef	syscallarg
#endif

#define	syscallarg(x)							\
	union {								\
		register_t pad;						\
		struct { x datum; } le;					\
		struct {						\
			int8_t pad[ (sizeof (register_t) < sizeof (x))	\
				? 0					\
				: sizeof (register_t) - sizeof (x)];	\
			x datum;					\
		} be;							\
	}

struct netbsd_sys_open_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
	syscallarg(int) mode;
};

struct compat_43_netbsd_sys_creat_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
};

struct netbsd_sys_link_args {
	syscallarg(char *) path;
	syscallarg(char *) link;
};

struct netbsd_sys_unlink_args {
	syscallarg(char *) path;
};

struct netbsd_sys_chdir_args {
	syscallarg(char *) path;
};

struct netbsd_sys_mknod_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
	syscallarg(dev_t) dev;
};

struct netbsd_sys_chmod_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
};

struct netbsd_sys_chown_args {
	syscallarg(char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct netbsd_sys_unmount_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct netbsd_sys_access_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct netbsd_sys_chflags_args {
	syscallarg(char *) path;
	syscallarg(u_int) flags;
};

struct compat_43_netbsd_sys_stat_args {
	syscallarg(char *) path;
	syscallarg(struct ostat *) ub;
};

struct compat_43_netbsd_sys_lstat_args {
	syscallarg(char *) path;
	syscallarg(struct ostat *) ub;
};

struct netbsd_sys_revoke_args {
	syscallarg(char *) path;
};

struct netbsd_sys_symlink_args {
	syscallarg(char *) path;
	syscallarg(char *) link;
};

struct netbsd_sys_readlink_args {
	syscallarg(char *) path;
	syscallarg(char *) buf;
	syscallarg(size_t) count;
};

struct netbsd_sys_execve_args {
	syscallarg(char *) path;
	syscallarg(char **) argp;
	syscallarg(char **) envp;
};

struct netbsd_sys_chroot_args {
	syscallarg(char *) path;
};

struct netbsd_sys_rename_args {
	syscallarg(char *) from;
	syscallarg(char *) to;
};

struct compat_43_netbsd_sys_truncate_args {
	syscallarg(char *) path;
	syscallarg(long) length;
};

struct netbsd_sys_mkfifo_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
};

struct netbsd_sys_mkdir_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
};

struct netbsd_sys_rmdir_args {
	syscallarg(char *) path;
};

struct netbsd_sys_stat_args {
	syscallarg(char *) path;
	syscallarg(struct stat *) ub;
};

struct netbsd_sys_lstat_args {
	syscallarg(char *) path;
	syscallarg(struct stat *) ub;
};

struct netbsd_sys_pathconf_args {
	syscallarg(char *) path;
	syscallarg(int) name;
};

struct netbsd_sys_truncate_args {
	syscallarg(char *) path;
	syscallarg(int) pad;
	syscallarg(off_t) length;
};

struct netbsd_sys_fdatasync_args {
	syscallarg(int) fd;
};

struct netbsd_sys_getdents_args {
	syscallarg(int) fd;
	syscallarg(char *) buf;
	syscallarg(size_t) count;
};

struct netbsd_sys_lchmod_args {
	syscallarg(const char *) path;
	syscallarg(netbsd_mode_t) mode;
};

struct netbsd_sys_lutimes_args {
	syscallarg(const char *) path;
	syscallarg(const struct timeval *) tptr;
};

struct netbsd_sys___stat13_args {
	syscallarg(char *) path;
	syscallarg(struct netbsd_stat *) ub;
};

struct netbsd_sys___fstat13_args {
	syscallarg(int) fd;
	syscallarg(struct netbsd_stat *) ub;
};

struct netbsd_sys___lstat13_args {
	syscallarg(char *) path;
	syscallarg(struct netbsd_stat *) ub;
};

struct netbsd_sys___sigaction14_args {
	syscallarg(int) signum;
	syscallarg(const struct netbsd_sigaction *) nsa;
	syscallarg(struct netbsd_sigaction *) osa;
};

struct netbsd_sys___sigpending14_args {
	syscallarg(netbsd_sigset_t *) set;
};

struct netbsd_sys___sigprocmask14_args {
	syscallarg(int) how;
	syscallarg(const netbsd_sigset_t *) set;
	syscallarg(netbsd_sigset_t *) oset;
};

struct netbsd_sys___sigsuspend14_args {
	syscallarg(const netbsd_sigset_t *) set;
};

struct netbsd_sys___sigreturn14_args {
	syscallarg(struct netbsd_sigcontext *) sigcntxp;
};

struct netbsd_sys___getcwd_args {
	syscallarg(char *) bufp;
	syscallarg(size_t) length;
};

/*
 * System call prototypes.
 */

int	sys_exit(struct proc *, void *, register_t *);
int	sys_fork(struct proc *, void *, register_t *);
int	sys_read(struct proc *, void *, register_t *);
int	sys_write(struct proc *, void *, register_t *);
int	netbsd_sys_open(struct proc *, void *, register_t *);
int	sys_close(struct proc *, void *, register_t *);
int	sys_wait4(struct proc *, void *, register_t *);
int	compat_43_netbsd_sys_creat(struct proc *, void *, register_t *);
int	netbsd_sys_link(struct proc *, void *, register_t *);
int	netbsd_sys_unlink(struct proc *, void *, register_t *);
int	netbsd_sys_chdir(struct proc *, void *, register_t *);
int	sys_fchdir(struct proc *, void *, register_t *);
int	netbsd_sys_mknod(struct proc *, void *, register_t *);
int	netbsd_sys_chmod(struct proc *, void *, register_t *);
int	netbsd_sys_chown(struct proc *, void *, register_t *);
int	sys_obreak(struct proc *, void *, register_t *);
int	compat_25_sys_getfsstat(struct proc *, void *, register_t *);
int	sys_lseek(struct proc *, void *, register_t *);
int	sys_getpid(struct proc *, void *, register_t *);
int	sys_mount(struct proc *, void *, register_t *);
int	netbsd_sys_unmount(struct proc *, void *, register_t *);
int	sys_setuid(struct proc *, void *, register_t *);
int	sys_getuid(struct proc *, void *, register_t *);
int	sys_geteuid(struct proc *, void *, register_t *);
#ifdef PTRACE
int	sys_ptrace(struct proc *, void *, register_t *);
#else
#endif
int	sys_recvmsg(struct proc *, void *, register_t *);
int	sys_sendmsg(struct proc *, void *, register_t *);
int	sys_recvfrom(struct proc *, void *, register_t *);
int	sys_accept(struct proc *, void *, register_t *);
int	sys_getpeername(struct proc *, void *, register_t *);
int	sys_getsockname(struct proc *, void *, register_t *);
int	netbsd_sys_access(struct proc *, void *, register_t *);
int	netbsd_sys_chflags(struct proc *, void *, register_t *);
int	sys_fchflags(struct proc *, void *, register_t *);
int	sys_sync(struct proc *, void *, register_t *);
int	sys_kill(struct proc *, void *, register_t *);
int	compat_43_netbsd_sys_stat(struct proc *, void *, register_t *);
int	sys_getppid(struct proc *, void *, register_t *);
int	compat_43_netbsd_sys_lstat(struct proc *, void *, register_t *);
int	sys_dup(struct proc *, void *, register_t *);
int	sys_opipe(struct proc *, void *, register_t *);
int	sys_getegid(struct proc *, void *, register_t *);
int	sys_profil(struct proc *, void *, register_t *);
#ifdef KTRACE
int	sys_ktrace(struct proc *, void *, register_t *);
#else
#endif
int	sys_sigaction(struct proc *, void *, register_t *);
int	sys_getgid(struct proc *, void *, register_t *);
int	sys_sigprocmask(struct proc *, void *, register_t *);
int	sys_getlogin(struct proc *, void *, register_t *);
int	sys_setlogin(struct proc *, void *, register_t *);
int	sys_acct(struct proc *, void *, register_t *);
int	sys_sigpending(struct proc *, void *, register_t *);
int	sys_osigaltstack(struct proc *, void *, register_t *);
int	sys_ioctl(struct proc *, void *, register_t *);
int	sys_reboot(struct proc *, void *, register_t *);
int	netbsd_sys_revoke(struct proc *, void *, register_t *);
int	netbsd_sys_symlink(struct proc *, void *, register_t *);
int	netbsd_sys_readlink(struct proc *, void *, register_t *);
int	netbsd_sys_execve(struct proc *, void *, register_t *);
int	sys_umask(struct proc *, void *, register_t *);
int	netbsd_sys_chroot(struct proc *, void *, register_t *);
int	sys_fstat(struct proc *, void *, register_t *);
int	compat_43_sys_getkerninfo(struct proc *, void *, register_t *);
int	compat_43_sys_getpagesize(struct proc *, void *, register_t *);
int	compat_25_sys_omsync(struct proc *, void *, register_t *);
int	sys_vfork(struct proc *, void *, register_t *);
int	sys_sbrk(struct proc *, void *, register_t *);
int	sys_sstk(struct proc *, void *, register_t *);
int	sys_mmap(struct proc *, void *, register_t *);
int	sys_ovadvise(struct proc *, void *, register_t *);
int	sys_munmap(struct proc *, void *, register_t *);
int	sys_mprotect(struct proc *, void *, register_t *);
int	sys_madvise(struct proc *, void *, register_t *);
int	sys_mincore(struct proc *, void *, register_t *);
int	sys_getgroups(struct proc *, void *, register_t *);
int	sys_setgroups(struct proc *, void *, register_t *);
int	sys_getpgrp(struct proc *, void *, register_t *);
int	sys_setpgid(struct proc *, void *, register_t *);
int	sys_setitimer(struct proc *, void *, register_t *);
int	compat_43_sys_wait(struct proc *, void *, register_t *);
int	compat_25_sys_swapon(struct proc *, void *, register_t *);
int	sys_getitimer(struct proc *, void *, register_t *);
int	compat_43_sys_gethostname(struct proc *, void *, register_t *);
int	compat_43_sys_sethostname(struct proc *, void *, register_t *);
int	compat_43_sys_getdtablesize(struct proc *, void *, register_t *);
int	sys_dup2(struct proc *, void *, register_t *);
int	sys_fcntl(struct proc *, void *, register_t *);
int	sys_select(struct proc *, void *, register_t *);
int	sys_fsync(struct proc *, void *, register_t *);
int	sys_setpriority(struct proc *, void *, register_t *);
int	sys_socket(struct proc *, void *, register_t *);
int	sys_connect(struct proc *, void *, register_t *);
int	sys_accept(struct proc *, void *, register_t *);
int	sys_getpriority(struct proc *, void *, register_t *);
int	compat_43_sys_send(struct proc *, void *, register_t *);
int	compat_43_sys_recv(struct proc *, void *, register_t *);
int	sys_sigreturn(struct proc *, void *, register_t *);
int	sys_bind(struct proc *, void *, register_t *);
int	sys_setsockopt(struct proc *, void *, register_t *);
int	sys_listen(struct proc *, void *, register_t *);
int	compat_43_sys_sigvec(struct proc *, void *, register_t *);
int	compat_43_sys_sigblock(struct proc *, void *, register_t *);
int	compat_43_sys_sigsetmask(struct proc *, void *, register_t *);
int	sys_sigsuspend(struct proc *, void *, register_t *);
int	compat_43_sys_sigstack(struct proc *, void *, register_t *);
#ifdef MSG_COMPAT
int	compat_43_sys_recvmsg(struct proc *, void *, register_t *);
#else
#endif
#ifdef MSG_COMPAT
int	compat_43_sys_sendmsg(struct proc *, void *, register_t *);
#else
#endif
int	sys_gettimeofday(struct proc *, void *, register_t *);
int	sys_getrusage(struct proc *, void *, register_t *);
int	sys_getsockopt(struct proc *, void *, register_t *);
int	sys_readv(struct proc *, void *, register_t *);
int	sys_writev(struct proc *, void *, register_t *);
int	sys_settimeofday(struct proc *, void *, register_t *);
int	sys_fchown(struct proc *, void *, register_t *);
int	sys_fchmod(struct proc *, void *, register_t *);
#ifdef MSG_COMPAT
int	compat_43_sys_recvfrom(struct proc *, void *, register_t *);
#else
#endif
int	sys_setreuid(struct proc *, void *, register_t *);
int	sys_setregid(struct proc *, void *, register_t *);
int	netbsd_sys_rename(struct proc *, void *, register_t *);
int	compat_43_netbsd_sys_truncate(struct proc *, void *, register_t *);
int	compat_43_sys_ftruncate(struct proc *, void *, register_t *);
int	sys_flock(struct proc *, void *, register_t *);
int	netbsd_sys_mkfifo(struct proc *, void *, register_t *);
int	sys_sendto(struct proc *, void *, register_t *);
int	sys_shutdown(struct proc *, void *, register_t *);
int	sys_socketpair(struct proc *, void *, register_t *);
int	netbsd_sys_mkdir(struct proc *, void *, register_t *);
int	netbsd_sys_rmdir(struct proc *, void *, register_t *);
int	sys_utimes(struct proc *, void *, register_t *);
int	sys_adjtime(struct proc *, void *, register_t *);
int	compat_43_sys_getpeername(struct proc *, void *, register_t *);
int	compat_43_sys_gethostid(struct proc *, void *, register_t *);
int	compat_43_sys_sethostid(struct proc *, void *, register_t *);
int	compat_43_sys_getrlimit(struct proc *, void *, register_t *);
int	compat_43_sys_setrlimit(struct proc *, void *, register_t *);
int	compat_43_sys_killpg(struct proc *, void *, register_t *);
int	sys_setsid(struct proc *, void *, register_t *);
int	sys_quotactl(struct proc *, void *, register_t *);
int	compat_43_sys_quota(struct proc *, void *, register_t *);
int	compat_43_sys_getsockname(struct proc *, void *, register_t *);
#if defined(NFSCLIENT) || defined(NFSSERVER)
int	sys_nfssvc(struct proc *, void *, register_t *);
#else
#endif
int	compat_43_sys_getdirentries(struct proc *, void *, register_t *);
int	compat_25_sys_statfs(struct proc *, void *, register_t *);
int	compat_25_sys_fstatfs(struct proc *, void *, register_t *);
#if defined(NFSCLIENT) || defined(NFSSERVER)
int	sys_getfh(struct proc *, void *, register_t *);
#else
#endif
int	compat_09_sys_getdomainname(struct proc *, void *, register_t *);
int	compat_09_sys_setdomainname(struct proc *, void *, register_t *);
int	compat_09_sys_uname(struct proc *, void *, register_t *);
int	sys_sysarch(struct proc *, void *, register_t *);
#if defined(SYSVSEM) && !defined(alpha)
int	compat_10_sys_semsys(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVMSG) && !defined(alpha)
int	compat_10_sys_msgsys(struct proc *, void *, register_t *);
#else
#endif
#if defined(SYSVSHM) && !defined(alpha)
int	compat_10_sys_shmsys(struct proc *, void *, register_t *);
#else
#endif
int	sys_pread(struct proc *, void *, register_t *);
int	sys_pwrite(struct proc *, void *, register_t *);
int	sys_setgid(struct proc *, void *, register_t *);
int	sys_setegid(struct proc *, void *, register_t *);
int	sys_seteuid(struct proc *, void *, register_t *);
#ifdef LFS
int	lfs_bmapv(struct proc *, void *, register_t *);
int	lfs_markv(struct proc *, void *, register_t *);
int	lfs_segclean(struct proc *, void *, register_t *);
int	lfs_segwait(struct proc *, void *, register_t *);
#else
#endif
int	netbsd_sys_stat(struct proc *, void *, register_t *);
int	sys_fstat(struct proc *, void *, register_t *);
int	netbsd_sys_lstat(struct proc *, void *, register_t *);
int	netbsd_sys_pathconf(struct proc *, void *, register_t *);
int	sys_fpathconf(struct proc *, void *, register_t *);
int	sys_swapctl(struct proc *, void *, register_t *);
int	sys_getrlimit(struct proc *, void *, register_t *);
int	sys_setrlimit(struct proc *, void *, register_t *);
int	sys_getdirentries(struct proc *, void *, register_t *);
int	sys_mmap(struct proc *, void *, register_t *);
int	sys_lseek(struct proc *, void *, register_t *);
int	netbsd_sys_truncate(struct proc *, void *, register_t *);
int	sys_ftruncate(struct proc *, void *, register_t *);
int	sys___sysctl(struct proc *, void *, register_t *);
int	sys_mlock(struct proc *, void *, register_t *);
int	sys_munlock(struct proc *, void *, register_t *);
int	sys_undelete(struct proc *, void *, register_t *);
int	sys_futimes(struct proc *, void *, register_t *);
int	sys_getpgid(struct proc *, void *, register_t *);
int	sys_xfspioctl(struct proc *, void *, register_t *);
int	sys_poll(struct proc *, void *, register_t *);
#ifdef LKM
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
#else	/* !LKM */
#endif	/* !LKM */
#ifdef SYSVSEM
int	compat_23_sys___semctl(struct proc *, void *, register_t *);
int	sys_semget(struct proc *, void *, register_t *);
int	sys_semop(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVMSG
int	compat_23_sys_msgctl(struct proc *, void *, register_t *);
int	sys_msgget(struct proc *, void *, register_t *);
int	sys_msgsnd(struct proc *, void *, register_t *);
int	sys_msgrcv(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVSHM
int	sys_shmat(struct proc *, void *, register_t *);
int	compat_23_sys_shmctl(struct proc *, void *, register_t *);
int	sys_shmdt(struct proc *, void *, register_t *);
int	sys_shmget(struct proc *, void *, register_t *);
#else
#endif
int	sys_clock_gettime(struct proc *, void *, register_t *);
int	sys_clock_settime(struct proc *, void *, register_t *);
int	sys_clock_getres(struct proc *, void *, register_t *);
int	sys_nanosleep(struct proc *, void *, register_t *);
int	netbsd_sys_fdatasync(struct proc *, void *, register_t *);
int	netbsd_sys_getdents(struct proc *, void *, register_t *);
int	sys_minherit(struct proc *, void *, register_t *);
int	netbsd_sys_lchmod(struct proc *, void *, register_t *);
int	sys_lchown(struct proc *, void *, register_t *);
int	netbsd_sys_lutimes(struct proc *, void *, register_t *);
int	sys_msync(struct proc *, void *, register_t *);
int	netbsd_sys___stat13(struct proc *, void *, register_t *);
int	netbsd_sys___fstat13(struct proc *, void *, register_t *);
int	netbsd_sys___lstat13(struct proc *, void *, register_t *);
int	sys_sigaltstack(struct proc *, void *, register_t *);
int	netbsd_sys___vfork14(struct proc *, void *, register_t *);
int	sys_getsid(struct proc *, void *, register_t *);
#ifdef KTRACE
#else
#endif
int	sys_preadv(struct proc *, void *, register_t *);
int	sys_pwritev(struct proc *, void *, register_t *);
int	netbsd_sys___sigaction14(struct proc *, void *, register_t *);
int	netbsd_sys___sigpending14(struct proc *, void *, register_t *);
int	netbsd_sys___sigprocmask14(struct proc *, void *, register_t *);
int	netbsd_sys___sigsuspend14(struct proc *, void *, register_t *);
int	netbsd_sys___sigreturn14(struct proc *, void *, register_t *);
int	netbsd_sys___getcwd(struct proc *, void *, register_t *);
int	sys_issetugid(struct proc *, void *, register_t *);
