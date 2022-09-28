/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm MutexEntry Class.

#ifndef _SDM_MUTEXENTRY_H
#define _SDM_MUTEXENTRY_H

#pragma ident "@(#)MutexEntry.hh	1.21 97/03/21 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/ThreadDefs.hh>

class SdmMutexEntry {

public:
  SdmMutexEntry(mutex_t& mutexToLock) : 
    _Mutex(&mutexToLock)
  {
#if defined(USE_SOLARIS_THREADS)
  mutex_lock(_Mutex);
#endif
  }
  
  virtual ~SdmMutexEntry()
  {
#if defined(USE_SOLARIS_THREADS)
      mutex_unlock(_Mutex);
#endif
  }
  
private:
  mutex_t*   _Mutex;
};

#endif
