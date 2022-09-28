/*
 *  MsguBcast.c -- Broadcast message utilities
 *
 *  $XConsortium: MsguBcast.c /main/cde1_maint/1 1995/07/14 20:37:09 drk $
 *  $XConsortium: MsguBcast.c /main/cde1_maint/1 1995/07/14 20:37:09 drk $
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
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

/*
 * global data
 */
static Window w4w_window = None;

/*
 * Forward references
 */
static void _DtMsgBroadcastSetRegistry (
#ifndef _NO_PROTO
	DtMsgHandle		handle,
	unsigned long *		plRegistry,
	int 			numEntries
#endif
);


/*
 * _DtMsgBroadcastGetSharedWindow -- Get the shared window for this
 *					broadcast message group
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastGetSharedWindow (handle, pWin)
	DtMsgHandle		handle;
	Window *		pWin;
#else
_DtMsgBroadcastGetSharedWindow (
	DtMsgHandle		handle,
	Window *		pWin 
	)
#endif
{
    int status;

    /* look in the registry for the window */
    status = _DtMsgBroadcastFindWindowInRegistry (handle, pWin);

    /* if not there, then create the window, adding it to the registry  */
    if (status == dtmsg_FAIL)
    {
	/* 
	 * Grab server to lock out other clients from 
	 * overwriting the registry.
	 */
	/* XGrabServer (XtDisplay(DtMsgH_Widget(handle))); */

	/* 
	 * Try once more incase it's been added real recently
	 */
	status = _DtMsgBroadcastFindWindowInRegistry (handle, pWin);

	if (status == dtmsg_FAIL)
	{
	    /* still not there, create the shared window */
	    status = _DtMsgBroadcastCreateSharedWindow (handle, pWin);

	    if (status == dtmsg_SUCCESS)
	    {
		/* OK, now add it to the registry */
		_DtMsgBroadcastAddToRegistry (handle, *pWin);

		/* stuff it into the handle */
		DtMsgH_SharedWindow(handle) = *pWin;
	    }
	}

	/* XUngrabServer (XtDisplay(DtMsgH_Widget(handle)));  */
    }

    return (status);
}

/*
 * _DtMsgBroadcastFindWindowInRegistry -- Get the shared window 
 *					   for this broadcast message 
 *					   group
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastFindWindowInRegistry (handle, pWin)
	DtMsgHandle		handle;
	Window *		pWin;
#else
_DtMsgBroadcastFindWindowInRegistry (
	DtMsgHandle		handle,
	Window *		pWin 
	)
#endif
{
    int status;
    Widget widget = DtMsgH_Widget (handle);
    unsigned long *plRegistry;
    int i, numEntries;
    Atom msg_group;

    /* get the registry property */
    status = _DtMsgBroadcastGetRegistry (handle, &plRegistry, &numEntries);

    if (status == dtmsg_SUCCESS)
    {
	msg_group = DtMsgH_Atom (handle);
	status = dtmsg_FAIL;

	/* search the list */
	for (i=0; i<numEntries; i++)
	{
	    if (msg_group == (Atom) MSG_GROUP_NAME (plRegistry, i))
	    {
		/* matched name, return associated window */
		*pWin = (Window) MSG_GROUP_WINDOW (plRegistry, i);
		status = dtmsg_SUCCESS;
		break;
	    }
	}
	XFree ((char *) plRegistry);
    }

    return (status);
}

/*
 * _DtMsgBroadcastAddToRegistry -- Add a shared window to the registry
 *				   for this broadcast message group
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastAddToRegistry (handle, window)
	DtMsgHandle		handle;
	Window 			window;
#else
_DtMsgBroadcastAddToRegistry (
	DtMsgHandle		handle,
	Window 			window 
	)
#endif
{
    int status;
    Widget widget = DtMsgH_Widget (handle);
    unsigned long *plRegistry;
    int i, numEntries;
    Atom msg_group;
    unsigned long *plNewRegistry;
    unsigned long miniRegistry[2];

    /* get our message group atom */
    msg_group = DtMsgH_Atom (handle);

    /* get the registry property */
    status = _DtMsgBroadcastGetRegistry (handle, &plRegistry, &numEntries);

    if (status == dtmsg_SUCCESS)
    {
	/* allocate a new array that is one entry bigger */
	plNewRegistry = (unsigned long *) XtMalloc (
	    (1+numEntries)*DT_MSG_REGISTRY_ENTRY_SIZE*sizeof (unsigned long)
	    );

	/* put our new addition in first */
	i = 0;
	MSG_GROUP_NAME (plNewRegistry, i) =  (unsigned long) msg_group;
	MSG_GROUP_WINDOW (plNewRegistry, i) = (unsigned long) window;

	/* Copy the rest of the list */
	for (i=0; i<numEntries; i++)
	{
	    MSG_GROUP_NAME (plNewRegistry, i+1) = 
			MSG_GROUP_NAME (plRegistry, i);
	    MSG_GROUP_WINDOW (plNewRegistry, i+1) =
			MSG_GROUP_WINDOW (plRegistry, i);
	}
	/* free the old list */
	XFree ((char *) plRegistry);

	/* write out the new list */
        _DtMsgBroadcastSetRegistry (handle, plNewRegistry, numEntries+1);

	/* free our copy of the new list */
	XtFree ((char *) plNewRegistry);
    }
    else
    {
	/* no registry! let's create one */
	MSG_GROUP_NAME (miniRegistry, 0) =  (unsigned long) msg_group;
	MSG_GROUP_WINDOW (miniRegistry, 0) = (unsigned long) window;

	/* write out the new list */
        _DtMsgBroadcastSetRegistry (handle, miniRegistry, 1);
    }

    return (status);
}

/*
 * _DtMsgBroadcastCreateSharedWindow -- Create the shared window
 *					   for this broadcast message 
 *					   group
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastCreateSharedWindow (handle, pWin)
	DtMsgHandle		handle;
	Window *		pWin;
#else
_DtMsgBroadcastCreateSharedWindow (
	DtMsgHandle		handle,
	Window *		pWin 
	)
#endif
{
   int status;
   Display *dpy;
   Widget widget = DtMsgH_Widget (handle);
   Window window;
   XSetWindowAttributes attributes;
   unsigned long mask;


   /* 
    * Need to open a new display connection. We're going to 
    * set the closedown mode to retain permanent so that this 
    * window lives for the life of the server. We don't want
    * to do this to all the resources we've got on the current
    * connection.
    */
   dpy = XOpenDisplay (DisplayString (XtDisplay (widget)));

   if (dpy)
   {
       attributes.override_redirect = True;
       mask = CWOverrideRedirect;
       window = XCreateWindow (
			dpy,
			XRootWindowOfScreen (XtScreen (widget)),
			-100, -100,		/* x, y */
			100, 100,		/* width, height */
			0,			/* border width */
			XDefaultDepthOfScreen (XtScreen (widget)),
			CopyFromParent,		/* class */
			CopyFromParent,		/* visual*/
			mask,
			&attributes
	   );

	*pWin = window;
	XSetCloseDownMode (dpy, RetainPermanent);
	XCloseDisplay (dpy);

	status = dtmsg_SUCCESS;
   }
   else 
   {
       status = dtmsg_FAIL;
   }

   return (status);
}

/*
 * _DtMsgBroadcastGetRegistry -- Get the broadcast window registry
 *
 * Free the data with XFree()
 */
int
#ifdef _NO_PROTO
_DtMsgBroadcastGetRegistry (handle, pplRegistry, pNumEntries)
	DtMsgHandle		handle;
	unsigned long **	pplRegistry;
	int *			pNumEntries;
#else
_DtMsgBroadcastGetRegistry (
	DtMsgHandle		handle,
	unsigned long **	pplRegistry,
	int *			pNumEntries
	)
#endif
{
    int status;
    Widget widget = DtMsgH_Widget (handle);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;

    status = XGetWindowProperty (
		    XtDisplay (widget),
		    XRootWindowOfScreen (XtScreen(widget)),
		    DtMsgH_RegistryAtom (handle),
		    0L,
		    DT_MSG_MAX_PROP_SIZE,
		    False,
		    AnyPropertyType,
		    &actual_type,
		    &actual_format,
		    &nitems,
		    &bytes_after,
		    (unsigned char **) pplRegistry
		);

    if ((status == Success) && (nitems > 0))
    {
	/* we actually got something! */
	if (actual_format != 32)
	{
	    /* bogus property */
	    XFree ((char *) *pplRegistry);
	    status = dtmsg_FAIL;
	}
	else
	{
	    *pNumEntries = (int) nitems/DT_MSG_REGISTRY_ENTRY_SIZE;
	    status = dtmsg_SUCCESS;
	}
    }
    else
    {
	status = dtmsg_FAIL;
    }

    return (status);
}

/*
 * _DtMsgBroadcastSetRegistry -- Set the broadcast window registry
 *
 */
static void
#ifdef _NO_PROTO
_DtMsgBroadcastSetRegistry (handle, plRegistry, pNumEntries)
	DtMsgHandle		handle;
	unsigned long *		plRegistry;
	int 			numEntries;
#else
_DtMsgBroadcastSetRegistry (
	DtMsgHandle		handle,
	unsigned long *		plRegistry,
	int 			numEntries
	)
#endif
{
    Widget widget = DtMsgH_Widget (handle);

    XChangeProperty (
		    XtDisplay (widget),
		    XRootWindowOfScreen (XtScreen(widget)),
		    DtMsgH_RegistryAtom (handle),
		    DtMsgH_RegistryAtom (handle),
		    32,
		    PropModeReplace,
		    (unsigned char *) plRegistry,
		    (numEntries * DT_MSG_REGISTRY_ENTRY_SIZE)
		);
}

/*
 * _DtMsgW4wRealize -- our very own realize proc for 
 *			funny widgets that we stuff  windows into.
 */
void
#ifdef _NO_PROTO
_DtMsgW4wRealize (widget, valueMask, attributes)
	Widget			widget;
	Mask			*valueMask;
	XSetWindowAttributes	*attributes;
#else
_DtMsgW4wRealize (
	Widget			widget,
	Mask			*valueMask,
	XSetWindowAttributes	*attributes 
	)
#endif
{
    /* stuff in the correct window */
    widget->core.window = w4w_window;
}


/*
 * _DtMsgW4wCreate -- Create a widget for an anonymous
 *				     window
 */
Widget
#ifdef _NO_PROTO
_DtMsgW4wCreate (handle, window)
	DtMsgHandle		handle;
	Window			window;
#else
_DtMsgW4wCreate (
	DtMsgHandle		handle,
	Window			window 
	)
#endif
{
    int status;
    Widget widget;
    Arg args[10];
    int n;
    XtRealizeProc  realRealize;

    /* 
     * Create a minimal widget.
     */
    n = 0;
    XtSetArg (args[n], XtNmappedWhenManaged, False);	n++;
    XtSetArg (args[n], XtNx, 0);			n++;
    XtSetArg (args[n], XtNy, 0);			n++;
    XtSetArg (args[n], XtNwidth, 10);			n++;
    XtSetArg (args[n], XtNheight, 10);			n++;
    widget = XtCreateWidget ("dtMsgMinimal",
			coreWidgetClass,
			DtMsgH_Widget (handle),
			args, n);

    /* replace the realize proc with our version */
    realRealize = widget->core.widget_class->core_class.realize;
    widget->core.widget_class->core_class.realize = 
			(XtRealizeProc) _DtMsgW4wRealize;

    /* "realize" and stuff in our window */
    w4w_window = window;
    XtRealizeWidget (widget);

    /* put back the right realize proc */
    widget->core.widget_class->core_class.realize = realRealize;

    /*
     * !!NOTE!!
     * Replace the window in this widget with "None" before calling
     * XtDestroyWidget. We tried to use the DestroyCallback, but
     * that was insufficient in preventing the window from getting
     * destroyed with the widget. The toolkit code saves the window
     * id before calling the callback, then goes ahead and destroys'
     * the window.
     */

    return (widget);
}


/*
 * _DtMsgXErrorHandler
 */
/* ARGSUSED */
static int
#ifdef _NO_PROTO
_DtMsgXErrorHandler (display, pErrorEvent)
	Display	*	display;
	XErrorEvent *	pErrorEvent;
#else
_DtMsgXErrorHandler (
	Display	*	display,
	XErrorEvent *	pErrorEvent 
	)
#endif
{
    return;
}


/*
 * _DtMsgW4wAddEventHandler -- add an event handler to one of our
 *				     funny widgets
 */
void
#ifdef _NO_PROTO
_DtMsgW4wAddEventHandler (widget, mask, nonmaskable, proc, client_data)
	Widget			widget;
	EventMask		mask;
	Boolean			nonmaskable;
	XtEventHandler		proc;
	Pointer		client_data;
#else
_DtMsgW4wAddEventHandler (
	Widget			widget,
	EventMask		mask,
	Boolean			nonmaskable,
	XtEventHandler		proc,
	Pointer		client_data 
	)
#endif
{
    int (*oldHandler)();

    oldHandler = XSetErrorHandler (_DtMsgXErrorHandler);

    XtAddRawEventHandler (widget, mask, nonmaskable, proc, client_data);

    XSelectInput (XtDisplay(widget), XtWindow(widget), mask);

    XSync (XtDisplay(widget), False);

    (void) XSetErrorHandler (oldHandler);
}
