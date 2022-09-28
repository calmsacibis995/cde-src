/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "C client" data channel subclass
// --> It implements the "c client channel adapter" that interfaces
// --> the c client API to the data port api

#pragma ident "@(#)DataChanCclient.cc	1.203 97/07/08 SMI"

// Include Files.
#include <string.h>
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChanCclient.hh>
#include <Utils/TnfProbe.hh>
#include <Utils/CclientUtility.hh>
#include <SDtMail/SimpleTuples.hh>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Utility.hh>
#include <assert.h>

// need to include external declaration for ctime_r
// because it doesn't get declared when REENTRANT is
// not defined.
extern "C" char *ctime_r(const time_t *, char *, int);

static const char *CHANNEL_NAME = "datachancclient";
static const int gkMinimumBufferLength = 256;
static const int INITIAL_HEADER_CACHE_SIZE = 15;	// number of header slots to preallocate in header arrays
static mutex_t gCclientLock = DEFAULTMUTEX;

int SdmDpDataChanCclient::_dcOneTimeFlag = 0;
SdmToken *SdmDpDataChanCclient::_dcRefToken;
SdmDpDataChanCclient* SdmDpDataChanCclient::_dcLastDataChanUsed = 0;

SdmDpDataChanCclient* SdmDpDataChanCclient::LastDataChanUsed()
{
  return(_dcLastDataChanUsed);
}

SdmDpDataChanCclient::SdmDpDataChanCclient(SdmDataPort* parentDataPort) :
  SdmDpDataChan(Sdm_CL_DataChannel, parentDataPort)
{
  _dcStream = NULL;
  _dcNmsgs = 0;
  _dcReadonly = Sdm_False;
  _dcRemote = Sdm_False;
  _dcDisconnected = Sdm_False;
  _dcDebug = Sdm_False;
  _dcScanCollector = NULL;
  _dcStatusCollector = NULL;
  _dcExpungeCollector = NULL;
  _dcSearchCollector = NULL;
  _dcMailboxConnectionReadOnly = Sdm_False;
  _dcServerConnectionBroken = Sdm_False;
  _dcMailboxChangedByOtherUser = Sdm_False;

#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockObject = new SdmLockUtility;
  _dcLockGrabIfOwnedByOther = Sdm_False;	// grab lock if owned by other
  _dcLockIgnoreIfOwnedByOther = Sdm_False;	// ignore if owned by other
  _dcLockNoSessionLocking = Sdm_False;		// do not do any session locking
#endif // defined(INCLUDE_SESSIONLOCKING)

}

SdmDpDataChanCclient::~SdmDpDataChanCclient() 
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = 0;    // no current data channel any more
#if defined(INCLUDE_SESSIONLOCKING)
  SdmLockUtility::DeleteObject(*_dcLockObject);
#endif // #if defined(INCLUDE_SESSIONLOCKING)
}

SdmErrorCode SdmDpDataChanCclient::StartUp(SdmError& err)
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_IsStarted)
    return (err = Sdm_EC_Success);

  if (!_dcOneTimeFlag) {
    // The following source file provides calls to `mail-link' to enable the
    // required c-client drivers.
#include <linkage.c>

    // setup the reference token used to filter out incompatible requests
    _dcRefToken = new SdmToken;		// create the reference token
    _dcRefToken->SetValue("servicechannel","cclient");
    _dcRefToken->SetValue("serviceclass","messagestore");
    _dcRefToken->SetValue("servicetype","imap");
    _dcRefToken->SetValue("servicetype","local");
    
    _dcOneTimeFlag++;
  }

 _IsStarted = Sdm_True;

 return (err = Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChanCclient::ShutDown(SdmError& err)
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = 0;	// no current data channel any more
  mail_stream_flush(_dcStream);
  return (err = Sdm_EC_Success);
}

// function SdmDataChanCclient factory : creates a SdmDpDataChanCclient object
//
SdmDpDataChan *SdmDpDataChanCclientFactory(SdmDataPort* parentDataPort)
{
  return (new SdmDpDataChanCclient(parentDataPort));
}

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode SdmDpDataChanCclient::AddToSubscribedNamespace
  (
  SdmError &err,
  const SdmString &name		// name to add to subscribed namespace
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;
  assert(_dcAttached || _dcOpen);

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  // Don't allow a "network" reference or pattern - this function
  // automatically prepends any necessary hostname prefix.
  if (name[0] == '{')
    return (err = Sdm_EC_Fail);

  SdmString full_name(name);

  // If remote, the reference argument must include the hostname.
  if (_dcRemote) {
    full_name = "{";
    full_name += _dcHostname;
    full_name += "}";
    full_name += name;
  }

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (!mail_subscribe(_dcStream, (char*)(const char*)full_name)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

#endif


// Append one message by contents to the message store
//
SdmErrorCode SdmDpDataChanCclient::AppendMessage
  (
  SdmError &err,
  const SdmString &contents,	// contents of the message to append
  const SdmString &date,
  const SdmString &unixFrom,
  SdmMessageFlagAbstractFlags flags 
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  STRING str;
  char *dst = NULL;
  unsigned long dstlen = 0;

  // Adjust the contents to skip past a "From " line if present
  const char *contentsStart = SdmCclientUtility::SkipPastFromLine((const char*)contents);
  unsigned long contentsLength = contents.Length() - (contentsStart - (const char*)contents);

  // If this is a remote mailstore, the appended string must be in RFC822
  // format with each line delimited by CRLF.  If this is a local mailstore,
  // RFC822 format is not required since the solaris driver strips any CR
  // characters.
  if (_dcRemote) {
    if (SdmCclientUtility::strcrlfcpy(dst, dstlen, (char*) contentsStart, contentsLength, 0) != Sdm_True)
      return (err = Sdm_EC_NoMemory);

    INIT(&str, mail_string, dst, dstlen);
  }
  else {
    INIT(&str, mail_string, (char*)contentsStart, contentsLength);
  }

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  // get the unix from line.  If none is passed in, try to get it from the contents
  // of the message.
  SdmString fromLine;
  if (unixFrom.Length() > 0) {
      fromLine = unixFrom;
  } else {
      const char* contentsStr = (const char*) contents;
      // hack if the from line is "From user@host ", this means that we were
      // unable to create the From line for the message and a dummy from line
      // was genereated for this message.  In this case, we don't want to pass
      // the dummy from line down to the c-client because it is not valid.
      if (contentsStart != contentsStr &&
             strncmp(contentsStr, "From ", 5) == 0 &&
             strncmp(contentsStr, "From user@host ", 15) != 0)
      {
        // fprintf(stderr, "getting unix from line from the contents itself.\n");
        // Skip past the "From " part of the from line and don't include
        // any trailing new line characters.
        const char* fromp = contentsStart-1;
        int newlines = 0;
        while (*fromp == '\n' && fromp >= contentsStr) {
          newlines++;
          fromp--;
        }
        fromLine = contents(5, contentsStart-contentsStr-5-newlines);
      }
  }
  
  SdmString flagsString;
  _GetStringFromFlags(flagsString, flags);
  
  char *datep = (char*)(const char*)date;
  char *unixFromp = (char*)(const char*)fromLine;
  char *flagsp = (char*)(const char*)flagsString;
  long status = mail_append_full(_dcStream, _dcStream->mailbox, 
                                 flagsp && *flagsp ? flagsp : NIL, 
                                 datep && *datep ? datep : NIL, 
                                 unixFromp && *unixFromp ? unixFromp : NIL, 
                                 &str);
                                 
/*
   fprintf(stderr, "unix from line is: %s\n",      
       unixFromp && *unixFromp ? unixFromp : "null");                   

   fprintf(stderr, "flags string is: %s\n",      
       flagsp && *flagsp ? flagsp : "null"); 
*/

  if (dst)
    free(dst);

  if (!status) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAppendMessageToMailbox));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode SdmDpDataChanCclient::Attach
  (
  SdmError &err,
  const SdmToken &tk		// token describing object to attach to
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  // If already open, this is an error
  if (_dcOpen)
    return (err = Sdm_EC_Fail);

  // Clean up from any previous attempts (e.g. connects)
  if (_dcAttached) {
    if (Close(err))
      return (err);
  }

  // Reset all object variables
  _dcUsername = "";	// no user name associated with this open yet
  _dcHostname = "";	// no host name associated with this open yet
  _dcPassword = "";	// no password associated with this open yet
  _dcReadonly = Sdm_False;	// not read only
  _dcAttached = Sdm_False;	// not attached
  _dcRemote = Sdm_False;	// not remote yet
  _dcDebug = Sdm_False;		// not debugging
  _dcDisconnected = Sdm_False;	// not disconnected

  // See if the token describing the object to open loosely matches
  // this data channel reference token for opening - if it does not
  // it means the object to open is incompatible with this data channel
  if (tk.CompareLoose(*_dcRefToken) != Sdm_True)
    return (err = Sdm_EC_Fail);

  SdmString dummy;
  long open_flags = OP_NOUSERSIGS|OP_HALFOPEN;	// halfopen == attach...

  // Determine if local/imap options are present
  SdmString serviceType;
  SdmBoolean serviceLocal = tk.CheckValue("servicetype", "local");
  SdmBoolean serviceImap = tk.CheckValue("servicetype", "imap");

  if (!tk.FetchValue(serviceType, "servicetype") || (!serviceLocal && !serviceImap))
    return (err = Sdm_EC_ServiceTypeRequired);

  // Ok, this token is acceptable to attempt attaching this channel adapter.
  SdmString storename;

  if (!serviceLocal && serviceImap) {
    if (!tk.FetchValue(_dcHostname, "serviceoption", "hostname"))
      return (err = Sdm_EC_HostnameRequired);

    if (!tk.FetchValue(_dcUsername, "serviceoption", "username"))
      return (err = Sdm_EC_UsernameRequired);

    if (!tk.FetchValue(_dcPassword, "serviceoption", "password"))
      return (err = Sdm_EC_PasswordRequired);

    // If a hostname is provided, then generate a message store name
    // that specifies just the host, for example: {grimsby}
    // The attach will be achieved by opening a connection to the host
    // in "half-open" mode.
    storename = "{";
    storename += _dcHostname;
    storename += "}";

    _dcRemote = Sdm_True;
  }
  else {
    _dcRemote = Sdm_False;
  }

  if (tk.FetchValue(dummy, "serviceoption", "debug")) {
    _dcDebug = Sdm_True;
    open_flags |= OP_DEBUG;
  }

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  _dcStream = mail_stream_create(this, NULL, NULL,
                                 _EnableGroupPrivileges, _DisableGroupPrivileges);

  if (_dcStream == NULL) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAttachToService));
  }

  // Set the max login attempts to one.
  mail_stream_setNIL(_dcStream);
  mail_parameters(_dcStream, SET_MAXLOGINTRIALS, (void*)1);
  mail_stream_unsetNIL(_dcStream);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (_dcRemote) {
    _dcStream = mail_open(_dcStream, (char*)(const char*)storename, open_flags);

    if (!(_dcStream->stream_status & S_OPENED)) {
      if (_dcCclientError != Sdm_EC_Success) {
        err = _dcCclientError;
        _dcCclientError.Reset();  // reset this now that we've saved it in err.    
      } 
      else if (_dcHostname.Length() > 0) {
        char *host = new char[_dcHostname.Length() + 3];
        sprintf(host, "{%s}", (const char*)_dcHostname);
        if (mail_echo(host) == NULL) {
          err.AddMajorErrorCodeAndMessage(Sdm_EC_CannotConnectToIMAPServer, (const char*)_dcHostname);
        }
        delete host;
      }
      if (_dcCclientMessages.Length()) {
	err.AddMinorErrorCodeAndMessage(Sdm_EC_CCL_ServerErrorMessage, (const char *)_dcCclientMessages);
	_dcCclientMessages = "";
      }
      err.AddMajorErrorCode(Sdm_EC_CannotAttachToService);
      mail_stream_flush(_dcStream);
      _dcStream = NULL;
      return err;
    }
  }

  assert(_dcStream->userdata == this);

  _dcAttached = Sdm_True;

  return (err = Sdm_EC_Success);
}

// Cancel any operations pending on the data port
//
SdmErrorCode SdmDpDataChanCclient::CancelPendingOperations
  (
  SdmError &err
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);

  // This often causes hanging during the close of a mailbox - there appears
  // to be a race condition in the c-client's tcp driver.
  // TODO: resolve this (it's needed for advanced).
#if 0
  mail_increment_send_urgent(_dcStream);
#endif

  return (err = Sdm_EC_Success);
}

// Close the current connection down
//
SdmErrorCode SdmDpDataChanCclient::Close
  (
  SdmError &err
  )
{
  // we don't want to get the mutex lock unless we
  // are actually handling the close.  Close will
  // be called by _HandleInvalidMailboxState 
  // when handling a broken server connection.
  // In this case, the mutex is already obtained by 
  // the initial cclient call.  _HandleInvalidMailboxState 
  // calls _Close directly before calling DataPort::Close.
  // If we try to get the mutex lock without checking 
  // if we are already closed, we will dead lock.  
  // 
  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  if (!_dcOpen && !_dcAttached)
    return (err = Sdm_EC_Closed);
    
  // The code below will ONLY be called if the data channel
  // is not already closed.  If we are being called by
  // _HandleInvalidMailboxState, the data channel is 
  // already closed so we will not get the mutex lock 
  // again or end up calling _HandleInvalidMailboxState 
  // again.
  //
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_Close(err)) 
    return (err);

  return (err = Sdm_EC_Success);
}

// Commit any pending changes to a message
//
SdmErrorCode SdmDpDataChanCclient::CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messageRewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  return (err = Sdm_EC_Success);	// commit is always successful: nothing to do
}

// Check for new messages in the message store
//
SdmErrorCode SdmDpDataChanCclient::CheckForNewMessages
  (
  SdmError &err,
  SdmMessageNumber &r_nummessages	// returned number of "new" messages in the message store
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);
  
  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  // mail_check results in a call to mail_exists which updates
  // _dcStream->nmsgs to reflect the new number of messages in the mailbox.
  mail_check(_dcStream);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAccessNewMail));
  } 
  _HandleMailboxConnectionReadOnly();

  r_nummessages = (_dcStream->nmsgs - _dcNmsgs);
  // SdmMessageNumber is signed, so the following check is OK
  // A negative number of new messages indicates a problem.
  // If remote, then force a close on the mailbox, otherwise return failure.
  if (r_nummessages < 0) {
    if (_dcRemote) {
      _dcServerConnectionBroken = Sdm_True;	// force _HandleErrorExitConditions to close
    } else {
      _dcMailboxChangedByOtherUser = Sdm_True;	// force _HandleErrorExitConditions to close
    }    
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAccessNewMail));
  }

  _HandleMailboxConnectionReadOnly();

  // Update internal cache of number of messages in the store
  _dcNmsgs = _dcStream->nmsgs;

  return (err = Sdm_EC_Success);
}
// Check for new messages in the message store
// Like CheckForNewMessages but does not grab the mutex, and does not update
// the _dcNmsgs internal value - meant to be called internally 
SdmErrorCode SdmDpDataChanCclient::_ReturnNumberOfNewMessages
  (
  SdmError &err,
  SdmMessageNumber &r_nummessages	// returned number of "new" messages in the message store
  )
{
  assert(!_dcAttached);
  assert(_dcStream);
  
  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  // mail_check results in a call to mail_exists which updates
  // _dcStream->nmsgs to reflect the new number of messages in the mailbox.
  mail_check(_dcStream);
  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAccessNewMail));
  } 
  _HandleMailboxConnectionReadOnly();

  r_nummessages = (_dcStream->nmsgs - _dcNmsgs);
  // SdmMessageNumber is signed, so the following check is OK
  // A negative number of new messages indicates a problem.
  // If remote, then force a close on the mailbox, otherwise return failure.
  if (r_nummessages < 0) {
    if (_dcRemote) {
      _dcServerConnectionBroken = Sdm_True;	// force _HandleErrorExitConditions to close
    }
    else {
      _dcMailboxChangedByOtherUser = Sdm_True;	// force _HandleErrorExitConditions to close
    }    
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotAccessNewMail));
  }

  return (err = Sdm_EC_Success);
}

// Simple sort function passed to `qsort'
static int compareLongs(const void *a, const void *b)
{
  long *i = (long *)a, *j = (long *)b;

  if (*i > *j) return (1);
  if (*i < *j) return (-1);
  return (0);
}

// Create a string sequence of longs separated by "," or ":" from an array
// of n longs, where n, n+1, ..,n+m becomes n:n+m, eg,
// 11,12,13,17,19,44,45,99 --> 11:13,17,19,44:45,99
static char *
_GenerateMessageSequence(const SdmMessageNumberL &msgnums)
{
  char *seq;
  char number[64];
  int i, j, numElts, len;

  numElts = msgnums.ElementCount();

  if (numElts == 0)
    return NULL;

  // Copy the elements into an array of longs (which may be used with qsort)
  long *sortedList = new long [numElts];
  for (i = 0; i < numElts; i++)
    sortedList[i] = msgnums[i];

  // Make the sequence monotonically increasing
  qsort((char*)sortedList, numElts, sizeof(long), compareLongs);

  // Calculate the sum of the lengths of the decimal representation of all
  // the numbers in the list.
  for (i = 0, len = 0; i < numElts; i++) {
    ++len;      // Each number must comprise at least one digit
    int unit = 10;
    while (sortedList[i] >= unit) {
      unit *= 10;
      ++len;
    }
  }
  len += numElts;                       // n-1 potential separators
  seq = new char [len];                 // Allocate the string

  // OK, make the sequence
  sprintf(seq, "%ld", sortedList[0]);   // First number

  for (i = 1; i < numElts; i++) {
    // Break on first non-consecutive successor
    for (j = i; j < numElts; j++) {
      // Don't allow duplicates
      if (sortedList[j - 1] == sortedList[j]) {
        delete sortedList;
        delete seq;
        return NULL;
      }
      if ((sortedList[j - 1] + 1) != sortedList[j])
        break;
    }
    if (j == i)                 // Not consecutive
      sprintf(number, ",%ld", sortedList[i]);
    else {
      sprintf(number, ":%ld", sortedList[j-1]);
      i = j - 1;                // i gets incremented to j in loop.
    }
    strcat(seq, number);
  }
  delete [] sortedList;
  return seq;
}

// Move a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::_CopyOrMoveMessages
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const char *sequence,
  long moveFlag
  )
{
  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);

  SdmMutexEntry entry(gCclientLock);

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  SdmBoolean dstLocal = token.CheckValue("servicetype", "local");
  SdmBoolean dstRemote = token.CheckValue("servicetype", "imap");

  // The token must specify a servicetype of local or imap, but not both
  if (!(dstLocal ^ dstRemote))
    return (err = Sdm_EC_ServiceTypeRequired);

  if ((_dcRemote && dstLocal) || (!_dcRemote && dstRemote))
    return (err = Sdm_EC_IncompatibleMessageStoreTypes);

  if (_dcRemote) {
    SdmString dstHost;

    if (!token.FetchValue(dstHost, "serviceoption", "hostname"))
      return (err = Sdm_EC_HostnameRequired);

    if (::strcasecmp(_dcHostname, dstHost) != 0)
      return (err = Sdm_EC_IncompatibleMessageStoreTypes);
  }
  // If the above tests have passed, then the servicetype in the token
  // matches the servicetype of this data channel instance (i.e. both are
  // local or both are imap).  Additionally, if they are both imap, then the
  // hostname in the token also matches the host this data channel is
  // connected to.
  SdmString mailBox;
  if (!token.FetchValue(mailBox, "serviceoption", "messagestorename"))
    return (err = Sdm_EC_MessageStoreNameRequired);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (!mail_copy_full(_dcStream, (char*)sequence, (char*)(const char*)mailBox, moveFlag)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotCopyOrMoveMessage));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

// Copy a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::CopyMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to copy message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[MAILTMPLEN];
  sprintf(sequence, "%d", msgnum);

  return (_CopyOrMoveMessages(err, token, sequence, NIL));
}

// Copy a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[MAILTMPLEN];
  if (startmsgnum == endmsgnum)
    sprintf(sequence, "%d", startmsgnum);
  else
    sprintf(sequence, "%d:%d", startmsgnum, endmsgnum);

  return (_CopyOrMoveMessages(err, token, sequence, NIL));
}

// Copy a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  char *sequence;
  if ((sequence = _GenerateMessageSequence(msgnums)) == NULL)
    return (err = Sdm_EC_BadMessageSequence);

  _CopyOrMoveMessages(err, token, sequence, NIL);
  delete sequence;
  return err;
}

// Create a new message store from a token stream
//
SdmErrorCode SdmDpDataChanCclient::Create
  (
  SdmError &err,
  const SdmString& name			// name of object to create
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcAttached || _dcOpen);
  assert(_dcStream);

  SdmString final_storename;

  if (_dcRemote) {
    assert(_dcHostname.Length());

    final_storename = "{";
    final_storename += _dcHostname;
    final_storename += "}";
    final_storename += name;
  }
  else {
    if (SdmUtility::ExpandAndMakeAbsolutePath(err, final_storename, name, Sdm_False) != Sdm_EC_Success)
      return(err);
  }

  char *mailbox = (char*) (const char*)final_storename;

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (!mail_create(_dcStream, mailbox)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotCreateMailbox));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

// Delete the currently open object
//
SdmErrorCode SdmDpDataChanCclient::Delete
  (
  SdmError &err,
  const SdmString& name			// name of the message store to delete
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcAttached || _dcOpen);
  assert(_dcStream);

  SdmString final_storename;

  if (_dcRemote) {
    assert(_dcHostname.Length());

    final_storename = "{";
    final_storename += _dcHostname;
    final_storename += "}";
    final_storename += name;
  }
  else {
    if (SdmUtility::ExpandAndMakeAbsolutePath(err, final_storename, name, Sdm_True) != Sdm_EC_Success)
      return(err);
  }

  char *mailbox = (char*) (const char*)final_storename;

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (!mail_delete(_dcStream, mailbox)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotDeleteMailbox));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

// Discard any pending changes to a message
//
SdmErrorCode SdmDpDataChanCclient::DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  return (err = Sdm_EC_Success);	// discard is always successful: nothing to do
}

// Disconnect the currently open object from its server
//
SdmErrorCode SdmDpDataChanCclient::Disconnect
  (
  SdmError &err
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);

  // Can only disconnect if already remote
  if (!_dcRemote)
    return (err = Sdm_EC_Fail);

  if (_dcDisconnected)
    return (err = Sdm_EC_AlreadyDisconnected);


#if 0
  assert (!_dcStream->record);

  mail_disconnect(_dcStream);
  // Disable the cclient's "record" driver.
  // mail_enable_recording(_dcStream, ".simc");

  // Open a "recording" stream
  MAILSTREAM *recording = mail_open(_dcStream, _dcStream->mailbox,
                                    (_dcDebug ? OP_DEBUG : 0) | OP_RECORDING | OP_NOUSERSIGS,
                                    (void *)&_dcStream->userdata,
                                    _dcStream->set_ttlock,
                                    _dcStream->clear_ttlock,
                                    _dcStream->set_group_access,
                                    _dcStream->clear_group_access);

  // This is rather misleading - the idea is to reenable the cclient's
  // "imap4" driver and disable the "record" driver so that other mailboxes
  // may be opened in a "connected" state.
  mail_disable_recording(_dcStream);

  return (err = recording ? Sdm_EC_Success : Sdm_EC_Fail);
#endif

  return (err = Sdm_EC_Fail);

#if 0
  // We should introduce a new method to do this.
  if (save_cache)
    cache_to_disk(recording == NULL);
#endif
}

// Expunge deleted messages from the currently open message store
//
SdmErrorCode SdmDpDataChanCclient::ExpungeDeletedMessages
  (
  SdmError &err,
  SdmMessageNumberL &r_msgnums	// augmented list of message numbers of messages deleted
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);
  assert(r_msgnums.ElementCount() == 0);

  if (_dcReadonly)
    return(err = Sdm_EC_Readonly);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  _dcExpungeCollector = &r_msgnums;
  mail_expunge(_dcStream);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  } 

  int num_expunged = r_msgnums.ElementCount();

  if (num_expunged > 1) {
    // During expunge, decrementing of msgnums for subsequent messages
    // happens immediately.  Therefore, if the IMAP server implements a
    // "lower-higher" algorithm (see RFC1730), if three consecutive messages
    // starting at msgno 5 are expunged, mm_expunged is called with a msgno
    // of 5 three times.  If the server uses a "higher-lower" algorithm,
    // mm_expunged is called with msgnums 7, 6 and 5.

    // If the returned expunged msgnums are in ascending order, we assume
    // the "lower-higher" algorithm and adjust the numbers to reflect the
    // deleted msgnums relative to the original numbers before the expunge.
    if (r_msgnums[1] >= r_msgnums[0]) {
      for (int i = 0; i < num_expunged; i++) {
        r_msgnums[i] += i;
      }
    }
  }
  else if (num_expunged == 0) {
    // No messages were expunged - if any text was returned from the IMAP server
    // treat this as an error
    if (_dcCclientMessages.Length())
      return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  // Update the internal message count
  _dcNmsgs -= num_expunged;
  assert((signed)_dcNmsgs >= 0);
  assert(_dcStream->nmsgs >= _dcNmsgs);

  _dcExpungeCollector = NULL;
  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageAttributes
  (
  SdmError &err,
  SdmStrStrL &r_attribute,		// augmented list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_attribute.ElementCount() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;
  if (cmcmp->_bodyPart) {
    for (PARAMETER *param = cmcmp->_bodyPart->parameter; param != NULL; param = param->next) {
      r_attribute(-1).SetBothStrings(param->attribute, param->value);
    }
  }
  else {
    return (err = Sdm_EC_Fail);
  }

  return (err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageAttribute
  (
  SdmError &err,
  SdmString &r_attribute,		// augmented attribute value fetched
  const SdmString &attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_attribute.Length() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;

  if (cmcmp->_bodyPart) {
    for (PARAMETER *param = cmcmp->_bodyPart->parameter; param != NULL; param = param->next) {
      if (::strcasecmp(attribute, param->attribute) == 0) {
        r_attribute = param->value;
        return (err = Sdm_EC_Success);
      }
    }
  }
  else {
    return (err = Sdm_EC_Fail);
  }

  return (err = Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageAttributes
  (
  SdmError &err,
  SdmStrStrL &r_attribute,		// augmented list attributes values fetched
  const SdmStringL &attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_attribute.ElementCount() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;

  int numEntries = attributes.ElementCount();

  if (cmcmp->_bodyPart) {
    for (PARAMETER *param = cmcmp->_bodyPart->parameter; param != NULL; param = param->next) {
      for (int i = 0; i < numEntries; i++) {
        if (::strcasecmp((const char*)attributes[i], param->attribute) == 0) {
          r_attribute(-1).SetBothStrings(param->attribute, param->value);
          return (err = Sdm_EC_Success);
        }
      }
    }
  }
  else {
    return (err = Sdm_EC_Fail);
  }

  return (err = Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageAttributes
  (
  SdmError &err,
  SdmStrStrL &r_attribute,		// augmented list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_attribute.ElementCount() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;

  if (cmcmp->_bodyPart) {
    for (SdmAbstractAttributeMap *aamp = _sdmAbstractAttributes; aamp->flag; aamp++) {
      if (attributes & aamp->flag) {
        assert(aamp->realName);
        for (PARAMETER *param = cmcmp->_bodyPart->parameter; param != NULL; param = param->next) {
          if (::strcasecmp(aamp->realName, param->attribute) == 0) {
            r_attribute(-1).SetBothStrings(param->attribute, param->value);
          }
        }
      }
    }
  }
  else {
    return (err = Sdm_EC_Fail);
  }

  return (err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif


SdmErrorCode SdmDpDataChanCclient::GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;

  const BODY *body = cmcmp->_bodyPart;

  if (body == NULL)
    return (err = Sdm_EC_Fail);

  r_cached = ((body->type != TYPEMULTIPART) &&
              (!_dcRemote || body->contents.text)) ? Sdm_True : Sdm_False;

  return (err = Sdm_EC_Success);
}

// Generate a hidden unix "From " line, for example:
// From jsc  Mon Aug  5 16:45:29 1996
//
// CAUTION: After this call, data may be cached from the c-client. 
// You must issue this call before you return:
//	  if (!_dcRemote) mail_free_cached_buffers_msg(_dcStream, msgnum);
// C'est bien moche indeed...
//
SdmErrorCode SdmDpDataChanCclient::_GenerateUnixFromLine
  (
  SdmError &err,
  SdmString &r_fromLine,
  SdmMessageNumber msgnum
  )
{
  // Fetch the SHORTINFO (which also populates MESSAGECACHE entry with the
  // corresponding delivery date fields.
  SHORTINFO sinfo;
  if (mail_fetchshort_full(_dcStream, msgnum, &sinfo, 0) != T) {
    if (!_dcRemote) {
      mail_free_cached_buffers_msg(_dcStream, msgnum);
    }
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  r_fromLine = "From ";

  // Only reconstruct a "From " line if all the constituent parts are
  // available in the SHORTINFO.
  if (sinfo.mailbox && *sinfo.mailbox &&
      sinfo.host && *sinfo.host &&
      sinfo.date && *sinfo.date) {
    char date[SMALLBUFLEN];
    r_fromLine += sinfo.mailbox;
    r_fromLine += "@";
    r_fromLine += sinfo.host;
    r_fromLine += "  ";
    r_fromLine += mail_cdate(date, _dcStream->cache.s[msgnum-1]);
  }
  else {                        // Invent a "From " line
    time_t tn = time(0);
    char buf[64];

    r_fromLine += "user@host  ";
    r_fromLine += ctime_r(&tn, buf, 64);
  }

  return (err = Sdm_EC_Success);
}

// Return complete contents for a specific component of a message.  The
// processedcomponent contenttype is not permitted, since it requires a
// component argument.  See the following function for obtaining processed
// body part contents.
//
SdmErrorCode SdmDpDataChanCclient::GetMessageContents
  (
  SdmError &err,
  SdmContentBuffer &r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum         // message number to get contents of
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_contents.Length() == 0);
  
  SdmString str_contents;

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char *s;
  unsigned long len;

  switch (contenttype) {
  case DPCTTYPE_processedcomponent:
    return (err = Sdm_EC_Fail);
    break;
  case DPCTTYPE_rawfull:
    _GenerateUnixFromLine(err, str_contents, msgnum);
    s = mail_fetchheader_full(_dcStream, msgnum, NIL, &len, FT_INTERNAL);
    str_contents.Append(s, len);
    s = mail_fetchtext_full(_dcStream, msgnum, &len, FT_INTERNAL | FT_PEEK);
    str_contents.Append(s, len);
   break;
  case DPCTTYPE_rawheader:
    _GenerateUnixFromLine(err, str_contents, msgnum);
    s = mail_fetchheader_full(_dcStream, msgnum, NIL, &len, FT_INTERNAL);
    str_contents.Append(s, len);
    break;
  case DPCTTYPE_rawbody:
    s = mail_fetchtext_full(_dcStream, msgnum, &len, FT_INTERNAL);
    str_contents.Append(s, len);
    break;
  default:
    return (err = Sdm_EC_Fail);
  }

  if (_dcRemote)
    str_contents.StripCharacter(CR);
  else
    mail_free_cached_buffers_msg(_dcStream, msgnum);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True) || (str_contents.Length()==0)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  } 

  _HandleMailboxConnectionReadOnly();

  assert(str_contents.Length());
  r_contents = str_contents;
  return (err = Sdm_EC_Success);
}

// Return complete contents for a specific component of a message.  If any
// contenttype other than processedcomponent is requested, this function
// currently returns an error.  For the "raw" contenttypes, see the above
// function which doesn't take a component argument.
//
SdmErrorCode SdmDpDataChanCclient::GetMessageContents
  (
  SdmError &err,
  SdmContentBuffer &r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_contents.Length() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  if (contenttype != DPCTTYPE_processedcomponent)
    return (err = Sdm_EC_Fail);

  const SdmDpCclientMessageComponent *cmcmp =
    (const SdmDpCclientMessageComponent*)&mcmp;

  // Return an error if the component doesn't specify a section number
  const char *section = (const char*)cmcmp->_section;
  if (section == NULL || *section == '\0')
    return (err = Sdm_EC_Fail);

  unsigned long len;
  char *s = mail_fetchbody_full(_dcStream, msgnum,
                                (char*)section, &len, FT_INTERNAL);

  // Return error if no string is returned
  if (s == NULL) {
    if (!_dcRemote) {
      mail_free_cached_buffers_msg(_dcStream, msgnum);
    }
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }
  else {
    _HandleMailboxConnectionReadOnly();

    err = Sdm_EC_Success;
    SdmString strContents;
    const BODY *body = cmcmp->_bodyPart;

    // Add a unix "From " line if the MIME type is message/rfc822
    if (body->type == TYPEMESSAGE && (strcasecmp(body->subtype, "rfc822") == 0)) {
      const char *p = SdmCclientUtility::SkipPastFromLine(s);
      if (s == p)       // If there's no "From " line already...
        _GenerateUnixFromLine(err, strContents, msgnum);
    }

    strContents.Append(s, len);

    if (_dcRemote)
      strContents.StripCharacter(CR);
    else
      mail_free_cached_buffers_msg(_dcStream, msgnum);

    r_contents = strContents;

    // Add encoding to content buffer for body
    if (body && body->encoding == ENCBASE64) {
      r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_Base64);
    }
    else if (body && body->encoding == ENCQUOTEDPRINTABLE) {
      r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_QuotedPrintable);
    }
    else if (body->encoding <= ENCMAX && body_encodings[body->encoding]) {
      // Handle the cases where mailtool has send out a message in "mime" format
      // that has an invalid content-transfer-encoding field as opposed to using
      // a "conversions=x-" content-type header parameter.
      if (::strcasecmp(body_encodings[body->encoding], "uuencode")==0) {
	r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_UUEncoded);      
      }
      else if (::strcasecmp(body_encodings[body->encoding], "compress, uuencode")==0) {
	r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_UUEncoded);      
	r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_Compressed);
      }
      else if ((::strcasecmp(body_encodings[body->encoding], "x-sun-compress") == 0)
	       || (::strcasecmp(body_encodings[body->encoding], "compress") == 0)) {
	r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_Compressed);
      }
    }
    // Handle explicit conversions that may have come from a sun v3 message
    // that has been translated by the c-client into MIME format
    {
      char *conversions = NULL;
      for (PARAMETER *param = body->parameter; param; param = param->next) {
        if (::strcasecmp("conversions", param->attribute) == 0) {
          conversions = ucase(param->value);
          break;
        }
      }
      // Note: uuencode and compress encodings are not mutually exclusive,
      // and the decoding order is crucial.  In the outgoing message, any
      // uuencoding must have been applied *after* compression; otherwise
      // binary data would have been transmitted.  Therefore, when decoding,
      // uudecoding must be done before uncompression.
      if (conversions) {
        if (::strstr(conversions, "X-UUENCODE"))
          r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_UUEncoded);

        if (::strstr(conversions, "X-COMPRESS") ||
            (body->subtype && (::strcasecmp(body->subtype, "X-SUN-COMPRESS") == 0)))
          r_contents.AddEncodeType(err, SdmContentBuffer::Sdm_CBE_Compressed);
      }
    }

    // I18N 
    // Set the charset on the content buffer
    if (cmcmp->_bodyPart) {
      for (PARAMETER *param = cmcmp->_bodyPart->parameter; param != NULL; param = param->next) {
        if (::strcasecmp("charset", param->attribute) == 0) {
	  SdmString charset_value (param->value);
          r_contents.SetDataTypeCharSet(err, charset_value);
          break;
        }
      }
    }
    else {
      return (err = Sdm_EC_Fail);
    }
  }

  return err;
}


#define SET_ABSTRACT_FLAG_BITMASK(msgnum, aflags) \
  if (mail_elt(_dcStream, (msgnum))->deleted)   (aflags) |= Sdm_MFA_Deleted; \
  if (mail_elt(_dcStream, (msgnum))->answered)  (aflags) |= Sdm_MFA_Answered; \
  if (mail_elt(_dcStream, (msgnum))->flagged)   (aflags) |= Sdm_MFA_Flagged; \
  if (mail_elt(_dcStream, (msgnum))->recent)    (aflags) |= Sdm_MFA_Recent; \
  if (mail_elt(_dcStream, (msgnum))->seen)      (aflags) |= Sdm_MFA_Seen; \
  if (mail_elt(_dcStream, (msgnum))->draft)     (aflags) |= Sdm_MFA_Draft; \

// Return flags for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageFlags
  (
  SdmError &err,
  SdmMessageFlagAbstractFlags &r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // If connected remotely and the SHORTINFO isn't present, then fetch the
  // flags.  (The presence of SHORTINFO implies the flags have been fetched.)
  if (_dcRemote && !mail_sinfo_present(_dcStream, msgnum, 0)) {
    char sequence[32];

    sprintf(sequence, "%d", msgnum);
    mail_fetchflags_full(_dcStream, sequence, 0);
    
    if (_dcServerConnectionBroken == Sdm_True) {
      return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
    } 
  }

  _HandleMailboxConnectionReadOnly();

  r_aflags = 0;				// reset returned flags bits first before setting
  SET_ABSTRACT_FLAG_BITMASK(msgnum, r_aflags);

  return (err = Sdm_EC_Success);
}

// Return a list of flags for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageFlags
  (
  SdmError &err,
  SdmMessageFlagAbstractFlagsL &r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert (r_aflags.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // If local, then flags will always be present in the c-client's
  // MESSAGECACHE.  If remote, determine the sequence of messages which have
  // no SHORTINFO, and issue a single IMAP request to fetch the flags for
  // these messages.  Note that if the SHORTINFO has already been fetched
  // then the c-client's MESSAGECACHE will contain the flags for the
  // message, and also the flags will be current, since the MESSAGECACHE
  // flags are updated with every STORE {+-}FLAGS command.
  if (_dcRemote) {
    SdmMessageNumberL fetchList;

    for (int i = startmsgnum; i <= endmsgnum; i++) {
      if (!mail_sinfo_present(_dcStream, i, 0)) {
        fetchList(-1) = i;
      }
    }
    if (fetchList.ElementCount()) {
      char *sequence;

      if ((sequence = _GenerateMessageSequence(fetchList)) == NULL)
        return (err = Sdm_EC_BadMessageSequence);
        
      mail_fetchflags_full(_dcStream, sequence, 0);
      delete sequence;

      if (_dcServerConnectionBroken == Sdm_True) {
        return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
      } 

      _HandleMailboxConnectionReadOnly();
    }
  }

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmMessageFlagAbstractFlags aflags = 0;
    SET_ABSTRACT_FLAG_BITMASK(i, aflags);
    r_aflags.AddElementToList(aflags);
  }

  return (err = Sdm_EC_Success);
}

// Return a list of flags for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageFlags
  (
  SdmError &err,
  SdmMessageFlagAbstractFlagsL &r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL &msgnums	// list of messages to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert (r_aflags.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int len = msgnums.ElementCount();

  // If local, then flags will always be present in the c-client's
  // MESSAGECACHE.  If remote, determine the sequence of messages which have
  // no SHORTINFO, and issue a single IMAP request to fetch the flags for
  // these messages.  Note that if the SHORTINFO has already been fetched
  // then the c-client's MESSAGECACHE will contain the flags for the
  // message, and also the flags will be current, since the MESSAGECACHE
  // flags are updated with every STORE {+-}FLAGS command.
  if (_dcRemote) {
    SdmMessageNumberL fetchList;
    int msgnum;

    for (int i = 0; i < len; i++) {
      msgnum = msgnums[i];
      if (!mail_sinfo_present(_dcStream, msgnum, 0)) {
        fetchList(-1) = msgnum;
      }
    }

    if (fetchList.ElementCount()) {
      char *sequence;

      if ((sequence = _GenerateMessageSequence(fetchList)) == NULL)
        return (err = Sdm_EC_BadMessageSequence);
        
      mail_fetchflags_full(_dcStream, sequence, 0);
      delete sequence;
      
      if (_dcServerConnectionBroken == Sdm_True) {
        return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
      } 

      _HandleMailboxConnectionReadOnly();
    }
  }

  for (int i = 0; i < len; i++) {
    SdmMessageFlagAbstractFlags aflags = 0;
    SET_ABSTRACT_FLAG_BITMASK(msgnums[i], aflags);
    r_aflags.AddElementToList(aflags);
  }

  return (err = Sdm_EC_Success);
}

#undef SET_ABSTRACT_FLAG_BITMASK


// Each string added to the STRINGLIST data structure is EITHER a const
// character pointer extracted from an SdmString object OR a const character
// pointer representing an abstract header (residing in the data segment).

// The c-client's mail_free_stringlist function cannot be used to free the
// STRINGLIST, since this function assumes the string data was also
// allocated on the heap and passes each character pointer to free().

// One solution is to always allocate a copy of each string on the heap and
// add this copy to the STRINGLIST; then mail_free_stringlist will work
// correctly.

// However, this unnecessary overhead is avoided by the following function
// which only frees the STRINGLIST structs, leaving the character pointers
// intact.
//
static void _SdmFreeStringList(STRINGLIST *stringlist)
{
  if (stringlist) {		/* only free if exists */
    _SdmFreeStringList(stringlist->next);
    free ((char*) stringlist);	/* return string to free storage */
  }
}


static const SdmMessageHeaderAbstractFlags SINFO_HDRS = \
  Sdm_MHA_P_MessageFrom | Sdm_MHA_P_MessageSize | Sdm_MHA_P_MessageType | \
  Sdm_MHA_P_ReceivedDate | Sdm_MHA_P_Sender | Sdm_MHA_P_SenderPersonalInfo | \
  Sdm_MHA_P_Subject | Sdm_MHA_P_SenderUsername | Sdm_MHA_P_AbbreviatedReceivedDate;

static void _RetrieveHeadersFromShortInfo
  (
  SHORTINFO *sinfo,
  SdmStrStrL *r_strstr_hdrs,
  SdmIntStrL *r_intstr_hdrs,
  SdmMessageHeaderAbstractFlags &abst_hdrs,
  SdmBoolean using_abstract
  )
{
  // If at least one SHORTINFO header is present in the abst_hdrs, then
  // check for the existence of each SHORTINFO header in abst_hdrs and
  // extract any values.
  if (abst_hdrs & SINFO_HDRS) {
#define SINFO_CHECK_HEADER(HDR,FIELD,PNAME) \
    if (abst_hdrs & (HDR)) { \
      if (sinfo->FIELD) \
        if (using_abstract) \
          (*r_intstr_hdrs)(-1).SetNumberAndString((HDR), sinfo->FIELD); \
        else \
          (*r_strstr_hdrs)(-1).SetBothStrings(PNAME, sinfo->FIELD); \
      abst_hdrs &= ~(HDR); \
    }

    SINFO_CHECK_HEADER(Sdm_MHA_From, from, "From");
    SINFO_CHECK_HEADER(Sdm_MHA_P_MessageFrom, from, "P_MessageFrom");
    SINFO_CHECK_HEADER(Sdm_MHA_P_MessageSize, size, "P_MessageSize");
    SINFO_CHECK_HEADER(Sdm_MHA_P_MessageType, bodytype, "P_MessageType");
    SINFO_CHECK_HEADER(Sdm_MHA_P_ReceivedDate, date, "P_ReceivedDate");
    SINFO_CHECK_HEADER(Sdm_MHA_P_AbbreviatedReceivedDate, date, "P_AbbreviatedReceivedDate");
    SINFO_CHECK_HEADER(Sdm_MHA_P_Sender, host, "P_Sender");
    SINFO_CHECK_HEADER(Sdm_MHA_P_SenderPersonalInfo, personal, "P_SenderPersonalInfo");
    SINFO_CHECK_HEADER(Sdm_MHA_P_SenderUsername, mailbox, "P_SenderUsername");
    SINFO_CHECK_HEADER(Sdm_MHA_Subject, subject, "Subject");
    SINFO_CHECK_HEADER(Sdm_MHA_P_Subject, subject, "P_Subject");
#undef SINFO_CHECK_HEADER
  }
}

// Construct a c-client STRINGLIST data structure from either a list of
// strings or a bitmask of abstract headers.
//
SdmErrorCode SdmDpDataChanCclient::_CreateStringList
  (
  SdmError &err,
  STRINGLIST *&lines,
  const SdmStringL *str_hdr,
  const SdmMessageHeaderAbstractFlags abst_hdrs,
  SdmBoolean using_abstract
  )
{
  // Create a c-client STRINGLIST data-structure to specify the headers
  // to fetch.
  if (using_abstract) {
    if (abst_hdrs == 0) {
      // This is a somewhat pathological case where no headers are
      // requested.  If all the abstract headers are desired, the abstract
      // header Sdm_MHA_ALL should be used.
      return (err = Sdm_EC_Success);
    }
    for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
      STRINGLIST *l;
      if (abst_hdrs & ahmp->flag) {
        if (ahmp->realName) {
          if (lines) {
            l->next = mail_newstringlist();
            l = l->next;
          }
          else
            l = lines = mail_newstringlist();

          l->text = (char*)ahmp->realName;
          l->size = ::strlen(l->text);
          l->next = NULL;
        }
        else if (ahmp->indirectHeaderList) {
          // Disallow "indirect" abstract headers here - they are handled at
          // a higher level
          _SdmFreeStringList(lines);
          return (err = Sdm_EC_Fail);
        }
      }
    }
  }
  else { // Not using abstract headers
    // If str_hdr is empty, then leave "lines" NULL so as to fetch all
    // headers.  Otherwise, construct the c-client's STRINGLIST linked list
    if ((*str_hdr).ElementCount() != 0) {
      int numHdrs = (*str_hdr).ElementCount();

      for (int i = 0; i < numHdrs; i++) {
        STRINGLIST *l;

        if (lines) {
          l->next = mail_newstringlist();
          l = l->next;
        }
        else
          l = lines = mail_newstringlist();

        l->text = (char*)(const char*)(*str_hdr)[i];
        l->size = ::strlen(l->text);
        l->next = NULL;
      }
    }
  }

  return (err = Sdm_EC_Success);
}


// The following _ExtractHeaders internal function is dual-purpose:
// It should be passed:
// EITHER:      (r_strstr_hrs AND str_hdrs)
// OR:          (r_intstr_hdrs AND abst_hdrs)
//
// When passed a list of string header names, the function augments the
// provided SdmStrStrL to contain the matching header {field, value} pairs.
//
// When passed a list of abstract header names, the function augments the
// provided SdmIntStrL to contain the matching {abstract_field, value} pairs.
//
SdmErrorCode SdmDpDataChanCclient::_ExtractHeaders
  (
  SdmError &err,
  SdmStrStrL *r_strstr_hdrs,             // list of header name/values fetched
  const SdmStringL *str_hdrs,            // ptr to list of names of headers to fetch
  SdmIntStrL *r_intstr_hdrs,             // list of header name/values fetched
  SdmMessageHeaderAbstractFlags abst_hdrs, // abstract headers to fetch
  const SdmMessageNumber msgnum         // single message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmBoolean using_abstract;

  // Check for invalid conditions
  if ((r_strstr_hdrs == NULL) && (r_intstr_hdrs == NULL))
    return (err = Sdm_EC_Fail);
  if ((r_strstr_hdrs != NULL) && (r_intstr_hdrs != NULL))
    return (err = Sdm_EC_Fail);

  if ((r_strstr_hdrs == NULL) && (r_intstr_hdrs != NULL))
    using_abstract = Sdm_True;
  else
    using_abstract = Sdm_False;

  // Always fetch SHORTINFO, since it's required when disconnected.
  // The SHORTINFO data is cached by the c-client.
  SHORTINFO sinfo;
  if (mail_fetchshort_full(_dcStream, msgnum, &sinfo, 0) != T) {
    if (!_dcRemote) {
      mail_free_cached_buffers_msg(_dcStream, msgnum);
    }
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  // If any abst_hdrs are specified, then first try to satisfy header
  // requests from the SHORTINFO.  Note that abst_hdrs may be specified when
  // using_abstract is False.  This is for the case where a StrStrL is to be
  // returned but some pseudo abstract headers are also required.
  if (abst_hdrs) {
    char tmp[SMALLBUFLEN];
    assert (!((abst_hdrs & Sdm_MHA_P_ReceivedDate) && 
              (abst_hdrs & Sdm_MHA_P_AbbreviatedReceivedDate)));

    // If the received date is requested, then convert the "IMAP" date in
    // the SHORTINFO struct to "cdate" format.
    if (abst_hdrs & Sdm_MHA_P_ReceivedDate) {
      MESSAGECACHE *elt = mail_elt(_dcStream, msgnum);
      sinfo.date = mail_cdate(tmp, elt);
    }
    // The following functions modifies abst_hdrs by removing headers that
    // are found in SHORTINFO.
    _RetrieveHeadersFromShortInfo(&sinfo, r_strstr_hdrs, r_intstr_hdrs,
                                  abst_hdrs, using_abstract);

    // If abst_hdrs is 0, then all requested headers may have been found in
    // SHORTINFO.
    if ((abst_hdrs == 0) &&
        (using_abstract || str_hdrs->ElementCount() == 0)) {
      if (!_dcRemote) mail_free_cached_buffers_msg(_dcStream, msgnum);
      return Sdm_EC_Success;
    }
  }

  // Create a c-client STRINGLIST data-structure to specify the headers
  // to fetch.
  STRINGLIST *lines = NULL;
  if (_CreateStringList(err, lines, str_hdrs, abst_hdrs, using_abstract) !=
      Sdm_EC_Success) {
    if (!_dcRemote) mail_free_cached_buffers_msg(_dcStream, msgnum);
    return (err);
  }

  // Fetch the requested headers from the c-client.
  // Each header is delimited by CRLF.
  unsigned long len;
  char *hdr = mail_fetchheader_full(_dcStream, msgnum, lines, &len, NIL);
  _SdmFreeStringList(lines);
  if (!_dcRemote) {
    mail_free_cached_buffers_msg(_dcStream, msgnum);
  }

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True) || len == 1) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  assert(len >= 2 || len == 0);

  if (len == 0 || ((len == 2) && (hdr[0] == CR) && (hdr[1] == LF))) {
    // The stringlist headers aren't present - however some headers may have
    // already been obtained through the shortinfo.
    if ((using_abstract && (*r_intstr_hdrs).ElementCount()) ||
        (!using_abstract && (*r_strstr_hdrs).ElementCount()))
      return (err = Sdm_EC_Success);
    else
      return (err = Sdm_EC_RequestedDataNotFound);
  }

  /* Duplicate the c-client string, since modifying it directly causes
   * memory access problems later because the c-client assumes the data
   * doesn't change - this data is actually stored in the c-client's cache
   * (elt->data1 in imap4.c).
   */
  char c, *p = strdup(hdr), *start = p, *end = p + len;
  if (p == NULL)
    return (err = Sdm_EC_Fail);

  char *hdr_field, *hdr_value, *hdr_value_p;
  SdmBoolean Found = Sdm_False;

  /* Parse each of the {header,value} pairs from p, modifying p in place to
   * tie off each field and skip certain newline characters.
   */
  while (p < end) {

    hdr_field = p;

    // Copy header field name; we stop at the first occurance of:
    // - a colon (:)
    // - the sequence '\r\n' not followed by a ' '
    // - a '\n' not followed by a ' '
    // - a '\r' not followed by a ' '
    // - a ' '
    while (p < end && *p != ':' && *p != ' ' && !(*p == '\r' && p[1] == '\n' && p[2] != ' ') && !(*p == '\n' && p[1] != ' ') && !(*p == '\r' && p[1] != ' '))
      p++;

    // The colon should always cause exit from the loop.
    // Discover any peculiar cases early!
    if (*p != ':') {
      // Found a space inside of a header name which is illegal
      // Treat the rest of this line as though it were a bad header
      p = hdr_field;
      hdr_field = "X-BadHeader";
    }
    else {
      *p = '\0';

      // Skip past the colon
      p++;
    }

    // Skip over whitespace and CR characters
    while (p < end &&
           ((*p == ' ') || (*p == '\t') || (*p == CR)))
      p++;

    hdr_value = hdr_value_p = p;
    // Copy header value - up to next delimiting LF that's not followed by
    // whitespace.  See RFC822 description of "long" header fields
    SdmBoolean done = Sdm_False;
    while (p < end && done != Sdm_True) {
      c = *p++;
      switch (c) {
      case CR:
        // Remove all occurrences of CR
        continue;
      case LF:
        if (p < end) {
          // Allow for multi-line headers - 
	  // Don't copy the LF - "unfold" as per RFC822
	  // And throw away all "whitespace" leaving just a single space
          if ((*p == ' ') || (*p == '\t')) {
	    while (p < end && (*p == ' ' || *p == '\t'))
	      p++;
	    *hdr_value_p++ = ' ';
            continue;	
	  }
          else
            done = Sdm_True;
        }
        else
          done = Sdm_True;
        break;
      default:
        *hdr_value_p++ = c;
        break;
      }
    }

    *hdr_value_p = '\0';

    // Skip over whitespace and CRLF delimiters
    while (p < end && ((*p == CR) || (*p == LF)))
      p++;

    if (using_abstract) {
      int len = ::strlen(hdr_field);

      // Since the STRINGLIST passed to the c-client was constructed from
      // strings in the abstract header map table, the header fields
      // returned from the c-client should only be in the set of abstract
      // headers names.
      // However, the ordering and multiplicity of headers are unknown, so it
      // is therefore necessary to map back from a header field value to
      // the abstract representation.
      for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
        if (abst_hdrs & ahmp->flag) {
          // This function only supports "real" abstract headers.
          // The "pseudo" abstract headers are handled at a higher level.
          assert(ahmp->realName);

          // Only perform a string compare if the lengths are the same.
          if (ahmp->realNameLength == len) {
            if (::strncasecmp(hdr_field, ahmp->realName, len) == 0) {
              Found = Sdm_True;
              break;
            }
          }
        }
      }
      // The code used to assert (ahmp->flag != 0) .  However, when mailbox
      // headers are corrupted, the c-client can return bogus header data
      // (see bug 4030738).  The following code works around this problem.
      if (ahmp->flag == 0) {
        free(start);
        if (Found)
          return (err = Sdm_EC_Success);
        else
          return (err = Sdm_EC_RequestedDataNotFound);
      }

      (*r_intstr_hdrs)(-1).SetNumberAndString(ahmp->flag, hdr_value);
    }
    else
      (*r_strstr_hdrs)(-1).SetBothStrings(hdr_field, hdr_value);
  }

  free(start);
  return (err = Sdm_EC_Success);
}

// Return a list of all headers for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrL &r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmStringL nullHdrList;

  return (err = _ExtractHeaders(err, &r_hdr, &nullHdrList, NULL, 0, msgnum));
}

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // Create a local lookahead object - this set the c-client IMAP lookahead
  // and resets it to the default on destruction.
  SdmDpCclientLookahead lookahead(_dcStream, startmsgnum, endmsgnum);

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, i);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      r_hdr.ClearAndDestroyAllElements();
      return (err);
    }

    r_hdr.AddElementToList(hdrList);
  }

  return (err = Sdm_EC_Success);
}

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL &msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int foundOne = 0;
  int len = msgnums.ElementCount();

  for (int i = 0; i < len; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, msgnums[i]);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      delete hdrList;
      r_hdr.ClearAndDestroyAllElements();
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of headers for a single header for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeader
  (
  SdmError &err,
  SdmStrStrL &r_hdr,			// header value fetched
  const SdmString &hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
#if 0
  // Cannot do this because abstract header fetch can call multiple times
  assert(r_hdr.ElementCount() == 0);
l
#endif
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmStringL hdrList;

  hdrList.AddElementToList(hdr);

  return (err = _ExtractHeaders(err, &r_hdr, &hdrList, NULL, NULL, msgnum));
}


// Return a list of specific headers for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrL &r_hdrs,			// list of header name/values fetched
  const SdmStringL &hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
#if 0
  // Cannot do this because abstract header fetch can call multiple times
  assert(r_hdr.ElementCount() == 0);
l
#endif
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  return (err = _ExtractHeaders(err, &r_hdrs, &hdrs, NULL, NULL, msgnum));
}


#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeader
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,			// list of header values fetched
  const SdmString &hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  int foundOne = 0;

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // Create a local lookahead object - this set the c-client IMAP lookahead
  // and resets it to the default on destruction.
  SdmDpCclientLookahead lookahead(_dcStream, startmsgnum, endmsgnum);

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeader(err, *hdrList, hdr, i);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      delete hdrList;
      r_hdr.ClearAndDestroyAllElements();
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}


// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeader
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,			// list of header values fetched
  const SdmString &hdr,			// name of header to fetch
  const SdmMessageNumberL &msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int foundOne = 0;
  int len = msgnums.ElementCount();

  for (int i = 0; i < len; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeader(err, *hdrList, hdr, msgnums[i]);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      delete hdrList;
      r_hdr.ClearAndDestroyAllElements();
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}




// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL &hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdrs.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // Create a local lookahead object - this set the c-client IMAP lookahead
  // and resets it to the default on destruction.
  SdmDpCclientLookahead lookahead(_dcStream, startmsgnum, endmsgnum);

  int foundOne = 0;

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdrs, i);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      delete hdrList;
      r_hdrs.ClearAndDestroyAllElements();
      return (err);
    }
    r_hdrs.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdrs.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL &hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL &msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdrs.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int foundOne = 0;
  int len = msgnums.ElementCount();

  for (int i = 0; i < len; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdrs, msgnums[i]);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      // Free the partially complete list
      delete hdrList;
      r_hdrs.ClearAndDestroyAllElements();
      return (err);
    }
    r_hdrs.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdrs.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif

// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrL &r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmStringL hdrList;
  SdmMessageHeaderAbstractFlags abst_hdrs = 0;

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr & ahmp->flag) {
      // Accumulate the headers which can be obtained from shortinfo.
      if (ahmp->useShortInfo) {
        abst_hdrs |= ahmp->flag;
      }
      else if (ahmp->realName) {
        // Accumulate the real header names in a list.  This way the headers
        // can be fetched "en masse" using a single c-client STRINGLIST
        hdrList(-1) = ahmp->realName;
      }
      else if (ahmp->indirectHeaderList) {
        SdmMessageHeaderAbstractFlag *abstHdrList = ahmp->indirectHeaderList;
        int numhdrs = r_hdr.ElementCount();

        for (int i = 0; abstHdrList[i]; i++) {
          const char *hdrnm = NULL;

          for (SdmAbstractHeaderMap *hdrmap = _sdmAbstractHeaders; hdrmap->flag; hdrmap++) {
            if (hdrmap->flag == abstHdrList[i]) {
              hdrnm = hdrmap->realName;
              break;
            }
          }
          // There must exist a name entry in the header map for each
          // indirect header.
          assert(hdrnm);

          SdmErrorCode status = GetMessageHeader(err, r_hdr, SdmString(hdrnm), msgnum);

          if (status != Sdm_EC_Success) {
            if (status == Sdm_EC_RequestedDataNotFound)
            // If header non-existent, continue search for other indirect headers
              continue;
            else {
              r_hdr.ClearAllElements();
              return (err);
            }
          }
          if (r_hdr.ElementCount() != numhdrs)
            break;
        }
      }
    }
  }

  // Fetch the list of individual headers using the real header names
  if (hdrList.ElementCount() != 0 || abst_hdrs != 0) {
    if (_ExtractHeaders(err, &r_hdr, &hdrList, NULL, abst_hdrs, msgnum) != Sdm_EC_Success) {
      r_hdr.ClearAllElements();
      hdrList.ClearAllElements();
      return (err);
    }
  }

  hdrList.ClearAllElements();
  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // Create a local lookahead object - this set the c-client IMAP lookahead
  // and resets it to the default on destruction.
  SdmDpCclientLookahead lookahead(_dcStream, startmsgnum, endmsgnum);

  int foundOne = 0;

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdr, i);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      r_hdr.ClearAndDestroyAllElements();
      delete hdrList;
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmStrStrLL &r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL &msgnums		// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();

  for (int i = 0; i < numMessages; i++) {
    SdmStrStrL *hdrList = new SdmStrStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdr, msgnums[i]);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      r_hdr.ClearAndDestroyAllElements();
      delete hdrList;
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}


#endif

// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmIntStrL &r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmMessageHeaderAbstractFlags abst_hdrs = 0;

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr & ahmp->flag) {
      if (ahmp->realName || ahmp->useShortInfo) {
        // Accumulate in a bitmask those abstract headers which have a
        // "real" name or are available in the shortinfo.  This way the
        // headers can be fetched "en masse" using a single c-client
        // STRINGLIST
        abst_hdrs |= ahmp->flag;
      }
      else if (ahmp->indirectHeaderList) {
        SdmMessageHeaderAbstractFlag *abstHdrList = ahmp->indirectHeaderList;
        int numhdrs = r_hdr.ElementCount();

        for (int i = 0; abstHdrList[i]; i++) {
          SdmErrorCode status =
            _ExtractHeaders(err, NULL, NULL, &r_hdr, abstHdrList[i], msgnum);

          if (status != Sdm_EC_Success) {
            if (status == Sdm_EC_RequestedDataNotFound)
            // If header non-existent, continue search for other pseudo headers
              continue;
            else {
              r_hdr.ClearAllElements();
              return (err);
            }
          }
          // Once a matching header has been found in the indirect header list,
          // cease searching any further.
          if (r_hdr.ElementCount() != numhdrs)
            break;
        }
      }
    }
  }

  // Fetch the list of individual headers using the real header names
  if (abst_hdrs != 0) {
    if (_ExtractHeaders(err, NULL, NULL, &r_hdr, abst_hdrs, msgnum) !=
        Sdm_EC_Success) {
      if (err != Sdm_EC_RequestedDataNotFound) {
        r_hdr.ClearAllElements();
        return (err);
      }
    }
  }

  // return error if no header found.
  if (r_hdr.ElementCount() == 0) {
     return (err = Sdm_EC_RequestedDataNotFound);
  }
  
  return (err = Sdm_EC_Success);
}

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmIntStrLL &r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  // Create a local lookahead object - this set the c-client IMAP lookahead
  // and resets it to the default on destruction.
  SdmDpCclientLookahead lookahead(_dcStream, startmsgnum, endmsgnum);

  int foundOne = 0;

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmIntStrL *hdrList = new SdmIntStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdr, i);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      r_hdr.ClearAndDestroyAllElements();
      delete hdrList;
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}


// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageHeaders
  (
  SdmError &err,
  SdmIntStrLL &r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL &msgnums		// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(r_hdr.ElementCount() == 0);
  assert(_dcOpen);
  assert(_dcStream);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();

  for (int i = 0; i < numMessages; i++) {
    SdmIntStrL *hdrList = new SdmIntStrL(INITIAL_HEADER_CACHE_SIZE);
    GetMessageHeaders(err, *hdrList, hdr, msgnums[i]);
    if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
      r_hdr.ClearAndDestroyAllElements();
      delete hdrList;
      return (err);
    }
    r_hdr.AddElementToList(hdrList);
    if (hdrList->ElementCount())
      foundOne++;
  }

  if (!foundOne)
    r_hdr.ClearAndDestroyAllElements();

  return (err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}


static SdmMsgStrEncoding _SdmGetMessageEncoding(short encoding)
{
  SdmMsgStrEncoding sdm_encoding;

  switch(encoding) {
  case ENC7BIT:
    sdm_encoding = Sdm_MSENC_7bit;
    break;
  case ENC8BIT:
    sdm_encoding = Sdm_MSENC_8bit;
    break;
  case ENCBINARY:
    sdm_encoding = Sdm_MSENC_binary;
    break;
  case ENCBASE64:
    sdm_encoding = Sdm_MSENC_base64;
    break;
  case ENCQUOTEDPRINTABLE:
    sdm_encoding = Sdm_MSENC_quoted_printable;
    break;
  case ENCOTHER:
  default:
    assert (encoding <= ENCMAX);
    sdm_encoding = Sdm_MSENC_other;
    break;
  }

  return sdm_encoding;
}

static SdmMsgStrType _SdmGetMessageType(short type)
{
  SdmMsgStrType sdm_type;

  switch (type) {
  case TYPETEXT:
    sdm_type = Sdm_MSTYPE_text;
    break;
  case TYPEMULTIPART:
    sdm_type = Sdm_MSTYPE_multipart;
    break;
  case TYPEMESSAGE:
    sdm_type = Sdm_MSTYPE_message;
    break;
  case TYPEAPPLICATION:
    sdm_type = Sdm_MSTYPE_application;
    break;
  case TYPEAUDIO:
    sdm_type = Sdm_MSTYPE_audio;
    break;
  case TYPEIMAGE:
    sdm_type = Sdm_MSTYPE_image;
    break;
  case TYPEVIDEO:
    sdm_type = Sdm_MSTYPE_video;
    break;
  case TYPEOTHER:
  default:
    // Note: The above c-client MIME type constants are indexes into an
    // internal body_types array of strings that represent the correponding
    // MIME type names.

    // If the c-client encounters a MIME type that is none of the above, it
    // adds its name, on the fly, to the internal body_types array and
    // assigns it a numeric "type" of the next available index
    // (TYPEOTHER+1).  However, there is no constant by which to identify
    // this, so Sdm_MSTYPE_other is used in this case.
    assert(type <= TYPEMAX);
    sdm_type = Sdm_MSTYPE_other;
    break;
  }

  return sdm_type;
}

// Close the current connection down
//
SdmErrorCode SdmDpDataChanCclient::_Close
  (
  SdmError &err
  )
{
  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  if (!_dcOpen && !_dcAttached)
    return (err = Sdm_EC_Closed);

  // do this first!!  This way, if the
  // mm_io_callback is called as a result of
  // the mail_close, then it will disregard
  // the server disconnect signal.
  _dcOpen = Sdm_False;

  // We must call mail_check because of bug 4049782 - when it is fixed
  // this can be removed.  mail_check results in a call to mail_exists
  // which updates _dcStream->nmsgs to reflect the new number of
  // messages in the mailbox. We need to cause the short info
  // structure for all new messages to be fetched.
#if 1
  if (!_dcAttached && _dcStream) {

    mail_check(_dcStream);
    if (_dcStream->nmsgs > _dcNmsgs) {

      unsigned long total_messages = _dcStream->nmsgs;

      for (unsigned long msg_num = total_messages-_dcNmsgs; msg_num <= total_messages; msg_num++) {
	// Always fetch SHORTINFO, since it's required when disconnected.
	// The SHORTINFO data is cached by the c-client.
	SHORTINFO sinfo;
	(void) mail_fetchshort_full(_dcStream, msg_num, &sinfo, 0);
	if (!_dcRemote) {
	  mail_free_cached_buffers_msg(_dcStream, msg_num);
	}
      }
    }
  }
#endif

  // mail_close_full always returns NIL
  _dcStream = mail_close_full(_dcStream, NIL);
  _dcReadonly = Sdm_False;
  _dcRemote = Sdm_False;
  _dcAttached = Sdm_False;
  _dcDisconnected = Sdm_False;

  return (err = Sdm_EC_Success);
}

// Copy the body information from the c-client's BODY structure into an
// SdmDpMessageStructure.
//
SdmErrorCode SdmDpDataChanCclient::_CopyBodyStructure
  (
  SdmError &err,
  SdmDpMessageStructure *dpms,
  const SdmMessageNumber msgnum,
  BODY *body,
  char *section
  )
{
  dpms->ms_type = _SdmGetMessageType(body->type);
  dpms->ms_encoding = _SdmGetMessageEncoding(body->encoding);

  if (body->subtype)
    dpms->ms_subtype = body->subtype;

  dpms->ms_lines = body->size.lines;
  dpms->ms_bytes = body->size.bytes;

  dpms->ms_cached = ((dpms->ms_type != Sdm_MSTYPE_multipart) &&
    (!_dcRemote || body->contents.text)) ? Sdm_True : Sdm_False;

  dpms->ms_component = new SdmDpCclientMessageComponent();
  if (section)
    ((SdmDpCclientMessageComponent *)(dpms->ms_component))->_section = section;
  ((SdmDpCclientMessageComponent *)(dpms->ms_component))->_bodyPart = body;

  if (body->id)
    dpms->ms_id = body->id;

  if (body->description)
    dpms->ms_description = body->description;

  // dpms->ms_ms is set by the caller

  if (body->md5)
    dpms->ms_md5 = body->md5;

  // We should really first look in the ContentDisposition field for an
  // attachment name (see RFC1806); however the cclient doesn't yet provide
  // any mechanism to retrieve this - this capability should be available in
  // IMAP4 rev1.

  // The following should be set to either "inline" or "attachment".
  // However, the Content-Disposition field is not yet available.
  // dpms->ms_disposition = 0;

  for (PARAMETER *param = body->parameter; param != NULL; param = param->next) {
    if (::strcasecmp(param->attribute, "name") == 0)
      dpms->ms_attachment_name = param->value;
    else if (::strcasecmp(param->attribute, "x-unix-mode") == 0) {
      unsigned long mode;
      if (mode = strtoul(param->value, NULL, 8))
        dpms->ms_unix_mode = mode;
    }
  }

  return (err = Sdm_EC_Success);
}


// Return the message structure for a single message.
//
SdmErrorCode SdmDpDataChanCclient::_GetMessageStructure
  (
  SdmError &err,
  SdmDpMessageStructureL &r_structure,  // returned message structure list describing messages
  const SdmMessageNumber msgnum,
  PART *part,                           // c-client description of this body part
  char *section                         // c-client section number identifying this body part
  )
{
  SdmDpMessageStructure *dpms = new SdmDpMessageStructure;
  SdmBoolean multipart = Sdm_False;
  char newSection[SMALLBUFLEN];

  if (part->body.type == TYPEMULTIPART && part->body.contents.part) {
    multipart = Sdm_True;
    // Note on c-client section numbering:
    // If a M/M (multipart/mixed) contains 3 T/P (text/plain) messages,
    // these T/P's are numbered: "1", "2", "3".
    //
    // However, if a message has the following nested structure:
    //
    // M/M
    //   T/P
    //   M/M
    //     T/P
    //     T/P
    //   T/P
    // .. the T/P's are numbered "1", "2.1", "2.2", "3"
    //
    // Note that the M/M parts are given a NULL number, since fetching the
    // contents of the actual containing M/M is not supported; only the
    // invididual parts of a M/M may be fetched.
    strcpy(newSection, section);
    strcat(newSection, ".1");

    dpms->ms_ms = new SdmDpMessageStructureL;
    _GetMessageStructure(err, *dpms->ms_ms, msgnum, part->body.contents.part,
                         newSection);
  }
  else {
    dpms->ms_ms = NULL;
  }

  // Note that the msgnum argument is 0 to prevent any msgnum being stored
  // in the component.  A "real" msgnum is only stored for the "toplevel"
  // part, so that the whole body hierarchy is only freed once.
  _CopyBodyStructure(err, dpms, 0, &part->body, multipart ? NULL : section);

  r_structure.AddElementToList(dpms);

  if (part->next) {
    char *p;
    int finalDigit;

    // Increment the section number
    strcpy(newSection, section);

    if ((p = strrchr(newSection, '.')) != NULL)
      ++p;      // Skip past the "."
    else
      p = newSection;

    finalDigit = atoi(p);
    sprintf(p, "%d", finalDigit + 1);

    _GetMessageStructure(err, r_structure, msgnum, part->next, newSection);
  }

  return (err = Sdm_EC_Success);
}


// Return the message structure for a single message
//
SdmErrorCode SdmDpDataChanCclient::GetMessageStructure
  (
  SdmError &err,
  SdmDpMessageStructureL &r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(!_dcAttached);
  assert(_dcStream);
  assert(r_structure.ElementCount() == 0);

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  ENVELOPE *envelope;
  BODY *body;
  // Fetch the full message structure from the c-client
  envelope = mail_fetchstructure_full(_dcStream, msgnum, &body, 0);

  if (!_dcRemote) {
    mail_free_cached_buffers_msg(_dcStream, msgnum);
  }

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True) || 
      envelope == NULL) 
  {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  assert(envelope != NULL);
  SdmDpMessageStructure *dpms = new SdmDpMessageStructure;
  SdmBoolean multipart = Sdm_False;

  // When body->type is multipart, body->contents.part should point to the
  // various parts.  However, in the case of an incorrectly specified
  // boundary, body->contents.parts is NULL.  In this case, the subpart
  // structure cannot be determined.
  if (body->type == TYPEMULTIPART && body->contents.part) {
    multipart = Sdm_True;

    dpms->ms_ms = new SdmDpMessageStructureL;

    if (_GetMessageStructure(err, *dpms->ms_ms, msgnum, body->contents.part, "1") != Sdm_EC_Success) {
      r_structure.ClearAndDestroyAllElements();
      return (err);
    }
  }
  else {
    dpms->ms_ms = NULL;
  }

  _CopyBodyStructure(err, dpms, msgnum, body, multipart ? NULL : "1");

  r_structure.AddElementToList(dpms);

  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageStructure
  (
  SdmError &err,
  SdmDpMessageStructureLL &r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);
  assert(r_structure.ElementCount() == 0);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  for (int i = startmsgnum; i <= endmsgnum; i++) {
    SdmDpMessageStructureL *newStr = new SdmDpMessageStructureL;
    if (GetMessageStructure(err, *newStr, i) != Sdm_EC_Success) {
      r_structure.ClearAndDestroyAllElements();
      delete newStr;
      return (err);
    }
    r_structure.AddElementToList(newStr);
  }

  return (err = Sdm_EC_Success);
}

// Return a list of message structures for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::GetMessageStructure
  (
  SdmError &err,
  SdmDpMessageStructureLL &r_structure,	// returned message structure list describing messages
  const SdmMessageNumberL &msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);
  assert(r_structure.ElementCount() == 0);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  int numMessages = msgnums.ElementCount();

  for (int i = 0; i < numMessages; i++) {
    SdmDpMessageStructureL *newStr = new SdmDpMessageStructureL;
    if (GetMessageStructure(err, *newStr, msgnums[i]) != Sdm_EC_Success) {
      r_structure.ClearAndDestroyAllElements();
      delete newStr;
      return (err);
    }
    r_structure.AddElementToList(newStr);
  }
  return (err = Sdm_EC_Success);
}

#endif

// Return a message store status structure for the message store
//
SdmErrorCode SdmDpDataChanCclient::GetMessageStoreStatus
  (
  SdmError &err,
  SdmMessageStoreStatus &r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  _dcStatusCollector = &r_status;

  // The following call to mail_status should result in a call to the
  // mm_status callback.  This callback will assign the c-client's status
  // to the _dcStatusCollector variable.

  if (!mail_status(_dcStream, _dcStream->mailbox, storeFlags)) {
    _dcStatusCollector = NULL;
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }
 
  // Update the internal message count
  _dcNmsgs = _dcStream->nmsgs;

  _dcStatusCollector = NULL;

  if (_dcCclientError != Sdm_EC_Success) {
    return(_HandleErrorExitConditions(err,(SdmErrorCode)_dcCclientError));
  }

  _HandleMailboxConnectionReadOnly();
  
  return (err = Sdm_EC_Success);
}

void SdmDpDataChanCclient::_HandleMailboxConnectionReadOnly()
{
  // This method is called to check when a connection to a mailbox
  // goes into "read only" mode - typically this means the kiss of
  // read only was sent from one mail process to another and both are
  // in read only mode.

  // See if the mailbox connection went read-only on us
  if (_dcMailboxConnectionReadOnly == Sdm_True) {
    // call service function to let fromt end know that 
    // the conection just went read only on us.
    CallLockActivityCallback(DPEVENT_MailboxConnectionReadOnly);

    // reset the flag indicating that the mailbox connection
    // is read only. 
    SetMailboxConnectionReadOnly(Sdm_False);
  }
}

void SdmDpDataChanCclient::_GetStringFromFlags
  (
  SdmString &r_flagsString, 
  SdmMessageFlagAbstractFlags flags
  )
{
  if (flags != 0) {
    char buffer[60];
    sprintf(buffer, "%s%s%s%s%s",
       flags & Sdm_MFA_Answered ? "\\ANSWERED " : "",
       flags & Sdm_MFA_Deleted ? "\\DELETED " : "",
       flags & Sdm_MFA_Draft ? "\\DRAFT " : "",
       flags & Sdm_MFA_Flagged ? "\\FLAGGED " : "",
       flags & Sdm_MFA_Seen ? "\\SEEN " : "");
    r_flagsString = buffer;
  } else {
    r_flagsString = "";
  }
}

  
SdmErrorCode SdmDpDataChanCclient::_HandleInvalidMailboxState
  (
  SdmError &err
  )
{
  // close the channel adapter.  
  // this must be done BEFORE calling SdmDataPort::Close 
  // below to avoid a deadlock when trying to get the 
  // gCclientLock mutex twice.  If _Close is called after 
  // SdmDataPort::Close, SdmDpDataChanCclient::Close will 
  // try to get the mutex.  This causes a deadlock because 
  // we currently already have the mutex from the initial 
  // cclient call that resulted in the broken connection 
  // being detected.
  _Close(err);

  // note: we can't check the _dcRemote flag because it got
  // reset in _Close.
  
  if (_dcServerConnectionBroken) { 
    // call service function to let front end know that
    // the server is dead.
    CallLockActivityCallback(DPEVENT_ServerConnectionBroken);

    // reset the flag indicating that the server connection
    // is broken.  we now have the data channel closed so any
    // future calls should return error value Sdm_EC_Closed.
    SetServerConnectionBroken(Sdm_False);
  } else if (_dcMailboxChangedByOtherUser) {
    CallLockActivityCallback(DPEVENT_MailboxChangedByOtherUser);
    SetMailboxChangedByOtherUser(Sdm_False);
  }
      
  // close the data port for this channel.
  // DO THIS LAST!!  it will result in this SdmDpDataChanCclient
  // object being destroyed.
  assert (_dcParentDataPort != NULL);
  _dcParentDataPort->Close(err);
  
  return err;
}

SdmErrorCode SdmDpDataChanCclient::MailStream(SdmError& err, void *&r_stream)
{
  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  r_stream = _dcStream;

  return (err = Sdm_EC_Success);
}

// open a connection to an object given a token stream
//
SdmErrorCode SdmDpDataChanCclient::Open
  (
  SdmError &err,
  SdmMessageNumber &r_nmsgs,
  SdmBoolean& r_readOnly,	// indicates if open succeeded in "read only" mode
  const SdmToken &tk		// token describing object to open
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  // if already open, this is an error
  if (_dcOpen)
    return (err = Sdm_EC_Fail);

  // clean up from any previous attempts (e.g. connects)
  if (_dcAttached) {
    if (Close(err))
      return (err);
  }

  // Reset all object variables
  _dcUsername = "";	// no user name associated with this open yet
  _dcHostname = "";	// no host name associated with this open yet
  _dcPassword = "";	// no password associated with this open yet
  _dcReadonly = Sdm_False;	// not read only
  _dcAttached = Sdm_False;	// not attached
  _dcRemote = Sdm_False;	// not remote yet
  _dcDebug = Sdm_False;		// not debugging
  _dcDisconnected = Sdm_False;	// not disconnected

  // See if the token describing the object to open loosely matches
  // this data channel reference token for opening - if it does not
  // it means the object to open is incompatible with this data channel
  if (tk.CompareLoose(*_dcRefToken) != Sdm_True)
    return (err = Sdm_EC_Fail);

  // Determine if local/imap options are present

  SdmString serviceType;
  SdmBoolean serviceLocal = tk.CheckValue("servicetype", "local");
  SdmBoolean serviceImap = tk.CheckValue("servicetype", "imap");

  if (!tk.FetchValue(serviceType, "servicetype") || (!serviceLocal && !serviceImap))
    return(err = Sdm_EC_ServiceTypeRequired);

  // Ok this token is acceptable to attempt opening this channel adapter.
  SdmString storename, remote_storename, dummy;
  long open_flags = OP_NOUSERSIGS;

  if (!tk.FetchValue(storename, "serviceoption", "messagestorename"))
    return (err = Sdm_EC_MessageStoreNameRequired);

  if (!serviceLocal && serviceImap) {
    if (!tk.FetchValue(_dcHostname, "serviceoption", "hostname"))
      return (err = Sdm_EC_HostnameRequired);

    // Store the username and password in private data members.
    // They will be accessed during login by the mm_login callback.
    if (!tk.FetchValue(_dcUsername, "serviceoption", "username"))
      return (err = Sdm_EC_UsernameRequired);

    if (!tk.FetchValue(_dcPassword, "serviceoption", "password"))
      return (err = Sdm_EC_PasswordRequired);

    // Construct the c-client's remote mailbox specification.
    // Eg. {machine}/home/user
    remote_storename = "{";
    remote_storename += _dcHostname;
    remote_storename += "}";
    remote_storename += storename;
    _dcRemote = Sdm_True;
  }
  else {
    // Local access - mark it as so and make the path name an absolute path
    // name to the mailbox, removing all links, variable expansions, etc.
    _dcRemote = Sdm_False;
    if (SdmUtility::ExpandAndMakeAbsolutePath(err, storename, storename, Sdm_True) != Sdm_EC_Success)
      return(err);
  }

  if (tk.FetchValue(dummy, "serviceoption", "debug")) {
    _dcDebug = Sdm_True;
    open_flags |= OP_DEBUG;
  }

  if (tk.FetchValue(dummy, "serviceoption", "readonly")) {
    _dcReadonly = Sdm_True;
    open_flags |= OP_READONLY;
  }

  // By default, Sun V3 messages are rewritten to the mailbox as MIME.  If
  // the following option is set, V3 messages are preserved.
  if (!tk.FetchValue(dummy, "serviceoption", "preservev3messages")) {
    open_flags |= OP_KEEP_MIME;
  }

  if (tk.FetchValue(dummy, "serviceoption", "disconnected"))
    _dcDisconnected = Sdm_True;

#if defined(INCLUDE_SESSIONLOCKING)
  // If 'grabsessionlock' is specified, then set flag that causes the session locking
  // code to attempt to steal the lock away from another mailer that has an active
  // session lock on the mailbox

  _dcLockGrabIfOwnedByOther = tk.FetchValue(dummy, "serviceoption", "grabsessionlock");

  // If 'ignoresessionlock' is specified, then set flag that causes the session locking
  // code to ignore a failure to obtain a session lock, allowing it to be overridden

  _dcLockIgnoreIfOwnedByOther = tk.FetchValue(dummy, "serviceoption", "ignoresessionlock");

  // If 'nosessionlocking' is specified, do not do any session locking

  _dcLockNoSessionLocking = tk.FetchValue(dummy, "serviceoption", "nosessionlocking");

#endif // defined(INCLUDE_SESSIONLOCKING)

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  _dcStream = mail_stream_create(this, NULL, NULL,
                                 _EnableGroupPrivileges, _DisableGroupPrivileges);

  if (_dcStream == NULL) {
    if (_dcCclientError != Sdm_EC_Success) {
      err = _dcCclientError;
      _dcCclientError.Reset();  // reset this now that we've saved it in err.    
    }
    err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return err;
  }

  // Set the max login attempts to one.
  mail_stream_setNIL(_dcStream);
  mail_parameters(_dcStream, SET_MAXLOGINTRIALS, (void*)1);
  mail_stream_unsetNIL(_dcStream);

  _dcStream = mail_open(_dcStream, (char*)(const char*)
                        (_dcRemote ? remote_storename : storename),
                        open_flags);

  // It appears that a failed open may leave the stream in a "halfopen" state
  if (_dcStream && _dcStream->halfopen) {
    _dcStream = mail_close_full(_dcStream, NIL);
    assert(_dcStream == NULL);
  }

  // If the returned stream is NULL or not in the S_OPENED state, then the
  // mail_open failed.  Perform the best approximation of "error handling"
  // as Le Client du C allows us
  if (_dcStream == NULL || !(_dcStream->stream_status & S_OPENED)) {
    _HandleErrorExitConditions(err,Sdm_EC_CannotAccessMessageStore);
    if (!serviceLocal && serviceImap) {
      char *host = new char[_dcHostname.Length() + 3];
      sprintf(host, "{%s}", (const char*)_dcHostname);
      if (mail_echo(host) == NULL) {
	err.AddMinorErrorCodeAndMessage(Sdm_EC_CannotConnectToIMAPServer, (const char*)_dcHostname);
      }
      delete host;
    }
    err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return (err);
  }

  assert(_dcStream->userdata == this);

  // The stream is open - if we have opened the user's inbox, then we
  // need to record the user name used so that when e-mail is sent the
  // correct user name is specified. If we opened the inbox locally,
  // then the current login user name is used; if we opened the inbox
  // remotely, then the login name used for remote access is used.

  if (::strcasecmp((const char *)storename, "inbox")==0) {
    // The user's inbox has been opened
    SdmMessageUtility::SetMailSendingUserName(_dcRemote ? (const char *)_dcUsername : (const char *)0);
  }

  // c-client hack: an open on a read only mailbox is indicated by the stream->rdonly flag
  // being set - if so, we need to inform our caller that this open was read-only

  r_readOnly = _dcStream->rdonly ? Sdm_True : Sdm_False;

  r_nmsgs = _dcNmsgs = _dcStream->nmsgs;
  _dcOpen = Sdm_True;
  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode SdmDpDataChanCclient::MoveMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[MAILTMPLEN];
  sprintf(sequence, "%d", msgnum);

  return (_CopyOrMoveMessages(err, token, sequence, CP_MOVE));
}

// Move a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[MAILTMPLEN];
  if (startmsgnum == endmsgnum)
    sprintf(sequence, "%d", startmsgnum);
  else
    sprintf(sequence, "%d:%d", startmsgnum, endmsgnum);

  return (_CopyOrMoveMessages(err, token, sequence, CP_MOVE));
}

// Move a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChanCclient::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  char *sequence;
  if ((sequence = _GenerateMessageSequence(msgnums)) == NULL)
    return (err = Sdm_EC_BadMessageSequence);

  _CopyOrMoveMessages(err, token, sequence, CP_MOVE);
  delete sequence;
  return err;
}

#endif

// Perform a search against all messages and return a list of messages that match
//
SdmErrorCode 
SdmDpDataChanCclient::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,	// augmented list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (r_msgnums.ElementCount() != 0) {
    err = Sdm_EC_Fail;
    return err;
  }
  
  SdmSearchLeaf *leaf;
  SEARCHPGM *pgm;
  SdmSearch *s = (SdmSearch*) &search;
  
  if (s->ParseSearch(err, leaf))
    return err;
 
  if (CreateSearchPgmForLeaf(err, pgm, leaf)) 
    return err;

  
  _dcSearchCollector = &r_msgnums;
  pgm->only_all = Sdm_True;    // ** ET is this the correct flag to 
                               // set for ALL searching.

  _PerformSearch(err, pgm);
  
  if (err != Sdm_EC_Closed) {
    // reset the pointer for the search collector.
    _dcSearchCollector = NULL;
  }
  
  SdmSearch::FreeLeaf(leaf);
  // don't need to delete pgm.  It's deleted by cclient search is called.

  return err;
}

// Perform a search against a range of messages and return a list of messages that match
//
SdmErrorCode 
SdmDpDataChanCclient::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (r_msgnums.ElementCount() != 0)
    return (err = Sdm_EC_Fail);

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmSearchLeaf *leaf;
  SEARCHPGM *pgm;
  SdmSearch *s = (SdmSearch*) &search;
  
  if (s->ParseSearch(err, leaf))
    return err;
 
  if (CreateSearchPgmForLeaf(err, pgm, leaf)) 
    return err;

  
  _dcSearchCollector = &r_msgnums;
  
  // create predicate that represents range of message numbers and
  // add this predicate to the search progrm.
  char buffer[gkMinimumBufferLength];
  sprintf(buffer, "%ld:%ld", startmsgnum, endmsgnum);
  AddToSearchProgram(buffer, Sdm_False, pgm);  
  
  _PerformSearch(err, pgm);
  
  if (err != Sdm_EC_Closed) {
    // reset the pointer for the search collector.
    _dcSearchCollector = NULL;
  }
  
  SdmSearch::FreeLeaf(leaf);
  // don't need to delete pgm.  It's deleted by cclinet search is called.

  return err;
}

// Perform a search against a list of messages and return a list of messages that match
//
SdmErrorCode 
SdmDpDataChanCclient::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (r_msgnums.ElementCount() != 0)
    return (err = Sdm_EC_Fail);

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  SdmSearchLeaf *leaf;
  SEARCHPGM *pgm;
  SdmSearch *s = (SdmSearch*) &search;

  if (s->ParseSearch(err, leaf))
    return err;
 
  if (CreateSearchPgmForLeaf(err, pgm, leaf)) 
    return err;
 
  _dcSearchCollector = &r_msgnums;
  
  // create predicate that represents list of message numbers and
  // add this predicate to the search progrm.
  char buffer[gkMinimumBufferLength];
  SdmBoolean first = Sdm_True;
  // note: we need to cast out the const for msgnums below 
  // in order to use the iterator on it.
  SdmMessageNumberL* nc_msgnums = (SdmMessageNumberL*) &msgnums;
  SdmVectorIterator<SdmMessageNumber> iter(nc_msgnums);
  SdmString msgno_predicate;
  SdmMessageNumber *msgnum_ptr;
  
  while ((msgnum_ptr = iter.NextListElement()) != NULL) {
    if (*msgnum_ptr <= 0) {
      err = Sdm_EC_BadMessageNumber;
      return err;
    }
      
    if (first) {
      sprintf(buffer, "%ld", *msgnum_ptr);
      first = Sdm_False;
    } else {
      sprintf (buffer, ",%ld", *msgnum_ptr);
    }
    msgno_predicate += buffer;
  }
  AddToSearchProgram(msgno_predicate, Sdm_False, pgm);  

  // call cclient to execute search.  
  _PerformSearch(err, pgm);
  
  if (err != Sdm_EC_Closed) {
    // reset the pointer for the search collector.
    _dcSearchCollector = NULL;
  }
  
  SdmSearch::FreeLeaf(leaf);
  // don't need to delete pgm.  It's deleted by cclinet search is called.
  
  return err;
}


// Perform a search against a single message and return an indication of the search results
//
SdmErrorCode 
SdmDpDataChanCclient::PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// augmented results of search against message
  const SdmSearch& search,		// search criteria to apply to message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  SdmSearchLeaf *leaf;
  SEARCHPGM *pgm;
  SdmSearch *s = (SdmSearch*) &search;
  
  if (s->ParseSearch(err, leaf))
    return err;
 
  if (CreateSearchPgmForLeaf(err, pgm, leaf)) 
    return err;

  SdmMessageNumberL results;
  _dcSearchCollector = &results;
  
  // create predicate that represents the message number and
  // add this predicate to the search progrm.
  char buffer[gkMinimumBufferLength];
  sprintf(buffer, "%ld", msgnum);
  AddToSearchProgram(buffer, Sdm_False, pgm);  

  // call cclient to execute search.  
  _PerformSearch(err, pgm);
  
  if (err != Sdm_EC_Closed) {
    // check the results to see if it contains one item and it
    // matches the given message number.
    if (results.ElementCount() == 1 && results[0] == msgnum) { 
      r_bool = Sdm_True;
    } else {
      r_bool = Sdm_False;
    }

    // reset the pointer for the search collector.
    _dcSearchCollector = NULL;
  }
  
  SdmSearch::FreeLeaf(leaf);
  // don't need to delete pgm.  It's deleted by cclinet search is called.
  
  return err;  
}


//
// ** ET 
// this is the actual call that calls the cclient to perform 
// the search.
//
SdmErrorCode 
SdmDpDataChanCclient::_PerformSearch(SdmError &err, SEARCHPGM *pgm)
{
  err = Sdm_EC_Success;

  // We must call mail_check because of bug 4049782 - when it is fixed
  // this can be removed.  mail_check results in a call to mail_exists
  // which updates _dcStream->nmsgs to reflect the new number of
  // messages in the mailbox. We need to cause the short info
  // structure for all new messages to be fetched.
#if 1
  SdmMessageNumber num_new;

  if (_ReturnNumberOfNewMessages(err, num_new) != Sdm_EC_Success)
    return(err);

  unsigned long total_messages = _dcStream->nmsgs;

  if (num_new) {
    for (unsigned long msg_num = total_messages-num_new; msg_num <= total_messages; msg_num++) {
      // Always fetch SHORTINFO, since it's required when disconnected.
      // The SHORTINFO data is cached by the c-client.
      SHORTINFO sinfo;
      if (mail_fetchshort_full(_dcStream, msg_num, &sinfo, 0) != T) {
	if (!_dcRemote) {
	  mail_free_cached_buffers_msg(_dcStream, msg_num);
	}
	return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
      }
      if (!_dcRemote) {
	mail_free_cached_buffers_msg(_dcStream, msg_num);
      }
    }
  }
#endif

  // ** ET setting the search flag to SE_FREE will delete the 
  // pgm object when the search is complete.  Is this ok?
  mail_search_full(_dcStream, NULL, pgm, SE_FREE|SE_NOPREFETCH);
  mail_free_cached_buffers_seq(_dcStream, NIL, NIL);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  return err;
}


// Ping the IMAP server.
//
SdmErrorCode SdmDpDataChanCclient::PingServer
  (
  SdmError &err
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  // note: mail_ping results in a call to mail_exists which updates
  // _dcStream->nmsgs to reflect the new number of messages in the mailbox.
  if (mail_ping(_dcStream) != T) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

 
// Reconnect the currently disconnected object to its server
//
SdmErrorCode SdmDpDataChanCclient::Reconnect
  (
  SdmError &err
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  return (err = Sdm_EC_Fail);
}


// Rename the current object given a new token stream
//
SdmErrorCode SdmDpDataChanCclient::Rename
  (
  SdmError &err,
  const SdmString& currentname,		// current name for message store
  const SdmString& newname		// new name for message store
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcAttached || _dcOpen);
  assert(_dcStream);

  SdmString final_currentname, final_newname;

  if (_dcRemote) {
    assert(_dcHostname.Length());

    final_currentname = "{";
    final_currentname += _dcHostname;
    final_currentname += "}";

    final_newname = final_currentname;

    final_currentname += currentname;
    final_newname += newname;
  }
  else {
    if (SdmUtility::ExpandAndMakeAbsolutePath(err, final_currentname, currentname, Sdm_True) != Sdm_EC_Success)
      return(err);
    if (SdmUtility::ExpandAndMakeAbsolutePath(err, final_newname, newname, Sdm_True) != Sdm_EC_Success)
      return(err);
  }

  char *cname = (char*) (const char*)final_currentname;

  char *nname = (char*) (const char*)final_newname;

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  if (!mail_rename(_dcStream, cname, nname)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_CannotRenameMailbox));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
SdmErrorCode SdmDpDataChanCclient::RemoveFromSubscribedNamespace
  (
  SdmError &err,
  const SdmString &name			// name to be removed from the subscribed namespace
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcAttached || _dcOpen);

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  // Don't allow a "network" reference or pattern - this function
  // automatically prepends any necessary hostname prefix.
  if (name[0] == '{')
    return (err = Sdm_EC_Fail);

  SdmString full_name(name);

  // If remote, the reference argument must include the hostname.
  if (_dcRemote) {
    full_name = "{";
    full_name += _dcHostname;
    full_name += "}";
    full_name += name;
  }

  if (!mail_unsubscribe(_dcStream, (char*)(const char*)full_name)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  }

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

#endif

SdmErrorCode SdmDpDataChanCclient::_ScanNamespace
  (
  SdmError &err,
  SdmIntStrL &r_names,		// augmented list of names returned
  const SdmString &reference,	// reference portion of names to scan for
  const SdmString &pattern,	// pattern portion of names to scan for
  void (*func)(MAILSTREAM *stream, char *ref, char *pat) // ptr to either mail_list or mail_lsub
  )
{
  // These are asserts since the DataPort verifies this
  assert(_dcAttached || _dcOpen);
  assert(_dcStream);

  if (CheckStarted(err) != Sdm_EC_Success)
    return (err);

  // Don't allow a "network" reference or pattern - this function
  // automatically prepends any necessary hostname prefix.
  if (reference[0] == '{' || pattern[0] == '{')
    return (err = Sdm_EC_Fail);

  SdmString full_reference(reference);

  // If remote, the reference argument must include the hostname.
  if (_dcRemote) {
    full_reference = "{";
    full_reference += _dcHostname;
    full_reference += "}";
    full_reference += reference;
  }

  _dcScanCollector = &r_names;

  (*func)(_dcStream, (char*)(const char*)full_reference,
          (char*)(const char*)pattern);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  } 

  _HandleMailboxConnectionReadOnly();

  _dcScanCollector = NULL;

  return (err = r_names.ElementCount() ?
	  Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Cause the complete state of the message store to be saved
// 
SdmErrorCode SdmDpDataChanCclient::SaveMessageStoreState
  (
  SdmError &err
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(!_dcAttached);
  assert(_dcOpen);
  assert(_dcStream);

  // Checkpoint the mailbox - this writes any "dirty" messages (those with
  // changed status) back to disk.
  mail_check(_dcStream);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  } 

  _HandleMailboxConnectionReadOnly();

  return (err = Sdm_EC_Success);
}

// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
SdmErrorCode SdmDpDataChanCclient::ScanNamespace
  (
  SdmError &err,
  SdmIntStrL &r_names,		// augmented list of names returned
  const SdmString &reference,	// reference portion of names to scan for
  const SdmString &pattern	// pattern portion of names to scan for
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  return (_ScanNamespace(err, r_names, reference, pattern, mail_list));
}

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode SdmDpDataChanCclient::ScanSubscribedNamespace
  (
  SdmError &err,
  SdmIntStrL &r_names,		// augmented list of names returned
  const SdmString &reference,	// reference portion of names to scan for
  const SdmString &pattern	// pattern portion of names to scan for
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  return (_ScanNamespace(err, r_names, reference, pattern, mail_lsub));
}

#endif

SdmErrorCode SdmDpDataChanCclient::_HandleErrorExitConditions
  (
  SdmError &err,
  const SdmErrorCode defaultErrorCode
  )
{
  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
      
    SdmString cclientMessages = _dcCclientMessages;	// cache before object destroyed.
    SdmBoolean dcRemote = _dcRemote;			// cache remote status
    _HandleInvalidMailboxState(err);
    // WARNING!  Don't access any data members after calling
    // _HandleInvalidMailboxState.  This object is destroyed
    // at the end of this call.
    err = Sdm_EC_Closed;
    // If c-client messages were trapped, save them in the error object
    if (cclientMessages.Length()) {
      err.AddMinorErrorCodeAndMessage(dcRemote ? Sdm_EC_CCL_ServerErrorMessage : Sdm_EC_CCL_LocalErrorMessage, (const char *)cclientMessages);
    }
    return (err);
  }
  // See if the mailbox connection went read-only on us
  _HandleMailboxConnectionReadOnly();

#if defined(INCLUDE_SESSIONLOCKING)  
  // See if a session locking error or c-client error occurred; if so, import that error
  if (_dcLockError != Sdm_EC_Success) {
    err = _dcLockError;
    _dcLockError.Reset();	// reset this now that we've saved it
  }
  else
#endif // defined(INCLUDE_SESSIONLOCKING) 
  // Set the error object to any c-client trapped error, else to general failure
  if (_dcCclientError != Sdm_EC_Success) {
    err = _dcCclientError;
    _dcCclientError.Reset();	// reset this now that we've saved it
  }
  else {
    err = defaultErrorCode;
  }
  // If c-client messages were trapped, save them in the error object
  if (_dcCclientMessages.Length()) {
    err.AddMinorErrorCodeAndMessage(_dcRemote ? Sdm_EC_CCL_ServerErrorMessage : Sdm_EC_CCL_LocalErrorMessage, (const char *)_dcCclientMessages);
    _dcCclientMessages = "";	// reset this now that we've saved it
  }
  assert(err != Sdm_EC_Success);
  return(err);
}


SdmErrorCode SdmDpDataChanCclient::_SetMessageFlagValues
  (
  SdmError &err,
  const SdmBoolean flagValue, 
  const SdmMessageFlagAbstractFlags aflags,
  char *sequence
  )
{
  SdmMutexEntry entry(gCclientLock);
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  assert(_dcOpen);
  assert(_dcStream);
  assert(!_dcAttached);

  if (_dcReadonly)
    return (err = Sdm_EC_Readonly);

  if (aflags & ~Sdm_MFA_ALL)
    return (err = Sdm_EC_Fail);

  // Reset the c-client error and message objects
  _dcCclientError.Reset();		// initialize(clear) cclient error before cclient call.
  _dcCclientMessages = "";		// reset any cclient messages cached from last call.
#if defined(INCLUDE_SESSIONLOCKING)
  _dcLockError.Reset();			// initialize(clear) lock error before cclient call.
#endif // defined(INCLUDE_SESSIONLOCKING)

  void (*action)(MAILSTREAM *, char *, char *, long);
  action = flagValue? mail_set1flag_full : mail_clear1flag_full;

  if (aflags & Sdm_MFA_Deleted)
    (*action)(_dcStream, sequence, "\\DELETED", 0);

  if (aflags & Sdm_MFA_Answered)
    (*action)(_dcStream, sequence, "\\ANSWERED", 0);

  if (aflags & Sdm_MFA_Flagged)
    (*action)(_dcStream, sequence, "\\FLAGGED", 0);

  if (aflags & Sdm_MFA_Recent)
    (*action)(_dcStream, sequence, "\\RECENT", 0);

  if (aflags & Sdm_MFA_Seen)
    (*action)(_dcStream, sequence, "\\SEEN", 0);

  if (aflags & Sdm_MFA_Draft)
    (*action)(_dcStream, sequence, "\\DRAFT", 0);

  if ((_dcRemote && _dcServerConnectionBroken == Sdm_True) ||
      (!_dcRemote && _dcMailboxChangedByOtherUser == Sdm_True)) {
    return(_HandleErrorExitConditions(err,Sdm_EC_Fail));
  } 

  _HandleMailboxConnectionReadOnly();
  
  return (err = Sdm_EC_Success);
}

// Set a set of flags for a single message
//
SdmErrorCode SdmDpDataChanCclient::SetMessageFlagValues
  (
  SdmError &err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[32];
  sprintf(sequence, "%d", msgnum);

  return (_SetMessageFlagValues(err, flagValue, aflags, sequence));
}

// Set a set of flags for a range of messages
//
SdmErrorCode SdmDpDataChanCclient::SetMessageFlagValues
  (
  SdmError &err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(startmsgnum, endmsgnum))
    return (err = Sdm_EC_BadMessageNumber);

  char sequence[64];
  sprintf(sequence, "%d:%d", startmsgnum, endmsgnum);

  return (_SetMessageFlagValues(err, flagValue, aflags, sequence));
}

// Set a set of flags for a list of messages
//
SdmErrorCode SdmDpDataChanCclient::SetMessageFlagValues
  (
  SdmError &err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL &msgnums		// list of messages to operate on
  )
{
  SdmDpDataChanCclient::_dcLastDataChanUsed = this;

  if (_OutOfBounds(msgnums))
    return (err = Sdm_EC_BadMessageNumber);

  char *sequence;
  if ((sequence = _GenerateMessageSequence(msgnums)) == NULL)
    return (err = Sdm_EC_BadMessageSequence);

  _SetMessageFlagValues(err, flagValue, aflags, sequence);
  delete sequence;
  return err;
}

void SdmDpDataChanCclient::SetMailboxConnectionReadOnly
  (
  SdmBoolean isReadOnly
  )
{
  _dcMailboxConnectionReadOnly = isReadOnly;
}

void SdmDpDataChanCclient::SetServerConnectionBroken
  (
  SdmBoolean isBroken
  )
{
  _dcServerConnectionBroken = isBroken;
}

void SdmDpDataChanCclient::SetMailboxChangedByOtherUser
  (
  SdmBoolean isChanged
  )
{
  _dcMailboxChangedByOtherUser = isChanged;
}

long SdmDpCclientLookahead::defaultLookahead = 20;

SdmDpCclientLookahead::SdmDpCclientLookahead
 (
 MAILSTREAM *stream,
 SdmMessageNumber start,
 SdmMessageNumber end
 ) : _stream(stream), _lookaheadIncreased(Sdm_False)
{
  assert(_stream);

  long lookahead = end - start + 1;

  if (lookahead > SdmDpCclientLookahead::defaultLookahead) {
    mail_stream_setNIL(_stream);
    mail_parameters(_stream, SET_LOOKAHEAD, (void*)(lookahead));
    mail_stream_unsetNIL(_stream);
    _lookaheadIncreased = Sdm_True;
  }
}

SdmDpCclientLookahead::~SdmDpCclientLookahead()
{
  // If the lookahead value was increased, then reset it to the default
  if (_lookaheadIncreased) {
    mail_stream_setNIL(_stream);
    mail_parameters(_stream, SET_LOOKAHEAD, (void*)SdmDpCclientLookahead::defaultLookahead);
    mail_stream_unsetNIL(_stream);
  }
}

// SdmDpCclientMessageComponent derivative of SdmDpMessageComponent Implementation
SdmDpCclientMessageComponent::SdmDpCclientMessageComponent() : 
  _id(CclientComponentId), _bodyPart(NULL)
{
}

SdmDpCclientMessageComponent::~SdmDpCclientMessageComponent()
{
}

// Deep Copy
// Required to make a legitimate usable copy of a message component
SdmDpMessageComponent* SdmDpCclientMessageComponent::DeepCopy()
{
  SdmDpCclientMessageComponent *mcmp = new SdmDpCclientMessageComponent();
  mcmp->_section = _section;
  mcmp->_bodyPart = _bodyPart;
  return (mcmp);
}

// Print method
// Used in testing only
void SdmDpCclientMessageComponent::Print() const 
{ 
  printf("cc%s", (const char*) _section); 
}

// Equality operator: needs to be able to identify an cclient component
// and compare it with another to see if its identical in value
int SdmDpCclientMessageComponent::operator== 
  (
  const SdmDpMessageComponent& mcmp
  ) const
{
  SdmDpCclientMessageComponent* ccmcmp = (SdmDpCclientMessageComponent *)&mcmp;
  return(this
	 && ccmcmp
	 && _id == CclientComponentId 
	 && ccmcmp->_id == CclientComponentId
	 && ccmcmp->_section == _section
	 && ccmcmp->_bodyPart == _bodyPart);
}

#if defined(INCLUDE_SESSIONLOCKING)
// --------------------------------------------------------------------------------
// Session locking callbacks
// These static methods are registered with the c-client, which calls them when a
// locally accessible mailbox requires some kind of session locking to be performed.
//
// Because the return codes are consumed by the c-client, the only values
// recognized are 0 for failure, or 1 for success - Oui, Oui, je sais que
// c'est tres primitif. Le client du c rebarbative rote et rugit comme un
// vieux Parisian.
// --------------------------------------------------------------------------------

// _SessionLockCallback is called by a lock object that is holding a session lock
// that we requested on an object when a significant event occurs. The userdata 
// should be the this pointer for the c client data channel object on which the 
// mail_open was requested. We need to interpret the lock event and issue appropriate
// calls to the client that initiated the data port open call.
//
// There is no error code returned from this callback - it is expected to do its
// function regardless of success/failure.

void SdmDpDataChanCclient::_SessionLockCallback(const SdmLockEventType lockEvent, void* userdata)
{
  TNF_PROBE_1 (SdmDpDataChan__SessionLockCallback_start,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__SessionLockCallback_start",
	       tnf_ulong, lockEvent, (unsigned long) lockEvent);

  SdmError dummyError;

  // Get the this pointer of the C client channel adapter object that contains
  // the context for the object to be locked

  SdmDpDataChanCclient* myself = (SdmDpDataChanCclient*)userdata;
  assert(myself);

  // Act on the specific lock event.
  // We inform the client of this library of the event. In the case of locks going
  // away or other events where access to the mailbox must be terminated, after
  // the callback is queued up we close the mailbox, as we cannot wait for the
  // queued callback to be processed before we return from this callback, and once
  // we do return from this callback the process that caused this event to happen
  // is given permission to continue.

  switch (lockEvent) {
  case Sdm_LKEC_SessionLockGoingAway:		// another mailer taking lock
    // Another mailer has informed us that they are about to take the lock on this
    // mailbox away from us as soon as we return from this callback. Inform the
    // client of this library that the session lock on this mailbox has been taken
    // away and that the mailbox has been closed.
    myself->CallLockActivityCallback(DPEVENT_SessionLockGoingAway);
    (void) myself->_dcParentDataPort->Close(dummyError);
    break;
  case Sdm_LKEC_SessionLockTakenAway:		// another mailer stole lock from us
    // Wow - another mailer has informed us that they have taken the lock on this
    // mailbox away from us already - in theory the other process has already written
    // out new contents; however, there is no way we can prevent the c-client and
    // other cached changes from being written, so its the last writer wins here.
    myself->CallLockActivityCallback(DPEVENT_SessionLockTakenAway);
    (void) myself->_dcParentDataPort->Close(dummyError);
    break;
  case Sdm_LKEC_WaitingForSessionLock:		// entered a wait for a session lock
    // Waiting for a session lock - inform client so it can display status message
    myself->CallLockActivityCallback(DPEVENT_WaitingForSessionLock);
    break;
  case Sdm_LKEC_NotWaitingForSessionLock:	// no longer waiting for a session lock
    // No longer waiting for a session lock - inform client so it can undisplay status message
    myself->CallLockActivityCallback(DPEVENT_NotWaitingForSessionLock);
    break;
  case Sdm_LKEC_WaitingForUpdateLock:		// entered a wait for an update lock
    // Waiting for update lock - inform client so it can display status message
    myself->CallLockActivityCallback(DPEVENT_WaitingForUpdateLock);
    break;
  case Sdm_LKEC_NotWaitingForUpdateLock:	// no longer waiting for an update lock
    // No longer waiting for update lock - inform client so it can undisplay status message
    myself->CallLockActivityCallback(DPEVENT_NotWaitingForUpdateLock);
    break;
  default:					// unknown lock callback event
    // none of these events have any meaning for us - just ignore them
    break;
  }

  TNF_PROBE_0 (SdmDpDataChan__SessionLockCallback_end,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__SessionLockCallback_end");

  return;
}

#endif // defined (INCLUDE_SESSIONLOCKING)

// _EnableGroupPrivileges is called by the c-client whenever group privileges are
// required to perform certain operations.

int SdmDpDataChanCclient::_EnableGroupPrivileges(void)
{
  SdmDpDataChan::CallGroupPrivilegeActionCallback(Sdm_True);
  return 1;
}

int SdmDpDataChanCclient::_DisableGroupPrivileges(void)
{
  SdmDpDataChan::CallGroupPrivilegeActionCallback(Sdm_False);
  return 1;
}

#if defined(INCLUDE_SESSIONLOCKING)

// _ObtainSessionLock is called by the c-client whenever a session lock is required
// on a locally accessible mailbox. The userdata should be the this pointer for the
// c client data channel object on which the mail_open was requested. We use the
// _dcLockObject SdmLockUtility object to obtain the session lock. 
//
// We can set flags in the C client data channel object on failure so
// that specific errors can be returned as opposed to the C clients
// "yea/ney" return codes.
// 

int SdmDpDataChanCclient::_ObtainSessionLock(char *mailbox, void *userdata)
{
  TNF_PROBE_1 (SdmDpDataChan__ObtainSessionLock_start,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__ObtainSessionLock_start",
	       tnf_string, mailbox, mailbox ? mailbox : "(NIL)");

  assert(mailbox);
  assert(*mailbox);

  // Get the this pointer of the C client channel adapter object that contains
  // the context for the object to be locked

  SdmDpDataChanCclient* myself = (SdmDpDataChanCclient*)userdata;
  assert(myself);

  // Reset the lock callback specific error container - after an appropriate c-client
  // call this is checked to see if any locking error occurred

  myself->_dcLockError = Sdm_EC_Success;

  // Perform session locking only if session locking has not been disabled
  // Identify the object (mailbox) to be locked
  // and identify the callback for lock operations
  // and set the flag that determines whether or not to steal the lock from another mailer

  if ( (myself->_dcLockNoSessionLocking == Sdm_False)
       && (myself->_dcLockObject->IdentifyLockObject(myself->_dcLockError,mailbox) == Sdm_EC_Success)
       && (myself->_dcLockObject->IdentifyLockCallback(myself->_dcLockError,&_SessionLockCallback,userdata) == Sdm_EC_Success) 
       && (myself->_dcLockObject->GrabLockIfOwnedByOtherMailer(myself->_dcLockError,myself->_dcLockGrabIfOwnedByOther) == Sdm_EC_Success) ) {

    // Try and obtain the lock

    if (myself->_dcLockObject->ObtainLocks(myself->_dcLockError,Sdm_LKT_Session) != Sdm_EC_Success) {
      // If told to ignore session lock error, do so now
      if (myself->_dcLockIgnoreIfOwnedByOther) {
	myself->_dcLockError = Sdm_EC_Success;
      }
    }
  }

  TNF_PROBE_1 (SdmDpDataChan__ObtainSessionLock_end,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__ObtainSessionLock_end",
	       tnf_ulong, _dcLockError, (unsigned long)((SdmErrorCode)myself->_dcLockError));

  return(((SdmErrorCode) myself->_dcLockError) == Sdm_EC_Success);
}

// _ReleaseSessionLock is called by the c-client whenever a session lock is
// no longer required on a locally accessible mailbox. The userdata should be
// the this pointer for the c client data channel object on which the mail_open
// was requested. We use the _dcLockObject SdmLockUtility object to release
// the session lock. 
// 
// We can set flags in the C client data channel object on failure so
// that specific errors can be returned as opposed to the C clients
// "yea/ney" return codes; HOWEVER, its not clear that returning an error
// on a release of a session lock makes much sense: the mailbox is being
// closed, and if you cant release the lock, WHAT DO YOU DO TO RECOVER?
//

int SdmDpDataChanCclient::_ReleaseSessionLock(char *mailbox, void *userdata)
{
  TNF_PROBE_1 (SdmDpDataChan__ReleaseSessionLock_start,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__ReleaseSessionLock_start",
	       tnf_string, mailbox, mailbox ? mailbox : "(NIL)");

  SdmLockTypes lockTypes = 0;

  // callback must specify a mailbox even if we dont use it

  assert(mailbox);
  assert(*mailbox);

  // Get the this pointer of the C client channel adapter object that contains
  // the context for the object to be locked

  SdmDpDataChanCclient* myself = (SdmDpDataChanCclient*)userdata;
  assert(myself);

  // Reset the lock callback specific error container - after an appropriate c-client
  // call this is checked to see if any locking error occurred

  myself->_dcLockError = Sdm_EC_Success;

  // We can only get here if the lock was successfully obtained - make sure we either
  // have a session lock or were told to ignore a failure to get one

  myself->_dcLockObject->DetermineLocks(lockTypes);

  // Release the session lock only if we have it.

  // QUESTION: WHAT TO DO IF THE RELEASE FAILS? SHOULD WE JUST RETURN TRUE? WHAT
  // KIND OF ERROR RECOVERY IS POSSIBLE?

  if (lockTypes & Sdm_LKT_Session) {
    (void) myself->_dcLockObject->ReleaseLocks(myself->_dcLockError,Sdm_LKT_Session);
  }

  TNF_PROBE_1 (SdmDpDataChan__ReleaseSessionLock_end,
	       "dcapi lockapi SdmDpDataChanCclient",
	       "sunw%debug SdmDpDataChanCclient__ReleaseSessionLock_end",
	       tnf_ulong, _dcLockError, (unsigned long)((SdmErrorCode)myself->_dcLockError));

  return(((SdmErrorCode) myself->_dcLockError) == Sdm_EC_Success);
}

#endif // defined(INCLUDE_SESSIONLOCKING)
