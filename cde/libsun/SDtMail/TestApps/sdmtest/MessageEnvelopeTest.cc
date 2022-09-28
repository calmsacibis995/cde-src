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
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageEnvelope.hh>



static  const char* kOutgoingStoreName = "/tmp/msgenv.test";


class SdmMessageEnvelopeTest
{
public:
     SdmMessageEnvelopeTest();
     virtual ~SdmMessageEnvelopeTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode Setup(SdmConnection *&, SdmSession *&ises, SdmSession *&oses, 
                     SdmMessageStore *&mbox, SdmMessageStore *&dbox);

     SdmErrorCode TestParent(SdmMessageEnvelope *, SdmMessage *);
     SdmErrorCode TestHeaderMethods(SdmMessageEnvelope *envelope);
     SdmErrorCode TestGetContents(SdmMessageEnvelope *envelope, SdmReplyQueue* frontEndQueue,
                      int readFileDesc);
     SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
                          
  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void GetContentsCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);

private:

  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
  
  static SdmString kServiceOption;
  static SdmString kServiceType;
  static SdmString kServiceClass;
  
  
  // the following need to be static because they are used by
  // the callback functions.
  static SdmBoolean sessionCallbackCalled;
  static SdmBoolean getContentsCallbackCalled;
  static SdmBoolean getContentsCallbackError;
  static SdmMessageEnvelope *lastMessageEnvelope;
  static SdmString lastContentBuffer;
};

const char* SdmMessageEnvelopeTest::kUserData = "message store test data";

SdmBoolean SdmMessageEnvelopeTest::sessionCallbackCalled;
SdmBoolean SdmMessageEnvelopeTest::getContentsCallbackCalled;
SdmBoolean SdmMessageEnvelopeTest::getContentsCallbackError;
SdmMessageEnvelope* SdmMessageEnvelopeTest::lastMessageEnvelope;
SdmString SdmMessageEnvelopeTest::lastContentBuffer;

SdmString SdmMessageEnvelopeTest::kServiceOption("serviceoption");
SdmString SdmMessageEnvelopeTest::kServiceType("servicetype");
SdmString SdmMessageEnvelopeTest::kServiceClass("serviceclass");
  


SdmErrorCode RunMessageEnvelopeTest()
{
  cout << endl << "************ RunMessageTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmMessageEnvelopeTest test;
  return test.Run();
}
  

SdmMessageEnvelopeTest::SdmMessageEnvelopeTest()
{
}

SdmMessageEnvelopeTest::~SdmMessageEnvelopeTest()
{
}

SdmErrorCode 
SdmMessageEnvelopeTest::Run()
{
  SdmError err;
  SdmSession *ises, *oses;
  SdmConnection *mcon;
  SdmMessage *thirdMessage, *fourthMessage;
  SdmMessageStore *mbox, *dbox;
  SdmMessageEnvelope *envelope1, *envelope4, *envelopeN;
  SdmMessage *sixthMessage;
  SdmMessageEnvelope *envelope6;
  SdmMessage *seventhMessage;
  SdmMessageEnvelope *envelope7;
  SdmMessage *eighthMessage;
  SdmMessageEnvelope *envelope8;
  
  cout << endl << "************ SdmMessageEnvelopeTest::Run Invoked. *************" << endl << endl;
  cout << endl;
    
  SdmErrorCode errorCode;
  
  if ((errorCode = Setup(mcon, ises, oses, mbox, dbox)) != Sdm_EC_Success)
    return errorCode;


  // ***************************************************************8
  // Testing Message Envelope
  // ***************************************************************8

  cout << "\nTesting incoming store.\n";

  cout << "Testing regular message envelope.\n";
  
  if (mbox->SdmMessageFactory(err, thirdMessage, 3)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  
  if (thirdMessage->SdmMessageEnvelopeFactory(err, envelope1)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(envelope1, thirdMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestHeaderMethods(envelope1)) != Sdm_EC_Success)
    return errorCode;
        
  if ((errorCode = TestGetContents(envelope1, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;


  cout << "Testing message envelope for multipart/mixed message.\n";
        
  // fourth message has multiple body parts.    
  if (mbox->SdmMessageFactory(err, fourthMessage, 4)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (fourthMessage->SdmMessageEnvelopeFactory(err, envelope4)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(envelope4, fourthMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestHeaderMethods(envelope1)) != Sdm_EC_Success)
    return errorCode;
                
  if ((errorCode = TestGetContents(envelope4, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;  
  
  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

#ifdef I18N
  // I18N tests
  // Test the Korean message
  if (dbox->SdmMessageFactory(err, sixthMessage, 6)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (sixthMessage->SdmMessageEnvelopeFactory(err, envelope6)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }
  if ((errorCode = TestHeaderMethods(envelope6)) != Sdm_EC_Success)
    return errorCode;
  delete sixthMessage;


  // Test the Japanese message
  if (dbox->SdmMessageFactory(err, eighthMessage, 8)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (eighthMessage->SdmMessageEnvelopeFactory(err, envelope8)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }
  if ((errorCode = TestHeaderMethods(envelope8)) != Sdm_EC_Success)
    return errorCode;
  delete eighthMessage;


  // Test the Japanese message
  if (dbox->SdmMessageFactory(err, seventhMessage, 7)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (seventhMessage->SdmMessageEnvelopeFactory(err, envelope7)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }
  if ((errorCode = TestHeaderMethods(envelope7)) != Sdm_EC_Success)
    return errorCode;
  delete seventhMessage;

  // I18N end
#endif // I18N
  
  delete thirdMessage;
  delete fourthMessage;


  cout << "\nTesting incoming store.\n";

  cout << "Testing regular message envelope.\n";
  
  if (mbox->SdmMessageFactory(err, thirdMessage, 3)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  
  if (thirdMessage->SdmMessageEnvelopeFactory(err, envelope1)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(envelope1, thirdMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestHeaderMethods(envelope1)) != Sdm_EC_Success)
    return errorCode;
        
  if ((errorCode = TestGetContents(envelope1, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;


  cout << "\nTesting outgoing store.\n";
  cout << "Testing message envelope for multipart/mixed message.\n";
        
  // fourth message has multiple body parts.    
  if (dbox->SdmMessageFactory(err, fourthMessage, 4)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (fourthMessage->SdmMessageEnvelopeFactory(err, envelope4)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(envelope4, fourthMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestHeaderMethods(envelope1)) != Sdm_EC_Success)
    return errorCode;
                
  if ((errorCode = TestGetContents(envelope4, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;  
  
  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  delete thirdMessage;
  delete fourthMessage;


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
SdmMessageEnvelopeTest::Setup(SdmConnection *&mcon, SdmSession *&ises, 
  SdmSession *&oses,  SdmMessageStore *&mbox, SdmMessageStore *&dbox)
{
  SdmError err;
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
          (void*)SdmMessageEnvelopeTest::kUserData,
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
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

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
SdmMessageEnvelopeTest::TestParent(SdmMessageEnvelope *envelope, SdmMessage *message)
{
  SdmMessage *parent;
  SdmError err;
  if (envelope->Parent(err, parent)) {
    cout << "*** Error: Parent fails.\n";
    return err;
  }

  if (parent != message) {
    cout << "Error: Parent returns wrong message store.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode
SdmMessageEnvelopeTest::TestHeaderMethods(SdmMessageEnvelope *envelope)
{
  SdmError err;
  SdmError localError;

  SdmStrStrL r_hdrL;
  SdmStrStrL r_dechdrL;
  SdmIntStrL r_int_hdrL;
  SdmStringL str_hdrL;
  SdmMessageHeaderAbstractFlags hdr, a_hdr;
  SdmString str_hdr("date");
  int i;

  hdr = Sdm_MHA_Subject;
  a_hdr = Sdm_MHA_From;

  str_hdrL(-1) = "from";
  str_hdrL(-1) = "to";
 
  if (envelope->GetHeader(err, r_hdrL, str_hdr)) {
    cout << "Error: GetHeader1 fails.\n";
    localError = err;
  }

#ifdef I18N
  // I18N
  // Decode the date header
  for (i=0; i<r_hdrL.ElementCount(); i++) {
    if (envelope->DecodeHeader(err, r_dechdrL, r_hdrL[i].GetSecondString())) {
      cout << "Error: DecodeHeader fails.\n";
      localError = err;
    }
  }
#endif // I18N

  if (envelope->GetHeaders(err, r_int_hdrL, hdr)) {
    cout << "Error: GetHeader2 fails\n";
    localError = err;
  }
  
  r_hdrL.ClearAllElements();
  if (envelope->GetHeaders(err, r_hdrL, hdr)) {
    cout << "Error: GetHeader3 fails\n";
    localError = err;
  }
  
#ifdef I18N
  // I18N
  // Decode the subject header
  r_dechdrL.ClearAllElements();
  for (i=0; i<r_hdrL.ElementCount(); i++) {
    if (envelope->DecodeHeader(err, r_dechdrL, r_hdrL[i].GetSecondString())) {
      cout << "Error: DecodeHeader fails.\n";
      localError = err;
    }
  }
#endif // I18N

  r_hdrL.ClearAllElements();
  if (envelope->GetHeaders(err,r_hdrL)) {
    cout << "Error: GetHeaders1 fails\n";
    localError = err;
  }
  
  r_hdrL.ClearAllElements();
  if (envelope->GetHeaders(err, r_hdrL, str_hdrL)) {
    cout << "Error: GetHeaders2 fails\n";
    localError = err;
  }
  
#ifdef I18N
  // I18N
  // Decode the from and to headers
  r_dechdrL.ClearAllElements();
  for (i=0; i<r_hdrL.ElementCount(); i++) {
    if (envelope->DecodeHeader(err, r_dechdrL, r_hdrL[i].GetSecondString())) {
      cout << "Error: DecodeHeader fails.\n";
      localError = err;
    }
  }
#endif // I18N

  return localError;
}
     
SdmErrorCode
SdmMessageEnvelopeTest::PollForReply(SdmReplyQueue *frontEndQueue, 
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
SdmMessageEnvelopeTest::TestGetContents(SdmMessageEnvelope *envelope, SdmReplyQueue* frontEndQueue,
  int readFileDesc) 
{
  SdmError err;
  SdmContentBuffer r_contents;
  
  // first test sync call.
  if (envelope->GetContents(err, r_contents)) {
    cout << "*** Error: GetContents failed.\n";
    return err;
  }
  
  if (r_contents.GetContents(err, lastContentBuffer)) {
    cout << "*** Error: can't get contents of content buffer.\n";
    return err;
  }
  lastMessageEnvelope = envelope;
  getContentsCallbackError = Sdm_False;
  getContentsCallbackCalled = Sdm_False;
  
  SdmServiceFunction svf(&GetContentsCallback,
        (void*)SdmMessageEnvelopeTest::kUserData, Sdm_SVF_Any);

  // test async call.
  if (envelope->GetContents_Async(err, svf, (void*)kUserData)){
    cout << "*** Error: Expunge Async failed. err = " << (int)err << "\n";
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
SdmMessageEnvelopeTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmMessageEnvelopeTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
  
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }
}
 


void 
SdmMessageEnvelopeTest::GetContentsCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  getContentsCallbackCalled = Sdm_True;
 
  cout << "getContentsCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageEnvelopeTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    getContentsCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_GetMessageEnvelopeContents:
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
        if (event->sdmGetContents.messageNumber != lastMessageEnvelope->_ParentMessgNum) {
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

