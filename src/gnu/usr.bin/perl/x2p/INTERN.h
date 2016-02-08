/* $RCSfile: INTERN.h,v $$Revision: 1.5 $$Date: 2001/05/24 18:36:39 $
 *
 *    Copyright (c) 1991-2001, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log: INTERN.h,v $
 * Revision 1.5  2001/05/24 18:36:39  millert
 * merge in perl 5.6.1 with our local changes
 *
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
