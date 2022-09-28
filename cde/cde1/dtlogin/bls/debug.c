/*
 * xdm - display manager daemon
 *
 * $XConsortium: error.c,v 1.11 89/10/31 14:31:10 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

 /***************
    debug.c
 ****************/

#ifndef NDEBUG
/* don't compile anything in this file unless this is pre-release code */
#include <stdio.h> 
#include <signal.h>
#include "../vg.h"
#include "bls.h"

#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif

char *DisplayName=NULL;


/****************************************************************************
 *
 *  Debug
 *
 *  Write a debug message to stderr
 *
 ****************************************************************************/

static int  DoName=TRUE;
static int  debugLevel=0;


int
#ifdef _NO_PROTO
BLS_ToggleDebug(arg)
	int arg;
#else
BLS_ToggleDebug( int arg)
#endif	/* _NO_PROTO_ */
{
	debugLevel = !debugLevel;
	(void) signal(SIGHUP,BLS_ToggleDebug);
}



void 
#ifdef _NO_PROTO
/*VARARGS1*/
Debug( fmt, va_alist )
        char *fmt ;
	va_dcl
#else
Debug( char *fmt, ...)
#endif /* _NO_PROTO */
{
	static int sentinel = 0;
	static char *debugLog;

    va_list  args;

    Va_start(args,fmt);



    if ( !sentinel ) {
    /* 
     * open up an error log for dtgreet
     */
	if ((debugLog = getenv("VG_DEBUG")) == 0)
		debugLog = "/usr/lib/X11/dt/Dtlogin/dtgreet.log";

	if ( !freopen(debugLog,"a",stderr)) {
		perror("Debug:");
	}
	DisplayName=dpyinfo.name;
	sentinel = 1;
    }

    if (debugLevel > 0)
    {
	if ( strlen(DisplayName) > 0 && DoName)
	    fprintf(stderr, "(%s) ", DisplayName);

	vfprintf (stderr,fmt, args);
	fflush (stderr);

	/*
	 * don't prepend the display name next time if this debug message
	 * does not contain a "new line" character...
	 */

	if ( strchr(fmt,'\n') == NULL )
	    DoName=FALSE;
	else
	    DoName=TRUE;
	    
    }

    va_end(args);
}

#else

/*
 * Debug stub for product purposes
 */

void 
Debug( )
{ }

#endif	/* NDEBUG */
