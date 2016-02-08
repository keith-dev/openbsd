/*	$OpenBSD: preadv.c,v 1.6 2003/06/11 21:03:10 deraadt Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
static char rcsid[] = "$OpenBSD: preadv.c,v 1.6 2003/06/11 21:03:10 deraadt Exp $";
#endif /* SYSLIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>

/*
 * This function provides 64-bit offset padding that
 * is not supplied by GCC 1.X but is supplied by GCC 2.X.
 */
ssize_t
preadv(int fd, const struct iovec *iovp, int iovcnt, off_t offset)
{
	extern off_t __syscall();
	quad_t q;
	int rv;

	q = __syscall((quad_t)SYS_preadv, fd, iovp, iovcnt, 0, offset);
	if (/* LINTED constant */ sizeof (quad_t) == sizeof (register_t) ||
	    /* LINTED constant */ BYTE_ORDER == LITTLE_ENDIAN)
		rv = (int)q;
	else
		rv = (int)((u_quad_t)q >> 32);
	return rv;
}
