/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Connection Class.

#ifndef _SDM_CONNECTION_H
#define _SDM_CONNECTION_H

#pragma ident "@(#)Connection.hh	1.49 97/02/06 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/SimpleTuples.hh>
#include <synch.h>
#include <lcl/lcl.h>
#include <thread.h>


class SdmIncomingSession;
class SdmOutgoingSession;
class SdmIntStr;


class SdmConnection : public SdmPrim {

public:
  static SdmConnection* GetConnection();

  SdmConnection(const char* const appName, void* appContext = 0);
  virtual ~SdmConnection();

  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

  // Session Creation
  SdmErrorCode SdmSessionFactory(SdmError& err, SdmSession *&r_session, 
		       SdmSessionType st);

  // MailRc Access
  SdmErrorCode SdmMailRcFactory(SdmError& err, SdmMailRc *&r_mailrc);

  // Service Function Registration
  SdmErrorCode RegisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
  SdmErrorCode UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft);
  SdmErrorCode GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, 
                 SdmServiceFunctionType sft);


#ifdef INCLUDE_UNUSED_API
  SdmErrorCode UnregisterServiceFunction(SdmError& err, const SdmServiceFunction &svf);
#endif

  // Group Privilege Control Registry
  // SdmGroupPrivilegeActionCallback is a pointer to a function that is passed a flag 
  // indicating whether special group privileges should be enabled or disabled. A single
  // such function can then be registered to provide group privilege enable/disable functions

  typedef void (*SdmGroupPrivilegeActionCallback)(void* clientData, SdmBoolean enableFlag);

  SdmErrorCode RegisterGroupPrivilegeActionCallback(SdmError& err,
						    SdmGroupPrivilegeActionCallback gpaf,
						    void* clientData);
  SdmErrorCode UnregisterGroupPrivilegeActionCallback(SdmError& err);

  // Stops all pending operations currently running in sessions for this connection.
          SdmErrorCode CancelPendingOperations(SdmError &err);
  LCLd GetLocaleDescriptor() { return lcld; }

  // checks if given session is a valid session for this connection.
  SdmBoolean IsValidSession(const SdmSession& session) const;

  // resets the polling tasks for check new mail and autosave.
  SdmErrorCode ResetPollingTasks(SdmError &err);

  thread_t	GetInitialThread() { return _InitialThread; }
    
protected:
  virtual SdmErrorCode HandleSessionDestruction(SdmError& err, SdmSession* session);

private:
  // Disallow creating copies of SdmConnection.
  SdmConnection(const SdmConnection& copy);
  SdmConnection& operator=(const SdmConnection &rhs);
  
  SdmIncomingSession* 	GetIncomingSession(SdmError& err);
  SdmOutgoingSession* 	GetOutgoingSession(SdmError& err);

  char *locale;
  LCLd lcld;

  SdmVector<SdmSession *>           _SessionList;
  SdmPtrVector<SdmServiceFunction*> _ServiceFunctionList;
  mutex_t                           _ServiceListLock;   
  static SdmConnection*             _GlobalConnection;
  void*				    _appContext;
  SdmString			    _appName;
  thread_t                          _InitialThread;
    
  friend class SdmSession;
  
};

#endif





