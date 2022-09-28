/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm SyncRequestEntry Class.

#pragma ident "@(#)SyncRequestEntry.cc	1.19 97/03/14 SMI"

// Include Files
#include <iostream.h>
#include <stdio.h>
#include <TranObjs/SyncRequestEntry.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/Data.hh>
#include <TranObjs/ArgumentList.hh>
#include <Utils/TnfProbe.hh>


SdmSyncRequestEntry::SdmSyncRequestEntry(SdmError& err, SdmPrim& caller)
    : _ServerLock(NULL), _Caller(&caller),
      _IsRequestPosted(Sdm_False)
{
  err = Sdm_EC_Success;
  
#if defined(USE_SOLARIS_THREADS)
  if (_Caller->_CollectorSession == NULL) {
    err = Sdm_EC_Fail;
    return;
  }

  if (sema_init(&_CallerLock, 0, USYNC_THREAD, 0)) {
    err = Sdm_EC_Fail;
    return;
  }

  _ServerLock = new sema_t;
  assert(_ServerLock != NULL);

  if (sema_init(_ServerLock, 0, USYNC_THREAD, 0)) {
    err = Sdm_EC_Fail;
    return;
  }

  // Debug ***
  TNF_PROBE_2 (SdmSyncRequestEntry_post_sync_request,
    "async SdmSyncRequestEntry",
    "sunw%debug SdmSyncRequestEntry::Post sync request with locks: ",
    tnf_long, callerLock, (int)&_CallerLock,
    tnf_long, serverLock, (int)_ServerLock);
  // Debug ***

  if (PostSynchronousRequest(err)) {
    sema_destroy(&_CallerLock);
    return;
  }

  // Debug ***
  TNF_PROBE_0 (SdmSyncRequestEntry_sleep_on_callerLock,
    "async SdmSyncRequestEntry",
    "sunw%debug SdmSyncRequestEntry: sleep on callerLock ");
  // Debug ***

  // wait until our request is being processed on the server.
  sema_wait(&_CallerLock);
  sema_destroy(&_CallerLock);

  // Debug ***
  TNF_PROBE_0 (SdmSyncRequestEntry_wakeup_from_callerLock,
    "async SdmSyncRequestEntry",
    "sunw%debug SdmSyncRequestEntry: wakeup from callerLock.");
  // Debug ***
  
#endif

}

SdmSyncRequestEntry::~SdmSyncRequestEntry()
{
#if defined(USE_SOLARIS_THREADS)
  if (_ServerLock != NULL) {
    if (_IsRequestPosted) {
      // wake up server in case the server is waiting on
      // the lock.  Don't delete the lock because it
      // gets deleted by the server after it wakes up.
      // cout << "SdmSyncRequestEntry: wakeup serverLock.\n";

      TNF_PROBE_0 (SdmSyncRequestEntry_wakeup_serverLock,
        "async SdmSyncRequestEntry",
        "sunw%debug SdmSyncRequestEntry: wakeup serverLock.");

      sema_post(_ServerLock);
    } else {
      // if no request was ever posted, then
      // we need to clean up the server lock.
      sema_destroy(_ServerLock);
      delete _ServerLock;
    }
  }
#endif
}

SdmErrorCode
SdmSyncRequestEntry::PostSynchronousRequest(SdmError &err)
{
  err = Sdm_EC_Success;
  
#if defined(USE_SOLARIS_THREADS)
  SdmArgumentList* args = new SdmArgumentList;

  args->PutArg(&_CallerLock);
  args->PutArg(_ServerLock);
  args->PutArg(&err);

  SdmRequest* reqst = new SdmRequest(args, SdmData::AOP_SyncOp, _Caller);  

  if (_Caller->_CollectorSession->PostRequest(err, _Caller, reqst)) {
    // need to clean up request and arguments;
    delete reqst;    
    delete args;
    return err;
  }
  _IsRequestPosted = Sdm_True;
#endif

  return err;
}
