/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Control Class.

#ifndef _SDM_CTRL_H
#define _SDM_CTRL_H

#pragma ident "@(#)Ctrl.hh	1.29 96/05/13 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>

class SdmController : public SdmPrim {

public:
  SdmController(const SdmClassId classId, SdmError& err);
  virtual ~SdmController();

private:
	
};

#endif
