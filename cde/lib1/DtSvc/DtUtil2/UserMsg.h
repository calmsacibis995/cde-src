/* $XConsortium: UserMsg.h /main/cde1_maint/1 1995/07/17 18:18:13 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* -*-C-*-
********************************************************************************
*
* File:         usermsg.h
* Description:  Header for error logging routines
* Status:       Experimental (Do Not Distribute)
*
* (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
*
********************************************************************************
*/

#ifndef _usermsg_h
#define _usermsg_h

#ifndef _NO_PROTO
#include <X11/Intrinsic.h>
#endif


#ifndef __STDC__
#  ifndef const
#    define const
#  endif
#endif

#include <bms/XeUserMsg.h>

#ifdef __cplusplus
extern "C" {
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif
#endif

typedef enum {
  DtIgnore,
  DtInformation,
  DtWarning,
  DtError,
  DtFatalError,
  DtInternalError
} DtSeverity;

#ifdef _NO_PROTO

extern void _DtSimpleError() ;
extern void _DtSimpleErrnoError() ;
extern Boolean _DtUserMsgHandler();

#else

extern void _DtSimpleError( 
                        char *progName,
                        DtSeverity severity,
                        char *help,
                        char *format,
                        ...) ;
extern void _DtSimpleErrnoError( 
                        char *progName,
                        DtSeverity severity,
                        char *help,
                        char *format,
                        ...) ;
extern Boolean _DtUserMsgHandler(
        		String progName,
        		String help,
        		String message,
        		DtSeverity severity,
        		unsigned int errnum,
        		String errname,
        		String errmsg,
        		XtPointer client_data );

#endif /* _NO_PROTO */

#define DtProgName  XeProgName

#ifdef _NO_PROTO
extern int _DtPrintDefaultError();
#else
extern _DtPrintDefaultError(
        Display *dpy,
        XErrorEvent *event,
        char *msg );
#endif

/*
    DESCRIPTION:

        Supply the standard Xerror output to a buffer (instead of stederr)
        so client can do what it wants with it.  Also don't exit.  Allow
client to decide what to do. 

        An appropriate thing to do would be to pass the buffer to 
        DtSimpleError() and return. ie. In your error callback:

    {
    char errmsg[1024];
    int ret ;
    ret = _DtPrintDefaultError (style.display, rep, errmsg);
    DtSimpleError(DtWarning, "><An X Error has occurred...continuing" ,
                             errmsg, NULL);
    return ret;
    }

    SYNOPSIS:

        int _DtPrintDefaultError (dpy, event, msg)

        Display *dpy;       The application's display structure.

        XErrorEvent *event; Error event returned to error callback.

        char *msg;          Buffer returning the formatted text of 
                            the error message.  It won't be more than
                            1024 bytes.

*/

#ifdef __cplusplus
}
#endif

#endif /* _usermsg_h */
/* DON'T ADD STUFF AFTER THIS #endif */
