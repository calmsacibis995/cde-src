/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the test for the expunging of deleted messages calls.
 

#include "Main.hh"

#include <SDtMail/Connection.hh>
#include <Manager/ISess.hh>
#include <Manager/OSess.hh>
#include <SDtMail/MessageStore.hh>
#include <DataStore/OutgoingStore.hh>
#include <SDtMail/Token.hh>



class SdmConnectionTest
{
public:
     SdmConnectionTest();
     virtual ~SdmConnectionTest();
     
     virtual SdmErrorCode Run();


private:
     static const char* kUserData;

     SdmErrorCode TestStartup();
     SdmErrorCode TestShutdown(SdmSession*, SdmSession*,
                        SdmMessageStore*, SdmMessageStore*);
     SdmErrorCode TestSessionFactory(SdmSession *&, SdmSession *&);
     SdmErrorCode TestMailRcFactory(SdmMailRc *&);
     SdmErrorCode TestServiceFunctionCalls();
     SdmErrorCode CreateAndStartUpStores(SdmSession*, SdmSession*,
                        SdmMessageStore*&, SdmMessageStore*&);
                        
  static void ConnectionTestCallback(SdmError&, void*, SdmServiceFunctionType type, SdmEvent*);

private:

  SdmConnection* mcon;
};

const char* SdmConnectionTest::kUserData = "connection test data";


SdmErrorCode RunConnectionTest()
{
  cout << endl << "************ RunConnectionTest Invoked. *************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmConnectionTest test;
  return test.Run();
}
  

SdmConnectionTest::SdmConnectionTest()
{
}

SdmConnectionTest::~SdmConnectionTest()
{
}

SdmErrorCode 
SdmConnectionTest::Run()
{
  SdmError err;
  SdmMessageStore *mbox, *dbox;
  SdmSession *ises, *oses;
  SdmMailRc *mailrc;
  
  cout << endl << "************ SdmConnectionTest::Run Invoked. *************" << endl << endl;
  cout << endl;


  mcon  = new SdmConnection("TestGUI");
  
  SdmErrorCode errorCode;
  
  // create and start up connection.
  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    return err;
  }

  if ((errorCode = TestStartup()) != Sdm_EC_Success)
    return errorCode;
   
  if ((errorCode = TestSessionFactory(ises, oses)) != Sdm_EC_Success)
    return errorCode;
   
  if ((errorCode = TestMailRcFactory(mailrc)) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = TestServiceFunctionCalls()) != Sdm_EC_Success)
    return errorCode;

  if ((errorCode = CreateAndStartUpStores(ises, oses, mbox, dbox)) != Sdm_EC_Success)
    return errorCode;

  // 
  // this call doesn't really do anything because there are no 
  // operations pending.  More extensive testing of the cancel
  // function is in the MessageStoreTest.  This call is strictly 
  // for exercising the function.
  if (mcon->CancelPendingOperations(err)) {
    cout << "*** Error: Cancel pending operations Failed\n" ;    
    return err;
  }
  

  // run the shutdown test last because the other tests will fail
  // if the connection is shut down.    
  if ((errorCode = TestShutdown(ises, oses, mbox, dbox)) != Sdm_EC_Success)
    return errorCode;

  delete mcon;
        
  return Sdm_EC_Success;
}


     
SdmErrorCode 
SdmConnectionTest::TestStartup()
{
  SdmError err;
  
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }
  
  // try to start up connection again.  this should result in error.
  if (mcon->StartUp(err)) {
    if (err != Sdm_EC_ConnectionAlreadyStarted) {
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
SdmConnectionTest::TestSessionFactory( 
  SdmSession *& ises, SdmSession *&oses)
{
  SdmSession* ses;
  SdmError err;
  
  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, ses, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed\n";
    return err;
  }
  if (ses->StartUp(err)) {
    cout << "*** Error: Startup incoming session failed\n";
    return err;
  }
  ises = (SdmIncomingSession*) ses;
  
  // test to make sure only one incoming session is ever returned.
  if (mcon->SdmSessionFactory(err, ses, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed (second time)\n";
    return err;
  }
  if ((SdmSession*)ises != ses) {
    cout << "*** Error: multiple incoming sessions created\n";
    err = Sdm_EC_Fail;
    return err;
  }  

  // create and startup outgoing session.
  if (mcon->SdmSessionFactory(err, ses, Sdm_ST_OutSession)) {
     cout << "*** Error: create outgoing session failed\n";
     return err;
  }
  if (ses->StartUp(err)) {
     cout << "*** Error: Startup outgoing session failed\n";
     return err;
  }
  oses = (SdmOutgoingSession*) ses;

  // test to make sure only one outgoing session is ever returned.
  if (mcon->SdmSessionFactory(err, ses, Sdm_ST_OutSession)) {
    cout << "*** Error: create outgoing session failed (second time)\n";
    return err;
  }
  if ((SdmSession*)oses != ses) {
    cout << "*** Error: multiple outgoing sessions created\n";
    err = Sdm_EC_Fail;
    return err;
  }  
  return err;
}

     
SdmErrorCode 
SdmConnectionTest::TestMailRcFactory(SdmMailRc *&mailrc)
{
  SdmMailRc *t_mailrc;
  SdmError err;
  
  // create mailrc object.
  if (mcon->SdmMailRcFactory(err, mailrc)) {
     cout << "*** Error: create mailrc failed\n";
     return err;
  }
  
  // test that only one mail rc object is ever returned.
  if (mcon->SdmMailRcFactory(err, t_mailrc)) {
     cout << "*** Error: create mailrc failed\n";
     return err;
  }  
  if (mailrc != t_mailrc) {
    cout << "*** Error: multiple mailrc objects created\n";
    err = Sdm_EC_Fail;
    return err;
  }
  return err;  
}  


    
void 
SdmConnectionTest::ConnectionTestCallback(SdmError& err, void* userdata, SdmServiceFunctionType type,
  SdmEvent* event)
{
  assert (event != NULL);
 
  cout << "ConnectionTestCallback Called.\n";
  if (userdata != SdmConnectionTest::kUserData) {
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
         
        sprintf(buffer, "ConnectionTestCallback called with args=%d,%d, buffer=%s\n",
          stOffS, length, (const char *)str_contents);
        cout << buffer;
        break;
      }
 
    default:
      cout << "Error: ConnectionTestCallback default event received.\n";
      break;
  }
}
 

    
SdmErrorCode 
SdmConnectionTest::TestServiceFunctionCalls()
{
  SdmError err;
  SdmServiceFunction *svfP;
  
  SdmServiceFunction svf(&ConnectionTestCallback, 
          (void*)SdmConnectionTest::kUserData, 
          Sdm_SVF_Any);

  if (mcon->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  if (mcon->GetServiceFunction(err, svfP, Sdm_SVF_Any)) {
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
  if (mcon->UnregisterServiceFunction(err, Sdm_SVF_Any)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  if (mcon->GetServiceFunction(err, svfP, Sdm_SVF_Any)) {
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
  if (mcon->UnregisterServiceFunction(err, Sdm_SVF_Any)) {
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


  if (mcon->RegisterServiceFunction(err, svf)) {
    cout << "*** Error: register service function failed\n";
    return err;
  }
  // test error code returned when trying to register 
  // the service function again.
  if (mcon->RegisterServiceFunction(err, svf)) {
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
  if (mcon->GetServiceFunction(err, svfP, Sdm_SVF_Any)) {
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
  if (mcon->UnregisterServiceFunction(err, svf)) {
    cout << "*** Error: unregister service function failed\n";
    return err;
  }
  // test error code returned when trying to unregister the service
  // function again.
  if (mcon->UnregisterServiceFunction(err, svf)) {
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
  
  if (mcon->GetServiceFunction(err, svfP, Sdm_SVF_Any)) {
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
SdmConnectionTest::CreateAndStartUpStores(SdmSession *ises, SdmSession *oses,
  SdmMessageStore *&mbox, SdmMessageStore *&dbox)
{
  SdmError err;
  
  // create and start up message store.
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (mbox->StartUp(err)) {
      cout << "*** Error: Startup message store failed\n";
      return err;
  }

  // create and start up Outgoing store.
  if (oses->SdmMessageStoreFactory(err, dbox)) {
     cout << "*** Error: Startup Outgoing store failed\n";
     return err;
  }
  if (dbox->StartUp(err)) {
      cout << "*** Error: Startup Outgoing store failed\n";
      return err;
  }
  return err;
}

     
SdmErrorCode 
SdmConnectionTest::TestShutdown(SdmSession *ises, SdmSession* oses, SdmMessageStore *mbox, 
  SdmMessageStore *dbox)
{
  SdmError err;
  
  if (mcon->ShutDown(err)) {
    cout << "*** Error: Connection shutdown failed.\n";
    return err;
  }
  
  if (mcon->IsStarted() != Sdm_False) {
    cout << "*** Error: session IsStarted flag still set after shutdown.\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (ises->IsStarted() != Sdm_False) {
    cout << "*** Error: incoming session shutdown failed.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  if (oses->IsStarted() != Sdm_False) {
    cout << "*** Error: outgoing session shutdown failed.\n";
    err = Sdm_EC_Fail;
    return err;
  }
 
  // try to shutdown connection again.  this should result in error.
  if (mcon->ShutDown(err)) {
    if (err != Sdm_EC_ConnectionNotStarted) {
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
