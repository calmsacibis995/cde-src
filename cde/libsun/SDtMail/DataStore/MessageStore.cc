/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Message Store Class.

#pragma ident "@(#)MessageStore.cc	1.150 97/06/09 SMI"

// Include Files.
#include <iostream.h>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Token.hh>
#include <Manager/ISess.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <PortObjs/DataPort.hh>
#include <CtrlObjs/RepSWER.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <TranObjs/ArgumentList.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <DataStructs/RWLockEntry.hh>
#include <Utils/TnfProbe.hh>
#include <SDtMail/Connection.hh>
#include <CtrlObjs/TimeDisp.hh>

SdmMessageStore::SdmMessageStore(SdmError& err, SdmSession& session)
  :SdmServer(Sdm_CL_MessageStore, err),
   _ServiceFunctionList(SdmD_ServiceFunctionSet_Size),
   _MessgList(), _NewMessgList(),  _DataPort(NULL),
   _ReadOnly(Sdm_False)
{
  TNF_PROBE_0 (SdmMessageStore_constructor_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_constructor_start");

  err = Sdm_EC_Success;
  __SdmPRIM_REGISTERENTITY(&session , this);

  _ParentSession = &session;
  
  int rc;
#if defined(USE_SOLARIS_THREADS)
  rc = rwlock_init(&_MessageListLock, USYNC_THREAD, NULL);
  assert (rc == 0);
#endif
  rc =   mutex_init(&_ServiceListLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  rc =   mutex_init(&_MessageLogLock, USYNC_THREAD, NULL);
  assert (rc == 0);

  for (int i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
    _ServiceFunctionList[i] = NULL;


  TNF_PROBE_0 (SdmMessageStore_constructor_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_constructor_end");
}

SdmMessageStore& 
SdmMessageStore::operator=(const SdmMessageStore &rhs)
{
  cout << "*** Error: SdmMessageStore assignment operator called\n";
  assert(Sdm_False);
  return *this;
}


SdmMessageStore::SdmMessageStore(const SdmMessageStore& copy) 
  :SdmServer(Sdm_CL_MessageStore, SdmError())
{
  cout << "*** Error: SdmMessageStore copy constructor called\n";
  assert(Sdm_False);
}

SdmMessageStore::~SdmMessageStore()
{
  TNF_PROBE_0 (SdmMessageStore_destructor_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_destructor_start");

  SdmError err;
  
  if (_IsStarted) {
    ShutDown(err);
  }

#if defined(USE_SOLARIS_THREADS)
  rwlock_destroy(&_MessageListLock);
#endif
  mutex_destroy(&_ServiceListLock);
  mutex_destroy(&_MessageLogLock);

  // tell the parent that we're being destroyed.
  if (_ParentSession) {
    _ParentSession->HandleServerDestruction(err, this);
  }
    
  __SdmPRIM_UNREGISTERENTITY;

  TNF_PROBE_0 (SdmMessageStore_destructor_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_destructor_end");
}

SdmErrorCode
SdmMessageStore::StartUp(SdmError& err)
{
  TNF_PROBE_0 (SdmMessageStore_StartUp_start,
	       "api SdmMessageStore",
	       "sunw%start SdmMessageStore_StartUp_start");

  err = Sdm_EC_Success;

  if (_IsStarted){
    err = Sdm_EC_MessageStoreAlreadyStarted;
    TNF_PROBE_0 (SdmMessageStore_StartUp_end,
		 "api SdmMessageStore",
		 "sunw%end SdmMessageStore_StartUp_end");
    return err;
  }

  // create data port for this store.
  _DataPort = new SdmDataPort(Sdm_CL_DataPort);

  {
    SdmMutexEntry entry(_ServiceListLock);  
    // initialize list of service functions.    
    _ServiceFunctionList.SetVectorSize(SdmD_ServiceFunctionSet_Size); 
    for (int i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
      _ServiceFunctionList[i] = NULL;
  }

  // start up the thread for the server of this store.
  if (_ParentSession->RegisterLocalServer(err, this)) {
    TNF_PROBE_0 (SdmMessageStore_StartUp_end,
		 "api SdmMessageStore",
		 "sunw%end SdmMessageStore_StartUp_end");
    return err;
  }

  // startup the dataport.
  if (_DataPort->StartUp(err)) {
    TNF_PROBE_0 (SdmMessageStore_StartUp_end,
		 "api SdmMessageStore",
		 "sunw%end SdmMessageStore_StartUp_end");
    return err;
  }


  // register callback to handle data port events.
  if (_DataPort->RegisterCallback(err, DPCBTYPE_ErrorLog,
				  SdmMessageStore::SR_ProcessDataPortEvent, this)  ||
      _DataPort->RegisterCallback(err, DPCBTYPE_DebugLog,
				  SdmMessageStore::SR_ProcessDataPortEvent, this)  ||
      _DataPort->RegisterCallback(err, DPCBTYPE_NotifyLog,
				  SdmMessageStore::SR_ProcessDataPortEvent, this)  ||
      _DataPort->RegisterCallback(err, DPCBTYPE_LockActivity,
				  SdmMessageStore::SR_ProcessDataPortEvent, this)  ||
      _DataPort->RegisterCallback(err, DPCBTYPE_BusyIndication,
				  SdmMessageStore::SR_ProcessDataPortEvent, this))
    {
      TNF_PROBE_0 (SdmMessageStore_StartUp_end,
		   "api SdmMessageStore",
		   "sunw%end SdmMessageStore_StartUp_end");
      return err;
    }

  _IsStarted = Sdm_True;
  _ParentSession->HandleMessageStoreStartUp(err, this);

  TNF_PROBE_0 (SdmMessageStore_StartUp_end,
	       "api SdmMessageStore",
	       "sunw%end SdmMessageStore_StartUp_end");
      
  return err;
}

SdmErrorCode
SdmMessageStore::ShutDown(SdmError& err)
{
  TNF_PROBE_0 (SdmMessageStore_ShutDown_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ShutDown_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  

  { 
    // We get a sync request lock in this scope so that any
    // pending requests can be finished before we start the
    // shutdown.
    SdmSyncRequestEntry entry(err, *this);  

    // ignore the error if the session is not attached.  this means
    // that no requests can be posted.  it is valid that the session
    // is started but not attached.
    if (err == Sdm_EC_SessionNotAttached) {
      err = Sdm_EC_Success;
    } 
    
    // let session know that this store is shut down so that no more
    // unsolicited requests will occur.
    _ParentSession->HandleMessageStoreShutDown(err, this);
        
    // unregister callback for data port events.  do this before we
    // close down data port and clean up service functions.
    if (_DataPort->RegisterCallback(err, DPCBTYPE_ErrorLog, NULL, NULL) ||  
        _DataPort->RegisterCallback(err, DPCBTYPE_DebugLog, NULL, NULL) ||
        _DataPort->RegisterCallback(err, DPCBTYPE_NotifyLog, NULL, NULL) ||
        _DataPort->RegisterCallback(err, DPCBTYPE_BusyIndication, NULL, NULL)) 
    {
      TNF_PROBE_0 (SdmMessageStore_ShutDown_end,
        "api SdmMessageStore",
        "sunw%end SdmMessageStore_ShutDown_end");
      return err;
    }

    // call CancelPendingOperations to cancel anything else left
    // in the request queue.
    if (!err) 
      CancelPendingOperations(err);
    // Note: Request queue should be empty after the 
    // CancelPendingOperations is done.


    // return if any error occurred.
    if (err) {
      TNF_PROBE_0 (SdmMessageStore_ShutDown_end,
        "api SdmMessageStore",
        "sunw%end SdmMessageStore_ShutDown_end");
      return err;
    }

    // clear out list of messages.  save it in local list.
    // we don't delete the message right away because the
    // destructor of SdmMessage tries to get the _MessageListLock.
    // This will cause a deadlock to occur.  Therefore,
    // we save the messages in a separate list, clear out the
    // original message lists, release the lock, and then
    // delete the message objects separately.
    SdmVector<SdmMessage *> saveMessgList;
    {
      SdmRWLockEntry entry(_MessageListLock, Sdm_False);
      SdmVectorIterator<SdmMessage *> iter(&_MessgList);
      SdmMessage **msg;
      while ( (msg = iter.NextListElement()) != NULL) {
        if (*msg != NULL) {
          saveMessgList.AddElementToList(*msg);
        }
      }
      _MessgList.ClearAllElements();
      SdmVectorIterator<SdmMessage *> iter2(&_NewMessgList);
      while ( (msg = iter2.NextListElement()) != NULL) {
        if (*msg != NULL) {
          saveMessgList.AddElementToList(*msg);
        }
      }
      _NewMessgList.ClearAllElements();
    }

    // delete message objects.  Do this before we shut 
    // down the data port so that changes to messages can be
    // commited/discarded into the store if necessary.  
    // note that we need to set the _HasSyncLock flag in the message
    // so that the SdmMessage destructor does not try to get the  
    // sync lock again.
    SdmVectorIterator<SdmMessage *> iter(&saveMessgList);
    SdmMessage **msg;
    while ( (msg = iter.NextListElement()) != NULL) {
      (*msg)->_HasSyncLock = Sdm_True;
      delete *msg;
    }


    if (_DataPort->ShutDown(err))
    {
      TNF_PROBE_0 (SdmMessageStore_ShutDown_end,
          "api SdmMessageStore",
          "sunw%end SdmMessageStore_ShutDown_end");
      return err;
    }
    
    delete _DataPort;
    _DataPort = NULL;
    
    {
      SdmMutexEntry entry(_ServiceListLock);  
      _ServiceFunctionList.ClearAndDestroyAllElements();
    }

    _IsStarted = Sdm_False;    
  }
	

  // At this point, the queue is empty and all the messages are 
  // removed from the store.  we now need to shutdown the
  // local server.  we do this without the sync request lock
  // because shutting down the local server requires the request
  // queue to be empty.  the queue would still have one request if
  // we were still holding the sync request lock.

  // Shutdown the server only if it is registered.
  if (_ParentSession->MapToServerQueue(err, this) != NULL) {
    ShutDownLocalServer(err);
  } else {
    err.Reset();
  }

  TNF_PROBE_0 (SdmMessageStore_ShutDown_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ShutDown_end");

  return err;
}


SdmErrorCode
SdmMessageStore::Parent(SdmError &err, SdmSession *&r_session)
{
  TNF_PROBE_0 (SdmMessageStore_Parent_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Parent_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    r_session = _ParentSession;
  }  

  TNF_PROBE_0 (SdmMessageStore_Parent_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Parent_end");

  return err;
}

SdmErrorCode
SdmMessageStore::Open(SdmError &err, SdmMessageNumber &r_nmsgs,
                      SdmBoolean &r_readOnly, const SdmToken &token)
{
  TNF_PROBE_0 (SdmMessageStore_Open_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Open_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _Open(err, r_nmsgs, r_readOnly, token);
  }  

  TNF_PROBE_1 (SdmMessageStore_Open_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Open_end",
      tnf_ulong, r_readOnly, (unsigned long) r_readOnly);

  return err;  
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmMessageStore::Open_Async(SdmError& err, const SdmServiceFunction& svf, void* clientData, 
                   const SdmToken &token)
{
  TNF_PROBE_0 (SdmMessageStore_Open_Async_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Open_Async_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessageStore *me = (SdmMessageStore*) this;  
  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);
  SdmArgumentList* args = new SdmArgumentList; 
  args->PutArg(token);

  
  SdmRequest* reqst = new SdmRequest(args, SdmMessageStore::AOP_Open, this);
  reqst->SetServiceFunction(svfClone);

  if (me->_CollectorSession->PostRequest(err, me, reqst))
    delete reqst;
	 
  TNF_PROBE_0 (SdmMessageStore_Open_Async_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Open_Async_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::Close_Async(SdmError &err, const SdmServiceFunction& svf, void* clientData)
{
  TNF_PROBE_0 (SdmMessageStore_Close_Async_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Close_Async_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessageStore *me = (SdmMessageStore*) this;  
  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);

  SdmRequest* reqst = new SdmRequest(NULL, SdmMessageStore::AOP_Close, this);
  reqst->SetServiceFunction(svfClone);

  if (me->_CollectorSession->PostRequest(err, me, reqst))
    delete reqst;

  TNF_PROBE_0 (SdmMessageStore_Close_Async_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Close_Async_end");

  return err;  
}

#endif

SdmErrorCode 
SdmMessageStore::Close(SdmError &err)
{
  TNF_PROBE_0 (SdmMessageStore_Close_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Close_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _Close(err);
  }  

  TNF_PROBE_0 (SdmMessageStore_Close_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Close_end");

  return err;  
}


SdmErrorCode 
SdmMessageStore::Create(SdmError &err, const SdmString& name)
{
  TNF_PROBE_0 (SdmMessageStore_Create_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Create_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Create(err, name);
  }  

  TNF_PROBE_0 (SdmMessageStore_Create_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Create_end");

  return err;  
}

SdmErrorCode 
SdmMessageStore::Delete(SdmError &err, const SdmString& name)
{
  TNF_PROBE_0 (SdmMessageStore_Delete_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Delete_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Delete(err, name);
  }  

  TNF_PROBE_0 (SdmMessageStore_Delete_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Delete_end");

  return err;  
}



SdmErrorCode 
SdmMessageStore::Rename(SdmError &err, const SdmString &currentname, const SdmString &newname)
{
  TNF_PROBE_0 (SdmMessageStore_Rename_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Rename_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Rename(err, currentname, newname);
  }  

  TNF_PROBE_0 (SdmMessageStore_Rename_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Rename_end");

  return err;  
}  



SdmErrorCode 
SdmMessageStore::Attach(SdmError &err, const SdmToken &token)
{
  TNF_PROBE_0 (SdmMessageStore_Attach_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Attach_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Attach(err, token);
  }  

  TNF_PROBE_0 (SdmMessageStore_Attach_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Attach_end");

  return err;  
}


SdmErrorCode 
SdmMessageStore::Disconnect(SdmError &err)
{
  TNF_PROBE_0 (SdmMessageStore_Disconnect_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Disconnect_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Disconnect(err);
  }  

  TNF_PROBE_0 (SdmMessageStore_Disconnect_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Disconnect_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::Reconnect(SdmError &err)
{
  TNF_PROBE_0 (SdmMessageStore_Reconnect_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_Reconnect_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->Reconnect(err);
  }  

  TNF_PROBE_0 (SdmMessageStore_Reconnect_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_Reconnect_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::CheckForNewMessages(SdmError& err, SdmMessageNumber &r_messages)
{
  TNF_PROBE_0 (SdmMessageStore_CheckForNewMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_CheckForNewMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->CheckForNewMessages(err, r_messages);
  }  

  TNF_PROBE_0 (SdmMessageStore_CheckForNewMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_CheckForNewMessages_end");
  return err;  
}

SdmErrorCode
SdmMessageStore::GetStatus(SdmError& err, SdmMessageStoreStatus &r_status, 
  const SdmMessageStoreStatusFlags storeflags)
{
  TNF_PROBE_0 (SdmMessageStore_GetStatus_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetStatus_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageStoreStatus(err, r_status, storeflags);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetStatus_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetStatus_end");

  return err;  
}

SdmErrorCode 
SdmMessageStore::SaveMessageStoreState(SdmError& err)
{
  TNF_PROBE_0 (SdmMessageStore_SaveMessageStoreState_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SaveMessageStoreState_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SaveMessageStoreState(err);
  }

  TNF_PROBE_0 (SdmMessageStore_SaveMessageStoreState_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SaveMessageStoreState_end");

  return err;  
}
 
SdmErrorCode 
SdmMessageStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message, 
  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SdmMessageFactory_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessage *message = NULL;
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      // need to get a lock because we use the data port to create 
      // the message.  want to make sure any outstanding expunge calls
      // are completed before we get the message.
      
      // scope the following code so we don't hold on to the
      // _MessageListLock at a minimum.
      {
        SdmRWLockEntry listEntry(_MessageListLock, Sdm_False);
        if (!GetMessageAt(err, r_message, msgnum)) {
          message = new SdmMessage(err, *this, msgnum); 
          if (!err && !AddToMessageList(err, message)) {
              r_message = message;
          }
        }
      }
      
      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }
  
  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SdmMessageFactory_end");

  return err;
}     

SdmErrorCode 
SdmMessageStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message) 
{
  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SdmMessageFactory_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessage *message = NULL;
  
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      message = new SdmMessage(err, *this);
      if (!err) {
        SdmRWLockEntry entry(_MessageListLock, Sdm_False);
        if (!AddToMessageList(err, message)) {
          r_message = message;
        }
      }
      
      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }

  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SdmMessageFactory_end");

  return err;
}                     
                    
SdmErrorCode 
SdmMessageStore::SdmMessageFactory(SdmError& err, SdmMessage *&r_message,
   SdmMessage &deriveFrom) 
{
  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SdmMessageFactory_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmMessage *message = NULL;
  
  {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      message = new SdmMessage(err, *this, deriveFrom);
      if (!err) {
        SdmRWLockEntry entry(_MessageListLock, Sdm_False);
        if (!AddToMessageList(err, message)) {
          r_message = message;
        }
      }
      // clean up message if error occurred.  we need to set the _HasSyncLock
      // flag to make sure the message destructor doesn't try to get the sync
      // lock again.
      if (err && message) {
        message->_HasSyncLock = Sdm_True;
        delete message;
      }
    }
  }

  TNF_PROBE_0 (SdmMessageStore_SdmMessageFactory_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SdmMessageFactory_end");

  return err;
}



SdmErrorCode 
SdmMessageStore::CancelPendingOperations(SdmError& err)
{
  TNF_PROBE_0 (SdmMessageStore_CancelPendingOperations_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_CancelPendingOperations_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmRequestQueue* queue = _ParentSession->GetRequestQueueFor(*this);
  assert(queue != NULL); 
  SdmRequestL outstandingRequests;
  
  // flush the queue and get the requests that still have not been
  // processed.
  queue->RemoveAllRequests(err, outstandingRequests);
    
  // call dataport to cancel any pending operation.  this must be
  // a direct call.  we should not try to post a sync or async request
  // to handle this function.
  int st = _DataPort->CancelPendingOperations(err);
  
  // ignore the error if the data port is already closed.  
  if (err == Sdm_EC_Closed) {
    err = Sdm_EC_Success;
  }

  // now cancel each of the requests that was left on the queue.
  int count = outstandingRequests.ElementCount();
  for (int i=0; i<count; i++) {
    _ParentSession->CancelRequest(outstandingRequests[i]);
    // note: don't delete the request.  it will get deleted by the
    // CancelRequest call.
  }

  TNF_PROBE_0 (SdmMessageStore_CancelPendingOperations_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_CancelPendingOperations_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::ExpungeDeletedMessages(SdmError &err, SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_ExpungeDeletedMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ExpungeDeletedMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _ExpungeDeletedMessages(err, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_ExpungeDeletedMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ExpungeDeletedMessages_end");

  return err;  
}

#ifdef INCLUDE_UNUSED_API
         
SdmErrorCode 
SdmMessageStore::ExpungeDeletedMessages_Async(SdmError &err, const SdmServiceFunction& svf, 
                        void* clientData)
{
  TNF_PROBE_0 (SdmMessageStore_ExpungeDeletedMessages_Async_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ExpungeDeletedMessages_Async_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmMessageStore *me = (SdmMessageStore*) this;  
  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);

  SdmRequest* reqst = new SdmRequest(NULL, SdmMessageStore::AOP_ExpungeDeletedMessages, this);
  reqst->SetServiceFunction(svfClone);

  if (me->_CollectorSession->PostRequest(err, me, reqst)) {
    // need to clean up request and arguments;
    // need to clean up request and arguments;
    delete reqst;    
    delete svfClone;
  }

  TNF_PROBE_0 (SdmMessageStore_ExpungeDeletedMessages_Async_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ExpungeDeletedMessages_Async_end");

  return err;
}

#endif


SdmErrorCode
SdmMessageStore::RegisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmMessageStore_RegisterServiceFunction_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_RegisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    SdmServiceFunctionType sft = sfv.GetType();
    
    // the followign service functions are only valid for Connection.
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
  }

  TNF_PROBE_0 (SdmMessageStore_RegisterServiceFunction_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_RegisterServiceFunction_end");

  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmMessageStore::UnregisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmMessageStore_UnregisterServiceFunction_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    SdmServiceFunctionType sft = sfv.GetType();
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }

  TNF_PROBE_0 (SdmMessageStore_UnregisterServiceFunction_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_UnregisterServiceFunction_end");

  return err;
}

#endif

SdmErrorCode
SdmMessageStore::UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmMessageStore_UnregisterServiceFunction_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }

  TNF_PROBE_0 (SdmMessageStore_UnregisterServiceFunction_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_UnregisterServiceFunction_end");

   return err;
}

SdmErrorCode
SdmMessageStore::GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmMessageStore_GetServiceFunction_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  {
    SdmMutexEntry entry(_ServiceListLock);  
    r_serviceFunc = _ServiceFunctionList[sft];
  }

  if (r_serviceFunc == NULL) {
    err = Sdm_EC_ServiceFunctionNotRegistered;
  }

  TNF_PROBE_0 (SdmMessageStore_GetServiceFunction_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetServiceFunction_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::GetFlags(SdmError &err, SdmMessageFlagAbstractFlags &r_flags, 
                          const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetServiceFunction_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetServiceFunction_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageFlags(err, r_flags, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetServiceFunction_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetServiceFunction_end");

  return err;  
}

SdmErrorCode 
SdmMessageStore::GetFlags(SdmError &err, SdmMessageFlagAbstractFlagsL &r_flags,  
  const SdmMessageNumber startmsgnum, const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageFlags(err, r_flags, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetFlags_end");

  return err;  
}

SdmErrorCode 
SdmMessageStore::GetFlags(SdmError &err, SdmMessageFlagAbstractFlagsL &r_flags, 
                          const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageFlags(err, r_flags, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetFlags_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_ClearFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ClearFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_False, flag, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_ClearFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ClearFlags_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
  const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_ClearFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ClearFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_False, flag, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_ClearFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ClearFlags_end");

  return err;
}


#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageStore::ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flag, 
  const SdmMessageNumber startmsgnum, const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_ClearFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ClearFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_False, flag, 
                  startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_ClearFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ClearFlags_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
  const SdmMessageNumber startmsgnum, const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_SetFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SetFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_True, aflags, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_SetFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SetFlags_end");

  return err;
}

#endif

SdmErrorCode 
SdmMessageStore::SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_SetFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SetFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_True, aflags, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_SetFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SetFlags_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags aflags, 
  SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_SetFlags_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_SetFlags_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->SetMessageFlagValues(err, Sdm_True, aflags, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_SetFlags_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_SetFlags_end");

  return err;
}




SdmErrorCode 
SdmMessageStore::GetHeader(SdmError &err, SdmStrStrL &r_hdr,
    const SdmString &hdr,		
    const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeader(err, r_hdr, hdr, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmIntStrL &r_hdrs, 
  const SdmMessageHeaderAbstractFlags header, 
  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_hdrs, header, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

     
SdmErrorCode  
SdmMessageStore::GetHeaders(SdmError &err, SdmIntStrLL &r_copiesof_hdrs, 
  const SdmMessageHeaderAbstractFlags header, 
  const SdmMessageNumber startmsgnum, 
  const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdrs, header, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

SdmErrorCode  
SdmMessageStore::GetHeaders(SdmError &err, SdmIntStrLL &r_copiesof_hdrs,  
  const SdmMessageHeaderAbstractFlags header, 
  const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdrs, header, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

           
#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageStore::GetHeader(SdmError &err,SdmStrStrLL &r_copiesof_hdr,			
    const SdmString &hdr,			
    const SdmMessageNumber startmsgnum,	
    const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeader(err, r_copiesof_hdr, hdr, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::GetHeader(SdmError &err, SdmStrStrLL &r_copiesof_hdr,	
    const SdmString &hdr,		
    const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeader(err, r_copiesof_hdr, hdr, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdr,				
   const SdmMessageHeaderAbstractFlags hdr,
   const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

   err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdr, hdr, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdr,
     const SdmMessageHeaderAbstractFlags hdr,	
     const SdmMessageNumber startmsgnum,		
     const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdr, hdr, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}


SdmErrorCode SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrL &r_hdr,
    const SdmMessageHeaderAbstractFlags hdr,	
    const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_hdr, hdr, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}

       

SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err,SdmStrStrL &r_hdr,			
    const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

   err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_hdr, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdr,		
    const SdmMessageNumber startmsgnum,	
    const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdr, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err,SdmStrStrLL &r_copiesof_hdr,			
    const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdr, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrL &r_hdrs,			
    const SdmStringL &hdrs,		
    const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_hdrs, hdrs, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdrs,		
    const SdmStringL &hdrs,		
    const SdmMessageNumber startmsgnum,	
    const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdrs, hdrs, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::GetHeaders(SdmError &err, SdmStrStrLL &r_copiesof_hdrs,			
    const SdmStringL &hdrs,		
    const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeaders_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeaders_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->GetMessageHeaders(err, r_copiesof_hdrs, hdrs, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeaders_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeaders_end");

  return err;
}


SdmErrorCode  
SdmMessageStore::CopyMessage(SdmError& err, const SdmToken& token,
                                  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_CopyMessage_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_CopyMessage_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->CopyMessage(err, token, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_CopyMessage_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_CopyMessage_end");

  return err;
}
 
SdmErrorCode  
SdmMessageStore::CopyMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumber startmsgnum,
                                  const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_CopyMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_CopyMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->CopyMessages(err, token, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_CopyMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_CopyMessages_end");

  return err;
}

#endif
          
          
SdmErrorCode  
SdmMessageStore::CopyMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumberL& msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_CopyMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_CopyMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->CopyMessages(err, token, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_CopyMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_CopyMessages_end");

  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode  
SdmMessageStore::MoveMessage(SdmError& err, const SdmToken& token,
                                  const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_MoveMessage_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_MoveMessage_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->MoveMessage(err, token, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_MoveMessage_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_MoveMessage_end");

  return err;
}
 
SdmErrorCode  
SdmMessageStore::MoveMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumber startmsgnum,
                                  const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_GetHeader_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetHeader_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->MoveMessages(err, token, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_GetHeader_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetHeader_end");

  return err;
}
          
SdmErrorCode  
SdmMessageStore::MoveMessages(SdmError& err, const SdmToken& token,	
                                  const SdmMessageNumberL& msgnums)                                  
{
  TNF_PROBE_0 (SdmMessageStore_MoveMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_MoveMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->MoveMessages(err, token, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_MoveMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_MoveMessages_end");

  return err;
}

#endif

SdmErrorCode 
SdmMessageStore::AppendMessage(SdmError& err, const SdmMessage& message)
{
  TNF_PROBE_0 (SdmMessageStore_AppendMessage_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_AppendMessage_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmContentBuffer  contents;
  SdmMessage *msg = (SdmMessage*) &message;

  SdmIntStrL hdrs;
  SdmString date;
  if (message._ParentStore->GetHeaders(err, hdrs, Sdm_MHA_P_AbbreviatedReceivedDate,
                                       msg->_MessgNumber) == Sdm_EC_Success) {
    // only expect one received date
    assert(hdrs.ElementCount() == 1);
    date = hdrs[0].GetString();
  }

  SdmStrStrL shdrs;
  SdmString from;
  if (message._ParentStore->GetHeader(err, shdrs, "x-unix-from",
                                       msg->_MessgNumber) == Sdm_EC_Success) {
    // only expect one x-unix-from header
    assert(shdrs.ElementCount() > 0);
    from = shdrs[0].GetSecondString();
  }

  SdmMessageFlagAbstractFlags flags;
  if (!message._ParentStore->GetFlags(err, flags, msg->_MessgNumber)) {
    if (!msg->GetContents(err, contents)) {
      SdmString str_contents;
      if (!contents.GetContents(err, str_contents)) {
        _AppendMessage(err, str_contents, date, from, flags);
      }
    }
  }   


  TNF_PROBE_0 (SdmMessageStore_AppendMessage_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_AppendMessage_end");

  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageStore::AppendMessages(SdmError& err, const SdmMessageL& messages)
{
  TNF_PROBE_0 (SdmMessageStore_AppendMessages_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_AppendMessages_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  SdmError localErr;
  SdmMessage *message = NULL;
  size_t count = messages.ElementCount();
  
  for (size_t i=0; i<count; i++) {
    message = messages[i];
    localErr = Sdm_EC_Success;
    
    // we try to process append all the message even if one failed.
    // therefore, if there is an error in trying to append the message,
    // we save the error if it but continue to try
    // to append the rest of the messages.
    if (AppendMessage(localErr, *message)) {
      err = localErr;
    }
  }

  TNF_PROBE_0 (SdmMessageStore_AppendMessages_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_AppendMessages_end");

  return err;
}

#endif

SdmErrorCode
SdmMessageStore::ScanNamespace(SdmError& err, SdmIntStrL& nameL, const SdmString& ref,
    const SdmString& pat)
{
  TNF_PROBE_0 (SdmMessageStore_ScanNamespace_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ScanNamespace_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->ScanNamespace(err, nameL, ref, pat);
  }  

  TNF_PROBE_0 (SdmMessageStore_ScanNamespace_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ScanNamespace_end");

  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmMessageStore::ScanSubscribedNamespace(SdmError& err, SdmIntStrL& nameL,
					 const SdmString& ref, const SdmString& pat)
{
  TNF_PROBE_0 (SdmMessageStore_ScanSubscribedNamespace_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_ScanSubscribedNamespace_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->ScanSubscribedNamespace(err, nameL, ref, pat);
  }  

  TNF_PROBE_0 (SdmMessageStore_ScanSubscribedNamespace_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_ScanSubscribedNamespace_end");

  return err;
}

SdmErrorCode
SdmMessageStore::AddToSubscribedNamespace(SdmError& err, const SdmString& name)
{
  TNF_PROBE_0 (SdmMessageStore_AddToSubscribedNamespace_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_AddToSubscribedNamespace_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->AddToSubscribedNamespace(err, name);
  }  

  TNF_PROBE_0 (SdmMessageStore_AddToSubscribedNamespace_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_AddToSubscribedNamespace_end");

  return err;
}

SdmErrorCode
SdmMessageStore::RemoveFromSubscribedNamespace(SdmError& err, const SdmString& name)
{
  TNF_PROBE_0 (SdmMessageStore_RemoveFromSubscribedNamespace_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_RemoveFromSubscribedNamespace_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->RemoveFromSubscribedNamespace(err, name);
  }  

  TNF_PROBE_0 (SdmMessageStore_RemoveFromSubscribedNamespace_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_RemoveFromSubscribedNamespace_end");

  return err;
}

#endif

SdmErrorCode 
SdmMessageStore::PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, const SdmSearch &srch)
{
  TNF_PROBE_0 (SdmMessageStore_PerformSearch_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_PerformSearch_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->PerformSearch(err, r_msgnums, srch);
  }  

  TNF_PROBE_0 (SdmMessageStore_PerformSearch_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_PerformSearch_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, const SdmSearch &srch, 
                          const SdmMessageNumber startmsgnum, const SdmMessageNumber endmsgnum)
{
  TNF_PROBE_0 (SdmMessageStore_PerformSearch_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_PerformSearch_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->PerformSearch(err, r_msgnums, srch, startmsgnum, endmsgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_PerformSearch_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_PerformSearch_end");


  return err;
}

SdmErrorCode 
SdmMessageStore::PerformSearch(SdmError &err, SdmMessageNumberL &r_msgnums, const SdmSearch &srch, 
                          const SdmMessageNumberL &msgnums)
{
  TNF_PROBE_0 (SdmMessageStore_PerformSearch_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_PerformSearch_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->PerformSearch(err, r_msgnums, srch, msgnums);
  }  

  TNF_PROBE_0 (SdmMessageStore_PerformSearch_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_PerformSearch_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::PerformSearch(SdmError &err, SdmBoolean &r_bool, const SdmSearch &srch, 
                          const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmMessageStore_PerformSearch_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_PerformSearch_start");

  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->PerformSearch(err, r_bool, srch, msgnum);
  }  

  TNF_PROBE_0 (SdmMessageStore_PerformSearch_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_PerformSearch_end");

  return err;
}


SdmDataPort* 
SdmMessageStore::GetDataPort()
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageStore_GetDataPort_start,
      "api SdmMessageStore",
      "sunw%start SdmMessageStore_GetDataPort_start");

  assert (_DataPort != NULL);

  TNF_PROBE_0 (SdmMessageStore_GetDataPort_end,
      "api SdmMessageStore",
      "sunw%end SdmMessageStore_GetDataPort_end");

  return _DataPort;
}

SdmErrorCode 
SdmMessageStore::_Open(SdmError& err, SdmMessageNumber& r_nmsgs,
                       SdmBoolean& r_readOnly, const SdmToken &token)
{
  TNF_PROBE_0 (SdmMessageStore__Open_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore__Open_start");

  err = Sdm_EC_Success;
  if (_DataPort->Open(err, r_nmsgs, r_readOnly, token) == Sdm_EC_Success) {
    _ReadOnly = r_readOnly;
  }

  TNF_PROBE_1 (SdmMessageStore__Open_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore__Open_end",
      tnf_ulong, r_readOnly, (unsigned long) r_readOnly);

  return err;
}

SdmErrorCode 
SdmMessageStore::_Close(SdmError& err)
{
  TNF_PROBE_0 (SdmMessageStore__Close_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore__Open_start");

  err = Sdm_EC_Success;
  _DataPort->Close(err);

  TNF_PROBE_0 (SdmMessageStore__Close_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore__Close_end");

  return err;
}
          
SdmErrorCode
SdmMessageStore::_ExpungeDeletedMessages(SdmError &err, SdmMessageNumberL &r_msgnums)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageStore__ExpungeDeletedMessages_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore__ExpungeDeletedMessages_start");

  err = Sdm_EC_Success;

  // get a lock on the message list.  we don't want it changing during the
  // expunge.
  SdmRWLockEntry entry(_MessageListLock, Sdm_False);

  // call the data port to do the actual expunge.    
  _DataPort->ExpungeDeletedMessages(err, r_msgnums);

  // we return in the case when the error is due to r_msgnums not being
  // empty.  No expunge could occur in this case.  Note that this is an
  // assumption on the implementation of SdmDataPort::ExpungeDeletedMessages.
  //
  if (err == Sdm_EC_ReturnArgumentListHasContents) {
    TNF_PROBE_0 (SdmMessageStore__ExpungeDeletedMessages_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore__ExpungeDeletedMessages_end");
    return err;
  }

  // make an assertion that no message numbers should be returned if an
  // error occurred.  This should never happen.  Expunge should either
  // be successful in deleting all messages or fail without changing the
  // message store.
  //
  // REVISIT:  What gets returned if a "cancel" is made during an expunge??
  //
  assert (!(err && r_msgnums.ElementCount() != 0));


  // if nothing got deleted, just return.
  int delCount = r_msgnums.ElementCount();
  if (delCount<=0) {
    TNF_PROBE_0 (SdmMessageStore__ExpungeDeletedMessages_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore__ExpungeDeletedMessages_end");
    return err;
  }

  
  //
  // Go through our cache and remove the message from our cache.
  // Then delete each message that is expunged.  Note that it is possible 
  // that we don't have a message object for the given msgnum in our cache 
  // so we don't return an error if we don't find it.
  //
  SdmError localErr;
  SdmMessage *deletedMsg;
  for (int i=0; i< delCount; i++) {
    if (GetMessageAt(localErr, deletedMsg, r_msgnums[i])) {
      RemoveFromMessageList(localErr, deletedMsg);
      // we need to set the expunged flag otherwise the destruction of the
      // message will try to remove the message from the list again.
      deletedMsg->SetExpunged(Sdm_True);
      // we need to set the _HasSyncLock flag otherwise the destruction
      // of the message will try to the sync lock again.
      deletedMsg->_HasSyncLock = Sdm_True;
      delete deletedMsg;
    }
  }
 

  //
  // Now let's renumber the messaages that are left in the cache.
  //
  SdmMessage *message;
  int cacheSize = _MessgList.ElementCount();
  int delMsgIndex = 0, offset = 0;

  for (i=1; i< cacheSize; i++) {

    if (delMsgIndex < delCount && i == r_msgnums[delMsgIndex]) {
      offset++;
      delMsgIndex++;
      //
      // we want to just null out the pointer for the deleted
      // message.  don't delete it.  it is owned by the front end.
      //
      _MessgList[i] = NULL;  

    } else if (offset > 0) {
      message = _MessgList[i];
      if (message) {
        message->SetMessageNumber(localErr, i-offset);
      }
      _MessgList[i-offset] = message;
      _MessgList[i] = NULL;
    }
  }
  if (offset > 0) {
    // shrink the list to free up some space.
    _MessgList.SetVectorSize(cacheSize-offset);
  }

  TNF_PROBE_0 (SdmMessageStore__ExpungeDeletedMessages_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore__ExpungeDeletedMessages_end");

  return err;
}  

SdmErrorCode 
SdmMessageStore::_AppendMessage(SdmError& err, SdmString& contents, SdmString& date,
    SdmString& from, SdmMessageFlagAbstractFlags flags)
{
  TNF_PROBE_0 (SdmMessageStore_AppendMessage_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore_AppendMessage_start");

  err = Sdm_EC_Success;  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    int st = _DataPort->AppendMessage(err, contents, date, from, flags);
  }

  TNF_PROBE_0 (SdmMessageStore_AppendMessage_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_AppendMessage_end");

  return err;
}


SdmErrorCode 
SdmMessageStore::AddToMessageList(SdmError &err, SdmMessage* message)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageStore_AddToMessageList_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore_AddToMessageList_start");

  static const int kBuffer = 20;
  
  err = Sdm_EC_Success;
  if (message == NULL) {
    err = Sdm_EC_Fail;
    TNF_PROBE_0 (SdmMessageStore_AddToMessageList_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_AddToMessageList_end");
    return err;
  }
  
  SdmMessageNumber msgnum = message->_MessgNumber;
  assert (msgnum != 0);
  
  // if message number is positive, then it is a message
  // that exists in the store.  otherwise, a newly created 
  // message has a negative message number.
  if (msgnum > 0) {
    int length = _MessgList.ElementCount();
    if (length <= msgnum) {
      int increment = msgnum+kBuffer;
      _MessgList.SetVectorSize(increment);
      for (int i=length; i< increment; i++) {
        _MessgList[i] = NULL;
      }
    }
    _MessgList[msgnum] = message;

  } else {
    _NewMessgList.AddElementToList(message);
  }
            

  TNF_PROBE_0 (SdmMessageStore_AddToMessageList_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_AddToMessageList_end");

  return err;
}

SdmErrorCode 
SdmMessageStore::RemoveFromMessageList(SdmError &err, SdmMessage* message)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageStore_RemoveFromMessageList_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore_RemoveFromMessageList_start");

  err = Sdm_EC_Success;
  
  // note: it is possible to get a message number that is 0 if the destructor 
  // of a message which encountered an error in construction was deleted.
  // therefore, we should not have an assertion that the message number
  // can not be zero.  If it is zero, let's just ignore it.
  
  if (message && message->_MessgNumber != 0) {
    SdmMessageNumber msgnum  = message->_MessgNumber;
    if (msgnum > 0) {
      // check if message is in existing message list.
      // if found, just null out the pointer.  don't delete it.
      if (msgnum < _MessgList.ElementCount()) {
        _MessgList[msgnum] = NULL;
      } 

    } else {
      // check if message if in new message list.
      // if found, just remove it from the list.  don't delete it.
      int index;
      if ((index = _NewMessgList.FindElement(message)) >= 0) {
        _NewMessgList.RemoveElement(index);
      }
    }
  }

  TNF_PROBE_0 (SdmMessageStore_RemoveFromMessageList_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_RemoveFromMessageList_end");

  return err;
}

SdmBoolean 
SdmMessageStore::GetMessageAt(SdmError &err, SdmMessage *&r_message, 
                          const SdmMessageNumber msgnum)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  TNF_PROBE_0 (SdmMessageStore_GetMessageAt_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore_GetMessageAt_start");

  SdmBoolean retval = Sdm_False;
  err = Sdm_EC_Success;
  r_message = NULL;

  {
    if (msgnum > 0) {
      // check message list to see if it's an existing message that
      // we already have.
      if (msgnum < _MessgList.ElementCount() && _MessgList[msgnum] != NULL) {
        r_message = _MessgList[msgnum];
        retval = Sdm_True;
      } 

    } else {
      // check new message list to see if the message is in there.
      SdmVectorIterator<SdmMessage*> iter(&_NewMessgList);
      SdmMessage **msg;

      while ((msg = iter.NextListElement()) != NULL) {
        assert (*msg != NULL);
        assert ((*msg)->_MessgNumber != 0);

        // break out of loop if we found the message we are looking for.
        if ((*msg)->_MessgNumber == msgnum) {
          r_message = *msg;
          retval = Sdm_True;
          break;
        }
      }
    }
  }

  TNF_PROBE_0 (SdmMessageStore_GetMessageAt_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_GetMessageAt_end");
  return retval;
}
 

void
SdmMessageStore::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event, 
  const SdmBoolean isCancelled)
{
  TNF_PROBE_0 (SdmMessageStore_GetEventForRequest_start,
      "SdmMessageStore",
      "sunw%start SdmMessageStore_GetEventForRequest_start");

  switch(reqst->GetOperator()) {

    case SdmMessageStore::AOP_Open: 
      {  
        SdmArgumentList* args = reqst->GetArguments();
        SdmToken *token = (SdmToken*) args->GetArg(SdmArgumentList::ArgT_Token);
        delete args;
        assert (token != NULL);
                            
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmOpenMessageStore.error = new SdmError;
        r_event->sdmOpenMessageStore.type = Sdm_Event_OpenMessageStore;

        if (!isCancelled) {
          // do the actual open of the message store.
          int st = _Open(*(r_event->sdmOpenMessageStore.error),
                         r_event->sdmOpenMessageStore.nmsgs,
                         r_event->sdmOpenMessageStore.readOnly, *token);
        } else {
          *(r_event->sdmOpenMessageStore.error) = Sdm_EC_Cancel;
        }

        delete token;
        break;
      }

    case SdmMessageStore::AOP_Close: 
      {  
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmCloseMessageStore.error = new SdmError;
        r_event->sdmCloseMessageStore.type = Sdm_Event_CloseMessageStore;

        if (!isCancelled) {
          // do the actual close of the message store.
          int st = _Close(*(r_event->sdmCloseMessageStore.error));
        } else {
          *(r_event->sdmCloseMessageStore.error) = Sdm_EC_Cancel;
        }

        break;
      }
      
    case SdmMessageStore::AOP_ExpungeDeletedMessages:
      {
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmExpungeDeletedMessages.error = new SdmError;
        r_event->sdmExpungeDeletedMessages.type = Sdm_Event_ExpungeDeletedMessages;
        r_event->sdmExpungeDeletedMessages.deletedMessages = new SdmMessageNumberL;

        if (!isCancelled) {
          // do the actual expunging of the deleted messages.
          int st = _ExpungeDeletedMessages(*(r_event->sdmExpungeDeletedMessages.error), 
                              *(r_event->sdmExpungeDeletedMessages.deletedMessages)); 
        } else {
          *(r_event->sdmExpungeDeletedMessages.error) = Sdm_EC_Cancel;
        }
        
        break;
      }
   default:
      {
        SdmPrim::GetEventForRequest(reqst, r_event, isCancelled);
        break;
      }
 }  

  TNF_PROBE_0 (SdmMessageStore_GetEventForRequest_end,
      "SdmMessageStore",
      "sunw%end SdmMessageStore_GetEventForRequest_end");

 return;
}


SdmErrorCode
SdmMessageStore::ShutDownLocalServer(SdmError &err)
{
  err = Sdm_EC_Success;

#if defined(USE_SOLARIS_THREADS)
  SdmRequestQueue* queue = GetServerRequestQueue();
  assert(queue != NULL);  

  sema_t	callerLock;
  if (sema_init(&callerLock, 0, USYNC_THREAD, 0)) {
    err = Sdm_EC_Fail;
    return err;
  }

  // create and post request to ping the queue.
  SdmArgumentList* args = new SdmArgumentList;
  args->PutArg(&callerLock);
  SdmRequest* reqst = new SdmRequest(args, SdmSession::AOP_Ping, this);
  queue->PostRequest(reqst);

  // wait for our turn on the queue.
  sema_wait(&callerLock);
  sema_destroy(&callerLock);
  
#endif 
  
  // queue is now empty.  we can now shutdown the local server.
  _ParentSession->UnRegisterLocalServer(err, this);
  
  // need to reset server queue that was cached by SdmServer.
  ResetServerQueue();
  
  // don't delete request or args.  it will get deleted by ProcessRequest.
    
  return err;
}


void 
SdmMessageStore::SR_ProcessDataPortEvent(void* objThis, SdmDpCallbackType type,
                        SdmDpEvent* response)
{
  ((SdmMessageStore *) objThis)->ProcessDataPortEvent(type, response);
}

// IMPORTANT NOTE!!

// The following code directly calls any error, debug, or notify callbacks.
// It does NOT place the "event" onto a queue and tickle the front-end by
// sending to the pipe registered with XtAppAddInput.  The reason is that
// these notification messages need to be processed immediately in order to
// serve a useful purpose.  Right now the front-end only utilizes the
// "synchronous" open call, and does not return until the open completes.

// When the front-end switches to using asynchronous open, this code should
// be changed back to use the pipe/XtAppAddInput mechanism to deliver these
// events to the front-end.
void 
SdmMessageStore::ProcessDataPortEvent(SdmDpCallbackType dpEventType, SdmDpEvent* dpEvent)
{
  // cout << "SdmMessageStore::ProcessDataPortEvent called.\n";
  SdmServiceFunctionType serviceType;
  assert (dpEvent != NULL);
  
  switch (dpEvent->sdmDpBase.type) {
  case DPEVENT_ErrorLog:
    serviceType = Sdm_SVF_ErrorLog; 
    break;
  case DPEVENT_DebugLog:  
    serviceType = Sdm_SVF_DebugLog; 
    break;
  case DPEVENT_NotifyLog:  
    serviceType = Sdm_SVF_NotifyLog; 
    break;
  case DPEVENT_BusyIndication:  
    serviceType = Sdm_SVF_DataPortBusy; 
    break;
  case DPEVENT_SessionLockGoingAway:
  case DPEVENT_SessionLockTakenAway:
  case DPEVENT_WaitingForSessionLock:
  case DPEVENT_NotWaitingForSessionLock:
  case DPEVENT_WaitingForUpdateLock:
  case DPEVENT_NotWaitingForUpdateLock:
  case DPEVENT_ServerConnectionBroken:
  case DPEVENT_MailboxConnectionReadOnly:
  case DPEVENT_MailboxChangedByOtherUser:
    serviceType = Sdm_SVF_DataPortLockActivity;
    break;
  default: 
    assert(0);  // assert if we encounter unknown event type.
  }

  // find service function to call.
  SdmError err;
  SdmServiceFunction *r_serviceFunc = NULL;
  SdmServiceFunctionType r_serviceFound;
  FindServiceFunction(err, r_serviceFunc, r_serviceFound, serviceType);

  // postOnQueue defines whether we post the log message events on the
  // reply queue.  For the error and notify messages, we need to post
  // these events on the queue if we are using threads and this is not
  // the initial thread.  The XEvent loop runs on the initial thread.
  // The callbacks for these events will make X calls.  We can only run
  // X calls on the main loop.
  //
  SdmBoolean postOnQueue = Sdm_False;
#if defined(USE_SOLARIS_THREADS)
  if (thr_self() != SdmConnection::GetConnection()->GetInitialThread()) {
    postOnQueue = Sdm_True;
  } else {
    postOnQueue = Sdm_False;
  }
#endif 
	

  // if function found, post reply for start of check new mail.
  if (r_serviceFunc) {
    // cout << "SdmMessageStore::ProcessDataPortEvent posting reply.\n";
    SdmEvent *event = new SdmEvent;
    SdmError err1, err2;
    switch (dpEvent->sdmDpBase.type) {
    case DPEVENT_ErrorLog:
      event->sdmErrorLog.type = Sdm_Event_ErrorLog;
      assert (dpEvent->sdmDpErrorLog.errorMessage != NULL);
      event->sdmErrorLog.errorMessage = new SdmString(*(dpEvent->sdmDpErrorLog.errorMessage));
      if (postOnQueue) {
        event->sdmErrorLog.error = new SdmError;
        break;
      } else {
        event->sdmErrorLog.error = &err1;
        // The error service function is special - just call it directly and then return.
        {
          SdmMutexEntry entry(_MessageLogLock);
          r_serviceFunc->Execute(err2, r_serviceFound, event);
        }
        delete event;
        return;
      }
    case DPEVENT_DebugLog:  
      event->sdmDebugLog.type = Sdm_Event_DebugLog;
      assert (dpEvent->sdmDpDebugLog.debugMessage != NULL);
      event->sdmDebugLog.debugMessage = new SdmString(*(dpEvent->sdmDpDebugLog.debugMessage));
      event->sdmDebugLog.error = &err1;
      // The debug service function is special - just call it directly and then return.
      {
        SdmMutexEntry entry(_MessageLogLock);
        r_serviceFunc->Execute(err2, r_serviceFound, event);
      }
      delete event;
      return;

    case DPEVENT_NotifyLog:  
      event->sdmNotifyLog.type = Sdm_Event_NotifyLog;
      assert (dpEvent->sdmDpNotifyLog.notifyMessage != NULL);
      event->sdmNotifyLog.notifyMessage = new SdmString(*(dpEvent->sdmDpNotifyLog.notifyMessage));
      if (postOnQueue) {
        event->sdmErrorLog.error = new SdmError;
        break;
      } else {
        event->sdmNotifyLog.error = &err1;
        // The notify service function is special - just call it directly and then return.
        {
          SdmMutexEntry entry(_MessageLogLock);
          r_serviceFunc->Execute(err2, r_serviceFound, event);
        }
        delete event;
        return;
      }

    case DPEVENT_BusyIndication:  
      event->sdmDebugLog.error = new SdmError;
      break;
    case DPEVENT_SessionLockGoingAway:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmDataPortLockActivity.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_SessionLockGoingAway;
      break;
    case DPEVENT_SessionLockTakenAway:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_SessionLockTakenAway;
      break;
    case DPEVENT_WaitingForSessionLock:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_WaitingForSessionLock;
      break;
    case DPEVENT_NotWaitingForSessionLock:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_NotWaitingForSessionLock;
      break;
    case DPEVENT_WaitingForUpdateLock:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_WaitingForUpdateLock;
      break;
    case DPEVENT_NotWaitingForUpdateLock:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_NotWaitingForUpdateLock;
      break;
    case DPEVENT_ServerConnectionBroken:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_ServerConnectionBroken;
      break;
    case DPEVENT_MailboxConnectionReadOnly:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_MailboxConnectionReadOnly;
      break;
    case DPEVENT_MailboxChangedByOtherUser:
      event->sdmDataPortLockActivity.error = new SdmError;
      event->sdmErrorLog.type = Sdm_Event_DataPortLockActivity;
      event->sdmDataPortLockActivity.lockActivityEvent = SdmDataPortLockActivity::Sdm_DPLA_MailboxChangedByOtherUser;
      break;
    }
    
    SdmReply* reply = new SdmReply;  
    reply->SetOperator(SdmMessageStore::AOP_DataPortEvent);
    reply->SetClientObject(this);
    reply->SetEvent(event);
    reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
    reply->SetServiceFunctionType(r_serviceFound);
    OutCallDispatchPoint(reply);
  }
}

SdmErrorCode 
SdmMessageStore::HandleUnsolicitMailCheck(SdmError &err)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  // cout << "SdmMessageStore::HandleUnsolicitMailCheck called.\n";
  
  if (_DataPort->CheckOpen(err) == Sdm_EC_Success) {
  
    // find service function to call.
    SdmServiceFunction *r_serviceFunc = NULL;
    SdmServiceFunctionType r_serviceFound;
    FindServiceFunction(err, r_serviceFunc, r_serviceFound, Sdm_SVF_CheckNewMail);

    // handle the check new mail only if service function is found. 
    if (r_serviceFunc) {
    
      // create event for start of mail check
      // cout << "SdmMessageStore::HandleUnsolicitMailCheck posting SdmCheckNewMail start.\n";
      SdmEvent *event;
      event = new SdmEvent;
      event->sdmCheckNewMail.error = new SdmError(err);
      event->sdmCheckNewMail.type = Sdm_Event_CheckNewMail;
      event->sdmCheckNewMail.isStart = Sdm_True;
      event->sdmCheckNewMail.numMessages = 0;

      SdmReply *reply;

#if defined(USE_SOLARIS_THREADS)      
      // post reply for start of check new mail.
      reply = new SdmReply;  
      reply->SetOperator(SdmMessageStore::AOP_CheckNewMail);
      reply->SetClientObject(this);
      reply->SetEvent(event);
      reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
      reply->SetServiceFunctionType(r_serviceFound);
      OutCallDispatchPoint(reply);
#else 
      // for single thread case, we call the service function 
      // directly for the start of the event so the front end
      // can post a status message to the user.
      r_serviceFunc->Execute(err, r_serviceFound, event);
      delete event->sdmCheckNewMail.error;
      delete event;
      event = NULL;
#endif

      SdmMessageNumber r_messages;  
      err = Sdm_EC_Success;    // reset error in case it was set earlier.

      // cout << "SdmMessageStore::HandleUnsolicitMailCheck checking for new messages.\n";

      // handle mail check here.
      _DataPort->CheckForNewMessages(err, r_messages);

      // reset error if the dataport got closed or shutdown before we
      // had a chance to do our check.  this is a timing thing where we
      // checked that it was open above but the dataport got closed before
      // our call to CheckForNewMessages.  we still need to send a reply
      // to the front end to let it know that our check has completed but
      // we don't want to return an error if the dataport got closed
      // before our check.
      if (err == Sdm_EC_Shutdown || err == Sdm_EC_Closed) {
        err = Sdm_EC_Success;
        r_messages = 0;
      }

      // create event for end of mail check
      // cout << "SdmMessageStore::HandleUnsolicitMailCheck posting SdmCheckNewMail end.\n";
      event = new SdmEvent;
      event->sdmCheckNewMail.error = new SdmError(err);
      event->sdmCheckNewMail.type = Sdm_Event_CheckNewMail;
      event->sdmCheckNewMail.isStart = Sdm_False;
      event->sdmCheckNewMail.numMessages = r_messages;

      // post reply for end of mail check.
      reply = new SdmReply;  
      reply->SetOperator(SdmMessageStore::AOP_CheckNewMail);
      reply->SetClientObject(this);
      reply->SetEvent(event);
      reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
      reply->SetServiceFunctionType(r_serviceFound);
      OutCallDispatchPoint(reply);
    }
  }
  
  return err;
}

SdmErrorCode         
SdmMessageStore::HandleUnsolicitAutoSave(SdmError &err)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  // cout << "SdmMessageStore::HandleUnsolicitAutoSave called.\n";
  
  if (_DataPort->CheckOpen(err) == Sdm_EC_Success) 
  {
    // find service function to call.
    SdmServiceFunction *r_serviceFunc = NULL;
    SdmServiceFunctionType r_serviceFound;
    FindServiceFunction(err, r_serviceFunc, r_serviceFound, Sdm_SVF_AutoSave);

    // if function found, post reply for start of autosave.
    if (r_serviceFunc) {
      // cout << "SdmMessageStore::HandleUnsolicitAutoSave posting SdmAutoSave start.\n";
      SdmEvent *event;
      event = new SdmEvent;
      event->sdmAutoSave.error = new SdmError(err);
      event->sdmAutoSave.type = Sdm_Event_AutoSave;
      event->sdmAutoSave.isStart = Sdm_True;

      SdmReply* reply;
      
#if defined(USE_SOLARIS_THREADS)    
      reply = new SdmReply;  
      reply->SetOperator(SdmMessageStore::AOP_AutoSave);
      reply->SetClientObject(this);
      reply->SetEvent(event);
      reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
      reply->SetServiceFunctionType(r_serviceFound);
      OutCallDispatchPoint(reply);

#else 
      // for single thread case, we call the service function 
      // directly for the start of the event so the front end
      // can post a status message to the user.
      r_serviceFunc->Execute(err, r_serviceFound, event);
      delete event->sdmAutoSave.error;
      delete event;
      event = NULL;
#endif

      err = Sdm_EC_Success;    // reset error in case it was set earlier.

      // cout << "SdmMessageStore::HandleUnsolicitAutoSave saving message store state.\n";

      // handle autosave here.
      _DataPort->SaveMessageStoreState(err);

      // reset error if the dataport got closed or shutdown before we
      // had a chance to do our autosave.  this is a timing thing where we
      // checked that it was open above but the dataport got closed before
      // our call to SaveMessageStoreState.  we still need to send a reply
      // to the front end to let it know that our check has completed but
      // we don't want to return an error if the dataport got closed
      // before our check.
      if (err == Sdm_EC_Shutdown || err == Sdm_EC_Closed) {
        err = Sdm_EC_Success;
      }

      // cout << "SdmMessageStore::HandleUnsolicitAutoSave posting SdmAutoSave end.\n";
      event = new SdmEvent;
      event->sdmAutoSave.error = new SdmError(err);
      event->sdmAutoSave.type = Sdm_Event_AutoSave;
      event->sdmAutoSave.isStart = Sdm_False;

      reply = new SdmReply;  
      reply->SetOperator(SdmMessageStore::AOP_AutoSave);
      reply->SetClientObject(this);
      reply->SetEvent(event);
      reply->SetServiceFunction(new SdmServiceFunction(*r_serviceFunc));
      reply->SetServiceFunctionType(r_serviceFound);
      OutCallDispatchPoint(reply);
    }
  }
  
  return err;
}




SdmErrorCode         
SdmMessageStore::HandleUnsolicitPing(SdmError &err)
{
  err = Sdm_EC_Success;
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_MessageStoreNotStarted)
  // cout << "SdmMessageStore::HandleUnsolicitPing called.\n";
  
  if (_DataPort->CheckOpen(err) == Sdm_EC_Success) 
  {
    // handle ping here.
    // we just call the message store to get the status.  this should ping the IMAP server.
    SdmMessageStoreStatus r_status;
    _DataPort->PingServer(err);
  }
  
  return err;
}


SdmErrorCode
SdmMessageStore::FindServiceFunctionForType(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
  SdmServiceFunctionType sft)
{
  err = Sdm_EC_Success;
  r_serviceFunc = NULL;
  
  // first try to look for it in the message store.
  if (GetServiceFunction(err, r_serviceFunc, sft) || r_serviceFunc) {
    if (err == Sdm_EC_ServiceFunctionNotRegistered) {
      err = Sdm_EC_Success;
    } else {
      return err;
    }
  } 

  // next try to look for it in the session.
  assert (_ParentSession != NULL);
  if (_ParentSession->GetServiceFunction(err, r_serviceFunc, sft) || r_serviceFunc) {
    if (err == Sdm_EC_ServiceFunctionNotRegistered) {
      err = Sdm_EC_Success;
    } else {
      return err;
    }
  } 

  // finally, try the connection.
  assert (_ParentSession->_ParentConnection != NULL);
  if (_ParentSession->_ParentConnection->GetServiceFunction(err, r_serviceFunc, sft) || r_serviceFunc) {
    if (err == Sdm_EC_ServiceFunctionNotRegistered) {
      err = Sdm_EC_Success;
    } else {
      return err;
    }
  } 
  
  return err;
}
 
SdmErrorCode
SdmMessageStore::FindServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
  SdmServiceFunctionType &r_serviceFound,
  SdmServiceFunctionType serviceToFind)
{
  err = Sdm_EC_Success;
  r_serviceFunc = NULL;
  
  // first try to find the service function for the specific service type.
  if (FindServiceFunctionForType(err, r_serviceFunc, serviceToFind) || r_serviceFunc) {
    if (r_serviceFunc) 
      r_serviceFound = serviceToFind;
    return err;
  }
  
  // if not found, then search for service function that processes any service type.
  if (FindServiceFunctionForType(err, r_serviceFunc, Sdm_SVF_Any) || r_serviceFunc) {
    if (r_serviceFunc) 
      r_serviceFound = Sdm_SVF_Any;
    return err;
  }

  return err;
}



 






