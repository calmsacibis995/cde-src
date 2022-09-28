/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the expunging of deleted messages calls.
 
#include <unistd.h>
#include <stropts.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Main.hh"

#include <SDtMail/Connection.hh>
#include <SDtMail/Session.hh>
#include <Manager/ISess.hh>
#include <Manager/OSess.hh>
#include <SDtMail/MessageStore.hh>
#include <DataStore/OutgoingStore.hh>
#include <SDtMail/Token.hh>
#include <SDtMail/Message.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/ReplyQueue.hh>


static  const char* kOutgoingStoreName = "/tmp/session.test";

class SdmSessionTest
{
public:
     SdmSessionTest();
     virtual ~SdmSessionTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode Setup(SdmConnection *&);

     SdmErrorCode TestStartup(SdmSession*);
     SdmErrorCode TestShutdown(SdmSession*, SdmMessageStore*);
     SdmErrorCode TestServiceFunctionCalls(SdmSession*);
     SdmErrorCode SetupMessageStore(SdmSession*, SdmMessageStore*&, SdmToken &token);
     SdmErrorCode TestParent(SdmSession *, SdmConnection *);
     SdmErrorCode TestAttach(SdmSession *,
                     int &readFileDesc, SdmReplyQueue *&frontEndQueue);
     SdmErrorCode TestDetach(SdmSession *,
                     int &readFileDesc, SdmReplyQueue *&frontEndQueue);
     SdmErrorCode TestQueues(SdmSession *session,  SdmMessageStore* mstore,
                     SdmBoolean isRegistered);
     SdmErrorCode TestAsyncAndSyncCalls(SdmSession *, SdmMessageStore*,
                     int readFileDesc, SdmReplyQueue *frontEndQueue, SdmBoolean createNewMsg);
                          
  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void AsyncCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);

private:

  SdmSession *ises, *oses;
  int iReadFileDesc, oReadFileDesc;
  SdmReplyQueue *oFrontEndQueue, *iFrontEndQueue;
  
  // the following need to be static because they are used by
  // the callback functions.
  static SdmBoolean asyncCallbackCalled;
  static SdmBoolean sessionCallbackCalled;
  static int lastStartOffS;
  static SdmBoolean asyncCallbackError;
};

const char* SdmSessionTest::kUserData = "session test data";
SdmBoolean SdmSessionTest::asyncCallbackCalled;
SdmBoolean SdmSessionTest::sessionCallbackCalled;
int SdmSessionTest::lastStartOffS;
SdmBoolean SdmSessionTest::asyncCallbackError;



SdmErrorCode RunSessionTest()
{
  cout << endl << "************ RunSessionTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmSessionTest test;
  return test.Run();
}
  

SdmSessionTest::SdmSessionTest()
{
}

SdmSessionTest::~SdmSessionTest()
{
}

SdmErrorCode 
SdmSessionTest::Run()
{
  SdmError err;
  SdmMessageStore *mbox, *dbox;
  SdmMailRc *mailrc;
  SdmConnection *mcon;
  SdmToken token;
  
  cout << endl << "************ SdmSessionTest::Run Invoked. *************" << endl << endl;
  cout << endl;
    
  SdmErrorCode errorCode;
  
  if ((errorCode = Setup(mcon)) != Sdm_EC_Success)
    return errorCode;



  // ***************************************************************8
  // Incoming Session Test....
  // ***************************************************************8

  cout << "Testing incoming session.\n";
  
  if ((errorCode = TestStartup(ises)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestServiceFunctionCalls(ises)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestParent(ises, mcon)) != Sdm_EC_Success)
    return errorCode;



  if ((errorCode = TestAttach(ises, iReadFileDesc, iFrontEndQueue)) != Sdm_EC_Success)
    return errorCode;

  token.SetValue("serviceclass","messagestore");
  token.SetValue("servicetype","test");
  token.SetValue("serviceoption","messagestorename","test.mbx");

  if ((errorCode = SetupMessageStore(ises, mbox, token)) != Sdm_EC_Success)
    return errorCode;

  
  if ((errorCode = TestAsyncAndSyncCalls(ises, mbox, iReadFileDesc, iFrontEndQueue, Sdm_False)) != Sdm_EC_Success)
    return errorCode;

  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // 
  // this call doesn't really do anything because there are no 
  // operations pending.  More extensive testing of the cancel
  // function is in the MessageStoreTest.  This call is strictly 
  // for exercising the function.
  if (ises->CancelPendingOperations(err)) {
    cout << "*** Error: Cancel pending operations Failed\n" ;    
    return err;
  }

  // run this test before shut down.
  if ((errorCode = TestDetach(ises, iReadFileDesc, iFrontEndQueue)) != Sdm_EC_Success)
    return errorCode;


  // run the shutdown test at the end because the other tests will fail
  // if the session is shut down.    
  if ((errorCode = TestShutdown(ises, mbox)) != Sdm_EC_Success)
    return errorCode;

  // ***************************************************************8
  // Outgoing Session Test....
  // ***************************************************************8


  cout << "Testing outgoing session.\n";
  
  if ((errorCode = TestStartup(oses)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestServiceFunctionCalls(oses)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestParent(oses, mcon)) != Sdm_EC_Success)
    return errorCode;
    

  if ((errorCode = TestAttach(oses, oReadFileDesc, oFrontEndQueue)) != Sdm_EC_Success)
    return errorCode;

  SdmToken outtoken;
  outtoken.SetValue("serviceclass","messagestore");
  outtoken.SetValue("servicetype","local");
  outtoken.SetValue("serviceoption","messagestorename", kOutgoingStoreName);
  outtoken.SetValue("serviceoption","disconnected");
  if ((errorCode = SetupMessageStore(oses, dbox, outtoken)) != Sdm_EC_Success)
    return errorCode;

  SdmMessageStore *tmpStore = NULL;
  if ((errorCode = oses->SdmMessageStoreFactory(err, tmpStore)) != Sdm_EC_Success) {
    cout << "*** Error: 2nd call to SdmMessageStore fails for outgoing store.\n";
    return errorCode;
  }
  if (tmpStore == dbox) {
    err = Sdm_EC_Fail;
    cout << "*** Error: 2nd call to SdmMessageStore returns same outgoing store.\n";
    return errorCode;
  }

  if ((errorCode = TestAsyncAndSyncCalls(oses, dbox, oReadFileDesc, oFrontEndQueue, Sdm_False)) != Sdm_EC_Success)
    return errorCode;

  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 
  // 
  // this call doesn't really do anything because there are no 
  // operations pending.  More extensive testing of the cancel
  // function is in the MessageStoreTest.  This call is strictly 
  // for exercising the function.
  if (oses->CancelPendingOperations(err)) {
    cout << "*** Error: Cancel pending operations Failed\n" ;    
    return err;
  }
  
  // run this test before shut down.
  if ((errorCode = TestDetach(oses, oReadFileDesc, oFrontEndQueue)) != Sdm_EC_Success)
    return errorCode;

  // run the shutdown test last because the other tests will fail
  // if the session is shut down.    
  if ((errorCode = TestShutdown(oses, dbox)) != Sdm_EC_Success)
    return errorCode;


  delete mcon;

  // Sessions should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(iFrontEndQueue, &iReadFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }

  if (SdmHandleSessionEvent(oFrontEndQueue, &oReadFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }

  // clean up temp outgoing store.
  unlink(kOutgoingStoreName);
        
  return Sdm_EC_Success;
}

SdmErrorCode 
SdmSessionTest::Setup(SdmConnection *&mcon)
{
  SdmError err;

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp outstore.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // create and start up connection.
  mcon  = new SdmConnection("TestSession");

  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    return err;
  }
  
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }
  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, ises, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed\n";
    return err;
  }
  
  // create and start up outgoing session.
  if (mcon->SdmSessionFactory(err, oses, Sdm_ST_OutSession)) {
    cout << "*** Error: create outgoing session failed\n";
    return err;
  }
  return err;
}  
  

     
SdmErrorCode 
SdmSessionTest::TestStartup(SdmSession *session)
{
  SdmError err;
  
  if (session->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }
  
  // try to start up session again.  this should result in error.
  if (session->StartUp(err)) {
    if (err != Sdm_EC_SessionAlreadyStarted) {
      cout << "*** Error: startup function returns incorrect error code\n";
      return err;
    }else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: startup function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

    


    
SdmErrorCode 
SdmSessionTest::TestServiceFunctionCalls(SdmSession *session)
{
  SdmError err;
  SdmServiceFunction *svfP;
  
  SdmServiceFunction svf(&SessionCallback, 
          (void*)SdmSessionTest::kUserData, 
          Sdm_SVF_Other);

  if (session->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  if (session->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
    cout << "*** Error: get service function failed\n";
    return err;
  }
  if (svfP == &svf) {
    cout << "*** Error: copy of service func not stored in connection.\n";
    return err;
  }
  if (svfP->GetType() != svf.GetType()) {
    cout << "*** Error: get service function returns incorrect result\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (session->UnregisterServiceFunction(err, Sdm_SVF_Other)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  if (session->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
    if (err != Sdm_EC_ServiceFunctionNotRegistered) {
      cout << "*** Error: get service function failed\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: get service function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (svfP != NULL) {
    cout << "*** Error: get service function returns result after function is unregistered\n";
    err = Sdm_EC_Fail;
    return err;
  }
  // test error code returned when trying to unregister the service
  // function again.
  if (session->UnregisterServiceFunction(err, Sdm_SVF_Other)) {
    if (err != Sdm_EC_ServiceFunctionNotRegistered) {
      cout << "*** Error: unregister service function returns incorrect error code\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: unregister service function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }


  if (session->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  // test error code returned when trying to register 
  // the service function again.
  if (session->RegisterServiceFunction(err, svf)) {
    if (err != Sdm_EC_ServiceFunctionAlreadyRegistered) {
      cout << "*** Error: register service function returns returns incorrect error code.\n";
      return err;
    }else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: register service function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (session->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
    cout << "*** Error: get service function failed\n";
    return err;
  }
  if (svfP == &svf) {
    cout << "*** Error: copy of service func not stored in connection.\n";
    return err;
  }
  if (svfP->GetType() != svf.GetType()) {
    cout << "*** Error: get service function returns incorrect result\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (session->UnregisterServiceFunction(err, svf)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  // test error code returned when trying to unregister the service
  // function again.
  if (session->UnregisterServiceFunction(err, svf)) {
    if (err != Sdm_EC_ServiceFunctionNotRegistered) {
      cout << "*** Error: unregister service function returns incorrect error code\n";
      return err;
    }else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: unregister service function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (session->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
    if (err != Sdm_EC_ServiceFunctionNotRegistered) {
      cout << "*** Error: get service function failed\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: get service function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (svfP != NULL) {
    cout << "*** Error: get service function returns result after function is unregistered\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  return err;
}


     
SdmErrorCode 
SdmSessionTest::SetupMessageStore(SdmSession *session, 
  SdmMessageStore *&mstore, SdmToken &token)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;
  
  // create and start up message store.
  if (session->SdmMessageStoreFactory(err, mstore)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (mstore->StartUp(err)) {
      cout << "*** Error: Startup message store failed\n";
      return err;
  }
  if (mstore->Open(err, nmsgs, readOnly, token)) {
      cout << "*** Error: Open message store failed\n";
      return err;
  }
  return Sdm_EC_Success;
}

     
SdmErrorCode 
SdmSessionTest::TestShutdown(SdmSession *session, SdmMessageStore *mstore)
{
  SdmError err;

  if (session->ShutDown(err)) {
    cout << "*** Error: Session shutdown failed.\n";
    return err;
  }
  
  if (session->IsStarted() != Sdm_False) {
    cout << "*** Error: session IsStarted flag still set after shutdown.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 
  // try to shutdown session again.  this should result in error.
  if (session->ShutDown(err)) {
    if (err != Sdm_EC_SessionNotStarted) {
      cout << "*** Error: shutdown function returns incorrect error code\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: shutdown function does not return error.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  return err;
}

SdmErrorCode
SdmSessionTest::TestParent(SdmSession *session, SdmConnection *mcon)
{
  SdmConnection *parent;
  SdmError err;
  if (session->Parent(err, parent)) {
    cout << "*** Error: Parent fails.\n";
    return err;
  }

  if (parent != mcon) {
    cout << "Error: Parent returns wrong connection.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}


SdmErrorCode
SdmSessionTest::TestAttach(SdmSession *session,
  int &readFileDesc, SdmReplyQueue *&frontEndQueue)
{
  SdmError err;
  
  
  // check that attach state is false by default.
  if (session->IsAttached()) {
    cout << "*** Error: session is attached by default.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // make sure that we are not allowed to post requests if 
  // we are not attached.  Need to be attached in order to handle
  // the replies from the requests.
  if (session->PostRequest(err, NULL, NULL)) {
    if (err != Sdm_EC_SessionNotAttached) {
      cout << "*** Error: PostRequest returns incorrect error code .\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  }

  void *vptr;
  
  // do the actual attach here.
  if (SdmAttachSession(err, readFileDesc, vptr, *session)) {
    cout << "*** Error: attach session failed.\n";
    return err;
  }
  frontEndQueue = (SdmReplyQueue*)vptr;
  
  // check that attach state is set in session.
  if (!session->IsAttached()) {
    cout << "*** Error: session not attached after call to SdmAttachSession.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // test that error returned when trying to attach again.
  if (SdmAttachSession(err, readFileDesc, vptr, *session)) {
    if (err != Sdm_EC_SessionAlreadyAttached) {
      cout << "*** Error: SdmAttachSession returns incorrect error code .\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  }
  return err;
}


SdmErrorCode
SdmSessionTest::TestDetach(SdmSession *session,
  int &readFileDesc, SdmReplyQueue *&frontEndQueue)
{
  SdmError err;
  
  
  // check that attach state is false by default.
  if (!session->IsAttached()) {
    cout << "*** Error: session is not attached.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  SdmReplyQueue *replyQueue;
  int writeFileDesc;
  
  // do the actual detach here.
  if (session->Detach(err, writeFileDesc, replyQueue)) {
    cout << "*** Error: detach session failed.\n";
    return err;
  }
  
  if (replyQueue != frontEndQueue) {
    cout << "*** Error: detach returns wrong front end queue.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 
  // check that attach state is not set in session.
  if (session->IsAttached()) {
    cout << "*** Error: session still attached after call to Detach.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  

  // make sure that we can no longer post requests.
  if (session->PostRequest(err, NULL, NULL)) {
    if (err != Sdm_EC_SessionNotAttached) {
      cout << "*** Error: PostRequest returns incorrect error code .\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  }

  // test calling Detach again.
  if (session->Detach(err, writeFileDesc, replyQueue)) {
    if (err != Sdm_EC_SessionNotAttached) {
      cout << "*** Error: SdmAttachSession returns incorrect error code .\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  }

  // Attach back to session so ShutDown can clean up write file descriptor.
  if (session->Attach(err, writeFileDesc,  *replyQueue)) {
    cout << "*** Error: Reattach fails.\n";
    return err;
  }
  
  return err;
}




SdmErrorCode
SdmSessionTest::TestQueues(SdmSession *session, 
  SdmMessageStore* mstore, SdmBoolean isRegistered)
{
  SdmError err;
  err = Sdm_EC_Success;
  
  // check to see that request and reply queues are created for session.
  if (session->GetRequestQueue() == NULL) {
    cout << "*** Error: request queue is null.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (session->GetReplyQueue() == NULL) {
    cout << "*** Error: reply  queue is null.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // check that the request queues are properly set up.
  SdmRequestQueue *queue = session->GetRequestQueueFor(*mstore);
  if (queue == NULL) {
    cout << "*** Error: no request queue assigned to store.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // next, check to see if the proper queues are used for the message
  // store based on whether the store is a registered local server.
  if (isRegistered) {
    SdmRequestQueue *serverQueue = session->MapToServerQueue(err, mstore);
    if (err) {
      cout << "*** Error: MapToServerQueue fails.\n";
      return err;
    } 
    if (serverQueue != queue) {
      cout << "*** Error: server queue not used for message store.\n";
      err = Sdm_EC_Fail;
      return err;
    }
    SdmMessageStore *tmpStore = (SdmMessageStore*) session->MapToLocalServer(err, serverQueue);
    if (err) {
      cout << "*** Error: MapToServerQueue fails.\n";
      return err;
    } 
    if (tmpStore != mstore) {
      cout << "*** Error: MapToServerQueue returns incorrect server.\n";
      err = Sdm_EC_Fail;
      return err;
    }
  } else {
    if (session->GetRequestQueue() != queue) {
      cout << "*** Error: session's request queue not used for message store.\n";
      err = Sdm_EC_Fail;
      return err;
    }
  }      
  return err;
}

SdmErrorCode
SdmSessionTest::TestAsyncAndSyncCalls(SdmSession *session, 
  SdmMessageStore* mstore, int readFileDesc, SdmReplyQueue *frontEndQueue,
  SdmBoolean createNewMsg)
{
  SdmError err;
  static const int kTotalAsyncCalls = 10;
  sessionCallbackCalled = Sdm_False;
  asyncCallbackCalled = Sdm_False;
  asyncCallbackError = Sdm_False;
  lastStartOffS = -1;
 
  SdmMessage *msg;
  if (!createNewMsg) {
    if (mstore->SdmMessageFactory(err, msg, 1)) {
      cout << "*** Error: SdmMessageFactory1 fails.\n";
      return err;
    }
  } else {
    if (mstore->SdmMessageFactory(err, msg)) {
      cout << "*** Error: SdmMessageFactory2 fails.\n";
      return err;
    }
  }
    
  // Check that the queue used for the message is the same as the store.
  if (session->GetRequestQueueFor(*mstore) != 
      session->GetRequestQueueFor(*msg)) 
  {
    cout << "*** Error: mstore and msg doesn't use same request queue.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  //-----------------------------
  SdmServiceFunction svf(&AsyncCallback, (void*)kUserData, Sdm_SVF_Other);
 
  // make multiple asynchronous calls.  the results will be stuck on the
  // front end queue.
  for (int i=0; i< kTotalAsyncCalls; i++) { 
    if (msg->GetContents_Async(err, svf, (void*)kUserData, i, 888)) {  // test async call.
      cout << "*** Error: SdmMessageStore::GetContents_Async failed.\n";
      return err;
    }
  }
  
  // make single synchronous call to make sure all the requests have
  // been processed.
  SdmContentBuffer contents;
  if (msg->GetContents(err, contents, i, 888)) {  // test async call.
    cout << "*** Error: SdmMessageStore::GetContents_Async failed.\n";
    return err;
  }
  
  // the request queue should be empty now and the front end queue should
  // have all the events from the async calls ONLY.  Check these queues.
  SdmRequestQueue *requestQueue = session->GetRequestQueueFor(*mstore);
  if (requestQueue == NULL) {
    cout << "*** Error: no request found for message store.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (requestQueue->ElementCount() != 0) {
    cout << "*** Error: request queues contains requests.\n";
    err = Sdm_EC_Fail;
    return err;
  }


  struct pollfd pollFileDesc;
  pollFileDesc.fd = readFileDesc;
  pollFileDesc.events = POLLIN;
  int rc;
  char buffer[2000];
 
  // go through front end queue and handle each of the events.
  for (i =0; i >= 0 ; i++) {
    if ((rc = poll(&pollFileDesc, 1, 3000)) > 0) {
      if (SdmHandleSessionEvent(frontEndQueue, &readFileDesc, NULL) < 0) {
        sprintf(buffer, "*** SdmHandleSessionEvent failed after processing %d events.\n", i);
        cout << buffer;
        err = Sdm_EC_Fail;
	break;
      }
    } else if (rc == 0) {
        sprintf(buffer, "*** Poll timed out after processing %d events.\n", i);
        cout << buffer;
        sprintf(buffer, "*** front queue element count = %d\n", frontEndQueue->ElementCount());
        cout << buffer;
        break;
    } else {
      err = Sdm_EC_Fail;
      cout << "*** Error: Poll failed.\n";
      return err;
    }
  }
  
  // should have kTotalAsyncCalls events from the session.  
  if (i != kTotalAsyncCalls) {
      err = Sdm_EC_Fail;
      sprintf(buffer, "*** Error: Poll timed out after processing %d events.\n", i);
      cout << buffer;
      return err;
  }
  
  // The following flags are set in the callback itself.
  if (!asyncCallbackCalled || asyncCallbackError) {
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (lastStartOffS != kTotalAsyncCalls-1) {
    cout << "*** Error: AsyncCallback called too many times.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}  

//
// Callbacks
//
void 
SdmSessionTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmSessionTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
  
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }

}
 
void 
SdmSessionTest::AsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  asyncCallbackCalled = Sdm_True;
 
  cout << "AsyncCallback Called.\n";
  if (userdata != SdmSessionTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    asyncCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_GetPartialContents:
      {
        char buffer[2000];
        int stOffS = event->sdmGetContents.startOffset;
        int length = event->sdmGetContents.length;

        SdmString str_contents;
        if (event->sdmGetContents.buffer->GetContents(err, str_contents)) {
           cout << "*** Error: can't get string from content buffer.\n";
           asyncCallbackError = Sdm_True;
        } 
        
        sprintf(buffer, "AsyncTestCallback called with args: startOffset=%d; length=%d; buffer=%s\n",
          stOffS, length, (const char *)str_contents);
        cerr << buffer;
          
        if (stOffS != lastStartOffS+1) {
            sprintf(buffer, 
                "*** Error: async calls are out of order. expected=%d, actual=%d\n",
                lastStartOffS+1, stOffS);
            cout << buffer;
            asyncCallbackError = Sdm_True;
        } else {
          lastStartOffS = stOffS;
        }

        delete (event->sdmGetContents.buffer);
        break;
      }
 
    default:
      cout << "*** Error: AyncCallback default event received.\n";
      asyncCallbackError = Sdm_True;
      break;
  }
}
 
