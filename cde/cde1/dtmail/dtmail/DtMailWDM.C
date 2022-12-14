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
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "10/10/94 @(#)DtMailWDM.C	1.6"
#else
static const char * __sccs_FILE = "10/10/94 @(#)DtMailWDM.C	1.6";
#endif


///////////////////////////////////////////////////////////
// DtMailWDM.C
//////////////////////////////////////////////////////////
#include "DtMailWDM.hh"
#include "Application.h"
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include "BusyPixmap.h"
#include <assert.h>

DtMailWDM *theDtMailWDM =
        new DtMailWDM ( "DtMailWDM" );

DtMailWDM::DtMailWDM ( char   *name ) 
         : WorkingDialogManager ( name )
{
    _text = NULL;
}


Widget 
DtMailWDM::post (char *title,
		 char          *text, 
		 void          *clientData,
		 DialogCallback ok,
		 DialogCallback cancel,
		 DialogCallback help )
{
    // The the dialog already exists, and is currently in use,
    // just return this dialog. The DtMailWDM
    // only supports one dialog.
    
    if ( _w && XtIsManaged ( _w ) )
	return _w;
    
    // Pass the message on to the base class
    
    DialogManager::post (title, text, clientData, ok, cancel, help );
    
    forceUpdate( _w );
    return _w;
}

Widget 
DtMailWDM::post (char *title,
		 char          *text, 
		 Widget wid,
		 void          *clientData,
		 DialogCallback ok,
		 DialogCallback cancel,
		 DialogCallback help )
{
    // The the dialog already exists, and is currently in use,
    // just return this dialog. The DtMailWDM
    // only supports one dialog.
    
    if ( _w && XtIsManaged ( _w ) )
	return _w;
    
    // Pass the message on to the base class
    
    DialogManager::post (title, text, wid, clientData, ok, cancel, help );
    
    forceUpdate( _w );
    return _w;
}


void
DtMailWDM::updateAnimation()
{
    if (_w) {
	XtVaSetValues ( _w, 
//	    XmNsymbolPixmap, _busyPixmaps->next(),
	    NULL );
	forceUpdate( _w );
    }
}
    
void
DtMailWDM::updateDialog( 
    char *text
)
{
    
    if ( _w )
    {
    
	// Just change the string displayed in the dialog
    
	XmString xmstr = XmStringCreateLocalized ( text ); 

	// Update the pixmap too...
	XtVaSetValues ( _w, 
//		    XmNsymbolPixmap, _busyPixmaps->next(),
		    XmNmessageString, xmstr, 
		    NULL );
	XmStringFree ( xmstr );

    }

    forceUpdate( _w );
}
