/*
 * File:	pathbase.c $Revision: 1.3 $
 *
 */
/*
 *		COPYRIGHT (c) HEWLETT-PACKARD COMPANY, 1984
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
/*
 *	pathbase() - very much like basename(1)
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <codelibs/mbstring.h>
#include <codelibs/nl_hack.h>
#include "pathutils.h"

/*
 *	return pointer to basename within name[..]
 *
 *	There are no error returns.  You will not like it if you call
 *	this routine with '/' as the last char in the string but it does
 *	the right thing.
 */
#ifdef __cplusplus
char *pathbase(char *name)
#else
char *pathbase(const char *name)
#endif
{
    register const char *ptr;

    ptr = _mb_srchr(name, '/');

    if (ptr == NULL)
	ptr = name;
    else
	ADVANCE(ptr);		/* point to char after / */

    return (char *)ptr;
}
