/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Control Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Ctrl.cc	1.25 96/05/13 SMI"
#endif

// Include Files.
#include <CtrlObjs/Ctrl.hh>

SdmController::SdmController(const SdmClassId classId, SdmError& err)
  :SdmPrim(classId)
{
  err = Sdm_EC_Success;
}

SdmController::~SdmController()
{
}

