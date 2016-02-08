#	from: @(#)Makefile	5.45.1.1 (Berkeley) 5/6/91

# Doing a make includes builds /usr/include/pthread
#
# The ``rm -rf''s used below are safe because rm doesn't follow symbolic
# links.

.PATH: ${.CURDIR}/arch/${MACHINE} ${.CURDIR}/include

FILES=	dirent.h endian.h errno.h math.h netdb.h pthread.h pwd.h \
	resolv.h sched.h signal.h stdio.h stdlib.h string.h syslog.h \
	time.h timers.h tzfile.h unistd.h \
	arpa/inet.h arpa/nameser.h \
	pthread/ac-types.h pthread/cleanup.h pthread/cond.h pthread/config.h \
	pthread/debug_out.h pthread/fd.h pthread/fd_pipe.h pthread/kernel.h \
	pthread/kthread.h pthread/mutex.h pthread/paths.h pthread/posix.h \
	pthread/prio_queue.h pthread/pthread_attr.h pthread/pthread_once.h \
	pthread/queue.h pthread/sleep.h pthread/specific.h pthread/types.h \
	pthread/unistd.h pthread/util.h pthread/version.h pthread/xtypes.h \
	sys/__math.h sys/__path.h sys/__signal.h sys/__stdio.h sys/__stdlib.h \
	sys/__string.h sys/__time.h sys/__unistd.h sys/compat.h sys/dirent.h \
	sys/errno.h sys/timers.h sys/wait.h \
	pthread/state.def

DIRS=	arpa pthread sys

# Machine dependent header file
MFILE= ${.CURDIR}/arch/${MACHINE}/machdep.h

includes:
	@install -d -m 755 ${DESTDIR}/usr/include/pthread
	@-for i in ${DIRS}; do \
	  install -d -m 755 ${DESTDIR}/usr/include/pthread/$$i; \
	done
	@echo installing ${FILES}
	@-for i in ${FILES}; do \
	  install -C -m 644 ${.CURDIR}/include/$$i ${DESTDIR}/usr/include/pthread/$$i; \
	done
	install -C -m 644 ${MFILE} ${DESTDIR}/usr/include/pthread/machdep.h
	rm -rf ${DESTDIR}/usr/include/pthread.h
	ln -s pthread/pthread.h ${DESTDIR}/usr/include/pthread.h
	@chown -R ${BINOWN}:${BINGRP} ${DESTDIR}/usr/include/pthread
	@chmod -R a-w ${DESTDIR}/usr/include/pthread
