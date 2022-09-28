/*
 * File:	buf.C $XConsortium: buf.C /main/cde1_maint/1 1995/07/14 20:04:10 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include <stdio.h>
#include <codelibs/nl_hack.h>
#include "buf.h"

void
_SHXbuf::start_token()
{
    vec[vec.size()] = (char *)buf.size();
    is_pattern = FALSE;
    _new_token = FALSE;
}

void
_SHXbuf::quote(Quote q)
{
    _quote ^= q;
    if (_new_token)
	start_token();
}

void
_SHXbuf::append(int const ch, char flag)
{
    flag |= _quote;

    if (ch == '\0' && !_new_token)
	_new_token = TRUE;
    else if (_new_token)
	start_token();

    if (flag == NOQUOTE)
	if (ch == '*' || ch == '?' || ch == '[')
	    if (glob)
		is_pattern = TRUE;
	    else
		flag |= SINGLEQUOTE;

    if (ch <= 0xFF)
    {
	// 8-bit char
	flags[buf.size()] = flag;
	buf[buf.size()] = (unsigned char)ch;
    }
    else
    {
	// 16-bit char
	long sz = buf.size();

	flags[sz] = flag;
	flags[sz + 1] = flag;
	buf.reset(sz + 2);
	char *cp = &buf.elt(sz);
	WCHAR(ch, cp);
    }

    // expand token into filename(s) if appropriate
    if (ch == '\0' && (is_pattern || completion))
	if (!(flag & EXPANDQUOTE))
	    filegen();
}

void
_SHXbuf::append(char const *cp, char flag)
{
    do
	append((int)CHARAT(cp), flag);
    while (CHARADV(cp) != '\0');
}

void
_SHXbuf::reset(boolean g, boolean comp)
{
    glob = (boolean) !!g;
    completion = (boolean ) !!comp;
    is_pattern = FALSE;
    _new_token = TRUE;
    _quote = NOQUOTE;
    buf.reset(0);
    flags.reset(0);
    vec.reset(0);
}

// Convert the subscripts that are stored in vec[] into
// char pointers.  The only legal operation on _SHXbuf after
// calling vector is to call reset.
char **
_SHXbuf::vector()
{
    for (int i = 0; i < vec.size(); i++)
	vec[i] = &buf[long(vec[i])];

    vec[i] = NULL;

    return vec.getarr();
}
