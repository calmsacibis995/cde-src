/* $XConsortium: Dt.h /main/cde1_maint/3 1995/11/07 16:32:34 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef _Dt_Dt_h
#define _Dt_Dt_h

#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constants
 */

/* CDE Version information */

#define DtVERSION     		1
#define DtREVISION    		0
#define DtUPDATE_LEVEL    	10

#define DtVERSION_NUMBER	(DtVERSION * 10000 +  \
				DtREVISION * 100 + \
				DtUPDATE_LEVEL)

#define DtVERSION_STRING "CDE Version 1.0.10"


/* Solaris CDE Version information */

#define SDtVERSION     		DtVERSION
#define SDtREVISION    		2
#define SDtUPDATE_LEVEL    	0

#define SDtVERSION_NUMBER	(SDtVERSION * 10000 +  \
				SDtREVISION * 100 + \
				SDtUPDATE_LEVEL)

#define SDtVERSION_STRING "CDE Version 1.2"

/*
 * DO-not use this #define, use the global 'const char * SDtCopyrightYears'
 * This is just defined here as an initalizer for Dt.c
 *
 * As the copyright needs to be updated in the product, we can
 * just update the years here in this one file for all of CDE.
 */
#define SDtCOPYRIGHT_YEARS "1993-1997"

/* From the old StandardM.h file */

#define DtMSG_SERVER_TOOL_CLASS "MSG-SERVER"

/* Special case that means "all tools" */

#define DtALL_TOOL_CLASS "-"

/* Define for the stop message (which requests that a tool quit) */

#define DtSTOP "STOP"

/* Define for the generic message telling tools that
 * the session is going down.  This is a (N)otification message.
 */
#define DtEND_SESSION "END_SESSION"

/* Command and data used for status requests and responses.  */

#define DtSTATUS_COMMAND		"STATUS"
#define DtSTATUS_READY_ALL_FTYPES	"* READY"
#define DtSTATUS_BUSY_ALL_FTYPES	"* BUSY"


/*
 * Data
 */

/* CDE Version information */

externalref const int DtVersion;
externalref const char *DtVersionString;

/* Solaris CDE Version information */

externalref const int SDtVersion;
externalref const char *SDtVersionString;

/* Copyright information */
extern const char *SDtCopyrightYears; 

/*
 * Functions
 */

extern Boolean DtInitialize(
		Display		*display,
		Widget		widget,
		char		*name,
		char		*tool_class);

extern Boolean DtAppInitialize(
		XtAppContext	app_context,
		Display		*display,
		Widget		widget,
		char		*name,
		char		*tool_class);

#ifdef __cplusplus
}
#endif

#endif /* _Dt_Dt_h */
