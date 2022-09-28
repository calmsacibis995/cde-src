/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Template Request Queue Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RequestQueue.cc	1.39 97/03/21 SMI"
#endif

// Include Files
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/RepSWER.hh>
#include <DataStructs/MutexEntry.hh>

SdmRequestQueue::SdmRequestQueue()
  :  SdmLinkedListElement(this)
{
  // Fix?? Should we change the following to return an error instead
  // or make an assertion?
  int rc;
  rc =  mutex_init(&_QueueAccessLock, USYNC_THREAD, NULL);
  assert (rc == 0);
#if defined(USE_SOLARIS_THREADS)
  rc = sema_init(&_ReqstDeposited, 0, USYNC_THREAD, 0);
  assert (rc == 0);
#endif
}
 
SdmRequestQueue::~SdmRequestQueue()
{
  mutex_destroy(&_QueueAccessLock);
#if defined(USE_SOLARIS_THREADS)
  sema_destroy(&_ReqstDeposited);
#endif
}

sema_t*
SdmRequestQueue::GetPrimarySemaVar()
{
  return &_ReqstDeposited;
}

void
SdmRequestQueue::PostRequest(const SdmRequest* reqst) 
{ 
  SdmRequest *request = (SdmRequest*) reqst;
  
  // we need to get lock before we update the queue.
  // note we create the entry within a narrower scope so that
  // lock is held for a minimal amount of time.
  {
	  SdmMutexEntry entry(_QueueAccessLock);  
      AddElementToList(request);    // add to end of queue.
  }
  
  // wake up any threads who are waiting for items in the queue.
  sema_post(&_ReqstDeposited);
} 

SdmRequest*
SdmRequestQueue::GetNextLogicalRequest()
{
  SdmRequest* reqst = NULL;

  // we need to get lock before we read from the queue.
  // note we create the entry within a narrower scope so that
  // lock is held for a minimal amount of time.
  {
    SdmMutexEntry entry(_QueueAccessLock);
    if (ElementCount() > 0) {
        reqst = (SdmRequest *) operator[](0);        // remove from front of queue.
        RemoveElementFromList(0);
    }
  }
  
  return reqst;
}

void 
SdmRequestQueue::RemoveAllRequests(SdmError& error, SdmRequestL& r_requests)
{
  SdmMutexEntry entry(_QueueAccessLock);
  SdmRequest *request;
  
  // add requests in queue into the list.
  for (int i=0; i< ElementCount(); i++)
  {
    request = (SdmRequest *) operator[](i);
    r_requests.AddElementToList(request);
  }

  ClearAllElements();    // remove requests from queue.
}

void 
SdmRequestQueue::ClearAllRequests(SdmError& error)
{
  SdmMutexEntry entry(_QueueAccessLock);
  ClearAndDestroyAllElements();    // remove requests from queue.
}

