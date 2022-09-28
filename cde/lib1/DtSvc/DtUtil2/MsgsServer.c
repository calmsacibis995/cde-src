/*
 *  MsgsServer.c -- Routines for the service provider
 *
 *  $XConsortium: MsgsServer.c /main/cde1_maint/1 1995/07/14 20:36:55 drk $
 *  $XConsortium: MsgsServer.c /main/cde1_maint/1 1995/07/14 20:36:55 drk $
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

/*
 * Global data
 */

static Time timeOwned = CurrentTime;

/*
 * _DtHandleSelectionRequestEvents -- handle events on the selection we own
 */
static XtEventHandler
#ifdef _NO_PROTO
_DtHandleSelectionRequestEvents (widget, client_data, pevent, pbDispatch)
	Widget		widget;
	Pointer	client_data;
	XEvent *	pevent;
	Boolean *	pbDispatch;
#else
_DtHandleSelectionRequestEvents (
	Widget		widget,
	Pointer	client_data,
	XEvent *	pevent,
	Boolean *	pbDispatch 
	)
#endif
{
    DtMsgServiceContext *pctx = (DtMsgServiceContext *) client_data;
    DtMsgReplyMessageContext *pReplyCtx;
    Atom aSelection;
    char **ppchMessage;
    int status;
    int count;
    Atom xaREQUEST;

    if (!pctx)
    {
	return;
    }

    aSelection = DtMsgH_Atom (pctx->handle);

    switch (pevent->type)
    {
	case SelectionClear:
	    /* we no longer own the selection, call the callback */
	    if ((aSelection == pevent->xselectionclear.selection) &&
		pctx->lose_proc)
	    {
	        (*(pctx->lose_proc)) (pctx->handle, 
					dtmsg_LOST_SERVICE, 
					pctx->lose_client_data);
	    }
	    break;

	case SelectionRequest:
	    xaREQUEST = XInternAtom (XtDisplay(widget), DT_MSG_XA_REQUEST,
					False);
	    if ((aSelection == pevent->xselectionrequest.selection) &&
	        (pevent->xselectionrequest.target == xaREQUEST))
	    {
		status = _DtMsgGetMessage (XtDisplay(widget), 
				    pevent->xselectionrequest.requestor, 
				    pevent->xselectionrequest.property, 
				    &ppchMessage,
				    &count);

		pReplyCtx = XtNew(DtMsgReplyMessageContext);

		pReplyCtx->handle = pctx->handle;
		pReplyCtx->window = pevent->xselectionrequest.requestor;
		pReplyCtx->target = pevent->xselectionrequest.target;
		pReplyCtx->property = pevent->xselectionrequest.property;

		if ((status == dtmsg_SUCCESS) &&
		    (pctx && pctx->request_proc))
		{
		    (*(pctx->request_proc)) (pctx->handle, 
					    (DtMsgContext) pReplyCtx,
					    pctx->receive_client_data,
					    ppchMessage,
					    count);
		}
		XFreeStringList (ppchMessage);
	    }
	    break;
    }
}



/*
 * _DtMsgServiceOffer -- make a service available
 */
int
#ifdef _NO_PROTO
_DtMsgServiceOffer (handle, seize, request_proc, request_client_data,
			lose_proc, lose_client_data)
	DtMsgHandle		handle;
	Boolean			seize;
	DtMsgReceiveProc	request_proc;
	Pointer			request_client_data;
	DtMsgStatusProc	lose_proc;
	Pointer			lose_client_data;
#else
_DtMsgServiceOffer (
	DtMsgHandle		handle,
	Boolean			seize,
	DtMsgReceiveProc	request_proc,
	Pointer			request_client_data,
	DtMsgStatusProc	lose_proc,
	Pointer			lose_client_data
	)
#endif
{
    DtMsgServiceContext *pctx;
    Atom aSelection = DtMsgH_Atom (handle);
    Widget widget = DtMsgH_Widget (handle);

    /* don't take this service if someone already owns it */
    if (!seize && _DtMsgServiceIsOffered(handle))
    {
	return (dtmsg_ANOTHER_PROVIDER);
    }

    /* Assert ownership of this selection */

    timeOwned = XtLastTimestampProcessed (XtDisplay(widget));
    XSetSelectionOwner (XtDisplay(widget), aSelection,
			XtWindow (widget), timeOwned);

    /* Construct service context */
    pctx = XtNew(DtMsgServiceContext);

    DtMsgH_SvcData(handle) = pctx;
    DtMsgH_Shandle(handle) = handle;
    DtMsgH_RequestProc(handle) = request_proc;
    DtMsgH_ReceiveCD(handle) = request_client_data;
    DtMsgH_LoseProc(handle) = lose_proc;
    DtMsgH_LoseCD(handle) = lose_client_data;


    /* Add event handler to listen for selection events */
    XtAddEventHandler (widget, 0, True, 
		    (XtEventHandler) _DtHandleSelectionRequestEvents, 
		    (Pointer) pctx);
    
    return (dtmsg_SUCCESS);
}

/*
 * _DtMsgServiceReply -- send reply to a requester 
 */
int
#ifdef _NO_PROTO
_DtMsgServiceReply (handle, replyCtx, ppchReply, count)
	DtMsgHandle		handle;
	DtMsgContext		replyCtx;
	char **			ppchRequest;
	int			count;
#else
_DtMsgServiceReply (
	DtMsgHandle		handle,
	DtMsgContext		replyCtx,
	char **			ppchReply,
	int			count 
	)
#endif
{
    int status;
    Atom xaREPLY;
    Time time;
    XSelectionEvent event;
    Atom aSelection = DtMsgH_Atom (handle);
    Widget widget = DtMsgH_Widget (handle);
    DtMsgReplyMessageContext *pReplyCtx;

    /* get the reply context */
    pReplyCtx = (DtMsgReplyMessageContext *) replyCtx;

    /* put the property on the window */
    if (pReplyCtx)
    {
	status = _DtMsgSetMessage (XtDisplay(widget), pReplyCtx->window,
			pReplyCtx->property, ppchReply, count);
    }
    else
    {
	status = dtmsg_FAIL;
    }

    if (status == dtmsg_SUCCESS)
    {
	/* get the time */
	time = XtLastTimestampProcessed (XtDisplay(widget));

	/* Trigger the message transfer */
	event.type = SelectionNotify;
	event.requestor = pReplyCtx->window;
	event.selection = aSelection;
	event.property = pReplyCtx->property;
	event.target = pReplyCtx->target;
	event.time = time;

        XSendEvent (XtDisplay(widget), pReplyCtx->window, True, 0, 
			(XEvent *) &event);

	/* reponded to the request, no longer need the reply context */
	XtFree ((char *) replyCtx);

	/* Force out X events so stuff isn't delayed */
	XSync (XtDisplay(widget), False);
    }

    return (status);
}

/*
 * _DtMsgServiceWithdraw -- make a service unavailable
 */
void
#ifdef _NO_PROTO
_DtMsgServiceWithdraw (handle)
	DtMsgHandle	handle;
#else
_DtMsgServiceWithdraw (
	DtMsgHandle	handle
	)
#endif
{
    DtMsgServiceContext *pctx;
    Widget widget = DtMsgH_Widget (handle);

    /* release ownership of this selection */
    XSetSelectionOwner (XtDisplay(widget), 
			DtMsgH_Atom (handle), 
			None, timeOwned);

    /* Add event handler to listen for selection events */
    pctx = (DtMsgServiceContext *) DtMsgH_SvcData (handle);
    XtRemoveEventHandler (widget, 0, True, 
		    (XtEventHandler) _DtHandleSelectionRequestEvents, 
		    (Pointer) pctx);

    /* free data */
    if (pctx) 
    {
	XtFree ((char *)pctx);
    }
    DtMsgH_SvcData(handle) = NULL;
}


