/*	$OpenBSD: config.c,v 1.9 2007/02/22 06:42:09 otto Exp $	*/
/*
 * Copyright (c) 2006 Joris Vink <joris@openbsd.org>
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

#include <sys/param.h>
#include <sys/dirent.h>
#include <sys/resource.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cvs.h"
#include "config.h"

void
cvs_parse_configfile(void)
{
	FILE *fp;
	size_t len;
	struct rlimit rl;
	const char *errstr;
	char *p, *buf, *lbuf, *opt, *val, fpath[MAXPATHLEN];

	(void)xsnprintf(fpath, sizeof(fpath), "%s/%s",
	    current_cvsroot->cr_dir, CVS_PATH_CONFIG);

	cvs_log(LP_TRACE, "cvs_parse_configfile(%s)", fpath);

	if ((fp = fopen(fpath, "r")) == NULL)
		fatal("cvs_config_parse: %s: %s",
		    CVS_PATH_CONFIG, strerror(errno));

	lbuf = NULL;
	while ((buf = fgetln(fp, &len))) {
		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
		} else {
			lbuf = xmalloc(len + 1);
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}

		p = buf;
		while (*p == ' ')
			p++;

		if (p[0] == '#' || p[0] == '\0') {
			if (lbuf != NULL)
				xfree(lbuf);
			continue;
		}

		opt = p;
		if ((val = strrchr(opt, '=')) == NULL)
			fatal("cvs_parse_configfile: bad option '%s'", opt);

		*(val++) = '\0';

		if (!strcmp(opt, "tag")) {
			cvs_tagname = xstrdup(val);
		} else if (!strcmp(opt, "umask")) {
			cvs_umask = (int)strtonum(val, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				fatal("cvs_parse_configfile: %s: %s", val,
				    errstr);
		} else if (!strcmp(opt, "dlimit")) {
			if (getrlimit(RLIMIT_DATA, &rl) != -1) {
				rl.rlim_cur = (int)strtonum(val, 0, INT_MAX,
				    &errstr);
				if (errstr != NULL)
					fatal("cvs_parse_configfile: %s: %s",
					    val, errstr);
				rl.rlim_cur = rl.rlim_cur * 1024;
				(void)setrlimit(RLIMIT_DATA, &rl);
			}
		} else {
			cvs_log(LP_ERR, "ignoring unknown option '%s'", opt);
		}

		if (lbuf != NULL)
			xfree(lbuf);
	}

	(void)fclose(fp);
}