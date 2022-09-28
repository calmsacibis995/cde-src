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

/////////////////////////////////////////////////////////
// Cmd.h: A base class for all command objects
/////////////////////////////////////////////////////////
#ifndef CMD_H
#define CMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Cmd.h	1.11 11/27/96"
#endif

#include <X11/Intrinsic.h>

class CmdList;
class CmdInterface;

class Cmd {
  friend CmdInterface;
    
private:
  void            revert();   // Reverts object to previous state
  int            _active;     // Is this command currently active?
  int            _previouslyActive; // Previous value of _active
  char          *_name;             // Name of this Cmd
  char          *_label;      // Label for the widget associated with Cmd.

protected:
    
  int           _hasUndo;    // True if this object supports undo
  static Cmd   *_lastCmd;    // Pointer to last Cmd executed
  CmdInterface **_ci;            
  int            _numInterfaces;
  Widget         _invoker;    // Widget invoking this command

  virtual void doit()   = 0;  // Specific actions must be defined
  virtual void undoit() = 0;  // by derived classes

public:
    
  Cmd (char *, char *,  int); // Protected constructor
    
  virtual ~Cmd ();            // Destructor

  // public interface for executing and undoing commands
  virtual void execute();  
  void    undo();

  void    activate();   // Activate this object
  void    deactivate(); // Deactivate this object
  char    *getLabel() { return _label; }  
    
  // Register an UIComponent used to execute this command
  void    registerInterface ( CmdInterface * );

  // Access functions 
  int active () { return _active; }
  int hasUndo() { return _hasUndo; }
  const char *name () { return _name; }
  virtual const char *const className () { return "Cmd"; }
};
#endif
