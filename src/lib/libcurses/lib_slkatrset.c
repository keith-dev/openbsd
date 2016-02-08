/*	$OpenBSD: lib_slkatrset.c,v 1.1 1997/12/03 05:21:32 millert Exp $	*/


/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/

/*
 *	lib_slkatrset.c
 *	Soft key routines.
 *      Set the labels attributes
 */
#include <curses.priv.h>

MODULE_ID("Id: lib_slkatrset.c,v 1.2 1997/10/18 18:06:23 tom Exp $")

int
slk_attrset(const attr_t attr)
{
  T((T_CALLED("slk_attrset(%s)"), _traceattr(attr)));

  if (SP!=0 && SP->_slk!=0)
    {
      SP->_slk->attr = attr;
      returnCode(OK);
    }
  else
    returnCode(ERR);
}