/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Dispatcher Class.

#ifndef _SDM_DISP_H
#define _SDM_DISP_H

#pragma ident "@(#)Disp.hh	1.37 97/03/24 SMI"

#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/ThreadDefs.hh>
#include <DataStructs/TodoVec.hh>
#include <SDtMail/LinkedList.hh>
#include <signal.h>


class SdmDispatcher :  public SdmLinkedListElement {

public:
  SdmDispatcher(const SdmClassId classId, SdmError& err,
    const unsigned int concLevel);
  virtual ~SdmDispatcher();

  virtual SdmErrorCode StartUp(SdmError& err) = 0;
  virtual SdmErrorCode ShutDown(SdmError& err) = 0;

  void AddTask(void (*todoFunc) (SdmPrim*, void*), SdmPrim* todoThis,
		       void* todoData);
  void RemTask(const int indx);
  
  SdmBoolean IsStarted() const { return _IsStarted; }
  

protected:
  static void GetSignalsIgnoredByThreads(sigset_t& r_signals);

  SdmTodoVector _TaskList;
  SdmVector<thread_t> _ThreadList;
  SdmBoolean          _IsStarted;
};

#endif
