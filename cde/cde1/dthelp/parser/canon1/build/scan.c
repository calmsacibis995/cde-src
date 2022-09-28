/* $XConsortium: scan.c /main/cde1_maint/1 1995/07/17 21:05:22 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Scan.c contains scanner procedures for program BUILD */

#include <string.h>
#if defined(MSDOS)
#include <process.h>
#endif
#include "build.h"
#define M_CONDEF
#include "context.h"
#define M_DELIMDEF
#include "delim.h"
#include "sref.h"

/* Reads a name */
LOGICAL getname(first)
  int first;
{
M_WCHAR *p;
int c;

*(p = name) = first;
if ((curcon != DEFAULT && curcon != DEFVAL &&
curcon != DEFAULT2 && curcon != DEF2VAL) ||
newpar->type != CDATA)
*p = m_ctupper(*p);
while (TRUE)
    {
    c = getachar();
    *++p = (M_WCHAR) c;
    if (m_cttype(*p) == M_NONNAME) break;
    if (p >= name + M_NAMELEN)
	{
	*(name + M_NAMELEN) = M_EOS;
	m_error("Name too long");
	return(FALSE);
	}
    if ((curcon != DEFAULT && curcon != DEFVAL &&
    curcon != DEFAULT2 && curcon != DEF2VAL) ||
    newpar->type != CDATA)
    *p = m_ctupper(*p);
    }
ungetachar(c);
*p = M_EOS;
return(TRUE);
}

/* Returns the next token to the main procedure */
int scan(M_NOPAR)
{
int c;
int n;
M_WCHAR wsp, wnl, wtb;

mbtowc(&wsp, " ", 1);
mbtowc(&wnl, "\n", 1);
mbtowc(&wtb, "\t", 1);

while (TRUE)
    {
    if (! m_newcon(curcon - 1, TEXT - 1))
	{
	while ((n = gettoken(&c, COMCON)) == COMMENT)
	    while ((n = gettoken(&c, COMCON)) != COMMENT)
		if (c == EOF)
		    {
		    m_error("EOF occurred within comment");
		    exit(ERREXIT);
		    }
	ungetachar(c);
	}
    n = gettoken(&c, curcon);
    if (n)
	{
	if (n != LIT && n != LITA) return(n);
	if (litproc(n))
	    {
	    if (scantrace)
		{
		char *mb_literal;

		mb_literal = MakeMByteString(literal);
		printf("literal '%s'\n", mb_literal);
		m_free(mb_literal, "multibyte string");
		}
	    return(LITERAL);
	    }
	/* Litproc returns FALSE in case of error, when returned have
	   scanned to the end of erroneous rule */
	else continue;
	}
    if (c == wsp || c == wnl || c == wtb)
	{
	if (m_newcon(curcon - 1, TEXT - 1))
	    {
	    scanval = (M_WCHAR) c;
	    return(TEXT);
	    }
	else continue;
	}
    if (m_newcon(curcon - 1, NAME - 1))
	if (m_cttype(c) == M_NMSTART ||
	    (m_cttype(c) != M_NONNAME &&
	      (curcon == DEFAULT || curcon == DEFVAL || curcon == NTNAME ||
	       curcon == DEFAULT2 || curcon == DEF2VAL || curcon == NT2NAME
	       ))
	    )
	    {
	    if (getname(c))
		{
		if (scantrace)
		    {
		    char *mb_name;

		    mb_name = MakeMByteString(name);
		    printf("name '%s'\n", mb_name);
		    m_free(mb_name, "multibyte string");
		    }
		return(NAME);
		}
	    else continue;
	    }
    if (c == EOF) return(ENDFILE);
    if (m_newcon(curcon - 1, TEXT - 1))
	{
	scanval = (M_WCHAR) c;
	return(TEXT);
	}
    if (curcon != ERROR)
	{
	char mbyte[32]; /* bigger than the biggest multibyte char */

	wctomb(mbyte, c);

	fprintf(stderr, "\n'%s' (%d)", mbyte, c);
	fprintf(m_errfile, "\n'%s' (%d)", mbyte, c);
	m_error("Unexpected character");
	}
    } /* End while */
}   /* End scan */

#include "scanutil.c"

#if defined(sparse)
#include "sparse.c"
#endif
