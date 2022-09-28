/*
 *  MsguProp.c -- Property handling utilities
 *
 *  $XConsortium: MsguProp.c /main/cde1_maint/1 1995/07/14 20:37:36 drk $
 *  $XConsortium: MsguProp.c /main/cde1_maint/1 1995/07/14 20:37:36 drk $
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
 * _DtMsgGetMessage  -- get a property and extract the message
 */

int
#ifdef _NO_PROTO
_DtMsgGetMessage (display, window, property, pppchMessage, pcount)
	Display		*display;
	Window		window;
	Atom		property;
	char ***	pppchMessage;
	int *		pcount;
#else 
_DtMsgGetMessage (
	Display		*display,
	Window		window,
	Atom		property,
	char ***	pppchMessage,
	int *		pcount 
	)
#endif
{
    int status;
    XTextProperty tp;

    if (XGetTextProperty (display, window, &tp, property))
    {
	if (tp.format == 8)
	{
	    /* clean up */
	    XDeleteProperty (display, window, property);

	    XmbTextPropertyToTextList (display, &tp, pppchMessage, pcount);

	    XFree (tp.value);
	    status = dtmsg_SUCCESS;
	}
	else
	{
	    status = dtmsg_WRONG_FORMAT;
	}
    }
    else 
    {
	status = dtmsg_FAIL;
    }
    return (status);
}

/*
 * _DtMsgSetMessage  -- Pack up a message and put it in the right
 * 			 place
 */

int
#ifdef _NO_PROTO
_DtMsgSetMessage (display, window, property, ppchMessage, count)
	Display		*display;
	Window		window;
	Atom		property;
	char **		ppchMessage;
	int		count;
#else 
_DtMsgSetMessage (
	Display		*display,
	Window		window,
	Atom		property,
	char **		ppchMessage,
	int		count 
	)
#endif
{
    int status;
    XTextProperty tp;
    
    status = XmbTextListToTextProperty (display, ppchMessage, count,
					XStdICCTextStyle, &tp);


    if ((status == Success) || (status > 0))
    {
	/*
	 * Complete or partial conversion
	 */
	
	XSetTextProperty (display, window, &tp, property);

	XFree (tp.value);

	status = dtmsg_SUCCESS;
    }
    else
    {
	status = dtmsg_FAIL; /* total failure */
    }

    return (status);
}
