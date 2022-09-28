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
#include <SDtMail/DataTypeUtility.hh>

static  const char* kOutgoingStoreName = "/tmp/msgbody.test";


class SdmMessageBodyTest
{
public:
     SdmMessageBodyTest();
     virtual ~SdmMessageBodyTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode Setup(SdmConnection *&, SdmSession *&ises, SdmSession *&oses, 
                     SdmMessageStore *&mbox, SdmMessageStore *&dbox);

     SdmErrorCode TestMessageBodyInStore(SdmMessageStore *, SdmReplyQueue *, int);
     SdmErrorCode TestParent(SdmMessageBody *, SdmMessage *);
     SdmErrorCode TestBodyPartNumber(SdmMessageBody *, int number);
     SdmErrorCode TestAttributeMethods(SdmMessageBody *body);
     SdmErrorCode TestGetStructure(SdmMessageBody *body);
     SdmErrorCode TestGetContents(SdmMessageBody *body, SdmReplyQueue* frontEndQueue,
                      int readFileDesc);
     SdmErrorCode TestCacheStatus(SdmMessageBody *body, const SdmBoolean expectedResult);
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
  static SdmMessageBody *lastMessageBody;
  static SdmString lastContentBuffer;
};

const char* SdmMessageBodyTest::kUserData = "message store test data";

SdmBoolean SdmMessageBodyTest::sessionCallbackCalled;
SdmBoolean SdmMessageBodyTest::getContentsCallbackCalled;
SdmBoolean SdmMessageBodyTest::getContentsCallbackError;
SdmMessageBody* SdmMessageBodyTest::lastMessageBody;
SdmString SdmMessageBodyTest::lastContentBuffer;

SdmString SdmMessageBodyTest::kServiceOption("serviceoption");
SdmString SdmMessageBodyTest::kServiceType("servicetype");
SdmString SdmMessageBodyTest::kServiceClass("serviceclass");
  


SdmErrorCode RunMessageBodyTest()
{
  cout << endl << "************ RunMessageTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmMessageBodyTest test;
  return test.Run();
}
  

SdmMessageBodyTest::SdmMessageBodyTest()
{
}

SdmMessageBodyTest::~SdmMessageBodyTest()
{
}

SdmErrorCode 
SdmMessageBodyTest::Run()
{
  SdmError err;
  SdmSession *ises, *oses;
  SdmConnection *mcon;
  SdmMessageStore *mbox, *dbox;
  
  cout << endl << "************ SdmMessageBodyTest::Run Invoked. *************" << endl << endl;
  cout << endl;
    
  SdmErrorCode errorCode;
  
  if ((errorCode = Setup(mcon, ises, oses, mbox, dbox)) != Sdm_EC_Success)
    return errorCode;

  cout << "Testing message body from message store.\n";
  if ((errorCode = TestMessageBodyInStore(mbox, i_frontEndQueue, i_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;


  cout << "Testing message body from outgoing store.\n";
  if ((errorCode = TestMessageBodyInStore(dbox, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) 
    return errorCode;

#ifdef I18N
  // I18N tests
  SdmMessageBody *body6, *body7, *body8;
  SdmMessage *sixthMessage, *seventhMessage, *eighthMessage;

  // Korean message
  if (dbox->SdmMessageFactory(err, sixthMessage, 6)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (sixthMessage->SdmMessageBodyFactory(err, body6, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }
  if ((errorCode = TestGetContents(body6, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) {
    return err;
  }
  delete sixthMessage;

  // Japanese message
  if (dbox->SdmMessageFactory(err, seventhMessage, 7)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (seventhMessage->SdmMessageBodyFactory(err, body7, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }
  if ((errorCode = TestGetContents(body7, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) {
    return err;
  }
  delete seventhMessage;


  // Japanese message
  if (dbox->SdmMessageFactory(err, eighthMessage, 8)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (eighthMessage->SdmMessageBodyFactory(err, body8, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }
  if ((errorCode = TestGetContents(body8, o_frontEndQueue, o_readFileDesc)) != Sdm_EC_Success) {
    return err;
  }
  delete eighthMessage;

  // I18N end
#endif // I18N

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

  return errorCode;
}


SdmErrorCode
SdmMessageBodyTest::TestMessageBodyInStore(SdmMessageStore *mbox, SdmReplyQueue *frontEndQueue, 
   int readFileDesc)
{
  SdmError err;
  SdmErrorCode errorCode;
  SdmMessageBody *body1, *body4, *bodyN;
  SdmMessage *thirdMessage, *fourthMessage;

  // ***************************************************************8
  // Testing Message Body
  // ***************************************************************8

  cout << "Testing regular message body.\n";
  
  if (mbox->SdmMessageFactory(err, thirdMessage, 3)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  
  if (thirdMessage->SdmMessageBodyFactory(err, body1, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(body1, thirdMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestBodyPartNumber(body1, 1)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestCacheStatus(body1, Sdm_True)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestGetStructure(body1)) != Sdm_EC_Success)
    return errorCode;
        
  if ((errorCode = TestAttributeMethods(body1)) != Sdm_EC_Success)
    return errorCode;
        
  if ((errorCode = TestGetContents(body1, frontEndQueue, readFileDesc)) != Sdm_EC_Success) 
    return errorCode;


  cout << "Testing message body for multipart/mixed body.\n";
        
  // fourth message has multiple body parts.    
  if (mbox->SdmMessageFactory(err, fourthMessage, 4)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  if (fourthMessage->SdmMessageBodyFactory(err, body4, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(body4, fourthMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestBodyPartNumber(body4, 1)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestCacheStatus(body1, Sdm_True)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestGetStructure(body4)) != Sdm_EC_Success)
    return errorCode;

/*
  // revisit: nested message currently does not currently have any attribute 
  // defined.
  if (readFileDesc == o_readFileDesc) {
    if ((errorCode = TestAttributeMethods(body4)) != Sdm_EC_Success)
      return errorCode;
  }
*/
        
  // note: cclient does not support getting raw contents of
  // message bodies.
  if (readFileDesc == i_readFileDesc) {
    if ((errorCode = TestGetContents(body4, frontEndQueue, readFileDesc)) != Sdm_EC_Success) 
      return errorCode;
  }
  
  cout << "Testing message body for nested message.\n";
  
  // create nested message from body of 4th message.  4th message has
  // multipart mixed body.
  SdmMessage *nestedMessage, *copyNestedMessage;
  if (body4->SdmMessageFactory(err, nestedMessage)) {
    cout << "*** Error: SdmMessageFactory failed for nested message.\n";
    return err;
  }

  // check that calling SdmMessageFactory again returns the same message.
  if (body4->SdmMessageFactory(err, copyNestedMessage)) {
    cout << "*** Error: SdmMessageFactory failed for copy nested message.\n";
    return err;
  }

  if (nestedMessage != copyNestedMessage) {
    cout << "*** Error: second call to SdmMessageFactory does not return same message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (nestedMessage->SdmMessageBodyFactory(err, bodyN, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }

  if ((errorCode = TestParent(bodyN, nestedMessage)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestBodyPartNumber(body4, 1)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestCacheStatus(body1, Sdm_True)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestGetStructure(bodyN)) != Sdm_EC_Success)
    return errorCode;

  // note: Nested message in tclient message store does not currently 
  // have any attributes defined.
  if (readFileDesc == o_readFileDesc) {    
    if ((errorCode = TestAttributeMethods(bodyN)) != Sdm_EC_Success)
      return errorCode;
  }
      
  if ((errorCode = TestGetContents(bodyN, frontEndQueue, readFileDesc)) != Sdm_EC_Success) 
    return errorCode;
  
  if (sessionCallbackCalled == Sdm_True) {
    cout << "*** Error: Session Callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }


  
  delete thirdMessage;
  delete fourthMessage;
  // note: don't need to delete nested message.  it gets deleted when the fourth message
  // gets deleted.
  // note: don't need to delete bodies or envelope.  they are deleted by the SdmMessage.
        
  return Sdm_EC_Success;
}

SdmErrorCode 
SdmMessageBodyTest::Setup(SdmConnection *&mcon, SdmSession *&ises, 
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
          (void*)SdmMessageBodyTest::kUserData,
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
SdmMessageBodyTest::TestParent(SdmMessageBody *body, SdmMessage *message)
{
  SdmMessage *parent;
  SdmError err;
  if (body->Parent(err, parent)) {
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
SdmMessageBodyTest::TestBodyPartNumber(SdmMessageBody *body, int bpNum)
{
  SdmError err;
  int num;

  if (body->GetBodyPartNumber(err, num)) {
    cout << "*** Error: GetBodyPartNumber fails.\n";
    return err;
  }
 
  if (num != bpNum) {
    cout << "Error: GetBodyPartNumber returns wrong number.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode 
SdmMessageBodyTest::TestCacheStatus(SdmMessageBody *messageBody, const SdmBoolean expectedResult)
{
  SdmBoolean result;
  SdmError err;
  if (messageBody->GetMessageCacheStatus(err, result)) {
    cout << "*** Error: GetMessageCacheStatus fails.\n";
    return err;
  }
  
  if (result != expectedResult) {
    err = Sdm_EC_Fail;
    cout << "*** Error: GetMessageCacheStatus returns wrong value.\n";
    return err;
  }
  return err;
}

SdmErrorCode
SdmMessageBodyTest::TestGetStructure(SdmMessageBody *body)
{
  SdmError err;
  SdmMessageBodyStructure r_struct;
  SdmDpMessageStructure *r_msg_struct;
  
  r_msg_struct = body->_MessgStruct;
  if (r_msg_struct == NULL) {
    cout << "*** Error: message structure for body is null.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (body->GetStructure(err, r_struct)) {
    cout << "*** Error: GetStructure fails.\n";
    return err;
  }
  
  short isEqual = 1;
  isEqual &= (r_struct.mbs_type == r_msg_struct->ms_type);
  isEqual &= (r_struct.mbs_encoding == r_msg_struct->ms_encoding);
  isEqual &= (r_struct.mbs_lines == r_msg_struct->ms_lines);
  isEqual &= (r_struct.mbs_bytes == r_msg_struct->ms_bytes);
  isEqual &= (r_struct.mbs_disposition == r_msg_struct->ms_disposition);
  isEqual &= (r_struct.mbs_unix_mode == r_msg_struct->ms_unix_mode);
  isEqual &= (r_struct.mbs_subtype == r_msg_struct->ms_subtype);
  isEqual &= (r_struct.mbs_id == r_msg_struct->ms_id);
  isEqual &= (r_struct.mbs_description == r_msg_struct->ms_description);
  isEqual &= (r_struct.mbs_md5 == r_msg_struct->ms_md5);
  isEqual &= (r_struct.mbs_attachment_name == r_msg_struct->ms_attachment_name);
  isEqual &= (r_struct.mbs_is_nested_message == (r_msg_struct->ms_ms != NULL));

  // check mime content type.
  SdmString r_content_type;
  SdmDataTypeUtility::GetMimeTypeStringFromMessage(err, r_content_type, *body);
  if (err) {
    cout << "*** Error: GetMimeTypeStringFromMessage failed.\n";
    return err;
  }
  isEqual &= (r_struct.mbs_mime_content_type == r_content_type);

  if (!isEqual) {
    cout << "*** Error: message body structure not equal to message structure data.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

     
SdmErrorCode
SdmMessageBodyTest::PollForReply(SdmReplyQueue *frontEndQueue, 
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
SdmMessageBodyTest::TestAttributeMethods(SdmMessageBody *body)
{

  SdmError localError, err;
  SdmStrStrL r_attributes;
  SdmString r_attribute;
  SdmMessageAttributeAbstractFlags attr_flags, attr_flag;
  SdmString attr_str("charset");
  SdmString attr_value1("my_special_boundary");
  SdmString attr_value2("us-ascii");
  SdmStringL attributes;
  SdmString check_value;

  attr_flag = Sdm_MAA_charset;
  attr_flags = Sdm_MAA_ALL;
  attributes.AddElementToList(attr_str);
                    
  if (body->GetAttribute(err, r_attribute, attr_str)) {
    cout << "*** Error: GetAttribute failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (body->GetAttributes(err, r_attributes)){
    cout << "*** Error: GetAttributes1 failed. err = " << (int)err << "\n";
    localError = err;
  }

  r_attributes.ClearAllElements();
  if (body->GetAttributes(err, r_attributes, attributes)){
    cout << "*** Error: GetAttributes2 failed. err = " << (int)err << "\n";
    localError = err;
  }
  r_attributes.ClearAllElements();
  if (body->GetAttributes(err, r_attributes, attr_flags)){
    cout << "*** Error: GetAttributes3 failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (body->SetAttribute(err, attr_str, attr_value1)){
    cout << "*** Error: SetAttributes1 failed. err = " << (int)err << "\n";
    localError = err;
  }
  
  if (body->GetAttribute(err, check_value, attr_str)) {
    cout << "*** Error: GetAttribute2 failed. err = " << (int)err << "\n";
    localError = err;
  }

  if (check_value != attr_value1) {
    cout << "*** Error: value does not match the one that was set.\n";
    localError = Sdm_EC_Fail;
  }
  
  if (body->SetAttribute(err, attr_flag, attr_value2)){
    cout << "*** Error: SetAttributes2 failed. err = " << (int)err << "\n";
    localError = err;
  }

  return localError;
}



  
SdmErrorCode
SdmMessageBodyTest::TestGetContents(SdmMessageBody *body, SdmReplyQueue* frontEndQueue,
  int readFileDesc) 
{
  SdmError err;
  SdmContentBuffer r_contents;
  
  // first test sync call.
  if (body->GetContents(err, r_contents)) {
    cout << "*** Error: GetContents failed.\n";
    return err;
  }
  
  if (r_contents.GetContents(err, lastContentBuffer)) {
    cout << "*** Error: can't get contents of content buffer.\n";
    return err;
  }
  lastMessageBody = body;
  getContentsCallbackError = Sdm_False;
  getContentsCallbackCalled = Sdm_False;
  
  SdmServiceFunction svf(&GetContentsCallback,
        (void*)SdmMessageBodyTest::kUserData, Sdm_SVF_Any);

  // test async call.
  if (body->GetContents_Async(err, svf, (void*)kUserData)){
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
SdmMessageBodyTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "SessionCallback Called.\n";
  if (userdata != SdmMessageBodyTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
  }
  
  // this callback should only be called for unsolicited events.  
  // we set this flag and check it in Run to make sure it never got called.
  if (event->sdmBase.type != Sdm_Event_CheckNewMail && event->sdmBase.type != Sdm_Event_AutoSave) {
    sessionCallbackCalled = Sdm_True;
  }
}
 


void 
SdmMessageBodyTest::GetContentsCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  getContentsCallbackCalled = Sdm_True;
 
  cout << "getContentsCallback Called.\n";
  
  // check userdata in event.
  if (userdata != SdmMessageBodyTest::kUserData) {
    cout << "*** Error: userdata given is not correct.\n";
    getContentsCallbackError = Sdm_True;
  }
 
  switch (event->sdmBase.type) {
    case Sdm_Event_GetMessageBodyContents:
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
        if (event->sdmGetContents.messageNumber != lastMessageBody->_ParentMessgNum) {
          cout << "*** Error: message number does not match in event.\n";
          getContentsCallbackError = Sdm_True;
        }
        if (event->sdmGetContents.bodyPartNumber != lastMessageBody->_BodyPartNum) {
          cout << "*** Error: body part number does not match in event.\n";
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


