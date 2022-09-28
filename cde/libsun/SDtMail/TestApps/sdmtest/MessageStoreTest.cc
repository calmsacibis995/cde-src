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
#include <SDtMail/MessageBody.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <unistd.h>
#include <stdlib.h>

static  const char* kNewStoreName = "/tmp/NewStoreName.test";
static  const char* kRenameStoreName = "/tmp/RenameStoreName.test";
static  const char* kOutgoingStoreName = "/tmp/msgstore.test";
static  const char* kSourceStoreName = "/tmp/sourcestore.test";
static  const char* kDestinationStoreName = "/tmp/destinationstore.test";

class SdmMessageStoreTest
{
public:
     SdmMessageStoreTest();
     virtual ~SdmMessageStoreTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode Setup(SdmConnection *&, SdmSession *&ises, 
                     SdmSession *&oses);

     SdmErrorCode TestParent(SdmMessageStore *, SdmSession *);
     SdmErrorCode TestStartup(SdmSession*, SdmMessageStore*, SdmBoolean);
     SdmErrorCode TestShutdown(SdmSession*, SdmMessageStore*, SdmBoolean);
     SdmErrorCode TestServiceFunctionCalls(SdmMessageStore*);
     SdmErrorCode TestOpenAndClose(SdmMessageStore*, SdmToken&, 
                     SdmReplyQueue*, int readfd);
     SdmErrorCode TestFlagMethods(SdmMessageStore *mstore);
     SdmErrorCode TestHeaderMethods(SdmMessageStore *mstore);
     SdmErrorCode TestMessageFactory(SdmMessageStore *mstore, SdmMessage *&, SdmMessage *&);
     SdmErrorCode TestExpunge(SdmMessageStore *mstore, SdmMessage *&, SdmMessage *&,
                     SdmReplyQueue *frontEndQueue, int readFileDesc);
     SdmErrorCode TestCancelPendingOps(SdmMessageStore *mstore, SdmMessage*,
                     SdmReplyQueue *frontEndQueue, int readFileDesc);
     SdmErrorCode TestCreateAndDelete(SdmSession* ses);               
     SdmErrorCode TestNameSpaceCalls(SdmMessageStore* store);   
     SdmErrorCode TestAppendMessage(SdmSession *ses);     
     SdmErrorCode TestCopyAndMove(SdmSession *ses);       
      
     SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
     SdmErrorCode CheckMessageCount(SdmError &err, SdmMessageStore* store, 
                      const SdmMessageNumber expectedMsgs, 
                      const SdmMessageNumber expectedNewMsgs);
                                         
  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void AsyncCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void OpenCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void CloseCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void ExpungeCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);

private:

  SdmMessageStore *mbox, *dbox;
  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
  
  static SdmString kServiceOption;
  static SdmString kServiceType;
  static SdmString kServiceClass;
  
  
  // the following need to be static because they are used by
  // the callback functions.
  static SdmBoolean asyncCallbackCalled;
  static SdmBoolean sessionCallbackCalled;
  static SdmBoolean openCallbackCalled;
  static SdmBoolean closeCallbackCalled;
  static SdmBoolean expungeCallbackCalled;
  static SdmToken lastOpenToken;
  static SdmBoolean asyncCallbackError;
  static SdmBoolean openCallbackError;
  static SdmBoolean closeCallbackError;
  static SdmBoolean expungeCallbackError;
  static SdmMessageStore *lastMessageStore;
  
  static SdmLongL cancelledAsyncCalls;
  static SdmLongL successAsyncCalls;
  
  static SdmErrorCode CompareTokens(SdmToken& , SdmToken&);
};

const char* SdmMessageStoreTest::kUserData = "message store test data";

SdmBoolean SdmMessageStoreTest::asyncCallbackCalled;
SdmBoolean SdmMessageStoreTest::sessionCallbackCalled;
SdmBoolean SdmMessageStoreTest::asyncCallbackError;
SdmBoolean SdmMessageStoreTest::openCallbackCalled;
SdmBoolean SdmMessageStoreTest::openCallbackError;
SdmBoolean SdmMessageStoreTest::closeCallbackCalled;
SdmBoolean SdmMessageStoreTest::closeCallbackError;
SdmToken SdmMessageStoreTest::lastOpenToken;
SdmMessageStore* SdmMessageStoreTest::lastMessageStore;
SdmBoolean SdmMessageStoreTest::expungeCallbackCalled;
SdmBoolean SdmMessageStoreTest::expungeCallbackError;

SdmString SdmMessageStoreTest::kServiceOption("serviceoption");
SdmString SdmMessageStoreTest::kServiceType("servicetype");
SdmString SdmMessageStoreTest::kServiceClass("serviceclass");
  
SdmLongL SdmMessageStoreTest::cancelledAsyncCalls;
SdmLongL SdmMessageStoreTest::successAsyncCalls;


SdmErrorCode RunMessageStoreTest()
{
  cout << endl << "************ RunMessageStoreTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;

  SdmMessageStoreTest test;
  SdmError err;
  err = test.Run();

  // clean up temporary files.
  unlink(kNewStoreName);
  unlink(kRenameStoreName);
  unlink(kOutgoingStoreName);
  unlink(kSourceStoreName);
  unlink(kDestinationStoreName);


  return err;
}
  

SdmMessageStoreTest::SdmMessageStoreTest()
{
}

SdmMessageStoreTest::~SdmMessageStoreTest()
{
}

SdmErrorCode 
SdmMessageStoreTest::Run()
{
  SdmError err;
  SdmSession *ises, *oses;
  SdmMailRc *mailrc;
  SdmConnection *mcon;
  SdmToken token;
  SdmMessage *firstMessage, *secondMessage;
  
  cout << endl << "************ SdmMessageStoreTest::Run Invoked. *************" << endl << endl;
  cout << endl;
    
  SdmErrorCode errorCode;
  
  if ((errorCode = Setup(mcon, ises, oses)) != Sdm_EC_Success)
    return errorCode;


  // ***************************************************************8
  // Testing Message Store
  // ***************************************************************8

  cout << "Testing message store.\n";
  
  if ((errorCode = TestStartup(ises, mbox, Sdm_True)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestServiceFunctionCalls(mbox)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestCreateAndDelete(ises)) != Sdm_EC_Success)
    return errorCode;

  token.SetValue(kServiceClass,SdmString("messagestore"));
  token.SetValue(kServiceType,SdmString("test"));
  token.SetValue(kServiceOption,SdmString("messagestorename"),SdmString("test.mbx"));

  if ((errorCode = TestOpenAndClose(mbox, token, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestFlagMethods(mbox)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestHeaderMethods(mbox)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestMessageFactory(mbox, firstMessage, secondMessage)) != Sdm_EC_Success)
    return errorCode;

  mbox->SdmMessageFactory(err, firstMessage, 1);
        
  if ((errorCode = TestCancelPendingOps(mbox, firstMessage, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestExpunge(mbox, firstMessage, secondMessage, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestNameSpaceCalls(mbox)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestAppendMessage(ises)) != Sdm_EC_Success) 
    return errorCode;     
      
/*  Fix!  Uncomment when Copy and Move message methods are implemented.
  if ((errorCode = TestCopyAndMove(mbox)) != Sdm_EC_Success) 
    return errorCode;
*/

  // run the shutdown test at the end because the other tests will fail
  // if the session is shut down.    
  if ((errorCode = TestShutdown(ises, mbox, Sdm_True)) != Sdm_EC_Success)
    return errorCode;

  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // ***************************************************************8
  // Test outgoing store
  // ***************************************************************8


  cout << "Testing outgoing store.\n";

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp messagestore.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmToken outtoken;
  outtoken.SetValue("serviceclass","messagestore");
  outtoken.SetValue("servicetype","local");
  outtoken.SetValue("serviceoption","messagestorename", kOutgoingStoreName);
  outtoken.SetValue("serviceoption","disconnected");

  if ((errorCode = TestStartup(oses, dbox, Sdm_False)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestServiceFunctionCalls(dbox)) != Sdm_EC_Success)
    return errorCode;
  
  if ((errorCode = TestOpenAndClose(dbox, outtoken, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestFlagMethods(dbox)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestHeaderMethods(dbox)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestMessageFactory(dbox, firstMessage, secondMessage)) != Sdm_EC_Success)
    return errorCode;

  dbox->SdmMessageFactory(err, firstMessage, 1);
        
  if ((errorCode = TestCancelPendingOps(dbox, firstMessage, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestExpunge(dbox, firstMessage, secondMessage, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestNameSpaceCalls(dbox)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestAppendMessage(oses)) != Sdm_EC_Success) 
    return errorCode;     

  // run the shutdown test last because the other tests will fail
  // if the session is shut down.    
  if ((errorCode = TestShutdown(oses, dbox, Sdm_True)) != Sdm_EC_Success)
    return errorCode;  


  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }


  delete mcon;

  // Sessions should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(i_frontEndQueue, &i_readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
 
  if (SdmHandleSessionEvent(o_frontEndQueue, &o_readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
        
  return Sdm_EC_Success;
}

SdmErrorCode 
SdmMessageStoreTest::Setup(SdmConnection *&mcon, SdmSession *&ises, 
  SdmSession *&oses)
{
  SdmError err;
  void* vptr;

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
  if (ises->StartUp(err)) {
    cout << "*** Error: Sdm Incoming Session StartUp Failed\n" ;
    return err;
  }
  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises)) {
    cout << "*** Error: attach incoming session failed.\n";
    return err;
  }
  i_frontEndQueue = (SdmReplyQueue*)vptr;

  
  // create and start up outgoing session.
  if (mcon->SdmSessionFactory(err, oses, Sdm_ST_OutSession)) {
    cout << "*** Error: create outgoing session failed\n";
    return err;
  }
  if (oses->StartUp(err)) {
    cout << "*** Error: Sdm Outgoing Session StartUp Failed\n" ;
    return err;
  }
  if (SdmAttachSession(err, o_readFileDesc, vptr, *oses)) {
    cout << "*** Error: attach outgoing session failed.\n";
    return err;
  }
  o_frontEndQueue = (SdmReplyQueue*)vptr;

  // create message store.
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cout << "*** Error: create outgoing store failed\n";
     return err;
  }
  
  return err;
}  
  
  
SdmErrorCode
SdmMessageStoreTest::TestParent(SdmMessageStore *mstore, SdmSession *session)
{
  SdmSession *parent;
  SdmError err;
  if (mstore->Parent(err, parent)) {
    cout << "*** Error: Parent fails.\n";
    return err;
  }

  if (parent != session) {
    cout << "Error: Parent returns wrong session.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}


     
SdmErrorCode 
SdmMessageStoreTest::TestStartup(SdmSession *session,
  SdmMessageStore *mstore, SdmBoolean isRegistered)
{
  SdmError err;
  
  if (mstore->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }

  if (isRegistered) {
    SdmRequestQueue *queue = session->MapToServerQueue(err, mstore);
#if !defined(USE_SOLARIS_THREADS)
    if (err == Sdm_EC_LocalServerNotRegistered) {
      err = Sdm_EC_Success;
    }
#endif
    if (err) {      
      cout << "*** Error: MapToServerQueue fails for registered mstore.\n";
      err = Sdm_EC_Fail;
    } else if (queue == NULL) {
#if defined(USE_SOLARIS_THREADS)
      cout << "*** Error: MapToServerQueue returns NULL queue for registered mstore.\n";
      err = Sdm_EC_Fail;
      return err;
#endif
    } else {
      SdmServer *server = session->MapToLocalServer(err, queue);
      if (err) {
        cout << "*** Error: MapToLocalServer fails for registered mstore.\n";
        err = Sdm_EC_Fail;
      } else if (server != (SdmServer*)mstore) {
        cout << "*** Error: MapToLocalServer returns wrong server.\n";
        err = Sdm_EC_Fail;
      }
    }
  }
 
  // try to start up mstore again.  this should result in error.
  if (mstore->StartUp(err)) {
    if (err != Sdm_EC_MessageStoreAlreadyStarted) {
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
SdmMessageStoreTest::TestServiceFunctionCalls(SdmMessageStore *mstore)
{
  SdmError err;
  SdmServiceFunction *svfP;
  
  SdmServiceFunction svf(&SessionCallback, 
          (void*)SdmMessageStoreTest::kUserData, 
          Sdm_SVF_Other);

  if (mstore->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  if (mstore->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
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
  if (mstore->UnregisterServiceFunction(err, Sdm_SVF_Other)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  if (mstore->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
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
  if (mstore->UnregisterServiceFunction(err, Sdm_SVF_Other)) {
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


  if (mstore->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  // test error code returned when trying to register 
  // the service function again.
  if (mstore->RegisterServiceFunction(err, svf)) {
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
  if (mstore->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
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
  if (mstore->UnregisterServiceFunction(err, svf)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  // test error code returned when trying to unregister the service
  // function again.
  if (mstore->UnregisterServiceFunction(err, svf)) {
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
  
  if (mstore->GetServiceFunction(err, svfP, Sdm_SVF_Other)) {
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
SdmMessageStoreTest::TestShutdown(SdmSession *session, SdmMessageStore *mstore,
  SdmBoolean isRegistered)
{
  SdmError err;

  if (mstore->ShutDown(err)) {
    cout << "*** Error: Session shutdown failed.\n";
    return err;
  }
  
  if (mstore->IsStarted() != Sdm_False) {
    cout << "*** Error: mstore IsStarted flag still set after shutdown.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (isRegistered) {
    SdmRequestQueue *queue = session->MapToServerQueue(err, mstore);
    if (err) {
      if (err != Sdm_EC_LocalServerNotRegistered) {
          cout << "*** Error: MapToServerQueue fails for unregistered mstore.\n";
          return err;
      } else {
          err = Sdm_EC_Success;
      }
    } else {
      cout << "*** Error: MapToServerQueue does not return error for unregistered mstore.\n";
      err = Sdm_EC_Fail;
      return err;
    }
  }
 
  // try to shutdown connection again.  this should result in error.
  if (mstore->ShutDown(err)) {
    if (err != Sdm_EC_MessageStoreNotStarted) {
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
SdmMessageStoreTest::TestOpenAndClose(SdmMessageStore *mstore, 
  SdmToken &token, SdmReplyQueue *frontEndQueue, 
  int readFileDesc)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

   SdmServiceFunction svf(&OpenCallback, (void*)kUserData, Sdm_SVF_Other);
   SdmServiceFunction svf2(&CloseCallback, (void*)kUserData, Sdm_SVF_Other);
   
  lastOpenToken = token;

  if (mstore->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  // try open again.  should return error.
  if (mstore->Open(err, nmsgs, readOnly, token)) {
    err = Sdm_EC_Success;
  } else {
    cout << "*** Error: Reopen does not return error.\n";
    return err;
  }
  
  if (mstore->Close(err)) {
    cout << "*** Error: Close failed.\n";
    return err;
  }

  lastMessageStore = mstore;
  openCallbackError = Sdm_False;
  openCallbackCalled = Sdm_False;
  
  if (mstore->Open_Async(err, svf, (void*) kUserData, token)) {
    cout << "*** Error: Open_Async failed.\n";
    return err;
  }

  if (PollForReply(frontEndQueue, readFileDesc, 1)) {
    return err;
  }
  
  if (!openCallbackCalled) {
    cout << "*** Error: Open callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (openCallbackError) {
    cout << "*** Error: error occurred in open callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  lastMessageStore = mstore;
  closeCallbackError = Sdm_False;
  closeCallbackCalled = Sdm_False;
  
  if (mstore->Close_Async(err, svf2, (void*) kUserData)) {
    cout << "*** Error: Close_Async failed.\n";
    return err;
  }

  if (PollForReply(frontEndQueue, readFileDesc, 1)) {
    return err;
  }
  
  if (!closeCallbackCalled) {
    cout << "*** Error: Close callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (closeCallbackError) {
    cout << "*** Error: error occurred in close callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (mstore->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open2 failed.\n";
    return err;
  }

  return err;
}

SdmErrorCode
SdmMessageStoreTest::TestCreateAndDelete(SdmSession *ses)
{
  SdmError err;
  SdmMessageStore *store;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  // first remove the new store in case it exists.
  int rc = unlink(kNewStoreName);
  rc = unlink(kRenameStoreName);
  
  if (ses->SdmMessageStoreFactory(err, store)) {
    cout << "*** Error: SdmMessageStoreFactory failed.\n";
    return err;
  }

  if (store->StartUp(err)) {
    cout << "*** Error: StartUp failed.\n";
    return err;
  }

  SdmToken token;
  token.SetValue("serviceclass","messagestore");
  token.SetValue("servicetype","local");
  
  if (store->Attach(err, token)) {
    cout << "*** Error: Attach failed.\n";
    return err;
  }

  if (store->Create(err, kNewStoreName)) {
    cout << "*** Error: Create failed.\n";
    return err;
  }

  // add store name to token in order to Open.
  token.SetValue("serviceoption","messagestorename",kNewStoreName);
  if (store->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  SdmMessageStoreStatus status;
  if (store->GetStatus(err, status, Sdm_MSS_ALL)) {
    cout << "*** Error: GetMessageStoreStatus failed. err = " << (int)err << "\n";
    return err;
  }
  
  // the store should not contain anything.
  if (status.messages != 0) {
    cout << "*** Error: newly created message store is not empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // close store before we can rename.
  if (store->Close(err)) {
    cout << "*** Error: Close failed.\n";
    return err;
  }

  // need to be attached in order for rename to work.
  if (store->Attach(err, token)) {
    cout << "*** Error: Attach2 failed.\n";
    return err;
  }

  if (store->Rename(err, kNewStoreName, kRenameStoreName)) {
    cout << "*** Error: Rename failed.\n";
    return err;
  }
 
  if (store->ShutDown(err)) {
    cout << "*** Error: ShutDown failed.\n";
    return err;
  }
  
  delete store;
   
  return err;
}



SdmErrorCode
SdmMessageStoreTest::TestNameSpaceCalls(SdmMessageStore *mstore)
{
  SdmError err;
  SdmIntStrL r_names;

  // reset name space first.
  mstore->RemoveFromSubscribedNamespace(err, "/usr/include/");
  mstore->RemoveFromSubscribedNamespace(err, "/usr/lib/");

  err = Sdm_EC_Success;

  if (mstore->ScanNamespace(err, r_names, "/usr/include/", "%")) {
    cout << "*** Error: ScanNamespace failed.\n";
    return err;
  }
 
  if (r_names.ElementCount() == 0) {
    cout << "*** Error: ScanNamespace returns empty list.\n";
    return err;
  }
  cout << "*** Info: ScanNamespace result = " 
       << r_names.ElementCount() << "\n";

  r_names.ClearAllElements();
  if (mstore->AddToSubscribedNamespace(err, "/usr/lib/")) {
    cout << "*** Error: AddToSubscribedNameSpace failed.\n";
    return err;
  }
  if (mstore->AddToSubscribedNamespace(err, "/usr/include/")) {
    cout << "*** Error: AddToSubscribedNameSpace failed.\n";
    return err;
  }

  /* Fix!  uncomment when ScanSubscribedNamespace works!!

  if (mstore->ScanSubscribedNamespace(err, r_names, "", "%")) {
    cout << "*** Error: ScanSubscribedNamespace failed.\n";
    return err;
  }
  cout << "*** Info: ScanSubscribedNamespace result = " 
       << r_names.ElementCount() << "\n";
       

  if (r_names.ElementCount() == 0) {
    cout << "*** Error: ScanSubscribedNamespace returns empty list.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  */

  if (mstore->RemoveFromSubscribedNamespace(err, "/usr/include/")) {
    cout << "*** Error: RemoveSubscribedNameSpace failed.\n";
    return err;
  }
  
  /* Fix!  uncomment when ScanSubscribedNamespace works!!
  
  r_names.ClearAllElements();
  if (mstore->ScanSubscribedNamespace(err, r_names, "", "")) {
    cout << "*** Error: ScanSubscribedNamespace failed.\n";
    return err;
  }
  if (r_names.ElementCount() == 0) {
    cout << "*** Error: ScanSubscribedNamespace returns empty list.\n";
    return err;
  }
  cout << "*** Info: ScanSubscribedNamespace result after remove = " 
       << r_names.ElementCount() << "\n";
  */

  if (mstore->RemoveFromSubscribedNamespace(err, "/usr/lib/")) {
    cout << "*** Error: RemoveSubscribedNameSpace failed.\n";
    return err;
  }

  return err;
}


SdmErrorCode
SdmMessageStoreTest::TestAppendMessage(SdmSession *ses)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  // first, create setup source and destination store.
  unlink(kDestinationStoreName);
  unlink(kSourceStoreName);
  char command[256];
  sprintf(command, "/bin/cp sourcestore.test %s", kSourceStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kSourceStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // create and open destination store.
  SdmMessageStore *src_store, *dest_store;
  if (ses->SdmMessageStoreFactory(err, dest_store)) {
    cout << "*** Error: SdmMessageStoreFactory failed.\n";
    return err;
  }
  if (dest_store->StartUp(err)) {
    cout << "*** Error: StartUp failed.\n";
    return err;
  }
  SdmToken token;
  token.SetValue("serviceclass","messagestore");
  token.SetValue("servicetype","local");
  if (dest_store->Attach(err, token)) {
    cout << "*** Error: Attach failed.\n";
    return err;
  }
  if (dest_store->Create(err, kDestinationStoreName)) {
    cout << "*** Error: Create failed.\n";
    return err;
  }
  token.SetValue("serviceoption","messagestorename",kDestinationStoreName);
  if (dest_store->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }  
  if (CheckMessageCount(err, dest_store, 0, 0)) {
    return err;
  }

  
  // open source store.
  if (ses->SdmMessageStoreFactory(err, src_store)) {
    cout << "*** Error: SdmMessageStoreFactory failed.\n";
    return err;
  }
  if (src_store->StartUp(err)) {
    cout << "*** Error: StartUp failed.\n";
    return err;
  }
  token.ClearValue("serviceoption","messagestorename",kDestinationStoreName);
  token.SetValue("serviceoption","messagestorename", kSourceStoreName);
  if (src_store->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  
  // get first message from source store and append it to destination.
  SdmMessage *firstmsg, *secondmsg, *thirdmsg;
  if (src_store->SdmMessageFactory(err, firstmsg, 1)) {
    cout << "*** SdmMessageFactory failed.\n";
    return err;
  }
  
  if (dest_store->AppendMessage(err, *firstmsg)) {
    cout << "*** AppendMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 1, 1)) {
    return err;
  }


  if (src_store->SdmMessageFactory(err, secondmsg, 2)) {
    cout << "*** SdmMessageFactory failed.\n";
    return err;
  }
  if (src_store->SdmMessageFactory(err, thirdmsg, 3)) {
    cout << "*** SdmMessageFactory failed.\n";
    return err;
  }
  
  SdmMessageL msglist;
  msglist.AddElementToList(secondmsg);
  msglist.AddElementToList(thirdmsg);
  
  if (dest_store->AppendMessages(err,msglist)) {
    cout << "*** AppendMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 3, 2)) {
    return err;
  }
    
  if (src_store->Close(err)) {
    cout << "*** Error: Close failed.\n";
    return err;
  }
  if (src_store->ShutDown(err)) {
    cout << "*** Error: ShutDown failed.\n";
    return err;
  }
  if (dest_store->Close(err)) {
    cout << "*** Error: Close failed.\n";
    return err;
  }
  if (dest_store->ShutDown(err)) {
    cout << "*** Error: ShutDown failed.\n";
    return err;
  }

  
  delete src_store;
  delete dest_store;
  
  return err;
}


SdmErrorCode
SdmMessageStoreTest::TestCopyAndMove(SdmSession *ses)
{

  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  // first, create setup source and destination store.
  unlink(kDestinationStoreName);
  unlink(kSourceStoreName);
  char command[256];
  sprintf(command, "/bin/cp sourcestore.test %s", kSourceStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kSourceStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // open source store.
  SdmMessageStore *src_store, *dest_store;
  if (ses->SdmMessageStoreFactory(err, src_store)) {
    cout << "*** Error: SdmMessageStoreFactory failed.\n";
    return err;
  }
  if (src_store->StartUp(err)) {
    cout << "*** Error: StartUp failed.\n";
    return err;
  }
  SdmToken token;
  token.SetValue("serviceclass","messagestore");
  token.SetValue("servicetype","local");
  token.SetValue("serviceoption","messagestorename", kSourceStoreName);
  if (src_store->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  // create and open dest store.
  token.ClearValue("serviceoption","messagestorename",kSourceStoreName);
  token.SetValue("serviceoption","messagestorename",kDestinationStoreName);
  if (dest_store->Attach(err, token)) {
    cout << "*** Error: Attach failed.\n";
    return err;
  }
  if (dest_store->Create(err, kDestinationStoreName)) {
    cout << "*** Error: Create failed.\n";
    return err;
  }
  if (dest_store->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }  
  
  // Test copy methods. 
  if (src_store->CopyMessage(err, token, 1)) {
    cout << "*** Error: CopyMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 1, 1)) {
    return err;
  }

  if (src_store->CopyMessages(err, token, 2, 3)) {
    cout << "*** Error: CopyMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 3, 2)) {
    return err;
  }

  SdmMessageNumberL msglist;
  msglist.AddElementToList(4);
  msglist.AddElementToList(5);
  if (src_store->CopyMessages(err, token, msglist)) {
    cout << "*** Error: CopyMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 5, 2)) {
    return err;
  }

  // Test copy methods. 
  if (src_store->MoveMessage(err, token, 1)) {
    cout << "*** Error: MoveMessage failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 6, 1)) {
    return err;
  }

  if (src_store->MoveMessages(err, token, 2, 3)) {
    cout << "*** Error: MoveMessages failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 8, 2)) {
    return err;
  }

  if (src_store->MoveMessages(err, token, msglist)) {
    cout << "*** Error: MoveMessages failed.\n";
    return err;
  }
  if (CheckMessageCount(err, dest_store, 10, 2)) {
    return err;
  }

  return err;
}

SdmErrorCode
SdmMessageStoreTest::CheckMessageCount(SdmError &err, SdmMessageStore* store, 
  const SdmMessageNumber expectedMsgs, 
  const SdmMessageNumber expectedNewMsgs)
{
  err = Sdm_EC_Success;
  
  SdmMessageNumber newmsgs;
  if (store->CheckForNewMessages(err, newmsgs)) {
    cout << "*** CheckForNewMessages failed.\n";
    return err;
  }  
  if (newmsgs != expectedNewMsgs) {
    cout << "*** Error: incorrect new message count from CheckForNewMessages.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  SdmMessageStoreStatus status;
  if (store->SaveMessageStoreState(err)) {
    cout << "*** SaveMessageStoreState failed.\n";
    return err;
  }  
  if (store->GetStatus(err, status, Sdm_MSS_Messages)) {
    cout << "*** Error: GetMessageStoreStatus failed. err = " << (int)err << "\n";
    return err;
  }
  if (status.messages != expectedMsgs) {
    cout << "*** Error: incorrect number of messages in store.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  return err;
}



SdmErrorCode
SdmMessageStoreTest::PollForReply(SdmReplyQueue *frontEndQueue, 
  int readFileDesc, int expectedCalls)
{
  SdmError err;
  int rc, i;
  struct pollfd pollFileDesc;
  pollFileDesc.fd = readFileDesc;
  pollFileDesc.events = POLLIN;
  char buffer[2000];
  
  for (i =0; i >= 0 ; i++) {
    if (i == expectedCalls) {
      break;
    }
    if ((rc = poll(&pollFileDesc, 1, INFTIM)) > 0) {
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
  
  if (i != expectedCalls) {
    sprintf(buffer, "*** Error: Did not get expected number of replies.  expected=%d, received = %d.\n",
      expectedCalls, i);
    cout << buffer;
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode
SdmMessageStoreTest::CompareTokens(SdmToken &source, SdmToken &target)
{
    SdmError err;
    SdmString value, name("messagestorename");
    if (source.FetchValue(value, kServiceClass)== Sdm_False){
      cout << "*** Error: service class not set.\n";
      err = Sdm_EC_Fail;
    }
    if (target.CheckValue(kServiceClass, value) == Sdm_False) {
      cout << "*** Error: serviceclass value incorrect in source from event.\n";
      err = Sdm_EC_Fail;
    }
    if (source.FetchValue(value, kServiceType) == Sdm_False) {
      cout << "*** Error: service type not set.\n";
      err = Sdm_EC_Fail;
    }
    if (target.CheckValue(kServiceType, value) == Sdm_False) {
      cout << "*** Error: servicetype value incorrect in source from event.\n";
      err = Sdm_EC_Fail;
    }
    if (source.FetchValue(value, (const char*)kServiceOption, "messagestorename")== Sdm_False) {
      cout << "*** Error: service option not set.\n";
      err = Sdm_EC_Fail;
    }
    if (target.CheckValue(kServiceOption, SdmString("messagestorename"), value) == Sdm_False) {
      cout << "*** Error: serviceoption value incorrect in source from event.\n";
      err = Sdm_EC_Fail;
    }
    return err;
}

SdmErrorCode
SdmMessageStoreTest::TestFlagMethods(SdmMessageStore *mstore)
{

  SdmError localError, err;
  SdmMessageFlagAbstractFlags r_flags, flag, aflags;
  SdmMessageFlagAbstractFlagsL r_flagsL;
  SdmMessageNumber msgnum = 1, startmsgnum=1, endmsgnum=3;
  SdmMessageNumberL msgnumL;
  
  msgnumL.AddElementToList(2);
  msgnumL.AddElementToList(4);
  
  flag = Sdm_MFA_Draft;
  aflags = (Sdm_MFA_Answered|Sdm_MFA_Seen);
                  
  if (mstore->GetFlags(err, r_flags, msgnum)) {
    cout << "*** Error: GetFlags1 failed. err = " << (int)err << "\n";
    localError = err;
  }
  if (mstore->GetFlags(err, r_flagsL, startmsgnum, endmsgnum)){
    cout << "*** Error: GetFlags2 failed. err = " << (int)err << "\n";
    localError = err;
  }
  r_flagsL.ClearAllElements();
  if (mstore->GetFlags(err, r_flagsL, msgnumL)){
    cout << "*** Error: GetFlags3 failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (mstore->ClearFlags(err, flag, msgnum)){
    cout << "*** Error: ClearFlags2 failed. err = " << (int)err << "\n";
    localError = err;
  }
  if (mstore->ClearFlags(err, flag, startmsgnum, endmsgnum)){
    cout << "*** Error: ClearFlags3 failed. err = " << (int)err << "\n";
    localError = err;
  }
  if (mstore->ClearFlags(err, flag, msgnumL)){
    cout << "*** Error: ClearFlags4 failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (mstore->SetFlags(err, aflags, msgnum)){
    cout << "*** Error: SetFlags2 failed. err = " << (int)err << "\n";
    localError = err;
  }
  if (mstore->SetFlags(err, aflags, startmsgnum, endmsgnum)){
    cout << "*** Error: SetFlags3 failed.  err = " << (int)err << "\n";
    localError = err;
  }
  if (mstore->SetFlags(err, aflags, msgnumL)){
    cout << "*** Error: SetFlags4 failed. err = " << (int)err << "\n";
    localError = err;
  }
  return localError;
}


SdmErrorCode
SdmMessageStoreTest::TestHeaderMethods(SdmMessageStore *mstore)
{
  SdmError localError, err;
  SdmStrStrLL r_hdrLL;
  SdmMessageHeaderAbstractFlags hdr;
  SdmMessageNumber msgnum=1, startmsgnum=1, endmsgnum=3;
  SdmString str_hdr("date");
  SdmStringL str_hdrL;
  SdmStrStrL r_hdrL;
  SdmIntStrL r_int_hdrL;
  SdmIntStrLL r_int_hdrLL;
  SdmMessageNumberL msgnumL;
  
  hdr = Sdm_MHA_Subject;
  
  str_hdrL(-1) = "from";
  str_hdrL(-1) = "to";
  
  msgnumL.AddElementToList(2);
  msgnumL.AddElementToList(4);
  
  
  if (mstore->GetHeaders(err, r_hdrLL, hdr, msgnumL)){
    cout << "*** Error: GetHeader1 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeader(err, r_hdrLL, str_hdr, startmsgnum, endmsgnum)){
    cout << "*** Error: GetHeader2 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_hdrLL, hdr, startmsgnum, endmsgnum)){
    cout << "*** Error: GetHeader3 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrL.ClearAllElements();
  if (mstore->GetHeaders(err, r_hdrL, hdr, msgnum)){
    cout << "*** Error: GetHeader4 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrL.ClearAllElements();
  if (mstore->GetHeader(err, r_hdrL, str_hdr, msgnum)){
    cout << "*** Error: GetHeader5 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeader(err, r_hdrLL, str_hdr, msgnumL)){
    cout << "*** Error: GetHeader6 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrL.ClearAllElements();
  if (mstore->GetHeaders(err, r_hdrL, msgnum)){
    cout << "*** Error: GetHeaders1 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_hdrLL, startmsgnum,	endmsgnum)){
    cout << "*** Error: GetHeaders2 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_hdrLL, msgnumL)){
    cout << "*** Error: GetHeaders3 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrL.ClearAllElements();
  if (mstore->GetHeaders(err, r_hdrL, str_hdrL,	msgnum)){
    cout << "*** Error: GetHeaders4 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_hdrLL,	str_hdrL, startmsgnum, endmsgnum)){
    cout << "*** Error: GetHeaders5 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_hdrLL, str_hdrL, msgnumL)){
    cout << "*** Error: GetHeaders6 failed. err = " << (int)err << "\n";
    localError = err;
  }
  
  r_int_hdrL.ClearAllElements();
  if (mstore->GetHeaders(err, r_int_hdrL, hdr, msgnum)){
    cout << "*** Error: GetHeaders7 failed. err = " << (int)err << "\n";
    localError = err;
  }
                                  
  r_int_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_int_hdrLL, hdr, startmsgnum, endmsgnum)){
    cout << "*** Error: GetHeaders8 failed. err = " << (int)err << "\n";
    localError = err;
  }
                                  
  r_int_hdrLL.ClearAndDestroyAllElements();
  if (mstore->GetHeaders(err, r_int_hdrLL, hdr, msgnumL)){
    cout << "*** Error: GetHeaders9 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_int_hdrLL.ClearAndDestroyAllElements();
  r_hdrLL.ClearAndDestroyAllElements();
                                  
  return localError;
}

SdmErrorCode
SdmMessageStoreTest::TestMessageFactory(SdmMessageStore *mstore, 
  SdmMessage *&firstMsg, SdmMessage *&secondMsg)
{
  SdmError err;
  SdmMessage *msg;
  
  if (mstore->SdmMessageFactory(err, firstMsg, 1)) {
    cout << "*** Error: SdmMessageFactory failed. err = " << (int)err << "\n";
    return err;
  }
    
  // check that same message is returned when called again.    
  if (mstore->SdmMessageFactory(err, msg, 1)) {
    cout << "*** Error: SdmMessageFactory failed getting msg1. err = " << (int)err << "\n";
    return err;
  }
  if (msg != firstMsg) {
    cout << "*** Error: SdmMessageFactory failed in second call. err = " << (int)err << "\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mstore->SdmMessageFactory(err, secondMsg, 2)) {
    cout << "*** Error: SdmMessageFactory failed getting msg2. err = " << (int)err << "\n";
    return err;
  }
  
  // check that message cache is cleaned up when message is deleted.
  SdmMessage *badMsg;
  delete secondMsg;    // this should remove reference to message from message store cache.
  secondMsg = NULL;
  
  // GetMessageAt should return false because we deleted the message.
  if (mstore->GetMessageAt(err, badMsg, 2)) {
    cout << "*** Error: message store has reference to deleted msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mstore->SdmMessageFactory(err, secondMsg, 2)) {
    cout << "*** Error: SdmMessageFactory failed getting msg2. err = " << (int)err << "\n";
    return err;
  }

  return err;
}


SdmErrorCode
SdmMessageStoreTest::TestExpunge(SdmMessageStore *mstore, 
  SdmMessage *&firstMsg, SdmMessage *&secondMsg,
  SdmReplyQueue *frontEndQueue, int readFileDesc)
{
  SdmError err;
  SdmMessageFlagAbstractFlags flag = Sdm_MFA_Deleted;
  SdmMessageNumber r_msgnum, msgnum;
  SdmMessageNumberL r_msgnums,  msgnums;
  long origMsgCount;

  SdmMessageStoreStatus status;
  if (mstore->GetStatus(err, status, Sdm_MSS_ALL)) {
    cout << "*** Error: GetMessageStoreStatus failed. err = " << (int)err << "\n";
    return err;
  }
  origMsgCount = status.messages;

  // first, let's get the other messages in the store.
  SdmMessage *thirdMsg, *fourthMsg;
  if (mstore->SdmMessageFactory(err, thirdMsg, 3)) {
    cout << "*** Error: SdmMessageFactory failed getting new msg1. err = " << (int)err << "\n";
    return err;
  }
  if (mstore->SdmMessageFactory(err, fourthMsg, 4)) {
    cout << "*** Error: SdmMessageFactory failed getting new msg1. err = " << (int)err << "\n";
    return err;
  }

  // get envelope, bodypart, nested message, and nested body from fourth message.
  SdmMessageBody *body4, *nestedBody;
  SdmMessage *nestedMsg;
  SdmMessageEnvelope *envelope4;
  
  if (fourthMsg->SdmMessageEnvelopeFactory(err, envelope4)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed getting envelope4. err = " << (int)err << "\n";
    return err;
  }
  if (fourthMsg->SdmMessageBodyFactory(err, body4, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed getting body4. err = " << (int)err << "\n";
    return err;
  }
  if (body4->SdmMessageFactory(err, nestedMsg)) {
    cout << "*** Error: SdmMessageFactory failed getting nested msg. err = " << (int)err << "\n";
    return err;
  }
  if (nestedMsg->SdmMessageBodyFactory(err, nestedBody, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed getting nested body. err = " << (int)err << "\n";
    return err;
  }
  
  // set the delete flag for the first message.
  msgnums.AddElementToList(1);
  msgnums.AddElementToList(3);
  if (mstore->SetFlags(err, flag, msgnums)){
    cout << "*** Error: SetFlags1 failed. err = " << (int)err << "\n";
    return err;
  }
  
  // expunge the deleted messages.
  if (mstore->ExpungeDeletedMessages(err, r_msgnums)){
    cout << "*** Error: Expunge failed. err = " << (int)err << "\n";
    return err;
  }
  
  // check the results from the expunge call.  
  if (r_msgnums.ElementCount() != 2) {
    cout << "*** Error: Wrong number of messages deleted.  count = " 
         << (int)r_msgnums.ElementCount() << "\n";
    err = Sdm_EC_Fail;
    return err;  
  } else if (r_msgnums[0] != 1 || r_msgnums[1] != 3) {
    cout << "*** Error: Wrong message deleted.\n";
    err = Sdm_EC_Fail;
    return err;  
  }
  
  // check the expunge flag for the first message is set.
  if (!firstMsg->IsExpunged() || !thirdMsg->IsExpunged()) {
    cout << "*** Error: Expunge flag not set for message after expunge. \n";
    err = Sdm_EC_Fail;
    return err;  
  }
  delete firstMsg;
  delete thirdMsg;

  // check the message number is set to 1 for the 2nd msg and 2 for the 4th msg.
  if (secondMsg->GetMessageNumber(err, r_msgnum)) {
    cout << "*** Error: GetMessageNumber2 failed. err = " << (int)err << "\n";
    return err;      
  }    
  if (r_msgnum != 1) {
    cout << "*** Error: Msg number for second message is wrong after expunge. msgnum = "
         << (int)r_msgnum << "\n";
    err = Sdm_EC_Fail;
    return err;  
  }
  if (fourthMsg->GetMessageNumber(err, r_msgnum)) {
    cout << "*** Error: GetMessageNumber4 failed. err = " << (int)err << "\n";
    return err;      
  }    
  if (r_msgnum != 2) {
    cout << "*** Error: Msg number for fourth message is wrong after expunge. msgnum = "
         << (int)r_msgnum << "\n";
    err = Sdm_EC_Fail;
    return err;  
  }
  
  if (body4->_ParentMessgNum != 2) {
    cout << "*** Error: Msg number for body4 is wrong after expunge.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (envelope4->_ParentMessgNum != 2) {
    cout << "*** Error: Msg number for envelope4 is wrong after expunge.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (nestedMsg->_MessgNumber != 2) {
    cout << "*** Error: Msg number for nestedMsg is wrong after expunge.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (nestedBody->_ParentMessgNum != 2) {
    cout << "*** Error: Msg number for nestedBody is wrong after expunge.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmMessage *newFirstMsg, *newSecondMsg;
  if (mstore->SdmMessageFactory(err, newFirstMsg, 1)) {
    cout << "*** Error: SdmMessageFactory failed getting new msg1. err = " << (int)err << "\n";
    return err;
  }
  if (mstore->SdmMessageFactory(err, newSecondMsg, 2)) {
    cout << "*** Error: SdmMessageFactory failed getting new msg2. err = " << (int)err << "\n";
    return err;
  }
 
  // check messages left in cache are in the right order.
  if (newFirstMsg != secondMsg || newSecondMsg != fourthMsg) {
    cout << "*** Error: Cache in message store out of sync after expunge.\n";
    err = Sdm_EC_Fail;
    return err;  
  }
  firstMsg = newFirstMsg;
  secondMsg = newSecondMsg;
  
  // check the message store status to see if it reflects the expunge.
  if (mstore->GetStatus(err, status, Sdm_MSS_ALL)) {
    cout << "*** Error: GetMessageStoreStatus failed. err = " << (int)err << "\n";
    return err;
  } else if (status.messages != (origMsgCount-2)) {
    cout << "*** Error: GetMessageStoreStatus returns incorrect message count. count = " 
         << (int)status.messages << "\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  //
  // Test ExpungeDeletedMessages_Async.
  //
  cout << "*** Info: Testing ExpungeDeletedMessages_Async\n";
    
  SdmServiceFunction svf(&ExpungeCallback, 
          (void*)SdmMessageStoreTest::kUserData, Sdm_SVF_Other);
  
  // delete the first message.
  msgnum = 1;
  if (mstore->SetFlags(err, flag, msgnum)) {
    cout << "*** Error: SetFlags2 failed. err = " << (int)err << "\n";
    return err;
  }

  lastMessageStore = mstore;
  expungeCallbackError = Sdm_False;
  expungeCallbackCalled = Sdm_False;
  
  if (mstore->ExpungeDeletedMessages_Async(err, svf, (void*)kUserData)){
    cout << "*** Error: Expunge Async failed. err = " << (int)err << "\n";
    return err;
  }

  if (PollForReply(frontEndQueue, readFileDesc, 1)) {
    return err;
  }
  
  if (!expungeCallbackCalled) {
    cout << "*** Error: expunge callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (expungeCallbackError) {
    cout << "*** Error: error occurred in expunge callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // check the expunge flag for the first message is set.
  if (!firstMsg->IsExpunged()) {
    cout << "*** Error: Expunge flag not set for message after expunge async. \n";
    err = Sdm_EC_Fail;
    return err;  
  }
  delete firstMsg;
  firstMsg = secondMsg;
  secondMsg = NULL;

  // check the message store status to see if it reflects the expunge.
  if (mstore->GetStatus(err, status, Sdm_MSS_ALL)) {
    cout << "*** Error: GetMessageStoreStatus2 failed. err = " << (int)err << "\n";
    return err;
  } else if (status.messages != (origMsgCount-3)) {
    cout << "*** Error: GetMessageStoreStatus2 returns incorrect message count. count = " 
         << (int)status.messages << "\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}


SdmErrorCode
SdmMessageStoreTest::TestCancelPendingOps(SdmMessageStore *mstore, SdmMessage *msg,
  SdmReplyQueue *frontEndQueue, int readFileDesc)
{
  SdmError err;
  static const kTotalAsyncCalls = 50;
  
  SdmServiceFunction svf(&AsyncCallback, (void*)kUserData, Sdm_SVF_Other);

  successAsyncCalls.ClearAllElements();
  cancelledAsyncCalls.ClearAllElements();
  
  // make multiple asynchronous calls.  the results will be stuck on the
  // front end queue.
  for (int i=0; i< kTotalAsyncCalls; i++) { 
    if (msg->GetContents_Async(err, svf, (void*)kUserData, i, 888)) {  // make async call.
      cout << "*** Error: SdmMessageStore::GetContents_Async failed.\n";
      return err;
    }
  }
  
  if (mstore->CancelPendingOperations(err)) {
      cout << "*** Error: CancelPendingOperations failed.\n";
      err = Sdm_EC_Success;
  }

  
  if (PollForReply(frontEndQueue, readFileDesc, kTotalAsyncCalls)) {
    err = Sdm_EC_Fail;
    return err;
  }

  if (successAsyncCalls.ElementCount()+cancelledAsyncCalls.ElementCount() != kTotalAsyncCalls) {
      cout << "*** Error: incorrect number of async calls returned.\n";
      return err;
  }  
  
  // check that the async calls are in ascending order.
  int count = successAsyncCalls.ElementCount();
  int last = -1;
  for (i=0; i< count; i++) {
      if (last <= 0) {
        last = successAsyncCalls[i];
      } else if (successAsyncCalls[i] != last+1) {
        cout << "*** Error: success async calls out of order.\n";
        err = Sdm_EC_Fail;
        return err;
     } else {
        last = successAsyncCalls[i];
     }
  }
  if (count) {
     cout << "*** Info: success async calls from " << successAsyncCalls[0] 
        <<  " to " << successAsyncCalls[count-1] << "\n";
  }
         
  // check that the async calls are in ascending order.
  count = cancelledAsyncCalls.ElementCount();
  last = -1;
  for (i=0; i< count; i++) {
      if (last <= 0) {
        last = cancelledAsyncCalls[i];
      } else if (cancelledAsyncCalls[i] != last+1) {
        cout << "*** Error: success async calls out of order.\n";
        err = Sdm_EC_Fail;
        return err;
     } else {
        last = cancelledAsyncCalls[i];
     }
  }
  if (count) {
     cout << "*** Info: cancelled async calls from " << cancelledAsyncCalls[0] 
        <<  " to " << cancelledAsyncCalls[count-1] << "\n";
  }
  return err;
}

//
// Callbacks
//
void 
SdmMessageStoreTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmMessageStoreTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
  
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }
}
 
void 
SdmMessageStoreTest::AsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  asyncCallbackCalled = Sdm_True;
 
  cout << "AsyncCallback Called.\n";
  if (userdata != SdmMessageStoreTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    asyncCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_GetPartialContents:
      {
        char buffer[2000];
        int stOffS = event->sdmGetContents.startOffset;
        int length = event->sdmGetContents.length;
 
        if (event->sdmGetContents.error == NULL) {
          cout << "*** Error: error is null in event.\n";
          asyncCallbackError = Sdm_True;
        } else {
          SdmError *error = event->sdmGetContents.error;
          if (*error == Sdm_EC_Cancel) {
             cancelledAsyncCalls.AddElementToList(stOffS);
          } else if (*error == Sdm_EC_Success) {
             successAsyncCalls.AddElementToList(stOffS);
          } else {
              cout << "*** Error: unknown error code returned from async call. err = "
                 << (int)*error << "\n" ;
              asyncCallbackError = Sdm_True;
          }
           sprintf(buffer, "AsyncCallback called with args=%d,%d, error=%d\n",
              stOffS, length, (int)(*(event->sdmGetContents.error)));
           cout << buffer;
        }

        if (event->sdmGetContents.buffer) {
          delete (event->sdmGetContents.buffer);
        }
        break;
      }
 
    default:
      cout << "*** Error: AyncCallback default event received.\n";
      asyncCallbackError = Sdm_True;
      break;
  }
}


void 
SdmMessageStoreTest::OpenCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  openCallbackCalled = Sdm_True;
 
  cout << "OpenCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageStoreTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    openCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_OpenMessageStore:
      {
        // check error in event
        if (event->sdmOpenMessageStore.error == NULL) {
          cout << "*** Error: error is NULL in event.\n";
          openCallbackError = Sdm_True;        
        } else if (*(event->sdmOpenMessageStore.error)) {
          cout << "*** Error: error is set in event.\n";
          openCallbackError = Sdm_True;
        }
        break;
      }
 
    default:
      cout << "*** Error: OpenCallback default event received.\n";
      openCallbackError = Sdm_True;
      break;
  }
}




void 
SdmMessageStoreTest::CloseCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  closeCallbackCalled = Sdm_True;
 
  cout << "CloseCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageStoreTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    closeCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_CloseMessageStore:
      {
        // check error in event
        if (event->sdmCloseMessageStore.error == NULL) {
          cout << "*** Error: error is NULL in event.\n";
          closeCallbackError = Sdm_True;        
        } else if (*(event->sdmCloseMessageStore.error)) {
          cout << "*** Error: error is set in event.\n";
          closeCallbackError = Sdm_True;
        }
        break;
      }
 
    default:
      cout << "*** Error: CloseCallback default event received.\n";
      closeCallbackError = Sdm_True;
      break;
  }
}


void 
SdmMessageStoreTest::ExpungeCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  expungeCallbackCalled = Sdm_True;
 
  cout << "ExpungeCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageStoreTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    expungeCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_ExpungeDeletedMessages:
      {
        // check token in event.           
        SdmMessageNumberL *msgs= event->sdmExpungeDeletedMessages.deletedMessages;
        if (msgs == NULL) {
          cout << "*** Error: msgs in event is null.\n";
          expungeCallbackError = Sdm_True;
        } else {
          if (msgs->ElementCount() != 1) {
             cout << "*** Error: wrong number of msgs deleted.\n";
             expungeCallbackError = Sdm_True;
          } else if ((*msgs)[0] != 1) {
             cout << "*** Error: wrong message deleted.\n";
             expungeCallbackError = Sdm_True;
          }
          delete msgs;
        }
        
        // check error in event
        if (event->sdmExpungeDeletedMessages.error == NULL) {
          cout << "*** Error: error is NULL in event.\n";
          expungeCallbackError = Sdm_True;        
        } else if (*(event->sdmExpungeDeletedMessages.error)) {
          cout << "*** Error: error is set in event.\n";
          expungeCallbackError = Sdm_True;
        }
        break;
      }
 
    default:
      cout << "*** Error: ExpungeCallback default event received.\n";
      expungeCallbackError = Sdm_True;
      break;
  }
}

