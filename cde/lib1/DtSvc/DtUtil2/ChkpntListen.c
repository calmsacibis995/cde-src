/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* -*-C-*-
*******************************************************************************
*
* File:         ChkpntListen.c
* Description:  CDE listener-side checkpoint protocol functions. Internal API 
*               functions for use by the CDE checkpoint listener.
* Created:      Mon Sep  6 09:00:00 1993
* Language:     C
*
* $XConsortium: ChkpntListen.c /main/cde1_maint/2 1995/10/19 10:20:31 drk $
*
* (C) Copyright 1993, Hewlett-Packard, all rights reserved.
*
*******************************************************************************
*/

#define INVALID_TIME	((Time) -1)

#include <stdio.h>
#include <string.h>
#include "Dt/ChkpntP.h"

static struct {
    Display	*display;		/* Display pointer 		*/
    Window      window;			/* Window id for the program    */
    Atom        aSelection;		/* Atom  for root selection 	*/
    Boolean     bListen;		/* Should I do listening ?      */
} dtsvc_info;

/* _DtPerfChkpntListenInit(): Start the Checkpoint listener */
#ifdef _NOPROTO
_DtPerfChkpntListenInit(display, parentwin)
Display *display;	/* Current display */
Window  parentwin;	/* Parent of window associated with listener */
#else
_DtPerfChkpntListenInit(Display *display, Window parentwin)
#endif
{
    Time   timestamp = INVALID_TIME;
    Window tmpwin;
    Bool   bsuccess = False;

    dtsvc_info.display    = display;
    dtsvc_info.window     = XCreateSimpleWindow(display, parentwin,
			      1, 1, 100, 100, 1,
			      BlackPixel(display,DefaultScreen(display)),
			      WhitePixel(display,DefaultScreen(display)));
    dtsvc_info.bListen    = True;
    /*
     * Assert ownership over the appropriate root window selection
     */
    dtsvc_info.aSelection = XInternAtom(display, DT_PERF_CHKPNT_SEL, False);
    if (XGetSelectionOwner(dtsvc_info.display, dtsvc_info.aSelection) == None) {
	if (timestamp == INVALID_TIME) { /* Generate a valid timestamp */
	    XEvent event;
	    Atom aProperty;
	    char   propname[80];	/* Temp buffer for property name */

	    sprintf(propname, "%s_%x", DT_PERF_CLIENT_CHKPNT_PROP, 0);
	    aProperty  = XInternAtom(dtsvc_info.display, propname,False);
	    XSelectInput(dtsvc_info.display,
				    dtsvc_info.window, PropertyChangeMask);

	    XChangeProperty(dtsvc_info.display,  dtsvc_info.window,
				    aProperty, XA_STRING, 8,
				    PropModeAppend, (unsigned char *) NULL, 0);
	    XFlush(dtsvc_info.display);
	    loop:
	    XWindowEvent(dtsvc_info.display, dtsvc_info.window,
				    PropertyChangeMask,  &event);
	    if (event.type == PropertyNotify) {
		timestamp = event.xproperty.time;
	    }
	    else goto loop;
	}
	XSetSelectionOwner(dtsvc_info.display, dtsvc_info.aSelection,
			   dtsvc_info.window,  timestamp);
	tmpwin = XGetSelectionOwner(dtsvc_info.display, dtsvc_info.aSelection);
	if ( tmpwin == dtsvc_info.window)
		bsuccess = True;/* We are now the listener! */
    }

    if (bsuccess == False) {
	fprintf(stderr,
	  "\t** Chkpnt listener: Cannot gain ownership of root selection **\n");
	fprintf(stderr,
	  "\t** Chkpnt listener: Selection is owned by window: %lx **\n",
	  (long) tmpwin);
	dtsvc_info.bListen = False;
	return(0);
    }

    /* Express interests in Events on this window */
    XSelectInput(dtsvc_info.display, dtsvc_info.window,
		 SelectionRequest | SelectionClear);

    return(1);
}

/* Helper Boolean function to pass to XCheckIfEvent() */
#ifdef _NO_PROTO
static Bool myCheckSelectionEvent(display, event, args)
Display *display;
XEvent  *event;
char    *args;

#else
static Bool myCheckSelectionEvent(Display *display, XEvent *event, char *args)
#endif
{
    if (event->xany.window != dtsvc_info.window)/* Only listener window events*/
	return(False);
    else switch(event->type) {		        /* Selection stuff ? */
	case SelectionClear:
	case SelectionRequest:
	    return (True);
	    break;
	default:
	    return (False);
	    break;
    }
}

/*
 * _DtPerfChkpntMsgReceive() Non blocking fetch from message queue
 */
#ifdef _NO_PROTO
Bool _DtPerfChkpntMsgReceive(dtcp_msg, bBlock)
DtChkpntMsg *dtcp_msg;
Bool        bBlock;		/* Block until a message is recieved ?*/

#else
Bool _DtPerfChkpntMsgReceive(DtChkpntMsg *dtcp_msg, Bool bBlock)
#endif
{
    XEvent        event;
    Bool          bool=True;
    XTextProperty tp;
    int 	  i;
    static char   **Stringlist;
    static int    numfields = 0;

    if (dtsvc_info.bListen == False)
	return(False);

    if (numfields) {	/* Free the storage from last time around */
	XFreeStringList(Stringlist);
	numfields = 0;
    }

    if (bBlock == True)
	XIfEvent(dtsvc_info.display, &event, myCheckSelectionEvent, NULL);
    else
	bool = XCheckIfEvent(dtsvc_info.display, &event,
				myCheckSelectionEvent, NULL);
    
    if (bool == True) {
	switch (event.type) {
	    case SelectionRequest:	/* Message received from a client */
		/* Is this a Checkpoint request ?*/
		if (event.xselectionrequest.selection == dtsvc_info.aSelection){
		    /* Correct selection, now fetch the property */
		    /*
		     * Note: Need to handle errors if the client is dead
		     * and the property no longer exists.
		     */
		    XGetTextProperty(dtsvc_info.display,
				     event.xselectionrequest.requestor,
				     &tp,
				     event.xselectionrequest.property);
		    XDeleteProperty (dtsvc_info.display, 
				     event.xselectionrequest.requestor,
				     event.xselectionrequest.property);
		    XTextPropertyToStringList(&tp, &Stringlist, &numfields);
		    XFree(tp.value);
		    for (i = 0; i < DT_PERF_CHKPNT_MSG_SIZE; i++)
			dtcp_msg->array[i] = (Stringlist)[i];
		}
		break;
	    /* */
	    case SelectionClear:	/* We no longer own the selection */
	    default:
		fprintf(stderr,"\t** Chkpnt listener: Warning - loss of Selection ownership **\n");
		bool = False;
		break;
	}
    }
    /* End? Destroy the, RetainPermanent client window */
    if (!strcmp(dtcp_msg->record.type, DT_PERF_CHKPNT_MSG_END) ){
      XDestroyWindow(dtsvc_info.display, event.xselectionrequest.requestor);
    }
    return(bool);
}
