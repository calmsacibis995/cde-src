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
#pragma ident "@(#)SelectFileCmd.C	1.36 03/28/97"
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
			      FileCallback2 dircallback,
			      void        *clientData,
			      void        *clientData2,
			      Widget       parent) :
			      NoUndoCmd ((char *)name, (char *)label, active )
{
    _ok_label = (ok_label ? strdup(ok_label) : strdup("OK"));
    _title = (title ? strdup(title) : strdup(name));
    _callback   = callback;
    _dircallback   = dircallback;
    _clientData = clientData;
    _clientData2 = clientData2;
    _fileBrowser = NULL;
    _parentWidget = parent;
    _directory = NULL;
    _defDirSearchProc = NULL;
}

SelectFileCmd::~SelectFileCmd()
{
    free(_ok_label);
    free(_title);
    if (_directory) {
        XmStringFree(_directory);
    }
}
void SelectFileCmd::doit()
{
    // Create a FileSelectionBox widget
    
    Arg args[2];
    int n = 0;
    XmString title;

    // If the FSB already exists and is managed, raise it.

    if (_fileBrowser) {
        XtManageChild (_fileBrowser );
        XtPopup (XtParent(_fileBrowser), XtGrabNone);
	XRaiseWindow(XtDisplay(_fileBrowser), XtWindow(XtParent(_fileBrowser)));
	forceUpdate(_fileBrowser);
	return;
    }

    _DtTurnOnHourGlass(_parentWidget);

    if (_clientData2) {
	XtSetArg(args[n], XmNuserData, _clientData2); n++;
    }
    if (_dircallback && _directory) {
		XtSetArg(args[n], XmNdirectory, _directory); n++;
    }

    _fileBrowser =
    	XmCreateFileSelectionDialog(_parentWidget, (char *) name(), args, n );       
    // Save the old routine for later use
    if (_dircallback) {
	XtVaGetValues(_fileBrowser, 
		XmNdirSearchProc, &_defDirSearchProc, 
		NULL);
	// I know we already set the directory on creation but
	// setting the directory forces the FSB to call the _dircallback
	// proc. Without it, it doesnt get called, so must do this.
	XtVaSetValues(_fileBrowser, 
		XmNdirectory, _directory,
		XmNdirSearchProc, _dircallback, NULL);
    }

    // Set the title right...
    title = XmStringCreateLocalized(_title);
    XmString ok_str = XmStringCreateLocalized(_ok_label);
    XtVaSetValues(_fileBrowser,
	      XmNdialogTitle, title,
	      XmNokLabelString, ok_str,
	      NULL);
    XmStringFree(title);
    XmStringFree(ok_str);

    // add help callback for the file selection box
    XtAddCallback(_fileBrowser, XmNhelpCallback, HelpCB, DTMAILHELPFSB);
	
    // Set up the callback to be called when the user chooses a file
	    
    XtAddCallback ( _fileBrowser, 
    	XmNokCallback,
     	&SelectFileCmd::fileSelectedCallback, 
     	(XtPointer) this );
    XtAddCallback ( _fileBrowser,
        XmNcancelCallback,
        &SelectFileCmd::cancelCallback,
        (XtPointer) this );

    _DtTurnOffHourGlass(_parentWidget);
    
    // Display the dialog
    
    // If theres a callback then that means there are extra widgets added to
    // the FSB so the calling object has to manage after those widgets are
    // created. Otherwise they wont show.
    if (!_dircallback)
    	XtManageChild ( _fileBrowser );

    // Raise it, because it might be buried.
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
	
	if ( status && name != NULL && *name != '\0')
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
