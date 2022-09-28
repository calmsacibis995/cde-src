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
#pragma ident "@(#)CmdList.h	1.19 04/15/97"
#endif

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

////////////////////////////////////////////////////////////
// CmdList.h: Maintain a list of Cmd objects
////////////////////////////////////////////////////////////

#include <X11/Intrinsic.h>
#include "Cmd.h"

typedef void (*MenuCompleteCallback) (void *);

class CmdList : public Cmd {
    
private:
    
  Cmd **_contents;    // The list of objects
  int   _numElements; // Current size of list
  Widget _pane;
  Widget _cascade;
  void * _data;
  void * _more_data;

  // _menu_pending flag is used to tell whether this cmdlist
  // is being used by a work proc to build a menu.  Once the
  // work proc executes, it is set to FALSE.
  Boolean _menu_pending;

  // _destroyed flag is set in CmdList::deleteMe() if _menu_pending
  // is TRUE so that the work proc doesn't get stuck referencing
  // a deleted object.
  Boolean _destroyed;

  // Users of this class can register a function to call if
  // they have things that need to be done after the cascade
  // button and pulldown menu pane have been created for this
  // cmdlist.  Of course this only makes sense if this CmdList
  // object is being used to create a pulldown menu.  The reason
  // this callback is needed is that MenuBar::createPulldown uses
  // an Xt work proc to create the widgets that correspond to each
  // of the commands in this CmdList object.  This means that
  // whoever called MenuBar::addCommands cannot access these widgets
  // because they will probably not be created yet (the nature of
  // Xt work procs).
  MenuCompleteCallback _callback;

  // The CmdList destructor is being declared as a private method because
  // of its use with deferred menu creation.  Xt work procs are used
  // to create menus that reference CmdList objects.  Making the destructor
  // private will prevent other objects from explicitly deleting it.
  // The only way to delete a CmdList object is to use the deleteMe
  // method.
  virtual ~CmdList();           // Destroys list, but not objects in list

  virtual void doit();  
  virtual void undoit();  

public:
 
  CmdList();    
  CmdList(char *, char *);  // Construct an empty list


  void add ( Cmd * );		// Add a single Cmd object to list
  void insert ( Cmd * );	// Insert a single Cmd object to list
  void collInsert ( Cmd * );	// Insert a single Cmd object to list
  void remove ( int );	        // Remove command from list by index
  void deleteMe ();

  Boolean isDestroyed() { return _destroyed; }
  void setDestroyed(Boolean val) { _destroyed = val; }
  Boolean isPending() { return _menu_pending; }
  void setPending(Boolean val) { _menu_pending = val; }
  void setPaneWidget(Widget pane) { _pane = pane; }
  Widget getPaneWidget(void) { return _pane; }
  void setCascadeWidget(Widget cascade) { _cascade = cascade; }
  Widget getCascadeWidget(void) { return _cascade; }
  void addMenuCompleteCallback(MenuCompleteCallback, void *);
  void menuComplete();

  void setClientData(void * data) { _data = data; }
  void *getClientData(void) { return _data; }

  Cmd **contents() { return _contents; } // Return the list
  int size() { return _numElements; }    // Return list size
  Cmd *operator[]( int );                // Return an element of the list
  virtual const char *const className () { return "CmdList"; }
};

#endif
