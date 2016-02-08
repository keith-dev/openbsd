%{
/* $OpenBSD: keynote-ver.l,v 1.6 1999/10/09 19:47:32 angelos Exp $ */
/*
 * The author of this code is Angelos D. Keromytis (angelos@dsl.cis.upenn.edu)
 *
 * This code was written by Angelos D. Keromytis in Philadelphia, PA, USA,
 * in April-May 1998
 *
 * Copyright (C) 1998, 1999 by Angelos D. Keromytis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software. 
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHORS MAKES NO
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <ctype.h>

#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include "z.tab.h"
#include "keynote.h"
#include "header.h"
%}
vstring		[a-zA-Z0-9][a-zA-Z0-9_]*
litstring	\"(([^\"\n])|(\\[\"\n\f\r\t.]))*\"
comment         "#"[^\n]*
%s FIRSTPART MIDDLEPART SECONDPART KEYSTATE
%pointer
%option noyywrap yylineno never-interactive
%%

<MIDDLEPART>"="               {
				BEGIN(SECONDPART);
			        return EQ;
			      }
<FIRSTPART>{vstring}          { kvlval.s.string = calloc(strlen(kvtext) + 1,
							 sizeof(char));
                                if (kvlval.s.string == (char *) NULL)
				{
				    keynote_errno = ERROR_MEMORY;
				    return -1;
				}
				strcpy(kvlval.s.string, kvtext);
				BEGIN(MIDDLEPART);
                                return VSTRING;
                              }
<KEYSTATE,SECONDPART>{litstring} { kvlval.s.string = calloc(strlen(kvtext) - 1,
				  		         sizeof(char));
                                   if (kvlval.s.string == (char *) NULL)
				   {
				       keynote_errno = ERROR_MEMORY;
				       return -1;
				   }
				   mystrncpy(kvlval.s.string, kvtext + 1,
					     strlen(kvtext) - 2);
				   BEGIN(FIRSTPART);
				   return STRING;
                                 }
<FIRSTPART,KEYSTATE>{comment} ;
[ \t\n]		              ;
.                             { keynote_errno = ERROR_SYNTAX; return -1; }

%%

/*
 * Return RESULT_TRUE if character is octal digit, RESULT_FALSE otherwise.
 */
static int
is_octal(char c)
{
    switch (c)
    {
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	    return RESULT_TRUE;

	default:
	    return RESULT_FALSE;
    }
}

/*
 * Return octal value (non-zero) if argument starts with such a
 * representation, otherwise 0.
 */
static unsigned char
get_octal(char *s, int len, int *adv)
{
    unsigned char res = 0;

    if (*s == '0')
    {
	if (len > 0)
	{
	    if (is_octal(*(s + 1)))
	    {
		res = *(s + 1) - '0';
		*adv = 2;

		if (is_octal(*(s + 2)) && (len - 1 > 0))
		{
		    res = res * 8 + (*(s + 2) - '0');
		    *adv = 3;
		}
	    }
	}
    }
    else
      if (is_octal(*s) && (len - 1 > 0))  /* Non-zero leading */
      {
	  if (is_octal(*(s + 1)) &&
	      is_octal(*(s + 2)))
	  {
	      *adv = 3;
	      res = (((*s) - '0') * 64) +
		    (((*(s + 1)) - '0') * 8) +
		    ((*(s + 2)) - '0');
	  }
      }

    return res;
}

/*
 * Copy at most len characters to string s1 from string s2, taking
 * care of escaped characters in the process. String s1 is assumed
 * to have enough space, and be zero'ed.
 */
void
mystrncpy(char *s1, char *s2, int len)
{
    unsigned char c;
    int advance;

    if (len == 0)
      return;

    while (len-- > 0)
    {
        if (*s2 == '\\')
	{
	    s2++;

	    if (len-- <= 0)
	      break;

	    if (*s2 == '\n')
	    {
		while (isspace(*(++s2)) && (len-- > 0))
		  ;
	    }
	    else
	      if ((c = get_octal(s2, len, &advance)) != 0)
	      {
		  len -= advance - 1;
		  s2 += advance;
		  *s1++ = c;
	      }
	      else
		if (*s2 == 'n')  /* Newline */
		{
		    *s1++ = '\n';
		    s2++;
		}
		else
		  if (*s2 == 't')  /* Tab */
		  {
		      *s1++ = '\t';
		      s2++;
		  }
		  else
		    if (*s2 == 'r')  /* Linefeed */
		    {
			*s1++ = '\r';
			s2++;
		    }
		    else
		      if (*s2 == 'f')  /* Formfeed */
		      {
			  *s1++ = '\f';
			  s2++;
		      }
		      else
			if ((*s1++ = *s2++) == 0)
			  break;

	    continue;
	}

        if ((*s1++ = *s2++) == 0)
	  break;
     }
}

/*
 * Parse a file that contains environment variable/value pairs.
 * Return -1 on failure.
 */
int
read_environment(char *filename)
{
    YY_BUFFER_STATE kvfoo;
    FILE *fp;
    int i;

    fp = fopen(filename, "r");
    if (fp == (FILE *) NULL)
    {
	perror(filename);
	return -1;
    }
 
    kvfoo = kv_create_buffer(fp, YY_BUF_SIZE);
    kv_switch_to_buffer(kvfoo);
    BEGIN(FIRSTPART);
    i = kvparse();
    kv_delete_buffer(kvfoo);
    fclose(fp);
    switch (i)
    {
	case 0:
	    return 0;
	    
	default:
	    if (keynote_errno == ERROR_MEMORY)
	      fprintf(stderr,
		      "Memory error while processing environment file <%s>\n",
		      filename);
	    else
	      fprintf(stderr,
		     "Syntax error in environment file <%s>, line %d\n",
		     filename, kvlineno);
	    return -1;
    }
}

/*
 * Parse a key.
 */
void
parse_key(char *buf)
{
    YY_BUFFER_STATE key_state;
    int err;

    key_state = kv_scan_string(buf);
    BEGIN(KEYSTATE);
    err = kvparse();
    kv_delete_buffer(key_state);

    if (err != 0)
      if (keynote_errno == 0)
	keynote_errno = ERROR_SYNTAX;
}