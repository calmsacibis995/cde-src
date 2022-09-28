/* $XConsortium: scan.c /main/cde1_maint/1 1995/07/17 22:09:27 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Scan.c contains scanner procedures for program ELTDEF */

#include <stdio.h>
#include <string.h>
#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"
#include "trie.h"

#include "dtdext.h"

#include "eltdef.h"
#define M_CONDEF
#include "context.h"
#define M_DELIMDEF
#include "delim.h"

/* Reads a name token */
void getname(first)
int first;
{
M_WCHAR *p, wus;
int c, cttype;
LOGICAL cname;

mbtowc(&wus, "_", 1);

cname = (LOGICAL) (curcon == INPARAM || curcon == INVALUE);
*(p = name) = (M_WCHAR) first;
if (! cname) *p = m_ctupper(*p);
while (TRUE)
    {
    c = getachar();
    if (c == EOF) break;
    *++p = (M_WCHAR) c;
    cttype = m_cttype(*p);
    if (! cname)
	{
	if (cttype == M_NONNAME) break;
	}
    else
	{
	if ((cttype != M_NMSTART) && (cttype != M_DIGIT) && (*p != wus))
	    break;
	}
    if (p >= name + M_NAMELEN)
	{
	*(name + M_NAMELEN) = M_EOS;
	m_error("Element name too long");
	}
    if (! cname) *p = m_ctupper(*p);
    }
ungetachar(c);
*p = M_EOS;
}

/* Reads the next token and returns it to the main procedure */
int scan(M_NOPAR)
{
int c;
int n;
static char unexp[] = "c";
M_WCHAR wus;

mbtowc(&wus, "_", 1);

while (TRUE)
    {
    while ((n = gettoken(&c, COMCON)) == STARTCOMMENT)
	while ((n = gettoken(&c, INCOM)) != ENDCOMMENT)
	    if (c == EOF)
		{
		m_error("EOF occurred within comment");
		done();
		exit(TRUE);
		}
    ungetachar(c);
    n = gettoken(&c, curcon);
    if (n)
	{
	if (n != LIT && n != LITA) return(n);
	if (litproc(n))
	    {
	    if (scantrace)
		{
		char *mbyte;

		mbyte = MakeMByteString(literal);
		printf("literal '%s'\n", mbyte);
		m_free(mbyte, "multi-byte string");
		}
	    return(LITERAL);
	    }
	}
    if (c == EOF) return(ENDFILE);
    if (m_newcon(curcon - 1, TEXT - 1))
	{
	textchar = (M_WCHAR) c;
	return(TEXT);
	}
    if (m_whitespace((M_WCHAR) c)) continue;
    if (m_newcon(curcon - 1, NAME - 1))
	{
	if (curcon == INPARAM || curcon == INVALUE)
	    {
	    /* Check for C identifier */
	    if (m_letter((M_WCHAR) c) || c == wus)
		{
		getname(c);
		return(NAME);
		}
	    }
	else
	    {
	    /* Check for SGML name */
	    if ((m_cttype(c) == M_NMSTART) ||
	        (m_cttype(c) != M_NONNAME && curcon == VALUE))
		{
		getname(c);
		return(NAME);
		}
	    }
	}
    if (curcon != ERROR)
	{
	m_mberr1("Unexpected character: '%s'", unexp);
	}
    } /* End while */
}   /* End scan */

#include "scanutil.c"

#if defined(sparse)
#include "sparse.c"
#endif
