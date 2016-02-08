%{
/*
 * Copyright (c) 1995, 1996, 1997, 1998 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Kungliga Tekniska
 *      H�gskolan and its contributors.
 * 
 * 4. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
RCSID("$Id: lex.l,v 1.1.1.1 1998/09/14 21:53:27 art Exp $");
#endif

#undef ECHO

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <roken.h>
#include "sym.h"
#include "types.h"
#include "parse.h"
#include "lex.h"
#include "output.h"

static unsigned lineno;

static char filename[256];

static void parse_filename (char *s);
static void parse_lineno (char *s);
%}

%%
^\#[ ][0-9]+[ ]\"[^\n\"]*\".*$	{ parse_filename (yytext); }
^\#line[ ][0-9]+[ ]\"[^\n\"]*\".*$	{ parse_filename (yytext); }
^\#[ ][0-9]+			{ parse_lineno (yytext); }
^\#line[ ][0-9]+		{ parse_lineno (yytext); }
^\#ident.*$			{ }
const				{ return T_CONST; }
enum				{ return T_ENUM; }
struct				{ return T_STRUCT; }
typedef				{ return T_TYPEDEF; }
unsigned			{ return T_UNSIGNED; }
long				{ return T_LONG; }
int32_t				{ return T_LONG; }
u_long				{ return T_ULONG; }
u_int32_t			{ return T_ULONG; }
short				{ return T_SHORT; }
int16_t				{ return T_SHORT; }
u_short				{ return T_USHORT; }
u_int16_t			{ return T_USHORT; }
int				{ return T_INT; }
u_char				{ return T_UCHAR; }
char				{ return T_CHAR; }
string				{ return T_STRING; }
opaque				{ return T_OPAQUE; }
package				{ return T_PACKAGE; }
split				{ return T_SPLIT; }
multi				{ return T_MULTI; }
IN				{ return T_IN; }
OUT				{ return T_OUT; }
INOUT				{ return T_INOUT; }
ASIS				{ return T_ASIS; }
"["|"]"|[,;=()<>]|"{"|"}"|"*"	{ return *yytext; }
^\%[^\n]*$			{ yylval.name = strdup (yytext+1); return T_VERBATIM; }
-?[0-9]+			{ yylval.constant = atoi(yytext); return T_CONSTANT; }
0[Xx][0-9a-fA-F]+		{ yylval.constant = (int)strtol(yytext+2, NULL, 0x10); return T_CONSTANT; }
[A-Za-z][A-Za-z0-9_]*		{
Symbol *sym;

sym = findsym(yytext);
yylval.sym = sym;
if (sym == NULL) {
    yylval.name = strdup(yytext);
    return T_IDENTIFIER;
} else if (sym->type == TCONST)
    return T_IDCONST;
else if (sym->type == TTYPEDEF || sym->type == TENUM || sym->type == TSTRUCT)
    return T_IDTYPE;
else
    error_message ("Ignoring \"%s\"\n", yytext);
}
[ \t]				;
\n				{ lineno++; }
.				{ error_message("Ignoring char(%c)\n", *yytext); }
%%

#ifndef yywrap
int
yywrap (void)
{
     return 1;
}
#endif /* !yywrap */

void
error_message (char *format, ...)
{
     va_list args;

     va_start (args, format);
     fprintf (stderr, "%s:%d: ", filename, lineno);
     vfprintf (stderr, format, args);
     va_end (args);
}

static void
parse_filename (char *s)
{
     char *d1, *d2;

     while (!isspace(*s))
	 ++s;
     while (isspace(*s))
	 ++s;

     lineno = atoi (s);
     d1 = strchr (s, '"') + 1;
     d2 = strchr (d1, '"');
     *d2 = '\0';
     if (strcmp (d1, "") != 0)
	 strcpy (filename, d1);
}

static void
parse_lineno (char *s)
{
     while (!isspace(*s))
	 ++s;
     while (isspace(*s))
	 ++s;

     lineno = atoi (s);
}