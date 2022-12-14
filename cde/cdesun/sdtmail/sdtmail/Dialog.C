/*
 *+SNOTICE
 *
 *	$Revision: 1.17 $
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
#pragma ident "@(#)Dialog.C	1.17 05/10/96"
#else
static char *sccs__FILE__ = "@(#)Dialog.C	1.17 05/10/96";
#endif

#include "Dialog.h"
#include <Application.h>
#include "RoamMenuWindow.h"
#include <Xm/DialogS.h>
#include <assert.h>
#include "Help.hh"

// The following header is private to CDE and should NOT be required
// but unfortunately is.
//
extern "C" {
#include <Dt/HourGlass.h>
}


Dialog::Dialog(char *name, RoamMenuWindow *parent) : UIComponent(name)
{
  _parent = parent;
  _workArea = NULL;
    
  _w = XtCreatePopupShell(_name,
			  xmDialogShellWidgetClass,
			  parent->baseWidget(),
			  NULL, 0 );

  XtVaSetValues(_w, XmNdefaultPosition, False, NULL);

  installDestroyHandler();

  assert( theApplication );
}

Dialog::Dialog(RoamMenuWindow *parent) : UIComponent("")
{
  _parent = parent;
  _workArea=NULL;
}

Dialog::~Dialog()
{
}

void
Dialog::initialize()
{
    // Since one of the constructors does not create the dialog shell,
    // we need to make sure it exists before we go off setting resources
    // for it.
    assert(_w);

    XtVaSetValues( _w,
		   XmNdefaultPosition, False, NULL );
    
    XtAddCallback( _w,
		   XmNpopupCallback,
		   ( XtCallbackProc ) &Dialog::popupCallback,
		   XtPointer( this ) );
    
    XtAddCallback( _w,
		   XmNpopdownCallback,
		   ( XtCallbackProc ) &Dialog::popdownCallback,
		   XtPointer( this ) );
    
    _workArea = createWorkArea ( _w );  
    assert ( _workArea );

    if ( !XtIsManaged ( _workArea ) )
	XtManageChild ( _workArea ); 
}


void
Dialog::title(
    char *text
)
{
    XtVaSetValues ( _w, XmNtitle, text, NULL );
}


void
Dialog::popupCallback( Widget ,
		       XtPointer clientData,
		       XmAnyCallbackStruct *
)
{
    Dialog *window=( Dialog * ) clientData;
    
    window->popped_up();
    
}


void
Dialog::popdownCallback( Widget ,		// w
			 XtPointer clientData,
			 XmAnyCallbackStruct *
)
{
    Dialog *window=( Dialog * ) clientData;
    
    window->popped_down();
    
}

void
Dialog::manage()
{
    if ( !XtIsManaged ( _workArea ) )
	XtManageChild ( _workArea ); 
    UIComponent::manage();
}

void
Dialog::busyCursor()
{
    // Do nothing if the widget has not been realized.

    if (XtIsRealized(_w)) {
	_DtTurnOnHourGlass(_w);
    }
}

void
Dialog::normalCursor()
{
    // Do nothing if the widget has not been realized

    if (XtIsRealized(_w)) {
	_DtTurnOffHourGlass(_w);
    }
}
