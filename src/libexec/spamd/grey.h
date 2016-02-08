/*	$OpenBSD: grey.h,v 1.3 2004/02/26 08:30:01 beck Exp $	*/

/*
 * Copyright (c) 2004 Bob Beck.  All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define MAX_MAIL 1024 /* how big an email address will we consider */
#define PASSTIME (60 * 30) /* pass after first retry seen after 30 mins */
#define GREYEXP (60 * 60 * 4) /* remove grey entries after 4 hours */
#define WHITEEXP (60 * 60 * 24 * 36) /* remove white entries after 36 days */
#define PATH_PFCTL "/sbin/pfctl"
#define DB_SCAN_INTERVAL 60
#define PATH_SPAMD_DB "/var/db/spamd"

struct gdata {
	time_t first;  /* when did we see it first */
	time_t pass;   /* when was it whitelisted */
	time_t expire; /* when will we get rid of this entry */
	int bcount;    /* how many times have we blocked it */
	int pcount;    /* how many good connections have we seen after wl */
};

extern int greywatcher(void);