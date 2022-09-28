/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Session Class.

#pragma ident "@(#)Session.cc	1.105 97/06/09 SMI"

// These constants are used to keep the ends of the pipe straight.
//
static const int        READ = 0;
static const int        WRITE = 1;
 

// Include Files.
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream.h>
#include <errno.h>
#include <stdlib.h>
#include <SDtMail/Session.hh>
#include <SDtMail/Data.hh>
#include <SDtMail/Connection.hh>
#include <DataStructs/MutexEntry.hh>
#include <CtrlObjs/TickDisp.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <CtrlObjs/Disp.hh>
#include <TranObjs/ArgumentList.hh>
#include <Utils/TnfProbe.hh>
#include <SDtMail/MessageStore.hh>
#include <CtrlObjs/TimeDisp.hh>
#include <SDtMail/SystemUtility.hh>


extern SdmErrorCode SdmXtAddSessionTimeOuts(SdmError &err, SdmSession &ses);
extern SdmErrorCode SdmXtRemoveSessionTimeOuts(SdmError &err, SdmSession &ses);


const time_t SdmSession::kMinMailCheckInterval = 15; // The minimum time is check mail every 15 seconds.
const time_t SdmSession::kMinAutoSaveInterval = 60;  // The minimum time is autosave every minute.
const time_t SdmSession::kMaxAutoSaveInterval = (30*60);  // The maximum time is autosave every 30 minutes.
const time_t SdmSession::kDefaultMailCheckInterval = 60;   // default; check for new mail every 60 seconds.
const time_t SdmSession::kDefaultAutoSaveInterval = (10*60); // default; autoSave every 10 minutes.
const time_t SdmSession::kDefaultInactivityInterval = 60;  // The default idle time before processing 
                                                        // autosave or mail check is 60 seconds.   
const time_t SdmSession::kMinInactivityInterval = 15;  // The mimimum idle time before processing 
                                                    // autosave or mail check is 15 seconds.   
const time_t SdmSession::kMaxInactivityInterval = 600; // The maximum idle time before processing 
                                                    // autosave or mail check is 10 minutes. 
const time_t SdmSession::kPingInterval = (20*60); // ping message stores every 20 minutes.  

const unsigned short SdmSession::SdmD_TimeOutDataSet_Size = 3;

SdmSession::SdmSession(SdmError& err, const SdmClassId classId, 
        SdmConnection& connection, SdmSessionType type,
        SdmBoolean startPollingTasks)
  :SdmPrim(classId), 
   _MasterRequestCollector(NULL), _MasterReplyCollector(NULL),
   _LocalServerList(), _ServerWiseQueueList(), _FrontEndEventQueue(NULL),
   _ServiceFunctionList(SdmD_ServiceFunctionSet_Size),
   _WriteFileDesc(-1), _SessionType(type),
   _TimeOutDataList(SdmD_TimeOutDataSet_Size),
   _DoPollingTasks(startPollingTasks)
{
  TNF_PROBE_0 (SdmSession_Constructor_start,
      "api SdmSession",
      "sunw%debug SdmSession_Constructor_start");

  err = Sdm_EC_Success;
  
  int rc;
  rc =   mutex_init(&_LocalServerDataLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  rc =   mutex_init(&_ServiceListLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  rc =   mutex_init(&_StartedMessgStoreListLock, USYNC_THREAD, NULL);
  assert (rc == 0);

  _ParentConnection = &connection;
  _MasterRequestCollector = new SdmRequestQueue;
  _MasterReplyCollector = new SdmReplyQueue;
  _MasterRequestDisp = NULL;
  _MasterReplyDisp = NULL;
  _MailCheckDisp = NULL;
  _AutoSaveDisp = NULL;
  _PingDisp = NULL;

  //----------------------------
  _MasterRequestDisp = new SdmTickleDispatcher(err, 
                               _MasterRequestCollector->GetPrimarySemaVar(), 1);
  _MasterReplyDisp = new SdmTickleDispatcher(err, 
                               _MasterReplyCollector->GetPrimarySemaVar(), 1);

  _MasterRequestDisp->AddTask(SdmSession::SR_ProcessRequest, (SdmPrim *) this, NULL);
  _MasterReplyDisp->AddTask(SdmSession::SR_ProcessReply, (SdmPrim *) this, NULL);
  //----------------------------

  // initialize service function list.
  // this should be done before starting up the timed dispatchers.
  for (int i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
    _ServiceFunctionList[i] = NULL;

  // initialize time out data list.  do this before calling SetupPollingTasks.
  for (i = 0; i < SdmD_TimeOutDataSet_Size; i++) {
     _TimeOutDataList[i] = NULL;
  }
  
  if (_DoPollingTasks)
    SetupPollingTasks(err);
  
  TNF_PROBE_0 (SdmSession_Constructor_end,
      "api SdmSession",
      "sunw%debug SdmSession_Constructor_end");
}


SdmSession::SdmSession(const SdmSession& copy) 
  :SdmPrim(Sdm_CL_Session)

{
  cout << "*** Error: SdmSession copy constructor called\n";
  assert(Sdm_False);
}

SdmSession& 
SdmSession::operator=(const SdmSession &rhs)
{
  cout << "*** Error: SdmSession assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmSession::~SdmSession()
{
  TNF_PROBE_0 (SdmSession_Destructor_start,
      "api SdmSession",
      "sunw%debug SdmSession_Destructor_start");

  // _ServiceFunctionList owned by caller.
  
  SdmError err;
  
  if (_IsStarted) {
    ShutDown(err);
  }
  
  // tell the parent that we're being destroyed.
  if (_ParentConnection) {
    _ParentConnection->HandleSessionDestruction(err, this);
  }

  mutex_destroy(&_LocalServerDataLock);
  mutex_destroy(&_ServiceListLock);
  
  delete _MasterRequestCollector;
  delete _MasterReplyCollector;

  delete _MasterRequestDisp;
  delete _MasterReplyDisp;
  delete _MailCheckDisp;
  delete _AutoSaveDisp;
  delete _PingDisp;

  _TimeOutDataList.ClearAndDestroyAllElements();

  TNF_PROBE_0 (SdmSession_Destructor_end,
      "api SdmSession",
      "sunw%debug SdmSession_Destructor_end");

}

SdmErrorCode
SdmSession::StartUp(SdmError& err)
{
  TNF_PROBE_0 (SdmSession_StartUp_start,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_start");

  err = Sdm_EC_Success;

  if (_IsStarted){
    err = Sdm_EC_SessionAlreadyStarted;
    TNF_PROBE_0 (SdmSession_StartUp_end,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_end");
    return err;
  }

  if (_MasterRequestDisp->StartUp(err))
  {
    TNF_PROBE_0 (SdmSession_StartUp_end,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_end");
    return err;
  }
    
  if (_MasterReplyDisp->StartUp(err))
  {
    TNF_PROBE_0 (SdmSession_StartUp_end,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_end");
    return err;
  }

  if (StartPollingTasks(err))
  {
    TNF_PROBE_0 (SdmSession_StartUp_end,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_end");
    return err;
  }


  {
    SdmMutexEntry entry(_ServiceListLock);  
    // initialize list of service functions.    
    _ServiceFunctionList.SetVectorSize(SdmD_ServiceFunctionSet_Size); 
    for (int i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
      _ServiceFunctionList[i] = NULL;
  }
	
  _IsStarted = Sdm_True;

  TNF_PROBE_0 (SdmSession_StartUp_end,
      "api SdmSession",
      "sunw%debug SdmSession_StartUp_end");

  return err;
}

SdmErrorCode
SdmSession::ShutDown(SdmError& err)
{
  TNF_PROBE_0 (SdmSession_ShutDown_start,
      "api SdmSession",
      "sunw%debug SdmSession_ShutDown_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  // First, shut down the message stores.
  SdmError localError;
  SdmLinkedListIterator dli (&_MessgStoreList);
  SdmMessageStore *ms_ptr;

  // What do we do if there are still requests on the queues?  
  // Answer: The ShutDown of the message store below will
  // call CancelPendingOperations on the store and flush
  // the request queue.
  
  // if there is an error in shutting down a message store,
  // save the error but continue shutting down the rest of 
  // the stores.
  while (ms_ptr = (SdmMessageStore *)(dli.NextListElement())) {
    if (ms_ptr->IsStarted()) {
      ms_ptr->ShutDown(localError);
      if (localError != Sdm_EC_Success) {
         err = localError;
         localError = Sdm_EC_Success;
      } 
    }			
  }
  _MessgStoreList.ClearAndDestroyAllElements();

  
  // shut down the threads monitoring the request and 
  // reply queues.
  if (ShutDownMasterRequestDispatcher(err))
  {
    TNF_PROBE_0 (SdmSession_ShutDown_end,
      "api SdmSession",
      "sunw%debug SdmSession_ShutDown_end");
    return err;
  }
    
  if (ShutDownMasterReplyDispatcher(err))
  {
    TNF_PROBE_0 (SdmSession_ShutDown_end,
      "api SdmSession",
      "sunw%debug SdmSession_ShutDown_end");
    return err;
  }

  // shutdown the dispatchers used to handle unsolicited requests.
  // do this after shutting down message stores.  
  if (StopPollingTasks(err)) 
  {
    TNF_PROBE_0 (SdmSession_ShutDown_end,
      "api SdmSession",
      "sunw%debug SdmSession_ShutDown_end");
    return err;
  }
		
  _MasterRequestCollector->ClearAllRequests(err);
  _MasterReplyCollector->ClearAllReplies(err);

  // Do we need to shutdown each of the local servers?? 
  // Answer: No.  this is down by the session subclasses
  // when it shuts down the message stores.  The shut down
  // of the message store should result in unregistering
  // the server from the session.  therefore, it is safe
  // to just clear any pointers left in our list.
  //
  _LocalServerList.ClearAllElements();
  _ServerWiseQueueList.ClearAndDestroyAllElements();
  _ServerWiseDispatcherList.ClearAndDestroyAllElements();

  {
    SdmMutexEntry entry(_ServiceListLock);  
    _ServiceFunctionList.ClearAndDestroyAllElements();
  }

  // if we are attached to the front end, then we
  // close the write file descriptor for the pipe.  
  // this should signal the session event handler to 
  // close the read file descriptor for the pipe and 
  // clean up the front end queue.  we DO NOT clean up
  // up the queue.
  ///
  if (IsAttached()) {
    close (_WriteFileDesc);
    _FrontEndEventQueue = NULL;
  }

  _IsStarted = Sdm_False;
  

  TNF_PROBE_0 (SdmSession_ShutDown_end,
      "api SdmSession",
      "sunw%debug SdmSession_ShutDown_end");

  return err;
}

SdmErrorCode
SdmSession::SetupPollingTasks(SdmError &err)
{
  // set up timed dispatchers to handle mail check and autosave.
  _MailCheckDisp = new SdmTimedDispatcher(err);
  _AutoSaveDisp = new SdmTimedDispatcher(err);
  _PingDisp = new SdmTimedDispatcher(err);
  
  if (err) return err;
  
  time_t mailCheck = GetInterval("retrieveinterval", Sdm_False, 
                  kDefaultMailCheckInterval, kMinMailCheckInterval);
                  
  time_t saveInterval = GetInterval("saveinterval", Sdm_True, 
                  kDefaultAutoSaveInterval, kMinAutoSaveInterval, kMaxAutoSaveInterval);
  
  _MailCheckDisp->SetFrequencyInSec(mailCheck);
  _AutoSaveDisp->SetFrequencyInSec(saveInterval);
  _PingDisp->SetFrequencyInSec(kPingInterval);

#if defined(USE_SOLARIS_THREADS)
  _MailCheckDisp->AddTask(SdmSession::SR_ProcessMailCheck, (SdmPrim *) this, NULL);
  _AutoSaveDisp->AddTask(SdmSession::SR_ProcessAutoSave, (SdmPrim *) this, NULL);
  _PingDisp->AddTask(SdmSession::SR_ProcessPing, (SdmPrim *) this, NULL);
#else
  // initialize timeout data list.
  for (int i = 0; i < SdmD_TimeOutDataSet_Size; i++) {
     SdmTimeOutInfo *info = new SdmTimeOutInfo;
     info->_appContext = NULL;
     info->_session = this;
     info->_event = (SdmPollEventType) i;
     info->_intervalInSecs = 0;
     info->_id = 0; 
     info->_isStarted = Sdm_False;
     _TimeOutDataList[i] = info;
  }
#endif

  return Sdm_EC_Success;
}

SdmErrorCode
SdmSession::ResetPollingTasks(SdmError &err)
{
  err = Sdm_EC_Success;
  
  if (StopPollingTasks(err)) 
      return err;

  if (StartPollingTasks(err)) 
    return err;
    
  return err;
}


SdmErrorCode
SdmSession::StartPollingTasks(SdmError &err)
{
  err = Sdm_EC_Success;
  if (!_DoPollingTasks) {
    return err;
  }  

  assert (_MailCheckDisp != NULL && _AutoSaveDisp != NULL && _PingDisp != NULL);
  
  // set the time intervals for the mail check and autosave dispatchers.
  time_t mailCheckInterval = GetInterval("retrieveinterval", Sdm_False, 
                  kDefaultMailCheckInterval, kMinMailCheckInterval);
  time_t saveInterval = GetInterval("saveinterval", Sdm_True, 
                  kDefaultAutoSaveInterval, kMinAutoSaveInterval, kMaxAutoSaveInterval);
                  
  _MailCheckDisp->SetFrequencyInSec(mailCheckInterval);
  _AutoSaveDisp->SetFrequencyInSec(saveInterval);
  _PingDisp->SetFrequencyInSec(kPingInterval);

  // start up dispatchers to handle unsolicited events.
  if (_MailCheckDisp->StartUp(err) || _AutoSaveDisp->StartUp(err) || _PingDisp->StartUp(err)) {
    TNF_PROBE_0 (SdmMessageStore_StartUp_end,
        "api SdmMessageStore",
        "sunw%end SdmMessageStore_StartUp_end");
    return err;
  }
  
#if !defined(USE_SOLARIS_THREADS)
  if (SdmXtAddSessionTimeOuts(err, *this)) {
    return err;
  }
#endif

  return err;
}

SdmErrorCode
SdmSession::StopPollingTasks(SdmError &err)
{
  err = Sdm_EC_Success;
  if (!_DoPollingTasks) {
    return err;
  }  

  assert (_MailCheckDisp != NULL && _AutoSaveDisp != NULL && _PingDisp != NULL);
  if (!_MailCheckDisp->IsStarted()) {
    return err;
  }
    
  // shutdown the dispatchers used to handle unsolicited requests.
  if (_MailCheckDisp->ShutDown(err) || _AutoSaveDisp->ShutDown(err) || _PingDisp->ShutDown(err)) {
    TNF_PROBE_0 (SdmSession_ShutDown_end,
      "api SdmSession",
      "sunw%end SdmSession_ShutDown_end");
    return err;
  }

#if !defined(USE_SOLARIS_THREADS)
  if (SdmXtRemoveSessionTimeOuts(err, *this)) {
    return err;
  }
#endif
  
  return err;
}

SdmErrorCode
SdmSession::RegisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmSession_RegisterServiceFunction_start,
      "api SdmSession",
      "sunw%debug SdmSession_RegisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  SdmServiceFunctionType sft = sfv.GetType();

  // the following service functions are only valid for Connection.

  if (sft == Sdm_SVF_LastInteractiveEvent)
  {
    err = Sdm_EC_InvalidServiceFunction;
  } else {
    SdmMutexEntry entry(_ServiceListLock);
    if (_ServiceFunctionList[sft] != NULL) {
      err = Sdm_EC_ServiceFunctionAlreadyRegistered;
    } else {
       SdmServiceFunction *copy = new SdmServiceFunction(sfv);
      _ServiceFunctionList[sft] = copy;
    }
  }
  
  TNF_PROBE_0 (SdmSession_RegisterServiceFunction_end,
      "api SdmSession",
      "sunw%debug SdmSession_RegisterServiceFunction_end");
  return err;

}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmSession::UnregisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmSession_UnregisterServiceFunction_start,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  SdmServiceFunctionType sft = sfv.GetType();

  {
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }

  TNF_PROBE_0 (SdmSession_UnregisterServiceFunction_end,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterServiceFunction_end");

   return err;
}

#endif

SdmErrorCode
SdmSession::UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmSession_UnregisterServiceFunction_start,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)

  {
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }
  
  TNF_PROBE_0 (SdmSession_UnregisterServiceFunction_end,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterServiceFunction_end");

   return err;
}

SdmErrorCode
SdmSession::GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmSession_GetServiceFunction_start,
      "api SdmSession",
      "sunw%debug SdmSession_GetServiceFunction_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)

  {
    SdmMutexEntry entry(_ServiceListLock);  
    r_serviceFunc = _ServiceFunctionList[sft];
  }

  if (r_serviceFunc == NULL) {
    err = Sdm_EC_ServiceFunctionNotRegistered;
  }

  TNF_PROBE_0 (SdmSession_GetServiceFunction_end,
      "api SdmSession",
      "sunw%debug SdmSession_GetServiceFunction_end");

  return err;
}



SdmErrorCode
SdmSession::RegisterLocalServer(SdmError& err, SdmServer* server)
{
  TNF_PROBE_0 (SdmSession_RegisterLocalServer_start,
      "api SdmSession",
      "sunw%debug SdmSession_RegisterLocalServer_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)


#if defined(USE_SOLARIS_THREADS)  
  SdmMutexEntry entry(_LocalServerDataLock);
  
  // first, check that the lists are in sync (ie. number of entries are equal).
  assert(_LocalServerList.ElementCount() 
    == _ServerWiseQueueList.ElementCount());
  assert(_LocalServerList.ElementCount() 
    == _ServerWiseDispatcherList.ElementCount());

  // add server to list of servers.
  _LocalServerList.AddElementToList(server);

  // create the request queue for the server.
  SdmRequestQueue* rq;
  rq = new SdmRequestQueue;
  _ServerWiseQueueList.AddElementToList(rq);

  // create and start up the dispatcher that monitors this queue.  
  // note that an error can occur in startup so it must be returned to
  // the caller.
  SdmTickleDispatcher *disp;  
  disp = new SdmTickleDispatcher(err, rq->GetPrimarySemaVar(), 1);
  disp->AddTask(SdmSession::SR_ProcessRequest, (SdmPrim *) this, (void*)server);
  _ServerWiseDispatcherList.AddElementToList(disp);
  disp->StartUp(err);
				     
  assert(_LocalServerList.ElementCount() == _ServerWiseQueueList.ElementCount());
#endif

  TNF_PROBE_0 (SdmSession_RegisterLocalServer_end,
      "api SdmSession",
      "sunw%debug SdmSession_RegisterLocalServer_end");

  return err;
}

SdmErrorCode
SdmSession::UnRegisterLocalServer(SdmError& err, SdmServer* server)
{
  TNF_PROBE_0 (SdmSession_UnregisterLocalServer_start,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterLocalServer_start");

  err = Sdm_EC_Success;
  
#if defined(USE_SOLARIS_THREADS)  
  // Note: it is valid to unregister a local server even if we are
  // shut down.  therefore, we don't check if we are started in this method.
  // __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  SdmMutexEntry entry(_LocalServerDataLock);
  assert(_LocalServerList.ElementCount() == _ServerWiseQueueList.ElementCount());
  assert(_LocalServerList.ElementCount() == _ServerWiseDispatcherList.ElementCount());

  // find the server.
  int ind = _LocalServerList.FindElement(server);
  assert(ind != Sdm_NPOS); 
  
  // before removing the dispatcher for this server, we need to
  // shut it down.
  SdmDispatcher* disp = (SdmDispatcher *)_ServerWiseDispatcherList[ind];
  assert(disp != NULL);
  disp->ShutDown(err);

  // need to get queue for this server.
  SdmRequestQueue* queue = (SdmRequestQueue *)_ServerWiseQueueList[ind];
  assert(queue != NULL);
 
  _ServerWiseDispatcherList.RemoveElementFromList(disp);  
  _ServerWiseQueueList.RemoveElementFromList(queue);
  _LocalServerList.RemoveElementFromList(server);

  // need to delete dispatcher and queue used for this server.
  delete disp;
  delete queue;
#endif

  TNF_PROBE_0 (SdmSession_UnregisterLocalServer_end,
      "api SdmSession",
      "sunw%debug SdmSession_UnregisterLocalServer_end");

  return err;
}

SdmServer*
SdmSession::MapToLocalServer(SdmError& err, SdmRequestQueue* queue) 
{
  TNF_PROBE_0 (SdmSession_MapToLocalServer_start,
      "api SdmSession",
      "sunw%debug SdmSession_MapToLocalServer_start");
  
  SdmServer *server = NULL;
  err = Sdm_EC_Success;
  
  if (!IsStarted()) {
    err = Sdm_EC_SessionNotStarted;
  } else {
    SdmMutexEntry entry(_LocalServerDataLock);
    assert(_LocalServerList.ElementCount() == _ServerWiseQueueList.ElementCount());
    int ind = _ServerWiseQueueList.FindElement(queue);
    if(ind != Sdm_NPOS) {
       server = (SdmServer *)_LocalServerList[ind];
    } else {
      err = Sdm_EC_LocalServerNotRegistered;
    }
  }

  TNF_PROBE_0 (SdmSession_MapToLocalServer_end,
      "api SdmSession",
      "sunw%debug SdmSession_MapToLocalServer_end");

  return server;
}

SdmRequestQueue*
SdmSession::MapToServerQueue(SdmError& err, SdmServer* server) 
{
  TNF_PROBE_0 (SdmSession_MapToServerQueue_start,
      "api SdmSession",
      "sunw%debug SdmSession_MapToServerQueue_start");

  err = Sdm_EC_Success;      
  SdmRequestQueue *queue = NULL;

  if (!IsStarted()) {
    err = Sdm_EC_SessionNotStarted;
  } else {
    SdmMutexEntry entry(_LocalServerDataLock);
    assert(_LocalServerList.ElementCount() == _ServerWiseQueueList.ElementCount());
    int ind = _LocalServerList.FindElement(server);
    if (ind != Sdm_NPOS) {
      queue =  (SdmRequestQueue *)_ServerWiseQueueList[ind]; 
    } else {
      err = Sdm_EC_LocalServerNotRegistered;
    }
  }

  TNF_PROBE_0 (SdmSession_MapToServerQueue_end,
      "api SdmSession",
      "sunw%debug SdmSession_MapToServerQueue_end");
      
  return queue;
}

SdmErrorCode 
SdmSession::HandleServerDestruction(SdmError& err, SdmServer* server)
{
  TNF_PROBE_0 (SdmSession_HandleServerDestruction_start,
      "api SdmSession",
      "sunw%debug SdmSession_HandleServerDestruction_start");
      
  err = Sdm_EC_Success;
  
  // we look to see if server is in list first before we try
  // to remove it.  It's possible that this method is being called
  // as a result of calling _MessgStoreList::ClearAndDestroyAllElements 
  // in SdmSession::ShutDown.  In this case, the server is no longer
  // in the list and calling RemoveElementFromList will result in 
  // an assertion.
  if (_MessgStoreList.FindElement(server) >= 0) {
    _MessgStoreList.RemoveElementFromList(server);
  }
  
  TNF_PROBE_0 (SdmSession_HandleServerDestruction_end,
      "api SdmSession",
      "sunw%debug SdmSession_HandleServerDestruction_end");
  return err;
}


SdmErrorCode 
SdmSession::AddMessageStore(SdmError &err, SdmMessageStore* store)
{
  err = Sdm_EC_Success;
  assert (store != NULL);
  _MessgStoreList.AddElementToList(store);
  return err;
}
  

SdmErrorCode 
SdmSession::HandleMessageStoreStartUp(SdmError &err, SdmMessageStore* store)
{
  err = Sdm_EC_Success;
  assert (store != NULL);
  SdmMutexEntry entry(_StartedMessgStoreListLock);
  _StartedMessgStoreList.AddElementToList(store);
  return err;
}

SdmErrorCode 
SdmSession::HandleMessageStoreShutDown(SdmError &err, SdmMessageStore* store)
{
  err = Sdm_EC_Success;
  assert (store != NULL);
  SdmMutexEntry entry(_StartedMessgStoreListLock);
  _StartedMessgStoreList.RemoveElementFromList(store);
  return err;
}

SdmRequestQueue*
SdmSession::GetRequestQueueFor(SdmPrim& caller)
{
  SdmRequestQueue* queue = caller.GetServerRequestQueue();
  if (!queue) {
    queue = _MasterRequestCollector;
  }
  return queue;
}

SdmReplyQueue*  
SdmSession::GetReplyQueue()
{
  return _MasterReplyCollector;
}

SdmRequestQueue*  
SdmSession::GetRequestQueue()
{
  return _MasterRequestCollector;
}

SdmErrorCode 
SdmSession::Parent(SdmError &err, SdmConnection *&r_connection)
{
  r_connection = _ParentConnection;
  return err;
}

SdmErrorCode 
SdmSession::CancelPendingOperations(SdmError &err)
{  
  TNF_PROBE_0 (SdmSession_CancelPendingOperations_start,
      "api SdmSession",
      "sunw%debug SdmSession_CancelPendingOperations_start");
  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  SdmError localError;
  SdmLinkedListIterator dli (&_MessgStoreList);
  SdmMessageStore *ms_ptr;

  while (ms_ptr = (SdmMessageStore *)(dli.NextListElement()))
  {
    ms_ptr->CancelPendingOperations(localError);
    // if the store is not started, then it's ok; ignor the error.	
    if (localError != Sdm_EC_Success && localError != Sdm_EC_MessageStoreNotStarted) {
      err = localError;
    }			
  }

  TNF_PROBE_0 (SdmSession_CancelPendingOperations_end,
      "api SdmSession",
      "sunw%debug SdmSession_CancelPendingOperations_end");

  return err;
}

SdmErrorCode 
SdmSession::SdmMessageStoreFactory(SdmError &err, SdmMessageStore *&r_mstore)
{
  TNF_PROBE_0 (SdmSession_SdmMessageStoreFactory_start,
      "api SdmSession",
      "sunw%debug SdmSession_SdmMessageStoreFactory_start");
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  err = Sdm_EC_Fail;
  TNF_PROBE_0 (SdmSession_SdmMessageStoreFactory_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmMessageStoreFactory_end");
}
  

void
SdmSession::SR_ProcessReply(SdmPrim* objThis, void* callData)
{
  ((SdmSession *) objThis)->ProcessReply(callData);
}

void
SdmSession::SR_ProcessRequest(SdmPrim* objThis, void* callData)
{
  ((SdmSession *) objThis)->ProcessRequest(callData);
}

void
SdmSession::SR_ProcessMailCheck(SdmPrim* objThis, void* callData)
{
  ((SdmSession *) objThis)->ProcessMailCheck(callData);
}

void
SdmSession::SR_ProcessAutoSave(SdmPrim* objThis, void* callData)
{
  ((SdmSession *) objThis)->ProcessAutoSave(callData);
}

void
SdmSession::SR_ProcessPing(SdmPrim* objThis, void* callData)
{
  ((SdmSession *) objThis)->ProcessPing(callData);
}


void 
SdmSession::ProcessMailCheck(void* callData)
{
  // cerr << "SdmSession::ProcessMailCheck called.\n";
  SdmError err;
  time_t nextInterval;
  time_t r_timeRemaining = 0;
  time_t now;
  
  if (CheckInactivityTime(err, r_timeRemaining, now) == Sdm_EC_Success) {
    SdmMessageStore *ms_ptr;
    SdmMutexEntry entry(_StartedMessgStoreListLock);
    SdmLinkedListIterator dli (&_StartedMessgStoreList);  
    err = Sdm_EC_Success;

    // go through list of stores and do mail check.
    while (ms_ptr = (SdmMessageStore *)(dli.NextListElement())) {
      assert (ms_ptr->IsStarted() == Sdm_True);
      if (ms_ptr->IsReadOnly() == Sdm_False) {
        ms_ptr->HandleUnsolicitMailCheck(err);
      }
    }
  }
  
  // if there is still time left because not enough inactive time has passed,
  // we reset the next time to do the mail check to be the time that is remaining.
  if (r_timeRemaining != 0) {
    nextInterval = r_timeRemaining;
  } else {
    // check for change in frequency of the check new mail interval.
    // if frequency not the same, reset frequency.  it is ok to do this because
    // we are holding the mutex in the timed dispatcher so no other thread can
    // be accessing the data in the dispatcher.
    nextInterval = GetInterval("retrieveinterval", Sdm_False, 
                    kDefaultMailCheckInterval, kMinMailCheckInterval);                  
  }
                  
  assert (_MailCheckDisp != NULL);
  if (nextInterval != _MailCheckDisp->GetFrequencyInSec()) {
    // cout << "SdmSession::ProcessMailCheck resetting frequency.\n";
    _MailCheckDisp->SetFrequencyInSec(nextInterval);
  }
}


void         
SdmSession::ProcessAutoSave(void* callData)
{
  // cerr << "SdmSession::ProcessAutoSave called.\n";
  SdmBoolean dontSave = SdmMailRc::GetMailRc()->IsValueDefined("dontautosave");
  SdmError err;
  time_t nextInterval;
  time_t r_timeRemaining = 0;
  time_t now; 
  
  if (!dontSave) {
    if (CheckInactivityTime(err, r_timeRemaining, now) == Sdm_EC_Success) {
      SdmMessageStore *ms_ptr;
      SdmMutexEntry entry(_StartedMessgStoreListLock);
      SdmLinkedListIterator dli (&_StartedMessgStoreList);  
      err = Sdm_EC_Success;

      // go through list of stores and do autosave.
      while (ms_ptr = (SdmMessageStore *)(dli.NextListElement())) {
        assert (ms_ptr->IsStarted() == Sdm_True);
        if (ms_ptr->IsReadOnly() == Sdm_False) {
          ms_ptr->HandleUnsolicitAutoSave(err);
        }
      }
    }
  }
  
  // if there is still time left because not enough inactive time has passed,
  // we reset the next time to do the autosave to be the time that is remaining.
  if (r_timeRemaining != 0) {
    nextInterval = r_timeRemaining;
  } else {
    // check for change in frequency of the autosave interval.
    // if frequency not the same, reset frequency.  it is ok to do this because
    // we are holding the mutex in the timed dispatcher so no other thread can
    // be accessing the data in the dispatcher.
    nextInterval = GetInterval("saveinterval", Sdm_True, 
                    kDefaultAutoSaveInterval, kMinAutoSaveInterval, kMaxAutoSaveInterval);
  }

  assert(_AutoSaveDisp != NULL);
  if (nextInterval != _AutoSaveDisp->GetFrequencyInSec()) {
    // cout << "SdmSession::ProcessAutoSave resetting frequency.\n";
    _AutoSaveDisp->SetFrequencyInSec(nextInterval);
  }
}



void         
SdmSession::ProcessPing(void* callData)
{
  // cerr << "SdmSession::ProcessPing called.\n";
  SdmError err;
  time_t nextInterval;
  time_t r_timeRemaining = 0;
  time_t now;
  
  if (CheckInactivityTime(err, r_timeRemaining, now) == Sdm_EC_Success) {
    SdmMessageStore *ms_ptr;
    SdmMutexEntry entry(_StartedMessgStoreListLock);
    SdmLinkedListIterator dli (&_StartedMessgStoreList);  
    err = Sdm_EC_Success;

    // go through list of stores and do ping.
    while (ms_ptr = (SdmMessageStore *)(dli.NextListElement())) {
      assert (ms_ptr->IsStarted() == Sdm_True);
      ms_ptr->HandleUnsolicitPing(err);
    }  
  }
  
  // if there is still time left because not enough inactive time has passed,
  // we reset the next time to do the ping to be the time that is remaining.
  if (r_timeRemaining != 0) {
    nextInterval = r_timeRemaining;
  } else {
    nextInterval = kPingInterval;
  }

  assert(_PingDisp != NULL);
  if (nextInterval != _PingDisp->GetFrequencyInSec()) {
    // cout << "SdmSession::ProcessPing resetting frequency.\n";
    _PingDisp->SetFrequencyInSec(nextInterval);
  }
}


SdmErrorCode
SdmSession::PostRequest(SdmError& err, SdmPrim* caller, SdmRequest *request)
{
  TNF_PROBE_0 (SdmSession_PostRequest_start,
      "api SdmSession",
      "sunw%debug SdmSession_PostRequest_start");
  err = Sdm_EC_Success;
  
  // We need to be started and attached in order to post a request!!
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  if (!IsAttached()) {
    err = Sdm_EC_SessionNotAttached;
    TNF_PROBE_0 (SdmSession_PostRequest_end,
      "api SdmSession",
      "sunw%debug SdmSession_PostRequest_end");
    return err;
  }
  
  assert(request != NULL);
  assert(caller != NULL);

#if defined(USE_SOLARIS_THREADS)  
  SdmRequestQueue* serverQueue = GetRequestQueueFor(*caller);
  assert (serverQueue != NULL);
  serverQueue->PostRequest(request);
  
#else
  HandleRequest(request);
#endif
  
  TNF_PROBE_0 (SdmSession_PostRequest_end,
      "api SdmSession",
      "sunw%debug SdmSession_PostRequest_end");
  return err;
}

void
SdmSession::ProcessRequest(void* callData)
{
  TNF_PROBE_0 (SdmSession_ProcessRequest_start,
      "api SdmSession",
      "sunw%debug SdmSession_ProcessRequest_start");

#if defined(USE_SOLARIS_THREADS)
  TNF_PROBE_0 (SdmSession_ProcessRequest_start,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessRequest Called ");

  TNF_PROBE_1 (SdmSession_ProcessRequest_thread_info,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessRequest: request processed on thread: ",
      tnf_long, thread, thr_self());
  
  // Note: callData is set to a server if this request is posted
  // on a server queue.
  SdmServer* server = (SdmServer*)callData;

  SdmRequestQueue* queue;
  if (server) {
    queue = server->GetRequestQueue();
  } else {
    queue = _MasterRequestCollector;
  }
  assert(queue != NULL);
  
  SdmRequest* request = queue->GetNextLogicalRequest();
  
  HandleRequest(request);
#endif
  
  TNF_PROBE_0 (SdmSession_ProcessRequest_end,
      "api SdmSession",
      "sunw%debug SdmSession_ProcessRequest_end");
}

void
SdmSession::HandleRequest(SdmRequest* request)
{
  // it is valid to have a NULL request.  The request queue could have been
  // emptied from and CancelPendingOperations but the semaphore still have
  // a count reflecting the posting of the request.
  if (request == NULL) {
    TNF_PROBE_0 (SdmSession_ProcessRequest_end,
      "api SdmSession",
      "sunw%debug SdmSession_ProcessRequest_end");
    return;
  }

  // Session will handle sync requests.
  if (request->GetOperator() == SdmData::AOP_SyncOp) 
  {
    SdmArgumentList* args = request->GetArguments();
    sema_t *callerLock = (sema_t*) args->GetArg(SdmArgumentList::ArgT_SemaP);
    sema_t *serverLock = (sema_t*) args->GetArg(SdmArgumentList::ArgT_SemaP);
    delete args;
    delete request;
 
    assert(callerLock != NULL);
    assert(serverLock != NULL);

    // Debug ***
    TNF_PROBE_2 (SdmSession_ProcessRequest_lock_info,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessRequest called with : ",
      tnf_ulong, callerLock, (int)callerLock,
      tnf_ulong, serverLock, (int)serverLock);
    // Debug ***

    // don't need to send reply.  only need to notify caller that
    // it can now handle the request.
    TNF_PROBE_0 (SdmSession_ProcessRequest_wakeup_caller,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessRequest waking up callerLock.");
    sema_post(callerLock);
 
    // wait until caller is done procession synchronous request.
    TNF_PROBE_0 (SdmSession_ProcessRequest_sleep_on_server,
      "async SdmSession",
      "sunw%debug SdmSession::SdmSession::ProcessRequest sleep on serverLock.");
    sema_wait(serverLock);

    TNF_PROBE_0 (SdmSession_ProcessRequest_wake_from_server,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessRequest wake from serverLock..");

    // clean up semaphore.
#if defined(USE_SOLARIS_THREADS)
    sema_destroy(serverLock);
#endif
    delete serverLock;
  

  // Session will handle ping requests.
  } 
  else if (request->GetOperator() == SdmSession::AOP_Ping) {
    SdmArgumentList* args = request->GetArguments();
    sema_t *callerLock = (sema_t*) args->GetArg(SdmArgumentList::ArgT_SemaP);
    delete args;
    delete request;
    assert(callerLock != NULL);
    sema_post(callerLock);
    
  } 
  else {
    // Debugs ***
    if (request->GetOperator() == SdmData::AOP_GetContents_2) {
      SdmArgumentList* args = request->GetArguments();
      int stOffS = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
      int length = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
       
      args->ResetGetSegment();
      
      /*
      sprintf(buffer, 
        "SdmSession::ProcessRequest called with server=%lu, args=%d,%d\n",
        (server ? ((SdmPrim*)server)->_ObjSignature : 0),
              stOffS, length);
      cout << buffer;
      */
      
      TNF_PROBE_2 (SdmSession_ProcessRequest_calls_server,
        "async SdmSession",
        "sunw%debug SdmSession::ProcessRequest called with args: ",
        tnf_long, startOffset, stOffS,
        tnf_long, length, length);
    }
    // ** Debugs **

    SdmPrim* obj = request->GetClientObject();
  
    assert(obj != NULL);	// we better have a client object in the request!!
  
    obj->InCallDispatchPoint(request);
  }
}


void
SdmSession::CancelRequest(const SdmRequest* request)
{
  TNF_PROBE_0 (SdmSession_CancelRequest_start,
      "api SdmSession",
      "sunw%debug SdmSession_CancelRequest_start");
  // Debugs ***
  /*
  char buffer[5000];
  cout << "SdmSession::CancelRequest Called\n";
  sprintf(buffer, "SdmSession::CancelRequest: request processed on thread %u\n",
		  thr_self());
  cout << buffer;
  */
  TNF_PROBE_1 (SdmSession_CancelRequest_thread_info,
    "async SdmSession",
    "sunw%debug SdmSession::CancelRequest: request processed on thread: ",
    tnf_ulong, thread, thr_self());
  // Debugs ***
  
  assert(request != NULL);

  // ** Debugs **
  if (request->GetOperator() == SdmData::AOP_SyncOp) 
  {
    SdmArgumentList* args = request->GetArguments();
    sema_t *callerLock = (sema_t*) args->GetArg(SdmArgumentList::ArgT_SemaP);
    sema_t *serverLock = (sema_t*) args->GetArg(SdmArgumentList::ArgT_SemaP);
    SdmError *callerError = (SdmError*) args->GetArg(SdmArgumentList::ArgT_ErrorP);
    delete args;
    delete ((SdmRequest*)request);
 
    assert(callerLock != NULL);
    assert(serverLock != NULL);
    assert(callerError != NULL);

    // ** Debug ***
    /*
    char buffer[2000];
    sprintf(buffer, 
      "SdmSession::CancelRequest called with locks=%d,%d\n",
      (int)callerLock, (int)serverLock);
      cout << buffer;	
    */	
    TNF_PROBE_2 (SdmSession_CancelRequest_lock_info,
      "async SdmSession",
      "sunw%debug SdmSession::CancelRequest called with locks: ",
      tnf_long, callerLock, (long)callerLock,
      tnf_long, serverLock, (long) serverLock);
    // Debug ***

    // set the error for the caller.  this needs to be done
    // before we wake up the caller.
    *callerError = Sdm_EC_Cancel;
    
    // don't need to send reply.  only need to wake up caller.
    //cout << "SdmSession::CancelRequest waking up callerLock.\n";
    TNF_PROBE_0 (SdmSession_CancelRequest_waking_up_caller,
      "async SdmSession",
      "sunw%debug SdmSession::CancelRequest waking up callerLock: ");
    sema_post(callerLock);
 
    // need to wait until caller is done.
    //cout << "SdmSession::CancelRequest sleep on serverLock\n";
    TNF_PROBE_0 (SdmSession_CancelRequest_sleep_on_server,
      "async SdmSession",
      "sunw%debug SdmSession::CancelRequest sleep on serverLock: ");
    sema_wait(serverLock);
    // cout << "SdmSession::CancelRequest wake from serverLock.\n";
    TNF_PROBE_0 (SdmSession_CancelRequest_wake_from_server,
      "async SdmSession",
      "sunw%debug SdmSession::CancelRequest sleep on serverLock: ");

    // still need to clean up semaphore.
    delete serverLock;
  }  
  else 
  {
    // Debugs ***
    if (request->GetOperator() == SdmData::AOP_GetContents_2) {
      SdmArgumentList* args = request->GetArguments();
      int stOffS = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
      int length = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
       
      args->ResetGetSegment();
      /*
      sprintf(buffer, 
        "SdmSession::CancelRequest called with args=%d,%d\n",
              stOffS, length);
      cout << buffer;
      */
      TNF_PROBE_2 (SdmSession_CancelRequest_wake_from_server,
        "async SdmSession",
        "sunw%debug SdmSession::CancelRequest called with args: ",
        tnf_long, startOffset, stOffS,
        tnf_long, length, length);
    }
    // ** Debugs **

    SdmPrim* obj = request->GetClientObject();
  
    assert(obj != NULL);	// we better have a client object in the request!!
  
    obj->CancelRequestDispatchPoint(request);
  }
  TNF_PROBE_0 (SdmSession_CancelRequest_end,
      "api SdmSession",
      "sunw%debug SdmSession_CancelRequest_end");
}

SdmErrorCode
SdmSession::PostReply(SdmError& err, SdmPrim* caller, SdmReply *reply)
{
  TNF_PROBE_0 (SdmSession_PostReply_start,
      "api SdmSession",
      "sunw%debug SdmSession_PostReply_start");
  err = Sdm_EC_Success;
  
  // We need to be started and attached in order to post a request!!
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  
  if (!IsAttached()) {
    err = Sdm_EC_SessionNotAttached;
    TNF_PROBE_0 (SdmSession_PostReply_end,
      "api SdmSession",
      "sunw%debug SdmSession_PostReply_end");
    return err;
  }
  
  assert(reply != NULL);
  assert(caller != NULL);

#if defined(USE_SOLARIS_THREADS)  
  assert (_MasterReplyCollector != NULL);
  _MasterReplyCollector->PostReply(reply);
  
#else
  HandleReply(reply);
#endif
  
  TNF_PROBE_0 (SdmSession_PostReply_end,
      "api SdmSession",
      "sunw%debug SdmSession_PostReply_end");
  return err;
}
void
SdmSession::ProcessReply(void* callData)
{
  TNF_PROBE_0 (SdmSession_ProcessReply_start,
      "api SdmSession",
      "sunw%debug SdmSession_ProcessReply_start");
  // Debugs ***
  TNF_PROBE_0 (SdmSession_ProcessReply_start,
    "async SdmSession",
    "sunw%debug SdmSession::ProcessReply Called ");


  TNF_PROBE_1 (SdmSession_ProcessReply_thread_info,
    "async SdmSession",
    "sunw%debug SdmSession::ProcessReply: This reply is being processed on thread: ",
    tnf_long, thread, thr_self());
  
  // Debugs ***
  	
  SdmReply* reply;
 
  reply = _MasterReplyCollector->GetNextLogicalReply();

  HandleReply(reply);

  TNF_PROBE_0 (SdmSession_ProcessReply_end,
      "api SdmSession",
      "sunw%debug SdmSession_ProcessReply_end");
}

void
SdmSession::HandleReply(SdmReply *reply)
{
  TNF_PROBE_0 (SdmSession_HandleReply_reply_retrieved,
    "async SdmSession",
    "sunw%debug SdmSession::Reply retrieved and ready for Dispatch");
  
  // ** Debugs **
  // Session will handle sync requests.
  if (reply->GetOperator() == SdmSession::AOP_Ping) 
  {
    SdmEvent* event = reply->GetEvent();
    sema_t *callerLock = event->sdmSessionShutdown.callerLock;
    assert(callerLock != NULL);
    sema_post(callerLock);
    return;
     
  } else if (reply->GetOperator() == SdmData::AOP_GetContents_2) {
    SdmEvent* event = reply->GetEvent();
    int stOffS = event->sdmGetContents.startOffset;
    int length = event->sdmGetContents.length;
    
    TNF_PROBE_2 (SdmSession_ProcessReply_reply_handle,
      "async SdmSession",
      "sunw%debug SdmSession::ProcessReply called with args: ",
      tnf_long, startOffset, stOffS,
      tnf_long, length, length);
  }
  
  if (!IsAttached()) {
    cout << "Error: SdmSession::HandleReply Session is not attached to front end event queue. Reply not dispatched.\n";
    // what to do when error occurs here??
    return;
  }
  
  // post reply on the front end queue and write to pipe to let front end know that
  // there is something in the queue.  Note: we should write to the pipe AFTER the
  // reply has been added to the reply queue.
  char bogus_buf = 0;
  _FrontEndEventQueue->PostReply(reply);

  // if write fails, then we are in BIG trouble.  we've lost contact with the
  // front end.  Assert and Abort!!
  if (SdmSystemUtility::SafeWrite(_WriteFileDesc, &bogus_buf, 1) < 0) {
     SdmUtility::LogError(Sdm_True, 
        "SdmSession::PostReply: can not write to pipe. errno=%d\n", errno);
     assert (0);
     abort();
  }
}



SdmErrorCode
SdmSession::Attach(SdmError &err, int fileDesc, SdmReplyQueue &frontEndQueue)
{
  TNF_PROBE_0 (SdmSession_Attach_start,
      "api SdmSession",
      "sunw%debug SdmSession_Attach_start");
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
  err = Sdm_EC_Success;

  // return error if we are already attached to a queue.
  if (IsAttached()) {
     err = Sdm_EC_SessionAlreadyAttached;
    TNF_PROBE_0 (SdmSession_Attach_end,
      "api SdmSession",
      "sunw%debug SdmSession_Attach_end");
     return err;
  }
   _FrontEndEventQueue = &frontEndQueue;
   _WriteFileDesc = fileDesc;
   
  TNF_PROBE_0 (SdmSession_Attach_end,
      "api SdmSession",
      "sunw%debug SdmSession_Attach_end");
   return err;
}
 

SdmErrorCode
SdmSession::Detach(SdmError &err, int &fileDesc, SdmReplyQueue *&frontEndQueue)
{ 
  TNF_PROBE_0 (SdmSession_Detach_start,
      "api SdmSession",
      "sunw%debug SdmSession_Detach_start");
  err = Sdm_EC_Success;

  // return error if we are not attached to a queue.
  if (!IsAttached()) {
     err = Sdm_EC_SessionNotAttached;
    TNF_PROBE_0 (SdmSession_Detach_end,
      "api SdmSession",
      "sunw%debug SdmSession_Detach_end");
     return err;
  }
  
  frontEndQueue = _FrontEndEventQueue;
  fileDesc = _WriteFileDesc;
  
  _FrontEndEventQueue = NULL;
  _WriteFileDesc = -1;
   
  TNF_PROBE_0 (SdmSession_Detach_end,
      "api SdmSession",
      "sunw%debug SdmSession_Detach_end");
  return err;
} 

SdmSessionType  
SdmSession::GetType() const
{
   return _SessionType;
}

 
 
SdmErrorCode
SdmSession::ShutDownMasterRequestDispatcher(SdmError &err)
{
  err = Sdm_EC_Success;

#if defined(USE_SOLARIS_THREADS)  
  sema_t	callerLock;
  if (sema_init(&callerLock, 0, USYNC_THREAD, 0)) {
    err = Sdm_EC_Fail;
    return err;
  }

  // create and post request to ping the queue.
  SdmArgumentList* args = new SdmArgumentList;
  args->PutArg(&callerLock);
  SdmRequest* reqst = new SdmRequest(args, SdmSession::AOP_Ping, this);
  _MasterRequestCollector->PostRequest(reqst);

  // wait for our turn on the queue.
  sema_wait(&callerLock);
  sema_destroy(&callerLock);
#endif
  
  // shut down the threads monitoring the request queue.
  _MasterRequestDisp->ShutDown(err);
  
  // don't delete request or args.  it will get deleted by ProcessRequest.
    
  return err;
}

SdmErrorCode
SdmSession::ShutDownMasterReplyDispatcher(SdmError &err)
{
  err = Sdm_EC_Success;

#if defined(USE_SOLARIS_THREADS)  
  sema_t	callerLock;
  if (sema_init(&callerLock, 0, USYNC_THREAD, 0)) {
    err = Sdm_EC_Fail;
    return err;
  }

  // create and post reply to ping the queue.
  SdmEvent *event = new SdmEvent;
  event->sdmSessionShutdown.error = new SdmError;
  event->sdmSessionShutdown.type = Sdm_Event_SessionShutdown;
  event->sdmSessionShutdown.callerLock = &callerLock;
  SdmReply* reply = new SdmReply;  
  reply->SetEvent(event);
  reply->SetOperator(SdmSession::AOP_Ping);
  reply->SetClientObject(this);
  reply->SetServiceFunctionType(Sdm_SVF_Other);
  _MasterReplyCollector->PostReply(reply);

  // wait for out turn on the queue.
  sema_wait(&callerLock);
  sema_destroy(&callerLock);

  delete (event->sdmSessionShutdown.error);
  delete event;
  delete reply;
  
#endif
  
  // shut down the threads monitoring the reply queue.
  _MasterReplyDisp->ShutDown(err);
    
  return err;
}

time_t
SdmSession::GetInterval(SdmPollEventType event)
{
    time_t interval = 0;
    switch (event) {
      case SESUETYPE_MailCheck:
        interval = _MailCheckDisp ? _MailCheckDisp->GetFrequencyInSec() : 
                      kDefaultMailCheckInterval;
        break;
      case SESUETYPE_AutoSave:
        interval = _AutoSaveDisp ? _AutoSaveDisp->GetFrequencyInSec() : 
                      kDefaultAutoSaveInterval;
        break;
      case SESUETYPE_Ping:
        interval = _PingDisp ? _PingDisp->GetFrequencyInSec() : 
                      kPingInterval;
        break;
      default:
        assert (0);
    }
    return interval;
}



time_t
SdmSession::GetInterval(const char* varName, const SdmBoolean varInMinutes, 
  const time_t defaultValue, const time_t minValue, const time_t maxValue)
{
  SdmError err;
  time_t interval = defaultValue;
  char *value = NULL;  
    
  // get time interval for mail check.
  SdmMailRc::GetMailRc()->GetValue(err, varName, &value);
  if (err == Sdm_EC_Success && value != NULL && *value != '\0') {
    interval = strtol(value, NULL, 10);
    if (varInMinutes) {
      interval *= 60;
    }
  }
  err = Sdm_EC_Success;
  
  // make sure interval is not less than the minimum.
  if (interval < minValue) {
    interval = minValue;   
  }  

  // make sure interval does not exceed maximum.
  if (maxValue > 0 && interval > maxValue) {
    interval = maxValue;   
  }  
  
  if (value) {
    free (value);
  }
  return interval;
}



SdmErrorCode
SdmSession::CheckInactivityTime(SdmError &err, time_t &r_timeRemaining, time_t &now)
{
  err = Sdm_EC_Success;
  r_timeRemaining = 0;

  // get the global connection.  assert if it's not defined.
  SdmConnection *connection = SdmConnection::GetConnection();
  assert (connection != NULL);
    
  // get service function that's used to get the last interactive time.
  SdmServiceFunction *r_serviceFunc = NULL;  
  connection->GetServiceFunction(err, r_serviceFunc, 
                      Sdm_SVF_LastInteractiveEvent);
   
  // reset error if the service function is not registered.                   
  if (err == Sdm_EC_ServiceFunctionNotRegistered) {
    err = Sdm_EC_Success;
  }
  
  // if there's no service function registered, then we just allow the 
  // unsolicited event to proceed.                    
  if (!r_serviceFunc) {
    now = time(NULL);  // call time() as late as possible.
    return err;
    
  } else {
    // call service function to get the last interactive time.
    time_t lastInteractive = 0;
    SdmEvent *event = new SdmEvent;
    SdmError *eventError = new SdmError;
    event->sdmGetLastInteractiveEventTime.type = Sdm_Event_GetLastInteractiveEventTime;
    event->sdmGetLastInteractiveEventTime.error = eventError;
    event->sdmGetLastInteractiveEventTime.lastInteractiveTime = &lastInteractive;
    r_serviceFunc->Execute(err, Sdm_SVF_LastInteractiveEvent, event);


    // return callback error is set.
    if (*(event->sdmGetLastInteractiveEventTime.error)) {
      err = *(event->sdmGetLastInteractiveEventTime.error);
    } else {
      now = time(NULL);  // call time() as late as possible.
      time_t interactiveIdleTime = (now - lastInteractive);
    
      // get the inactivity interval from mailrc.
      time_t inactivityInterval = GetInterval("inactivityinterval", Sdm_False, 
                  kDefaultInactivityInterval, kMinInactivityInterval, kMaxInactivityInterval);
            
      // if not enough inactive time has passed, return error.
      if (interactiveIdleTime < inactivityInterval) {
         r_timeRemaining =  inactivityInterval - interactiveIdleTime;
         err = Sdm_EC_InactivityIntervalNotPassed;
      } 
    }
    
    // clean up error in event.  this is not cleaned up by user's callback.
    // clean up event.  this is not cleaned up by the user's callback.
    delete eventError;
    delete event;
  }

  return err;    
}


//
// Global functions for handling session linkage to the caller.
//
SdmErrorCode 
SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata, SdmSession &session)
{
  TNF_PROBE_0 (SdmSession_SdmAttachSession_start,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_start");
  err = Sdm_EC_Success;
  
  if (!session.IsStarted()) {
    err = Sdm_EC_SessionNotStarted;
    TNF_PROBE_0 (SdmSession_SdmAttachSession_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_end");
    return err;
  }

  // return error if the session is already attached to something.
  if (session.IsAttached()) {
    err = Sdm_EC_SessionAlreadyAttached;
    TNF_PROBE_0 (SdmSession_SdmAttachSession_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_end");
    return err;
  }
    
  SdmReplyQueue *frontEndQueue = new SdmReplyQueue;
  
  // create the pipe.
  int pipe_fds[2];
  if (pipe(pipe_fds) < 0) {
    err = Sdm_EC_Fail;
    TNF_PROBE_0 (SdmSession_SdmAttachSession_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_end");
    return err;
  }
  
  // attach the pipe to the session.
  if (session.Attach(err, pipe_fds[WRITE], *frontEndQueue)) {
    TNF_PROBE_0 (SdmSession_SdmAttachSession_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_end");
    return err;
  }
  
  r_pipe = pipe_fds[READ];
  r_calldata = frontEndQueue;
  
  TNF_PROBE_0 (SdmSession_SdmAttachSession_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmAttachSession_end");
  return err;
}

SdmErrorCode 
SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata, SdmSession &session, void* appContext)
{
  SdmAttachSession(err, r_pipe, r_calldata, session);
#if !defined(USE_SOLARIS_THREADS)
  if (appContext) {
    for (int i = 0; i < session.SdmD_TimeOutDataSet_Size; i++) {
        SdmTimeOutInfo *info = session._TimeOutDataList[i];
        if (info) {
          info->_appContext = appContext;
        }
    }
  }  
#endif
  return err;
}

int          
SdmHandleSessionEvent(void *calldata, int *pipe, unsigned long *id)
{
  TNF_PROBE_0 (SdmSession_SdmHandleSessionEvent_start,
      "api SdmSession",
      "sunw%debug SdmSession_SdmHandleSessionEvent_start");

  SdmReplyQueue *frontEndQueue = (SdmReplyQueue*) calldata;
   
  // the following pointers better not be NULL or else we're in trouble.
  assert (frontEndQueue != NULL);
  assert (pipe != NULL);

  // There was activity on the pipe. Read one byte, and get one reply
  // from the reply queue. We don't want to get more than that or we will run
  // the risk of spending too much time in the callbacks.
  //
  char bogus_buf;
  ssize_t rc;
  if ((rc = SdmSystemUtility::SafeRead(*pipe, &bogus_buf, 1)) <= 0) {

    // if the error is NOT because the pipe got close, we want to log the error.
    if (!(rc == 0 && errno == 0)) {
      SdmUtility::LogError(Sdm_False, 
        "SdmHandleSessionEvent: can not read from pipe. errno=%d\n", errno);
    }

    // regardless of the error, we want to clean up the queue and close the
    // read file descriptor in the pipe.
    delete frontEndQueue;
    close (*pipe);
 
    TNF_PROBE_0 (SdmSession_SdmHandleSessionEvent_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmHandleSessionEvent_end");

    return -1;		// return error to caller.
  }
  	
  SdmReply* reply = frontEndQueue->GetNextLogicalReply();
  if (reply == NULL) {
    SdmUtility::LogError(Sdm_True, "SdmHandleSessionEvent: can not read from reply queue\n");

    // this is a serious error if there is nothing on the reply queue.
    // somthing is out of sync.
    // delete the front end queue and close the read file descriptor.
    delete frontEndQueue;
    close (*pipe);

    TNF_PROBE_0 (SdmSession_SdmHandleSessionEvent_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmHandleSessionEvent_end");

    return -1;		// return error to caller.
  }

  SdmEvent *event = reply->GetEvent();

  // ** Debugs **
  if (reply->GetOperator() == SdmData::AOP_GetContents_2) {
    int stOffS = event->sdmGetContents.startOffset;
    int length = event->sdmGetContents.length;
     
    TNF_PROBE_2 (SdmSession_HandleSessionEvent_args_info,
      "async SdmSession",
      "sunw%debug SdmSession::SdmHandleSessionEvent called with args",
      tnf_long, startOffset, stOffS,
      tnf_long, length, length);
  }
  // ** Debugs **  
  
  SdmServiceFunction *serviceFunc = reply->GetServiceFunction();
  assert(serviceFunc != NULL);
  SdmServiceFunctionType serviceType = reply->GetServiceFunctionType();
  assert (serviceType != Sdm_SVF_Unknown);  // we better have a service type defined.
  
  SdmError err;
  // save pointer to error in event.  we need to clean it up later.
  SdmError *eventError = event->sdmBase.error;    
  serviceFunc->Execute(err, serviceType, event);
 
  // clean up error in event.  this is not cleaned up by user's callback.
  // clean up serviceFunc.  this is a copy that got created for this
  // reply so we need to delete it when we are done.
  // clean up event.  this is not cleaned up by the user's callback.
  // clean up reply.  this is must be cleaned up by the mid-end.
  delete (eventError);
  delete serviceFunc;
  delete event;
  delete reply;

  TNF_PROBE_0 (SdmSession_SdmHandleSessionEvent_end,
      "api SdmSession",
      "sunw%debug SdmSession_SdmHandleSessionEvent_end");

  return 0;
}

int
SdmHandleTimeOutInterval(SdmSession *ses, SdmPollEventType event, unsigned long &nextInterval)
{
#if !defined(USE_SOLARIS_THREADS)
  
  SdmConnection *conn = SdmConnection::GetConnection();
  
  // return error if session is undefined, connection is undefined,
  // or session is no longer valid.
  if (ses == NULL || conn == NULL || !conn->IsValidSession(*ses)) {
    return -1;
    
  // we've got a valid session.  check to see if session is started.  
  // If so, call session to process the unsolicited event.    
  } else if (ses->IsStarted()) {
    switch (event) {
      case SdmSession::SESUETYPE_MailCheck:
        ses->ProcessMailCheck(NULL);
        break;
      case SdmSession::SESUETYPE_AutoSave:
        ses->ProcessAutoSave(NULL);
        break;
      case SdmSession::SESUETYPE_Ping:
        ses->ProcessPing(NULL);
        break;
      default:
        assert (0);
        break;
    }
    nextInterval = ses->GetInterval(event) * 1000;  // nextInterval must be in milliseconds.
  }
#endif

  return 0;
} 
 
