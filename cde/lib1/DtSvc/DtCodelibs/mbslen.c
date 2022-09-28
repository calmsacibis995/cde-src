/*
 * File:	mbslen.c $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include "mbstring.h"
#include <codelibs/nl_hack.h>

#if defined(__cplusplus) || defined(__STDC__)
int
mbslen(const char *str)
#else
int
mbslen(str)
register unsigned char *str;
#endif
{
    register int len = 0;

    for (; *str != '\0'; len++)
	ADVANCE(str);

    return len;
}
