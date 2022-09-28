/*
 *  MsgbSender.c -- Routines for the broadcast message senders
 *
 *  $XConsortium: MsgbSender.c /main/cde1_maint/1 1995/07/14 20:36:28 drk $
 *  $XConsortium: MsgbSender.c /main/cde1_maint/1 1995/07/14 20:36:28 drk $
 *
 * (C) Copyright 1993, Hewlett-Packard, all rights reserved.
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/MsgP.h>
#include <X11/IntrinsicP.h>
#include <X11/Core.h>
#include <X11/CoreP.h>


/*
 * Forward references
 */
void _DtMsgBroadcastAddNewReceiver (
#ifndef _NO_PROTO
	DtMsgHandle		handle,
	Window			window
#endif
);

void _DtMsgBroadcastDeleteReceiverEntry (
#ifndef _NO_PROTO
	DtMsgHandle		handle,
	int			i
#endif
);

/*
 * _DtHandleEventsOnShared -- handle events on the shared window
 */
static XtEventHandler
#ifdef _NO_PROTO
_DtHandleEventsOnShared (widget, client_data, pevent, pbDispatch)
	Widget		widget;
	Pointer	client_data;
	XEvent *	pevent;
	Boolean *	pbDispatch;
#else
_DtHandleEventsOnShared (
	Widget		widget,
	Pointer	client_data,
	XEvent *	pevent,
	Boolean *	pbDispatch 
	)
#endif
{
    DtMsgHandle handle = (DtMsgHandle) client_data;
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr;
    int i;

    pBcData = DtMsgH_BcData (handle);
    if (!pBcData) return;

    switch (pevent->type)
    {
	case DestroyNotify:
	 
	    /*
	     * This receiver is no more. Clean up
	     * data we have on him.
	     */

	    /* find the data for this receiver */
	    pRcvr = pBcData->pReceivers;

	    for (i=0; i<pBcData->numReceivers; i++, pRcvr++)
	    {
		if (XtWindow(pRcvr->widget) == pevent->xdestroywindow.window)
		{
		    _DtMsgBroadcastDeleteReceiverEntry (handle, i);
		    break;
		}
	    }

	    break;

	case CreateNotify:

	    /*
	     * New receiver, initialize.
	     */
	    _DtMsgBroadcastAddNewReceiver (handle, 
					pevent->xcreatewindow.window);
	    break;

	case ReparentNotify:

	    /*
	     * Receiver window has moved into or out of the group.
	     * (Duplicates code from Create and Destroy cases above!)
	     */

	    if (pevent->xreparent.parent == DtMsgH_SharedWindow(handle))
	    {
		/* receiver moved into the group */

		_DtMsgBroadcastAddNewReceiver (handle, 
					pevent->xreparent.window);
	    }
	    else
	    {
		/* receiver moved out of the group */
		/* find the data for this receiver */
		pRcvr = pBcData->pReceivers;

		for (i=0; i<pBcData->numReceivers; i++, pRcvr++)
		{
		    if (XtWindow(pRcvr->widget) == pevent->xreparent.window)
		    {
			_DtMsgBroadcastDeleteReceiverEntry (handle, i);
			break;
		    }
		}
	    }
	    break;
    }
}

/*
 * _DtHandleEventsOnReceivers -- handle events on receiver windows
 */
static XtEventHandler
#ifdef _NO_PROTO
_DtHandleEventsOnReceivers (widget, client_data, pevent, pbDispatch)
	Widget		widget;
	Pointer	client_data;
	XEvent *	pevent;
	Boolean *	pbDispatch;
#else
_DtHandleEventsOnReceivers (
	Widget		widget,
	Pointer	client_data,
	XEvent *	pevent,
	Boolean *	pbDispatch 
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    DtMsgHandle handle = (DtMsgHandle) client_data;
    DtMsgBroadcastPerReceiverData *pRcvr;
    int i, j;


    /*
     * Find the receiver data for this receiver
     */
    pBcData = DtMsgH_BcData (handle);
    if (!pBcData) return;

    pRcvr = pBcData->pReceivers;

    for (i=0; i<pBcData->numReceivers; i++, pRcvr++)
    {
	if (pRcvr->widget == widget)
	{
	    break;
	}
    }

    /* no receiver data!! */
    if (i >= pBcData->numReceivers) return;
    

    switch (pevent->type)
    {
	case PropertyNotify:

	    if (pevent->xproperty.state == PropertyDelete)
	    {
		/* 
		 * This message property can be reused for 
		 * this receiver.
		 */
		for (i=0; i<pRcvr->numPropsUnread; i++)
		{
		    if (pRcvr->propsUnread[i] == pevent->xproperty.atom)
		    {
			/* this is it! copy data down */
			for (j=i+1; j<pRcvr->numPropsUnread; j++, i++)
			{
			    pRcvr->propsUnread[i] = pRcvr->propsUnread[j];
			}
			pRcvr->numPropsUnread--;
		    }
		}
	    }
	    else if (pevent->xproperty.state == PropertyNewValue)
	    {
		/* 
		 * This message property is being used
		 * (possibly by another sender)
		 */

		/* see if it's already in the list */
		for (i=0; i<pRcvr->numPropsUnread; i++)
		{
		    if (pRcvr->propsUnread[i] == pevent->xproperty.atom)
		    {
			/* it's already here! */ 
			break; 
		    } 
		}

		/* only add it if it wasn't there */
		if (i >= pRcvr->numPropsUnread)
		{
		    /* insure enough memory */
		    if (pRcvr->numPropsUnread >= pRcvr->sizePropsUnread)
		    {
			pRcvr->sizePropsUnread += DT_MSG_PROP_INC_AMT;
			pRcvr->propsUnread = (Atom *) 
				XtRealloc ((char *)pRcvr->propsUnread,
					    pRcvr->sizePropsUnread);
		    }

		    /* add this property to the end of the list */
		    pRcvr->propsUnread[pRcvr->numPropsUnread] = 
						pevent->xproperty.atom;
		    pRcvr->numPropsUnread++;
		}
	    }
	    break;
    }
}

/*
 * _DtMsgBroadcastSetPropsUnread -- Set the list of unread props
 *				for a receiver
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastSetPropsUnread (handle, pRcvr)
	DtMsgHandle		handle;
        DtMsgBroadcastPerReceiverData *pRcvr;
#else
_DtMsgBroadcastSetPropsUnread (
	DtMsgHandle		handle,
        DtMsgBroadcastPerReceiverData *pRcvr 
	)
#endif
{
    Atom *props;
    int numProps;
    int size, i;

    props = XListProperties (XtDisplay(DtMsgH_Widget(handle)),
			XtWindow(pRcvr->widget),
			&numProps);
    
    if (props)
    {
	/* allocate enough memory to hold the list of props */
	if (pRcvr->sizePropsUnread < numProps)
	{
	    /* make size an integral number of our increment value */
	    size = (pRcvr->sizePropsUnread + numProps + DT_MSG_PROP_INC_AMT);
	    size -= size % DT_MSG_PROP_INC_AMT;

	    if (pRcvr->sizePropsUnread > 0)
	    {
		pRcvr->propsUnread = (Atom *) XtRealloc (
						(char *) pRcvr->propsUnread,
						size*sizeof(Atom));
	    }
	    else
	    {
		pRcvr->propsUnread = (Atom *) XtMalloc (size*sizeof(Atom));
	    }
	    pRcvr->sizePropsUnread = size;
	}

	/* copy the props into our list */
	for (i=0; i<numProps; i++)
	{
	    pRcvr->propsUnread[i] = props[i];
	}
	pRcvr->numPropsUnread = numProps;

	/* free list returned from XListProperties */
	XFree ((char *) props);
    }
}

/*
 * _DtMsgBroadcastAddNewReceiver -- Add a new receiver for this message
 *				group
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastAddNewReceiver (handle, window)
	DtMsgHandle		handle;
	Window			window;
#else
_DtMsgBroadcastAddNewReceiver (
	DtMsgHandle		handle,
	Window			window
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr;

    pBcData = DtMsgH_BcData (handle);

    /* do initial space allocation if never done before */
    if (pBcData->sizeReceivers == 0)
    {
	pBcData->pReceivers = (DtMsgBroadcastPerReceiverData *)
	    XtMalloc (DT_MSG_PROP_INC_AMT * 
		      sizeof(DtMsgBroadcastPerReceiverData));
	pBcData->sizeReceivers = DT_MSG_PROP_INC_AMT;
    }

    /* bump receiver allocation if we're full */
    if ((pBcData->sizeReceivers - pBcData->numReceivers) <= 0)
    {
	pBcData->sizeReceivers += DT_MSG_PROP_INC_AMT;
	pBcData->pReceivers = (DtMsgBroadcastPerReceiverData *)
	    XtRealloc ((char *) pBcData->pReceivers,
		       (pBcData->sizeReceivers *
		       sizeof(DtMsgBroadcastPerReceiverData)));
    }

    /* use next available space */
    pRcvr = &(pBcData->pReceivers[pBcData->numReceivers]);
    pRcvr->sizePropsUnread = 0;
    pRcvr->numPropsUnread = 0;
    pRcvr->propsUnread = NULL;

    /* create widget, stuff the receiver window id into it */
    pRcvr->widget = _DtMsgW4wCreate (handle, window);

    /* get the initial list of properties existing on the receiver */
    _DtMsgBroadcastSetPropsUnread (handle, pRcvr);

    /* add event handler to track property events */
    _DtMsgW4wAddEventHandler (pRcvr->widget,
		PropertyChangeMask, False, 
		(XtEventHandler) _DtHandleEventsOnReceivers, 
		(Pointer) handle);

    pBcData->numReceivers++;
}


/*
 * _DtMsgBroadcastSenderInitialize -- Do setup necessary 
 * 				before sending any messages
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastSenderInitialize (handle)
	DtMsgHandle		handle;
#else
_DtMsgBroadcastSenderInitialize (
	DtMsgHandle		handle
	)
#endif
{
    int status;
    int i;
    Window root, parent, *children, winShared;
    int numChildren;
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pPerReceiver;

    /* find shared window */
    (void) _DtMsgBroadcastGetSharedWindow (handle, &winShared);
    DtMsgH_SharedWindow(handle) = winShared;

    /* Fill in the rest of the sender data structure */
    pBcData = DtMsgH_BcData (handle);
    pBcData->props = (Atom *) 
	XtMalloc (DT_MSG_PROP_INC_AMT*sizeof(Atom));
    pBcData->sizeProps = DT_MSG_PROP_INC_AMT;

    /* stuff in base message name for first property */
    pBcData->numProps = 1;
    pBcData->props[0] = DtMsgH_PropertyAtom (handle);

    pBcData->numReceivers = 0;
    pBcData->pReceivers = NULL;
    pBcData->sizeReceivers = 0;

    DtMsgH_SharedWidget(handle) = 
		_DtMsgW4wCreate (handle, winShared);

    /*
     * Set up event handler on shared window
     */
    _DtMsgW4wAddEventHandler (DtMsgH_SharedWidget(handle),
		SubstructureNotifyMask, False, 
		(XtEventHandler) _DtHandleEventsOnShared, 
		(Pointer) handle);

    /* Enumerate child windows */
    status = XQueryTree (XtDisplay(DtMsgH_Widget(handle)),
			DtMsgH_SharedWindow(handle),
			&root,
			&parent,
			&children,
			(unsigned int *)&numChildren);
#ifdef DEBUG
    if (status == 0)
    {
	fprintf (stderr, "XQueryTree failed !!\n");
	fflush (stderr);
    }
#endif /* DEBUG */

    /* 
     * Process each child of the shared window
     */
    for (i=0; i<numChildren; i++)
    {
	_DtMsgBroadcastAddNewReceiver (handle, children[i]);
    }

    /* free the children list */
    XFree ((char *) children);

    DtMsgH_BSenderInit(handle) = True;
    
    return (status);
}

/*
 * _DtMsgBroadcastSenderUnintialize -- Undo sender intialization,
 * 				freeing all allocated data
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastSenderUninitialize (handle)
	DtMsgHandle		handle;
#else
_DtMsgBroadcastSenderUninitialize (
	DtMsgHandle		handle
	)
#endif
{
    int i;
    DtMsgBroadcastData *pSender;

    pSender = DtMsgH_BcData (handle);

    if (DtMsgH_BSenderInit(handle) && pSender)
    {
	/* 
	 * Repeatedly delete the last receiver until the list is empty.
	 * (Delete the last one to avoid useless memory copies to fill 
	 *  up the holes)
	 */
	if ((pSender->numReceivers > 0) && pSender->pReceivers)
	{
	    while (pSender->numReceivers > 0)
	    {
		_DtMsgBroadcastDeleteReceiverEntry (handle, 
				    (pSender->numReceivers-1));
	    }

	    XtFree ((char *) pSender->pReceivers);
	}

	XtFree ((char *) pSender->props);
	XtFree ((char *) pSender);
    }
    
}

/*
 * _DtMsgBroadcastDeleteReceiverEntry -- remove a receiver for this message
 *				group
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastDeleteReceiverEntry (handle, iRcvr)
	DtMsgHandle		handle;
	int			iRcvr;
#else
_DtMsgBroadcastDeleteReceiverEntry (
	DtMsgHandle		handle,
	int			iRcvr
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr, *pSrc;
    int nbytes, i;

    pBcData = DtMsgH_BcData (handle);

    /* find receiver data for this window */
    if (iRcvr<pBcData->numReceivers)
    {
	pRcvr = &(pBcData->pReceivers[iRcvr]);
	XtFree ((char *) pRcvr->propsUnread);

	/* If we're not at the end, then move receiver array down */
	i = iRcvr+1;
	if (i < pBcData->numReceivers)
	{
	    /* 
	     * Get pointer to the next (start) location,  
	     * the destination is the receiver we've deleted.
	     */
	    pSrc = &(pBcData->pReceivers[i]);

	    /* compute number of bytes to copy */
	    nbytes = (pBcData->numReceivers - i) * 
		     sizeof (DtMsgBroadcastPerReceiverData);

	    /* move memory (memmove is designed to handle overlaps) */
	    (void) memmove (pRcvr, pSrc, nbytes);

	}
	pBcData->numReceivers--;
    }
}

/*
 * _DtMsgBroadcastDeleteReceiver -- remove a receiver for this message
 *				group
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastDeleteReceiver (handle, window)
	DtMsgHandle		handle;
	Window			window;
#else
_DtMsgBroadcastDeleteReceiver (
	DtMsgHandle		handle,
	Window			window
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr;
    int i;

    pBcData = DtMsgH_BcData (handle);

    /* find receiver data for this window */
    for (i=0; i<pBcData->numReceivers; i++)
    {
	pRcvr = &(pBcData->pReceivers[i]);

	if (window == XtWindow(pRcvr->widget))
	{
	    break;
	}
    }

    if (i < pBcData->numReceivers)
    {
	/* found a match */
	_DtMsgBroadcastDeleteReceiverEntry (handle, i);
    }
}

/*
 * _DtMsgBroadcastGenerateNewPropertyName -- 
 *			Create a new property name, add it to our list
 */
void
#ifdef _NO_PROTO
_DtMsgBroadcastGenerateNewPropertyName (handle)
	DtMsgHandle		handle;
#else
_DtMsgBroadcastGenerateNewPropertyName (
	DtMsgHandle		handle
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    Atom property;
    char *pch;

    pBcData = DtMsgH_BcData (handle);

    /* insure enough memory */
    if (pBcData->numProps >= pBcData->sizeProps)
    {
	pBcData->sizeProps += DT_MSG_PROP_INC_AMT;
	pBcData->props = (Atom *) XtRealloc (
			    (char *)pBcData->props,
			    pBcData->sizeProps * sizeof(Atom));
    }

    /* create new property name */
    pch = XtMalloc (strlen (DtMsgH_PropertyName(handle)) + 6);
    sprintf (pch, "%s%d", DtMsgH_PropertyName(handle), pBcData->numProps);

    /* convert it to an atom */
    property = XInternAtom (XtDisplay(DtMsgH_Widget(handle)), pch, False);

    /* add the atom to our internal list */
    pBcData->props[pBcData->numProps] = property;
    pBcData->numProps++;
}

/*
 * _DtMsgBroadcastGetUnusedPropertyForReceiver -- 
 *			Find a suitable property name that doesn't 
 *			conflict with a property existing on the 
 *			receiver
 */
Atom
#ifdef _NO_PROTO
_DtMsgBroadcastGetUnusedPropertyForReceiver (handle, iRcvr)
	DtMsgHandle		handle;
	int			iRcvr;
#else
_DtMsgBroadcastGetUnusedPropertyForReceiver (
	DtMsgHandle		handle,
	int			iRcvr
	)
#endif
{
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr;
    int i,j;
    Atom property;

    pBcData = DtMsgH_BcData (handle);

    pRcvr = &(pBcData->pReceivers[iRcvr]);

    for (i=0; i<pBcData->numProps; i++)
    {
	property = pBcData->props[i];
	for (j=0; j<pRcvr->numPropsUnread; j++)
	{
	    if (pRcvr->propsUnread[j] == property)
	    {
	       /* found a conflict */
	       break;
	    }
	}

	if (j >= pRcvr->numPropsUnread)
	{
	    /* no conflict with this one */
	    break;
	}
    }

    if (i >= pBcData->numProps)
    {
	/* total conflict, need to come up with a new name */
	_DtMsgBroadcastGenerateNewPropertyName (handle);
	property = 
	    _DtMsgBroadcastGetUnusedPropertyForReceiver (handle, iRcvr);
    }

    return (property);
}
/*
 * _DtMsgBroadcastSend -- Send a broadcast message
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastSend (handle, ppchMessage, count)
	DtMsgHandle		handle;
	char **			ppchMessage;
	int			count;
#else
_DtMsgBroadcastSend (
	DtMsgHandle		handle,
	char **			ppchMessage,
	int			count  
	)
#endif
{
    int status, istatus;
    DtMsgBroadcastData *pBcData;
    DtMsgBroadcastPerReceiverData *pRcvr;
    int i;
    Atom property;

    pBcData = DtMsgH_BcData (handle);

    /* make sure we're initialized for sending */
    if (!DtMsgH_BSenderInit(handle))
    {
	_DtMsgBroadcastSenderInitialize (handle);
    }

    pBcData = DtMsgH_BcData (handle);

    if (pBcData)
    {
	status = dtmsg_SUCCESS;

	/* Send the message to each listener */
	for (i = 0; i < pBcData->numReceivers; i++)
	{
	    pRcvr = &pBcData->pReceivers[i];
	    property = 
		_DtMsgBroadcastGetUnusedPropertyForReceiver (handle, i);

	    istatus = _DtMsgSetMessage (XtDisplay(DtMsgH_Widget(handle)), 
				XtWindow(pRcvr->widget), 
				property, ppchMessage, count);
	    
	    if (istatus == dtmsg_FAIL)
	    {
		status = dtmsg_FAIL;
	    }
	    else
	    {
		/*  
		 * Add this property to the receiver's list of unread ones 
		 */

		/* allocate enough memory to hold the list of props */
		if (pRcvr->numPropsUnread >= pRcvr->sizePropsUnread)
		{
		    pRcvr->sizePropsUnread += DT_MSG_PROP_INC_AMT;

		    if (pRcvr->sizePropsUnread == DT_MSG_PROP_INC_AMT)
		    {
			pRcvr->propsUnread = (Atom *) XtRealloc (
				    (char *) pRcvr->propsUnread,
				    pRcvr->sizePropsUnread*sizeof(Atom));
		    }
		    else
		    {
			pRcvr->propsUnread = (Atom *) XtMalloc (
					pRcvr->sizePropsUnread*sizeof(Atom));
		    }
		}
	    }

	    /* update the receiver's unread property list */
	    pRcvr->propsUnread[pRcvr->numPropsUnread] = property;
	    pRcvr->numPropsUnread++;
	}

	/*
	 * Flush out events so that clients see messages sooner
	 * rather than later.
	 */
	XSync (XtDisplay(DtMsgH_Widget(handle)), False);

	if (pBcData->numReceivers == 0)
	{
	    status = dtmsg_NO_LISTENERS;
	}
    }
    else
    {
	status = dtmsg_FAIL;
    }

    return (status);
}

