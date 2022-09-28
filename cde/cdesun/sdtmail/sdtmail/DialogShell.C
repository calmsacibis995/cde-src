/*
 *+SNOTICE
 *
 *	$Revision: 1.1 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DialogShell.C	1.12 10/07/96"
#else
static char *sccs__FILE__ = "@(#)DialogShell.C	1.12 10/07/96";
#endif

#include <assert.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <Dt/Wsm.h>
#include <Dt/Session.h>
#include <DtMail/IO.hh>

// The following headers are private to CDE and should NOT be required
// but unfortunately are.
//
extern "C" {
#include <Dt/HourGlass.h>
}
#include <Dt/Icon.h>
#include <Dt/IconP.h>
#include <Dt/IconFile.h>

#include "DialogShell.h"
#include "Application.h"
#include "RoamMenuWindow.h"


DialogShell::DialogShell(
    char *name,
    RoamMenuWindow *parent
)
    : UIComponent(
	name
)
{
    _parent=parent;
    _workArea=NULL;
    
    assert( theApplication );
}

DialogShell::~DialogShell()
{
    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( _w ),
					"WM_DELETE_WINDOW",
					False );
    XmRemoveWMProtocolCallback( _w,
				WM_DELETE_WINDOW,
				( XtCallbackProc ) quitCallback,
				NULL );

}


void
DialogShell::initialize()
{
    _w = XtVaCreatePopupShell( _name,
			       xmDialogShellWidgetClass,
			       _parent->baseWidget(),
			       XmNdefaultPosition, False,
			       NULL, 0 );
    installDestroyHandler();

     XtVaSetValues( _w,
		    XmNdefaultPosition, False,
		    NULL );

    XtAddCallback( _w,
		   XmNpopupCallback,
		   ( XtCallbackProc ) &DialogShell::popupCallback,
		   XtPointer( this ) );


    
     XtAddCallback( _w,
		   XmNpopdownCallback,
		   ( XtCallbackProc ) &DialogShell::popdownCallback,
		   XtPointer( this ) );



     _workArea = createWorkArea ( _w );  

     assert ( _workArea );

    Atom WM_DELETE_WINDOW=XmInternAtom( XtDisplay( _w ),
					"WM_DELETE_WINDOW",
					False );

    XmAddWMProtocolCallback( _w,
			     WM_DELETE_WINDOW,
			     ( XtCallbackProc ) quitCallback,
			     this );
    
//      if (!XtIsManaged(_workArea))
// 	 XtManageChild(_workArea); 
 }


void
DialogShell::title(
    const char *text
)
{
    XtVaSetValues ( _w, XmNtitle, text, NULL );
}


void
DialogShell::popupCallback( Widget ,
		       XtPointer clientData,
		       XmAnyCallbackStruct *
)
{
    DialogShell *obj=( DialogShell * ) clientData;
    
    obj->popped_up();
    
}


void
DialogShell::popdownCallback( Widget ,		// w
			 XtPointer clientData,
			 XmAnyCallbackStruct *
)
{
    DialogShell *obj=( DialogShell * ) clientData;
    
    obj->popped_down();
    
}

void
DialogShell::manage()
{
    if ( !XtIsManaged ( _workArea ) )
	XtManageChild ( _workArea ); 
    UIComponent::manage();
}


void
DialogShell::quitCallback( Widget,
			  XtPointer clientData,
			  XmAnyCallbackStruct *)
{
  DialogShell *dlg = ( DialogShell *) clientData;
  dlg->quit();
}

void
DialogShell::busyCursor()
{
    // Do nothing if the widget has not been realized

    if (XtIsRealized(_w)) {
	_DtTurnOnHourGlass(_w);
    }
}

void
DialogShell::normalCursor()
{
    // Do nothing if the widget has not been realized
    
    if (XtIsRealized ( _w ))
    {
	_DtTurnOffHourGlass(_w);
    }
}
