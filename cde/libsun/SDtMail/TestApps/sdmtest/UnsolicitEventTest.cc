/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the expunging of deleted messages calls.

#include <unistd.h>
#include <stropts.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Xm/Xm.h>

#include "Main.hh"
#include <SDtMail/MailRc.hh>

#include <SDtMail/Connection.hh>
#include <SDtMail/Session.hh>
#include <Manager/ISess.hh>
#include <Manager/OSess.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <DataStore/OutgoingStore.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <thread.h>
#include <SDtMail/DeliveryResponse.hh>


static const int        READ = 0;
static const int        WRITE = 1;


extern SdmErrorCode SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata,
                      SdmSession &session, void* appContext);




static SdmString kServiceOption("serviceoption");
static SdmString kServiceType("servicetype");
static SdmString kServiceClass("serviceclass");
  
static const char* kOutgoingStoreName = "/tmp/unsolicitevt.test";


class SdmUnsolicitEventTest
{
public:
     SdmUnsolicitEventTest();
     virtual ~SdmUnsolicitEventTest();
     
     virtual SdmErrorCode Run();

     void SetFinishSleep(SdmBoolean b) { _finishSleep = b; }

  static void SessionCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void CheckMailCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void AutoSaveCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void DebugLogCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void LastInteractiveCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);
  static void CheckCheckNewMailEvent(SdmEvent* event, SdmBoolean &errorFlag);

  static void HandleSleepTimeOut(XtPointer, XtIntervalId*);

private:
  static const char* kUserData;
  static SdmBoolean checkMailCallbackCalled;
  static SdmBoolean checkMailCallbackError;
  static SdmBoolean expectedCheckMailStartState;
  static SdmBoolean expectedAutoSaveStartState;
  static SdmBoolean sessionCallbackCalled;
  static SdmBoolean sessionCallbackError;
  static SdmBoolean autosaveCallbackCalled;
  static SdmBoolean autosaveCallbackError;
  static SdmBoolean debugLogCallbackCalled;
  static SdmBoolean debugLogCallbackError;
  static SdmBoolean lastInteractiveCallbackCalled;
  static SdmBoolean lastInteractiveCallbackError;
  
  static time_t lastInteractiveTime;
  static int    lastInteractiveCallCount;

  SdmErrorCode Setup();
  SdmErrorCode Cleanup();

  SdmErrorCode TestNoRegisteredFunctions();
  SdmErrorCode TestMailCheckAtSession();
  SdmErrorCode TestLastInteractiveTime();
  SdmErrorCode TestMailCheckAtStore();
  SdmErrorCode TestAutoSave();
  SdmErrorCode TestDataPortEvent();

  SdmErrorCode PollForReply(SdmReplyQueue*, int readfd, int expectedCalls);
  SdmErrorCode StartUpAndOpen(SdmMessageStore *store);
  SdmErrorCode SleepOnMainThread(const time_t secsToSleep);

  int i_readFileDesc, o_readFileDesc;
  SdmReplyQueue *i_frontEndQueue, *o_frontEndQueue;
  SdmSession *ises, *oses;
  SdmConnection *mcon;
  SdmMessageStore *mbox, *dbox;                     

  SdmBoolean   _finishSleep;
  Widget       _toplevel;
  XtAppContext _appContext;
};

const char* SdmUnsolicitEventTest::kUserData = "message store test data";
SdmBoolean SdmUnsolicitEventTest::checkMailCallbackCalled;
SdmBoolean SdmUnsolicitEventTest::checkMailCallbackError;
SdmBoolean SdmUnsolicitEventTest::expectedCheckMailStartState;
SdmBoolean SdmUnsolicitEventTest::expectedAutoSaveStartState;
SdmBoolean SdmUnsolicitEventTest::sessionCallbackCalled;
SdmBoolean SdmUnsolicitEventTest::sessionCallbackError;
SdmBoolean SdmUnsolicitEventTest::autosaveCallbackCalled;
SdmBoolean SdmUnsolicitEventTest::autosaveCallbackError;
SdmBoolean SdmUnsolicitEventTest::debugLogCallbackCalled;
SdmBoolean SdmUnsolicitEventTest::debugLogCallbackError;
SdmBoolean SdmUnsolicitEventTest::lastInteractiveCallbackCalled;
SdmBoolean SdmUnsolicitEventTest::lastInteractiveCallbackError;
time_t     SdmUnsolicitEventTest::lastInteractiveTime;
int        SdmUnsolicitEventTest::lastInteractiveCallCount;






SdmErrorCode RunUnsolicitEventTest()
{
  cerr << endl << "************RunUnsolicitEventTest Invoked.*************" << endl << endl;
  cerr << "======================================================" << endl;
  cerr << endl;
  SdmUnsolicitEventTest test;
  return test.Run();
}

SdmUnsolicitEventTest::SdmUnsolicitEventTest()
{
}

SdmUnsolicitEventTest::~SdmUnsolicitEventTest()
{
}

SdmErrorCode
SdmUnsolicitEventTest::Run()
{
  cerr << endl << "************ SdmUnsolicitEventTest::Run Invoked. *************" << endl << endl;
  cerr << endl;
    
  cerr << endl << "*** Info: Warning - this test runs for about 2 minutes before output is printed\n";
  cerr << endl;

  SdmErrorCode errorCode;
  SdmError err;
  
  if ((errorCode = Setup()) != Sdm_EC_Success)
    return errorCode;
  
  if ((errorCode = TestNoRegisteredFunctions()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestMailCheckAtSession()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestLastInteractiveTime()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestMailCheckAtStore()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestAutoSave()) != Sdm_EC_Success)
    return errorCode;

  if (ises->UnregisterServiceFunction(err, Sdm_SVF_Any)) {
    cerr << "*** Error: UnregisterServiceFunction failed for session.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if ((errorCode = TestDataPortEvent()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = Cleanup()) != Sdm_EC_Success)
    return errorCode;  

  return err;
}


SdmErrorCode
SdmUnsolicitEventTest::TestNoRegisteredFunctions()
{
  SdmError err;
  SdmErrorCode errorCode;

  // -------------------------------------------------------------
  // first test without any service functions registered.
  // -------------------------------------------------------------  
  cerr << "*** Info: Testing without any service funcs registered.\n";

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;


  if ((errorCode = SleepOnMainThread(20)) != Sdm_EC_Success)
    return errorCode;


  // queue should be empty.  we haven't registered any service functions.
  if (i_frontEndQueue->ElementCount() != 0) {
    cerr << "*** Error: reply queue contains events.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  

  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }
  
  return err;
}





SdmErrorCode
SdmUnsolicitEventTest::TestMailCheckAtSession()
{
  SdmError err;
  SdmErrorCode errorCode;
  // -------------------------------------------------------------
  // next test with service function registered at Session.
  // -------------------------------------------------------------  
  cerr << "\n*** Info: Testing service func registered in Session.\n";

  // register service function to check for new mail.
  SdmServiceFunction svf(&SessionCallback,
          (void*)SdmUnsolicitEventTest::kUserData,
          Sdm_SVF_Any);

  expectedCheckMailStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;

  if (ises->RegisterServiceFunction(err, svf)) {
    cerr << "*** Error: register service function failed\n";
    return err;
  }

  // time specified below must be > 15 (which is the interval
  // for mail check) and <60 (which is the save interval).
  // we don't want any replies being created for the autosave
  // because it will make the callback checking more difficult.
  if ((errorCode = SleepOnMainThread(20)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should have gotten some events in the queue.
  if (i_frontEndQueue->ElementCount() == 0) {
    cerr << "*** Error: reply queue empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // we'll just poll until we empty the queue since we don't
  // know exactly how may replies we'll get.
  if (PollForReply(i_frontEndQueue, i_readFileDesc, -1)) {
    return err;
  }
  
  // the checkMailCallback should not have been called.
  if (checkMailCallbackCalled || autosaveCallbackCalled) {
    cerr << "*** Error: wrong callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the session callback should have been called.
  if (!sessionCallbackCalled) {
    cerr << "*** Error: session callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (sessionCallbackError) {
    cerr << "*** Error: error occurred in session callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // the expected start state should be true.  otherwise, there
  // was not an even number of replies sent for each mail check.
  if (expectedCheckMailStartState != Sdm_True) {
    cerr << "*** Error: odd number of events processed.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  return err;

}


SdmErrorCode
SdmUnsolicitEventTest::TestLastInteractiveTime()
{
  SdmError err;
  SdmErrorCode errorCode;
  // -------------------------------------------------------------
  // next test handling check mail with various inactivity times.
  // -------------------------------------------------------------  
  cerr << "\n*** Info: Testing last interactive time.\n";
  SdmMailRc::GetMailRc()->SetValue(err, "inactivityinterval", "15");
  if (err) {
     cerr << "*** Error: can't set inactivityinterval.\n";
     return err;
  }

  // not that session callback should still be set from 
  // TestMailCheckAtSession.

  cerr << "\n*** Info: Testing lastInteractiveTime > inactivityInterval.\n";
    
  // register service function to last interactive calback.
  SdmServiceFunction svf(&LastInteractiveCallback,
          (void*)SdmUnsolicitEventTest::kUserData,
          Sdm_SVF_LastInteractiveEvent);

  expectedCheckMailStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;
  lastInteractiveCallbackCalled = Sdm_False;
  lastInteractiveCallbackError = Sdm_False;
  lastInteractiveCallCount = 0;
  
  // set the last interactive time to be now.  the mail check 
  // callback should be called.
  lastInteractiveTime = time(NULL);

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;

  if (mcon->RegisterServiceFunction(err, svf)) {
    cerr << "*** Error: register service function failed\n";
    return err;
  }

  // time specified below must be > 15 (which is the interval
  // for mail check) and <60 (which is the save interval).
  // we don't want any replies being created for the autosave
  // because it will make the callback checking more difficult.
  if ((errorCode = SleepOnMainThread(20)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should have gotten some events in the queue.
  if (i_frontEndQueue->ElementCount() == 0) {
    cerr << "*** Error: reply queue empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // we'll just poll until we empty the queue since we don't
  // know exactly how may replies we'll get.
  if (PollForReply(i_frontEndQueue, i_readFileDesc, -1)) {
    return err;
  }
  
  // the checkMailCallback should not have been called.
  if (checkMailCallbackCalled || autosaveCallbackCalled) {
    cerr << "*** Error: wrong callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the session callback should have been called.
  if (!sessionCallbackCalled) {
    cerr << "*** Error: session callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (sessionCallbackError) {
    cerr << "*** Error: error occurred in session callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // the expected start state should be true.  otherwise, there
  // was not an even number of replies sent for each mail check.
  if (expectedCheckMailStartState != Sdm_True) {
    cerr << "*** Error: odd number of events processed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // make sure that lastInteractiveCallback was called.
  if (!lastInteractiveCallbackCalled) {
    cerr << "*** Error: lastInteractiveCallbackCalled was not called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallbackError) {
    cerr << "*** Error: lastInteractiveCallback error occurred.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallCount < 1) {
    cerr << "*** Error: unexpected number of calls to last interactive callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  cerr << "\n*** Info: Testing lastInteractiveTime < inactivityInterval.\n";
  
  expectedCheckMailStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;
  lastInteractiveCallbackCalled = Sdm_False;
  lastInteractiveCallbackError = Sdm_False;
  lastInteractiveCallCount = 0;
  
  // set the last interactive 10 seconds from now.
  // that way, when the check mail occurs, it should not
  // handle the mail check because there is not enough 
  // inactivity time.
  // 
  lastInteractiveTime = time(NULL) + 10;

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;


  // time specified below must be > 15 (which is the interval
  // for mail check) and <60 (which is the save interval).
  // we don't want any replies being created for the autosave
  // because it will make the callback checking more difficult.
  if ((errorCode = SleepOnMainThread(20)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should not get any events in the queue.
  if (i_frontEndQueue->ElementCount() != 0) {
    cerr << "*** Error: reply queue not empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }
    
  // no should not have been called.
  if (checkMailCallbackCalled || autosaveCallbackCalled || sessionCallbackCalled) {
    cerr << "*** Error: some callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // make sure that lastInteractiveCallback was called.
  if (!lastInteractiveCallbackCalled) {
    cerr << "*** Error: lastInteractiveCallbackCalled was not called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallbackError) {
    cerr << "*** Error: lastInteractiveCallback error occurred.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallCount < 1) {
    cerr << "*** Error: unexpected number of calls to last interactive callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  cerr << "\n*** Info: Testing last interactive callback called twice to finally handle event.\n";
  
  expectedCheckMailStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;
  lastInteractiveCallbackCalled = Sdm_False;
  lastInteractiveCallbackError = Sdm_False;
  lastInteractiveCallCount = 0;
  
  // set the last interactive 8 seconds from now.
  lastInteractiveTime = time(NULL) + 8;

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;


  // time specified for sleep is 35 because of the following setup:
  // * 15 (check interval)
  // * 8 (delay) 
  // * 15 (inactivity interval)
  // * 35 (allows for 2 check intervals only if there is no problem with meeting inactivity interval)
  //
  // With the sleep time set to 35 and the last interactive interval set to now+8, the following
  // should happen:
  // a.  check mail event wakes up at now+15. it checks inactivity and sees that not enough
  //     time has gone by so it resets the next delay interval to be 8 seconds as calculated below:
  //     (inactivityInterval - (wakeupTime - latestInteractiveTime)) = (15 - ((now+15) - (now+8)) )
  //
  // b.  check mail event will wake up at now+23.  enough inactivity time will have passed and
  //     the check mail will occur.  it will then set the delay time to 15 (original check mail interval).
  //
  // c.  check mail event should wake up at now+38 but since we only sleep for 35 seconds, the 
  //     second check mail event should never happen.  therefore, no other last interactive callback
  //     should be called either.
  //
  // d.  what to check for:  last interactive callback should have been called twice and check mail event
  //     callback should have been called twice (for start and stop of one check mail event).
  //
  if ((errorCode = SleepOnMainThread(35)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should not get 2 events in the queue for the check mail (start & stop) events
  // for the case of multi-threading.  In the single-thread case, we only post a
  // reply for the stop event so there should only be one event in the queue.
  // The service function is called directly for the start event.
  //
#if defined(USE_SOLARIS_THREADS)
  if (i_frontEndQueue->ElementCount() != 2) {
#else 
  if (i_frontEndQueue->ElementCount() != 1) {
#endif
    cerr << "*** Error: reply queue contains unexpected number of items.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // we'll just poll until we empty the queue.
  if (PollForReply(i_frontEndQueue, i_readFileDesc, -1)) {
    return err;
  }
    
  // these should not have been called.
  if (checkMailCallbackCalled || autosaveCallbackCalled) {
    cerr << "*** Error: wrong callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // make sure that sessionCallback was called.
  if (!sessionCallbackCalled) {
    cerr << "*** Error: session callback was not called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (sessionCallbackError) {
    cerr << "*** Error: session callback error occurred.\n";
    err = Sdm_EC_Fail;
    return err;
  } 

  // make sure that lastInteractiveCallback was called.
  if (!lastInteractiveCallbackCalled) {
    cerr << "*** Error: lastInteractiveCallbackCalled was not called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallbackError) {
    cerr << "*** Error: lastInteractiveCallback error occurred.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (lastInteractiveCallCount < 2) {
    cerr << "*** Error: unexpected number of calls to last interactive callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // set last interactive time to current time so that later
  // parts of the test can proceed without having the inactivity interval causing
  // the event to be skipped.
  lastInteractiveTime = time(NULL);
  
  return err;

}


SdmErrorCode
SdmUnsolicitEventTest::TestMailCheckAtStore()
{
  SdmError err;
  SdmErrorCode errorCode;

  // -------------------------------------------------------------
  // next test with service function registered at message store.
  // -------------------------------------------------------------  
  cerr << "\n*** Info: Testing service func registered in Message Store.\n";

  // register service function to check for new mail.
  SdmServiceFunction svf2(&CheckMailCallback,
          (void*)SdmUnsolicitEventTest::kUserData,
          Sdm_SVF_CheckNewMail);

  // set up flags.
  expectedCheckMailStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;

  if (mbox->RegisterServiceFunction(err, svf2)) {
    cerr << "*** Error: register service function failed\n";
    return err;
  }

  if ((errorCode = SleepOnMainThread(20)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should have gotten some events in the queue.
  if (i_frontEndQueue->ElementCount() == 0) {
    cerr << "*** Error: reply queue empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // we'll just poll until we empty the queue since we don't
  // know exactly how may replies we'll get.
  if (PollForReply(i_frontEndQueue, i_readFileDesc, -1)) {
    return err;
  }
  
  
  if (sessionCallbackCalled || autosaveCallbackCalled) {
    cerr << "*** Error: wrong callback called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the check mail callback should have been called.
  if (!checkMailCallbackCalled) {
    cerr << "*** Error: checkNewMail callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (checkMailCallbackError) {
    cerr << "*** Error: error occurred in checkNewMail callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 
  // the expected start state should be true.  otherwise, there
  // was not an even number of replies sent for each mail check.
  if (expectedCheckMailStartState != Sdm_True) {
    cerr << "*** Error: odd number of events processed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode
SdmUnsolicitEventTest::TestAutoSave()
{
  SdmError err;
  SdmErrorCode errorCode;

  SdmMailRc::GetMailRc()->RemoveValue(err, "dontautosave");
  if (err) {
     cerr << "*** Error: can't remove dontautosave.\n";
     return err;
  }


  // -------------------------------------------------------------
  // next test with autosave service function registered at message store.
  // -------------------------------------------------------------  
  cerr << "\n*** Info: Testing autosave func registered in message store.\n";

  // register service function to check for new mail.
  SdmServiceFunction svf3(&AutoSaveCallback,
          (void*)SdmUnsolicitEventTest::kUserData,
          Sdm_SVF_AutoSave);

  // set up flags.
  expectedCheckMailStartState = Sdm_True;
  expectedAutoSaveStartState = Sdm_True;
  checkMailCallbackError = Sdm_False;
  checkMailCallbackCalled = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;

  if ((errorCode = StartUpAndOpen(mbox)) != Sdm_EC_Success)
    return errorCode;

  // note: CheckNewMail service function is only registered
  // with the session because the ShutDown cleaned up the
  // service functions in the message store.
  
  if (mbox->RegisterServiceFunction(err, svf3)) {
    cerr << "*** Error: register service function failed\n";
    return err;
  }

  // need to sleep for >60 seconds in order for autosave to
  // occur.
  if ((errorCode = SleepOnMainThread(70)) != Sdm_EC_Success)
    return errorCode;


  if (mbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // we should have gotten some events in the queue.
  if (i_frontEndQueue->ElementCount() == 0) {
    cerr << "*** Error: reply queue empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // we'll just poll until we empty the queue since we don't
  // know exactly how may replies we'll get.
  if (PollForReply(i_frontEndQueue, i_readFileDesc, -1)) {
    return err;
  }
  
  // the checkMailCallback should not have been called.
  if (checkMailCallbackCalled) {
    cerr << "*** Error: wrong callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the session callback should have been called.
  if (!sessionCallbackCalled) {
    cerr << "*** Error: session callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (sessionCallbackError) {
    cerr << "*** Error: error occurred in session callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  // the expected start state should be true.  otherwise, there
  // was not an even number of replies sent for each mail check.
  if (expectedCheckMailStartState != Sdm_True) {
    cerr << "*** Error: odd number of events processed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  

  // the autosave callback should have been called.
  if (!autosaveCallbackCalled) {
    cerr << "*** Error: autosave callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (autosaveCallbackError) {
    cerr << "*** Error: error occurred in autosave callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
    // the expected start state should be true.  otherwise, there
  // was not an even number of replies sent for each mail check.
  if (expectedAutoSaveStartState != Sdm_True) {
    cerr << "*** Error: odd number of events processed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}

SdmErrorCode
SdmUnsolicitEventTest::TestDataPortEvent()
{
  SdmError err;
  SdmErrorCode errorCode;

  // create and start up outgoing session.
  if (mcon->SdmSessionFactory(err, oses, Sdm_ST_OutSession)) {
    cerr << "*** Error: create outgoing session failed\n";
    return err;
  }
  if (oses->StartUp(err)) {
    cerr << "*** Error: Sdm outgoing Session StartUp Failed\n" ;
    return err;
  }

  // attach to session.
  void *vptr;
#if defined(USE_SOLARIS_THREADS)
  if (SdmAttachSession(err, o_readFileDesc, vptr, *oses)) {
    cerr << "*** Error: attach outgoing session failed.\n";
    return err;
  }
#else
  if (SdmAttachSession(err, o_readFileDesc, vptr, *oses, _appContext)) {
    cerr << "*** Error: attach incoming session failed.\n";
    return err;
  }
  if (SdmXtAddSessionTimeOuts(err, *oses)) {
     cerr << "*** Error: add time outs to incoming session failed.\n";
     return err;
  }
#endif
  o_frontEndQueue = (SdmReplyQueue*)vptr;

  // create and start up outgoing store store.
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cerr << "*** Error: create outgoing store failed\n";
     return err;
  }

  // -------------------------------------------------------------
  // test data port event.
  // -------------------------------------------------------------  
  cerr << "\n*** Info: Testing dataport debug log event.\n";

  // register service function to check for new mail.
  SdmServiceFunction svf3(&DebugLogCallback,
          (void*)SdmUnsolicitEventTest::kUserData,
          Sdm_SVF_DebugLog);

  // set up flags.
  debugLogCallbackCalled = Sdm_False;
  debugLogCallbackError = Sdm_False;
  sessionCallbackCalled = Sdm_False;
  sessionCallbackError = Sdm_False;
  autosaveCallbackCalled = Sdm_False;
  autosaveCallbackError = Sdm_False;

  if ((errorCode = StartUpAndOpen(dbox)) != Sdm_EC_Success)
    return errorCode;

  // note: CheckNewMail service function is only registered
  // with the session because the ShutDown cleaned up the
  // service functions in the message store.
  
  if (dbox->RegisterServiceFunction(err, svf3)) {
    cerr << "*** Error: register service function failed\n";
    return err;
  }

  // calling check for new messages should cause a debug
  // message to be printed.
  SdmMessageNumber  nummsgs;
  if (dbox->CheckForNewMessages(err, nummsgs)) {
    cerr << "*** Error: CheckForNewMessages failed\n";
    return err;
  }

  if (dbox->ShutDown(err)) {
    cerr << "*** Error: Message store Shutdown Failed\n" ;
    return err;
  }

  // need to wait for replies to get processed.  
  // poll and check the reply queue and continue until it's empty.
  SdmBoolean replyQueueEmpty = Sdm_False;
  const int kMaxTries = 5;
  for (int i=0; i<kMaxTries; i++) {
    // break out of loop if the reply queue is empty.
    if (oses->GetReplyQueue()->ElementCount() == 0) {
      replyQueueEmpty = Sdm_True;
      break;
    }
   
    // if reply queue is not empty, sleep and let replies get 
    // processed.
    if ((errorCode = SleepOnMainThread(5)) != Sdm_EC_Success)
      return errorCode;
  }

  if (replyQueueEmpty == Sdm_False) {
    cerr << "*** Error: reply queue never emptied.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the reply queue should be empty.  debug callback is
  // called directly and doesn't go through the queue.
  if (o_frontEndQueue->ElementCount() != 0) {
    cerr << "*** Error: front end reply queue not empty.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the session callback should not have been called.
  if (sessionCallbackCalled || autosaveCallbackCalled) {
    cerr << "*** Error: wrong callback was called.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // the debug log callback should have been called.
  if (!debugLogCallbackCalled) {
    cerr << "*** Error: debug log callback never called.\n";
    err = Sdm_EC_Fail;
    return err;
  } else if (debugLogCallbackError) {
    cerr << "*** Error: error occurred in debug log callback.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  return err;
}


SdmErrorCode
SdmUnsolicitEventTest::StartUpAndOpen(SdmMessageStore *store)
{
  SdmError err;
  SdmToken token;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;
  
  if (store == mbox) {
    token.SetValue(kServiceClass,SdmString("messagestore"));
    token.SetValue(kServiceType,SdmString("test"));
    token.SetValue(kServiceOption,SdmString("messagestorename"),SdmString("test.mbx"));
  } else {
    token.SetValue(kServiceClass,SdmString("messagestore"));
    token.SetValue(kServiceType,SdmString("local"));
    token.SetValue(kServiceOption,SdmString("messagestorename"), SdmString(kOutgoingStoreName));
  }
  
  if (store->StartUp(err)) {
    cerr << "*** Error: Message store StartUp Failed\n" ;
    return err;
  }

  // open the message stores.
  if (store->Open(err, nmsgs, readOnly, token)) {
    cerr << "*** Error: Open failed.\n";
    return err;
  }
  
  return err;
}


SdmErrorCode
SdmUnsolicitEventTest::SleepOnMainThread(const time_t secToSleep)
{
  SdmError err;
  
#if defined(USE_SOLARIS_THREADS)
  cond_t _TimeCondVar;
  int rc = cond_init(&_TimeCondVar, USYNC_THREAD, 0);
  if (rc != 0) {
    cerr << "*** Error: cond_init failed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  mutex_t _DummyMut;
  rc =  mutex_init(&_DummyMut, USYNC_THREAD, NULL);
  if (rc != 0) {
    cout << "*** Error: mutex_init failed.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  timestruc_t waitTime;
  waitTime.tv_sec = time(NULL) + secToSleep;
  waitTime.tv_nsec = 0;
  cerr << "*** Info: main thread going to sleep...\n";
  cond_timedwait(&_TimeCondVar, &_DummyMut, &waitTime);
  cerr << "*** Info: main thread wake up...\n";

#else

  XEvent    event;

  _finishSleep = Sdm_False;

  // create a time out to wake us up when the sleep interval is done.
  XtIntervalId  id = XtAppAddTimeOut(_appContext, secToSleep*1000,
                   SdmUnsolicitEventTest::HandleSleepTimeOut, this);

  cerr << "*** Info: main thread going to sleep...\n";

  // we loop until the _finishSleep flag is set.  this flag will 
  // get set when the time out above (HandleSleepTimeOut) is called.
  //
  for (;;) {
    cerr << "XtAppNextEvent start.\n";
    XtAppNextEvent( _appContext, &event );
    cerr << "XtAppNextEvent end.\n";
    if (_finishSleep) break;
    XtDispatchEvent( &event );
  }

  cerr << "*** Info: main thread wake up...\n";

#endif


  return err;
}



SdmErrorCode
SdmUnsolicitEventTest::Setup()
{
  SdmError err;
  mcon  = new SdmConnection("TestSession");

  // first, set up the appropriate intervals for the events.
  // must do this before creating session.
  SdmMailRc *mail_rc = SdmMailRc::GetMailRc();
  if (mail_rc == NULL) {
     cerr << "*** Error: can't get mailrc object\n";
     return err;
  }
  
  mail_rc->SetValue(err, "retrieveinterval", "15");
  if (err) {
     cerr << "*** Error: can't set retrieveinterval\n";
     return err;
  }
    
  mail_rc->SetValue(err, "saveinterval", "1");
  if (err) {
     cerr << "*** Error: can't set saveinterval.\n";
     return err;
  }

  mail_rc->SetValue(err, "dontautosave", "true");
  if (err) {
     cerr << "*** Error: can't set inactivityinterval.\n";
     return err;
  }


  // next, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp unsolicitevent.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cerr << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (mcon == NULL) {
    cerr << "*** Error: Sdm Connection Creation Failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mcon->StartUp(err)) {
    cerr << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }
 
  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, ises, Sdm_ST_InSession)) {
    cerr << "*** Error: create incoming session failed\n";
    return err;
  }
  if (ises->StartUp(err)) {
    cerr << "*** Error: Sdm Incoming Session StartUp Failed\n" ;
    return err;
  }

  // attach to session.
  void *vptr;

#if defined(USE_SOLARIS_THREADS)
  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises)) {
    cerr << "*** Error: attach incoming session failed.\n";
    return err;
  }
#else
  int argc;
  char **argv = NULL;

  _toplevel = XtVaAppInitialize(&_appContext, "Hello", NULL, 0,
        &argc, argv, NULL, NULL);
 
  XtRealizeWidget(_toplevel);

  // we register for the key press event (do this after we realize
  // the widget).  this event is sent during our test to wake 
  // up the main thread when it is asleep in the X event loop
  // (see SleepOnMainThread method).
  //
  XSelectInput(XtDisplay(_toplevel), XtWindow(_toplevel), KeyPressMask);

  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises, _appContext)) {
    cerr << "*** Error: attach incoming session failed.\n";
    return err;
  }

  // the following needs to be called otherwise the timeouts are not setup.
  // this must be done after the app context is defined and after
  // the session has been attached.
  //
  if (SdmXtAddSessionTimeOuts(err, *ises)) {
     cerr << "*** Error: add time outs to incoming session failed.\n";
     return err;
  }

#endif

  i_frontEndQueue = (SdmReplyQueue*)vptr;

  // create and start up message store.
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cerr << "*** Error: create message store failed\n";
     return err;
  }
  
  return err;
}

SdmErrorCode
SdmUnsolicitEventTest::Cleanup()
{
  SdmError err;
  
  //
  // shutdown objects
  //
  if (ises->ShutDown(err)) {
    cerr << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (oses->ShutDown(err)) {
    cerr << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (mcon->ShutDown(err)) {
    cerr << "*** Error: SdmConnection shutdown failed.\n";
    return err;
  }


  // clean up objects.
  // note: don't delete envelope or body.  they got deleted by message.
  // note: don't delete message.  it got deleted by session.
  // note: don't delete session.  it got deleted by connection.
  delete mcon;

  // we'll poll until we empty the queue to flush out any remaining
  // events left on the queue.
  if (i_frontEndQueue->ElementCount() > 0) {	
    if (PollForReply(i_frontEndQueue, i_readFileDesc, i_frontEndQueue->ElementCount())) {
      return err;
    }
  }

  if (o_frontEndQueue->ElementCount() > 0) {	
    if (PollForReply(o_frontEndQueue, o_readFileDesc, o_frontEndQueue->ElementCount())) {
      return err;
    }
  }


  // Sessions should all be shut down now and queues are all empty.  
  // Call SdmHandleSessionEvent one more time to cause the delete the 
  // front end queue and closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(i_frontEndQueue, &i_readFileDesc, NULL) >= 0) {
     cerr <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }
 
  if (SdmHandleSessionEvent(o_frontEndQueue, &o_readFileDesc, NULL) >= 0) {
     cerr <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }

  // clean up temp store.
  unlink(kOutgoingStoreName);

  return err;
}


void 
SdmUnsolicitEventTest::CheckMailCallback(SdmError& err, void* userdata, 
  SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
  
  checkMailCallbackCalled = Sdm_True;
 
  cerr << "CheckMailCallback Called.\n";
  if (userdata != SdmUnsolicitEventTest::kUserData) {
    cerr << "*** Error: userdata given is not correct.\n";
    checkMailCallbackError = Sdm_True;
  }
 
  if (type != Sdm_SVF_CheckNewMail) {
    cerr << "*** Error: type argument is not correct.\n";
    debugLogCallbackError = Sdm_True;
  }
  switch (event->sdmBase.type) {
    case Sdm_Event_CheckNewMail:
      {
        CheckCheckNewMailEvent(event, checkMailCallbackError);          
        break;
      }
 
    default:
      cerr << "*** Error: CheckMailCallback default event received.\n";
      checkMailCallbackError = Sdm_True;
      break;
  }
}


void 
SdmUnsolicitEventTest::SessionCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cerr << "SessionCallback Called.\n";
  if (userdata != SdmUnsolicitEventTest::kUserData) {
    cerr << "*** Error: userdata given is not correct.\n";
    sessionCallbackError = Sdm_True;
  }
  
  if (type != Sdm_SVF_Any) {
    cerr << "*** Error: type argument is not correct.\n";
    debugLogCallbackError = Sdm_True;
  }

  sessionCallbackCalled = Sdm_True;

  switch (event->sdmBase.type) {
    case Sdm_Event_CheckNewMail:
      {
        CheckCheckNewMailEvent(event, sessionCallbackError);
        break;
      }
 
    default:
      cerr << "*** Error: SessionCallback default event received.\n";
      cerr << "***         event type = " << (int)event->sdmBase.type << "\n";
      sessionCallbackError = Sdm_True;
      break;
  }
}

void 
SdmUnsolicitEventTest::AutoSaveCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cerr << "AutoSaveCallback Called.\n";
  if (userdata != SdmUnsolicitEventTest::kUserData) {
    cerr << "*** Error: userdata given is not correct.\n";
    autosaveCallbackError = Sdm_True;
  }
  
  if (type != Sdm_SVF_AutoSave) {
    cerr << "*** Error: type argument is not correct.\n";
    debugLogCallbackError = Sdm_True;
  }

  autosaveCallbackCalled = Sdm_True;

  switch (event->sdmBase.type) {
    case Sdm_Event_AutoSave:
      {
        // check for error.
        if (event->sdmAutoSave.error == NULL) {
          cerr << "*** Error: error is null in autosave event.\n";
          autosaveCallbackError = Sdm_True;
        } else {
          SdmError *error = event->sdmAutoSave.error;
          if (*error) {
            cerr << "*** Error: error returned from SaveMessageStoreState call. err = "
               << (int)*error << "\n" ;
            autosaveCallbackError = Sdm_True;
          }
        }

        // check isStart flag.
  	// We just post a warning in the single-threaded case.  The starts and
  	// stops are not necessarily together in the case when mutiple calls to
  	// execute the event are made.  This is because the start for the event 
  	// calls the service function directly while the end posts a reply.
  	//
        if (event->sdmAutoSave.isStart != expectedAutoSaveStartState) {
#if defined(USE_SOLARIS_THREADS)
    	cerr << "*** Error: isStart not correct in AutoSave event.\n"
               << " \texpect = " << (int) expectedAutoSaveStartState
               << " actual = " << (int)event->sdmAutoSave.isStart
               << "\n";
 
    	autosaveCallbackError = Sdm_True;
#else
    	cerr << "Warning: isStart not correct in AutoSave event.\n"
               << " \texpect = " << (int) expectedAutoSaveStartState
               << " actual = " << (int)event->sdmAutoSave.isStart
               << "\n";
#endif
  	}

        
        if (expectedAutoSaveStartState == Sdm_True) {
          expectedAutoSaveStartState = Sdm_False;
        } else {
          expectedAutoSaveStartState = Sdm_True;
        }

        break;
      }
 
    default:
      cerr << "*** Error: AutoSaveCallback default event received.\n";
      autosaveCallbackError = Sdm_True;
      break;
  }
}



void 
SdmUnsolicitEventTest::DebugLogCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cerr << "DebugLogCallback Called.\n";
  if (userdata != SdmUnsolicitEventTest::kUserData) {
    cerr << "*** Error: userdata given is not correct.\n";
    debugLogCallbackError = Sdm_True;
  }

  if (type != Sdm_SVF_DebugLog) {
    cerr << "*** Error: type argument is not correct.\n";
    debugLogCallbackError = Sdm_True;
  }
  
  debugLogCallbackCalled = Sdm_True;

  switch (event->sdmBase.type) {
    case Sdm_Event_DebugLog:
      {
        // check for error.
        if (event->sdmDebugLog.error == NULL) {
          cerr << "*** Error: error is null in debug log event.\n";
          debugLogCallbackError = Sdm_True;
        } else {
          SdmError *error = event->sdmDebugLog.error;
          if (*error) {
              cerr << "*** Error: error returned from CheckNewMail call. err = "
                 << (int)*error << "\n" ;
              debugLogCallbackError = Sdm_True;
          }
        }

        // check debug message.
        if (event->sdmDebugLog.debugMessage == NULL) {
          cerr << "*** Error: message is null in debug log event.\n";
          debugLogCallbackError = Sdm_True;
        } else {
          SdmString *debugMsg = event->sdmDebugLog.debugMessage;
          cerr << "*** Info: debug message = " << (const char *)*debugMsg << "\n" ;
          delete debugMsg;
        }
        break;
      }
 
    default:
      cerr << "*** Error: DebugLogCallback default event received.\n";
      debugLogCallbackError = Sdm_True;
      break;
  }
}



void 
SdmUnsolicitEventTest::LastInteractiveCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);

  lastInteractiveCallCount++;
 
  cerr << "LastInteractiveCallback Called.\n";
  if (userdata != SdmUnsolicitEventTest::kUserData) {
    cerr << "*** Error: userdata given is not correct.\n";
    lastInteractiveCallbackError = Sdm_True;
  }

  if (type != Sdm_SVF_LastInteractiveEvent) {
    cerr << "*** Error: type argument is not correct.\n";
    lastInteractiveCallbackError = Sdm_True;
  }
  
  lastInteractiveCallbackCalled = Sdm_True;

  switch (event->sdmBase.type) {
    case Sdm_Event_GetLastInteractiveEventTime:
      {
        // check for error.
        if (event->sdmGetLastInteractiveEventTime.error == NULL) {
          cerr << "*** Error: error is null in debug log event.\n";
          lastInteractiveCallbackError = Sdm_True;
        } else {
          SdmError *error = event->sdmGetLastInteractiveEventTime.error;
          if (*error) {
              cerr << "*** Error: error returned from CheckNewMail call. err = "
                 << (int)*error << "\n" ;
              lastInteractiveCallbackError = Sdm_True;
          }
        }

        // set the last interactive time.
        if (event->sdmGetLastInteractiveEventTime.lastInteractiveTime == NULL) {
          cerr << "*** Error: lastInteractiveTime is null in GetLastInteractiveTime event.\n";
          lastInteractiveCallbackError = Sdm_True;        
        } else {
          *(event->sdmGetLastInteractiveEventTime.lastInteractiveTime) = lastInteractiveTime;
        }
        break;
      }
 
    default:
      cerr << "*** Error: LastInteractiveCallback default event received.\n";
      lastInteractiveCallbackError = Sdm_True;
      break;
  }
}



void
SdmUnsolicitEventTest::CheckCheckNewMailEvent(SdmEvent* event, SdmBoolean &errorFlag)
{
  // check for error.
  if (event->sdmCheckNewMail.error == NULL) {
    cerr << "*** Error: error is null in CheckNewMail event.\n";
    errorFlag = Sdm_True;
  } else {
    SdmError *error = event->sdmCheckNewMail.error;
    if (*error) {
        cerr << "*** Error: error returned from CheckNewMail call. err = "
           << (int)*error << "\n" ;
        errorFlag = Sdm_True;
    }
  }

  // check isStart flag.
  // We just post a warning in the single-threaded case.  The starts and
  // stops are not necessarily together in the case when mutiple calls to
  // execute the event are made.  This is because the start for the event 
  // calls the service function directly while the end posts a reply.
  //
  if (event->sdmCheckNewMail.isStart != expectedCheckMailStartState) {
#if defined(USE_SOLARIS_THREADS)
    cerr << "*** Error: isStart not correct in CheckNewMail event.\n"
               << " \texpect = " << (int)expectedCheckMailStartState
               << " actual = " << (int)event->sdmCheckNewMail.isStart
               << "\n";

    errorFlag = Sdm_True;
#else
    cerr << "Warning: isStart not correct in CheckNewMail event.\n"
               << " \texpect = " << (int)expectedCheckMailStartState
               << " actual = " << (int)event->sdmCheckNewMail.isStart
               << "\n";
#endif
  }

  if (expectedCheckMailStartState == Sdm_True) {
    expectedCheckMailStartState = Sdm_False;
  } else {
    expectedCheckMailStartState = Sdm_True;
  }

  // check numMessages.
  if (event->sdmCheckNewMail.isStart && event->sdmCheckNewMail.numMessages > 0) {
    cerr << "*** Error: numMessages set for checkNewMail start.\n";
    errorFlag = Sdm_True;
  } else if (!event->sdmCheckNewMail.isStart && event->sdmCheckNewMail.numMessages < 0) {
    cerr << "*** Error: numMessages not set for checkNewMail end.\n";
    errorFlag = Sdm_True;
  }
}


SdmErrorCode
SdmUnsolicitEventTest::PollForReply(SdmReplyQueue *frontEndQueue, 
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
    if ((rc = poll(&pollFileDesc, 1, 10000)) > 0) {
      if (SdmHandleSessionEvent(frontEndQueue, &readFileDesc, NULL) < 0) {
        sprintf(buffer, "*** SdmHandleSessionEvent failed after processing %d events.\n", i);
        cerr << buffer;
        err = Sdm_EC_Fail;
        break;
      }

    } else if (rc == 0) {
        sprintf(buffer, "*** Poll timed out after processing %d events.\n", i);
        cerr << buffer;
        sprintf(buffer, "*** front queue element count = %d\n", frontEndQueue->ElementCount());
        cerr << buffer;
        break;
    } else {
      err = Sdm_EC_Fail;
      cerr << "*** Error: Poll failed.\n";
      return err;
    }
  }
  
  if (i != expectedCalls && expectedCalls >= 0) {
    sprintf(buffer, "*** Error: Did not get expected number of replies.  expected=%d, received = %d.\n",
      expectedCalls, i);
    cerr << buffer;
    err = Sdm_EC_Fail;
    return err;
  }
  return err;
}



void
SdmUnsolicitEventTest::HandleSleepTimeOut(XtPointer calldata, XtIntervalId *id)
{
  SdmUnsolicitEventTest *test = (SdmUnsolicitEventTest*) calldata;

  // set flag indicating that the sleep on the main thread is complete.
  test->SetFinishSleep(Sdm_True);

  // send an event to wake up the main thread.
  XEvent event;
  event.type = KeyPress;
  XSendEvent(XtDisplay(test->_toplevel), XtWindow(test->_toplevel), True, KeyPressMask, &event);
}

