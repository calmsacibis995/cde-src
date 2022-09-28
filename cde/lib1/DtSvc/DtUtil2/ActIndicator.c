/* $XConsortium: ActIndicator.c /main/cde1_maint/1 1995/07/17 18:08:19 drk $ */

/******************************************************************************
 *
 * File Name: Indicator.c
 *
 *  This file defines the API for interacting with the activity indicator.
 *
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 *
 *****************************************************************************/

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#define TOOLKIT
#include <Dt/DtP.h>
#include <Dt/IndicatorM.h>
#include <Tt/tttk.h>


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void _DtSendActivityNotification() ;
extern void _DtSendActivityDoneNotification() ;

#else

extern void _DtSendActivityNotification( int ) ;
extern void _DtSendActivityDoneNotification( void ) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/



/*
 * _DtSendActivityNotification()
 *
 *   This function provides the client with a means for broadcasting
 *   notification that an activity has been started.  For the present
 *   time, this will enable the activity indicator for upto a specified 
 *   number of seconds.
 */

void 
#ifdef _NO_PROTO
_DtSendActivityNotification( duration )
        int duration ;
#else
_DtSendActivityNotification(
        int duration )
#endif /* _NO_PROTO */
{
    Tt_message	msg;
    Tt_status	status;

    msg = tt_pnotice_create(TT_SESSION, "DtActivity_Beginning");
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
	return;
    }
    tt_message_destroy(msg);
}


/*
 * _DtSendActivityDoneNotification()
 *
 *   This function provides the client with a means for broadcasting
 *   notification that an activity which had earlier been started, is
 *   now down.
 */

void 
#ifdef _NO_PROTO
_DtSendActivityDoneNotification()
#else
_DtSendActivityDoneNotification( void )
#endif /* _NO_PROTO */
{
    Tt_message	msg;
    Tt_status	status;

    msg = tt_pnotice_create(TT_SESSION, "DtActivity_Began");
    status = tt_ptr_error(msg);
    if (status != TT_OK) {
	return;
    }
    status = tt_message_send(msg);
    if (status != TT_OK) {
	return;
    }
    tt_message_destroy(msg);
}
