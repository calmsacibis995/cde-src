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
#pragma ident "@(#)SelectFileCmd.C	1.29 05/23/95"
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


//////////////////////////////////////////////////////////
// SelectFileCmd.C: 
//////////////////////////////////////////////////////////
#include <unistd.h>
#include <stdlib.h>
#include "SelectFileCmd.h"
#include "Application.h"
#include <Xm/FileSB.h>
#include "Help.hh"

#include <nl_types.h>
extern nl_catd catd;

extern "C" {
#include <Dt/HourGlass.h>
}

extern forceUpdate( Widget );

SelectFileCmd::SelectFileCmd (const char * name, 
			      const char * label,
			      const char * title,
			      const char * ok_label,
			      int          active,
			      FileCallback callback,
			      void        *clientData,
			      Widget       parent) :
			      NoUndoCmd ((char *)name, (char *)label, active )
{
    _ok_label = (ok_label ? strdup(ok_label) : strdup("OK"));
    _title = (title ? strdup(title) : strdup(name));
    _callback   = callback;
    _clientData = clientData;
    _fileBrowser = NULL;
    _parentWidget = parent;
    _directory = NULL;
}

SelectFileCmd::~SelectFileCmd()
{
    free(_ok_label);
    free(_title);
}


void SelectFileCmd::doit()
{
    // Create a FileSelectionBox widget
    
    Arg args[1];
    Cardinal n = 0;
    XmString title;

    // If the FSB already exists and is managed, raise it.

    if (_fileBrowser) {
        XtManageChild ( _fileBrowser );
        XtPopup (XtParent(_fileBrowser), XtGrabNone );
	XRaiseWindow(XtDisplay(_fileBrowser), XtWindow(XtParent(_fileBrowser)));
	forceUpdate(_fileBrowser);
	return;
    }

    // If there is no FSB, create it and manage it.
    // If there is one, just manage it.

    // Creating one is different from the book. cast required.
    // Also, its the "new CDE" FSB!

    if (!_fileBrowser) {
	_DtTurnOnHourGlass(_parentWidget);

	_fileBrowser =
	    XmCreateFileSelectionDialog (_parentWidget,
					 (char *) name(), 
					 args, n );       
	// Set the title right...
	title = XmStringCreateLocalized(_title);
	XmString ok_str = XmStringCreateLocalized(_ok_label);
	XtVaSetValues(_fileBrowser,
		      XmNdialogTitle, title,
		      XmNokLabelString, ok_str,
		      NULL);
	XmStringFree(title);
	XmStringFree(ok_str);

    	if (_directory)
	    XtVaSetValues(_fileBrowser, XmNdirectory, _directory, NULL);
	XmString hidden_str = XmStringCreateLocalized(catgets(catd, 1, 11, "Show hidden folders and files"));
	_hidden_button = XtVaCreateManagedWidget("hidden",
						 xmToggleButtonWidgetClass,
						 _fileBrowser,
						 XmNlabelString, hidden_str,
						 XmNalignment, XmALIGNMENT_BEGINNING,
						 XmNsensitive, False,
						 NULL);
	XmStringFree(hidden_str);

	printHelpId("_fileBrowser", _fileBrowser);
	/* add help callback */
	// XtAddCallback(_fileBrowser, XmNhelpCallback, HelpCB, helpId);
	
	// Set up the callback to be called when the user chooses a file
	    
	XtAddCallback ( _fileBrowser, 
	    XmNokCallback,
	    &SelectFileCmd::fileSelectedCallback, 
	    (XtPointer) this );
	XtAddCallback ( _fileBrowser,
	    XmNcancelCallback,
	    &SelectFileCmd::cancelCallback,
	    (XtPointer) this );
	XtAddCallback(_hidden_button,
		      XmNvalueChangedCallback,
		      &SelectFileCmd::hiddenCB,
		      this);

	XtSetSensitive(
	    XmFileSelectionBoxGetChild(_fileBrowser, XmDIALOG_HELP_BUTTON),
	    False);

	_DtTurnOffHourGlass(_parentWidget);
    }
    
    // Display the dialog
    
    XtManageChild ( _fileBrowser );

    // Raise it, because it might be buried.
    //
    XRaiseWindow(XtDisplay(_fileBrowser), XtWindow(XtParent(_fileBrowser)));
}

void SelectFileCmd::fileSelectedCallback ( Widget    w,
					  XtPointer clientData,
					  XtPointer callData )
{
    SelectFileCmd * obj = (SelectFileCmd *) clientData;
    XmFileSelectionBoxCallbackStruct *cb = 
	(XmFileSelectionBoxCallbackStruct *) callData;
    char     *name   = NULL;
    XmString  xmstr  = cb->value;  // The selected file
    int       status = 0;

    XtUnmanageChild ( w );   // Bring the file selection dialog down.
    
    if ( xmstr )   // Make sure a file was selected
    {
	// Extract the first character string matching the default
	// character set from the compound string
	
	status = XmStringGetLtoR ( xmstr, 
				  XmSTRING_DEFAULT_CHARSET,
				  &name );
	
	// If a string was succesfully extracted, call
	// fileSelected to handle the file.
	
	if ( status )
	    obj->fileSelected ( name );
    }

}

void SelectFileCmd::cancelCallback ( Widget w,
				     XtPointer,
				     XtPointer )
{
    XtUnmanageChild ( w );   // Bring the file selection dialog down.
}

void SelectFileCmd::fileSelected ( char *filename )
{
    if ( _callback )
	_callback ( _clientData, filename );
}

void
SelectFileCmd::hiddenCB(Widget,
			XtPointer client_data,
			XtPointer cb_data)
{
    SelectFileCmd * self = (SelectFileCmd *)client_data;
    XmToggleButtonCallbackStruct * cbs = (XmToggleButtonCallbackStruct *)cb_data;

    self->doHidden(cbs);
}

void
SelectFileCmd::doHidden(XmToggleButtonCallbackStruct * cbs)
{
    // Need to add our own fileSearchProc and dirSearchProc to
    // handle displaying hidden files and directories.  In this
    // function we will need to set some data member like
    // _showHidden so that our new search procs can do the right
    // thing.
//    _showHidden = cbs->set;

    XmFileSelectionDoSearch(_fileBrowser, NULL);
}

void
SelectFileCmd::setDirectory(char * path)
{
    // Set the default directory where the file selection box points.
    if (_directory) {
	XmStringFree(_directory);
    }
    _directory = XmStringCreateLocalized(path);
    if (_fileBrowser) {
	XtVaSetValues(_fileBrowser, XmNdirectory, _directory, NULL);
    }
}
