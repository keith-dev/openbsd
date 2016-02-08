/* $RCSfile: INTERN.h,v $$Revision: 1.4 $$Date: 2000/04/06 17:09:13 $
 *
 *    Copyright (c) 1991-1997, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: INTERN.h,v $
 * Revision 1.4  2000/04/06 17:09:13  millert
 * perl-5.6.0 + local changes
 *
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
