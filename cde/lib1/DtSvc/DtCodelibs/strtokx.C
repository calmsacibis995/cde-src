/*
 * File:	strtokx.C $XConsortium: strtokx.C /main/cde1_maint/1 1995/07/14 20:09:03 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include <codelibs/nl_hack.h>
#include <codelibs/mbstring.h>
#include "stringx.h"

char *
strtokx(register char *&ptr, register const char *sep)
{
    if (ptr == NULL)
	return NULL;

    // find the beginning of the token
    register char *ret = ptr;
    while (*ret != '\0' && _mb_schr(sep, *ret) != NULL)
	ADVANCE(ret);

    // find the end of the token
    register char *end = ret;
    while (*end != '\0' && _mb_schr(sep, *end) == NULL)
	ADVANCE(end);

    ptr = end;

    // If this isn't the last token, advance pointer and terminate
    // current token.
    if (*end != '\0')
    {
	ADVANCE(ptr);
	WCHAR('\0', end);
    }

    if (*ret == '\0')
	return NULL;

    return ret;
}
