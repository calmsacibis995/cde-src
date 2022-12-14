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
#pragma ident "@(#)UndoCmd.C	1.6 03/03/95"
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
// UndoCmd.C: An interface to undoing the last command
//////////////////////////////////////////////////////////
#include "UndoCmd.h"

#define NULL  0

// Declare a global object: theUndoCmd

UndoCmd *theUndoCmd = new UndoCmd ( "Undo", "Undo" ); 

UndoCmd::UndoCmd ( char *name, char *label ) : NoUndoCmd ( name, label, 0 )
{
    // Empty
}

void UndoCmd::doit()
{
    // If there is a current command, undo it
    
    if ( _lastCmd != NULL )
    {
	// Undo the previous command
	
	_lastCmd->undo();
	
	_lastCmd = NULL; // Make sure we can't undo twice
    }
}
