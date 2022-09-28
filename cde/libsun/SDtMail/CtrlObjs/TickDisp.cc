/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Dispatcher Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)TickDisp.cc	1.40 97/03/14 SMI"
#endif

// Include Files.
#include <CtrlObjs/TickDisp.hh>
#include <DataStructs/MutexEntry.hh>

SdmTickleDispatcher::SdmTickleDispatcher(SdmError& err, sema_t* semaVar, 
	int threadCount)
  :SdmDispatcher(Sdm_CL_TickleDispatcher, err, SdmTickDisp_DefConcLevel),
    _TickSemaVar(semaVar), _ThreadCount(threadCount), _DoShutdown(Sdm_False)
{
#if defined(USE_SOLARIS_THREADS)
  int rc = sema_init(&_OneThreadShutdown, 0, USYNC_THREAD, 0);
  assert (rc == 0);
  rc =  mutex_init(&_DataLock, USYNC_THREAD, NULL);
  assert (rc == 0);
#endif

  err = Sdm_EC_Success;
}

SdmTickleDispatcher::~SdmTickleDispatcher()
{
  SdmError err;
  if (_IsStarted) {
    ShutDown(err);
  }
  
#if defined(USE_SOLARIS_THREADS)
  mutex_destroy(&_DataLock);
  sema_destroy(&_OneThreadShutdown);
#endif
}

SdmErrorCode
SdmTickleDispatcher::StartUp(SdmError& err)
{
  if (_IsStarted) {
    err = Sdm_EC_Fail;
  } else {
#if defined(USE_SOLARIS_THREADS)
    StartThrdServices();
#endif
    _IsStarted = Sdm_True;
    err = Sdm_EC_Success;
  }
  return err;
}

SdmErrorCode
SdmTickleDispatcher::ShutDown(SdmError& err)
{
  if (!_IsStarted) {
    err = Sdm_EC_Fail;
  } else {
#if defined(USE_SOLARIS_THREADS)
    StopThrdServices();
#endif
    _IsStarted = Sdm_False;
    err = Sdm_EC_Success;
  }
  return err;
}

void
SdmTickleDispatcher::StartThrdServices()
{
#if defined(USE_SOLARIS_THREADS)
  int thrstat;
  thread_t tid;

  {
     SdmMutexEntry entry(_DataLock);
     _DoShutdown = Sdm_False;
  }

  for (int i = 0; i < _ThreadCount; i++) {
    thrstat = thr_create(0, 0, SR_WaitAndService, (void *) this, THR_DETACHED, &tid);
    assert (thrstat == 0);
    _ThreadList.AddElementToList(tid);
  }

#endif
}

void
SdmTickleDispatcher::StopThrdServices()
{
#if defined(USE_SOLARIS_THREADS)
  // note: because we are using solaris threads, there is no way
  // to kill the thread without killing the whole process.  calling
  // thr_kill ends up killing the whole process.  therefore, to 
  // shutdown the threads, we set the _DoShutdown flag and tickle the 
  // semaphore and to wake up all the threads.  when the threads wake
  // up, they will tickle another semaphore to let us know that they
  // have exited and then do the actual exit.  we'll wait until all
  // the threads exit before returning.

  {
     SdmMutexEntry entry(_DataLock);
     _DoShutdown = Sdm_True;
  }
  
  // tickle the semaphore and wake up each thread.  when
  // each thread wakes up, it will tickle _OneThreadShutdown and
  // exit.  once we get responses from all the threads, we 
  // can return.
  for (int i = 0; i < _ThreadCount; i++) { 
    sema_post(_TickSemaVar);
  }
  // wait for all the threads to finish before returning.
  for (i = 0; i < _ThreadCount; i++) { 
    sema_wait(&_OneThreadShutdown);
  }
  _ThreadList.ClearAllElements();
  
  {
     SdmMutexEntry entry(_DataLock);
     _DoShutdown = Sdm_False;
  }
#endif
}

void*
SdmTickleDispatcher::SR_WaitAndService(void* objThis)
{
#if defined(USE_SOLARIS_THREADS)
  sigset_t ignoreSignals;
  GetSignalsIgnoredByThreads(ignoreSignals);
  thr_sigsetmask(SIG_BLOCK, &ignoreSignals, NULL);
#endif
  return ((SdmTickleDispatcher *) objThis)->WaitAndService();
}

void*
SdmTickleDispatcher::WaitAndService()
{
#if defined(USE_SOLARIS_THREADS)
  while (1) {
    sema_wait(_TickSemaVar);
    
    mutex_lock(&_DataLock);
    if (_DoShutdown) {
      sema_post(&_OneThreadShutdown);
      mutex_unlock(&_DataLock);
      thr_exit(0);
    } else {
      mutex_unlock(&_DataLock);
    }
    

    SdmTodo* todo;

    for (int i = 0; i < _TaskList.ElementCount(); i++) {
      todo = _TaskList[i];
      todo->_TodoFunc(todo->_TodoThis, todo->_TodoData);
    }
  }
#endif

  return NULL;
}
