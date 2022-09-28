/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm MutexEntry Class.

#ifndef _SDM_RECURSIVEMUTEXENTRY_H
#define _SDM_RECURSIVEMUTEXENTRY_H

#pragma ident "@(#)RecursiveMutexEntry.hh	1.4 96/06/04 SMI"

// First, to preserve familiar tradition, the all important welcoming diary
// entry:

// It is very true that from time to time I prefer England to the USA.  Bah,
// yeah, it's the fish 'n chips, the beer, the green countryside, the humor
// and above all mes petites amis with whom I visit the pubs and eat Indian
// curries.  C'est magnifique, JSC MV 5/30/96

// Include Files
#include <assert.h>
#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/ThreadDefs.hh>

class SdmRecursiveMutexEntry {
public:
  class Data {
    friend class SdmRecursiveMutexEntry;
  public:
    Data(SdmBoolean testing = Sdm_False) : _Testing(testing), _Busy(0) {
      int rc;
      rc = mutex_init(&_Monitor, USYNC_THREAD, NULL);
      assert (rc == 0);
      rc = cond_init(&_NotBusy, USYNC_THREAD, NULL);
      assert (rc == 0);
    }
    ~Data() {
      mutex_destroy(&_Monitor);
      cond_destroy(&_NotBusy);
    }
  private:
    mutex_t _Monitor;
    thread_t _Owner;
    unsigned int _Busy;
    cond_t _NotBusy;
    SdmBoolean _Testing;
  };

  SdmRecursiveMutexEntry(Data &data);
  virtual ~SdmRecursiveMutexEntry();

private:
  Data &_Data;
};

#endif /* _SDM_RECURSIVEMUTEXENTRY_H */
