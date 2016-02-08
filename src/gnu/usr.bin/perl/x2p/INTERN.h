/* $RCSfile: INTERN.h,v $$Revision: 1.1.1.1 $$Date: 1996/08/19 10:13:32 $
 *
 *    Copyright (c) 1991, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: INTERN.h,v $
 * Revision 1.1.1.1  1996/08/19 10:13:32  downsj
 * Import of Perl 5.003 into the tree.  Makefile.bsd-wrapper and
 * config.sh.OpenBSD are the only local changes.
 *
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
