/* $RCSfile: INTERN.h,v $$Revision: 1.2 $$Date: 1997/11/30 08:06:57 $
 *
 *    Copyright (c) 1991-1997, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: INTERN.h,v $
 * Revision 1.2  1997/11/30 08:06:57  millert
 * perl 5.004_04
 *
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
