/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Dispatcher Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)TimeDisp.cc	1.42 96/11/21 SMI"
#endif

// Include Files.
#include <CtrlObjs/TimeDisp.hh>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <stropts.h>
#include <errno.h>
#include <DataStructs/MutexEntry.hh>
#include <SDtMail/Utility.hh>

static const int        READ = 0;
static const int        WRITE = 1;


SdmTimedDispatcher::SdmTimedDispatcher(SdmError& err)
  :SdmDispatcher(Sdm_CL_TimedDispatcher, err, SdmTimeDisp_DefConcLevel),
  _StopFileDescs(NULL), _Frequency(0)
{
  
#if defined(USE_SOLARIS_THREADS)
  int rc;
  rc =  mutex_init(&_DataLock, USYNC_THREAD, NULL);
  assert (rc == 0);
  rc = sema_init(&_OneThreadShutdown, 0, USYNC_THREAD, 0);
  assert (rc == 0);

  // create the pipe used for signaling shutdown.
  _StopFileDescs = new int[2];
  rc = pipe(_StopFileDescs);
  assert (rc == 0);
#endif
  
  err = Sdm_EC_Success;
}

SdmTimedDispatcher::~SdmTimedDispatcher()
{
  SdmError err;
  if (_IsStarted) {
    ShutDown(err);
  }

#if defined(USE_SOLARIS_THREADS)  
  // close file descripors used for pipe.
  if (_StopFileDescs) {
    close(_StopFileDescs[READ]);
    close(_StopFileDescs[WRITE]);
  }
  
  delete [] _StopFileDescs;
  sema_destroy(&_OneThreadShutdown);
  mutex_destroy(&_DataLock);
#endif

}

SdmErrorCode
SdmTimedDispatcher::StartUp(SdmError& err)
{
  if (_IsStarted) {
    err = Sdm_EC_Fail;
  } else {
    err = Sdm_EC_Success;
#if defined(USE_SOLARIS_THREADS)
    StartThrdServices();
#endif
    _IsStarted = Sdm_True;
  }
  return err;
}

SdmErrorCode
SdmTimedDispatcher::ShutDown(SdmError& err)
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
SdmTimedDispatcher::SetFrequencyInSec(const time_t numSecs)
{
  SdmMutexEntry entry(_DataLock);
  _Frequency = numSecs;
}

time_t
SdmTimedDispatcher::GetFrequencyInSec() const
{
  SdmTimedDispatcher *me = (SdmTimedDispatcher*) this;
  SdmMutexEntry entry(me->_DataLock);
  return (_Frequency);
}


void
SdmTimedDispatcher::StartThrdServices()
{
#if defined(USE_SOLARIS_THREADS)
  int thrstat;
  thread_t tid;
  thrstat = thr_create(0, 0, SR_WaitAndService, (void *) this, 0, &tid);
  assert (thrstat == 0);
  _ThreadList.AddElementToList(tid);
#endif
}
 
void
SdmTimedDispatcher::StopThrdServices()
{
#if defined(USE_SOLARIS_THREADS)
  assert (_ThreadList.ElementCount() > 0);
  char bogus_buf = 0;
  // write to the pipe to let the other thread know that it should exit.
  write(_StopFileDescs[WRITE], &bogus_buf, 1);
  sema_wait(&_OneThreadShutdown);
#endif
}

void*
SdmTimedDispatcher::SR_WaitAndService(void* objThis)
{
#if defined(USE_SOLARIS_THREADS)
  sigset_t ignoreSignals;
  GetSignalsIgnoredByThreads(ignoreSignals);
  thr_sigsetmask(SIG_BLOCK, &ignoreSignals, NULL);
#endif
  return ((SdmTimedDispatcher *) objThis)->WaitAndService();
}

// Note Actual Wait Here is = Frequency + Actual Time To do Task
void*
SdmTimedDispatcher::WaitAndService()
{
#if defined(USE_SOLARIS_THREADS)
  int rc, i;
  SdmTodo* todo;
  char bogus_buf;
  time_t frequency;
  struct pollfd pollFileDesc;
  pollFileDesc.fd = _StopFileDescs[READ];
  pollFileDesc.events = POLLIN;

  while (1) {
  
    frequency = GetFrequencyInSec();

    // wait for the time out to happen or for the file descriptor to get tickled.
    // if the file descriptor gets tickled, it means that the thread should exit.
    //
    if ((rc = poll(&pollFileDesc, 1, frequency*1000)) > 0) {
      // if pipe was tickled, then we exit the thread.
      if (read(_StopFileDescs[READ], &bogus_buf, 1) != 1) {
        // what should we do here is there is an error??
        SdmUtility::LogError(Sdm_True,
          "Error:  SdmTimedDispatcher::WaitAndService can not read from pipe. errno=%d\n", (int)errno);
      } 
      sema_post(&_OneThreadShutdown);
      thr_exit(0);

    } else if (rc == 0) {
      // poll timed out so we execute the tasks.
      for (i = 0; i < _TaskList.ElementCount(); i++) {
        todo = (SdmTodo *)_TaskList[i];
        todo->_TodoFunc(todo->_TodoThis, todo->_TodoData);
      }
    } else {
      // exit thread if there is an error.
      if (errno != EINTR && errno != EAGAIN) {
        SdmUtility::LogError(Sdm_True,
         "Error:  SdmTimedDispatcher::WaitAndService poll failed. errno=%d\n", (int)errno);
        sema_post(&_OneThreadShutdown);
        thr_exit(0);
      }
    }    
  }
#endif
  
  return NULL;
}
