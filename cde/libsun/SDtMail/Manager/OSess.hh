/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the OSession Class.

#ifndef _SDM_OSESS_H
#define _SDM_OSESS_H

#pragma ident "@(#)OSess.hh	1.39 96/05/30 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Session.hh>

class SdmOutgoingStore;
 
class SdmOutgoingSession : public SdmSession {

public:
  SdmOutgoingSession(SdmError& err, SdmConnection& connnection);
  virtual ~SdmOutgoingSession();

  // Outgoing store creation.
  virtual SdmErrorCode SdmMessageStoreFactory(SdmError &err, SdmMessageStore *&r_mstore);

  // Stops all pending operations currently running in this session.
  virtual SdmErrorCode CancelPendingOperations(SdmError &err);

  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError &err);
  
protected:
/*
  static void SR_ProcessSubmitRequest(SdmPrim* objThis, void* callData);  
  void ProcessSubmitRequest(void* callData);
  
  //Posting requests.  
  SdmErrorCode PostSubmitRequest(SdmError &err, SdmPrim* caller, SdmRequest* request); 
*/
private:  
  // SdmTickleDispatcher* _SubmitRequestDisp;
  // SdmRequestQueue *_SubmitRequestCollector;

  // SdmOutgoingMessage is a frield so that it can post the submit requests.
  friend class SdmOutgoingMessage;
	
};

#endif

