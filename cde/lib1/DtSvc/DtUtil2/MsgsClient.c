/*
 *  MsgsClient.c -- Routines for the service consumer
 *
 *  $XConsortium: MsgsClient.c /main/cde1_maint/1 1995/07/14 20:36:41 drk $
 *  $XConsortium: MsgsClient.c /main/cde1_maint/1 1995/07/14 20:36:41 drk $
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


static int nextAtomIndex = 1;
static int numCachedAtoms = 0;
static Atom * cachedAtoms = NULL;


/*
 * This function will check to see if the specified property is already
 * being used by the communications window; i.e. if it is already in the
 * list of properties associated with the window.
 */

static Boolean
#ifdef _NO_PROTO
_DtPropertyIsUnique ( property, propertyList, numProperties)
   Atom property;
   Atom * propertyList;
   int numProperties;
#else
_DtPropertyIsUnique (
   Atom property,
   Atom * propertyList,
   int numProperties)
#endif

{
   int i;

   for (i = 0; i < numProperties; i++)
   {
      if (property == propertyList[i])
         return(False);
   }

   return(True);
}


/* 
 * Generate a unique property name which can be used to pass our
 * request information to the service provider.  The property must be
 * unique within this client so that two quick requests for the
 * service do not overwrite each others request information.
 */


Atom
#ifdef _NO_PROTO
_DtMsgServiceUniqueProperty (handle)
   DtMsgHandle handle;
#else
_DtMsgServiceUniqueProperty (
   DtMsgHandle handle)
#endif

{
   Atom uniqueProperty = DtMsgH_PropertyAtom (handle);
   Atom * atomsInUse;
   int numAtomsInUse;
   Widget widget;
   int i;

   widget = DtMsgH_Widget (handle);
   atomsInUse = XListProperties (XtDisplay (widget), XtWindow (widget),
                                 &numAtomsInUse);

   /* 
    * If the default property is already being used by this client,
    * then we need to generate a unique property name.  We will first
    * attempt to reuse any properties we created due to earlier
    * conflicts.  If this fails, then we will create a new property.
    */
   if (atomsInUse && (numAtomsInUse > 0))
   {
      if (!_DtPropertyIsUnique (uniqueProperty, atomsInUse, numAtomsInUse))
      {
         /* Try to reuse an existing property created by an earlier conflict */
         for (i = 0; i < numCachedAtoms; i ++)
         {
            if (_DtPropertyIsUnique (cachedAtoms [i], atomsInUse,
                                       numAtomsInUse))
            {
               uniqueProperty = cachedAtoms [i];
               /* printf("Reusing a property: (%d)\n", uniqueProperty); */
               break;
            }
         }

         /* See if we need to create a new property */
         if (i >= numCachedAtoms)
         {
            char * newProperty;

            newProperty = XtMalloc(strlen (DtMsgH_PropertyName (handle)) + 10);
            sprintf (newProperty, "%s_%d", DtMsgH_PropertyName (handle),
                        nextAtomIndex);
            nextAtomIndex ++;
            cachedAtoms = (Atom *) XtRealloc ((char *) cachedAtoms,
                              sizeof (Atom)*(numCachedAtoms + 1));
            uniqueProperty = cachedAtoms [numCachedAtoms] =
                        XInternAtom (XtDisplay (widget), newProperty, False);
            numCachedAtoms ++;
            /* printf("New property: %s (%d)\n", newProperty, uniqueProperty);*/
            XtFree (newProperty);
         }
      }
/*
      else
        printf ("Incoming property is OK: (%d)\n", uniqueProperty);
*/
   }
/*
   else
     printf ("Incoming property is OK: (%d)\n", uniqueProperty);
*/

   if (atomsInUse)
      XFree ((char *) atomsInUse);

   return (uniqueProperty);

}


/*
 * _DtMsgServiceIsOffered -- Indicate whether or not a service is available
 */
Boolean
#ifdef _NO_PROTO
_DtMsgServiceIsOffered (handle)
	DtMsgHandle	handle;
#else
_DtMsgServiceIsOffered (
	DtMsgHandle	handle
	)
#endif
{
    Window window;

    window = XGetSelectionOwner (XtDisplay(DtMsgH_Widget(handle)), 
			DtMsgH_Atom(handle));

    return (window != None);
}

/*
 * _DtHandleSelectionReplyEvents -- handle reply events to a selection
 * 					request
 */
static XtEventHandler
#ifdef _NO_PROTO
_DtHandleSelectionReplyEvents (widget, client_data, pevent, pbDispatch)
	Widget		widget;
	Pointer	client_data;
	XEvent *	pevent;
	Boolean *	pbDispatch;
#else
_DtHandleSelectionReplyEvents (
	Widget		widget,
	Pointer	client_data,
	XEvent *	pevent,
	Boolean *	pbDispatch 
	)
#endif
{
    DtMsgRequestContext *pctx = (DtMsgRequestContext *) client_data;
    Atom aSelection = DtMsgH_Atom (pctx->handle);
    char **ppchMessage;
    int status;
    int count;

    switch (pevent->type)
    {
	case SelectionNotify:

	    if (aSelection == pevent->xselection.selection)
	    {
		/* get the message from the property */
		status = _DtMsgGetMessage (XtDisplay(widget), 
					    XtWindow(widget), 
					    pevent->xselection.property, 
					    &ppchMessage,
					    &count);

		if ((status == dtmsg_SUCCESS) &&
		    (pctx && pctx->reply_proc))
		{
		    /* send reply back to client */

		    (*(pctx->reply_proc)) (pctx->handle, 
					   NULL,
					   pctx->client_data,
					   ppchMessage,
					   count);
		}

		/* free up message data */
		XFreeStringList (ppchMessage);

		/* Reply received, don't need to listen for events anymore */
		XtRemoveEventHandler (widget, 0, True, 
			(XtEventHandler) _DtHandleSelectionReplyEvents, 
			client_data);

		/* free up context for this request-reply */
		XtFree ((char *)pctx);
	    }
	    break;
    }
}

/*
 * _DtMsgServiceRequest -- make a request of a service
 */
int
#ifdef _NO_PROTO
_DtMsgServiceRequest (handle, ppchRequest, count, 
					reply_proc, client_data)
	DtMsgHandle	handle;
	char **			ppchRequest;
	int			count;
	DtMsgReceiveProc	reply_proc;
	Pointer		client_data;
#else
_DtMsgServiceRequest (
	DtMsgHandle	handle,
	char **			ppchRequest,
	int			count,
	DtMsgReceiveProc	reply_proc,
	Pointer		client_data
	)
#endif
{
    int status;
    DtMsgRequestContext *pctx;
    Atom xaREQUEST;
    Time time;
    Widget widget = DtMsgH_Widget (handle);
    Atom aSelection = DtMsgH_Atom (handle);
    Atom aProperty =  _DtMsgServiceUniqueProperty (handle);

    if (_DtMsgServiceIsOffered (handle))
    {
	/* intern the request atom */
	xaREQUEST = XInternAtom (XtDisplay(widget), DT_MSG_XA_REQUEST, 
				    False);

	/* put the property on the window */
	status = _DtMsgSetMessage (XtDisplay(widget), XtWindow(widget),
			aProperty, ppchRequest, count);

	if (status == dtmsg_SUCCESS)
	{
	    /* save context to handle reply message */
	    pctx = XtNew(DtMsgRequestContext);

	    pctx->handle = handle;
	    pctx->reply_proc = reply_proc;
	    pctx->client_data = client_data;

	    /* 
	     * Add event handler to handle property notify message
	     * when it comes.
	     */
	    XtAddEventHandler (widget, 0, True, 
			(XtEventHandler) _DtHandleSelectionReplyEvents, 
			(Pointer) pctx);

	    /* get the time */
	    time = XtLastTimestampProcessed (XtDisplay(widget));

	    /* Trigger the message transfer */
	    XConvertSelection (XtDisplay(widget), aSelection, xaREQUEST,
			aProperty, XtWindow(widget), time);
            XSync(XtDisplay(widget), False);
	}
    }
    else
    {
	status = dtmsg_NO_SERVICE;
    }
    return (status);
}

