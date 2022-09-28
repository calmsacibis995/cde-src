/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Connection Class.

#pragma ident "@(#)Connection.cc	1.77 97/02/06 SMI"

// Include Files.
#include <assert.h>
#include <iostream.h>
#include <locale.h>
#include <lcl/lcl.h>
#include <SDtMail/Connection.hh>
#include <SDtMail/SimpleTuples.hh>
#include <Manager/ISess.hh>
#include <Manager/OSess.hh>
#include <Utils/TnfProbe.hh>
#include <PortObjs/DataPort.hh>
#include <DataStructs/MutexEntry.hh>
#include <DataStructs/RecursiveMutexEntry.hh>
#include <Utils/LockUtility.hh>
#include <sys/time.h>
#include <sys/resource.h>

static const int Sdm_DefConcLevel = 6;
SdmConnection* SdmConnection::_GlobalConnection = NULL;

// note: need to use recursive mutex because there is
// re-entry code when GetConnection calls the SdmConnection
// constructor.  we also make the mutex data to be global 
// to this file instead of a data member of SdmConnection so
// SdmRecursiveMutexEntry is not exposed in a public header.
static SdmRecursiveMutexEntry::Data gGlobalConnectionMutexData;



SdmConnection::SdmConnection(const char* const appName, void* appContext)
  :SdmPrim(Sdm_CL_Connect),
  _SessionList(SdmD_SessionSet_Size),
  _ServiceFunctionList(SdmD_ServiceFunctionSet_Size),
   locale(NULL), lcld((LCLd)0)
{
  TNF_PROBE_1 (SdmConnection_constructor_start,
      "api SdmConnection",
      "sunw%start SdmConnection_constructor_start",
      tnf_string, appName, appName);

  // we are going to abort if a connection was already created.
  // there should only be one connection per process.
  SdmRecursiveMutexEntry entry(gGlobalConnectionMutexData);
  assert (_GlobalConnection == NULL);	
  // set the global connection to this object.
  _GlobalConnection = this;

  _InitialThread = thr_self();
    
  int rc =   mutex_init(&_ServiceListLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  
  for (int i = 0; i < SdmD_SessionSet_Size; i++)
    _SessionList[i] = NULL;

  for (i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
    _ServiceFunctionList[i] = NULL;

  // Setting BackEnd Global Thread Concurrency Level
  thr_setconcurrency(Sdm_DefConcLevel);

  // Cause various system resource limits to be raised 

  struct rlimit rls;
  rls.rlim_cur = 0;
  rls.rlim_max = 0;

  if ((getrlimit(RLIMIT_NOFILE, &rls) == 0) && (rls.rlim_max != 0)) {
    rls.rlim_cur = rls.rlim_max;	// raise number of possible open files to maximum
    (void) setrlimit(RLIMIT_NOFILE, &rls);
  }

  // Save the input arguments for the startup function

  _appName = appName;
  _appContext = appContext;

  TNF_PROBE_0 (SdmConnection_constructor_end,
      "api SdmConnection",
      "sunw%end SdmConnection_constructor_end");
}

SdmConnection::SdmConnection(const SdmConnection& copy) 
  :SdmPrim(Sdm_CL_Connect)
{
  cout << "*** Error: SdmConnection copy constructor called\n";
  assert(Sdm_False);
}

SdmConnection& 
SdmConnection::operator=(const SdmConnection &rhs)
{
  cout << "*** Error: SdmConnection assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmConnection::~SdmConnection()
{
  TNF_PROBE_0 (SdmConnection_destructor_start,
      "api SdmConnection",
      "sunw%start SdmConnection_destructor_start");

  SdmError err;
  if (_IsStarted) {
    ShutDown(err);
  }

  for (int i = 0; i < SdmD_SessionSet_Size; i++)
    delete _SessionList[i];

  mutex_destroy(&_ServiceListLock);

  // reset _GlobalConnnection last.  The destruction of
  // of objects as a result of the ShutDown might need
  // to access the global connection.
  SdmRecursiveMutexEntry entry(gGlobalConnectionMutexData);
  _GlobalConnection = NULL;

  TNF_PROBE_0 (SdmConnection_destructor_end,
      "api SdmConnection",
      "sunw%end SdmConnection_destructor_end");
}

SdmErrorCode
SdmConnection::StartUp(SdmError& err)
{
  TNF_PROBE_0 (SdmConnection_StartUp_start,
	       "api SdmConnection",
	       "sunw%start SdmConnection_StartUp_start");

  err = Sdm_EC_Success;
  
  if (_IsStarted) { 
    err = Sdm_EC_ConnectionAlreadyStarted; 
    
  } else {
    SdmMutexEntry entry(_ServiceListLock);  
    // initialize list of service functions.    
    _ServiceFunctionList.SetVectorSize(SdmD_ServiceFunctionSet_Size); 
    for (int i = 0; i < SdmD_ServiceFunctionSet_Size; i++)
      _ServiceFunctionList[i] = NULL;

    // I18N - open the lcl database
    setlocale (LC_ALL, "");

    if ((locale = setlocale(LC_CTYPE, NULL)) == NULL) {
      err = Sdm_EC_LocaleNotSetCorrectly;
    } else {
      lcld = lcl_open(locale);
      if (!lcld) {
        err = Sdm_EC_LclOpenFailed;
      }

      // Startup the mailbox locking apparatus connection
      if (!SdmLockUtility::ConnectionStartup(err, _appName, _appContext)) {
        // set _IsStarted flag only if everything gets set up correctly.
        _IsStarted = Sdm_True;
      }
    }
  }
  

  TNF_PROBE_0 (SdmConnection_StartUp_end,
      "api SdmConnection",
      "sunw%end SdmConnection_StartUp_end");

  return err;
}

SdmErrorCode
SdmConnection::ShutDown(SdmError& err)
{
  TNF_PROBE_0 (SdmConnection_ShutDown_start,
      "api SdmConnection",
      "sunw%start SdmConnection_ShutDown_start");

  err = Sdm_EC_Success;

  SdmError localErr;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  // shut down the sessions.  if an error occurs, save it 
  // but continue to shut down the other sessions.
  //
  SdmSession *ses;
  for (int i = 0; i < SdmD_SessionSet_Size; i++) {
     ses = _SessionList[i];
     if (ses && ses->IsStarted()) { 
       if (ses->ShutDown(localErr)) {
         err = localErr;
         localErr = Sdm_EC_Success;
       }
     }
  }

  {
    SdmMutexEntry entry(_ServiceListLock);  
    _ServiceFunctionList.ClearAndDestroyAllElements();
  }

  // Shutdown the mailbox locking apparatus connection

  (void) SdmLockUtility::ConnectionShutdown(localErr);

  // Unregister any group privilege callback that may have been registered earlier

  (void) UnregisterGroupPrivilegeActionCallback(localErr = Sdm_EC_Success);

  _IsStarted = Sdm_False;

#ifdef HACK
#endif /* HACK */
  lcl_close (lcld);


  TNF_PROBE_0 (SdmConnection_ShutDown_end,
      "api SdmConnection",
      "sunw%end SdmConnection_ShutDown_end");

  return err;
}

SdmErrorCode SdmConnection::RegisterGroupPrivilegeActionCallback(SdmError& err,
								 SdmGroupPrivilegeActionCallback gpaf,
								 void* clientData)
{
  return(SdmDataPort::RegisterGroupPrivilegeActionCallback(err = Sdm_EC_Success,
						    gpaf, clientData));
}

SdmErrorCode SdmConnection::UnregisterGroupPrivilegeActionCallback(SdmError& err)
{
  return(SdmDataPort::RegisterGroupPrivilegeActionCallback(err = Sdm_EC_Success,
						    (SdmDpGroupPrivilegeActionCallback)0));
}

SdmErrorCode
SdmConnection::SdmSessionFactory(SdmError& err, SdmSession *&r_session, 
                        SdmSessionType st)
{
  TNF_PROBE_0 (SdmConnection_SdmSessionFactory_start,
      "api SdmConnection",
      "sunw%start SdmConnection_SdmSessionFactory_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  switch(st) {
    case Sdm_ST_InSession :
      r_session = GetIncomingSession(err);
      break;
    case Sdm_ST_OutSession :
      r_session = GetOutgoingSession(err);
      break;
    default:
      r_session = NULL;
      err = Sdm_EC_Fail;
  }
  
  TNF_PROBE_0 (SdmConnection_SdmSessionFactory_end,
      "api SdmConnection",
      "sunw%end SdmConnection_SdmSessionFactory_end");

  return err;
}


SdmBoolean
SdmConnection::IsValidSession(const SdmSession& session) const
{
  if (&session == _SessionList[Sdm_ST_InSession] ||
      &session == _SessionList[Sdm_ST_OutSession]) 
    return Sdm_True;
  else 
    return Sdm_False;
}

SdmErrorCode
SdmConnection::ResetPollingTasks(SdmError &err)
{
  err = Sdm_EC_Success;
  
  for (int i = 0; i < SdmD_SessionSet_Size; i++) {
    if (_SessionList[i] != NULL) {
      if (_SessionList[i]->ResetPollingTasks(err)) {
        return err;
      }
    }
  }
  
  return err;
}
  


SdmIncomingSession*
SdmConnection::GetIncomingSession(SdmError& err)
{
  TNF_PROBE_0 (SdmConnection_SdmSessionFactory_start,
      "api SdmConnection",
      "sunw%start SdmConnection_SdmSessionFactory_start");

  SdmIncomingSession *return_session;
  err = Sdm_EC_Success;


  if (_SessionList[Sdm_ST_InSession  ] != NULL)
  {
      return_session = (SdmIncomingSession*)_SessionList[Sdm_ST_InSession];
  }
  else
  {
    SdmIncomingSession* inSession ;
    
    inSession = new SdmIncomingSession(err, *this);
    if (inSession == NULL) {
      err = Sdm_EC_Fail;
      return_session = NULL;
    }
    else
    {
      _SessionList[Sdm_ST_InSession] = inSession ;
      return_session = inSession ;
    }
  }

  TNF_PROBE_0 (SdmConnection_SdmSessionFactory_end,
      "api SdmConnection",
      "sunw%end SdmConnection_SdmSessionFactory_end");

  return return_session;
}


SdmOutgoingSession*
SdmConnection::GetOutgoingSession(SdmError& err)
{
  TNF_PROBE_0 (SdmConnection_GetOutgoingSession_start,
      "api SdmConnection",
      "sunw%start SdmConnection_GetOutgoingSession_start");

  SdmOutgoingSession *return_session;
  err = Sdm_EC_Success;

  if (_SessionList[Sdm_ST_OutSession  ] != NULL)
  {
      return_session 
        = (SdmOutgoingSession*)_SessionList[Sdm_ST_OutSession ];
  }
  else
  {
    SdmOutgoingSession* outSession;

    outSession = new SdmOutgoingSession(err, *this);
    if (outSession == NULL) 
    {
      err = Sdm_EC_Fail;
      return_session = NULL;
    }
    else
    {
      _SessionList[Sdm_ST_OutSession] = outSession ;
      return_session = outSession ;
    }
  }

  TNF_PROBE_0 (SdmConnection_GetOutgoingSession_end,
      "api SdmConnection",
      "sunw%end SdmConnection_GetOutgoingSession_end");

  return return_session ;
}


SdmErrorCode
SdmConnection::SdmMailRcFactory(SdmError& err, SdmMailRc *&r_mailrc)
{
  TNF_PROBE_0 (SdmConnection_SdmMailRcFactory_start,
      "api SdmConnection",
      "sunw%start SdmConnection_SdmMailRcFactory_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  r_mailrc =  SdmMailRc::GetMailRc();
  if (r_mailrc == NULL) {
    err = Sdm_EC_Fail;
  }

  TNF_PROBE_0 (SdmConnection_SdmMailRcFactory_end,
      "api SdmConnection",
      "sunw%end SdmConnection_SdmMailRcFactory_end");

  return err;
}

SdmErrorCode
SdmConnection::RegisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmConnection_RegisterServiceFunction_start,
      "api SdmConnection",
      "sunw%start SdmConnection_RegisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  SdmServiceFunctionType sft = sfv.GetType();
  {
    SdmMutexEntry entry(_ServiceListLock);
    if (_ServiceFunctionList[sft] != NULL) {
      err = Sdm_EC_ServiceFunctionAlreadyRegistered;
    } else {
       SdmServiceFunction *copy = new SdmServiceFunction(sfv);
      _ServiceFunctionList[sft] = copy;
    }
  }
  
  TNF_PROBE_0 (SdmConnection_RegisterServiceFunction_end,
      "api SdmConnection",    
      "sunw%end SdmConnection_RegisterServiceFunction_end");

  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode
SdmConnection::UnregisterServiceFunction(SdmError& err, const SdmServiceFunction& sfv)
{
  TNF_PROBE_0 (SdmConnection_UnregisterServiceFunction_start,
      "api SdmConnection",
      "sunw%start SdmConnection_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  
  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  SdmServiceFunctionType sft = sfv.GetType();
  
  {
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }

  TNF_PROBE_0 (SdmConnection_UnregisterServiceFunction_end,
      "api SdmConnection",
      "sunw%end SdmConnection_UnregisterServiceFunction_end");

  return err;
}

#endif

SdmErrorCode
SdmConnection::UnregisterServiceFunction(SdmError& err, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmConnection_UnregisterServiceFunction_start,
      "api SdmConnection",
      "sunw%start SdmConnection_UnregisterServiceFunction_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  {
    SdmMutexEntry entry(_ServiceListLock);  
    if (_ServiceFunctionList[sft] == NULL) {
      err = Sdm_EC_ServiceFunctionNotRegistered;
    } else {
      delete (_ServiceFunctionList[sft]);
      _ServiceFunctionList[sft] = NULL;
    }
  }
  
  TNF_PROBE_0 (SdmConnection_UnregisterServiceFunction_end,
      "api SdmConnection",
      "sunw%end SdmConnection_UnregisterServiceFunction_end");

   return err;
}

SdmErrorCode
SdmConnection::GetServiceFunction(SdmError& err, SdmServiceFunction *&r_serviceFunc, SdmServiceFunctionType sft)
{
  TNF_PROBE_0 (SdmConnection_GetServiceFunction_start,
      "api SdmConnection",
      "sunw%start SdmConnection_GetServiceFunction_start");

  err = Sdm_EC_Success;


  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)

  {
    SdmMutexEntry entry(_ServiceListLock);  
    r_serviceFunc = _ServiceFunctionList[sft];
  }
  
  if (r_serviceFunc == NULL) {
    err = Sdm_EC_ServiceFunctionNotRegistered;
  }

  TNF_PROBE_0 (SdmConnection_GetServiceFunction_end,
      "api SdmConnection",
      "sunw%end SdmConnection_GetServiceFunction_end");

  return err;
}

SdmErrorCode 
SdmConnection::CancelPendingOperations(SdmError &err)
{
  TNF_PROBE_0 (SdmConnection_CancelPendingOperations_start,
      "api SdmConnection",
      "sunw%start SdmConnection_CancelPendingOperations_start");

  err = Sdm_EC_Success;

  __SdmPRIM_STARTUPCHECK2(err, Sdm_EC_ConnectionNotStarted)
  
  for (int i = 0; i < SdmD_SessionSet_Size; i++) {
    if (_SessionList[i] != NULL) {
      if (_SessionList[i]->CancelPendingOperations(err))
          return err;
    }
  }

  TNF_PROBE_0 (SdmConnection_CancelPendingOperations_end,
      "api SdmConnection",
      "sunw%end SdmConnection_CancelPendingOperations_end");
      
  return err;
}



SdmErrorCode 
SdmConnection::HandleSessionDestruction(SdmError& err, SdmSession* session)
{
  TNF_PROBE_0 (SdmConnection_HandleSessionDestruction_start,
      "api SdmConnection",
      "sunw%start SdmConnection_HandleSessionDestruction_start");

  err = Sdm_EC_Success;
  assert (session != NULL);
  SdmSessionType type = session->GetType();
  if (_SessionList[type] != NULL) {
    _SessionList[type] = NULL;
  }

  TNF_PROBE_0 (SdmConnection_HandleSessionDestruction_end,
      "api SdmConnection",
      "sunw%end SdmConnection_HandleSessionDestruction_end");

  return err;
}


SdmConnection*
SdmConnection::GetConnection()
{
  SdmRecursiveMutexEntry entry(gGlobalConnectionMutexData);  
  return _GlobalConnection;
}


