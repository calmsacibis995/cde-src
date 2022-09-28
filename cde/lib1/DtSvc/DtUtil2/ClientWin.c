/* $XConsortium: ClientWin.c,v 1.3 89/12/10 10:26:35 rws Exp $ */

/* 
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Window XmuClientWindow() ;

#else

extern Window XmuClientWindow( 
                        Display *dpy,
                        Window win) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Window TryChildren() ;

#else

static Window TryChildren( 
                        Display *dpy,
                        Window win,
                        Atom WM_STATE) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/



/* Find a window with WM_STATE, else return win itself, as per ICCCM */

Window 
#ifdef _NO_PROTO
XmuClientWindow( dpy, win )
        Display *dpy ;
        Window win ;
#else
XmuClientWindow(
        Display *dpy,
        Window win )
#endif /* _NO_PROTO */
{
    Atom WM_STATE;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf;

    WM_STATE = XInternAtom(dpy, "WM_STATE", True);
    if (!WM_STATE)
	return win;
    XGetWindowProperty(dpy, win, WM_STATE, 0, 0, False, AnyPropertyType,
		       &type, &format, &nitems, &after, &data);
    if (type)
	return win;
    inf = TryChildren(dpy, win, WM_STATE);
    if (!inf)
	inf = win;
    return inf;
}

static Window 
#ifdef _NO_PROTO
TryChildren( dpy, win, WM_STATE )
        Display *dpy ;
        Window win ;
        Atom WM_STATE ;
#else
TryChildren(
        Display *dpy,
        Window win,
        Atom WM_STATE )
#endif /* _NO_PROTO */
{
    Window root, parent;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf = 0;

    if (!XQueryTree(dpy, win, &root, &parent, &children, &nchildren))
	return 0;
    for (i = 0; !inf && (i < nchildren); i++) {
	XGetWindowProperty(dpy, children[i], WM_STATE, 0, 0, False,
			   AnyPropertyType, &type, &format, &nitems,
			   &after, &data);
	if (type)
	    inf = children[i];
    }
    for (i = 0; !inf && (i < nchildren); i++)
	inf = TryChildren(dpy, children[i], WM_STATE);
    if (children) XFree((char *)children);
    return inf;
}
