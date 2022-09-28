/*
 * File:	mbsrchr.c $Revision: 1.4 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include "mbstring.h"
#include <codelibs/nl_hack.h>

#if defined(__cplusplus) || defined(__STDC__)
char *
_mb_srchr(const char *str, wchar_t ch)
#else
char *
_mb_srchr(str, ch)
register unsigned char *str;
register wchar_t ch;
#endif
{
    register char *sav = (char *)0;

    for (; *str != '\0'; ADVANCE(str))
	if (CHARAT(str) == ch)
	    sav = (char *)str;

    return sav;
}
