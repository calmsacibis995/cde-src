/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	lint
#ifdef	VERBOSE_REV_INFO
	/* add any additional revision info here */
#endif	/* VERBOSE_REV_INFO */
#endif	/* lint */

/********************************************************

Copyright (c) 1988, 1990 by Hewlett-Packard Company
Copyright (c) 1988 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that the above copyright notice appear in all 
copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the names of 
Hewlett-Packard or  M.I.T.  not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.

********************************************************/

#include <X11/hpversion.h>
/*#include <X11/dtversion.h>*/

#ifndef	lint
hpversion(dtgreet:       $XConsortium: vgversion.c /main/cde1_maint/2 1995/10/23 09:24:49 gtsang $)
/*dtversion();*/

#ifdef __AFS
static char _afs_version[] = "@(#)AFS 3 Authentication";
#endif

#ifdef __KERBEROS
static char _krb_version[] = "@(#)Kerberos Authentication";
#endif

#ifdef __PASSWD_ETC
static char _rgy_version[] = "@(#)Passwd Etc. - Domain Registry Authentication";
#endif

#ifdef BLS
static char _bls_version[] = "@(#)HP BLS Authentication";
#endif

#endif	/* lint */







