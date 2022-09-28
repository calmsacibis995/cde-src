/*
 * File:         Message.h $XConsortium: Message.h /main/cde1_maint/1 1995/07/14 20:35:33 drk $
 * Language:     C
 *
 * (c) Copyright 1990, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _Dt_message_h
#define _Dt_message_h

#include <Dt/DataTypes.h>
#include <Dt/DtP.h>

#include <Dt/Service.h>

/* This header file is a modifed version of <Xv/Message.h> that 
 * provides compatibility between the BMS messaging and the new
 * ICCCM messaging used in DT.
 *
 * The following symbols that were defined in <Xv/Message.h> are
 * no longer available.  Any code that depended on them will have
 * to be rewritten:
 *
 *	DtServerDeathFn
 *	DtCloseMsgServerConnect
 *	DtAddFailNotificationCallback
 *	DtStatusResponse
 *	DtUniqueRequestId
 *	DtSendFailNotification
 *	DtSendMsg
 */


/**********************************
 *
 * Message Format
 *
 **********************************/

/*
 * DT messages have the following format:
 *
 *	Request message: 	<request> [args ...]
 *	Reply message:		<SUCCESS | FAILURE> [args ...]
 *	Notify message:		<notification> [args ...]
 *
 * The following fields in BMS messages are no longer supported:
 *
 *	DT_MSG_SENDER
 *	DT_MSG_REQUEST_ID
 *	DT_MSG_TOOL
 *	DT_MSG_HOST
 *	DT_MSG_DIR
 *	DT_MSG_FILE
 */

#define DT_MSG_TYPE		0
#define DT_MSG_COMMAND		0
#define DT_MSG_DATA_1		1
#define DT_MSG_DATA_2		2
#define DT_MSG_DATA_3		3
#define DT_MSG_DATA_4		4
#define DT_MSG_DATA_5		5
#define DT_MSG_DATA_6		6
#define DT_MSG_DATA_7		7
#define DT_MSG_DATA_8		8
#define DT_MSG_DATA_9		9
#define DT_MSG_DATA_10		10
#define DT_MSG_DATA_11		11
#define DT_MSG_DATA_12		12
#define DT_MSG_DATA_13		13
#define DT_MSG_DATA_14		14
#define DT_MSG_DATA_15		15
#define DT_MSG_DATA_16		16
#define DT_MSG_DATA_17		17
#define DT_MSG_DATA_18		18
#define DT_MSG_DATA_19		19
#define DT_MSG_DATA_20		20

#define DtDONT_CARE_FIELD		"*"
/*
	If a particular message does not require a value in one of the
	fields, use DtDONT_CARE_FIELD for the value of the field.
*/

/*********************************
 *
 * Receiving Messages 
 *
 *********************************/

/*
    GENERAL INFORMATION:
    
	Applications receive messages by calling one of the "DtAdd...Callback"
	functions (or _DtSendRequestCatchResponse which is listed in the
	"Sending Messages" section).  The arguments to the DtAdd...Callback 
	functions specify both the message pattern to be received and the
	callback (supplied by your application) that is to receive it.
	
	Message patterns are made up of either strings which must exactly
	match the corresponding field of the message, or DtDONT_CARE_FIELD
	which will match all values of the corresponding field.
*/

#ifdef _NO_PROTO

extern void _DtSendReady ();
extern int _DtAddRequestCallback ();
extern String _DtSendRequest ();
extern int _DtSendResponse();
extern int _DtSendRequestCatchResponse ();
extern void _DtAddNotificationCallback ();
extern int _DtSendSuccessNotification ();

#else
	/* Use Prototypes */
extern void _DtSendReady (
     String tool_class);
extern int _DtAddRequestCallback (
     String tool_class,
     String command,
     DtSvcReceiveProc callback_proc,
     void *client_data,
     int num_data_fields,
     ...);
extern String _DtSendRequest (
     String tool_class,
     String command,
     ...);
extern int _DtSendResponse (
    Boolean		is_success,
    Boolean		echo_data,
    DtSvcMsgContext	message,
    String		response_info,
    ... );
extern int _DtSendRequestCatchResponse (
    String		tool_class,
    String		command,
    DtSvcMessageProc	success_callback_proc,
    caddr_t		success_data,
    int 		num_success_fields,
    DtSvcMessageProc	fail_callback_proc,
    caddr_t		fail_data,
    int 		num_fail_fields,
    ...);
extern void _DtAddNotificationCallback (
    String		tool_class,
    String		command,
    DtSvcMessageProc	callback_proc,
    caddr_t		client_data,
    int			num_data_fields,
    ...);
extern int _DtSendSuccessNotification (
    String		tool_class,
    String		request_id,
    String		command,
    ...);

#endif	/* _NO_PROTO */

#endif /*_Dt_message_h*/
/* Do not add anything after this endif. */
