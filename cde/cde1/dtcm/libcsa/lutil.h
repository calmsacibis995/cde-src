/* $XConsortium: lutil.h /main/cde1_maint/1 1995/07/17 20:00:19 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _LUTIL_H
#define _LUTIL_H

#pragma ident "@(#)lutil.h	1.7 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

extern char *_DtCmGetPrefix(char *str, char sep);

extern char *_DtCmGetLocalHost();

extern char *_DtCmGetHostAtDomain();

extern char *_DtCmGetUserName();

extern boolean_t _DtCmIsUserName(char *user);

#endif
