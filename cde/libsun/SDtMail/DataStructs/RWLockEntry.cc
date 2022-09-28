/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm RWLockEntry Class.

#pragma ident "@(#)RWLockEntry.cc	1.2 96/11/21 SMI"

// Include Files
#include <DataStructs/RWLockEntry.hh>

SdmRWLockEntry::SdmRWLockEntry(rwlock_t& readWriteLock, const SdmBoolean readOnly)
    : _Locked(Sdm_False)
{
  _RWLock = &readWriteLock;
  _ReadOnly = readOnly;

  int rc = 0;
  
#if defined(USE_SOLARIS_THREADS)
  if (_ReadOnly) {
    rc = rw_rdlock(_RWLock);
  } else {
    rc = rw_wrlock(_RWLock);
  }
#endif

  if (rc == 0)
    _Locked = Sdm_True;
}

SdmRWLockEntry::~SdmRWLockEntry()
{
#if defined(USE_SOLARIS_THREADS)
  if (_Locked)
      rw_unlock(_RWLock);
#endif
}

