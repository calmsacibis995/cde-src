#ifndef NO_OL_COMPAT
/* 
 * (c) Copyright 1989 Sun Microsystems, Inc.
 * (c) Copyright 1993 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
 */ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmOL.c /main/cde1_maint/1 1995/07/15 01:55:10 drk $"
#endif
#endif

/*
 * Included Files:
 */
#include "WmGlobal.h"
#include "WmOL.h"

#define ValidPropertyList(pcd) ((pcd)->paInitialProperties != NULL)

/* DSDM - used in ProcessDragdrop */
#define OL_PREVIEW_INDEX 4
#define MOTIF_RECEIVER_FLAG	0x80000000

/* DSDM */
/*************************************<->*************************************
 *
 *  GetPropertyWindow (dpy, in_win, atom)
 *
 *  Description:
 *  -----------
 *  get the window that is a property of the inputed window
 *
 *  Inputs:
 *  ------
 *  the display, the window with the property, and the property
 *
 *  Outputs:
 *  --------
 *  returns the window which is the property of the inputed window
 * 
 *************************************<->***********************************/
Window
GetPropertyWindow(dpy, in_win, atom)
    Display *dpy;
    Window in_win;
    Atom atom;
{
    Atom            type;
    int             format;
    unsigned long   lengthRtn;
    unsigned long   bytesafter;
    Window         *property = NULL;
    Window          win = None;

    if ((XGetWindowProperty (dpy,
			     in_win,
			     atom,
                             0L, 1,
                             False,
                             AnyPropertyType,
                             &type,
                             &format,
                             &lengthRtn,
                             &bytesafter,
                             (unsigned char **) &property) == Success) &&
         (type == XA_WINDOW) &&
         (format == 32) &&
         (lengthRtn == 1)) {
        win = *property;
    }
    if (property) {
        XFree ((char *)property);
    }

    return (win);
}

/* DSDM */
/*************************************<->*************************************
 *
 *  InitDsdmProtocol ()
 *
 *  Description:
 *  -----------
 *  Initialize the atoms associated with drag and drop and find the proxy 
 *  window.
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/
void 
InitDsdmProtocol()
{
  Window motif_drag_win;
  Atom ATOM_MOTIF_DRAG_WIN;
  Atom ATOM_MOTIF_PROXY_WIN;

  ATOM_MOTIF_DRAG_WIN = XInternAtom(DISPLAY, "_MOTIF_DRAG_WINDOW", False);
  ATOM_MOTIF_PROXY_WIN = XInternAtom(DISPLAY, "_MOTIF_DRAG_PROXY_WINDOW", False);
  wmGD.ATOM_DRAGDROP_INTEREST = XInternAtom(DISPLAY, "_SUN_DRAGDROP_INTEREST", False);

  /* Find the proxy window used by drag and drop. */
  motif_drag_win = GetPropertyWindow(DISPLAY, DefaultRootWindow(DISPLAY), ATOM_MOTIF_DRAG_WIN);
  wmGD.proxy_win = GetPropertyWindow(DISPLAY, motif_drag_win, ATOM_MOTIF_PROXY_WIN);
}

/* DSDM */
/*************************************<->*************************************
 *
 *  SendDsdmStop ()
 *
 *  Description:
 *  -----------
 *  Send a client message to the proxy window where the dsdm will see it.  The 
 *  dsdm will stop processing mapped windows.  This "stop" command informs the 
 *  dsdm that dtwm will detect and inform the dsdm about OL windows when they 
 *  are mapped.  This means the dsdm does not have to wake up every time a 
 *  property changes on a top level window.  This is intended as a performance 
 *  improvement. 
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/
void
SendDsdmStop()
{
    XClientMessageEvent event;
    Atom ATOM_DT_PROXY_STOP;

    ATOM_DT_PROXY_STOP = XInternAtom(DISPLAY, "_DT_PROXY_STOP", False);
    /* Send an event to the proxy window. */
    event.type = ClientMessage;
    event.send_event = True;
    event.display = DISPLAY;
    event.message_type = ATOM_DT_PROXY_STOP;
    event.format = 32;
    XSendEvent(DISPLAY, wmGD.proxy_win, False, NoEventMask, (XEvent*)&event);
    XFlush(DISPLAY);
}

/*************************************<->*************************************
 *
 *  SendDsdmStart ()
 *
 *  Description:
 *  -----------
 *  Send a message to the dsdm to restart processing grag and drop interest
 *  properties.
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/
void
SendDsdmStart()
{
  XClientMessageEvent event;
  Atom ATOM_DT_PROXY_START;

  ATOM_DT_PROXY_START = XInternAtom(DISPLAY, "_DT_PROXY_START", False);
  event.type = ClientMessage;
  event.send_event = True;
  event.display = DISPLAY;
  event.message_type = ATOM_DT_PROXY_START;
  event.format = 32;
  XSendEvent(DISPLAY, wmGD.proxy_win, False, NoEventMask, (XEvent*)&event);
  XFlush(DISPLAY);
}

/* DSDM */
/*************************************<->*************************************
 *
 *  GetInterestProperty ()
 *
 *  Description:
 *  -----------
 *
 *  Get the interest property from this window.  If a valid interest property
 *  was found, a pointer to the data is returned.  This data must be freed with
 *  XFree().  If no valid property is found, NULL is returned.
 *
 *  Inputs:
 *  ------
 *  the display, the window, and an address to store nitems in
 * 
 *  Outputs:
 *  --------
 *  a pointer to the data
 * 
 *************************************<->***********************************/
unsigned char *
GetInterestProperty(dpy, win, nitems)
    Display *dpy;
    Window win;
    unsigned long *nitems;
{
    Status s;
    Atom acttype;
    int actfmt;
    unsigned long remain;
    unsigned char *data;

#define INTEREST_MAX 100000000L /*(1024L*1024L)*/
#define XFreeDefn	void *

    s = XGetWindowProperty(dpy, win, wmGD.ATOM_DRAGDROP_INTEREST, 0L, INTEREST_MAX,
			   False, wmGD.ATOM_DRAGDROP_INTEREST, &acttype, &actfmt,
			   nitems, &remain, &data);

    if (s != Success)
	return NULL;
    if (acttype == None)
	/* property does not exist */
	return NULL;
    if (acttype != wmGD.ATOM_DRAGDROP_INTEREST) {
	fputs("dsdm: interest property has wrong type\n", stderr);
	return NULL;
    }
    if (actfmt != 32) {
	fputs("dsdm: interest property has wrong format\n", stderr);
	XFree((XFreeDefn) data);
	return NULL;
    }
    if (remain > 0) {
	/* didn't read it all, punt */
	fputs("dsdm: interest property too long\n", stderr);
	XFree((XFreeDefn) data);
	return NULL;
    }
    return data;
} /* end of GetInterestProperty */

/*************************************<->*************************************
 *
 *  ProcessDragdrop (Window)
 *
 *  Description:
 *  -----------
 *  If a client has announced drag and drop interest, then notify the dsdm.
 *
 *  Inputs:
 *  ------
 *  The client window.
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/
void
ProcessDragdrop(Window clientWindow)
{
  XClientMessageEvent event;
  CARD32 *data;
  unsigned long nitems;
  int motif_receiver = False;
  Atom ATOM_DT_PROXY_WINDOW;

  ATOM_DT_PROXY_WINDOW = XInternAtom(DISPLAY, "_DT_PROXY_WINDOW", False);
  data = (CARD32 *) GetInterestProperty(DISPLAY, clientWindow, &nitems);
  if (data != NULL) {
    if (nitems > OL_PREVIEW_INDEX &&
	(data[OL_PREVIEW_INDEX] & MOTIF_RECEIVER_FLAG))
      motif_receiver = True;
    XFree(data);
  }
  if (!motif_receiver) {
    event.type = ClientMessage;
    event.send_event = True;
    event.display = DISPLAY;
    event.message_type = ATOM_DT_PROXY_WINDOW;
    event.window = clientWindow;
    event.format = 32;
    XSendEvent(DISPLAY, wmGD.proxy_win, False, NoEventMask, (XEvent*)&event);
    XFlush(DISPLAY);
  }
}


/*************************************<->*************************************
 *
 *  InitOLCompat ()
 *
 *  Description:
 *  -----------
 *  Interns the atoms necessary for OL protocols.
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
InitOLCompat()
#else
InitOLCompat(void)
#endif /* _NO_PROTO */
{

    wmGD.xa_OL_WIN_ATTR =
	    XmInternAtom (DISPLAY, OL_WIN_ATTR, False);
    wmGD.xa_OL_DECOR_RESIZE =
	    XmInternAtom (DISPLAY, OL_DECOR_RESIZE, False);
    wmGD.xa_OL_DECOR_HEADER =
	    XmInternAtom (DISPLAY, OL_DECOR_HEADER, False);
    wmGD.xa_OL_DECOR_CLOSE =
	    XmInternAtom (DISPLAY, OL_DECOR_CLOSE, False);
    wmGD.xa_OL_DECOR_PIN =
	    XmInternAtom (DISPLAY, OL_DECOR_PIN, False);
    wmGD.xa_OL_DECOR_ADD =
	    XmInternAtom (DISPLAY, OL_DECOR_ADD, False);
    wmGD.xa_OL_DECOR_DEL =
	    XmInternAtom (DISPLAY, OL_DECOR_DEL, False);
    wmGD.xa_OL_WT_BASE =
	    XmInternAtom (DISPLAY, OL_WT_BASE, False);
    wmGD.xa_OL_WT_COMMAND =
	    XmInternAtom (DISPLAY, OL_WT_CMD, False);
    wmGD.xa_OL_WT_HELP =
	    XmInternAtom (DISPLAY, OL_WT_HELP, False);
    wmGD.xa_OL_WT_NOTICE =
	    XmInternAtom (DISPLAY, OL_WT_NOTICE, False);
    wmGD.xa_OL_WT_OTHER =
	    XmInternAtom (DISPLAY, OL_WT_OTHER, False);
    wmGD.xa_OL_PIN_IN =
	    XmInternAtom (DISPLAY, OL_PIN_IN, False);
    wmGD.xa_OL_PIN_OUT =
	    XmInternAtom (DISPLAY, OL_PIN_OUT, False);
    wmGD.xa_OL_MENU_LIMITED =
	    XmInternAtom (DISPLAY, OL_MENU_LIMITED, False);
    wmGD.xa_OL_MENU_FULL =
	    XmInternAtom (DISPLAY, OL_MENU_FULL, False);

    /* DSDM - initialize the dsdm */
    InitDsdmProtocol();

    /* Send an event to the dsdm informing it we will take responsibility 
     * for detecting windows with drag and drop interest.
     */
    SendDsdmStop();

} /* END OF FUNCTION InitOLCompat */



/*************************************<->*************************************
 *
 *  HasOpenLookHints (pCD)
 *
 *  Description:
 *  -----------
 *  Returns True if this client has OpenLook hints on it.  
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data
 *
 *  Outputs:
 *  --------
 *  Returns True if client has the _OL_WIN_ATTR property on it.
 * 
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
HasOpenLookHints( pCD )
        ClientData *pCD;
#else
HasOpenLookHints(
        ClientData *pCD )
#endif /* _NO_PROTO */
{
    Boolean rval = False;
    OLWinAttr *property = NULL;

    if (ValidPropertyList (pCD) &&
	HasProperty(pCD, wmGD.xa_OL_WIN_ATTR))
    {
	rval = True;
    }
    else if ((property=GetOLWinAttr (pCD)) != NULL)
    {
	XFree ((char *) property);
	rval = True;
    }

    return (rval);

} /* END OF FUNCTION HasOpenLookHints */



/*************************************<->*************************************
 *
 *  GetOLWinAttr (pCD)
 *
 *  Description:
 *  -----------
 *  Fetches the OLWinAttr property off of the client
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data
 *
 *  Outputs:
 *  --------
 *  Returns a pointer to the OLWinAttr property if found and valid.
 *  (Returned data should be freed with XFree())
 *  Returns NULL pointer otherwise.
 * 
 *************************************<->***********************************/
OLWinAttr * 
#ifdef _NO_PROTO
GetOLWinAttr( pCD )
        ClientData *pCD;
#else
GetOLWinAttr(
        ClientData *pCD )
#endif /* _NO_PROTO */
{
    Boolean rval = False;
    OLWinAttr *property = NULL;
    OLWinAttr *prop_new;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;
    int ret_val;

    ret_val = XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa_OL_WIN_ATTR, 
		  0L, ENTIRE_CONTENTS,
		  False, wmGD.xa_OL_WIN_ATTR, 
		  &actual_type, &actual_format, 
		  &nitems, &leftover, (unsigned char **)&property);

    if (ret_val != Success)
    {
	    property = NULL;
    }
    else if ((actual_format != 32) || 
	(actual_type != wmGD.xa_OL_WIN_ATTR)) 
    {
	    if (property) 
		XFree((char *)property);
	    property = NULL;
    }

    if (property && (nitems == OLDOLWINATTRLENGTH))
    {
	/* Old size, convert to new size */
	/* 
	 * !!! Should use XAlloc() here, but Xlib doesn't
	 *     define an inverse function of XFree(). !!!
	 */
	prop_new = (OLWinAttr *) malloc (sizeof(OLWinAttr));

	prop_new->flags = WA_WINTYPE | WA_MENUTYPE | WA_PINSTATE;
	prop_new->win_type = ((old_OLWinAttr *)property)->win_type;
	prop_new->menu_type = ((old_OLWinAttr *)property)->menu_type;
	prop_new->pin_initial_state = 
			((old_OLWinAttr *)property)->pin_initial_state;

	XFree ((char *) property);
	property = prop_new;
    }

    /* convert pin state for old clients */
    if (property && (property->flags & WA_PINSTATE))
    {
	if (property->pin_initial_state == wmGD.xa_OL_PIN_IN)
	{
	    property->pin_initial_state = PIN_IN;
	}
	else if (property->pin_initial_state == wmGD.xa_OL_PIN_OUT)
	{
	    property->pin_initial_state = PIN_OUT;
	}
    }

    return (property);

} /* END OF FUNCTION GetOLWinAttr */


/*************************************<->*************************************
 *
 *  GetOLDecorFlags (pCD, property, pDecor)
 *
 *  Description:
 *  -----------
 *  Fetches the _OL_DECOR_ADD or _OL_DECOR_DEL property off of the 
 *  client and returns OL flavored decor flags.
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data
 *  property = property to fetch
 *  pDecor = pointer to OL decor flags word
 *
 *  Outputs:
 *  --------
 *  Return =  True if property found, False otherwise
 *  *pDecor = OL decor flags if valid property found, 
 *	      undefined if property not found.
 * 
 *************************************<->***********************************/
Boolean
#ifdef _NO_PROTO
GetOLDecorFlags( pCD, property, pDecor )
        ClientData *pCD;
	Atom property;
	unsigned long *pDecor;
#else
GetOLDecorFlags(
        ClientData *pCD,
	Atom property,
	unsigned long *pDecor)
#endif /* _NO_PROTO */
{
    int status, i;
    Boolean rval;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;
    Atom *pAtoms = NULL;

    status = XGetWindowProperty (DISPLAY, pCD->client, property,
		  0L, ENTIRE_CONTENTS,
		  False, XA_ATOM, 
		  &actual_type, &actual_format, 
		  &nitems, &leftover, (unsigned char **)&pAtoms);

    if ((status != Success) || 
	!pAtoms || 
	(nitems == 0) ||
	(actual_type != XA_ATOM) ||
	(actual_format != 32)) 
    {
	    if (pAtoms)
		    XFree((char *)pAtoms);
	    rval = False;
    }
    else
    {
	*pDecor = 0;

	/*
	 * We only look for the ones we might be interested in.
	 * Several OL decoration types are ignored.
	 */
	for (i = 0; i < nitems; i++) {
		if (pAtoms[i] == wmGD.xa_OL_DECOR_RESIZE)
			*pDecor |= OLDecorResizeable;
		else if (pAtoms[i] == wmGD.xa_OL_DECOR_HEADER)
			*pDecor |= OLDecorHeader;
		else if (pAtoms[i] == wmGD.xa_OL_DECOR_CLOSE)
			*pDecor |= OLDecorCloseButton;
		else if (pAtoms[i] == wmGD.xa_OL_DECOR_PIN)
			*pDecor |= OLDecorPushPin;
	}

	XFree((char *)pAtoms);
	rval = True;

    }
    return (rval);

} /* END OF FUNCTION GetOLDecorFlags */



/*************************************<->*************************************
 *
 *  ProcessOLDecoration (pCD)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data
 *
 *  Outputs:
 *  --------
 *  pCD = possibly modified with new decoration info
 * 
 *************************************<->***********************************/
void
#ifdef _NO_PROTO
ProcessOLDecoration( pCD )
	ClientData *pCD;
#else
ProcessOLDecoration(
        ClientData *pCD)
#endif /* _NO_PROTO */
{
    OLWinAttr * pOLWinAttr;
    unsigned long OLdecor;
    long decorMask;

    if (HasOpenLookHints (pCD) && 
	((pOLWinAttr = GetOLWinAttr (pCD)) != NULL))
    {
	/*
	 * This window already has some decoration applied to
	 * it based on its ICCCM type (transient or not).
	 * Use the OL hints to construct a mask to further 
	 * modify these decorations.
	 */
	if ((pOLWinAttr->flags & WA_WINTYPE) == 0) 
        {
	    /*
	     * Window type not specified, assume all decorations
	     */
	    decorMask = WM_DECOR_ALL;
	} 
	else if (pOLWinAttr->win_type == wmGD.xa_OL_WT_BASE)
	{
	    /* 
	     * Base windows can have all decorations
	     */
	    decorMask = WM_DECOR_ALL;
	} 
	else if (pOLWinAttr->win_type == wmGD.xa_OL_WT_COMMAND) 
	{
	    /*
	     * Command windows have titles, pins (close button), and
	     * resize handles.
	     */
	    decorMask = WM_DECOR_TITLE | WM_DECOR_SYSTEM | WM_DECOR_RESIZEH;
	}
	else if (pOLWinAttr->win_type == wmGD.xa_OL_WT_HELP) 
	{
	    /*
	     * Help windows have titles and pins (close button).
	     * No resize, but give it a border to look nicer.
	     */
	    decorMask = (WM_DECOR_TITLE | WM_DECOR_SYSTEM | WM_DECOR_BORDER);
	}
	else if ((pOLWinAttr->win_type == wmGD.xa_OL_WT_NOTICE)  &&
		 (pOLWinAttr->win_type == wmGD.xa_OL_WT_OTHER))
	{
	    decorMask = WM_DECOR_NONE;
	} 
	else
	{
	    decorMask = WM_DECOR_ALL;
	}

	if (GetOLDecorAdd(pCD,&OLdecor))
	{
	    if (OLdecor & OLDecorResizeable)
	    {
		decorMask |= WM_DECOR_RESIZEH;
	    }
	    if (OLdecor & OLDecorHeader)
	    {
		decorMask |= WM_DECOR_TITLE;
	    }
	    if (OLdecor & OLDecorCloseButton)
	    {
		/* OL "close" is same as "Motif" minimize */
		decorMask |= MWM_DECOR_MINIMIZE;
	    }
	    if (OLdecor & OLDecorPushPin)
	    {
		/* 
		 * windows with pins can't be minimized 
		 */
		decorMask &= ~MWM_DECOR_MINIMIZE;
		decorMask |= MWM_DECOR_TITLE | MWM_DECOR_MENU;
	    }
	}

	if (GetOLDecorDel(pCD,&OLdecor))
	{
	    if (OLdecor & OLDecorResizeable)
	    {
		decorMask &= ~MWM_DECOR_RESIZEH;
	    }
	    if (OLdecor & OLDecorHeader)
	    {
		decorMask &= ~WM_DECOR_TITLEBAR;
	    }
	    if (OLdecor & OLDecorCloseButton)
	    {
		/* OL "close" is same as "Motif" minimize */
		decorMask &= ~MWM_DECOR_MINIMIZE;
	    }

	    /* push pin is ignored here */

	}

	/*
	 * If the window has a push pin or a limited menu,
	 * then consider it very similar to a secondary window.
	 */
#ifdef FWS
	/*
	 * Secondary windows are not allowed for FWS clients.
	 */
	if (!pCD->fwsClient) {
#endif /* FWS */
	if (((pOLWinAttr->flags & WA_PINSTATE) &&
	     (pOLWinAttr->pin_initial_state == PIN_IN)) ||
	    ((pOLWinAttr->flags & WA_MENUTYPE) &&
	     (pOLWinAttr->menu_type == wmGD.xa_OL_MENU_LIMITED)))
	{
	    decorMask &= ~(MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE);
	    pCD->bPseudoTransient = True;
	    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
	}
#ifdef FWS
	}
#endif /* FWS */

	/* 
	 * Reduce decoration on this window according to OL hints
	 */
	pCD->clientDecoration &= decorMask;

	/*
	 * Reduce client functions if necessary.
	 */
	if (!(decorMask & MWM_DECOR_MINIMIZE))
	{
	    pCD->clientFunctions &= ~MWM_FUNC_MINIMIZE;
	}
	if (!(decorMask & MWM_DECOR_MAXIMIZE))
	{
	    pCD->clientFunctions &= ~MWM_FUNC_MAXIMIZE;
	}
	if (!(decorMask & MWM_DECOR_RESIZEH))
	{
	    pCD->clientFunctions &= ~MWM_FUNC_RESIZE;
	}

        /* 
	 * Set the clients secondariesOnTop value to false to allow
	 * open look transient behaviour (transients below primary).
	 */
        pCD->secondariesOnTop = False;
	
	if (pOLWinAttr)
	    XFree((char *)pOLWinAttr);
    }

} /* END OF FUNCTION ProcessOLDecoration */

#endif /* NO_OL_COMPAT */
