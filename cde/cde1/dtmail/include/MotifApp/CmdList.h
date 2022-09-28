/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
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

#ifndef CMDLIST_H
#define CMDLIST_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)CmdList.h	1.16 05/08/96"
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


////////////////////////////////////////////////////////////
// CmdList.h: Maintain a list of Cmd objects
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// MODIFIED TO INHERIT FROM CMD - not described in Book
///////////////////////////////////////////////////////////

#include <X11/Intrinsic.h>
#include "Cmd.h"

class CmdList : public Cmd {
    
  private:
    
    Cmd **_contents;    // The list of objects
    int   _numElements; // Current size of list
    Widget _pane;
    Widget _cascade;
    void * _data;

    virtual void doit();  
    virtual void undoit();  

  public:
 
    CmdList();    
    CmdList(char *, char *);           // Construct an empty list
    virtual ~CmdList();  // Destroys list, but not objects in list
    
    void add ( Cmd * );		// Add a single Cmd object to list
    void insert ( Cmd * );	// Insert a single Cmd object to list
    void collInsert ( Cmd * );	// Insert a single Cmd object to list
    void remove ( int );	// Remove command from list by index

    void setPaneWidget(Widget pane) { _pane = pane; }
    Widget getPaneWidget(void) { return _pane; }
    void setCascadeWidget(Widget cascade) { _cascade = cascade; }
    Widget getCascadeWidget(void) { return _cascade; }

    void setClientData(void * data) { _data = data; }
    void *getClientData(void) { return _data; }

    Cmd **contents() { return _contents; } // Return the list
    int size() { return _numElements; }    // Return list size
    Cmd *operator[]( int );            // Return an element of the list
    virtual const char *const className () { return "CmdList"; }
};


#endif
