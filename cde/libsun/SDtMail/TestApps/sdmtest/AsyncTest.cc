/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the async calls.

#include <iostream.h>
#include <stdlib.h>
#include "Main.hh"
#include <SDtMail/Connection.hh>
#include <Manager/ISess.hh>
#include <Manager/OSess.hh>
#include <SDtMail/MessageStore.hh>
#include <DataStore/OutgoingStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/Token.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageBody.hh>

#include <unistd.h>
#include <stropts.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>

static  const char* kOutgoingStoreName = "/tmp/msgstore.test";

void
PrintSizeOfSelectedObjects()
{
  cout << endl << "************SdmMain Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;

  cout << "Size Of SdmPrim: " << sizeof(SdmPrim) << " Bytes" << endl;
  cout << "Size Of SdmConnection: " << sizeof(SdmConnection) << " Bytes" << endl;
  cout << "Size Of SdmRequest: " << sizeof(SdmRequest) << " Bytes" << endl;
  cout << "Size Of SdmRequestQueue: " << sizeof(SdmRequestQueue)
       << " Bytes" << endl;
  cout << "Size Of SdmMessage: " << sizeof(SdmMessage) << " Bytes" << endl;
  cout << "Size Of SdmMessageEnvelope: " << sizeof(SdmMessageEnvelope) << " Bytes" << endl;
  cout << "Size Of SdmMessageBody: " << sizeof(SdmMessageBody) << " Bytes" << endl;
  cout << "Size Of SdmMessageStore: " << sizeof(SdmMessageStore)
       << " Bytes" << endl;
  cout << "Size Of SdmOutgoingStore: " << sizeof(SdmOutgoingStore)
       << " Bytes" << endl;
  cout << "======================================================" << endl;
  cout << endl;
}


 
class SdmAsyncTest
{
public:
     SdmAsyncTest();
     virtual ~SdmAsyncTest();
 
     virtual SdmErrorCode Run();
 
 
private:
  static const char* kMyUserData;
 
  static void AsyncTestCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
};



 
const char* SdmAsyncTest::kMyUserData = "hello world";


SdmErrorCode RunAsyncTest()
{
  cout << endl << "************ RunAsyncTest Invoked. *************" << endl
          << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmAsyncTest test;
  return test.Run();
}
 
 

SdmAsyncTest::SdmAsyncTest()
{
}
 
SdmAsyncTest::~SdmAsyncTest()
{
}


SdmErrorCode
SdmAsyncTest::Run()
{
  cout << endl << "************ SdmAsyncTest::Run Invoked.*************" 
       << endl <<  endl;
  cout << "======================================================" << endl;
  cout << endl;

  SdmString kServiceOption("serviceoption");
  SdmString kServiceType("servicetype");
  SdmString kServiceClass("serviceclass");

  SdmError err;
  SdmConnection* mcon;
  SdmIncomingSession* ises;
  SdmOutgoingSession* oses;
  SdmSession* ses;
  SdmToken opTok;
  SdmMessageStore* mbox, *mbox2;
  SdmMessageStore* dbox;
  SdmMessage *msg, *msg2, *dft;

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

  mcon = new SdmConnection("TestGUI");
 
  if (mcon != NULL)
    cout << "Sdm Connection Creation Successfull" << endl;
  else
    cout << "Sdm Connection Creation Failed" << endl;

  int st = mcon->StartUp(err);
  if (st == Sdm_EC_Success)
    cout << "Sdm Connection StartUp Successfull" << endl;
  else
    cout << "Sdm Connection StartUp Failed" << endl;

  mcon->SdmSessionFactory(err, ses, Sdm_ST_InSession);
  ses->StartUp(err);
  ises = (SdmIncomingSession*) ses;

  mcon->SdmSessionFactory(err, ses, Sdm_ST_OutSession);
  ses->StartUp(err);
  oses = (SdmOutgoingSession*) ses;

  ises->SdmMessageStoreFactory(err, mbox);
  mbox->StartUp(err);

  ises->SdmMessageStoreFactory(err, mbox2);
  mbox2->StartUp(err);
  
  oses->SdmMessageStoreFactory(err, dbox);
  dbox->StartUp(err);
	  
  //-----------------------------
  SdmServiceFunction svf(&AsyncTestCallback, (void*)kMyUserData, Sdm_SVF_Any);
  SdmContentBuffer buf;
  
  char buffer[2000];
  sprintf(buffer, "Main running on thread: %d\n", thr_self());
  cout << buffer;

  int readFileDesc;
  void *frontEndQueue;
  int readFileDesc2;
  void *frontEndQueue2;
  
  if (SdmAttachSession(err, readFileDesc, frontEndQueue, *ises)) {
    cout << "Error in attaching to input session.\n";
    return err;
  }
  if (SdmAttachSession(err, readFileDesc2, frontEndQueue2, *oses)) {
    cout << "Error in attaching to output session.\n";
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

  if (mbox2->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  SdmToken outtoken;
  outtoken.SetValue("serviceclass","messagestore");
  outtoken.SetValue("servicetype","local");
  outtoken.SetValue("serviceoption","messagestorename",kOutgoingStoreName);
  outtoken.SetValue("serviceoption","disconnected");

  if (dbox->Open(err, nmsgs, readOnly, outtoken)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }
  
  const int kTotalCalls = 50;
    
  for (int i =0; i < kTotalCalls; i++) {
      mbox->SdmMessageFactory(err, msg);
      dbox->SdmMessageFactory(err, dft);
      mbox2->SdmMessageFactory(err, msg2); 
      
      if (i%5 == 0) {
        cout << "Start sync call.\n";
        msg->GetContents(err, buf, i, 9999);  // test synchronous call.
        cout << "End sync call.\n";
      } else {
        msg->GetContents_Async(err, svf, NULL, i, 9999);
      }

      dft->GetContents_Async(err, svf, NULL, i, 8888);   // test access to non-server queue.
      msg2->GetContents_Async(err, svf, NULL, i, 7777);  // test access to >1 mboxes.
  }

  //-----------------------------

  struct pollfd pollFileDesc;
  pollFileDesc.fd = readFileDesc;
  pollFileDesc.events = POLLIN;
  int rc;
  
  for (i =0; i >= 0 ; i++) {
    if ((rc = poll(&pollFileDesc, 1, 3000)) > 0) {
      if (SdmHandleSessionEvent(frontEndQueue, &readFileDesc, NULL) < 0) {
        sprintf(buffer, "*** SdmHandleSessionEvent failed after processing %d events.\n", i);
        cout << buffer;
        err = Sdm_EC_Fail;
        break;
      }

    } else if (rc == 0) {
    	sprintf(buffer, "*** Poll timed out after processing %i events.\n", i);
        cout << buffer;
    	break;
    } else {
      err = Sdm_EC_Fail;
      cout << "*** Error: Poll failed.\n";
      break;
    }
  }

  // should have 90 events from the input session.  the sync calls should
  // not have generated any events.
  if (i != 90) {
      err = Sdm_EC_Fail;
      sprintf(buffer, "*** Error Poll timed out after processing %i events.\n", i);
      cout << buffer;
  }
  
  pollFileDesc.fd = readFileDesc2;
  for (i =0; i >= 0 ; i++) {
    if ((rc = poll(&pollFileDesc, 1, 3000)) > 0) {
      if (SdmHandleSessionEvent(frontEndQueue2, &readFileDesc2, NULL) < 0) {
        sprintf(buffer, "*** SdmHandleSessionEvent failed after processing %d events.\n", i);
        cout << buffer;
        err = Sdm_EC_Fail;
        break;
      }

    } else if (rc == 0) {
      sprintf(buffer, "*** Poll timed out after processing %i events.\n", i);
      cerr << buffer;
      break;
    } else {
      err = Sdm_EC_Fail;
      cout << "*** Error: Poll failed.\n";
      break;
    }
  }
  
  // should have 50 events from the output session.
  if (i != 50) {
      err = Sdm_EC_Fail;
      sprintf(buffer, "*** Error Poll timed out after processing %i events.\n", i);
      cout << buffer;
  }
  
  cout << "**** We're done!! ****\n";

  delete mcon;
 
  // Sessions should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(frontEndQueue, &readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
 
  if (SdmHandleSessionEvent(frontEndQueue2, &readFileDesc2, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }

  return err;;
}

void 
SdmAsyncTest::AsyncTestCallback(SdmError& err, void* userdata, SdmServiceFunctionType type, 
  SdmEvent* event)
{
  assert (event != NULL);

  cerr << "AsyncTestCallback Called.\n";
  if (userdata != kMyUserData) {
    cout << "*** Error: userdata given is not correct.\n";
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
        } 
        
        sprintf(buffer, "AsyncTestCallback called with args: startOffset=%d; length=%d; buffer=%s\n",
          stOffS, length, (const char *)str_contents);
        cerr << buffer;

        delete (event->sdmGetContents.buffer);

        break;
      }
      
    default:
      cout << "Error: AsyncTestCallback default event received.\n";
      break;      
  }
}

