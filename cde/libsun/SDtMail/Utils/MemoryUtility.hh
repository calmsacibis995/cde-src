/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Externalized Memory Manager.

#ifndef _SDM_MEMOU_H
#define _SDM_MEMOU_H

#pragma ident "@(#)MemoryUtility.hh	1.28 96/05/13 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>

class SdmMemoryUtility {

public:
  SdmMemoryUtility();
  ~SdmMemoryUtility();

  enum SdmDataT{VCHAR, VINT};

  static SdmErrorCode VAlloc(void* dPtr, const int dTyp, const int size);
  static SdmErrorCode VDlloc(void* dPtr, const int dTyp);

};

#endif
