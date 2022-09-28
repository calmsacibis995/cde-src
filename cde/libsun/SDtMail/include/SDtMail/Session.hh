/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Session Class.

#ifndef _SDM_SESSION_H
#define _SDM_SESSION_H

#pragma ident "@(#)Session.hh	1.67 97/06/09 SMI"

#include <thread.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/DoubleLinkedList.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/Server.hh>
#include <synch.h>

class SdmConnection;
class SdmMessageStore;

class SdmRequestQueue;
class SdmReplyQueue;
class SdmDispatcher;
class SdmTickleDispatcher;
class SdmRequest;
class SdmTimedDispatcher;



class SdmSession : public SdmPrim {

public:
  virtual ~SdmSession();

  // Startup and Shutdown methods
  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);
  
  // Parent Access
          SdmErrorCode Parent(SdmError& err, SdmConnection *&r_session);
  
  // Stops all pending operations currently running in this session.
  virtual SdmErrorCode CancelPendingOperations(SdmError &err);
  
  virtual SdmErrorCode SdmMessageStoreFactory(SdmError &err, 
                          SdmMessageStore *&r_mstore);
  
  // Service Function Registration
  SdmErrorCode RegisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
  SdmErrorCode UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft);
  SdmErrorCode GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
                 SdmServiceFunctionType sft);

#ifdef INCLUDE_UNUSED_API
  SdmErrorCode UnregisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
#endif

  SdmSessionType  GetType() const;
  
  SdmErrorCode ResetPollingTasks(SdmError &err);

  typedef enum SdmPollEventType_t {
    SESUETYPE_MailCheck = 0,			
    SESUETYPE_AutoSave,			
    SESUETYPE_Ping
  } SdmPollEventType;


protected:
  enum OperationList {AOP_Ping=500};

  SdmSession(SdmError& err, const SdmClassId classId, SdmConnection& connection,
              SdmSessionType type, SdmBoolean startPollingTasks = Sdm_True);

  // methods for attaching to front end.
  SdmErrorCode     Attach(SdmError &err, int fileDesc, SdmReplyQueue &frontEndQueue);
  SdmBoolean       IsAttached() const { return (SdmBoolean)(_FrontEndEventQueue != NULL); }

  SdmErrorCode     Detach(SdmError &err, int &fileDesc, SdmReplyQueue *&frontEndQueue);
  
  //Posting requests.  
  SdmErrorCode     PostRequest(SdmError &err, SdmPrim* caller, SdmRequest* request); 
  SdmErrorCode     PostReply(SdmError &err, SdmPrim* caller, SdmReply* reply); 
   
  // Local Server Registration.
  SdmErrorCode     RegisterLocalServer(SdmError& err, SdmServer* server);
  SdmErrorCode     UnRegisterLocalServer(SdmError& err, SdmServer* server);
  SdmServer*       MapToLocalServer(SdmError& err, SdmRequestQueue* queue);
  SdmRequestQueue* MapToServerQueue(SdmError& err, SdmServer* server);  
  
  SdmErrorCode     HandleServerDestruction(SdmError& err, SdmServer* server);
  SdmErrorCode     AddMessageStore(SdmError &err, SdmMessageStore* store);
  SdmErrorCode     HandleMessageStoreStartUp(SdmError &err, SdmMessageStore* store);
  SdmErrorCode     HandleMessageStoreShutDown(SdmError &err, SdmMessageStore* store);

  // Queue Access
  SdmRequestQueue* GetRequestQueueFor(SdmPrim& caller); 
  SdmRequestQueue* GetRequestQueue();
  SdmReplyQueue*   GetReplyQueue();

  void             CancelRequest(const SdmRequest* request);

 
private:
  // Request/Reply queue handling methods.
  static  void SR_ProcessRequest(SdmPrim* objThis, void* callData);
  static  void SR_ProcessReply(SdmPrim* objThis, void* callData);
          void ProcessRequest(void* callData);
          void ProcessReply(void* callData);
          void HandleRequest(SdmRequest *request);
          void HandleReply(SdmReply *reply);

  // AutoSave, Mail Check, and Ping handling methods.
  static  void SR_ProcessMailCheck(SdmPrim* objThis, void* callData);
  static  void SR_ProcessAutoSave(SdmPrim* objThis, void* callData);
  static  void SR_ProcessPing(SdmPrim* objThis, void* callData);
          void ProcessMailCheck(void* callData);
          void ProcessAutoSave(void* callData);
          void ProcessPing(void* callData);         

  SdmErrorCode CheckInactivityTime(SdmError &err, time_t &r_timeRemaining, time_t &r_now);
  time_t       GetInterval(const char* varName, const SdmBoolean varInMinutes, 
                  const time_t defaultValue, const time_t minValue, 
                  const time_t maxValue = 0);
  time_t       GetInterval(SdmPollEventType type);
  
  SdmErrorCode ShutDownMasterRequestDispatcher(SdmError &err);
  SdmErrorCode ShutDownMasterReplyDispatcher(SdmError &err);

  SdmErrorCode StartPollingTasks(SdmError &err);
  SdmErrorCode StopPollingTasks(SdmError &err);
  SdmErrorCode SetupPollingTasks(SdmError &err);
  

private: 
  // Disallow creating copies of SdmSession.
  SdmSession(const SdmSession& copy);
  SdmSession& operator=(const SdmSession &rhs);

  // Dispatchers for handline queues and unsolicited events.
  SdmTickleDispatcher* _MasterRequestDisp;
  SdmTickleDispatcher* _MasterReplyDisp;
  SdmTimedDispatcher*  _MailCheckDisp;
  SdmTimedDispatcher*  _AutoSaveDisp;
  SdmTimedDispatcher*  _PingDisp;

  // Queues
  SdmRequestQueue     *_MasterRequestCollector;
  SdmReplyQueue       *_MasterReplyCollector;
  SdmReplyQueue       *_FrontEndEventQueue;
  
  // file descriptor used to tickle the front end when things get
  // added to the front end event queue.
  int _WriteFileDesc;
  
  // type of session
  SdmSessionType  _SessionType;
  
  // Both these lists are Index Synchronized, Size Assertion Required
  SdmDoubleLinkedList   _LocalServerList;
  SdmDoubleLinkedList   _ServerWiseQueueList;
  SdmDoubleLinkedList   _ServerWiseDispatcherList;

  // lock for exclusive access to data.
  mutex_t             _LocalServerDataLock;   
  mutex_t             _ServiceListLock;   
  mutex_t             _StartedMessgStoreListLock;   
  
  // parent connection
  SdmConnection*      _ParentConnection;
  
  // service functions registered in this session.
  SdmPtrVector<SdmServiceFunction*> _ServiceFunctionList;

  // list of message stores and message stores which have been started.
  SdmLinkedList _MessgStoreList;
  SdmLinkedList _StartedMessgStoreList;
  
  // boolean indicates whether the session should start up the polling
  // tasks/threads (eg. check new mail, autosave, ping)
  SdmBoolean    _DoPollingTasks;
    
  // constants that define the min, max, and default for event intervals.
  static const time_t kMinMailCheckInterval;
  static const time_t kMinAutoSaveInterval;
  static const time_t kMaxAutoSaveInterval;
  static const time_t kDefaultMailCheckInterval;
  static const time_t kDefaultAutoSaveInterval;
  static const time_t kDefaultInactivityInterval;
  static const time_t kMinInactivityInterval;
  static const time_t kMaxInactivityInterval;
  static const time_t kPingInterval;
  

  // structure used to store the information used for a time out.
  // this is only used in the case on single thread access to mid-end.
  typedef struct {
    void*                             _appContext;
    SdmSession*                       _session;
    unsigned long                     _intervalInSecs;
    SdmSession::SdmPollEventType      _event;
    unsigned long                     _id;
    SdmBoolean                        _isStarted;
  } SdmTimeOutInfo;
  
  static const unsigned short SdmD_TimeOutDataSet_Size;

  // list of time out information for each poll event.
  SdmPtrVector<SdmTimeOutInfo*>       _TimeOutDataList;



  // make SdmMessageStore a friend so it can call CancelRequest.
  friend class SdmMessageStore;
  friend class SdmOutgoingStore;
  
  // server class needs access to Local Server methods.
  friend class SdmServer;
  friend class SdmPrim;
  
  // the following classes need to be a friend to call PostRequest.
  friend class SdmSyncRequestEntry;
  friend class SdmData;
  friend class SdmMessage;
  friend class SdmMessageBody;
  friend class SdmMessageEnvelope;
  friend class SdmOutgoingMessage;
  
  friend class SdmSessionTest;    // for testing.
  friend class SdmMessageStoreTest;
  friend class SdmUnsolicitEventTest;

  // global functions used to handle session attach and timeouts.
  friend SdmErrorCode SdmAttachSession(SdmError &err, int &r_pipe, 
                          void *&r_calldata, SdmSession &session);
  friend SdmErrorCode SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata, 
                          SdmSession &session, void* appContext);
  friend int          SdmHandleTimeOutInterval(SdmSession *ses, 
                          SdmPollEventType event, unsigned long &nextInterval);
  friend SdmErrorCode SdmXtAddSessionTimeOuts(SdmError &err, SdmSession &ses);
  friend SdmErrorCode SdmXtRemoveSessionTimeOuts(SdmError &err, SdmSession &ses);
};


//
// global functions for handling session linkage to the caller.
//
extern SdmErrorCode SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata, SdmSession &ses);
extern int          SdmHandleSessionEvent(void *calldata, int *pipe, unsigned long *id);


#endif




