/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the composing new messages.
 

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
#include <DataStructs/MutexEntry.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <SDtMail/SystemUtility.hh>


static  const char* kOutgoingStoreName = "/tmp/outgoingmsg.test";


class SdmOutgoingMessageTest
{
public:
  SdmOutgoingMessageTest();
  virtual ~SdmOutgoingMessageTest();

  virtual SdmErrorCode Run();

private:
  static const char* kUserData;

  SdmErrorCode Setup();
  
  SdmErrorCode CreateNewMessage(SdmError &err, SdmMessage *&newmsg);
  SdmErrorCode CreateNestedMessage(SdmError &err, SdmMessage *&newmsg);
  SdmErrorCode CreateKoreanMessage(SdmError &err, SdmMessage *&newmsg);
  SdmErrorCode CreateJapaneseMessage(SdmError &err, SdmMessage *&newmsg);
  SdmErrorCode TestAsyncSubmit();
  SdmErrorCode TestSubmit();

  SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);

  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void SubmitCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
 
  static SdmBoolean submitCallbackCalled;
  static SdmBoolean submitCallbackError;
  static SdmBoolean sessionCallbackCalled;

  SdmConnection *mcon;
  SdmSession *oses;
  SdmMessageStore *dbox;
  
  static mutex_t gMessageNumberListLock;
  static SdmMessageNumberL gSentMessages;

  int o_readFileDesc;
  SdmReplyQueue *o_frontEndQueue;
};

SdmBoolean SdmOutgoingMessageTest::submitCallbackCalled;
SdmBoolean SdmOutgoingMessageTest::submitCallbackError;
SdmBoolean SdmOutgoingMessageTest::sessionCallbackCalled;
mutex_t SdmOutgoingMessageTest::gMessageNumberListLock;
SdmMessageNumberL SdmOutgoingMessageTest::gSentMessages;
const char* SdmOutgoingMessageTest::kUserData = "compose test data";

SdmErrorCode RunOutgoingMessageTest()
{
  cout << endl << "************RunOutgoingMessageTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmOutgoingMessageTest test;
  return test.Run();
}


SdmOutgoingMessageTest::SdmOutgoingMessageTest()
{
}
 
SdmOutgoingMessageTest::~SdmOutgoingMessageTest()
{
}

SdmErrorCode
SdmOutgoingMessageTest::Run()
{
  SdmError err;

  cout << endl << "************ SdmOutgoingMessageTest::Run Invoked. *************" << endl << endl;
  cout << endl;
 
  SdmErrorCode errorCode;

 
  if ((errorCode = Setup()) != Sdm_EC_Success)
    return errorCode;
 
 
  // ***************************************************************8
  // Testing Message
  // ***************************************************************8
 
  cout << "Testing Submit.\n";

  if ((errorCode = TestSubmit()) != Sdm_EC_Success)
    return errorCode;

  cout << "Testing Async Submit.\n";

  if ((errorCode = TestAsyncSubmit()) != Sdm_EC_Success)
     return errorCode;
     
  delete mcon;

  // Sessions should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(o_frontEndQueue, &o_readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
 
  // remove temp store file.
  unlink(kOutgoingStoreName);
  
  return Sdm_EC_Success;
}


SdmErrorCode
SdmOutgoingMessageTest::Setup()
{
  SdmError err;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;
  
  mutex_init(&gMessageNumberListLock, USYNC_THREAD, NULL);

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp outgoingmessage.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  SdmString kServiceOption("serviceoption");
  SdmString kServiceType("servicetype");
  SdmString kServiceClass("serviceclass");

  mcon = new SdmConnection("TestSession");
  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }
 

  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, oses, Sdm_ST_OutSession)) {
    cout << "*** Error: create outgoing session failed\n";
    return err;
  }
  if (oses->StartUp(err)) {
    cout << "*** Error: Sdm outgoing Session StartUp Failed\n" ;
    return err;
  }

  // attach to session.
  void *vptr;
  if (SdmAttachSession(err, o_readFileDesc, vptr, *oses)) {
    cout << "*** Error: attach outgoing session failed.\n";
    return err;
  }
  o_frontEndQueue = (SdmReplyQueue*)vptr;

  // create and start up message store.
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cout << "*** Error: create outgoing store failed\n";
     return err;
  }
  if (dbox->StartUp(err)) {
    cout << "*** Error: outgoing store StartUp Failed\n" ;
    return err;
  }

  SdmToken outtoken;
  outtoken.SetValue(kServiceClass,SdmString("messagestore"));
  outtoken.SetValue(kServiceType,SdmString("local"));
  outtoken.SetValue(kServiceOption,SdmString("messagestorename"), SdmString(kOutgoingStoreName));

  if (dbox->Open(err, nmsgs, readOnly, outtoken)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }
  
  return err;
}



SdmErrorCode 
SdmOutgoingMessageTest::CreateNewMessage(SdmError &err, SdmMessage *&newmsg)
{
  err = Sdm_EC_Success;
    
  // create new message and new body.  get new envelope for new message.
  if (dbox->SdmMessageFactory(err, newmsg)) {
    cout << "*** Error: SdmMessageFactory failed for new message.\n";
    return err;
  }
  
  SdmMessageBody *newbody;
  if (newmsg->SdmMessageBodyFactory(err, newbody, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for new message.\n";
    return err;
  }
  SdmMessageEnvelope *newenvelope;
  if (newmsg->SdmMessageEnvelopeFactory(err, newenvelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed for new message.\n";
    return err;
  }
  
  // set contents of new body.
  SdmString newContents("new message contents");
  if (newbody->SetContents(err, newContents)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;        
  }
  
  // check body count of new message.
  int bodyCount;
  if (newmsg->GetBodyCount(err, bodyCount)) {
    cout << "*** Error: can't get body count for new message.\n";
    return err;    
  }
  if (bodyCount != 1) {
    cout << "*** Error: new message has invalid body count.\n";
    return err;    
  }

  passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);
  if (pw.pw_name == NULL) {
    cout << "*** Error: can not get user name.\n";
    err = Sdm_EC_Fail;
    return err;        
  }
  
  
  // set headers for new message.
  SdmStrStrL newHeaders;
  newHeaders.AddElementToList(SdmStrStr("from", pw.pw_name));
  newHeaders.AddElementToList(SdmStrStr("to", pw.pw_name));
  newHeaders.AddElementToList(SdmStrStr("subject", "outgoing message test"));
  newHeaders.AddElementToList(SdmStrStr("X-Mustard", "Please pass the Grey Poupon!"));
  newHeaders.AddElementToList(SdmStrStr("X-Errno", "Juste en cas, mon ami!"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  return err;
}


SdmErrorCode 
SdmOutgoingMessageTest::CreateKoreanMessage(SdmError &err, SdmMessage *&newmsg)
{
  err = Sdm_EC_Success;
    
  // create new message and new body.  get new envelope for new message.
  if (dbox->SdmMessageFactory(err, newmsg)) {
    cout << "*** Error: SdmMessageFactory failed for new message.\n";
    return err;
  }
  
  SdmMessageBody *newbody;
  if (newmsg->SdmMessageBodyFactory(err, newbody, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for new message.\n";
    return err;
  }
  SdmMessageEnvelope *newenvelope;
  if (newmsg->SdmMessageEnvelopeFactory(err, newenvelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed for new message.\n";
    return err;
  }
  
  // set contents of new body.
  SdmString newContents(
"\
GQ1[ @|@Z?lFm@; test GO0m @V@=.\
@L ?lFm@; 9+=CGO1b9Y6x4O4Y. Thx.");


  if (newbody->SetContents(err, newContents)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;        
  }
  
  // check body count of new message.
  int bodyCount;
  if (newmsg->GetBodyCount(err, bodyCount)) {
    cout << "*** Error: can't get body count for new message.\n";
    return err;    
  }
  if (bodyCount != 1) {
    cout << "*** Error: new message has invalid body count.\n";
    return err;    
  }
  
  // set headers for new message.
  SdmStrStrL newHeaders;
  newHeaders.AddElementToList(SdmStrStr("from", "sara"));
  newHeaders.AddElementToList(SdmStrStr("to", "sara"));
  newHeaders.AddElementToList(SdmStrStr("subject", 
"=?EUC-KR?B?x9Gx2w==?= =?EUC-KR?B?wPzA2r/sxu0=?= roundtrip =?EUC-KR?B?vcfH6A==?="));
  newHeaders.AddElementToList(SdmStrStr("X-Mustard", "Please pass the Grey Poupon!"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  return err;
}


SdmErrorCode 
SdmOutgoingMessageTest::CreateJapaneseMessage(SdmError &err, SdmMessage *&newmsg)
{
  err = Sdm_EC_Success;
    
  // create new message and new body.  get new envelope for new message.
  if (dbox->SdmMessageFactory(err, newmsg)) {
    cout << "*** Error: SdmMessageFactory failed for new message.\n";
    return err;
  }
  
  SdmMessageBody *newbody;
  if (newmsg->SdmMessageBodyFactory(err, newbody, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for new message.\n";
    return err;
  }
  SdmMessageEnvelope *newenvelope;
  if (newmsg->SdmMessageEnvelopeFactory(err, newenvelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed for new message.\n";
    return err;
  }
  
  // set contents of new body.
  SdmString newContents("後藤と申します。サブジェクト直しました。(^_^;)");


  if (newbody->SetContents(err, newContents)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;        
  }
  
  // check body count of new message.
  int bodyCount;
  if (newmsg->GetBodyCount(err, bodyCount)) {
    cout << "*** Error: can't get body count for new message.\n";
    return err;    
  }
  if (bodyCount != 1) {
    cout << "*** Error: new message has invalid body count.\n";
    return err;    
  }
  
  // set headers for new message.
  SdmStrStrL newHeaders;
  newHeaders.AddElementToList(SdmStrStr("from", "sara"));
  newHeaders.AddElementToList(SdmStrStr("to", "sara"));
  newHeaders.AddElementToList(SdmStrStr("subject",
"[mime-ml:00201] Re: ＭＩＭＥメールヘ完伽ー"));
  newHeaders.AddElementToList(SdmStrStr("X-Mustard", "Please pass the Grey Poupon!"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  return err;
}


SdmErrorCode 
SdmOutgoingMessageTest::CreateNestedMessage(SdmError &err, SdmMessage *&newmsg)
{
  err = Sdm_EC_Success;
  
  // create new message.  get new envelope for new message.
  if (dbox->SdmMessageFactory(err, newmsg)) {
    cout << "*** Error: SdmMessageFactory failed for new message.\n";
    return err;
  }
  SdmMessageEnvelope *newenvelope;
  if (newmsg->SdmMessageEnvelopeFactory(err, newenvelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed for new message.\n";
    return err;
  }

  passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);
  if (pw.pw_name == NULL) {
    cout << "*** Error: can not get user name.\n";
    err = Sdm_EC_Fail;
    return err;        
  }
  
 
  // set headers for new message.
  SdmStrStrL newHeaders;
  newHeaders.AddElementToList(SdmStrStr("from", pw.pw_name));
  newHeaders.AddElementToList(SdmStrStr("to", pw.pw_name));
  newHeaders.AddElementToList(SdmStrStr("subject", "outgoing nested message test"));
  newHeaders.AddElementToList(SdmStrStr("X-Nested", "Grab some bandwidth"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;       
  }
  
  // create multipart body and nested message from it.
  SdmMessageBody *newbody;
  if (newmsg->SdmMessageBodyFactory(err, newbody, Sdm_MSTYPE_multipart, SdmString("mixed"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for new message.\n";
    return err;
  }    
  SdmMessage *nestedMsg;
  if (newbody->SdmMessageFactory(err, nestedMsg)) {
    cout << "*** Error: SdmMessageFactory failed for nested message.\n";
    return err;
  }  


  // create first body in nested message.  set it's contents and check results.
  SdmMessageBody *nestedBody;
  if (nestedMsg->SdmMessageBodyFactory(err, nestedBody, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for nested message.\n";
    return err;
  }
  SdmString newContents("new message contents in body 1");
  if (nestedBody->SetContents(err, newContents)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;
  }
   
  // create second body in nested message.  set it's contents and check results.
  SdmMessageBody *nestedBody2;
  if (nestedMsg->SdmMessageBodyFactory(err, nestedBody2, Sdm_MSTYPE_application, SdmString("octet-stream"))) {
    cout << "*** Error: SdmMessageBodyFactory2 failed for nested message.\n";
    return err;
  }
  SdmString newContents2("new message contents in body 2");
  if (nestedBody2->SetContents(err, newContents2)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;
    err = Sdm_EC_Success;
  }
 
  // set the name of the attachment.
  SdmMessageBodyStructure r_bodyStruct;
  if (nestedBody2->GetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: GetStructure failed.\n";
    return err;
  }
  r_bodyStruct.mbs_attachment_name = "TheAttachment";
  if (nestedBody2->SetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: SetStructure failed.\n";
    return err;
  }
  
  return err;
}

SdmErrorCode
SdmOutgoingMessageTest::TestSubmit()
{
  SdmError err;
  err = Sdm_EC_Success;
  SdmMessage *newmsg;
  
  if (CreateNewMessage(err, newmsg)) {
    return err;
  }

#ifdef I18N
  if (CreateKoreanMessage(err, newmsg)) {
    return err;
  }

// I18N
  if (CreateJapaneseMessage(err, newmsg)) {
    return err;
  }
#endif // I18N

  SdmMsgFormat format = Sdm_MSFMT_SunV3;

  SdmStringL customHeaders;
  customHeaders(-1) = "X-Mustard";
  customHeaders(-1) = "X-Errno";
  customHeaders(-1) = "X-Nested";

  SdmDeliveryResponse r_response;
  if (newmsg->Submit(err, r_response, format, Sdm_True, customHeaders)) {
    cout << "*** Error: Submit fails.\n";
    return err;    
  }


  if (CreateNestedMessage(err, newmsg)) {
    return err;
  }

  format = Sdm_MSFMT_Mime;
  if (newmsg->Submit(err, r_response, format, Sdm_True, customHeaders)) {
    cout << "*** Error: Submit fails.\n";
    return err;    
  }

  if (CreateNestedMessage(err, newmsg)) {
    return err;
  }

  format = Sdm_MSFMT_SunV3;
  if (newmsg->Submit(err, r_response, format, Sdm_True, customHeaders)) {
    cout << "*** Error: Submit fails.\n";
    return err;    
  }

  return err;
}


SdmErrorCode 
SdmOutgoingMessageTest::TestAsyncSubmit()
{
  SdmError err;
  err = Sdm_EC_Success;
  static int kTotalMessages = 2;
  SdmMessageNumber msgno;
  SdmMsgFormat  format = Sdm_MSFMT_Mime;
  SdmMessage *newmsg;
  SdmVector<SdmMessage *> messageList;


  SdmServiceFunction svf(&SubmitCallback,
        (void*)SdmOutgoingMessageTest::kUserData, Sdm_SVF_Other);
  
  for (int i=0; i< kTotalMessages; i++) {
  
    if (CreateNewMessage(err, newmsg)) {
      return err;
    }

    if (newmsg->GetMessageNumber(err, msgno)) {
      cout << "*** Errror: can't get message number for new message.\n";
      return err;
    }

    {
      SdmMutexEntry entry(gMessageNumberListLock);
      gSentMessages.AddElementToList(msgno);
    }
    
    messageList.AddElementToList(newmsg);
  }

  submitCallbackError = Sdm_False;
  submitCallbackCalled = Sdm_False;

  SdmStringL customHeaders;
  customHeaders(-1) = "X-Mustard";
  customHeaders(-1) = "X-Errno";

  for (i=0; i<kTotalMessages; i++) {
    if (messageList[i]->Submit_Async(err, svf, (void*)kUserData, format,
                                     Sdm_False, customHeaders)) {
      cout << "*** Error: Submit_Async fails.\n";
      return err;    
    }
  }

  if (PollForReply(o_frontEndQueue, o_readFileDesc, kTotalMessages)) {
    return err;
  }
 
  if (!submitCallbackCalled) {
    cout << "*** Error: GetContents callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (submitCallbackError) {
    cout << "*** Error: error occurred in Submit callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  {
    SdmMutexEntry entry(gMessageNumberListLock);
    if (gSentMessages.ElementCount() != 0) {
      cout << "*** Error: message list not empty after all async calls completed.\n";
      err = Sdm_EC_Fail;
      return err;
    } 
  }
  

  return err;
}



SdmErrorCode
SdmOutgoingMessageTest::PollForReply(SdmReplyQueue *frontEndQueue,
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
 
  if (i != expectedCalls && i >= 0) {
    sprintf(buffer, "*** Error: Did not get expected number of replies.  expected=%d, received = %d.\n",
      expectedCalls, i);
    cout << buffer;
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

//
// Callbacks
//
void
SdmOutgoingMessageTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmOutgoingMessageTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
 
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }
}
 
void
SdmOutgoingMessageTest::SubmitCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  int index;
  SdmMessageNumber msgno;
  submitCallbackCalled = Sdm_True;
 
  cout << "SubmitCallback Called.\n";
 
  // check userdata in event.
  if (userdata != SdmOutgoingMessageTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    submitCallbackError = Sdm_True;
  }
  
 
  switch (event->sdmBase.type) {
    case Sdm_Event_SubmitMessage:
      // check message number in event.
      msgno = event->sdmSubmitMessage.messageNumber;
      if (msgno >= 0) {
        cout << "*** Error: message number not negative.\n";
        submitCallbackError = Sdm_True;
      }

      cout << "SubmitCallback Called for message " << msgno << ".\n";
      
      {
        SdmMutexEntry entry(gMessageNumberListLock);
        index = gSentMessages.FindElement(msgno);

        if (index < 0) {
          cout << "*** Error: message number not found in sent list.  msgno =  " << msgno << "\n";
          submitCallbackError = Sdm_True;
        } else {
          gSentMessages.RemoveElement(index);
        }
      }

      // check error in event
      if (event->sdmSubmitMessage.error == NULL) {
        cout << "*** Error: error is NULL in event.\n";
        submitCallbackError = Sdm_True;
      } else if (*(event->sdmSubmitMessage.error)) {
        cout << "*** Error: error is set in event.\n";
        submitCallbackError = Sdm_True;
      }

      if (event->sdmSubmitMessage.response == NULL) {
        cout << "*** Error: response is NULL in event.\n";
        submitCallbackError = Sdm_True;
      } else {
        delete (event->sdmSubmitMessage.response);
      }
      break;
    
    default:
      cout << "*** Error: SubmitCallback default event received.\n";
      submitCallbackError = Sdm_True;
      break;
  }
}
 
 

