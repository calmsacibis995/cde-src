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
#pragma ident "@(#)Cmd.C	1.17 12/05/96"
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

///////////////////////////////////////////////////////
// Cmd.C
///////////////////////////////////////////////////////
#include <stdlib.h>
#include "Cmd.h"
#include "CmdList.h"
#include "CmdInterface.h"

// External object that reverses the most recent Cmd when executed
extern Cmd *theUndoCmd; 

Cmd *Cmd::_lastCmd = NULL;  // Pointer to most recent Cmd

Cmd::Cmd (char *name, char *label, int active)
{
  _previouslyActive  = 0;
  if (name) {
    _name = strdup(name);
  } else {
    _name = strdup("Cmd");
  }

  _active = active;
  _numInterfaces     = 0;    
  _ci                = NULL;
  _hasUndo           = TRUE;
  _invoker           = NULL;

  if (label) {
    _label = strdup(label);
  } else {
    _label = strdup(_name);
  }
}

Cmd::~Cmd()
{
  free (_label);
  free (_name);
  if (_ci) {
    for (int i = 0; i < _numInterfaces; i++)
      delete _ci[i];
    delete _ci;
  }
}

void Cmd::registerInterface (CmdInterface *ci)
{
  // Make a new list, large enough for the new object
  CmdInterface **newList = new CmdInterface*[_numInterfaces + 1];

  // Copy the contents of the previous list to the new list
  for(int i = 0; i < _numInterfaces; i++)
    newList[i] = _ci[i];

  // Free the old list
  if (_ci)
    delete []_ci;

  // Install the new list
  _ci =  newList;

  // Add the object to the list and update the list size.
  _ci[_numInterfaces] = ci;
  _numInterfaces++;

  if (ci)
    if (_active)
      ci->activate();
    else
      ci->deactivate();      
}

void Cmd::activate()
{
  // Activate the associated interfaces
  for (int i = 0; i < _numInterfaces; i++)
    _ci[i]->activate ();
    
  // Save the current value of active before setting the new state
  _previouslyActive = _active;
  _active = TRUE;
}

void Cmd::deactivate()
{
  // Deactivate the associated interfaces
  for (int i = 0; i < _numInterfaces; i++)
    _ci[i]->deactivate ();
    
  // Save the current value of active before setting the new state
  _previouslyActive = _active;
  _active = FALSE;
}

void Cmd::revert()
{
  // Activate or deactivate, as necessary, to return to the previous state
  if (_previouslyActive)
    activate();
  else
    deactivate();
}

void Cmd::execute()
{
  // If a command is inactive, it cannot be executed
  if (!_active)
    return;
    
  // Activate or deactivate the global theUndoCmd, and remember the last
  // command, as needed
  if (_hasUndo)
    {
      Cmd::_lastCmd = this;
      theUndoCmd->activate();
    }
  else  
    {      
      Cmd::_lastCmd = NULL;
      theUndoCmd->deactivate();
    }
    
  // Call the derived class's doit member function to 
  // perform the action represented by this object
  doit();
}

void Cmd::undo()
{
  // Call the derived class's undoit() member function.
    
  undoit();
    
  // The system only supports one level of undo, and this is it,
  // so deactivate the undo facility.
    
  theUndoCmd->deactivate();
    
  // Reverse the effects of the execute() member function by 
  // reverting all dependent objects to their previous state
}
