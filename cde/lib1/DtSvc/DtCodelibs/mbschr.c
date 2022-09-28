/*
 * File:	mbschr.c $XConsortium: mbschr.c /main/cde1_maint/1 1995/07/14 20:06:56 drk $
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
_mb_schr(const char *str, wchar_t ch)
#else
char *
_mb_schr(str, ch)
register unsigned char *str;
register wchar_t ch;
#endif
{
    for (; *str != '\0'; ADVANCE(str))
	if (CHARAT(str) == ch)
	    return (char *)str;

    return (char *)0;
}
