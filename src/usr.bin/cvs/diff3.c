/*	$OpenBSD: diff3.c,v 1.35 2007/02/22 06:42:09 otto Exp $	*/

/*
 * Copyright (C) Caldera International Inc.  2001-2002.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 *    copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed or owned by Caldera
 *	International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*-
 * Copyright (c) 1991, 1993
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
 *
 *	@(#)diff3.c	8.1 (Berkeley) 6/6/93
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char rcsid[] =
    "$OpenBSD: diff3.c,v 1.35 2007/02/22 06:42:09 otto Exp $";
#endif /* not lint */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cvs.h"
#include "diff.h"

/* diff3 - 3-way differential file comparison */

/* diff3 [-ex3EX] d13 d23 f1 f2 f3 [m1 m3]
 *
 * d13 = diff report on f1 vs f3
 * d23 = diff report on f2 vs f3
 * f1, f2, f3 the 3 files
 * if changes in f1 overlap with changes in f3, m1 and m3 are used
 * to mark the overlaps; otherwise, the file names f1 and f3 are used
 * (only for options E and X).
 */

/*
 * "from" is first in range of changed lines; "to" is last+1
 * from=to=line after point of insertion for added lines.
 */
struct range {
	int from;
	int to;
};

struct diff {
	struct range old;
	struct range new;
};

static size_t szchanges;

static struct diff *d13;
static struct diff *d23;

/*
 * "de" is used to gather editing scripts.  These are later spewed out in
 * reverse order.  Its first element must be all zero, the "new" component
 * of "de" contains line positions or byte positions depending on when you
 * look (!?).  Array overlap indicates which sections in "de" correspond to
 * lines that are different in all three files.
 */
static struct diff *de;
static char *overlap;
static int overlapcnt = 0;
static FILE *fp[3];
static int cline[3];		/* # of the last-read line in each file (0-2) */

/*
 * the latest known correspondence between line numbers of the 3 files
 * is stored in last[1-3];
 */
static int last[4];
static int eflag;
static int oflag;		/* indicates whether to mark overlaps (-E or -X)*/
static int debug  = 0;
static char f1mark[MAXPATHLEN], f3mark[MAXPATHLEN];	/* markers for -E and -X */

static int duplicate(struct range *, struct range *);
static int edit(struct diff *, int, int);
static char *getchange(FILE *);
static char *getline(FILE *, size_t *);
static int number(char **);
static size_t readin(char *, struct diff **);
static int skip(int, int, char *);
static int edscript(int);
static int merge(size_t, size_t);
static void change(int, struct range *, int);
static void keep(int, struct range *);
static void prange(struct range *);
static void repos(int);
static void separate(const char *);
static void increase(void);
static int diff3_internal(int, char **, const char *, const char *);

int diff3_conflicts = 0;

void
cvs_merge_file(struct cvs_file *cf, int verbose)
{
	int argc;
	char *data, *patch;
	char *argv[5], r1[16], r2[16];
	char *dp13, *dp23, *path1, *path2, *path3;
	BUF *b1, *b2, *b3, *d1, *d2, *diffb;
	size_t dlen, plen;
	struct cvs_line *lp;
	struct cvs_lines *dlines, *plines;

	b1 = b2 = b3 = d1 = d2 = diffb = NULL;
	rcsnum_tostr(cf->file_ent->ce_rev, r1, sizeof(r1));
	rcsnum_tostr(cf->file_rcsrev, r2, sizeof(r2));

	b1 = cvs_buf_load_fd(cf->fd, BUF_AUTOEXT);
	d1 = cvs_buf_alloc((size_t)128, BUF_AUTOEXT);
	d2 = cvs_buf_alloc((size_t)128, BUF_AUTOEXT);
	diffb = cvs_buf_alloc((size_t)128, BUF_AUTOEXT);

	(void)close(cf->fd);
	cf->fd = open(cf->file_path, O_WRONLY | O_TRUNC);
	if (cf->fd == -1) {
		fatal("cvs_merge_file: failed to reopen fd for writing: %s",
		    strerror(errno));
	}

	(void)xasprintf(&path1, "%s/diff1.XXXXXXXXXX", cvs_tmpdir);
	(void)xasprintf(&path2, "%s/diff2.XXXXXXXXXX", cvs_tmpdir);
	(void)xasprintf(&path3, "%s/diff3.XXXXXXXXXX", cvs_tmpdir);

	cvs_buf_write_stmp(b1, path1, NULL);
	if (verbose == 1)
		cvs_printf("Retrieving revision %s\n", r1);
	rcs_rev_write_stmp(cf->file_rcs, cf->file_ent->ce_rev, path2, 0);
	if (verbose == 1)
		cvs_printf("Retrieving revision %s\n", r2);
	rcs_rev_write_stmp(cf->file_rcs, cf->file_rcsrev, path3, 0);

	cvs_diffreg(path1, path3, d1);
	cvs_diffreg(path2, path3, d2);

	(void)xasprintf(&dp13, "%s/d13.XXXXXXXXXX", cvs_tmpdir);
	cvs_buf_write_stmp(d1, dp13, NULL);
	cvs_buf_free(d1);

	(void)xasprintf(&dp23, "%s/d23.XXXXXXXXXX", cvs_tmpdir);
	cvs_buf_write_stmp(d2, dp23, NULL);
	cvs_buf_free(d2);

	argc = 0;
	diffbuf = diffb;
	argv[argc++] = dp13;
	argv[argc++] = dp23;
	argv[argc++] = path1;
	argv[argc++] = path2;
	argv[argc++] = path3;

	diff3_conflicts = diff3_internal(argc, argv, cf->file_path, r2);
	if (diff3_conflicts < 0)
		fatal("cvs_merge_file: merging failed for an unknown reason");

	plen = cvs_buf_len(diffb);
	patch = cvs_buf_release(diffb);
	dlen = cvs_buf_len(b1);
	data = cvs_buf_release(b1);

	cvs_printf("Merging differences between %s and %s into `%s'\n",
	    r1, r2, cf->file_path);

	dlines = cvs_splitlines(data, dlen);
	plines = cvs_splitlines(patch, plen);
	ed_patch_lines(dlines, plines);
	cvs_freelines(plines);

	if (verbose == 1 && diff3_conflicts != 0) {
		cvs_log(LP_ERR, "%d conflict%s found during merge, "
		    "please correct.", diff3_conflicts,
		    (diff3_conflicts > 1) ? "s" : "");
	}

	TAILQ_FOREACH(lp, &(dlines->l_lines), l_list) {
		if (lp->l_line == NULL)
			continue;

		if (write(cf->fd, lp->l_line, lp->l_len) == -1)
			fatal("cvs_merge_file: %s", strerror(errno));
	}

	cvs_freelines(dlines);

	if (data != NULL)
		xfree(data);
	xfree(patch);

	(void)unlink(path1);
	(void)unlink(path2);
	(void)unlink(path3);
	(void)unlink(dp13);
	(void)unlink(dp23);

	xfree(path1);
	xfree(path2);
	xfree(path3);
	xfree(dp13);
	xfree(dp23);
}

static int
diff3_internal(int argc, char **argv, const char *fmark, const char *rmark)
{
	size_t m, n;
	int i;

	/* XXX */
	eflag = 3;
	oflag = 1;

	if (argc < 5)
		return (-1);

	(void)xsnprintf(f1mark, sizeof(f1mark), "<<<<<<< %s", fmark);
	(void)xsnprintf(f3mark, sizeof(f3mark), ">>>>>>> %s", rmark);

	increase();
	m = readin(argv[0], &d13);
	n = readin(argv[1], &d23);

	for (i = 0; i <= 2; i++) {
		if ((fp[i] = fopen(argv[i + 2], "r")) == NULL) {
			cvs_log(LP_ERR, "%s", argv[i + 2]);
			return (-1);
		}
	}

	return (merge(m, n));
}

int
ed_patch_lines(struct cvs_lines *dlines, struct cvs_lines *plines)
{
	char op, *ep;
	struct cvs_line *sort, *lp, *dlp, *ndlp, *insert_after;
	int start, end, i, lineno;
	u_char tmp;

	dlp = TAILQ_FIRST(&(dlines->l_lines));
	lp = TAILQ_FIRST(&(plines->l_lines));

	end = 0;
	for (lp = TAILQ_NEXT(lp, l_list); lp != NULL;
	    lp = TAILQ_NEXT(lp, l_list)) {
		/* Skip blank lines */
		if (lp->l_len < 2)
			continue;
		/* NUL-terminate line buffer for strtol() safety. */
		tmp = lp->l_line[lp->l_len - 1];
		lp->l_line[lp->l_len - 1] = '\0';
		/* len - 1 is NUL terminator so we use len - 2 for 'op' */
		op = lp->l_line[strlen(lp->l_line) - 1];
		start = (int)strtol(lp->l_line, &ep, 10);
		/* Restore the last byte of the buffer */
		lp->l_line[lp->l_len - 1] = tmp;
		if (op == 'a') {
			if (start > dlines->l_nblines ||
			    start < 0 || *ep != 'a')
				fatal("ed_patch_lines");
		} else if (op == 'c') {
			if (start > dlines->l_nblines ||
			    start < 0 || (*ep != ',' && *ep != 'c'))
				fatal("ed_patch_lines");

			if (*ep == ',') {
				ep++;
				end = (int)strtol(ep, &ep, 10);
				if (end < 0 || *ep != 'c')
					fatal("ed_patch_lines");
			} else {
				end = start;
			}
		}


		for (;;) {
			if (dlp == NULL)
				break;
			if (dlp->l_lineno == start)
				break;
			if (dlp->l_lineno > start) {
				dlp = TAILQ_PREV(dlp, cvs_tqh, l_list);
			} else if (dlp->l_lineno < start) {
				ndlp = TAILQ_NEXT(dlp, l_list);
				if (ndlp->l_lineno > start)
					break;
				dlp = ndlp;
			}
		}

		if (dlp == NULL)
			fatal("ed_patch_lines");


		if (op == 'c') {
			insert_after = TAILQ_PREV(dlp, cvs_tqh, l_list);
			for (i = 0; i <= (end - start); i++) {
				ndlp = TAILQ_NEXT(dlp, l_list);
				TAILQ_REMOVE(&(dlines->l_lines), dlp, l_list);
				dlp = ndlp;
			}
			dlp = insert_after;
		}

		if (op == 'a' || op == 'c') {
			for (;;) {
				ndlp = lp;
				lp = TAILQ_NEXT(lp, l_list);
				if (lp == NULL)
					fatal("ed_patch_lines");

				if (!memcmp(lp->l_line, ".", 1))
					break;

				TAILQ_REMOVE(&(plines->l_lines), lp, l_list);
				TAILQ_INSERT_AFTER(&(dlines->l_lines), dlp,
				    lp, l_list);
				dlp = lp;

				lp->l_lineno = start;
				lp = ndlp;
			}
		}

		/*
		 * always resort lines as the markers might be put at the
		 * same line as we first started editing.
		 */
		lineno = 0;
		TAILQ_FOREACH(sort, &(dlines->l_lines), l_list)
			sort->l_lineno = lineno++;
		dlines->l_nblines = lineno - 1;
	}

	return (0);
}

/*
 * Pick up the line numbers of all changes from one change file.
 * (This puts the numbers in a vector, which is not strictly necessary,
 * since the vector is processed in one sequential pass.
 * The vector could be optimized out of existence)
 */
static size_t
readin(char *name, struct diff **dd)
{
	int a, b, c, d;
	char kind, *p;
	size_t i;

	fp[0] = fopen(name, "r");
	for (i = 0; (p = getchange(fp[0])); i++) {
		if (i >= szchanges - 1)
			increase();
		a = b = number(&p);
		if (*p == ',') {
			p++;
			b = number(&p);
		}
		kind = *p++;
		c = d = number(&p);
		if (*p==',') {
			p++;
			d = number(&p);
		}
		if (kind == 'a')
			a++;
		if (kind == 'd')
			c++;
		b++;
		d++;
		(*dd)[i].old.from = a;
		(*dd)[i].old.to = b;
		(*dd)[i].new.from = c;
		(*dd)[i].new.to = d;
	}

	if (i) {
		(*dd)[i].old.from = (*dd)[i-1].old.to;
		(*dd)[i].new.from = (*dd)[i-1].new.to;
	}
	(void)fclose(fp[0]);

	return (i);
}

static int
number(char **lc)
{
	int nn;

	nn = 0;
	while (isdigit((unsigned char)(**lc)))
		nn = nn*10 + *(*lc)++ - '0';

	return (nn);
}

static char *
getchange(FILE *b)
{
	char *line;

	while ((line = getline(b, NULL))) {
		if (isdigit((unsigned char)line[0]))
			return (line);
	}

	return (NULL);
}

static char *
getline(FILE *b, size_t *n)
{
	char *cp;
	size_t len;
	static char *buf;
	static size_t bufsize;

	if ((cp = fgetln(b, &len)) == NULL)
		return (NULL);

	if (cp[len - 1] != '\n')
		len++;
	if (len + 1 > bufsize) {
		char *newbuf;
		do {
			bufsize += 1024;
		} while (len + 1 > bufsize);
		newbuf = xrealloc(buf, 1, bufsize);
		buf = newbuf;
	}
	memcpy(buf, cp, len - 1);
	buf[len - 1] = '\n';
	buf[len] = '\0';
	if (n != NULL)
		*n = len;

	return (buf);
}

static int
merge(size_t m1, size_t m2)
{
	struct diff *d1, *d2, *d3;
	int dpl, j, t1, t2;

	d1 = d13;
	d2 = d23;
	j = 0;
	while ((t1 = d1 < d13 + m1) | (t2 = d2 < d23 + m2)) {
		if (debug) {
			printf("%d,%d=%d,%d %d,%d=%d,%d\n",
			d1->old.from, d1->old.to,
			d1->new.from, d1->new.to,
			d2->old.from, d2->old.to,
			d2->new.from, d2->new.to);
		}

		/* first file is different from others */
		if (!t2 || (t1 && d1->new.to < d2->new.from)) {
			/* stuff peculiar to 1st file */
			if (eflag==0) {
				separate("1");
				change(1, &d1->old, 0);
				keep(2, &d1->new);
				change(3, &d1->new, 0);
			}
			d1++;
			continue;
		}

		/* second file is different from others */
		if (!t1 || (t2 && d2->new.to < d1->new.from)) {
			if (eflag==0) {
				separate("2");
				keep(1, &d2->new);
				change(2, &d2->old, 0);
				change(3, &d2->new, 0);
			}
			d2++;
			continue;
		}

		/*
		 * Merge overlapping changes in first file
		 * this happens after extension (see below).
		 */
		if (d1 + 1 < d13 + m1 && d1->new.to >= d1[1].new.from) {
			d1[1].old.from = d1->old.from;
			d1[1].new.from = d1->new.from;
			d1++;
			continue;
		}

		/* merge overlapping changes in second */
		if (d2 + 1 < d23 + m2 && d2->new.to >= d2[1].new.from) {
			d2[1].old.from = d2->old.from;
			d2[1].new.from = d2->new.from;
			d2++;
			continue;
		}
		/* stuff peculiar to third file or different in all */
		if (d1->new.from == d2->new.from && d1->new.to == d2->new.to) {
			dpl = duplicate(&d1->old,&d2->old);
			if (dpl == -1)
				return (-1);

			/*
			 * dpl = 0 means all files differ
			 * dpl = 1 means files 1 and 2 identical
			 */
			if (eflag==0) {
				separate(dpl ? "3" : "");
				change(1, &d1->old, dpl);
				change(2, &d2->old, 0);
				d3 = d1->old.to > d1->old.from ? d1 : d2;
				change(3, &d3->new, 0);
			} else
				j = edit(d1, dpl, j);
			d1++;
			d2++;
			continue;
		}

		/*
		 * Overlapping changes from file 1 and 2; extend changes
		 * appropriately to make them coincide.
		 */
		if (d1->new.from < d2->new.from) {
			d2->old.from -= d2->new.from-d1->new.from;
			d2->new.from = d1->new.from;
		} else if (d2->new.from < d1->new.from) {
			d1->old.from -= d1->new.from-d2->new.from;
			d1->new.from = d2->new.from;
		}
		if (d1->new.to > d2->new.to) {
			d2->old.to += d1->new.to - d2->new.to;
			d2->new.to = d1->new.to;
		} else if (d2->new.to > d1->new.to) {
			d1->old.to += d2->new.to - d1->new.to;
			d1->new.to = d2->new.to;
		}
	}

	return (edscript(j));
}

static void
separate(const char *s)
{
	diff_output("====%s\n", s);
}

/*
 * The range of lines rold.from thru rold.to in file i is to be changed.
 * It is to be printed only if it does not duplicate something to be
 * printed later.
 */
static void
change(int i, struct range *rold, int fdup)
{
	diff_output("%d:", i);
	last[i] = rold->to;
	prange(rold);
	if (fdup || debug)
		return;
	i--;
	(void)skip(i, rold->from, NULL);
	(void)skip(i, rold->to, "  ");
}

/*
 * print the range of line numbers, rold.from thru rold.to, as n1,n2 or n1
 */
static void
prange(struct range *rold)
{
	if (rold->to <= rold->from)
		diff_output("%da\n", rold->from - 1);
	else {
		diff_output("%d", rold->from);
		if (rold->to > rold->from+1)
			diff_output(",%d", rold->to - 1);
		diff_output("c\n");
	}
}

/*
 * No difference was reported by diff between file 1 (or 2) and file 3,
 * and an artificial dummy difference (trange) must be ginned up to
 * correspond to the change reported in the other file.
 */
static void
keep(int i, struct range *rnew)
{
	int delta;
	struct range trange;

	delta = last[3] - last[i];
	trange.from = rnew->from - delta;
	trange.to = rnew->to - delta;
	change(i, &trange, 1);
}

/*
 * skip to just before line number from in file "i".  If "pr" is non-NULL,
 * print all skipped stuff with string pr as a prefix.
 */
static int
skip(int i, int from, char *pr)
{
	size_t j, n;
	char *line;

	for (n = 0; cline[i] < from - 1; n += j) {
		if ((line = getline(fp[i], &j)) == NULL)
			return (-1);
		if (pr != NULL)
			diff_output("%s%s", pr, line);
		cline[i]++;
	}
	return ((int) n);
}

/*
 * Return 1 or 0 according as the old range (in file 1) contains exactly
 * the same data as the new range (in file 2).
 */
static int
duplicate(struct range *r1, struct range *r2)
{
	int c,d;
	int nchar;
	int nline;

	if (r1->to-r1->from != r2->to-r2->from)
		return (0);
	(void)skip(0, r1->from, NULL);
	(void)skip(1, r2->from, NULL);
	nchar = 0;
	for (nline=0; nline < r1->to - r1->from; nline++) {
		do {
			c = getc(fp[0]);
			d = getc(fp[1]);
			if (c == -1 || d== -1)
				return (-1);
			nchar++;
			if (c != d) {
				repos(nchar);
				return (0);
			}
		} while (c != '\n');
	}
	repos(nchar);
	return (1);
}

static void
repos(int nchar)
{
	int i;

	for (i = 0; i < 2; i++)
		(void)fseek(fp[i], (long)-nchar, 1);
}

/*
 * collect an editing script for later regurgitation
 */
static int
edit(struct diff *diff, int fdup, int j)
{
	if (((fdup + 1) & eflag) == 0)
		return (j);
	j++;
	overlap[j] = !fdup;
	if (!fdup)
		overlapcnt++;
	de[j].old.from = diff->old.from;
	de[j].old.to = diff->old.to;
	de[j].new.from = de[j-1].new.to + skip(2, diff->new.from, NULL);
	de[j].new.to = de[j].new.from + skip(2, diff->new.to, NULL);
	return (j);
}

/* regurgitate */
static int
edscript(int n)
{
	int j, k;
	char block[BUFSIZ+1];

	for (n = n; n > 0; n--) {
		if (!oflag || !overlap[n])
			prange(&de[n].old);
		else
			diff_output("%da\n=======\n", de[n].old.to -1);
		(void)fseek(fp[2], (long)de[n].new.from, 0);
		for (k = de[n].new.to-de[n].new.from; k > 0; k-= j) {
			j = k > BUFSIZ ? BUFSIZ : k;
			if (fread(block, (size_t)1, (size_t)j,
			    fp[2]) != (size_t)j)
				return (-1);
			block[j] = '\0';
			diff_output("%s", block);
		}

		if (!oflag || !overlap[n])
			diff_output(".\n");
		else {
			diff_output("%s\n.\n", f3mark);
			diff_output("%da\n%s\n.\n", de[n].old.from - 1, f1mark);
		}
	}

	return (overlapcnt);
}

static void
increase(void)
{
	struct diff *p;
	char *q;
	size_t newsz, incr;

	/* are the memset(3) calls needed? */
	newsz = szchanges == 0 ? 64 : 2 * szchanges;
	incr = newsz - szchanges;

	p = xrealloc(d13, newsz, sizeof(*d13));
	memset(p + szchanges, 0, incr * sizeof(*d13));
	d13 = p;
	p = xrealloc(d23, newsz, sizeof(*d23));
	memset(p + szchanges, 0, incr * sizeof(*d23));
	d23 = p;
	p = xrealloc(de, newsz, sizeof(*de));
	memset(p + szchanges, 0, incr * sizeof(*de));
	de = p;
	q = xrealloc(overlap, newsz, sizeof(*overlap));
	memset(q + szchanges, 0, incr * sizeof(*overlap));
	overlap = q;
	szchanges = newsz;
}