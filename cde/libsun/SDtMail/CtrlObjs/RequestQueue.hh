/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Template Request Queue Class.

#ifndef _SDM_REQUESTQUEUE_H
#define _SDM_REQUESTQUEUE_H

#pragma ident "@(#)RequestQueue.hh	1.34 97/03/21 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <TranObjs/Request.hh>
#include <CtrlObjs/ThreadDefs.hh>
#include <SDtMail/DoubleLinkedList.hh>


class SdmRequestQueue : 
  public SdmLinkedListElement , public SdmDoubleLinkedList{

public:
  SdmRequestQueue();
  virtual ~SdmRequestQueue();
  SdmBoolean operator== (const SdmRequestQueue& queue)
  { return this == &queue ? Sdm_True : Sdm_False; }

public:
  sema_t* GetPrimarySemaVar();
  void PostRequest(const SdmRequest* reqst);
  SdmRequest* GetNextLogicalRequest();
  void RemoveAllRequests(SdmError& error, SdmRequestL& r_requests);
  void ClearAllRequests(SdmError& error);


private:
  mutex_t _QueueAccessLock; 
  sema_t _ReqstDeposited;
};

#endif
