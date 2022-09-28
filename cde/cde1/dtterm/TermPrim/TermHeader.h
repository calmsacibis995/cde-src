/*
 * $XConsortium: TermHeader.h /main/cde1_maint/3 1995/10/09 11:48:07 pascale $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_Dt_TermHeader_h
#define	_Dt_TermHeader_h
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef __osf__
#include <sys/param.h>	/* for MIN, MAX macros */
#endif /* __osf__ */
#include <Xm/Xm.h>
#include <X11/Xos.h>

#if defined(UXPArchitecture) || (defined(USL) && (OSMAJORVERSION > 1))
#define _NFILE   FOPEN_MAX
#endif

#ifndef	MIN
#define	MIN(a,b)	(((a) > (b)) ? (b) : (a))
#endif	/* MIN */
#ifndef	MAX
#define	MAX(a,b)	(((a) < (b)) ? (b) : (a))
#endif	/* MAX */
#endif	/* _Dt_TermHeader_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
