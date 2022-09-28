/*
 * File:         nls.c $XConsortium: nls.c /main/cde1_maint/1 1995/07/14 20:16:03 drk $
 * Language:     C
 *
 * (c) Copyright 1989, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <bms/sbport.h> /* NOTE: sbport.h must be the first include. */

#include <bms/bms.h>

#ifndef CDE_LOGFILES_TOP
#define CDE_LOGFILES_TOP "/var/dt/tmp"
#endif

/*------------------------------------------------------------------------+*/
XeString
XeSBTempPath(XeString suffix)
/*------------------------------------------------------------------------+*/
{
    XeString dir   = (XeString) CDE_LOGFILES_TOP;
    XeString path;

    if (!suffix || !*suffix)
	return strdup(dir);

    path = (XeString)XeMalloc(strlen(dir) + strlen(suffix) + 2 );

    strcpy(path, dir);
    strcat(path, "/");
    strcat(path, suffix);

    return path;
}
