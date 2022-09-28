/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* -*-C-*-
**************************************************************************
*
* File:         Chkpnt.h
* Description:  CDE Private header file. Private API for sending checkpoint
*		messages between compliant clients and the checkpoint service
*		provider. This API is designed for use by performance
*		measurement programs.
*
* Created:      Mon Sep  6 09:00 1993
* Language:     C
*
* $XConsortium: ChkpntP.h /main/cde1_maint/1 1995/07/14 20:30:52 drk $
*
* (C) Copyright 1993, Hewlett-Packard, all rights reserved.
*
**************************************************************************
*/

#ifndef _Dt_Perf_Checkpoint_P_h
#define _Dt_Perf_Checkpoint_P_h

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>

/*************************************************************************/
/************* Data types ************************************************/
/*************************************************************************/

/* The following definition is ONLY meant for union that follows */
typedef struct {
    char	*pname;		/* Client program name			*/
    char	*window;	/* Window Id for client			*/
    char 	*type;		/* Type of message			*/
    char	*count;		/* Running count of messages		*/
    char        *seconds;	/* Time in seconds from gettimeofday()	*/
    char	*message;	/* Actual message			*/
} _DtChkpntMsgFormat;

#define DT_PERF_CHKPNT_MSG_SIZE (sizeof(_DtChkpntMsgFormat) / sizeof(char *))
/* Use the following union for actual message declaration */
typedef union {
    _DtChkpntMsgFormat record;
    char               *array[DT_PERF_CHKPNT_MSG_SIZE];
} DtChkpntMsg;

#define DT_PERF_CHKPNT_MSG_INIT  	"Init"
#define DT_PERF_CHKPNT_MSG_CHKPNT	"Chkpnt"
#define DT_PERF_CHKPNT_MSG_END		"End"

/*************************************************************************/
/************* Atom Names ************************************************/
/*************************************************************************/
/* Selection for ICCCM style interaction of client and listener*/
/* This selection is owned by the listener */
#define DT_PERF_CHKPNT_SEL		"_DT_PERF_CHKPNT_SEL"

/* Properties attached to the client: Used for message transmission */
#define DT_PERF_CLIENT_CHKPNT_PROP	"_DT_PERF_CHKPNT_PROP"

/*************************************************************************/
/************* Client Functions ******************************************/
/*************************************************************************/

/* Note: It is expected that users will invoke the following functions within
   #ifdef DT_PERFORMANCE directives. The DT_PERFORMANCE flag should be set,
   in the build environment, for the performance-test-enabled builds.
*/

/* Initialize the checkpointing mechanism */
extern _DtPerfChkpntInit(
#ifndef _NO_PROTO
    Display	   *display,	   /* Display pointer			   */
    Window	   parentwin,	   /* Parent window id                     */
    char           *prog_name,	   /* Name of the client program (argv[0]) */
    Boolean        bChkpnt 	   /* Boolean: True or False		   */
#endif
);

/* Send a checkpoint message to the listener */
extern _DtPerfChkpntMsgSend(
#ifndef _NO_PROTO
    char           *message	   /* Acual message for transmission     */
#endif
);

/* End the checkpointing message delivery */
extern _DtPerfChkpntEnd(
#ifndef _NO_PROTO
#endif
);

/*************************************************************************/
/************* Listener Service Functions ********************************/
/*************************************************************************/

/* Initialize the listener */
extern _DtPerfChkpntListenInit(
#ifndef _NO_PROTO
    Display *display,	/* Current display */
    Window parentwin	/* Parent of window associated with listener */
#endif
);

/* Fetch a message from message queue */
extern Bool _DtPerfChkpntMsgReceive(
#ifndef _NO_PROTO
DtChkpntMsg *dtcp_msg,		/* Above message available as a structure */
Bool        bBlock		/* Block until a message is received ?    */
#endif
);

#endif /*_Dt_Perf_Checkpoint_h*/
/* Do not add anything after this endif. */
