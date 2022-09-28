/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the functions used to link the session to the XtEventLoop.

#pragma ident "@(#)SessionLink.cc	1.14 97/05/09 SMI"

#include <SDtMail/SessionLink.hh>


//
// forward declarations for external and internal functions
//
extern int           SdmHandleTimeOutInterval(SdmSession *ses, 
                      SdmPollEventType event, unsigned long &nextInterval);

extern SdmErrorCode SdmAttachSession(SdmError &err, int &r_pipe, void *&r_calldata, 
                      SdmSession &session, void* appContext);

SdmErrorCode SdmXtAddSessionTimeOuts(SdmError &err, SdmSession &ses);
             
                
// Define Xt functions as a weak symbols
// so that clients are not required to link with libXt.so.  
// If libXt.so is not available, then the Xt functions
// are just not called.
//
extern "C" XtInputId XtAppAddInput(XtAppContext, int, XtPointer,
                                   XtInputCallbackProc, XtPointer);

extern "C" void XtRemoveInput(XtInputId);

extern "C"  XtIntervalId  XtAppAddTimeOut(XtAppContext, unsigned long, 
                                   XtTimerCallbackProc, XtPointer);

extern "C" void XtRemoveTimeOut(XtIntervalId);

#pragma weak XtAppAddInput
#pragma weak XtRemoveInput
#pragma weak XtAppAddTimeOut
#pragma weak XtRemoveTimeOut





// Global functions for handling session linkage to the caller.
//

void
SdmXtHandleTimeOutInterval(XtPointer calldata, XtIntervalId *id)
{
  //
  // the time out is automatically unregistered after this
  // procedure is called.  therefore, we need to re-register
  // the timeout procedure if there was no error in handling
  // the timeout.
  // 
  SdmTimeOutInfo *info = (SdmTimeOutInfo*) calldata;
  if (SdmHandleTimeOutInterval(info->_session, info->_event, info->_intervalInSecs) >= 0) {
   if (XtAppAddTimeOut) 
     // we need to set the id for the new time out.
     info->_id = XtAppAddTimeOut((XtAppContext)info->_appContext, info->_intervalInSecs, 
                   SdmXtHandleTimeOutInterval, calldata);
  } else {
    if (XtRemoveTimeOut)
      XtRemoveTimeOut(*id);
    info->_id = 0;
    info->_isStarted = Sdm_False;
  }
}

SdmErrorCode
SdmXtAttachSession(SdmError &err, XtAppContext &appcontext, SdmSession &ses)
{
   int readFileDesc;
   void *eventQueue;

   if (SdmAttachSession(err, readFileDesc, eventQueue, ses, appcontext))
     return err;

#if !defined(USE_SOLARIS_THREADS)
  // we need to add the session time outs in the case when we are single-threaded.
  // StartUp is called before SdmAttachSession so the StartUp would not have
  // added the time outs because the app context was undefined at the time.
  if (SdmXtAddSessionTimeOuts(err, ses))
     return err;
#endif 

   if (XtAppAddInput)
     XtAppAddInput(appcontext, readFileDesc, (XtPointer)XtInputReadMask,
                   SdmXtHandleSessionEvent, (XtPointer) eventQueue);

   return err;
}
 
void
SdmXtHandleSessionEvent(XtPointer calldata, int *pipe, XtInputId *id)
{
  // if an error occurred in handling the session event, 
  // remove this input source.  there queue and pipe used
  // to get replies from the session have been deleted by
  // SdmHandleSessionEvent so we can't get events from the
  // session anymore.
  //
  if (SdmHandleSessionEvent(calldata, pipe, id) < 0) {
   if (XtRemoveInput)
      XtRemoveInput(*id);
  }
}


SdmErrorCode
SdmXtAddSessionTimeOuts(SdmError &err, SdmSession &ses)
{
  err = Sdm_EC_Success;
  
#if !defined(USE_SOLARIS_THREADS)
  // set up the interval and timeout for each event.
  for (int i = 0; i < SdmSession::SdmD_TimeOutDataSet_Size; i++) {
    SdmTimeOutInfo *info = ses._TimeOutDataList[i];
    if (info && info->_appContext != NULL && XtAppAddTimeOut && info->_isStarted == Sdm_False) {
       info->_intervalInSecs = (ses.GetInterval(info->_event) * 1000);
       info->_id = XtAppAddTimeOut((XtAppContext)info->_appContext, 
                     info->_intervalInSecs, SdmXtHandleTimeOutInterval, (XPointer) info);
       info->_isStarted = Sdm_True;
    }
  }
#endif
  
  return err;
}

SdmErrorCode
SdmXtRemoveSessionTimeOuts(SdmError &err, SdmSession &ses)
{
  err = Sdm_EC_Success;
  
#if !defined(USE_SOLARIS_THREADS)
  // remove the timeout for each event.
  for (int i = 0; i < SdmSession::SdmD_TimeOutDataSet_Size; i++) {
    SdmTimeOutInfo *info = ses._TimeOutDataList[i];
    if (info && info->_isStarted && XtRemoveTimeOut) {
       XtRemoveTimeOut(info->_id);
       info->_id = 0;
       info->_isStarted = Sdm_False;
    }
  }
#endif
  
  return err;
}
