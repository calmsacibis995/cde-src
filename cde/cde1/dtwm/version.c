#ifndef	lint
#ifdef	VERBOSE_REV_INFO
	/* add any additional revision info here */
#endif	/* VERBOSE_REV_INFO */
#endif	/* lint */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
*/ 

#ifdef hpux
#include <X11/hpversion.h>

#ifndef	lint
hpversion(dtwm:          $XConsortium: version.c /main/cde1_maint/2 1995/10/09 13:46:38 pascale $)
#endif	/* lint */

#else  /* hpux */

#ifndef	lint
static char rcsid[] = "$XConsortium: version.c /main/cde1_maint/2 1995/10/09 13:46:38 pascale $";
#endif /* lint */
#endif /* hpux */

#ifdef __osf__
#include <limits.h>
#include <nl_types.h>
nl_catd Xm_catd;
#endif /* __osf__ */
