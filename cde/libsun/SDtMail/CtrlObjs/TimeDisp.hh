/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Tickle Timed Dispatcher Class.

#ifndef _SDM_TIMEDISP_H
#define _SDM_TIMEDISP_H

#pragma ident "@(#)TimeDisp.hh	1.31 96/06/20 SMI"

#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/Disp.hh>
#include <sys/time.h>

static const int SdmTimeDisp_DefConcLevel = 1;
class SdmTimedDispatcher : public SdmDispatcher {

public:
  SdmTimedDispatcher(SdmError& err);
  virtual ~SdmTimedDispatcher();
  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

  void SetFrequencyInSec(const time_t numSecs);
  time_t GetFrequencyInSec() const;

private:
  mutex_t _DataLock;
  sema_t  _OneThreadShutdown;
  time_t  _Frequency;
  int*    _StopFileDescs;

private:
  void StartThrdServices();
  void StopThrdServices();
  static void* SR_WaitAndService(void* objThis);
  void* WaitAndService();
};

#endif
