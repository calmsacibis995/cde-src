/*
 * File:         usersig.h $XConsortium: usersig.h /main/cde1_maint/1 1995/07/14 20:15:06 drk $
 * Language:     C
 *
 * (c) Copyright 1989, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _usersig_h
#define _usesig_h

/* -------------------------------------------- */
/* Requires:					*/
#ifdef __recursive_includes
#include <signal.h>
#endif
/* -------------------------------------------- */

#define XE_SIG_NOT_IN_TABLE -2

extern int XeNameToSignal
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
    (XeString name);
#else
    ();
#endif

extern XeString XeSignalToName
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
    (int sig);
#else
    ();
#endif

#endif /* _usersig_h */
