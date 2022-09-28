/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the ISession Class.

#pragma ident "@(#)ISess.cc	1.50 96/07/09 SMI"

// Include Files.
#include <SDtMail/MessageStore.hh>
#include <Manager/ISess.hh>
#include <SDtMail/Token.hh>
#include <CtrlObjs/TickDisp.hh>
#include <CtrlObjs/RepSWER.hh>


SdmIncomingSession::SdmIncomingSession(SdmError& err, SdmConnection& conn)
  :SdmSession(err, Sdm_CL_InSession, conn, Sdm_ST_InSession)
{
  __SdmPRIM_REGISTERENTITY(this, NULL);
  __SdmPRIM_REGISTERCOLLECTOR(this, GetRequestQueue(), GetReplyQueue());
}

SdmIncomingSession::~SdmIncomingSession()
{
  SdmError err;
  
  if (_IsStarted) {
    ShutDown(err);
  }

  __SdmPRIM_UNREGISTERCOLLECTOR;
  __SdmPRIM_UNREGISTERENTITY;
}

SdmErrorCode
SdmIncomingSession::SdmMessageStoreFactory(SdmError& err, SdmMessageStore *&r_store)
{
  err = Sdm_EC_Success;
   __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_SessionNotStarted)
 
  SdmMessageStore *t_store = new SdmMessageStore(err, *this);
  if (t_store == NULL) {
    err = Sdm_EC_Fail;
    return err;
  }

  if (AddMessageStore(err, t_store)) {
    delete t_store;
    return err;
  }
  
  r_store = t_store;
  return err;
}



