/* $XConsortium: debug.h /main/cde1_maint/1 1995/07/17 19:56:31 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#pragma ident "@(#)debug.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

#ifdef CM_DEBUG
#define DP(a)	{ printf a ; }
#else
#define DP(a)
#endif /* CM_DEBUG */

extern void		_DtCm_print_tick(long);

extern void		_DtCm_print_errmsg(const char *);

#endif /* _DEBUG_H */
