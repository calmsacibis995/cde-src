/*
 *  MsgbRecvr.c -- Routines for the broadcast receivers
 *
 *  $XConsortium: MsgbRecvr.c /main/cde1_maint/1 1995/07/14 20:36:13 drk $
 *  $XConsortium: MsgbRecvr.c /main/cde1_maint/1 1995/07/14 20:36:13 drk $
 *
 * (C) Copyright 1993, Hewlett-Packard, all rights reserved.
 */

/*
 * Include Files
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/MsgP.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>

/*
 * Forward references
 */
static void _DtMsgBroadcastCreateListener (
#ifndef _NO_PROTO
	DtMsgHandle		handle
#endif
);

/*
 * _DtHandleBroadcastEvents -- handle broadcast message events
 */
static XtEventHandler
#ifdef _NO_PROTO
_DtHandleBroadcastEvents (widget, client_data, pevent, pbDispatch)
	Widget		widget;
	Pointer	client_data;
	XEvent *	pevent;
	Boolean *	pbDispatch;
#else
_DtHandleBroadcastEvents (
	Widget		widget,
	Pointer	client_data,
	XEvent *	pevent,
	Boolean *	pbDispatch 
	)
#endif
{
    DtMsgHandle handle = (DtMsgHandle) client_data;
    char **ppchMessage;
    int status;
    int count;

    switch (pevent->type)
    {
	case PropertyNotify:

	    if (pevent->xproperty.state == PropertyNewValue)
	    {
		/* get the message from the property */
		status = _DtMsgGetMessage (pevent->xproperty.display,
					    pevent->xproperty.window,
					    pevent->xproperty.atom, 
					    &ppchMessage,
					    &count);

		if (status == dtmsg_SUCCESS)
		{
		    /* call the receive message callback */

		    (*(DtMsgH_BreceiveProc(handle))) 
					(handle, 
					NULL,
					DtMsgH_BclientData(handle),
					ppchMessage,
					count);
		}

		/* free up message data */
		XFreeStringList (ppchMessage);
	    }
	    break;
    }
}

/*
 * _DtMsgBroadcastRegister -- Register interest in a group of broadcast
 *				messages
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastRegister (handle, receive_proc, client_data)
	DtMsgHandle		handle;
	DtMsgReceiveProc	receive_proc;
	Pointer		client_data;
#else
_DtMsgBroadcastRegister (
	DtMsgHandle		handle,
	DtMsgReceiveProc	receive_proc,
	Pointer		client_data 
	)
#endif
{
    int status;
    Window winShared;

    /* get shared window id */
    status = _DtMsgBroadcastGetSharedWindow (handle, &winShared);

    if (status == dtmsg_SUCCESS)
    {
	/* create listener window as child of shared window */
	DtMsgH_SharedWindow(handle) = winShared;

	_DtMsgBroadcastCreateListener (handle);

	/* wire in receive_proc to be called for each message */
	DtMsgH_BreceiveProc(handle) = receive_proc;
	DtMsgH_BclientData(handle) = client_data;

	XtAddEventHandler (DtMsgH_Listener(handle),
		    PropertyChangeMask, False, 
		    (XtEventHandler) _DtHandleBroadcastEvents, 
		    (Pointer) handle);
    }

    return (status);
}


/*
 * _DtMsgBroadcastUnregister -- Unregister interest in a group of broadcast
 *				messages
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastUnregister (handle)
	DtMsgHandle		handle;
#else
_DtMsgBroadcastUnregister (
	DtMsgHandle		handle
	)
#endif
{
    /* stop listening for events */
    XtRemoveEventHandler (DtMsgH_Listener(handle),
		PropertyChangeMask, False, 
		(XtEventHandler) _DtHandleBroadcastEvents, 
		(Pointer) handle);

    /* destroy the widget and window */
    XtDestroyWidget (DtMsgH_Listener(handle));

    /* clean up data in handle */
    DtMsgH_Listener(handle) = None;
    DtMsgH_BreceiveProc(handle) = NULL;
}


/*
 * _DtMsgBroadcastCreateListener -- Unregister interest in a group 
 *				     of broadcast messages
 */
static void
#ifdef _NO_PROTO
_DtMsgBroadcastCreateListener (handle)
	DtMsgHandle		handle;
#else
_DtMsgBroadcastCreateListener (
	DtMsgHandle		handle
	)
#endif
{
    int status;
    Widget widget;
    Arg args[10];
    int n;

    /* 
     * Create a minimal widget and reparent it to the 
     * shared window. We need a window over there.
     * Having it inside a widget allows us to use Xt
     * event handlers for convenience.
     */
    n = 0;
    XtSetArg (args[n], XtNmappedWhenManaged, False);	n++;
    XtSetArg (args[n], XtNx, 0);			n++;
    XtSetArg (args[n], XtNy, 0);			n++;
    XtSetArg (args[n], XtNwidth, 10);			n++;
    XtSetArg (args[n], XtNheight, 10);			n++;
    widget = XtCreateWidget ("dtMsgListener",
			coreWidgetClass,
			DtMsgH_Widget (handle),
			args, n);

    /* force creation of the widget's window */
    XtRealizeWidget (widget);

    /* reparent the window to the shared window */
    XReparentWindow (XtDisplay(widget), XtWindow(widget),
		DtMsgH_SharedWindow(handle), 0, 0);
    XFlush (XtDisplay(widget));

    /* stuff it in our handle */
    DtMsgH_Listener(handle) = widget;
}
