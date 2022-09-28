/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* $XConsortium: Init.h /main/cde1_maint/1 1995/07/14 20:35:05 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:       	Init.h
 **
 **  Project:  		HP DT Developer's Kit
 **
 **  Description:	External declarations for Dt initializations.
 **  -----------
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/

#ifndef _Init_h
#define _Init_h

/* If <X11/Intrinsic.h> is included along with this file, <X11/Intrinsic.h>
   MUST be included first due to "typedef Boolean ..." problems.  The following
   conditional #define is also part of the solution to this problem. */
#ifdef _XtIntrinsic_h
#	define TOOLKIT
#endif

/*********************************************************************
 *
 * Function:	Boolean _DtInit (Display *display, 
 *				char * name);
 *				
 * Parameters:  display		The X server display.
 *
 *              name		Name used to invoke this application, 
 *				typically argv[0].  This is used to
 *				identify error messages.
 *
 * Return value:  Returns TRUE if the client is successfully initialized,
 *		  FALSE otherwise.
 *
 * Purpose:  Initialize a Dt client.  Uses the default Xt app-context
 *	     created by XtInitialize.
 *
 *********************************************************************/
#ifndef _NO_PROTO
extern Boolean _DtInit(Display *,
		      char *);
#else
extern Boolean _DtInit();
#endif

/*********************************************************************
 *
 * Function:	Boolean _DtAppInit (XtAppContext appContext,
 *		                   Display *display,
 *		                   char * name);
 *				
 * Parameters:  appContext	The application context obtained from
 *				XtCreateApplicationContext
 *
 *              display		The X server display.
 *
 *              name		Name used to invoke this application, 
 *				typically argv[0].  This is used to
 *				identify error messages.
 *
 * Return value:  Returns TRUE if the client is successfully initialized,
 *		  FALSE otherwise.
 *
 * Purpose:  Initialize a Dt client in the specified app-context.  NOTE
 *           that multiple app-contexts are not supported.
 *
 *********************************************************************/
#ifndef _NO_PROTO
extern Boolean _DtAppInitXtAppContext,
			 Display *,
			 char *);
#else
extern Boolean _DtAppInit();
#endif

#endif /* _Init_h */
/* Do not add anything after this endif. */
