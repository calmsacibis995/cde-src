/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Tickle Timed Dispatcher Class.

#ifndef _SDM_TICKDISP_H
#define _SDM_TICKDISP_H

#pragma ident "@(#)TickDisp.hh	1.32 96/11/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/Disp.hh>

static const int SdmTickDisp_DefConcLevel = 5;
class SdmTickleDispatcher : public SdmDispatcher {

public:
  SdmTickleDispatcher(SdmError& err, sema_t* semaVar, 
		  int threadCount = SdmTickDisp_DefConcLevel);
  virtual ~SdmTickleDispatcher();
  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

private:
  sema_t* _TickSemaVar;
  sema_t  _OneThreadShutdown;
  mutex_t _DataLock;
  int     _ThreadCount;
  SdmBoolean _DoShutdown;

private:
  void StartThrdServices();
  void StopThrdServices();
  static void* SR_WaitAndService(void* objThis);
  void* WaitAndService();
	
};

#endif
