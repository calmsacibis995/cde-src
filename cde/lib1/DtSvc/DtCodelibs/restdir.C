/*
 * File:	restdir.C $Revision: 1.3 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <codelibs/mbstring.h>
#include <codelibs/nl_hack.h>
#include "pathutils.h"


char *
restdirname(const char *name)
{
    register const char *ptr;

    if ((ptr = _mb_schr(name, '/')) == NULL)
	return NULL;

    ADVANCE(ptr);
    return (char *)ptr;
}
