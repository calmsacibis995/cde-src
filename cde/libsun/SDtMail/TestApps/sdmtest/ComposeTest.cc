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
#include <SDtMail/SystemUtility.hh>

static  const char* kOutgoingStoreName = "/tmp/compose.test";


class SdmComposeTest
{
public:
  SdmComposeTest();
  virtual ~SdmComposeTest();

  virtual SdmErrorCode Run();

private:
  static const char* kUserData;

  SdmErrorCode Setup();
  SdmErrorCode Cleanup();
  
  SdmErrorCode TestNewMessage(SdmBoolean);
  SdmErrorCode TestEditedMessage();
  SdmErrorCode TestDerivedMessage(SdmBoolean);
  SdmErrorCode TestNestedMessage(SdmBoolean);
  SdmErrorCode TestAddRemoveReplaceHeaders(SdmError &err, SdmMessageEnvelope *envelope);

  SdmErrorCode CheckContents(SdmError &err, SdmMessage* msg, SdmContentBuffer &correctContents);
  SdmErrorCode CheckContents(SdmError &err, SdmMessageBody* body, SdmString &correctContents);
  SdmErrorCode CheckHeaders(SdmError &err, SdmMessageEnvelope* env, SdmStrStrL &correctHdrs);

  SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
  void PrintHeaders(const SdmStrStrL &headers);

  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void SetContentsCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
 
  static SdmBoolean setContentsCallbackCalled;
  static SdmBoolean setContentsCallbackError;
  static SdmBoolean sessionCallbackCalled;

  SdmConnection *mcon;
  SdmSession *ises, *oses;
  SdmMessageStore *mbox, *dbox;

  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
  static int lastBodyPartNumber;
  static int lastMessageNumber;

};

SdmBoolean SdmComposeTest::setContentsCallbackCalled;
SdmBoolean SdmComposeTest::setContentsCallbackError;
SdmBoolean SdmComposeTest::sessionCallbackCalled;
int SdmComposeTest::lastBodyPartNumber;
int SdmComposeTest::lastMessageNumber;

const char* SdmComposeTest::kUserData = "compose test data";

SdmErrorCode RunComposeTest()
{
  cout << endl << "************RunComposeTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmComposeTest test;
  return test.Run();
}


SdmComposeTest::SdmComposeTest()
{
}
 
SdmComposeTest::~SdmComposeTest()
{
}

SdmErrorCode
SdmComposeTest::Run()
{
  SdmError err;

  cout << endl << "************ SdmComposeTest::Run Invoked. *************" << endl << endl;
  cout << endl;
 
  SdmErrorCode errorCode;

 
  if ((errorCode = Setup()) != Sdm_EC_Success)
    return errorCode;
 
 
  // ***************************************************************8
  // Testing Message
  // ***************************************************************8
 
  cout << "Testing edit existing message.\n";

  if ((errorCode = TestEditedMessage()) != Sdm_EC_Success)
    return errorCode;


  cout << "Testing derived message.\n";

  if ((errorCode = TestDerivedMessage(Sdm_False)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestDerivedMessage(Sdm_True)) != Sdm_EC_Success)
    return errorCode;


  cout << "Testing new message.\n";

  if ((errorCode = TestNewMessage(Sdm_False)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestNewMessage(Sdm_True)) != Sdm_EC_Success)
    return errorCode;


  cout << "Testing nested message.\n";

  if ((errorCode = TestNestedMessage(Sdm_False)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestNestedMessage(Sdm_True)) != Sdm_EC_Success)
    return errorCode;


  if ((errorCode = Cleanup()) != Sdm_EC_Success)
    return errorCode;

  return Sdm_EC_Success;
}


SdmErrorCode
SdmComposeTest::Setup()
{
  SdmError err;

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp compose.test %s", kOutgoingStoreName);
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
  if (mcon->SdmSessionFactory(err, ises, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed\n";
    return err;
  }
  if (ises->StartUp(err)) {
    cout << "*** Error: Sdm Incoming Session StartUp Failed\n" ;
    return err;
  }
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
  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises)) {
    cout << "*** Error: attach incoming session failed.\n";
    return err;
  }
  i_frontEndQueue = (SdmReplyQueue*)vptr;
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
  if (mbox->StartUp(err)) {
    cout << "*** Error: Message store StartUp Failed\n" ;
    return err;
  }
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cout << "*** Error: create outgoing store failed\n";
     return err;
  }
  if (dbox->StartUp(err)) {
    cout << "*** Error: outgoing store StartUp Failed\n" ;
    return err;
  }

  // register default service function for any service type at the session level.
  SdmServiceFunction svf(&SessionCallback,
          (void*)SdmComposeTest::kUserData, Sdm_SVF_Other);
 
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
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  token.SetValue(kServiceClass,SdmString("messagestore"));
  token.SetValue(kServiceType,SdmString("local"));
  token.SetValue(kServiceOption,SdmString("messagestorename"), SdmString(kOutgoingStoreName));
 
  if (mbox->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }
  
  return err;
}




SdmErrorCode
SdmComposeTest::Cleanup()
{
  SdmError err;

  //
  // close and shutdown objects
  //
  if (mbox->Close(err)) {
    cout << "*** Error: SdmMessageStore close failed.\n";
    return err;
  }

  if (mbox->ShutDown(err)) {
    cout << "*** Error: SdmMessageStore shutdown failed.\n";
    return err;
  }

  if (ises->ShutDown(err)) {
    cout << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (mcon->ShutDown(err)) {
    cout << "*** Error: SdmConnection shutdown failed.\n";
    return err;
  }

  // clean up connection.
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

  return err;
}

SdmErrorCode 
SdmComposeTest::TestNewMessage(SdmBoolean commit)
{
  SdmError err;
  
  // create new message and new body.  get new envelope for new message.
  SdmMessage *newmsg;
  if (mbox->SdmMessageFactory(err, newmsg)) {
    cout << "*** Error: SdmMessageFactory failed for new message.\n";
    return err;
  }

  SdmMessageNumber msgno;
  if (newmsg->GetMessageNumber(err, msgno)) {
    cout << "*** Error: GetMessageNumber failed for new messgae.\n";
    return err;
  }
 
  // check that same pointer is returned if factory method is called again.
  SdmMessage *samemsg;
  if (mbox->SdmMessageFactory(err, samemsg, msgno)) {
    cout << "*** Error: SdmMessageFactory failed to get second message.\n";
    return err;
  }
  if (newmsg != samemsg) {
    cout << "*** Error: wrong message returned for new message.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmMessageBody *newbody;
  if (newmsg->SdmMessageBodyFactory(err, newbody, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory failed for new message.\n";
    return err;
  }
  int bodyNumber;
  if (newbody->GetBodyPartNumber(err, bodyNumber)) {
    cout << "*** Error: GetBodyPartNumber failed.\n";
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
  
  // check contents of body.
  if (CheckContents(err, newbody, newContents)) {
    return err;
  }

  lastMessageNumber = msgno;
  lastBodyPartNumber = bodyNumber;
  setContentsCallbackError = Sdm_False;
  setContentsCallbackCalled = Sdm_False;
  
  SdmServiceFunction svf(&SetContentsCallback,
        (void*)SdmComposeTest::kUserData, Sdm_SVF_Other);

  // test async call.
  newContents = "new message contests update.";
  if (newbody->SetContents_Async(err, svf, (void*)kUserData, newContents)) {
    cout << "*** Error: SetContents_Async failed.\n";
    return err;        
  }

  if (PollForReply(i_frontEndQueue, i_readFileDesc, 1)) {
    return err;
  }

  // check contents of body.
  if (CheckContents(err, newbody, newContents)) {
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
  newHeaders.AddElementToList(SdmStrStr("subject", "compose test"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  // check new headers.
  if (CheckHeaders(err, newenvelope, newHeaders)) {
    return err;
  }
  
  // Test AddHeader, RemoveHeader, and ReplaceHeader methods in SdmMessageEnvelope.
  if (TestAddRemoveReplaceHeaders(err, newenvelope)) {
    return err;
  }

  if (commit) {
    SdmBoolean r_disposition;
    if (newmsg->CommitPendingChanges(err, r_disposition)) {
      cout << "*** Error: CommitPendingChanges fail for new msg.\n"; 
      return err;  
    }
    return err;
  } else {
    if (newmsg->DiscardPendingChanges(err)) {
      cout << "*** Error: DiscardPendingChanges fail for new msg.\n"; 
      return err;  
    }
  }



  // regardless of whether the message was committed or disgarded,
  // the new msg# should no longer be valid.  Call something and make sure
  // an error of Sdm_EC_BadMessageNumber is returned.
  SdmMessageFlagAbstractFlags r_flags;
  if (newmsg->GetFlags(err, r_flags)) {
    if (err == Sdm_EC_BadMessageNumber) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: GetFlags fails for committed new message.\n"; 
      return err;
    }
  } else {
    err = Sdm_EC_Fail;
    cout << "*** Error: GetFlags succeeds after new message committed.\n"; 
    return err;
  }

  return err;
}


SdmErrorCode
SdmComposeTest::TestAddRemoveReplaceHeaders(SdmError &err, SdmMessageEnvelope *envelope)
{
  static const SdmString kTestHeader("X-EnvelopeTestHeader");
  static const SdmString kTestHeaderValue("this is a test header");
  static const SdmString kNewTestHeaderValue("this is a new test header");
  static const SdmString kReallyNewTestHeaderValue("this is a really new test header");
  SdmStrStrL r_hdrL;  

  // test AddHeader
  if (envelope->AddHeader(err, kTestHeader, kTestHeaderValue)) {
    cout << "Error: AddHeader fails\n";
    return err;
  }
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    cout << "Error: GetHeader fails\n";
    return err;
  }
  if (r_hdrL.ElementCount() != 1) {
    cout << "Error: can't get test header\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetFirstString() != kTestHeader) {
    cout << "Error: incorrect test header in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetSecondString() != kTestHeaderValue) {
    cout << "Error: incorrect test header value in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }    
 

  // test ReplaceHeader
  if (envelope->ReplaceHeader(err, kTestHeader, kNewTestHeaderValue)) {
    cout << "Error: ReplaceHeader2 fails\n";
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    cout << "Error: GetHeader fails\n";
    return err;
  }
  if (r_hdrL.ElementCount() != 1) {
    cout << "Error: can't get test header\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetFirstString() != kTestHeader) {
    cout << "Error: incorrect test header in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetSecondString() != kNewTestHeaderValue) {
    cout << "Error: incorrect test header value in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }    
 

   // test remove header
  if (envelope->RemoveHeader(err, kTestHeader)) {
    cout << "Error: RemoveHeader fails\n";
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    if (err != Sdm_EC_RequestedDataNotFound) {
      cout << "Error: GetHeader fails\n";
      return err;
    }
  } else {
    cout << "Error: removed header found in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }    
    
  // test add header for multiple headers
  if (envelope->AddHeader(err, kTestHeader, kTestHeaderValue)) {
    cout << "Error: AddHeader fails\n";
    return err;
  }
  if (envelope->AddHeader(err, kTestHeader, kNewTestHeaderValue)) {
    cout << "Error: AddHeader fails\n";
    return err;
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    cout << "Error: GetHeader fails\n";
    return err;
  }
  if (r_hdrL.ElementCount() != 2) {
    cout << "Error: can't get test header\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetFirstString() != kTestHeader && r_hdrL[1].GetFirstString() != kTestHeader) {
    cout << "Error: incorrect test header in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if ( (r_hdrL[0].GetSecondString() == kTestHeaderValue && r_hdrL[1].GetSecondString() == kNewTestHeaderValue) ||
              (r_hdrL[1].GetSecondString() == kTestHeaderValue && r_hdrL[0].GetSecondString() == kNewTestHeaderValue) )
  {
    err = Sdm_EC_Success;
  } else {
    cout << "Error: incorrect test header value in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }


   // test remove header for multiple headers
  if (envelope->RemoveHeader(err, kTestHeader)) {
    cout << "Error: RemoveHeader fails\n";
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    if (err != Sdm_EC_RequestedDataNotFound) {
      cout << "Error: GetHeader fails\n";
      return err;
    }
  } else {
    cout << "Error: removed header found in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }    

  
  // add multiple headers again.
  if (envelope->AddHeader(err, kTestHeader, kTestHeaderValue)) {
    cout << "Error: AddHeader fails\n";
    return err;
  }
  if (envelope->AddHeader(err, kTestHeader, kNewTestHeaderValue)) {
    cout << "Error: AddHeader fails\n";
    return err;
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    cout << "Error: GetHeader fails\n";
    return err;
  }
  if (r_hdrL.ElementCount() != 2) {
    cout << "Error: can't get test header\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // test replace header for multiple headers
  if (envelope->ReplaceHeader(err, kTestHeader, kReallyNewTestHeaderValue)) {
    cout << "Error: ReplaceHeader2 fails\n";
  }
  r_hdrL.ClearAllElements();
  if (envelope->GetHeader(err, r_hdrL, kTestHeader)) {
    cout << "Error: GetHeader fails\n";
    return err;
  }
  if (r_hdrL.ElementCount() != 1) {
    cout << "Error: can't get test header\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetFirstString() != kTestHeader) {
    cout << "Error: incorrect test header in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (r_hdrL[0].GetSecondString() != kReallyNewTestHeaderValue) {
    cout << "Error: incorrect test header value in envelope.\n";
    err = Sdm_EC_Fail;
    return err;
  }    

  return err;
}

SdmErrorCode 
SdmComposeTest::TestNestedMessage(SdmBoolean commit)
{
  SdmError err;
  
  // create new message.  get new envelope for new message.
  SdmMessage *newmsg;
  if (mbox->SdmMessageFactory(err, newmsg)) {
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
  newHeaders.AddElementToList(SdmStrStr("subject", "compose nested message test"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;       
  }

  // check new headers.
  if (CheckHeaders(err, newenvelope, newHeaders)) {
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
  // check contents of body.
  if (CheckContents(err, nestedBody, newContents)) {
    return err;
  }

  // create second body in nested message.  set it's contents and check results.
  SdmMessageBody *nestedBody2;
  if (nestedMsg->SdmMessageBodyFactory(err, nestedBody2, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory2 failed for nested message.\n";
    return err;    
  }
  SdmString newContents2("new message contents in body 2");
  if (nestedBody2->SetContents(err, newContents2)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;       
    err = Sdm_EC_Success;
  }
  if (CheckContents(err, nestedBody2, newContents2)) {
    return err;
  }
  
  // set the name of the attachment.
  SdmMessageBodyStructure r_bodyStruct;
  if (nestedBody2->GetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: GetStructure failed.\n";
    return err;
  }
  r_bodyStruct.mbs_attachment_name = "MyAttachment";
  if (nestedBody2->SetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: SetStructure failed.\n";
    return err;
  }


  // create third body in nested message.  set it's contents and check results.
  SdmMessageBody *nestedBody3;
  if (nestedMsg->SdmMessageBodyFactory(err, nestedBody3, Sdm_MSTYPE_text, SdmString("plain"))) {
    cout << "*** Error: SdmMessageBodyFactory2 failed for nested message.\n";
    return err;    
  }
  SdmString newContents3("new message contents in body 3");
  if (nestedBody3->SetContents(err, newContents3)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;       
    err = Sdm_EC_Success;
  }
  if (CheckContents(err, nestedBody3, newContents3)) {
    return err;
  }
  
  // set the name of the attachment.
  if (nestedBody3->GetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: GetStructure failed.\n";
    return err;
  }
  r_bodyStruct.mbs_attachment_name = "SecondAttachment";
  if (nestedBody3->SetStructure(err,  r_bodyStruct)) {
    cout << "*** Error: SetStructure failed.\n";
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

  // check body count of nested message.
  if (nestedMsg->GetBodyCount(err, bodyCount)) {
    cout << "*** Error: can't get body count for nested message.\n";
    return err;    
  }
  if (bodyCount != 3) {
    cout << "*** Error: nested message has invalid body count.\n";
    return err;    
  }

  // delete the first attachment.
  SdmBoolean previousState;
  if (nestedBody2->DeleteMessageBodyFromMessage(err, previousState, Sdm_True)) {
    cout << "*** Error: DeleteMessageBodyFromMessage failed.\n";
    return err;
  }
  if (previousState != Sdm_False) {
    cout << "*** Error: invalid previous state from DeleteMessageBodyFromMessage.\n";
    err = Sdm_EC_Fail;
    return err;
  }
   
  // check body count of nested message.  should still be three.
  if (nestedMsg->GetBodyCount(err, bodyCount)) {
    cout << "*** Error: can't get body count for nested message.\n";
    return err;    
  }
  if (bodyCount != 3) {
    cout << "*** Error: nested message has invalid body count.\n";
    return err;    
  }

  SdmBoolean r_disposition;
  // CommitPendingChanges should fail for a nested message.
  if (nestedMsg->CommitPendingChanges(err, r_disposition)) {
    if (err != Sdm_EC_MessageIsNested) {
      cout << "*** Error: CommitPendingChanges returned wrong error for nested msg.\n"; 
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: CommitPendingChanges does not return error for nested msg.\n"; 
    err = Sdm_EC_Fail;
    return err;
  }

  // DiscardPendingChanges should fail for a nested message.
  if (nestedMsg->DiscardPendingChanges(err)) {
    if (err != Sdm_EC_MessageIsNested) {
      cout << "*** Error: DiscardPendingChanges returned wrong error for nested msg.\n"; 
      return err;
    } else {
      err = Sdm_EC_Success;
    }
  } else {
    cout << "*** Error: DiscardPendingChanges does not return error for nested msg.\n"; 
    err = Sdm_EC_Fail;
    return err;
  }

  if (commit) {
    if (newmsg->CommitPendingChanges(err, r_disposition)) {
      cout << "*** Error: CommitPendingChanges fail for nested msg.\n"; 
      return err;  
    }
  } else {
    if (newmsg->DiscardPendingChanges(err)) {
      cout << "*** Error: DiscardPendingChanges fail for nested msg.\n"; 
      return err;  
    }
  }
   
  return err;
}


SdmErrorCode 
SdmComposeTest::TestEditedMessage()
{
  SdmError err;

  // get message and it's envelope
  SdmMessage *msg2;
  if (mbox->SdmMessageFactory(err, msg2, 2)) {
    cout << "*** Error: SdmMessageFactory failed to create second message.\n";
    return err;
  }
  SdmMessageEnvelope *envelope;
  if (msg2->SdmMessageEnvelopeFactory(err, envelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed for second message.\n";
    return err;
  }

  // get first body.
  SdmMessageBody *body;
  if (msg2->SdmMessageBodyFactory(err, body, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed for second message.\n";
    return err;
  }

  // change contents of first body.
  SdmString newContents("updated message contents");
  if (body->SetContents(err, newContents)) {
    cout << "*** Error: SdmMessageBody set contents failed.\n";
    return err;       
  }
  
  // check contents of body.
  if (CheckContents(err, body, newContents)) {
    return err;
  }

  // check that same pointer is returned if factory method is called again.
  SdmMessage *samemsg;
  if (mbox->SdmMessageFactory(err, samemsg, 2)) {
    cout << "*** Error: SdmMessageFactory failed to get second message.\n";
    return err;
  }
  if (samemsg != msg2) {
    cout << "*** Error: wrong message returned for msgnum 2.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // get the headers for message.
  SdmStrStrL r_headers;
  if (envelope->GetHeaders(err, r_headers)) {
    cout << "*** Error: get headers failed for second message.\n";
    return err;
  }
  
  PrintHeaders(r_headers);
  
  // find the subject header and change the value of it.
  SdmBoolean found = Sdm_False;
  for (int i=0; i< r_headers.ElementCount(); i++) {
    if (r_headers[i].GetFirstString() == "Subject") {
      found = Sdm_True;
      break;
    }
  }
  if (!found) {
    cout << "*** Error: can't get subject header for second message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  SdmString originalSubject = r_headers[i].GetSecondString();
  r_headers.InsertElementAfter(SdmStrStr("Subject", "updated subject"), i);
  r_headers.RemoveElement(i);

  PrintHeaders(r_headers);
  
  if (envelope->SetHeaders(err, r_headers)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  SdmStrStrL r_headers2;
  if (envelope->GetHeaders(err, r_headers2)) {
    cout << "*** Error: get headers failed for second message.\n";
    return err;
  }

  PrintHeaders(r_headers2);

  // check new headers.
  if (CheckHeaders(err, envelope, r_headers)) {
    return err;
  }
   
  // DiscardPendingChanges should fail for an edited message.  
  if (msg2->DiscardPendingChanges(err)) {
    // check for proper error code.
    if (err == Sdm_EC_Operation_Unavailable) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: DiscardPendingChanges fails for edited msg.\n"; 
      return err;  
    }
  } else {
    cout << "*** Error: DiscardPendingChanges does not return error for edited msg.\n"; 
    err = Sdm_EC_Fail;
    return err;  
  }

  SdmBoolean r_disposition;
  if (msg2->CommitPendingChanges(err, r_disposition)) {
    cout << "*** Error: CommitPendingChanges fail for edited msg.\n"; 
    err = Sdm_EC_Fail;
    return err;  
  }

  //
  // Check that original message is restored back to the original data and the
  // deleted flag is set for this message.
  //

  // note:  old envelope is no longer valid after Commit.  need to get it again.
  SdmMessageEnvelope *newenvelope;
  if (msg2->SdmMessageEnvelopeFactory(err, newenvelope)) {
    cout << "*** Error: can't get envelope after commit.\n"; 
    return err;  
  }

  r_headers2.ClearAllElements();
  if (newenvelope->GetHeaders(err, r_headers2, Sdm_MHA_Subject)) {
    cout << "*** Error: GetHeaders fail after commit.\n"; 
    return err;
  }

  if (r_headers2.ElementCount() != 1) {
    cout << "*** Error: can't get Subject after commit.\n"; 
    return err;
  }

  if (r_headers2[0].GetSecondString() != originalSubject) {
    cout << "*** Error: Subject not restored after commited message.\n";
    return err;
  }
 

  SdmMessageFlagAbstractFlags r_flags;
  if (msg2->GetFlags(err, r_flags)) {
    cout << "*** Error: GetFlags fail after commit.\n"; 
    return err;
  }

  if ((r_flags & Sdm_MFA_Deleted) == 0) {
    cout << "*** Error: delete flag not set for edited msg that's commited.\n"; 
    return err;
  }

  return err;
}

SdmErrorCode 
SdmComposeTest::TestDerivedMessage(SdmBoolean commit)
{
  SdmError err;

  // get first message.
  SdmMessage *msg1;
  if (mbox->SdmMessageFactory(err, msg1, 1)) {
    cout << "*** Error: SdmMessageFactory failed to create first message.\n";
    return err;
  }  

  // create derived message.
  SdmMessage *dmsg;
  if (mbox->SdmMessageFactory(err, dmsg, *msg1)) {
    cout << "*** Error: SdmMessageFactory failed to create derived message.\n";
    return err;
  }  

  SdmMessageNumber msgno;
  if (dmsg->GetMessageNumber(err, msgno)) {
    cout << "*** Error: GetMessageNumber failed for new messgae.\n";
    return err;
  }
 
  // check that same pointer is returned if factory method is called again.
  SdmMessage *samemsg;
  if (mbox->SdmMessageFactory(err, samemsg, msgno)) {
    cout << "*** Error: SdmMessageFactory failed to get derived message.\n";
    return err;
  }
  if (dmsg != samemsg) {
    cout << "*** Error: wrong message returned for derived message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mbox->SdmMessageFactory(err, samemsg, 1)) {
    cout << "*** Error: SdmMessageFactory failed to get first message.\n";
    return err;
  }
  if (msg1 != samemsg) {
    cout << "*** Error: wrong message returned for msgno 1.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 

  // check the contents of the entire message.
  SdmContentBuffer r_contents;
  if (msg1->GetContents(err, r_contents)) {
    cout << "*** Error: get contents failed for first message.\n";
    return err;
  }  
  /* Fix!! uncomment later.  GetContents not supported for edited or new message.
  if (CheckContents(err, dmsg, r_contents)) {
    return err;
  }  
  */
  
  // check the contents of the body.
  SdmMessageBody *body;
  SdmContentBuffer r_bodyContents;
  SdmString r_bodyStrContent;
  if (msg1->SdmMessageBodyFactory(err, body, 1)) {
    cout << "*** Error: can't get body for first message.\n";
    return err;
  }  
  if (body->GetContents(err, r_bodyContents)) {
    cout << "*** Error: get contents failed for body of first message.\n";
    return err;
  }  
  if (r_bodyContents.GetContents(err, r_bodyStrContent)) {
    cout << "*** Error: get contents failed for content buffer of first message.\n";
    return err;
  }
  SdmMessageBody *dbody;
  if (dmsg->SdmMessageBodyFactory(err, dbody, 1)) {
    cout << "*** Error: can't get body for derived message.\n";
    return err;
  }  
  if (CheckContents(err, dbody, r_bodyStrContent)) {
    return err;
  }  
  
  // check the headers in the envelope.
  SdmMessageEnvelope *env;
  if (msg1->SdmMessageEnvelopeFactory(err, env)) {
    cout << "*** Error: can't get envelope for first message.\n";
    return err;
  }  
  SdmStrStrL r_hdrs;
  if (env->GetHeaders(err, r_hdrs)) {
    cout << "*** Error: get headers failed for envelope of first message.\n";
    return err;
  }  
  SdmMessageEnvelope *denv;
  if (dmsg->SdmMessageEnvelopeFactory(err, denv)) {
    cout << "*** Error: can't get envelope for derived message.\n";
    return err;
  }  
  if (CheckHeaders(err, denv, r_hdrs)) {
    return err;
  }

  // get the headers for message.
  SdmStrStrL r_headers;
  if (denv->GetHeaders(err, r_headers)) {
    cout << "*** Error: get headers failed for second message.\n";
    return err;
  }
  
  PrintHeaders(r_headers);
  
  // find the subject header and change the value of it.
  SdmBoolean found = Sdm_False;
  for (int i=0; i< r_headers.ElementCount(); i++) {
    if (r_headers[i].GetFirstString() == "Subject") {
      found = Sdm_True;
      break;
    }
  }
  if (!found) {
    cout << "*** Error: can't get subject header for derived message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  SdmString originalSubject = r_headers[i].GetSecondString();
  r_headers.InsertElementAfter(SdmStrStr("Subject", "updated subject for derived msg"), i);
  r_headers.RemoveElement(i);
  
  if (denv->SetHeaders(err, r_headers)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  // check updated headers.
  if (CheckHeaders(err, denv, r_headers)) {
    return err;
  }

  if (commit) {
    SdmBoolean r_disposition;
    if (dmsg->CommitPendingChanges(err, r_disposition)) {
      cout << "*** Error: CommitPendingChanges fail for derived msg.\n"; 
      return err;  
    }
    return err;
  } else {
    if (dmsg->DiscardPendingChanges(err)) {
      cout << "*** Error: DiscardPendingChanges fail for derived msg.\n"; 
      return err;  
    }
  }

  // regardless of whether the message was committed or disgarded,
  // the derived msg# should no longer be valid.  Call something and 
  // make sure an error of Sdm_EC_BadMessageNumber is returned.
  SdmMessageFlagAbstractFlags r_flags;
  if (dmsg->GetFlags(err, r_flags)) {
    if (err == Sdm_EC_BadMessageNumber) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: GetFlags fails for committed derived message.\n"; 
      return err;
    }
  } else {
    err = Sdm_EC_Fail;
    cout << "*** Error: GetFlags succeeds after derived message committed.\n"; 
    return err;
  }


  return err;
}
  

SdmErrorCode
SdmComposeTest::CheckContents(SdmError &err, SdmMessage* msg, SdmContentBuffer &correctContents)
{
  err = Sdm_EC_Success;
    
  SdmContentBuffer r_contents;
  SdmString r_strContent;

  if (msg->GetContents(err, r_contents)) {
    cout << "*** Error: get contents failed for message to check.\n";
    return err;
  }  
  if (r_contents.GetContents(err, r_strContent)) {
    cout << "*** Error: get contents failed for msg content buffer.\n";
    return err;
  }  
  
  SdmString correctStr;
  if (correctContents.GetContents(err, correctStr)) {
    cout << "*** Error: get contents failed for correct content buffer.\n";
    return err;
  }  
  
  if (correctStr != r_strContent) {
    err = Sdm_EC_Fail;
    cout << "*** Error: contents don't match for derived message.\n";
    return err;
  }  
  return err;  
}


SdmErrorCode
SdmComposeTest::CheckContents(SdmError &err, SdmMessageBody* body, SdmString &correctContents)
{
  err = Sdm_EC_Success;
  
  SdmContentBuffer r_contents;

  if (body->GetContents(err, r_contents)) {
    cout << "*** Error: get contents failed for body to check.\n";
    return err;
  }  
  
  SdmString str_contents;
  if (r_contents.GetContents(err, str_contents)) {
    cout << "*** Error: get contents on content buffer failed for body to check.\n";
    return err;
  }  
  
  if (correctContents != str_contents) {
    err = Sdm_EC_Fail;
    cout << "*** Error: contents don't match for message body.\n";
    return err;
  }  
  return err;  
}  

SdmErrorCode
SdmComposeTest::CheckHeaders(SdmError &err, SdmMessageEnvelope* env, 
  SdmStrStrL &correctHdrs)
{
  err = Sdm_EC_Success;

  SdmStrStrL r_hdrs;
  
  // go through each header that we are to check.  Get each header
  // from the envelope and check the value to make sure it matches.  
  for (int i=0; i<correctHdrs.ElementCount(); i++) {
    r_hdrs.ClearAllElements();
    if (env->GetHeader(err, r_hdrs, correctHdrs[i].GetFirstString())) {
      cout << "*** Error: get headers fails.\n";
      cout << "Header is " << correctHdrs[i].GetFirstString() << "\n";
      cout << "i is " << i << "\n\n";
      return err;
    }
    SdmBoolean found = Sdm_False;
    for (int j=0; j<r_hdrs.ElementCount(); j++) {
      if (r_hdrs[j].GetSecondString() == correctHdrs[i].GetSecondString()) {
        found = Sdm_True;
        break;
      }
    }
    if (!found) {
      err = Sdm_EC_Fail;
      cout << "*** Error: header value not found in envelope.\n";
      cout << "Header is " << correctHdrs[i].GetFirstString() << "\n\n";
      return err;
    }
  }
  
  return err;  
}  

SdmErrorCode
SdmComposeTest::PollForReply(SdmReplyQueue *frontEndQueue,
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

//
// Callbacks
//
void
SdmComposeTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmComposeTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
 
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }
 
}

void 
SdmComposeTest::SetContentsCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  setContentsCallbackCalled = Sdm_True;
 
  cout << "setContentsCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmComposeTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    setContentsCallbackError = Sdm_True;
  }

  if (type != Sdm_SVF_SolicitedEvent) {
    cout << "*** Error: type in call back is not correct.\n";
    setContentsCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_SetMessageBodyContents:
      {        
        // check body part number and message number in event.
        if (event->sdmSetContents.messageNumber != lastMessageNumber) {
          cout << "*** Error: message number does not match in event.\n";
          setContentsCallbackError = Sdm_True;
        }
        if (event->sdmSetContents.bodyPartNumber != lastBodyPartNumber) {
          cout << "*** Error: body part number does not match in event.\n";
          setContentsCallbackError = Sdm_True;
        }
        
        // check error in event
        if (event->sdmSetContents.error == NULL) {
          cout << "*** Error: error is NULL in event.\n";
          setContentsCallbackError = Sdm_True;        
        } else if (*(event->sdmGetContents.error)) {
          cout << "*** Error: error is set in event.\n";
          setContentsCallbackError = Sdm_True;
        }
        break;
      }
 
    default:
      cout << "*** Error: setContentsCallback default event received.\n";
      setContentsCallbackError = Sdm_True;
      break;
  }
}
 
void
SdmComposeTest::PrintHeaders(const SdmStrStrL &headers)
{
  for (int i=0; i< headers.ElementCount(); i++) {
    cout << "first: " << (const char*)headers[i].GetFirstString()
         << " second: " << (const char*)headers[i].GetSecondString() << "\n";
  }
  cout << "\n\n";
}
