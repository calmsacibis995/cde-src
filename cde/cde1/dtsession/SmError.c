/* $XConsortium: SmError.c /main/cde1_maint/3 1995/10/09 10:56:46 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmError.c
 **
 **  Project:     HP DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains all session manager error functions.  The session
 **  manager traps all errors from the toolkit and server, and takes action
 **  depending on the type of the error.
 **
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

#include <stdio.h>
#ifdef _SUN_OS
#include <string.h>
#endif
#include <X11/Intrinsic.h>
#include <Dt/UserMsg.h>
#include "Sm.h"
#include "SmError.h"
#include "SmGlobals.h"


/*
 * Global variables
 */
NlsStrings   		smNLS;

/*
 * Local functions
 */
#ifdef _NO_PROTO

static int LibError() ;
static int LibIOError() ;
static void ToolkitWarning() ;
static void ToolkitError() ;

#else

static int LibError( Display *, XErrorEvent *) ;
static int LibIOError( void ) ;
static void ToolkitWarning( char *) ;
static void ToolkitError( char *) ;

#endif


/*************************************<->*************************************
 *
 *  InitErrorHandler ()
 *
 *
 *  Description:
 *  -----------
 *  Initialize all error handlers for use with the session manager
 *  session manager should only exit on real severe conditions.
 *  it should try to gracefully recover on the rest.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitErrorHandler()
#else
InitErrorHandler( void )
#endif /* _NO_PROTO */
{
    XSetErrorHandler(LibError);
    XSetIOErrorHandler( (IOErrorHandlerProc) LibIOError);
    XtSetWarningHandler(ToolkitWarning);
    XtSetErrorHandler(ToolkitError);
}


/*************************************<->*************************************
 *
 *  LibError (display, errorEvent)
 *
 *
 *  Description:
 *  -----------
 *  X error handler.  Takes care of X errors so that the server will
 *  not terminate the session manager on any error.
 *
 *
 *  Inputs:
 *  ------
 *  errorEvent = pointer to error event returned by the server.
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
LibError( display, errorEvent )
        Display *display ;
        XErrorEvent *errorEvent ;
#else
LibError(
        Display *display,
        XErrorEvent *errorEvent )
#endif /* _NO_PROTO */
{
#ifdef DEBUG
    switch (errorEvent->error_code)
    {
      case Success:
	break;
      case BadAccess:
	PrintError(DtError, BAD_ACCESS);
	break;
      case BadAtom:
	PrintError(DtError, BAD_ATOM);	
	break;
      case BadDrawable:
	PrintError(DtError, BAD_DRAWABLE);
	break;
      case BadMatch:
	PrintError(DtError, BAD_MATCH);
	break;
      case BadValue:
	PrintError(DtError, BAD_VALUE);
	break;
      case BadWindow:
	PrintError(DtError, BAD_WINDOW);
	break;
      default:
	PrintError(DtError, DEFAULT_ERROR);
	break;
    }
#endif /*DEBUG*/
    return 0;
}


/*************************************<->*************************************
 *
 *  LibIOError ()
 *
 *
 *  Description:
 *  -----------
 *  IO error handler.  In charge of handling IO events from the
 *  X server
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static int 
#ifdef _NO_PROTO
LibIOError()
#else
LibIOError( void )
#endif /* _NO_PROTO */
{
    PrintError(DtError, GETMESSAGE(8, 1, "Connection to server lost - exiting."));
    SM_EXIT(-1);
    return 0;
}


/*************************************<->*************************************
 *
 *  ToolkitWarning (message)
 *
 *
 *  Description:
 *  -----------
 *  Handles all toolkit warnings
 *
 *
 *  Inputs:
 *  ------
 *  message = error message sent by toolkit
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ToolkitWarning( message )
        char *message ;
#else
ToolkitWarning(
        char *message )
#endif /* _NO_PROTO */
{
#ifdef DEBUG
    PrintError(DtError, message);
#endif /*DEBUG*/    
}



/*************************************<->*************************************
 *
 *  ToolkitError (message)
 *
 *
 *  Description:
 *  -----------
 *  Handles all toolkit errors
 *
 *
 *  Inputs:
 *  ------
 *  message = error message sent by toolkit
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *  Xt assumes the client will exit when an XtError is generated
 *  so we must exit since the state will be undefined if we
 *  continue
 * 
 * 
 *************************************<->***********************************/
static void 
#ifdef _NO_PROTO
ToolkitError( message )
        char *message ;
#else
ToolkitError(
        char *message )
#endif /* _NO_PROTO */
{

    PrintError(DtError, message);
    SM_EXIT(-1);
}


/*************************************<->*************************************
 *
 *  PrintError (severity, help)
 *
 *
 *  Description:
 *  -----------
 *  Handles the printing of all session manager errors using the dt API
 *  These are simple errors that don't set errno
 *
 *
 *  Inputs:
 *  ------
 *  severity = severity of the error
 *  help = help message to user (what type of error)
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  WARNING: Currently the va_alist parameter is not used in the
 *  DtSimpleError is not used
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
PrintError( severity, help )
        DtSeverity severity ;
        char *help ;
#else
PrintError(
        DtSeverity severity,
        char *help )
#endif /* _NO_PROTO */
{
    _DtSimpleError(DtProgName, severity, NULL, "%.2000s", help);
}


/*************************************<->*************************************
 *
 *  PrintErrnoError (severity, help)
 *
 *
 *  Description:
 *  -----------
 *  Handles the printing of all session manager errors using the dt API
 *  These are simple errors that set errno
 *
 *
 *  Inputs:
 *  ------
 *  severity = severity of the error
 *  help = help message to user (what type of error)
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  WARNING: Currently the va_alist parameter is not used in the
 *  DtSimpleErrnoError is not used
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
PrintErrnoError( severity, help )
        DtSeverity severity ;
        char *help ;
#else
PrintErrnoError(
        DtSeverity severity,
        char *help )
#endif /* _NO_PROTO */
{
    _DtSimpleErrnoError(DtProgName, severity, NULL, "%s", help);
}
