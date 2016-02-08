/* $RCSfile: INTERN.h,v $$Revision: 1.7 $$Date: 2003/12/03 03:02:53 $
 *
 *    Copyright (C) 1993, 1994, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: INTERN.h,v $
 * Revision 1.7  2003/12/03 03:02:53  millert
 * Resolve conflicts for perl 5.8.2, remove old files, and add OpenBSD-specific scaffolding
 *
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
