/* $RCSfile: util.h,v $$Revision: 1.1.1.1 $$Date: 1996/08/19 10:13:37 $
 *
 *    Copyright (c) 1991, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: util.h,v $
 * Revision 1.1.1.1  1996/08/19 10:13:37  downsj
 * Import of Perl 5.003 into the tree.  Makefile.bsd-wrapper and
 * config.sh.OpenBSD are the only local changes.
 *
 */

/* is the string for makedir a directory name or a filename? */

#define fatal Myfatal

#define MD_DIR 0
#define MD_FILE 1

#ifdef SETUIDGID
    int		eaccess();
#endif

char	*getwd();
int	makedir();

char * cpy2 _(( char *to, char *from, int delim ));
char * cpytill _(( char *to, char *from, int delim ));
void croak _(( char *pat, int a1, int a2, int a3, int a4 ));
void growstr _(( char **strptr, int *curlen, int newlen ));
char * instr _(( char *big, char *little ));
void Myfatal ();
char * safecpy _(( char *to, char *from, int len ));
char * savestr _(( char *str ));
void warn ();
