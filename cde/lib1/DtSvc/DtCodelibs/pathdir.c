/*
 * File:	pathdir.c $Revision: 1.3 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif

#include <stdio.h>
#include <string.h>
#if defined(apollo) || defined(__aix)
#include <stdlib.h>
#else
#include <malloc.h>
#endif /* apollo */
#include <sys/types.h>
#include <codelibs/mbstring.h>
#include <codelibs/nl_hack.h>
#include "pathutils.h"

char *
pathdir(const char *src, char *dst)
{
    register char *ptr;

    if (dst == NULL)
	if ((dst = (char *)malloc(strlen(src) + 1)) == NULL)
	    return NULL;

    if (dst != src)
	strcpy(dst, src);

    ptr = _mb_srchr(dst, '/');
    if (ptr == NULL)
    {
	ptr = dst;
	*ptr++ = '.';
    }
    else if (ptr == dst)
	ADVANCE(ptr);

    *ptr = '\0';

    return dst;
}
