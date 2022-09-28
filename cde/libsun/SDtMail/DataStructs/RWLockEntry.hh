/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm RWLockEntry Class.

#ifndef _SDM_RWLOCKENTRY_H
#define _SDM_RWLOCKENTRY_H

#pragma ident "@(#)RWLockEntry.hh	1.1 96/05/29 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <synch.h>

class SdmRWLockEntry {

public:
  SdmRWLockEntry(rwlock_t& mutexToLock, const SdmBoolean readOnly);
  virtual ~SdmRWLockEntry();

private:
  SdmBoolean _ReadOnly;
  SdmBoolean _Locked;
  rwlock_t*   _RWLock;
};

#endif
