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

static  const char* kOutgoingStoreName = "/tmp/msg.test";


class SdmMessageTest
{
public:
     SdmMessageTest();
     virtual ~SdmMessageTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode Setup(SdmConnection *&, SdmSession *&ises, SdmSession *&oses, 
                     SdmMessageStore *&mbox, SdmMessageStore *&dbox);
                     
      SdmErrorCode TestMessageInStore(SdmMessageStore* mbox, 
                      SdmReplyQueue *frontEndQueue, int readFileDesc);

     SdmErrorCode TestParent(SdmMessage *, SdmMessageStore *);
     SdmErrorCode TestMessageNumber(SdmMessage *message, SdmMessageNumber msgnum);
     SdmErrorCode TestFlagMethods(SdmMessage *message);
     SdmErrorCode TestMessageBodyFactory(SdmMessage *message);
     SdmErrorCode TestMessageEnvelopeFactory(SdmMessage *message);
     SdmErrorCode TestNestedMessage(SdmMessage *message, SdmMessage *parentMsg);
     SdmErrorCode TestGetContents(SdmMessage *message, SdmReplyQueue* frontEndQueue,
                      int readFileDesc);
     SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
                          
  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void AsyncCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void GetContentsCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);

private:

  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
  
  static SdmString kServiceOption;
  static SdmString kServiceType;
  static SdmString kServiceClass;
  
  
  // the following need to be static because they are used by
  // the callback functions.
  static SdmBoolean asyncCallbackCalled;
  static SdmBoolean sessionCallbackCalled;
  static SdmBoolean getContentsCallbackCalled;
  static SdmBoolean asyncCallbackError;
  static SdmBoolean getContentsCallbackError;
  static SdmMessage *lastMessage;
  static int lastStartOffS;
  static SdmString lastContentBuffer;
};

const char* SdmMessageTest::kUserData = "message store test data";

SdmBoolean SdmMessageTest::asyncCallbackCalled;
SdmBoolean SdmMessageTest::sessionCallbackCalled;
SdmBoolean SdmMessageTest::asyncCallbackError;
SdmBoolean SdmMessageTest::getContentsCallbackCalled;
SdmBoolean SdmMessageTest::getContentsCallbackError;
SdmMessage* SdmMessageTest::lastMessage;
int SdmMessageTest::lastStartOffS;
SdmString SdmMessageTest::lastContentBuffer;

SdmString SdmMessageTest::kServiceOption("serviceoption");
SdmString SdmMessageTest::kServiceType("servicetype");
SdmString SdmMessageTest::kServiceClass("serviceclass");
  


SdmErrorCode RunMessageTest()
{
  cout << endl << "************ RunMessageTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmMessageTest test;
  return test.Run();
}
  

SdmMessageTest::SdmMessageTest()
{
}

SdmMessageTest::~SdmMessageTest()
{
}

SdmErrorCode 
SdmMessageTest::Run()
{
  SdmError err;
  SdmSession *ises, *oses;
  SdmMailRc *mailrc;
  SdmConnection *mcon;
  SdmToken token;
  SdmMessageStore *mbox, *dbox;
  
  cout << endl << "************ SdmMessageTest::Run Invoked. *************" << endl << endl;
  cout << endl;
    
  SdmErrorCode errorCode;
  
  if ((errorCode = Setup(mcon, ises, oses, mbox, dbox)) != Sdm_EC_Success)
    return errorCode;


  // ***************************************************************8
  // Testing Message
  // ***************************************************************8

/*
  cout << "Testing message.\n";

  if ((errorCode = TestMessageInStore(mbox, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success) {
    return errorCode;
  }
*/

  // ***************************************************************8
  // Test outgoing store
  // ***************************************************************8


  cout << "Testing outgoing.\n";
  
  if ((errorCode = TestMessageInStore(dbox, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) {
    return errorCode;
  }
  
  // note: don't need to delete bodies or envelope.  they are deleted by the SdmMessage.
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
 
  // clean up temp outgoing store.
  unlink(kOutgoingStoreName);
        
  return Sdm_EC_Success;
}

SdmErrorCode
SdmMessageTest::TestMessageInStore(SdmMessageStore* mbox, SdmReplyQueue *frontEndQueue, int readFileDesc)
{
  SdmError err;
  SdmErrorCode errorCode;
  SdmMessage *firstMessage, *fourthMessage;
  
  if (mbox->SdmMessageFactory(err, firstMessage, 1)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  
  if (mbox->SdmMessageFactory(err, fourthMessage, 4)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }

 if ((errorCode = TestParent(firstMessage, mbox)) != Sdm_EC_Success)
    return errorCode;
        
 if ((errorCode = TestParent(fourthMessage, mbox)) != Sdm_EC_Success)
    return errorCode;
        
 if ((errorCode = TestMessageNumber(firstMessage, 1)) != Sdm_EC_Success)
    return errorCode;
        
 if ((errorCode = TestMessageNumber(fourthMessage, 4)) != Sdm_EC_Success)
    return errorCode;
        
  if ((errorCode = TestFlagMethods(firstMessage)) != Sdm_EC_Success)
    return errorCode;
      
  if ((errorCode = TestMessageBodyFactory(firstMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestMessageEnvelopeFactory(firstMessage)) != Sdm_EC_Success)
    return errorCode;

  // fourth message has multiple body parts.
  if ((errorCode = TestMessageBodyFactory(fourthMessage)) != Sdm_EC_Success)
    return errorCode;
    
  if ((errorCode = TestGetContents(firstMessage, frontEndQueue, readFileDesc)) != Sdm_EC_Success) 
    return errorCode;

  SdmMessageBody *body;
  SdmMessage *nestedMessage;
  
  // create nested message from body of 4th message.  4th message has
  // multipart mixed body.
  if (fourthMessage->SdmMessageBodyFactory(err, body, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed for message 4.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (body->SdmMessageFactory(err, nestedMessage)) {
    cout << "*** Error: SdmMessageFactory failed for nested message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if ((errorCode = TestNestedMessage(nestedMessage, fourthMessage)) != Sdm_EC_Success)
    return errorCode;

// Fix! remove when Cclient can get raw contents for nested message.
// Only check contents of tested message for tclient message store (eg. inbox).
  if (i_frontEndQueue == frontEndQueue) {
    if ((errorCode = TestGetContents(nestedMessage, frontEndQueue, readFileDesc)) != Sdm_EC_Success) 
      return errorCode;
  }

  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  delete firstMessage;
  delete fourthMessage;
  
  return err;
}


SdmErrorCode 
SdmMessageTest::Setup(SdmConnection *&mcon, SdmSession *&ises, 
  SdmSession *&oses,  SdmMessageStore *&mbox, SdmMessageStore *&dbox)
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;
  void* vptr;

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


  // create and start up message store.
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cout << "*** Error: create outgoing store failed\n";
     return err;
  }
  if (mbox->StartUp(err)) {
    cout << "*** Error: message store StartUp Failed\n" ;
    return err;
  }
  if (dbox->StartUp(err)) {
    cout << "*** Error: outgoing store StartUp Failed\n" ;
    return err;
  }

  // register default service function for any service type at the session level.
  SdmServiceFunction svf(&SessionCallback,
          (void*)SdmMessageTest::kUserData,
          Sdm_SVF_Other);
 
  if (mbox->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  if (dbox->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
 
  // open the message stores.
  SdmToken token;
  token.SetValue(kServiceClass,SdmString("messagestore"));
  token.SetValue(kServiceType,SdmString("test"));
  token.SetValue(kServiceOption,SdmString("messagestorename"),SdmString("test.mbx"));
 
  if (mbox->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  SdmToken outtoken;
  outtoken.SetValue("serviceclass","messagestore");
  outtoken.SetValue("servicetype","local");
  outtoken.SetValue("serviceoption","messagestorename", kOutgoingStoreName);
  outtoken.SetValue("serviceoption","disconnected");
  if (dbox->Open(err, nmsgs, readOnly, outtoken)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }
  
  return err;
}  
  
  
SdmErrorCode
SdmMessageTest::TestParent(SdmMessage *message, SdmMessageStore *mstore)
{
  SdmMessageStore *parent;
  SdmError err;
  if (message->Parent(err, parent)) {
    cout << "*** Error: Parent fails.\n";
    return err;
  }

  if (parent != mstore) {
    cout << "*** Error: Parent returns wrong message store.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode
SdmMessageTest::TestMessageNumber(SdmMessage *message, SdmMessageNumber msgnum)
{
  SdmMessageNumber r_msgnum;
  SdmError err;
  if (message->GetMessageNumber(err, r_msgnum)) {
    cout << "*** Error: GetMessageNumber fails.\n";
    return err;
  }

  if (r_msgnum != msgnum) {
    err = Sdm_EC_Fail;
    cout << "*** Error: GetMessageNumber returns wrong msg number.\n";
    return err;
  }
  return err;
}



     
SdmErrorCode
SdmMessageTest::PollForReply(SdmReplyQueue *frontEndQueue, 
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
SdmMessageTest::TestFlagMethods(SdmMessage *message)
{

  SdmError localError, err;
  SdmMessageFlagAbstractFlags r_flags, aflags, flag;

  flag = Sdm_MFA_Draft;
  aflags = (Sdm_MFA_Answered|Sdm_MFA_Seen);
                  
  if (message->GetFlags(err, r_flags)) {
    cout << "*** Error: GetFlags failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (message->ClearFlags(err, flag)){
    cout << "*** Error: ClearFlags2 failed. err = " << (int)err << "\n";
    localError = err;
  }
  
  if (message->SetFlags(err, aflags)){
    cout << "*** Error: SetFlags2 failed. err = " << (int)err << "\n";
    localError = err;
  }

  return localError;
}



SdmErrorCode
SdmMessageTest::TestMessageBodyFactory(SdmMessage *message) 
{
  SdmError err;
  SdmMessageBody *body;
  int body_count;
  
  if (message->GetBodyCount(err, body_count)) {
    cout << "*** Error: GetBodyCount failed. err = " << (int)err << "\n";
    return err;
  }
  
  SdmVector<SdmMessageBody *> bodyList(body_count);

  // save the bodies for this message.
  for (int i=1; i<= body_count; i++) {      
    if (message->SdmMessageBodyFactory(err, body, i)) {
      cout << "*** Error: SdmMessageBodyFactory failed for body "
           << i << ". err = " << (int)err << "\n";
      return err;
    }
    bodyList.AddElementToList(body);
  }

  // check that error is returned for out of range body number.
  if (message->SdmMessageBodyFactory(err, body, 0)) {
    err = Sdm_EC_Success;
  } else {
    cout << "*** Error: SdmMessageBodyFactory doesn't return error for body 0.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (message->SdmMessageBodyFactory(err, body, i)) {
    err = Sdm_EC_Success;
  } else {
    cout << "*** Error: SdmMessageBodyFactory doesn't return error for body count+1.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // check that second call to SdmMessageBodyFactory returns the same bodies.
  for (i=1; i<= body_count; i++) {      
    if (message->SdmMessageBodyFactory(err, body, i)) {
      cout << "*** Error: SdmMessageBodyFactory2 failed for body "
           << i << ". err = " << (int)err << "\n";
      return err;
    }
    if (bodyList[i-1] != body) {
      cout << "*** Error: SdmMessageBodyFactory returns new body for duplicate calls. body = "
           << i <<  "\n";
      err = Sdm_EC_Fail;
      return err;
    }
  }

  return err;
}

SdmErrorCode
SdmMessageTest::TestMessageEnvelopeFactory(SdmMessage *message) 
{
  SdmError err;
  SdmMessageEnvelope *envelope, *envelope2;
  if (message->SdmMessageEnvelopeFactory(err, envelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory fails.\n";
    return err;
  }
  
  // check that same envelope is returned for multiple calls.
  if (message->SdmMessageEnvelopeFactory(err, envelope2)) {
    cout << "*** Error: SdmMessageEnvelopeFactory2 fails.\n";
    return err;
  }
  if (envelope != envelope2) {
    cout << "*** Error: different envelopes returned for a message.\n";
    return err;
  }
  return err;
}

SdmErrorCode
SdmMessageTest::TestNestedMessage(SdmMessage *message, SdmMessage *parentMsg) 
{
  SdmError localError, err;
  SdmMessageFlagAbstractFlags r_flags, aflags, flag;
  SdmMessage *parent;

  flag = Sdm_MFA_Draft;
  aflags = (Sdm_MFA_Answered|Sdm_MFA_Seen);

  // check parent of nested message.
  if (message->Parent(err, parent)) {
    cout << "*** Error: Parent fails in returning parent message.\n";
    return err;
  }
  
  if (parent != parentMsg) {
    cout << "*** Error: Parent fails returns incorrect parent msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  SdmMessageNumber msgnum, parentMsgnum;
  if (message->GetMessageNumber(err, msgnum)) {
    cout << "*** Error: Parent fails in returning parent message.\n";
    return err;
  }
  if (parentMsg->GetMessageNumber(err, parentMsgnum)) {
    cout << "*** Error: Parent fails in returning parent message.\n";
    return err;
  }
  if (parentMsgnum != msgnum) {
    cout << "*** Error: Mesage number of nested msg not equal to that of parent.\n";
    err = Sdm_EC_Fail;
    return err;
  }

   
  SdmMessageEnvelope *envelope;
  
  // SdmMessageEnvelopeFactory should return error for nested message.
  if (message->SdmMessageEnvelopeFactory(err, envelope)) {
    if (err != Sdm_EC_MessageIsNested) { 
      cout << "*** Error: SdmMessageEnvelopeFactory returns incorrect error for nested msg.\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: SdmMessageEnvelopeFactory doesn't return error for nested msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // Flag methods should return error for nested message. 
  if (message->GetFlags(err, r_flags)) {
    if (err != Sdm_EC_MessageIsNested) { 
      cout << "*** Error: GetFlags returns incorrect error for nested msg.\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: GetFlags doesn't return error for nested msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (message->ClearFlags(err, flag)) {
    if (err != Sdm_EC_MessageIsNested) { 
      cout << "*** Error: ClearFlags returns incorrect error for nested msg.\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: ClearFlags doesn't return error for nested msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (message->SetFlags(err, aflags)) {
    if (err != Sdm_EC_MessageIsNested) { 
      cout << "*** Error: SetFlags returns incorrect error for nested msg.\n";
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: SetFlags doesn't return error for nested msg.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // we can run the same MessageBodyFactory test for the nested message.
  SdmErrorCode errCode;
  if ((errCode = TestMessageBodyFactory(message)) != Sdm_EC_Success) {
    err = errCode;
    return err;
  }

  return err;
}
  
SdmErrorCode
SdmMessageTest::TestGetContents(SdmMessage *message, SdmReplyQueue* frontEndQueue,
  int readFileDesc) 
{
  SdmError err;
  SdmContentBuffer r_contents;
  
  // first test sync call.
  if (message->GetContents(err, r_contents)) {
    cout << "*** Error: GetContents failed.\n";
    return err;
  }
  
  if (r_contents.GetContents(err, lastContentBuffer)) {
    cout << "*** Error: can't get contents of content buffer.\n";
    return err;
  }
  lastMessage = message;
  getContentsCallbackError = Sdm_False;
  getContentsCallbackCalled = Sdm_False;
  
  SdmServiceFunction svf(&GetContentsCallback,
        (void*)SdmMessageTest::kUserData, Sdm_SVF_Other);

  // test async call.
  if (message->GetContents_Async(err, svf, (void*)kUserData)){
    cout << "*** Error: GetContents_Async failed. err = " << (int)err << "\n";
    return err;
  }
  
  if (PollForReply(frontEndQueue, readFileDesc, 1)) {
    return err;
  }
 
  if (!getContentsCallbackCalled) {
    cout << "*** Error: GetContents callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (getContentsCallbackError) {
    cout << "*** Error: error occurred in GetContents callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  return err;
}

//
// Callbacks
//
void 
SdmMessageTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmMessageTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
  
  if (type == Sdm_SVF_SolicitedEvent) {
    cout << "*** Error: type in call back is not correct.\n";
    getContentsCallbackError = Sdm_True;
  }

  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }

}
 
void 
SdmMessageTest::AsyncCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  asyncCallbackCalled = Sdm_True;
 
  cout << "AsyncCallback Called.\n";
  if (userdata != SdmMessageTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    asyncCallbackError = Sdm_True;
  }
 
  if (type != Sdm_SVF_SolicitedEvent) {
    cout << "*** Error: type in call back is not correct.\n";
    getContentsCallbackError = Sdm_True;
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
          if (*error) {
              cout << "*** Error: error returned from async call. err = "
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
SdmMessageTest::GetContentsCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  getContentsCallbackCalled = Sdm_True;
 
  cout << "getContentsCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    getContentsCallbackError = Sdm_True;
  }

  if (type != Sdm_SVF_SolicitedEvent) {
    cout << "*** Error: type in call back is not correct.\n";
    getContentsCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_GetMessageContents:
      {
        // check content buffer in event.           
        SdmContentBuffer *buffer= event->sdmGetContents.buffer;
        if (buffer == NULL) {
          cout << "*** Error: buffer in event is null.\n";
          getContentsCallbackError = Sdm_True;
        } else {
          SdmString str_contents;
          if (event->sdmGetContents.buffer->GetContents(err, str_contents)) {
             cout << "*** Error: can't get string from content buffer.\n";
             getContentsCallbackError = Sdm_True;
          } 
         
          if (str_contents != lastContentBuffer) {
            cout << "*** Error: buffer is not equal to last contents retrieved.\n";
            getContentsCallbackError = Sdm_True;
          }
	  delete buffer;
        }
        
        // check body part number and message number in event.
        if (event->sdmGetContents.messageNumber != lastMessage->_MessgNumber) {
          cout << "*** Error: message number does not match in event.\n";
          getContentsCallbackError = Sdm_True;
        }
        if (event->sdmGetContents.bodyPartNumber != -1) {
          cout << "*** Error: body part number is not -1  in event.\n";
          getContentsCallbackError = Sdm_True;
        }
        
        // check error in event
        if (event->sdmGetContents.error == NULL) {
          cout << "*** Error: error is NULL in event.\n";
          getContentsCallbackError = Sdm_True;        
        } else if (*(event->sdmGetContents.error)) {
          cout << "*** Error: error is set in event.\n";
          getContentsCallbackError = Sdm_True;
        }
        break;
      }
 
    default:
      cout << "*** Error: getContentsCallback default event received.\n";
      getContentsCallbackError = Sdm_True;
      break;
  }
}

