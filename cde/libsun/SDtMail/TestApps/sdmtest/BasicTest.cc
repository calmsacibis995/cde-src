/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the expunging of deleted messages calls.

#include <unistd.h>
#include <stropts.h>
#include <poll.h>

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


SdmErrorCode RunBasicTest()
{
  cout << endl << "************RunBasicTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;

  SdmError err;
  SdmString kServiceOption("serviceoption");
  SdmString kServiceType("servicetype");
  SdmString kServiceClass("serviceclass");

  SdmConnection *mcon  = new SdmConnection("TestSession");

  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }

  SdmMailRc *mailRc;
  // create mailRc
  if (mcon->SdmMailRcFactory(err, mailRc)) {
    cout << "*** Error: create mailrc failed\n";
    return err;
  }
 
  SdmSession *ises;
  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, ises, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed\n";
    return err;
  }
  if (ises->StartUp(err)) {
    cout << "*** Error: Sdm Incoming Session StartUp Failed\n" ;
    return err;
  }

  // attach to session.
  int i_readFileDesc;
  void *vptr;
  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises)) {
    cout << "*** Error: attach incoming session failed.\n";
    return err;
  }
  SdmReplyQueue* i_frontEndQueue = (SdmReplyQueue*)vptr;

  // create and start up message store.
  SdmMessageStore *mbox;
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (mbox->StartUp(err)) {
    cout << "*** Error: Message store StartUp Failed\n" ;
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

  // create message, body, and envelope.
  SdmMessage *message;
  if (mbox->SdmMessageFactory(err, message, 1)) {
    cout << "*** Error: SdmMessageFactory failed.\n";
    return err;
  }
  SdmMessageBody *body;
  if (message->SdmMessageBodyFactory(err, body, 1)) {
    cout << "*** Error: SdmMessageBodyFactory failed.\n";
    return err;
  }
  SdmMessageEnvelope *envelope;
  if (message->SdmMessageEnvelopeFactory(err, envelope)) {
    cout << "*** Error: SdmMessageEnvelopeFactory failed.\n";
    return err;
  }


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

  // clean up objects.
  // note: don't delete envelope or body.  they got deleted by message.
  // note: don't delete message.  it got deleted by session.
  // note: don't delete session.  it got deleted by connection.
  delete mcon;


  // Session should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(i_frontEndQueue, &i_readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
 
  return err;
}



