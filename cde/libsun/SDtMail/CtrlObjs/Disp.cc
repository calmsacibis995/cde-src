/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Dispatcher Class.

#pragma ident "@(#)Disp.cc	1.30 97/03/14 SMI"

// Include Files.
#include <CtrlObjs/Disp.hh>

SdmDispatcher::SdmDispatcher(const SdmClassId classId, SdmError& err,
  const unsigned int concLevel)
  :  SdmLinkedListElement(this),
  _TaskList(), _ThreadList(concLevel), _IsStarted(Sdm_False)
{
  err = Sdm_EC_Success;
}

SdmDispatcher::~SdmDispatcher()
{
}

void
SdmDispatcher::AddTask(void (*todoFunc) (SdmPrim*, void*), SdmPrim* todoThis,
                 void* todoData)
{
  _TaskList.AddTodoFunc(todoFunc, todoThis, todoData);
}

void
SdmDispatcher::RemTask(const int indx)
{
  _TaskList.RemTodoFunc(indx);
}

void 
SdmDispatcher::GetSignalsIgnoredByThreads(sigset_t& r_signals)
{
  sigemptyset(&r_signals);
  sigaddset(&r_signals, SIGHUP);
  sigaddset(&r_signals, SIGINT);
  sigaddset(&r_signals, SIGPIPE);
  sigaddset(&r_signals, SIGALRM);
  sigaddset(&r_signals, SIGTERM);
  sigaddset(&r_signals, SIGUSR1);
  sigaddset(&r_signals, SIGUSR2);
  sigaddset(&r_signals, SIGCHLD);
  sigaddset(&r_signals, SIGPWR);
  sigaddset(&r_signals, SIGHUP);
  sigaddset(&r_signals, SIGWINCH);
  sigaddset(&r_signals, SIGURG);
  sigaddset(&r_signals, SIGSTOP);
  sigaddset(&r_signals, SIGTSTP);
  sigaddset(&r_signals, SIGCONT);
  sigaddset(&r_signals, SIGTTIN);
  sigaddset(&r_signals, SIGTTOU);
}

