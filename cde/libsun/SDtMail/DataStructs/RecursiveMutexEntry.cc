/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm RecursiveMutexEntry Class.

#pragma ident "@(#)RecursiveMutexEntry.cc	1.5 96/11/13 SMI"

// Include Files
#include <DataStructs/RecursiveMutexEntry.hh>
#include <SDtMail/Sdtmail.hh>

SdmRecursiveMutexEntry::SdmRecursiveMutexEntry(SdmRecursiveMutexEntry::Data &data) : _Data(data)
{
#if defined(USE_SOLARIS_THREADS)
  mutex_lock(&_Data._Monitor);

  if (_Data._Testing) cout << "thread " << thr_self() << " ";

  if (_Data._Busy != 0) {
    if (_Data._Owner != thr_self()) {
      if (_Data._Testing) {
        cout << "Recursive mutex: concurrent thread access attempted" << endl;
        cout << "waiting for lock - held by thread " << _Data._Owner << endl;
      }
      do {
        cond_wait(&_Data._NotBusy, &_Data._Monitor);
      } while (_Data._Busy != 0);
      if (_Data._Testing)
        cout << "thread " << thr_self() << " now has lock" << endl;
    }
    else {
      if (_Data._Testing)
        cout << "proceeds (owner) increments busy count == " <<
          _Data._Busy + 1 << endl;
    }
  }
  else {
    if (_Data._Testing) cout << "gains lock (no contenders)" << endl;
  }

  _Data._Owner = thr_self();
  ++_Data._Busy;
  mutex_unlock(&_Data._Monitor);
#endif
}

SdmRecursiveMutexEntry::~SdmRecursiveMutexEntry()
{
#if defined(USE_SOLARIS_THREADS)
  unsigned int ownershipCount;

  mutex_lock(&_Data._Monitor);
  ownershipCount = --_Data._Busy;
  if (_Data._Testing) {
    if (_Data._Busy)
      cout << "thread " << thr_self() << " decrements busy count == " <<
        _Data._Busy << endl;
    else
      cout << "thread " << thr_self() << " gives up lock" << endl;
  }
  
  mutex_unlock(&_Data._Monitor);

  if (ownershipCount == 0)
    cond_signal(&_Data._NotBusy);
#endif
}
