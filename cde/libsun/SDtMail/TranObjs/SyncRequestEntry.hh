/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm SyncRequestEntry Class.

#ifndef _SDM_SYNCREQUEST_H
#define _SDM_SYNCREQUEST_H

#pragma ident "@(#)SyncRequestEntry.hh	1.10 96/06/11 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/ThreadDefs.hh>


class SdmPrim;
class SdmError;

class SdmSyncRequestEntry {

public:
  SdmSyncRequestEntry(SdmError& err, SdmPrim& caller);
  virtual ~SdmSyncRequestEntry();

private:
  SdmErrorCode   PostSynchronousRequest(SdmError &err);

  SdmBoolean	_IsRequestPosted;
  sema_t*       _ServerLock;
  sema_t	_CallerLock;
  SdmPrim*	_Caller;
};

#endif
