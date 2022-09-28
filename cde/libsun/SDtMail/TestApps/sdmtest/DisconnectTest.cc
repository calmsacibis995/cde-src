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
#include <SDtMail/DeliveryResponse.hh>
#include <SDtMail/SystemUtility.hh>


static  SdmString kServiceOption("serviceoption");
static  SdmString kServiceType("servicetype");
static  SdmString kServiceClass("serviceclass");

static const char* kOutgoingStoreName =    "/tmp/disconnect.test";



class SdmDisconnectTest
{
public:
  SdmDisconnectTest();
  virtual ~SdmDisconnectTest();

  virtual SdmErrorCode Run();

private:
  static const char* kUserData;

  SdmErrorCode Setup();
  SdmErrorCode Cleanup();
  
  SdmErrorCode CreateNewMessage(SdmMessage *&newmsg);
  SdmErrorCode TestOpen();
  SdmErrorCode TestDisconnect();
  SdmErrorCode SendNewMessage(SdmMessage *&msg, SdmBoolean isConnected, SdmMsgFormat format);

  SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
  void PrintHeaders(const SdmStrStrL &headers);

  static void SubmitCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
 
  static SdmBoolean submitCallbackCalled;
  static SdmBoolean submitCallbackError;
  static SdmBoolean sessionCallbackCalled;
  static SdmMessageNumber expectedMessageNumber;
  
  SdmConnection *mcon;
  SdmSession *ises, *oses;
  SdmMessageStore *mbox, *dbox;

  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
};

SdmBoolean SdmDisconnectTest::submitCallbackCalled;
SdmBoolean SdmDisconnectTest::submitCallbackError;
SdmBoolean SdmDisconnectTest::sessionCallbackCalled;
SdmMessageNumber SdmDisconnectTest::expectedMessageNumber;

const char* SdmDisconnectTest::kUserData = "compose test data";

SdmErrorCode RunDisconnectTest()
{
  cout << endl << "************RunDisconnectTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmDisconnectTest test;
  return test.Run();
}


SdmDisconnectTest::SdmDisconnectTest()
{
}
 
SdmDisconnectTest::~SdmDisconnectTest()
{
}

SdmErrorCode
SdmDisconnectTest::Run()
{
  SdmError err;

  cout << endl << "************ SdmDisconnectTest::Run Invoked. *************" << endl << endl;
  cout << endl;
 
  SdmErrorCode errorCode;

 
  if ((errorCode = Setup()) != Sdm_EC_Success)
    return errorCode;
 
 
  // ***************************************************************8
  // Testing Message
  // ***************************************************************8
 
  cout << "Testing Open.\n";

  if ((errorCode = TestOpen()) != Sdm_EC_Success)
    return errorCode;

  //
  // we should be opened at this point.

  if ((errorCode = TestDisconnect()) != Sdm_EC_Success)
    return errorCode;

  //
  // we should be connected at this point.

  SdmMessage *msg;
  if ((errorCode = SendNewMessage(msg, Sdm_True, Sdm_MSFMT_Mime)) != Sdm_EC_Success)
    return errorCode;


  if ((errorCode = SendNewMessage(msg, Sdm_True, Sdm_MSFMT_SunV3)) != Sdm_EC_Success)
    return errorCode;

  if (o_frontEndQueue->ElementCount() != 0) {
    cout << "*** Error: reply queue not empty after sending message connected\n";
    err = Sdm_EC_Fail;
    return err;
  }
  

  if (dbox->Disconnect(err)) {
    cout << "*** Error: Disconnect failed\n";
    err = Sdm_EC_Fail;
    return err;
  }


  if ((errorCode = SendNewMessage(msg, Sdm_False, Sdm_MSFMT_Mime)) != Sdm_EC_Success)
    return errorCode;


  if ((errorCode = SendNewMessage(msg, Sdm_False, Sdm_MSFMT_SunV3)) != Sdm_EC_Success)
    return errorCode;


  if (o_frontEndQueue->ElementCount() != 0) {
    cout << "*** Error: reply queue not empty after sending message disconnected\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (dbox->Reconnect(err)) {
    cout << "*** Error: Reconnect failed\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmMessageStoreStatus r_status;
  if (dbox->GetStatus(err, r_status, Sdm_MSS_Messages)) {
    cout << "*** Error: GetStatus failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (o_frontEndQueue->ElementCount() != 2) {
    cout << "*** Error: reply queue empty after reconnect\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the expected message number for the submit should be the last message in the store.
  expectedMessageNumber = r_status.messages-1;
  submitCallbackError = Sdm_False;
  submitCallbackCalled = Sdm_False;

  // process the reply on the queue.
  if (PollForReply(o_frontEndQueue, o_readFileDesc, 2)) {
    return err;
  }

  // the session callback should have been called.
  if (!submitCallbackCalled) {
    cout << "*** Error: submit callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (submitCallbackError) {
    cout << "*** Error: error occurred in submit callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
    
  if ((errorCode = Cleanup()) != Sdm_EC_Success)
    return errorCode;
 
  return Sdm_EC_Success;
}


SdmErrorCode
SdmDisconnectTest::Setup()
{
  SdmError err;

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp disconnect.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
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
  SdmServiceFunction svf2(&SubmitCallback,
          (void*)SdmDisconnectTest::kUserData, Sdm_SVF_SubmitOutgoingCachedMessage);
 
  if (mbox->RegisterServiceFunction(err, svf2)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  if (dbox->RegisterServiceFunction(err, svf2)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }

  submitCallbackError = Sdm_False;
  submitCallbackCalled = Sdm_False;

  return err;
}


SdmErrorCode
SdmDisconnectTest::TestOpen()
{
  SdmError err;
  err = Sdm_EC_Success;
  SdmToken token;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  // open with invalid service type.
  token.SetValue(kServiceType,SdmString("remote"));
  if (dbox->Open(err, nmsgs, readOnly, token)) {
    if (err == Sdm_EC_InvalidOutgoingStoreToken) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Open failed for outgoing store.\n";
      return err;
    }
  } else {
    cout << "*** Error: Open did not return error for invalid token.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // set token to correct service type.
  token.ClearValue(kServiceType);
  token.SetValue(kServiceType,SdmString("local"));

  // open with invalid service class.
  token.SetValue(kServiceClass,SdmString("messagetransport"));
  if (dbox->Open(err, nmsgs, readOnly, token)) {
    if (err == Sdm_EC_InvalidOutgoingStoreToken) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Open failed for outgoing store.\n";
      return err;
    }
  } else {
    cout << "*** Error: Open did not return error for invalid token.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // set token to correct service class.
  token.ClearValue(kServiceClass);
  token.SetValue(kServiceClass,SdmString("messagestore"));

  // open with missing message store name.
  if (dbox->Open(err, nmsgs, readOnly, token)) {
    if (err == Sdm_EC_InvalidOutgoingStoreToken) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Open failed for outgoing store.\n";
      return err;
    }
  } else {
    cout << "*** Error: Open did not return error for invalid token.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // Open with valid token.
  token.SetValue(kServiceOption,SdmString("messagestorename"), SdmString(kOutgoingStoreName));
  if (dbox->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  // close store.
  if (dbox->Close(err)) {
    cout << "*** Error: Close failed.\n";
    return err;
  }
  
  SdmToken token2;
  
  // Open with token that only has message store name.
  token2.SetValue(kServiceOption,SdmString("messagestorename"), SdmString(kOutgoingStoreName));
  if (dbox->Open(err, nmsgs, readOnly, token2)) {
    cout << "*** Error: Open failed with storename token.\n";
    return err;
  }
 
  return err;
}


SdmErrorCode
SdmDisconnectTest::TestDisconnect()
{
  SdmError err;
  err = Sdm_EC_Success;

  // we should start out connected.
  if (!((SdmOutgoingStore*)dbox)->IsConnected()) {
    cout << "*** Error: IsConnected failed for opened store.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // reconnect when already connected.
  if (dbox->Reconnect(err)) {
    if (err == Sdm_EC_AlreadyConnected) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Reconnect failed for outgoing store.\n";
      return err;
    }
  } else {
    cout << "*** Error: Reconnect did not return error for invalid token.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // disconnect. this should work.
  if (dbox->Disconnect(err)) {
    cout << "*** Error: Disconnect failed for outgoing store.\n";
    return err;
  }

  if (((SdmOutgoingStore*)dbox)->IsConnected()) {
    cout << "*** Error: IsConnected failed for disconnected store.\n";
    err = Sdm_EC_Fail;
    return err;
  }
    
  // disconnect again.  should return error.
  if (dbox->Disconnect(err)) {
    if (err == Sdm_EC_AlreadyDisconnected) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Disconnect failed for outgoing store.\n";
      return err;
    }
  } else {
    cout << "*** Error: Disconnect did not return error for invalid token.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // reconnect. this should work.
  if (dbox->Reconnect(err)) {
    cout << "*** Error: Reconnect failed for outgoing store.\n";
    return err;
  }

  if (!((SdmOutgoingStore*)dbox)->IsConnected()) {
    cout << "*** Error: IsConnected failed for connected store.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  return err;
}


SdmErrorCode
SdmDisconnectTest::SendNewMessage(SdmMessage *&msg, SdmBoolean isConnected, SdmMsgFormat format)
{
  SdmError err;
  if ((err = CreateNewMessage(msg)) != Sdm_EC_Success) {
    return err;
  } 

  SdmStringL customHeaders;
  customHeaders(-1) = "X-Mustard";
  customHeaders(-1) = "X-Mailer";
  SdmDeliveryResponse response;
  if (msg->Submit(err, response, format, Sdm_False, customHeaders)) {
    // if we are disconnected, then we should be getting
    // the error Sdm_EC_SubmitDeletedMessage.  check for this here.
    if (!isConnected && err == Sdm_EC_CommitDisconnectedMessage) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: Submit failed.\n";
      return err;
    }
  }

  // we should be able to get the flags of the message after the 
  // message has been submitted (when connected) or a copy of the
  // message has been committed (when disconnected).
  SdmMessageFlagAbstractFlags flags;
  if (msg->GetFlags(err, flags)) {
      cout << "*** Error: GetFlags failed after message is submitted.\n";
      return err;
  } 

  // check delete flag if message was submitted.
  if (isConnected) {
    if ((flags & Sdm_MFA_Deleted) == 0) {
      cout << "*** Error: message delete flag not set after Submit.\n";
      err = Sdm_EC_Fail;
      return err;
    }
  }

  return err;
}

SdmErrorCode
SdmDisconnectTest::Cleanup()
{
  SdmError err;

  //
  // close and shutdown objects
  //
  if (mbox->ShutDown(err)) {
    cout << "*** Error: SdmMessageStore shutdown failed.\n";
    return err;
  }

  if (ises->ShutDown(err)) {
    cout << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (dbox->ShutDown(err)) {
    cout << "*** Error: SdmMessageStore shutdown failed.\n";
    return err;
  }

  if (oses->ShutDown(err)) {
    cout << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (mcon->ShutDown(err)) {
    cout << "*** Error: SdmConnection shutdown failed.\n";
    return err;
  }

  // clean up objects.
  // note: don't delete envelope or body.  they got deleted by message.
  // note: don't delete message.  it got deleted by session.
  // note: don't delete session.  it got deleted by connection.
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
SdmDisconnectTest::CreateNewMessage(SdmMessage *&r_message)
{
  SdmError err;
  
  // create new message and new body.  get new envelope for new message.
  SdmMessage *newmsg;
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
  newHeaders.AddElementToList(SdmStrStr("subject", "disconnect test"));
  newHeaders.AddElementToList(SdmStrStr("X-Mustard", "Please pass the Grey Poupon!"));
  newHeaders.AddElementToList(SdmStrStr("X-Mailer", "Disconnected test!"));
  if (newenvelope->SetHeaders(err, newHeaders)) {
    cout << "*** Error: SdmMessageEnvelope set headers failed.\n";
    return err;        
  }

  r_message = newmsg;

  return err;
}





SdmErrorCode
SdmDisconnectTest::PollForReply(SdmReplyQueue *frontEndQueue,
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
SdmDisconnectTest::SubmitCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  submitCallbackCalled = Sdm_True;
 
  cout << "SubmitCallback Called.\n";
 
  // check userdata in event.
  if (userdata != SdmDisconnectTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    submitCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_SubmitMessage:
        // check for error.
        if (event->sdmSubmitMessage.error == NULL) {
          cout << "*** Error: error is null in submit event.\n";
          submitCallbackError = Sdm_True;
        } else {
          SdmError *error = event->sdmSubmitMessage.error;
          if (*error) {
              cout << "*** Error: error returned from Submit call. err = "
                 << (int)*error << "\n" ;
              submitCallbackError = Sdm_True;
          }
        }
        
        if (event->sdmSubmitMessage.response == NULL) {
          cout << "*** Error: delivery response is null in submit event.\n";
          submitCallbackError = Sdm_True;
        } else {
          delete (event->sdmSubmitMessage.response);
        }
        
        if (event->sdmSubmitMessage.messageNumber != expectedMessageNumber) {
          cout << "*** Error: unexpected message number in submit event.\n";
          submitCallbackError = Sdm_True;
        } else {
          expectedMessageNumber++;
        }
        break;
    
    default:
      cout << "*** Error: SubmitCallback default event received.\n";
      submitCallbackError = Sdm_True;
      break;
  }
}
 
 
void
SdmDisconnectTest::PrintHeaders(const SdmStrStrL &headers)
{
  for (int i=0; i< headers.ElementCount(); i++) {
    cout << "first: " << (const char*)headers[i].GetFirstString()
         << " second: " << (const char*)headers[i].GetSecondString() << "\n";
  }
  cout << "\n\n";
}
