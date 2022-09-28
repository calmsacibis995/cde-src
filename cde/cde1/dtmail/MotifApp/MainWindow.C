/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MainWindow.C	1.27 10/20/95"
#endif

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// MainWindow.C: Support a toplevel window
////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/MainW.h>
#include <Dt/Wsm.h>
#include <Dt/Session.h>
#include <DtMail/IO.hh>
#include "Application.h"
#include "Shell.h"
#include "MainWindow.h"
#include "Help.hh"

// The following headers are private to CDE and should NOT be required
// but unfortunately are.
//
extern "C" {
#include <Dt/HourGlass.h>
}
#include <Dt/Icon.h>
#include <Dt/IconP.h>
#include <Dt/IconFile.h>

// This is a private CDE function that should be public, but is not,
// and does not even have a prototype in a header. Yes, it is required.
//
extern "C" Pixmap _DtGetMask(Screen * screen, char * image_name);

static const char * DefaultIcon = "Dtablnk";
static const unsigned long FLASH_INTERVAL = 250; // milliseconds

MainWindow::MainWindow( char *name ) : UIComponent ( name )
{
    _workArea = NULL;
    _flashing = 0;
    _icon_invert = NULL;
    _window_invert = NULL;
    _icon = NULL;

    assert ( theApplication ); // Application object must exist
    // before any MainWindow object
    theApplication->registerWindow ( this );
}

void
MainWindow::initialize( )
{
    // All toplevel windows in the MotifApp framework are 
    // implemented as a popup shell off the Application's
    // base widget.
    //
    // XmNdeleteResponse is being set to DO_NOTHING so 
    // that the user can Cancel their close request.

    _w = XtVaCreatePopupShell ( _name, 
				topLevelShellWidgetClass,
				theApplication->baseWidget(),
				XmNdeleteResponse, XmDO_NOTHING,
				NULL, 0 );

    installDestroyHandler();
    
    // Use a Motif XmMainWindow widget to handle window layout
    
    _main = XtCreateManagedWidget ( "mainWindow", 
				   xmMainWindowWidgetClass,
				   _w, 
				   NULL, 0 );
    printHelpId("_main", _main);
    /* install callback */
    // XtAddCallback(_main, XmNhelpCallback, HelpCB, helpId);
    XtAddCallback(_main, XmNhelpCallback, 
			    HelpCB, "_HOMETOPIC");
    
    // Called derived class to create the work area
    
    _workArea = createWorkArea ( _main );  
    assert ( _workArea );
    
    // Designate the _workArea widget as the XmMainWindow
    // widget's XmNworkWindow widget
    
    XtVaSetValues ( _main, 
		   XmNworkWindow, _workArea,
		   NULL );

    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( _w ),
					"WM_DELETE_WINDOW",
					False );

    XmAddWMProtocolCallback( _w,
			     WM_DELETE_WINDOW,
			     ( XtCallbackProc ) quitCallback,
			     this );

// Why are we setting the icon to Dtablnk.  This is simply going to
// be overriden by some other function setting it to the appropriate 
// icon.
//    setIconName(DefaultIcon);

    _window_invert = NULL;

    // Manage the work area if the derived class hasn't already.
    
    if ( !XtIsManaged ( _workArea ) )
	XtManageChild ( _workArea ); 
}

MainWindow::~MainWindow( )
{
    // Unregister this window with the Application object

    if (_w) {
	Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( _w ),
					    "WM_DELETE_WINDOW",
					    False );
	XmRemoveWMProtocolCallback( _w,
				    WM_DELETE_WINDOW,
				    ( XtCallbackProc ) quitCallback,
				    NULL );

	if (_icon_invert) {
	    XFreeGC(XtDisplay(_w), _icon_invert);
	}
	if (_window_invert) {
	    XFreeGC(XtDisplay(_w), _window_invert);
	}
    }
    
    theApplication->unregisterWindow ( this );
}

void
MainWindow::manage()
{
    assert ( _w );
    XtPopup ( _w, XtGrabNone );
    
    // Map the window, in case the window is iconified

    if ( XtIsRealized ( _w ) ) 
	XMapRaised ( XtDisplay ( _w ), XtWindow ( _w ) );
}

void
MainWindow::unmanage()
{
    assert ( _w );
    XtPopdown ( _w );
}

void
MainWindow::iconify()
{
    assert ( _w );
    
    // Set the widget to have an initial iconic state
    // in case the base widget has not yet been realized
    
    XtVaSetValues ( _w, XmNiconic, TRUE, NULL );
    
    // If the widget has already been realized,
    // iconify the window
    
    if ( XtIsRealized ( _w ) )
	XIconifyWindow ( XtDisplay ( _w ), XtWindow ( _w ), 0 );
}

void
MainWindow::setIconTitle(const char * title)
{
    XtVaSetValues(_w, XmNiconName, title, NULL);
}

void
MainWindow::setIconName(const char * path)
{
    char * icon_filename = XmGetIconFileName(XtScreen(_w),
					     NULL,
					     (char *)path, // Bug!
					     NULL,
					     DtLARGE);

    if (icon_filename == NULL) {
	return;
    }

    Pixel fg = 0, bg = 0;

    getIconColors(fg, bg);

    _icon = XmGetPixmap(XtScreen(_w),
			icon_filename,
			fg, bg);

//    Pixmap icon_mask_map = _DtGetMask(XtScreen(_w), icon_filename);

//     if (!_icon || !icon_mask_map) {
// 	return;
//     }

    if (!_icon) {
	return;
    }

    XtVaSetValues(_w,
		  XmNiconPixmap, _icon,
//		  XmNiconMask, icon_mask_map,
		  NULL);

    // Build the inverted icon mask for flashing.
    //
    if (_icon_invert) {
	XFreeGC(XtDisplay(_w), _icon_invert);
    }

    XGCValues	gc_vals;

    gc_vals.foreground = bg;
    gc_vals.function = GXxor;
    _icon_invert = XCreateGC(XtDisplay(_w), _icon, GCForeground | GCFunction,
			     &gc_vals);

    free(icon_filename);
}

void
MainWindow::busyCursor()
{
    // Do nothing if the widget has not been realized

    if (XtIsRealized(_w)) {
	_DtTurnOnHourGlass(_w);
    }
}

void
MainWindow::normalCursor()
{
    // Do nothing if the widget has not been realized
    
    if (XtIsRealized ( _w ))
    {
	_DtTurnOffHourGlass(_w);
    }
}

void
MainWindow::setStatus(const char *)
{
    // Noop in our case.
}

void
MainWindow::clearStatus(void)
{
    // Noop in our case.
}

void
MainWindow::title(const char *text )
{
    XtVaSetValues ( _w, XmNtitle, (char *)text, NULL );
}

void
MainWindow::quitCallback( Widget,
			  XtPointer clientData,
			  XmAnyCallbackStruct *)
{
    MainWindow *window=( MainWindow *) clientData;
    window->quit();
}

void
MainWindow::getIconColors(Pixel & fore, Pixel & back)
{
    XtVaGetValues (_w,
		   XmNforeground, &fore,
		   XmNbackground, &back,
		   NULL);
}

struct WM_STATE {
    int		state;
    Window	icon;
};

static int
getWindowState(Widget w)
{
    Atom 	wmStateAtom, actualType;
    int 	actualFormat;
    unsigned long nitems, leftover;
    WM_STATE *wmState;
 
    /*  Getting the WM_STATE property to see if iconified or not */
    wmStateAtom = XInternAtom(XtDisplay(w), "WM_STATE", False);
 
    XGetWindowProperty (XtDisplay(w), XtWindow(w), 
			wmStateAtom, 0L,
                        (long)BUFSIZ, False, wmStateAtom, &actualType,
                        &actualFormat, &nitems, &leftover,
                        (unsigned char **) &wmState);

    if (wmState) {
	return(wmState->state);
    } else {
	return(NULL);
    }
}

void
MainWindow::flash(const int count)
{
    if (count == 0) {
	return;
    }

    if (_flashing > 0) {
	// Don't be annoying.
	//
	return;
    }

    if (_window_invert == NULL) {
	// Create a GC to flash the window.
	//
	XGCValues	gc_vals;
	Pixel	fg, bg;
	getIconColors(fg, bg);
	
	gc_vals.foreground = bg;
	gc_vals.function = GXxor;
	_window_invert = XCreateGC(XtDisplay(_w), XtWindow(_w),
				   GCForeground | GCFunction, &gc_vals);
	XSetSubwindowMode(XtDisplay(_w), _window_invert, IncludeInferiors);
    }

    _last_state = getWindowState(_w);

    _flashing = count * 2;
    XtAppAddTimeOut(XtWidgetToApplicationContext(_w),
		    FLASH_INTERVAL, flashCallback, this);
}

void
MainWindow::flashCallback(XtPointer client_data, XtIntervalId * interval_id)
{
    MainWindow * mw = (MainWindow *)client_data;

    mw->doFlash(interval_id);
}

void
MainWindow::doFlash(XtIntervalId *)
{
    int state = getWindowState(_w);

    // We are going to make things flash an even number of times.
    // to do this, we will lie about the state, and leave it at the
    // old state for one iteration.
    if (state != _last_state && (_flashing % 2) != 0) {
	state = _last_state;
    }

    if (state == IconicState) {
	Pixmap	image = _icon;

	XFillRectangle(XtDisplay(_w), image, _icon_invert, 0, 0, 48, 48);
	XtVaSetValues(_w, XmNiconPixmap, NULL, NULL);
	XtVaSetValues(_w, XmNiconPixmap, image, NULL);
    }
    else if (state != NULL) {  // If (state == NULL) we couldn't get the state.
	XWindowAttributes	window_attributes;

	XGetWindowAttributes(XtDisplay(_w), XtWindow(_w), &window_attributes);

	XFillRectangle(XtDisplay(_w), XtWindow(_w), _window_invert, 0, 0,
		       window_attributes.width, window_attributes.height);
    }

    _flashing -= 1;
    if (_flashing > 0) {
	XtAppAddTimeOut(XtWidgetToApplicationContext(_w),
			FLASH_INTERVAL, flashCallback, this);
	_last_state = state;
    }
}

Boolean
MainWindow::isIconified()
{
 
    Atom 	wmStateAtom, actualType;
    int 	actualFormat;
    unsigned long nitems, leftover;
    WM_STATE *wmState;
 
    assert ( _w );

    /*  Getting the WM_STATE property to see if iconified or not */
    wmStateAtom = XInternAtom(XtDisplay(_w), "WM_STATE", False);
 
    XGetWindowProperty (XtDisplay(_w), XtWindow(_w), 
			wmStateAtom, 0L,
                        (long)BUFSIZ, False, wmStateAtom, &actualType,
                        &actualFormat, &nitems, &leftover,
                        (unsigned char **) &wmState);


    if (wmState) {
	if (wmState->state == IconicState) {
	    return(TRUE);
	}
    }
    return(FALSE);
}
