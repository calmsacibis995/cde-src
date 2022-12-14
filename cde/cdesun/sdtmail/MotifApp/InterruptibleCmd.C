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
#pragma ident "@(#)InterruptibleCmd.C	1.9 04/24/96"
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


///////////////////////////////////////////////////////////////
// InterruptibleCmd.C: Abstract class that supports lengthy,
//                     user-interruptible activities
//////////////////////////////////////////////////////////////
#include "InterruptibleCmd.h"
#include "WorkingDialogManager.h"
#include "Application.h"
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <assert.h>
InterruptibleCmd::InterruptibleCmd ( char *name, char *label, int active ) :
                        NoUndoCmd ( name, label, active )
{
    _wpId        = NULL;   // There is no work procedure yet
    _callback    = NULL;	   // Callbacks are specified in execute()
    _clientData  = NULL;
    _done        = FALSE; 
}

InterruptibleCmd::~InterruptibleCmd()
{
    // Clean up by removing all callbacks
    
    if ( _wpId)    
	XtRemoveWorkProc ( _wpId );
}

void InterruptibleCmd::execute ( TaskDoneCallback callback, void *clientData )
{
    _callback   = callback;
    _clientData = clientData;
    execute();
}

void InterruptibleCmd::execute()
{
    char *name_str;
    char *label_str;
    
    name_str = (char *) name();
    label_str = (char *) getLabel();

    _done  = FALSE;  // Initialize flag
    
    // Call the Cmd execute function to handle the Undo, and other
    // general mechanisms supported by Cmd. Execute calls doit()
    
    Cmd::execute();  
    
    // If the task was completed in a single call,
    // don't bother to set up a work procedure. Just
    // give derived classes a chance to cleanup and
    // call the application's callback function
    
    if ( _done )
    {
	cleanup();
	
	if ( _callback )
	    ( *_callback )( this, FALSE, _clientData );
    }
    
    // If the task is not done, post a WorkingDialog and 
    // install a work procedure to continue the task 
    // as soon as possible.
    
    if ( !_done )
    {
	theWorkingDialogManager->post (label_str,
				       "Fetching" , 
				       (void *) this,
				       NULL, 
				       &InterruptibleCmd::interruptCallback );
	
	_wpId = XtAppAddWorkProc ( theApplication->appContext(), 
				  &InterruptibleCmd::workProcCallback,
				  (XtPointer) this );
    }
}

Boolean InterruptibleCmd::workProcCallback ( XtPointer clientData )
{
    InterruptibleCmd *obj = (InterruptibleCmd *) clientData;
    
    // The work procedure just returns the value returned by the
    // workProc member function.
    
    return ( obj->workProc() );
}

Boolean InterruptibleCmd::workProc()
{
    doit();
    
    // If the task has been completed, hide the dialog,
    // give the derived class a chance to clean up, and notify
    // the application that instantiated this object.
    
    if ( _done )
    {
	theWorkingDialogManager->unpost();
	cleanup();
	
	if ( _callback )
	    ( *_callback )( this, FALSE, _clientData );
    }
    
    return _done;
}

void InterruptibleCmd::cleanup()
{
    // Empty
}

void InterruptibleCmd::interruptCallback ( void * clientData )
{
    InterruptibleCmd *obj = ( InterruptibleCmd * ) clientData;
    
    // Just set the _interrupt flag to TRUE. The workProc() 
    // function will notice the next time it is called
    
    obj->interrupt();
}

void InterruptibleCmd::interrupt()
{
    // Remove the work procedure
    
    XtRemoveWorkProc ( _wpId );
    
    // Remove the working dialog and give derived 
    // classes a chance to clean up 
    
    theWorkingDialogManager->unpost();
    cleanup();
    
    // Notify the application that the task was interrupted
    
    if ( _callback )
	( *_callback )( this, TRUE, _clientData);
}

void InterruptibleCmd::updateMessage ( char * msg )
{
    theWorkingDialogManager->updateMessage ( msg );   
}
