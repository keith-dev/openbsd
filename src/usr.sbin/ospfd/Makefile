#	$OpenBSD: Makefile,v 1.8 2010/05/26 16:44:32 nicm Exp $

PROG=	ospfd
SRCS=	area.c auth.c carp.c control.c database.c hello.c \
	in_cksum.c interface.c iso_cksum.c kroute.c lsack.c \
	lsreq.c lsupdate.c log.c neighbor.c ospfd.c ospfe.c packet.c \
	parse.y printconf.c rde.c rde_lsdb.c rde_spf.c name2id.c

MAN=	ospfd.8 ospfd.conf.5

CFLAGS+= -Wall -I${.CURDIR}
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
YFLAGS=
LDADD+=	-levent -lutil
DPADD+= ${LIBEVENT} ${LIBUTIL}

.include <bsd.prog.mk>
