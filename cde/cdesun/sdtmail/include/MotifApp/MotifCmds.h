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


/////////////////////////////////////////////////////////
// MotifCmds.h: Menu cmds
/////////////////////////////////////////////////////////
#ifndef MOTIFCMDS_H
#define MOTIFCMDS_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "0 96/11/27"
#endif

#include "Cmd.h"
#include "CmdInterface.h"
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>


// Elsewhere in MotifApp, we refer to instances of these classes
// via their className() method. We compare them to SeparatorCmd
// or ToggleButtonCmd.
// We want derived classes to also answer to the same className().
// So, we make className() a non-virtual method.

class SeparatorCmd : public Cmd {
public:
  virtual void doit() {}
  virtual void undoit() {}
  SeparatorCmd(char *name, int active) : Cmd(name, NULL, active) {}
  const char *const className () {return "SeparatorCmd";}
};

// Its critical that children of ToggleButtonCmd not have a 
// const className() method.
// Make className() a non-virtual method then.

class ToggleButtonCmd : public Cmd {
public:
  virtual void doit() {}
  virtual void undoit() {}
  ToggleButtonCmd( char *name, char *label, int active) : Cmd(name, label, active) {}
  Boolean getButtonState();
  void setButtonState(Boolean, Boolean);
    
  const char *const className () {return "ToggleButtonCmd";}
};
    
#endif
