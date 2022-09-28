/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Data Port class.
// --> It implements the "data port api" that is used to
// --> gain access to underlying channel adapters that provide
// --> interfaces to various service providers.

#pragma ident "@(#)DataPort.cc	1.133 97/05/13 SMI"

// Include Files.
#include <assert.h>
#include <iostream.h>
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChanEditMsg.hh>
#include <PortObjs/DataChanTclient.hh>
#include <Utils/TnfProbe.hh>
#include <SDtMail/SystemUtility.hh>

extern SdmDpDataChan *SdmDpDataChanCclientFactory(SdmDataPort* parentDataPort);
#if	defined(INCLUDE_TCLIENT)
extern SdmDpDataChan *SdmDpDataChanTclientFactory(SdmDataPort* parentDataPort);
#endif	// defined(INCLUDE_TCLIENT)
extern SdmDpDataChan *SdmDpDataChanSmtpFactory(SdmDataPort* parentDataPort);

#define DATAPORT_SIGNATURE_VALID (_dpObjSignature && _dpObjSignature <= _dpObjSignatureCounter)
static const int INITIAL_HEADER_CACHE_SIZE = 15;        // number of header slots to preallocate in header arrays
static const int INITIAL_SINGLE_HEADER_CACHE_SIZE = 5;        // number of header slots to preallocate in single header fetch arrays

SdmObjectSignature SdmDataPort::_dpObjSignatureCounter = 0;   // 0 reserved for "none"
SdmDpGroupPrivilegeActionCallback SdmDataPort::_dpGroupPrivilegeActionCallback = 0;
void* SdmDataPort::_dpGroupPrivilegeActionUserData = NULL;

SdmAbstractAttributeMap _sdmAbstractAttributes[] =
{
  {Sdm_MAA_charset,	"charset",	"charset"},
  {0,                   NULL,           NULL}
};

static SdmMessageHeaderAbstractFlag P_To[] =
  {Sdm_MHA_To, Sdm_MHA_Apparently_To, Sdm_MHA_Resent_To, 0};

static SdmMessageHeaderAbstractFlag P_Sender[] =
  {Sdm_MHA_Reply_To, Sdm_MHA_From, Sdm_MHA_Return_Path, Sdm_MHA_Resent_From, 0};


// This is the global abstract header table
SdmAbstractHeaderMap _sdmAbstractHeaders[] =
{
  // The first set simply map on to real header names
  {
    Sdm_MHA_Bcc, "rbcc", "bcc", sizeof("bcc")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Cc, "rcc", "cc", sizeof("cc")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Date, "rdate", "date", sizeof("date")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_From, "rfrom", "from", sizeof("from")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Subject, "rsubject", "subject", sizeof("subject")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_To, "rto", "to", sizeof("to")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Apparently_To, "rapparentlyto", "apparently-to", sizeof("apparently-to")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Resent_To, "rresent-To", "resent-to", sizeof("resent-to")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Reply_To, "rreply-To", "reply-to", sizeof("reply-to")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Return_Path, "rreturn-Path", "return-path", sizeof("return-path")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_Resent_From, "rresent-From", "resent-from", sizeof("resent-from")-1, NULL, Sdm_False
  },
  // The next set describes "pseudo" headers - their meaning is intended to
  // be channel adapter dependent, although right now only the c-client
  // channel adapter really supports most of them.  If an entry specifies a
  // "realName", then it's just a synonym for a real header; otherwise it's
  // a header which can be fetched in a more optimum way using the
  // c-client's SHORTINFO.
  {
    Sdm_MHA_P_Bcc, "pbcc", "bcc", sizeof("bcc")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_P_Cc, "pcc", "cc", sizeof("cc")-1, NULL, Sdm_False
  },
  {
    Sdm_MHA_P_SentDate, "psentdate", "date", sizeof("date")-1, NULL, Sdm_False
  },
  // The following entries have no "realName" - there's no corresponding
  // real header to search for.  However, these fields are available from
  // the c-client's SHORTINFO.
  {
    Sdm_MHA_P_MessageFrom, "pmessagefrom", "from", sizeof("from")-1, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_MessageSize, "pmessagesize", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_MessageType, "pmessagetype", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_ReceivedDate, "preceiveddate", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_AbbreviatedReceivedDate, "pabbreviatedreceiveddate", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_SenderPersonalInfo, "psenderpersonalinfo", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_SenderUsername, "psenderusername", NULL, 0, NULL, Sdm_True
  },
  {
    Sdm_MHA_P_Subject, "psubject", "subject", sizeof("subject")-1, NULL, Sdm_True
  },
  // The next entries are "indirect" pseudo headers - they specify a list of
  // other real headers to search for.
  {
    Sdm_MHA_P_To, "pto", NULL, 0, P_To, Sdm_False
  },
  {
    Sdm_MHA_P_Sender, "psender", NULL, 0, P_Sender, Sdm_False
  },
  {
    0, NULL, NULL,0, NULL, Sdm_False
  }
};

SdmAbstractFlagMap _sdmAbstractFlags[] =
{
  {Sdm_MFA_Answered,	"answered"},
  {Sdm_MFA_Deleted,	"deleted"},
  {Sdm_MFA_Draft,	"draft"},
  {Sdm_MFA_Flagged,	"flagged"},
  {Sdm_MFA_Recent,	"recent"},
  {Sdm_MFA_Seen,	"seen"},
  {Sdm_MFA_ALL,		"all"},
  {0,			NULL}
};

// -- begin stuff

// ProcessExpungeResults
// This method is called upon conclusion of an ExpungeDeletedMessages call;
// it is given the result list from the expunge call. It performs these functions:
//  -> unredirect (discard) all edited messages that were expunged
//  -> adjust the message numbers of all redirected messages that were
//	affected by the expunge (e.g. either the message itself has a new number
//	or the original source message the redirected message is based on has
//	changed as a result of the expunge)
//

void SdmDataPort::ProcessExpungeResults(SdmMessageNumberL& msgnums)
{
  TNF_PROBE_0 (SdmDataPort_ProcessExpungeResults_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ProcessExpungeResults_start");

  assert(_dpDataChan);

  // Spin through and unredirect all messages that have just been expunged

  int numMsgMaps = _dpMsgnumToChanL.ElementCount();
  if (numMsgMaps) {
    for (int i = numMsgMaps-1; i >= 0; i--) {
      DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
      if (msgnums.FindElement(mtc->_mtcMsgnum) != Sdm_Not_Found) {
	// This redirected message has been expunged - must nuke it
	// Unfortunately the data channel edit message adapter is purposefully stubborn
	// about not wanting to throw away changes to existing messages - it will attempt
	// to save it on shutdown; by calling ShutdownRedirectedMessage with a shutdownFlag
	// argument of Sdm_False, ShutDown is bypassed and the channel adapter directly 
	// destroyed - this causes the commit to be bypassed and the changes discarded.

	ShutdownRedirectedMessage(mtc->_mtcMsgnum, Sdm_False);
      }
    }
  }

  // Spin through each redirected message remaining and pass in the expunged list
  // to allow the redirected message to renumber itself appropriately

  numMsgMaps = _dpMsgnumToChanL.ElementCount();
  for (int i = 0; i < numMsgMaps; i++) {
    SdmError localError;
    DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
    SdmMessageNumber newMsgnum = mtc->_mtcMsgnum;
    SdmMessageNumber newSrcMsgnum = mtc->_mtcSrcMsgnum;

    // Compute the adjusted message number for this message

    int numEntries = msgnums.ElementCount();
    for (int j = 0; j < numEntries; j++) {
      SdmMessageNumber targMsgnum = msgnums[j];
      assert(targMsgnum != mtc->_mtcMsgnum);
      if (targMsgnum < mtc->_mtcMsgnum)
	newMsgnum--;
      if (targMsgnum < mtc->_mtcSrcMsgnum)
	newSrcMsgnum--;
    }
    assert(newMsgnum);

    // Cause this map entry and the edited message to take on the new adjusted message number

    if (mtc->_mtcMsgnum != newMsgnum || mtc->_mtcSrcMsgnum != newSrcMsgnum) {
      mtc->_mtcMsgnum = newMsgnum;
      mtc->_mtcSrcMsgnum = newSrcMsgnum;
      _dpMsgnumToChanL[i]._mtcChan->ReviseOriginalMessageNumber(localError, newMsgnum, newSrcMsgnum);
    }
  }

  TNF_PROBE_0 (SdmDataPort_ProcessExpungeResults_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ProcessExpungeResults_end");
}

// FetchAllContents
// This method is called to cause the contents of all derived messages to be cached
// in case subsequent operations might cause the original source messages to be deleted
//

void SdmDataPort::FetchAllContents()
{
  TNF_PROBE_0 (SdmDataPort_FetchAllContents_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_FetchAllContents_start");

  assert(_dpDataChan);

  // Spin through and cause all redirected messages to cache themselves
  // Done as a prelude to an expunge or other activity

  int numMsgMaps = _dpMsgnumToChanL.ElementCount();
  if (numMsgMaps) {
    for (int i = numMsgMaps-1; i >= 0; i--) {
      SdmError localError;
      DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
      // Only cache non edited messages, as edited messages if they are expunged or moved
      // will cause their edited components to be tossed when the operation completes.
      //
      if (mtc->_mtcMsgnum != mtc->_mtcSrcMsgnum) {
	(mtc->_mtcChan)->FetchAllContents(localError, mtc->_mtcMsgnum);
      }
    }
  }

  TNF_PROBE_0 (SdmDataPort_FetchAllContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_FetchAllContents_end");
}

SdmDpDataChan* SdmDataPort::MsgnumToChan(SdmBoolean& r_isRedirected, const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmDataPort_MsgnumToChan_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MsgnumToChan_start");

  assert(_dpDataChan);

  int numMsgMaps = _dpMsgnumToChanL.ElementCount();
  if (numMsgMaps) {
    for (int i = 0; i < numMsgMaps; i++) {
      DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
      if (mtc->_mtcMsgnum == msgnum) {
	assert(mtc->_mtcChan);
	r_isRedirected = Sdm_True;
          TNF_PROBE_0 (SdmDataPort_MsgnumToChan_end,
            "dpapi SdmDataPort",
            "sunw%debug SdmDataPort_MsgnumToChan_end");
	return(mtc->_mtcChan);
      }
    }
  }

  r_isRedirected = Sdm_False;

  TNF_PROBE_0 (SdmDataPort_MsgnumToChan_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MsgnumToChan_end");

  return(_dpDataChan);
}

SdmDpDataChan* SdmDataPort::MsgnumToChan(const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmDataPort_MsgnumToChan_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MsgnumToChan_start");
  SdmBoolean isRedirected;
  TNF_PROBE_0 (SdmDataPort_MsgnumToChan_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MsgnumToChan_end");
  return(MsgnumToChan(isRedirected, msgnum));
}

SdmErrorCode SdmDataPort::RedirectMessage(SdmError& err, const SdmMessageNumber msgnum)
{
  TNF_PROBE_0 (SdmDataPort_RedirectMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RedirectMessage_start");
  SdmBoolean isRedirected = Sdm_False;
  assert(MsgnumToChan(isRedirected, msgnum) == _dpDataChan);
  assert(isRedirected == Sdm_False);
  assert(err == Sdm_EC_Operation_Unavailable);

  err = Sdm_EC_Success;

  // Create an instance of the edit message pseudo channel adapter and cause
  // it to be derived from this message
  SdmDpDataChan* dc = new SdmDpDataChanEditMsg(this, this, msgnum);
  if (dc->StartUp(err) != Sdm_EC_Success) {
    delete dc;
    TNF_PROBE_0 (SdmDataPort_RedirectMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RedirectMessage_end");
    return(err);
  }

  // Insert the redirection into the redirection list
  DpMsgnumToChan* mtc = &_dpMsgnumToChanL[_dpMsgnumToChanL.AddElementToVector()];
  mtc->_mtcMsgnum = msgnum;	// redirect is based upon the original message number
  mtc->_mtcChan = dc;		// redirect message to new edit message pseudo channel adapter 
  mtc->_mtcSrcMsgnum = msgnum;	// original message has same message number
  mtc->_mtcSrcPort = this;	// original message is on this data port

  TNF_PROBE_0 (SdmDataPort_RedirectMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RedirectMessage_end");
  return(err = Sdm_EC_Success);
}

// This macro takes a message number and a data channel call as arguments;
// it executes the data channel call and if it returns a specific error
// code that signals that the operation requested is not implemented for
// this message or provider, then the message is redirected to the edit
// message pseudo channel adapter and the operation is retried

#define HANDLE_REDIRECT_INSERT(ERR,MSGNUM,STATEMENT)				\
{										\
  SdmBoolean __isRedirected = Sdm_False;					\
  while (!__isRedirected && ERR == Sdm_EC_Success) {				\
    if (MsgnumToChan(__isRedirected, MSGNUM)->STATEMENT == Sdm_EC_Operation_Unavailable) {	\
      RedirectMessage(ERR,MSGNUM);						\
    }										\
    else {									\
      break;									\
    }										\
  }										\
}

// -- end stuff

// Constructor

SdmDataPort::SdmDataPort
  (
  const SdmClassId classId
  ) : SdmPrim(classId),
    _dpErrorLogCallback(0), _dpErrorLogUserdata(0),
    _dpDebugLogCallback(0), _dpDebugLogUserdata(0),
    _dpNotifyLogCallback(0), _dpNotifyLogUserdata(0),
    _dpBusyIndicationCallback(0), _dpBusyIndicationUserdata(0),
    _dpLockActivityCallback(0), _dpLockActivityUserdata(0)
{
  TNF_PROBE_0 (SdmDataPort_constructor_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_constructor_start");
  _dpObjSignature = ++_dpObjSignatureCounter;
  _dpOpen = Sdm_False;		// no channel is open
  _dpAttached = Sdm_False;	// no channels are Attached
  _dpDataChan = (SdmDpDataChan* )0;

  // This data member is used to uniquely identify all locally created
  // messages - they grow down from the highest possible message number
  // and represent the ceiling for locally created messages.
  // Since there are 4294967296 possible messages, it would take a long
  // time before they collide with the number of messages in a store.
  //

  _dpLocalMessageNumberCeiling = ~0;

  TNF_PROBE_0 (SdmDataPort_constructor_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_constructor_end");
}

// destructor

SdmDataPort::~SdmDataPort()
{
  TNF_PROBE_0 (SdmDataPort_destructor_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_destructor_start");
  SdmError err;


  if (_IsStarted) {
    ShutDown(err);
  }
  assert(!_IsStarted);

  // Make sure the connection is fully closed down
  assert(!_dpOpen);
  assert(!_dpAttached);

  // Make sure all resouces that may have been allocated by this data port
  // have been successfully deallocated
  assert(!_dpDataChan);
  assert(!_dpDataChanL.ElementCount());
  assert(!_dpDataChanProvL.ElementCount());

  _dpObjSignature = 0;		// set signature to invalid so further use is flagged as error
  TNF_PROBE_0 (SdmDataPort_destructor_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_destructor_end");
}

// Catch assignment operator and trap

SdmDataPort& 
SdmDataPort::operator=(const SdmDataPort& rhs)
{
  cout << "*** Error: SdmDataPort assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

// catch copy constructor and trap

SdmDataPort::SdmDataPort(const SdmDataPort& copy) 
 : SdmPrim(0)
{
  cout << "*** Error: SdmDataPort copy constructor called\n";
  assert(Sdm_False);
}

void SdmDataPort::CallBusyIndicationCallback()
{
  TNF_PROBE_0 (SdmDataPort_CallBusyIndicationCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallBusyIndicationCallback_start");

  if (_dpBusyIndicationCallback) {
    SdmDpEvent theResponse;
    SdmDpBusyIndicationEvent* theCallbackEvent = &theResponse.sdmDpBusyIndication;
    theCallbackEvent->type = DPEVENT_BusyIndication;
    (*_dpBusyIndicationCallback)(_dpBusyIndicationUserdata, DPCBTYPE_BusyIndication, &theResponse);
  }

  TNF_PROBE_0 (SdmDataPort_CallBusyIndicationCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallBusyIndicationCallback_end");
}

void SdmDataPort::CallLockActivityCallback(SdmDpEventType lockActivityEvent)
{
  TNF_PROBE_1 (SdmDataPort_CallLockActivityCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallLockActivityCallback_start",
	       tnf_ulong, lockActivityEvent, (unsigned long) lockActivityEvent);

  if (_dpLockActivityCallback) {
    SdmDpEvent theResponse;
    SdmDpLockActivityEvent* theCallbackEvent = &theResponse.sdmDpLockActivity;
    theCallbackEvent->type = lockActivityEvent;
    (*_dpLockActivityCallback)(_dpLockActivityUserdata, DPCBTYPE_LockActivity, &theResponse);
  }

  TNF_PROBE_0 (SdmDataPort_CallLockActivityCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallLockActivityCallback_end");
}

// This is a static function as the group privileges are not based on any
// particular data port but are global to all data ports

void SdmDataPort::CallGroupPrivilegeActionCallback(SdmBoolean enableFlag)
{
  TNF_PROBE_1 (SdmDataPort_CallGroupPrivilegeActionCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallGroupPrivilegeActionCallback_start",
	       tnf_ulong, enableFlag, (unsigned long) enableFlag);

  if (SdmDataPort::_dpGroupPrivilegeActionCallback) {
    if(SdmSystemUtility::GroupPrivilegeLockoutControl(enableFlag) == Sdm_True)
      (*SdmDataPort::_dpGroupPrivilegeActionCallback)(SdmDataPort::_dpGroupPrivilegeActionUserData,
						      enableFlag);
  }

  TNF_PROBE_0 (SdmDataPort_CallGroupPrivilegeActionCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallGroupPrivilegeActionCallback_end");
}

void SdmDataPort::CallDebugLogCallback(SdmString& debugMessage)
{
  TNF_PROBE_1 (SdmDataPort_CallDebugLogCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallDebugLogCallback_start",
	       tnf_string, debugMessage, (const char *) debugMessage);

  if (_dpDebugLogCallback && debugMessage.Length()) {
    SdmDpEvent theResponse;
    SdmDpDebugLogEvent* theCallbackEvent = &theResponse.sdmDpDebugLog;
    theCallbackEvent->type = DPEVENT_DebugLog;
    theCallbackEvent->debugMessage = &debugMessage;
    (*_dpDebugLogCallback)(_dpDebugLogUserdata, DPCBTYPE_DebugLog, &theResponse);
  }

  TNF_PROBE_0 (SdmDataPort_CallDebugLogCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallDebugLogCallback_end");
}

void SdmDataPort::CallErrorLogCallback(SdmString& errorMessage)
{
  TNF_PROBE_1 (SdmDataPort_CallErrorLogCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallErrorLogCallback_start",
	       tnf_string, debugMessage, (const char *) errorMessage);

  if (_dpErrorLogCallback && errorMessage.Length()) {
    SdmDpEvent theResponse;
    SdmDpErrorLogEvent* theCallbackEvent = &theResponse.sdmDpErrorLog;
    theCallbackEvent->type = DPEVENT_ErrorLog;
    theCallbackEvent->errorMessage = &errorMessage;
    (*_dpErrorLogCallback)(_dpErrorLogUserdata, DPCBTYPE_ErrorLog, &theResponse);
  }

  TNF_PROBE_0 (SdmDataPort_CallErrorLogCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallErrorLogCallback_end");
}

void SdmDataPort::CallNotifyLogCallback(SdmString& notifyMessage)
{
  TNF_PROBE_1 (SdmDataPort_CallNotifyLogCallback_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallNotifyLogCallback_start",
	       tnf_string, debugMessage, (const char *) notifyMessage);

  if (_dpNotifyLogCallback && notifyMessage.Length()) {
    SdmDpEvent theResponse;
    SdmDpNotifyLogEvent* theCallbackEvent = &theResponse.sdmDpNotifyLog;
    theCallbackEvent->type = DPEVENT_NotifyLog;
    theCallbackEvent->notifyMessage = &notifyMessage;
    (*_dpNotifyLogCallback)(_dpNotifyLogUserdata, DPCBTYPE_NotifyLog, &theResponse);
  }

  TNF_PROBE_0 (SdmDataPort_CallNotifyLogCallback_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_CallNotifyLogCallback_end");
}

// startup the data port object

SdmErrorCode SdmDataPort::StartUp(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_StartUp_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_StartUp_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_StartUp_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_StartUp_end");
    return(err = Sdm_EC_Success);
  }
  
  // Initialize the data channel providers master list
  _dpDataChanProvL.ClearAndDestroyAllElements();
  _dpDataChanProvL.AddElementToList(new SdmDpDataChanProvider(&SdmDpDataChanCclientFactory, "cclient"));
#if	defined(INCLUDE_TCLIENT)
  _dpDataChanProvL.AddElementToList(new SdmDpDataChanProvider(&SdmDpDataChanTclientFactory, "tclient"));
#endif	// defined(INCLUDE_TCLIENT)
  _dpDataChanProvL.AddElementToList(new SdmDpDataChanProvider(&SdmDpDataChanSmtpFactory, "smtp"));
  
  // Clear the message to data channel mapping list
  _dpMsgnumToChanL.ClearAllElements();

  _IsStarted = Sdm_True;
  TNF_PROBE_0 (SdmDataPort_StartUp_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_StartUp_end");
  return (err = Sdm_EC_Success);
}

// shutdown the data port object

SdmErrorCode SdmDataPort::ShutDown(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_ShutDown_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutDown_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_ShutDown_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutDown_end");
    return(Sdm_EC_Success);
  }

  // Close and release any resources that may be open on this data port object

  ShutdownAllRedirectedMessages();
  ReleaseOpenChannels();
  ReleaseAttachedChannels();

  // Destroy the data channel providers master list

  _dpDataChanProvL.ClearAndDestroyAllElements();

  // Make sure all resouces that may have been allocated by this data port
  // have been successfully deallocated

  assert(!_dpDataChan);
  assert(!_dpDataChanL.ElementCount());
  assert(!_dpDataChanProvL.ElementCount());

  _IsStarted = Sdm_False;
  TNF_PROBE_0 (SdmDataPort_ShutDown_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutDown_end");
  return (err = Sdm_EC_Success);
}

// --------------------------------------------------------------------------------
// DATA PORT API 
// This is the API that is exported to the users of the data port
// --------------------------------------------------------------------------------

// Add a message body to the top level of an existing message
//
SdmErrorCode SdmDataPort::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success)
    return(err);

  HANDLE_REDIRECT_INSERT(err, msgnum, AddMessageBodyToMessage(err, r_msgstruct, bodytype, bodysubtype, msgnum))

  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return(err);
}

// Add a message body to a component of an existing message
//
SdmErrorCode SdmDataPort::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, AddMessageBodyToMessage(err, r_msgstruct, bodytype, bodysubtype, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return (err);
}

// Add additional header text to existing header for a single message
//
SdmErrorCode SdmDataPort::AddMessageHeader
  (
  SdmError& err, 
  const SdmString& hdr, 		// name of header to add
  const SdmString& newvalue, 		// new header value 
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success)
    return(err);

  HANDLE_REDIRECT_INSERT(err, msgnum, AddMessageHeader(err, hdr, newvalue, msgnum))

  return (err);
}

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode SdmDataPort::AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(err);
  }

  // If open, scan the open channel
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return (_dpDataChan->AddToSubscribedNamespace(err, name));
  }

  // Hard way - spin through the list
  assert (_dpAttached);
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->AddToSubscribedNamespace(err, name) == Sdm_EC_Success) {
      break;
    }
  }

  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return(err);
}

#endif

// Append one message by raw contents to the message store
//
SdmErrorCode SdmDataPort::AppendMessage
  (
  SdmError& err,
  const SdmString& contents,		      	// message raw contents to append
  const SdmString& date,
  const SdmString& from,
  SdmMessageFlagAbstractFlags flags 
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return(_dpDataChan->AppendMessage(err, contents, date, from, flags));
}

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode SdmDataPort::Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to Attach to
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  // ISSUES: HANDLE ERROR BETTER?
  SdmError tempErr;		// "ignorable" errors

  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  // must be initialized before a Attach is possible
  if (CheckStarted(err) != Sdm_EC_Success)
  {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(err);
  }

  // If already open, this is an error
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return (err = Sdm_EC_Open);
  }

  // clean up from any previous attempts (e.g. Attaches)
  // Release all channels if attached
  ReleaseAttachedChannels();

  // purge current channels list if it exists??
  assert(!_dpDataChanL.ElementCount());

  // Go through the data channel providers list and stop at the first one that can 
  // handle this attach request
  SdmDpDataChan* dc = 0;
  int numProviders = _dpDataChanProvL.ElementCount();
  for (int i = 0; i < numProviders; i++) {
    SdmDpDataChanProvider* dcp = _dpDataChanProvL[i];
    dc = dcp->DataChanFactory(this);
    err = Sdm_EC_Success;
    if (dc->StartUp(err) == Sdm_EC_Success) {
      if (dc->Attach(err, tk) == Sdm_EC_Success) {
        _dpDataChanL.AddElementToList(dc);
        break;
      } 
       dc->ShutDown(tempErr);
    }
    delete dc;
    dc = 0;
    
    if (err != Sdm_EC_Fail)  // Any error other than general failure stops search and is returned
      break;
  }

  // Didn't get any response from any data channel providers; return.
  if (dc == 0) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return (err);
  }

  // At least one channel responded - return success
  _dpAttached = Sdm_True;
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return (err = Sdm_EC_Success);
}

// Cancel any operations pending on the data port
//
// NOTE: Do NOT grab a concurrency lock, as a cancel "in line" makes no sense
// All channel adapters must be able to handle a CancelPendingOperations in
// a multi-threaded safe manner

SdmErrorCode SdmDataPort::CancelPendingOperations
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(err);
  }

  // If a data channel is open, pass the cancel request down to it

  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
    return(_dpDataChan->CancelPendingOperations(err));
  }

  // This means we are attached, for now the cancel is ignored as
  // there is not much that can be done besides a scan or get cap

  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return(err = Sdm_EC_Success);
}

// Close the current connection down
//
SdmErrorCode SdmDataPort::Close
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  // All redirected messages must be committed or discarded
  ShutdownAllRedirectedMessages();

  // Close the current connection down if open
  ReleaseOpenChannels();

  // Release all channels if attached
  ReleaseAttachedChannels();

  // All done - return
  TNF_PROBE_0 (SdmDataPort_AddMessageBodyToMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_AddMessageBodyToMessage_end");
  return(err = Sdm_EC_Success);
}

// Commit any pending changes to a message
//
SdmErrorCode SdmDataPort::CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messageRewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  )
{
  TNF_PROBE_0 (SdmDataPort_CommitPendingMessageChanges_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CommitPendingMessageChanges_start");
  SdmBoolean isRedirected;

  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CommitPendingMessageChanges_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CommitPendingMessageChanges_end");
    return(err);
  }

  MsgnumToChan(isRedirected, msgnum)->CommitPendingMessageChanges(err, r_messageRewritten, msgnum, includeBcc);

  if ((err == Sdm_EC_Success) && isRedirected)
    ShutdownRedirectedMessage(msgnum, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_CommitPendingMessageChanges_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CommitPendingMessageChanges_end");
  return(err);
}

// Copy a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::CopyMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to copy message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_CopyMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CopyMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessage_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_CopyMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessage_end");
  return(_dpDataChan->CopyMessage(err, token, msgnum));
}

// Copy a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_CopyMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CopyMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_CopyMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_end");
  return(_dpDataChan->CopyMessages(err, token, startmsgnum, endmsgnum));
}

// Copy a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_CopyMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CopyMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_CopyMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CopyMessages_end");
  return(_dpDataChan->CopyMessages(err, token, msgnums));
}

// Check for new messages in the message store
//
SdmErrorCode SdmDataPort::CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  )
{
  TNF_PROBE_0 (SdmDataPort_CheckForNewMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckForNewMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CheckForNewMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckForNewMessages_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_CheckForNewMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckForNewMessages_end");
  return(_dpDataChan->CheckForNewMessages(err, r_nummessages));
}

// Create a new message store from a token stream
//
SdmErrorCode SdmDataPort::Create
  (
  SdmError& err,
  const SdmString& name			// name of object to create
  )
{
  TNF_PROBE_0 (SdmDataPort_Create_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Create_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Create_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Create_end");
    return(err);
  }

  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_Create_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Create_end");
    return (_dpDataChan->Create(err, name));
  }

  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->Create(err, name) == Sdm_EC_Success) {
      break;
    }
  }

  TNF_PROBE_0 (SdmDataPort_Create_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Create_end");
  return(err);
}

// Create a new message in a message store
//
SdmErrorCode SdmDataPort::CreateNewMessage
  (
  SdmError& err,
  SdmMessageNumber& r_msgnum		// message number of newly created message
  )
{
  TNF_PROBE_0 (SdmDataPort_CreateNewMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateNewMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CreateNewMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateNewMessage_end");
    return(err);
  }

  // Create an instance of the edit message pseudo channel adapter.

  SdmDpDataChan* dc = new SdmDpDataChanEditMsg(this);
  if (dc->StartUp(err) != Sdm_EC_Success) {
    delete dc;
    TNF_PROBE_0 (SdmDataPort_CreateNewMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateNewMessage_end");
    return(err);
  }

  // Create a new message
  SdmMessageNumber newmsgnum = _dpLocalMessageNumberCeiling--;

  if (dc->CreateNewMessage(err, newmsgnum) != Sdm_EC_Success) {
    SdmError localError;
    dc->ShutDown(localError);
    delete dc;
    TNF_PROBE_0 (SdmDataPort_CreateNewMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateNewMessage_end");
    return(err);
  }

  // Insert the redirection into the redirection list

  DpMsgnumToChan* mtc = &_dpMsgnumToChanL[_dpMsgnumToChanL.AddElementToVector()];
  mtc->_mtcMsgnum = newmsgnum;	// redirect is based upon the new local message number
  mtc->_mtcChan = dc;		// redirect message to new edit message pseudo channel adapter 
  mtc->_mtcSrcMsgnum = 0;	// original message has same message number
  mtc->_mtcSrcPort = 0;		// original message from the specified data port

  r_msgnum = newmsgnum;

  TNF_PROBE_0 (SdmDataPort_CreateNewMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateNewMessage_end");
  return (err = Sdm_EC_Success);
}

// Create a new message in a message store derived from an existing message
//
SdmErrorCode SdmDataPort::CreateDerivedMessage
  (
  SdmError& err,
  SdmMessageNumber& r_msgnum,		// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  )
{
  TNF_PROBE_0 (SdmDataPort_CreateDerivedMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateDerivedMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_CreateDerivedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateDerivedMessage_end");
    return(err);
  }

  // Create an instance of the edit message pseudo channel adapter.

  SdmDpDataChan* dc = new SdmDpDataChanEditMsg(this);
  if (dc->StartUp(err) != Sdm_EC_Success) {
    delete dc;
    TNF_PROBE_0 (SdmDataPort_CreateDerivedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateDerivedMessage_end");
    return(err);
  }

  // Cause the edit message to derive itself from the specified message
  SdmMessageNumber newmsgnum = _dpLocalMessageNumberCeiling--;

  if (dc->CreateDerivedMessage(err, newmsgnum, msgdp, msgnum) != Sdm_EC_Success) {
    SdmError localError;
    dc->ShutDown(localError);
    delete dc;
    TNF_PROBE_0 (SdmDataPort_CreateDerivedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateDerivedMessage_end");
    return(err);
  }

  // Insert the redirection into the redirection list

  DpMsgnumToChan* mtc = &_dpMsgnumToChanL[_dpMsgnumToChanL.AddElementToVector()];
  mtc->_mtcMsgnum = newmsgnum;	// redirect is based upon the new local message number
  mtc->_mtcChan = dc;		// redirect message to new edit message pseudo channel adapter 
  mtc->_mtcSrcMsgnum = msgnum;	// original message has same message number
  mtc->_mtcSrcPort = (SdmDataPort*)&msgdp;	// original message from the specified data port

  r_msgnum = newmsgnum;

  TNF_PROBE_0 (SdmDataPort_CreateDerivedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CreateDerivedMessage_end");
  return (err = Sdm_EC_Success);
}

// Delete the currently open object
//
SdmErrorCode SdmDataPort::Delete
  (
  SdmError& err,
  const SdmString& name			// name of the message store to delete
  )
{
  TNF_PROBE_0 (SdmDataPort_Delete_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Delete_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Delete_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Delete_end");
    return(err);
  }

  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_Delete_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Delete_end");
    return (_dpDataChan->Delete(err, name));
  }

  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->Delete(err, name) == Sdm_EC_Success) {
      break;
    }
  }

  TNF_PROBE_0 (SdmDataPort_Delete_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Delete_end");
  return(err);
}

// Delete a message body from an existing message
//
SdmErrorCode SdmDataPort::DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to delete
  )
{
  TNF_PROBE_0 (SdmDataPort_DeleteMessageBodyFromMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DeleteMessageBodyFromMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_DeleteMessageBodyFromMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DeleteMessageBodyFromMessage_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, DeleteMessageBodyFromMessage(err, r_previousState, newState, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_DeleteMessageBodyFromMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DeleteMessageBodyFromMessage_end");
  return (err);
}

// Discard any pending changes to a message
//
SdmErrorCode SdmDataPort::DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  )
{
  TNF_PROBE_0 (SdmDataPort_DiscardPendingMessageChanges_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DiscardPendingMessageChanges_start");
  SdmBoolean isRedirected;

  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_DiscardPendingMessageChanges_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DiscardPendingMessageChanges_end");
    return(err);
  }

  MsgnumToChan(isRedirected, msgnum)->DiscardPendingMessageChanges(err, msgnum);

  if ((err == Sdm_EC_Success) && isRedirected)
    ShutdownRedirectedMessage(msgnum, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_DiscardPendingMessageChanges_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_DiscardPendingMessageChanges_end");
  return(err);
}

// Disconnect the currently open object from its server
//
SdmErrorCode SdmDataPort::Disconnect
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDataPort_Disconnect_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Disconnect_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Disconnect_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Disconnect_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_Disconnect_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Disconnect_end");
  return(_dpDataChan->Disconnect(err));
}

// Expunge deleted messages from the currently open message store
//
SdmErrorCode SdmDataPort::ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// list of message numbers of messages deleted
  )
{
  TNF_PROBE_0 (SdmDataPort_ExpungeDeletedMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ExpungeDeletedMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (r_msgnums.ElementCount()) { 
    // passed in return value list must be empty
    err = Sdm_EC_ReturnArgumentListHasContents;
  } else if (CheckOpen(err) == Sdm_EC_Success) {
    FetchAllContents();
    _dpDataChan->ExpungeDeletedMessages(err, r_msgnums);
    if (err == Sdm_EC_Success)
      ProcessExpungeResults(r_msgnums);
  }

  TNF_PROBE_0 (SdmDataPort_ExpungeDeletedMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ExpungeDeletedMessages_end");

  return (err);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err);
  }

  if (r_attribute.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
  return (MsgnumToChan(msgnum)->GetMessageAttributes(err, r_attribute, msgnum, mcmp)
	  != Sdm_EC_Success ? err : (r_attribute.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a specific attribute for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageAttribute_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttribute_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (r_attribute.Length())		// does the passed in return object have contents?
    r_attribute = "";			// yes: string is "base" type, empty it out

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttribute_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttribute_end");
  return(MsgnumToChan(msgnum)->GetMessageAttribute(err, r_attribute, attribute, msgnum, mcmp) 
	  != Sdm_EC_Success ? err : (r_attribute.Length() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err);
  }

  if (r_attribute.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
  return(MsgnumToChan(msgnum)->GetMessageAttributes(err, r_attribute, attributes, msgnum, mcmp)
	  != Sdm_EC_Success ? err : (r_attribute.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err);
  }

  if (r_attribute.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageAttributes_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageAttributes_end");
  return(MsgnumToChan(msgnum)->GetMessageAttributes(err, r_attribute, attributes, msgnum, mcmp) 
	  != Sdm_EC_Success ? err : (r_attribute.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

#endif


// Returns boolean indicating whether message is currently cached.
//
SdmErrorCode SdmDataPort::GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageCacheStatus_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageCacheStatus_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageCacheStatus_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageCacheStatus_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageCacheStatus_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageCacheStatus_end");
  return(_dpDataChan->GetMessageCacheStatus(err, r_cached, msgnum, mcmp));
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageContents_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (r_contents.Length())		// does the passed in return object have contents?
    r_contents = "";			// yes: string is "base" type, empty it out

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_end");
  return(MsgnumToChan(msgnum)->GetMessageContents(err, r_contents, contenttype, msgnum));
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDataPort::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageContents_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (r_contents.Length())		// does the passed in return object have contents?
    r_contents = "";			// yes: string is "base" type, empty it out

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageContents_end");
  return(MsgnumToChan(msgnum)->GetMessageContents(err, r_contents, contenttype, msgnum, mcmp));
}

// Return flags for a single message
//
SdmErrorCode SdmDataPort::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
  return(MsgnumToChan(msgnum)->GetMessageFlags(err, r_aflags, msgnum));
}

// Return a list of flags for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
    return(err);
  }

  if (r_aflags.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_aflags.SetVectorSize((endmsgnum-startmsgnum)+1, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
  return(_dpDataChan->GetMessageFlags(err, r_aflags, startmsgnum, endmsgnum));
}

// Return a list of flags for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
    return(err);
  }

  if (r_aflags.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_aflags.SetVectorSize(msgnums.ElementCount(), Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageFlags_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageFlags_end");
  return(_dpDataChan->GetMessageFlags(err, r_aflags, msgnums));
}


// Return a list of headers for a single header for a single message
//
SdmErrorCode SdmDataPort::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err);
  }

  if (r_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_hdr.SetVectorSize(INITIAL_SINGLE_HEADER_CACHE_SIZE, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
  return(MsgnumToChan(msgnum)->GetMessageHeader(err, r_hdr, hdr, msgnum) 
	  != Sdm_EC_Success ? err : (r_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of all headers for a single message
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_hdr.SetVectorSize(INITIAL_HEADER_CACHE_SIZE, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(MsgnumToChan(msgnum)->GetMessageHeaders(err, r_hdr, msgnum) 
	  != Sdm_EC_Success ? err : (r_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of specific headers for a single message
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_hdrs.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_hdrs.SetVectorSize(INITIAL_HEADER_CACHE_SIZE, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(MsgnumToChan(msgnum)->GetMessageHeaders(err, r_hdrs, hdrs, msgnum) 
	  != Sdm_EC_Success ? err : (r_hdrs.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of headers for a single message
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_hdr.SetVectorSize(INITIAL_HEADER_CACHE_SIZE, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(MsgnumToChan(msgnum)->GetMessageHeaders(err, r_hdr, hdr, msgnum) 
	  != Sdm_EC_Success ? err : (r_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of headers for a single message
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_hdr.SetVectorSize(INITIAL_HEADER_CACHE_SIZE, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(MsgnumToChan(msgnum)->GetMessageHeaders(err, r_hdr, hdr, msgnum) 
	  != Sdm_EC_Success ? err : (r_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_copiesof_hdr.SetVectorSize(msgnums.ElementCount(), Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, hdr, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of lists of headers for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  r_copiesof_hdr.SetVectorSize((endmsgnum-startmsgnum)+1, Sdm_True);

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, hdr, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
  return(_dpDataChan->GetMessageHeader(err, r_copiesof_hdr, hdr, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeader_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeader_end");
  return(_dpDataChan->GetMessageHeader(err, r_copiesof_hdr, hdr, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of lists of all headers for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdrs,		// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdrs.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdrs, hdrs, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdrs.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdrs,		// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdrs.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdrs, hdrs, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdrs.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of lists of headers for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, hdr, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err);
  }

  if (r_copiesof_hdr.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageHeaders_end");
  return(_dpDataChan->GetMessageHeaders(err, r_copiesof_hdr, hdr, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_hdr.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}


#endif

// Return the message structure for a single message
//
SdmErrorCode SdmDataPort::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err);
  }

  if (r_structure.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
  return(MsgnumToChan(msgnum)->GetMessageStructure(err, r_structure, msgnum) 
	  != Sdm_EC_Success ? err : (r_structure.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
SdmErrorCode SdmDataPort::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_copiesof_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err);
  }

  if (r_copiesof_structure.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
  return(_dpDataChan->GetMessageStructure(err, r_copiesof_structure, startmsgnum, endmsgnum) 
	  != Sdm_EC_Success ? err : (r_copiesof_structure.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

// Return a list of message structures for a list of messages
//
SdmErrorCode SdmDataPort::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_copiesof_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err);
  }

  if (r_copiesof_structure.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStructure_end");
  return(_dpDataChan->GetMessageStructure(err, r_copiesof_structure, msgnums) 
	  != Sdm_EC_Success ? err : (r_copiesof_structure.ElementCount() ? err : (err = Sdm_EC_RequestedDataNotFound)));
}

#endif

// Return a message store status structure for the message store
//
SdmErrorCode SdmDataPort::GetMessageStoreStatus
  (
  SdmError& err,
  SdmMessageStoreStatus& r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  )
{
  TNF_PROBE_0 (SdmDataPort_GetMessageStoreStatus_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStoreStatus_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_GetMessageStoreStatus_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStoreStatus_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_GetMessageStoreStatus_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_GetMessageStoreStatus_end");
  return(_dpDataChan->GetMessageStoreStatus(err, r_status, storeFlags));
}

// open a connection to an object given a token stream
//
SdmErrorCode SdmDataPort::Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  )
{
  TNF_PROBE_0 (SdmDataPort_Open_start,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_Open_start");
  SdmError tempErr;		// "ignorable" errors

  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  // must be initialized before a connect is possible
  if (CheckStarted(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Open_end,
		 "dpapi SdmDataPort",
		 "sunw%debug SdmDataPort_Open_end");
    return(err);
  }

  // if already open, this is an error
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_Open_end,
		 "dpapi SdmDataPort",
		 "sunw%debug SdmDataPort_Open_end");
    return (err = Sdm_EC_Open);
  }

  // clean up from any previous attempts (e.g. connects)
  ReleaseAttachedChannels();

  // purge current channels list if it exists??
  assert(!_dpDataChanL.ElementCount());

  // Go through the data channel providers list and stop at the first one that can 
  // handle this open request
  int numProviders = _dpDataChanProvL.ElementCount();
  SdmDpDataChanProvider* dcp = 0;
  SdmDpDataChan* dc = 0;
  for (int i = 0; i < numProviders; i++) {
    dcp = _dpDataChanProvL[i];
    dc = dcp->DataChanFactory(this);
    err = Sdm_EC_Success;
    if (dc->StartUp(err) == Sdm_EC_Success) {
      if (dc->Open(err, r_nmsgs, r_readOnly, tk) == Sdm_EC_Success)
        break;
      dc->ShutDown(tempErr);	// shutdown port and ignore any error in doing so
    }
    delete dc;
    dc = 0;
    if (err != Sdm_EC_Fail)	// Any error other than general failure stops search and is returned
      break;
  }

  // If no data channel worked, return the error from the last one encountered
  //
  if (!dc) {
    TNF_PROBE_0 (SdmDataPort_Open_end,
		 "dpapi SdmDataPort",
		 "sunw%debug SdmDataPort_Open_end");
    return(err);
  }

  _dpOpen = Sdm_True;
  _dpDataChan = dc;
  assert(CheckOpen(err) == Sdm_EC_Success);
  TNF_PROBE_0 (SdmDataPort_Open_end,
	       "dpapi SdmDataPort",
	       "sunw%debug SdmDataPort_Open_end");
  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Move a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::MoveMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_MoveMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessage_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_MoveMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessage_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_MoveMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessage_end");
  return(_dpDataChan->MoveMessage(err, token, msgnum));
}

// Move a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_MoveMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_MoveMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_MoveMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_end");
  return(_dpDataChan->MoveMessages(err, token, startmsgnum, endmsgnum));
}

// Move a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDataPort::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_MoveMessages_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_MoveMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_MoveMessages_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_MoveMessages_end");
  return(_dpDataChan->MoveMessages(err, token, msgnums));
}

#endif

// Perform a search against all messages and return a list of messages that match
//
SdmErrorCode SdmDataPort::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,	// list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  )
{
  TNF_PROBE_0 (SdmDataPort_PerformSearch_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err);
  }

  if (r_msgnums.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
  return(_dpDataChan->PerformSearch(err, r_msgnums, search));
}

// Perform a search against a range of messages and return a list of messages that match
//
SdmErrorCode SdmDataPort::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_PerformSearch_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err);
  }

  if (r_msgnums.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
  return(_dpDataChan->PerformSearch(err, r_msgnums, search, startmsgnum, endmsgnum));
}

// Perform a search against a list of messages and return a list of messages that match
//
SdmErrorCode SdmDataPort::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_PerformSearch_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err);
  }

  if (r_msgnums.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
  return(_dpDataChan->PerformSearch(err, r_msgnums, search, msgnums));
}


// Perform a search against a single message and return an indication of the search results
//
SdmErrorCode SdmDataPort::PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// results of search against message
  const SdmSearch& search,		// search criteria to apply to message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_PerformSearch_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_PerformSearch_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PerformSearch_end");
  return(MsgnumToChan(msgnum)->PerformSearch(err, r_bool, search, msgnum));
}


// Ping the server.
//
SdmErrorCode SdmDataPort::PingServer
  (
  SdmError &err
  )
{
  TNF_PROBE_0 (SdmDataPort_PingServer_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PingServer_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_PingServer_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PingServer_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_PingServer_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_PingServer_end");
  return(_dpDataChan->PingServer(err));
}


// Reconnect the currently disconnected object to its server
//
SdmErrorCode SdmDataPort::Reconnect
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDataPort_Reconnect_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Reconnect_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Reconnect_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Reconnect_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_Reconnect_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Reconnect_end");
  return(_dpDataChan->Reconnect(err));
}

// Register a "callback" for the group privilege action 
// This is a global as it does not relate to a particular data port object

SdmErrorCode SdmDataPort::RegisterGroupPrivilegeActionCallback
  (
  SdmError& err,
  SdmDpGroupPrivilegeActionCallback callback,
  void* userdata
  )
{
  TNF_PROBE_0 (SdmDataPort_RegisterCallback_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RegisterCallback_start");

  SdmDataPort::_dpGroupPrivilegeActionCallback = callback;
  SdmDataPort::_dpGroupPrivilegeActionUserData = userdata;

  TNF_PROBE_0 (SdmDataPort_RegisterCallback_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RegisterCallback_end");

  return(err = Sdm_EC_Success);
}

// Register a "callback" with the data port object
//
SdmErrorCode SdmDataPort::RegisterCallback
  (
  SdmError& err,
  const SdmDpCallbackType callbacktype,
  SdmDpCallback callback,
  void* userdata
  )
{
  TNF_PROBE_0 (SdmDataPort_RegisterCallback_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RegisterCallback_start");

  err = Sdm_EC_Success;			// default to return success

  switch(callbacktype) {
  case DPCBTYPE_ErrorLog:		// "error" type messages generated by the data port
    _dpErrorLogCallback = callback;
    _dpErrorLogUserdata = userdata;
    break;
  case DPCBTYPE_DebugLog:		// "debug" type messages generated by the data port
    _dpDebugLogCallback = callback;
    _dpDebugLogUserdata = userdata;
    break;
  case DPCBTYPE_NotifyLog:		// "notify" type messages generated by the data port
    _dpNotifyLogCallback = callback;
    _dpNotifyLogUserdata = userdata;
    break;
  case DPCBTYPE_BusyIndication:		// an indication that the data port is busy processing a command
    _dpBusyIndicationCallback = callback;
    _dpBusyIndicationUserdata = userdata;
    break;
  case DPCBTYPE_LockActivity:		// lock event generated by the data port
    _dpLockActivityCallback = callback;
    _dpLockActivityUserdata = userdata;
    break;
  default:				// a type we are not prepared for??
    assert(callbacktype != callbacktype);
    err = Sdm_EC_Fail;
  }

  TNF_PROBE_0 (SdmDataPort_RegisterCallback_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RegisterCallback_end");
  return(err);
}


// Rename the current object given a new token stream
//
SdmErrorCode SdmDataPort::Rename
  (
  SdmError& err,
  const SdmString& currentname,		// current name for message store
  const SdmString& newname		// new name for message store
  )
{
  TNF_PROBE_0 (SdmDataPort_Rename_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Rename_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Rename_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Rename_end");
    return(err);
  }

  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_Rename_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Rename_end");
    return (_dpDataChan->Rename(err, currentname, newname));
  }

  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->Rename(err, currentname, newname) == Sdm_EC_Success) {
      break;
    }
  }

  TNF_PROBE_0 (SdmDataPort_Rename_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Rename_end");
  return(err);
}

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
SdmErrorCode SdmDataPort::RemoveFromSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name			// name to be removed from the subscribed namespace
  )
{
  TNF_PROBE_0 (SdmDataPort_RemoveFromSubscribedNamespace_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RemoveFromSubscribedNamespace_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_RemoveFromSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RemoveFromSubscribedNamespace_end");
    return(err);
  }

  // If open, scan the open channel
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_RemoveFromSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RemoveFromSubscribedNamespace_end");
    return (_dpDataChan->RemoveFromSubscribedNamespace(err, name));
  }

  // Hard way - spin through the list
  assert (_dpAttached);
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->RemoveFromSubscribedNamespace(err, name) == Sdm_EC_Success) {
      break;
    }
  }

  TNF_PROBE_0 (SdmDataPort_RemoveFromSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_RemoveFromSubscribedNamespace_end");
  return(err);
}

#endif


// Remove a specific message header from a single message
//
SdmErrorCode SdmDataPort::RemoveMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be removed
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success)
    return(err);

  HANDLE_REDIRECT_INSERT(err, msgnum, RemoveMessageHeader(err, hdr, msgnum))

  return (err);
}

// Replace specific header completely with new value for a single message
//
SdmErrorCode SdmDataPort::ReplaceMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be replaced
  const SdmString& newvalue,		// new header value
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success)
    return(err);

  HANDLE_REDIRECT_INSERT(err, msgnum, ReplaceMessageHeader(err, hdr, newvalue, msgnum))

  return (err);
}

// Determine if a message has any addressess that a transport can handle
// and if so resolve (dealias) and produce a list of them - preparing
// the message for transmission
//
SdmErrorCode SdmDataPort::ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_ResolveAndExtractAddresses_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ResolveAndExtractAddresses_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_ResolveAndExtractAddresses_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ResolveAndExtractAddresses_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_ResolveAndExtractAddresses_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ResolveAndExtractAddresses_end");
  return(_dpDataChan->ResolveAndExtractAddresses(err, r_addressList, r_filenameList, dataPort, msgnum));
}

// Cause the complete state of the message store to be saved
// 
SdmErrorCode SdmDataPort::SaveMessageStoreState
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDataPort_SaveMessageStoreState_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SaveMessageStoreState_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SaveMessageStoreState_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SaveMessageStoreState_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_SaveMessageStoreState_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SaveMessageStoreState_end");
  return(_dpDataChan->SaveMessageStoreState(err));
}


// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
SdmErrorCode SdmDataPort::ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  )
{
  TNF_PROBE_0 (SdmDataPort_ScanNamespace_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanNamespace_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_ScanNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanNamespace_end");
    return(err);
  }

  if (r_names.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_ScanNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanNamespace_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  // Ok, you can do a "scan" with multiple channels attached, or a single
  // channel open. In the former case, spin through the attached channels
  // doing a scan and accumulating names, in the latter case do a single
  // scan of the open channel

  // If open, scan the open channel
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_ScanNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanNamespace_end");
    return (_dpDataChan->ScanNamespace(err, r_names, reference, pattern));
  }

  // Hard way - spin through the list
  assert (_dpAttached);
#if 0
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmIntStrL c_names;
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->ScanNamespace(err, c_names, reference, pattern) == Sdm_EC_Success) {
      int numNames = c_names.ElementCount();
      for (int j = 0; j < numNames; j++)
	r_names.AddElementToList(c_names[j]);
    }
  }
#else
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->ScanNamespace(err, r_names, reference, pattern) == Sdm_EC_Success) {
      break;
    }
  }
#endif
  TNF_PROBE_0 (SdmDataPort_ScanNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanNamespace_end");
  return(err = r_names.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode SdmDataPort::ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  )
{
  TNF_PROBE_0 (SdmDataPort_ScanSubscribedNamespace_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanSubscribedNamespace_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckAttachedOrOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_ScanSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanSubscribedNamespace_end");
    return(err);
  }

  if (r_names.ElementCount()) {		// passed in return value list must be empty
    TNF_PROBE_0 (SdmDataPort_ScanSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanSubscribedNamespace_end");
    return(err = Sdm_EC_ReturnArgumentListHasContents);
  }

  // If open, scan the open channel
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_ScanSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanSubscribedNamespace_end");
    return (_dpDataChan->ScanSubscribedNamespace(err, r_names, reference, pattern));
  }

#if 0
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmIntStrL c_names;
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->ScanSubscribedNamespace(err, c_names, reference, pattern) == Sdm_EC_Success) {
      int numNames = c_names.ElementCount();
      for (int j = 0; j < numNames; j++)
	r_names.AddElementToList(c_names[j]);
    }
  }
#else
  int numChans = _dpDataChanL.ElementCount();
  for (int i = 0; i < numChans; i++) {
    SdmDpDataChan* dc = _dpDataChanL[i];
    assert(dc);
    if (dc->ScanSubscribedNamespace(err, r_names, reference, pattern) == Sdm_EC_Success) {
      break;
    }
  }
#endif
  TNF_PROBE_0 (SdmDataPort_ScanSubscribedNamespace_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ScanSubscribedNamespace_end");
  return(err = r_names.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDataPort::SetMessageAttribute
  (
  SdmError& err,
  const SdmString& attribute,		// name of attribute to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageAttribute(err, attribute, attribute_value, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_end");
  return (err);
}


// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDataPort::SetMessageAttribute
  (
  SdmError& err,
  const SdmMessageAttributeAbstractFlags attribute,	// abstract attribute value to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageAttribute(err, attribute, attribute_value, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_SetMessageAttribute_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageAttribute_end");
  return (err);
}

#endif


// Set the complete contents for a specific component of a message
//
SdmErrorCode SdmDataPort::SetMessageContents
  (
  SdmError& err,
  const SdmContentBuffer& contents,	// new contents for component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageContents_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageContents_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageContents_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageContents(err, contents, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_SetMessageContents_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageContents_end");
  return (err);
}

// Set a set of flags for a single message
//
SdmErrorCode SdmDataPort::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageFlagValues(err, flagValue, aflags&  Sdm_MFA_ALL, msgnum))

  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
  return(err);
}

// Set a set of flags for a range of messages
//
SdmErrorCode SdmDataPort::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
  return(_dpDataChan->SetMessageFlagValues(err, flagValue, aflags&  Sdm_MFA_ALL, startmsgnum, endmsgnum));
}

// Set a set of flags for a list of messages
//
SdmErrorCode SdmDataPort::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_SetMessageFlagValues_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageFlagValues_end");
  return(_dpDataChan->SetMessageFlagValues(err, flagValue, aflags&  Sdm_MFA_ALL, msgnums));
}

// Set all headers for a single message, replacing all existing headers
//
SdmErrorCode SdmDataPort::SetMessageHeaders
  (
  SdmError& err,
  const SdmStrStrL& hdr,		// list of header name/values to set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageHeaders_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageHeaders_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageHeaders_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageHeaders(err, hdr, msgnum))

  TNF_PROBE_0 (SdmDataPort_SetMessageHeaders_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageHeaders_end");
  return (err);
}

// Set the message structure for a component of a message
//
SdmErrorCode SdmDataPort::SetMessageBodyStructure
  (
  SdmError& err,
  const SdmDpMessageStructure& msgstr,	// new message structure value for the specified component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDataPort_SetMessageBodyStructure_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageBodyStructure_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_SetMessageBodyStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageBodyStructure_end");
    return(err);
  }

  HANDLE_REDIRECT_INSERT(err, msgnum, SetMessageBodyStructure(err, msgstr, msgnum, mcmp))

  TNF_PROBE_0 (SdmDataPort_SetMessageBodyStructure_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_SetMessageBodyStructure_end");
  return (err);
}

// Submit a fully constructed message for transmission
//
SdmErrorCode SdmDataPort::Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL& customHeaders
  )
{
  TNF_PROBE_0 (SdmDataPort_Submit_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Submit_start");
  assert(DATAPORT_SIGNATURE_VALID);
  SdmRecursiveMutexEntry entry(_dpMutexData);	// assure single thread access to port object

  if (CheckOpen(err) != Sdm_EC_Success) {
    TNF_PROBE_0 (SdmDataPort_Submit_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Submit_end");
    return(err);
  }

  TNF_PROBE_0 (SdmDataPort_Submit_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_Submit_end");
  return(_dpDataChan->Submit(err, r_deliveryResponse, msgFormat, dataPort,
                             msgnum,customHeaders));
}

// --------------------------------------------------------------------------------
// Utility functions that are not part of the Data Port API and are private to
// this interface are included below.
// --------------------------------------------------------------------------------

// Shutdown a single redirected message
// Probably after a commit or discard is done

void SdmDataPort::ShutdownRedirectedMessage(SdmMessageNumber msgnum, SdmBoolean shutdownFlag)
{
  TNF_PROBE_0 (SdmDataPort_ShutdownRedirectedMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutdownRedirectedMessage_start");
  SdmError localError;

  int numMsgMaps = _dpMsgnumToChanL.ElementCount();
  assert(numMsgMaps);

  assert(_dpDataChan);
  for (int i = 0; i < numMsgMaps && _dpMsgnumToChanL[i]._mtcMsgnum != msgnum; i++)
    ;
  assert(i < numMsgMaps);
  DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
  assert(mtc->_mtcChan);
  if (shutdownFlag == Sdm_True)
    mtc->_mtcChan->ShutDown(localError);
  delete mtc->_mtcChan;
  _dpMsgnumToChanL.RemoveElement(i);
  TNF_PROBE_0 (SdmDataPort_ShutdownRedirectedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutdownRedirectedMessage_end");
}

// Shutdown all redirected messages - prelude to closing down this data port
//

void SdmDataPort::ShutdownAllRedirectedMessages()
{
  TNF_PROBE_0 (SdmDataPort_ShutdownRedirectedMessage_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutdownRedirectedMessage_start");
  SdmError localError;

  int numMsgMaps = _dpMsgnumToChanL.ElementCount();
  if (numMsgMaps) {
    assert(_dpDataChan);
    for (int i = 0; i < numMsgMaps; i++) {
      DpMsgnumToChan* mtc = &_dpMsgnumToChanL[i];
      assert(mtc->_mtcChan);
      mtc->_mtcChan->ShutDown(localError);
      delete mtc->_mtcChan;
    }
    _dpMsgnumToChanL.ClearAllElements();
  }
  TNF_PROBE_0 (SdmDataPort_ShutdownRedirectedMessage_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ShutdownRedirectedMessage_end");
}

// ReleaseOpenChannels - close, shutdown and release any channel adapters
// that may have been activated as a result of a previous Open() call

void SdmDataPort::ReleaseOpenChannels()
{
  TNF_PROBE_0 (SdmDataPort_ReleaseOpenChannels_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ReleaseOpenChannels_start");
  SdmError err;

  if (_dpOpen) {
    _dpDataChan->Close(err);
    err = Sdm_EC_Success;   // reset err.  if set the ShutDown call will assert.
    _dpDataChan->ShutDown(err);	// should probably fix this one in case close fails?
    _dpOpen = Sdm_False;
    delete _dpDataChan;
    _dpDataChan = 0;
  }

  assert(!_dpOpen);
  assert(!_dpDataChan);
  TNF_PROBE_0 (SdmDataPort_ReleaseOpenChannels_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ReleaseOpenChannels_end");
}

// ReleaseAttachedChannels - close, shutdown and release any channel adapters
// that may have been activated as a result of a previous Attach() call

void SdmDataPort::ReleaseAttachedChannels()
{
  TNF_PROBE_0 (SdmDataPort_ReleaseAttachedChannels_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ReleaseAttachedChannels_start");
  SdmError err;

  if (_dpAttached) {
    int numChans = _dpDataChanL.ElementCount();
    for (int i = 0; i < numChans; i++) {
      SdmDpDataChan* dc = _dpDataChanL[i];
      assert(dc);
      dc->Close(err);
      err = Sdm_EC_Success;   // reset err.  if set the ShutDown call will assert.
      dc->ShutDown(err);
      delete dc;
    }
    _dpAttached = Sdm_False;
    _dpDataChanL.ClearAllElements();
  }
  assert(!_dpDataChanL.ElementCount());
  TNF_PROBE_0 (SdmDataPort_ReleaseAttachedChannels_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_ReleaseAttachedChannels_end");
}

SdmErrorCode SdmDataPort::CheckAttachedOrOpen(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_CheckAttachedOrOpen_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttachedOrOpen_start");
  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_CheckAttachedOrOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttachedOrOpen_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (!_dpOpen && !_dpAttached) {
    TNF_PROBE_0 (SdmDataPort_CheckAttachedOrOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttachedOrOpen_end");
    return(err = Sdm_EC_Closed);
  }
  TNF_PROBE_0 (SdmDataPort_CheckAttachedOrOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttachedOrOpen_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDataPort::CheckAttached(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_CheckAttached_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttached_start");
  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_CheckAttached_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttached_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_CheckAttached_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttached_end");
    return(err = Sdm_EC_Open);
  }
  if (!_dpAttached) {
    TNF_PROBE_0 (SdmDataPort_CheckAttached_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttached_end");
    return(err = Sdm_EC_Closed);
  }
  assert(_dpDataChan);
  TNF_PROBE_0 (SdmDataPort_CheckAttached_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckAttached_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDataPort::CheckOpen(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_CheckOpen_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckOpen_start");
  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_CheckOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckOpen_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (_dpAttached) {
    TNF_PROBE_0 (SdmDataPort_CheckOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckOpen_end");
    return(err = Sdm_EC_Attached);
  }
  if (!_dpOpen) {
    TNF_PROBE_0 (SdmDataPort_CheckOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckOpen_end");
    return(err = Sdm_EC_Closed);
  }
  assert(_dpDataChan);
  TNF_PROBE_0 (SdmDataPort_CheckOpen_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckOpen_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDataPort::CheckStarted(SdmError& err)
{
  TNF_PROBE_0 (SdmDataPort_CheckStarted_start,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckStarted_start");
  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDataPort_CheckStarted_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckStarted_end");
    return(err = Sdm_EC_Shutdown);
  }
  TNF_PROBE_0 (SdmDataPort_CheckStarted_end,
      "dpapi SdmDataPort",
      "sunw%debug SdmDataPort_CheckStarted_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDataPort::MailStream(SdmError& err, void *&r_stream)
{
  return(_dpDataChan->MailStream(err, r_stream));
}

