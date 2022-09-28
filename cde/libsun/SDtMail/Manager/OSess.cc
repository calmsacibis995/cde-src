/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the O Session Class.

#pragma ident "@(#)OSess.cc	1.49 97/05/09 SMI"

// Include Files.
#include <Manager/OSess.hh>
#include <DataStore/OutgoingStore.hh>
#include <CtrlObjs/TickDisp.hh>
#include <CtrlObjs/RepSWER.hh>
#include <TranObjs/Request.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <Utils/TnfProbe.hh>



SdmOutgoingSession::SdmOutgoingSession(SdmError& err, SdmConnection& connection)
  :SdmSession(err, Sdm_CL_OutSession, connection, Sdm_ST_OutSession, Sdm_False) 
  /* _SubmitRequestCollector(NULL) */
{
  __SdmPRIM_REGISTERENTITY(this, NULL);
  __SdmPRIM_REGISTERCOLLECTOR(this, GetRequestQueue(), GetReplyQueue());

/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//
  _SubmitRequestCollector = new SdmRequestQueue;

  // create dispatcher for the SubmitRequest server.
  _SubmitRequestDisp = NULL;
  _SubmitRequestDisp = new SdmTickleDispatcher(err, 
                               _SubmitRequestCollector->GetPrimarySemaVar(), 8);

  _SubmitRequestDisp->AddTask(SdmOutgoingSession::SR_ProcessSubmitRequest, (SdmPrim *) this, NULL);
*/
}

SdmOutgoingSession::~SdmOutgoingSession()
{
  SdmError err;
  
  if (_IsStarted) {
    ShutDown(err);
  }

/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//
  delete _SubmitRequestCollector;
*/

  __SdmPRIM_UNREGISTERCOLLECTOR;
  __SdmPRIM_UNREGISTERENTITY;
}

SdmErrorCode
SdmOutgoingSession::SdmMessageStoreFactory(SdmError& err, SdmMessageStore *&r_store)
{
  err = Sdm_EC_Success;
   __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
   
  SdmMessageStore *t_store = new SdmOutgoingStore(err, *this);
  if (t_store == NULL) {
    err = Sdm_EC_Fail;
    return err;
  }
  if (AddMessageStore(err, t_store)) {
    delete t_store;
    return err;
  }
  
  r_store = t_store;
  return err;
}

SdmErrorCode 
SdmOutgoingSession::CancelPendingOperations(SdmError &err)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)

  // call inherited method.  this will call Cancel on all the
  // outgoing stores in this session.
  if (SdmSession::CancelPendingOperations(err)) {
    return err;
  }

/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//
  
  // Empty out submit requeset queue.  Do this after we cancel the
  // pending operations in the stores because we need to make sure the
  // request queue is empty before we clear out the submit queue.
  SdmRequestL outstandingRequests;
  _SubmitRequestCollector->RemoveAllRequests(err, outstandingRequests);
  

  // now cancel each of the requests that was left on the queue.
  int count = outstandingRequests.ElementCount();
  for (int i=0; i<count; i++) {
    CancelRequest(outstandingRequests[i]);
    // note: don't delete the request.  it will get deleted by the
    // CancelRequest call.
  }  
*/
  
  return err;
}

SdmErrorCode
SdmOutgoingSession::StartUp(SdmError& err)
{
  err = Sdm_EC_Success;

  if (_IsStarted){
    err = Sdm_EC_SessionAlreadyStarted;
    return err;
  };
  
  if (SdmSession::StartUp(err)) {
    return err;
  };

/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//

  // start up dispatcher for the SubmitRequest server.    
  if (_SubmitRequestDisp->StartUp(err)) {
    _IsStarted = Sdm_False;    //  need to reset this because it was 
                               // set by SdmSession::Startup.
    return err;
  }
  _IsStarted = Sdm_True;
*/

  return err;
}


SdmErrorCode 
SdmOutgoingSession::ShutDown(SdmError &err)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  SdmError localError;
  
  // Call inherited method for shutdown.  Call this before
  // shutting down the request dispatcher because SdmSession::Shutdown
  // should result in the request queue being flushed.
  if (SdmSession::ShutDown(localError))
    err = localError;    // save error but continue.

/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//

  // shut down the threads monitoring the submit request queue.
  if (_SubmitRequestDisp->ShutDown(localError)) {
    err = localError;   // save error but continue.
  }

  // clear out all requests from Submit request queue.
  // there actually should not be any requests left becuase the
  // inherited ShutDown calls CancelPendingOperations which should
  // flush out this queue.
  _SubmitRequestCollector->ClearAndDestroyAllElements();
*/

  return err;
}



/*
// commment out following code.  use if we want to support separate
// request queue for handling Submit requests.
//
void
SdmOutgoingSession::SR_ProcessSubmitRequest(SdmPrim* objThis, void* callData)
{
  TNF_PROBE_0 (SdmSession_SR_ProcessSubmitRequest_start,
      "async SdmSession",
      "sunw%debug SdmSession::SR_ProcessSubmitRequest Called ");

  ((SdmOutgoingSession *) objThis)->ProcessSubmitRequest(callData);
}

void
SdmOutgoingSession::ProcessSubmitRequest(void* callData)
{
  // The difference between this and ProcessRequest is we know
  // that all request are coming from the Submit Request queue.
  // Therefore, we don't need to check which queue to pull the
  // request from.

  TNF_PROBE_0 (SdmSession_ProcessSubmitRequest_start,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessSubmitRequest Called ");

  TNF_PROBE_1 (SdmSession_ProcessSubmitRequest_thread_info,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessSubmitRequest: request processed on thread: ",
      tnf_long, thread, thr_self());
  
 
  SdmRequest* request = _SubmitRequestCollector->GetNextLogicalRequest();
  
  // it is valid to have a NULL request.  The request queue could have been
  // emptied from and CancelPendingOperations but the semaphore still have
  // a count reflecting the posting of the request.
  if (request == NULL) {
    return;
  }

  SdmPrim* obj = request->GetClientObject();

  assert(obj != NULL);	// we better have a client object in the request!!

  obj->InCallDispatchPoint(request);
}

// Post Submit Request puts request on the Submit Request queue.
//
SdmErrorCode
SdmOutgoingSession::PostSubmitRequest(SdmError& err, SdmPrim* caller, SdmRequest *request)
{
  err = Sdm_EC_Success;

  TNF_PROBE_0 (SdmSession_PostSubmitRequest_start,
      "async SdmSession",
      "sunw%debug SdmSession::PostSubmitRequest Called ");
  
  // We need to be started and attached in order to post a request!!
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  if (!IsAttached()) {
    err = Sdm_EC_SessionNotAttached;
    return err;
  }
  
  assert(request != NULL);
  assert(caller != NULL);
  
  _SubmitRequestCollector->PostRequest(request);
  
  return err;
}
*/
