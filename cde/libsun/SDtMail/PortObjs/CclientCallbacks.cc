/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This file implements the "C client" channel adapter's callbacks.

#pragma ident "@(#)CclientCallbacks.cc	1.30 97/06/11 SMI"

// There's no corresponding header file, since these functions are all
// declared in mail.h

#include <iostream.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <PortObjs/DataChanCclient.hh>
#include <os_sv5.h>

typedef enum {CclientError, CclientDebug, CclientNotify} CclientDisposition;
typedef struct {
  int cclientCode;
  CclientDisposition disposition;
  SdmErrorCode sdmCode;
} LogMessageTable;


// The following table maps c-client "error codes" to sdm error codes.  It
// also provides for a disposition that indicates how the error should be
// handled.  Note that "debug" errors are for internal use only (the
// "logdebug" .mailrc setting) and therefore do not require encapsulation
// in minor sdm error codes.  Note that the notification messages each have
// a corresponding minor code to facilitate internationalization.

static const LogMessageTable logMessageTable[] = {
  105, CclientNotify, Sdm_EC_CCL_CantCreateSessionLock,
  106, CclientNotify, Sdm_EC_CCL_WaitingForLock,
  107, CclientNotify, Sdm_EC_CCL_TryingForLock,
  108, CclientDebug,  0,
  109, CclientDebug,  0,
  115, CclientError,  Sdm_EC_CCL_IllegalFromLine,
  116, CclientError,  Sdm_EC_CCL_IllegalMailbox,
  117, CclientError,  Sdm_EC_CCL_MailboxShrank,
  // 131, Ignored - also delivered to mm_notify
  148, CclientError,  Sdm_EC_CCL_CantLoginToIMAPServer,
  154, CclientError,  Sdm_EC_CCL_LockFailure,
  159, CclientError,  Sdm_EC_CCL_MailboxContainsNulls,
  // 160, Ignored - also delivered to mm_notify
  161, CclientError,  Sdm_EC_CCL_MailboxInUse,
  162, CclientNotify, Sdm_EC_CCL_CantSetGid,
  163, CclientDebug,  0,
  164, CclientDebug,  0,
  165, CclientDebug,  0,
  166, CclientError,  Sdm_EC_CCL_MailboxReadError,
  167, CclientNotify, Sdm_EC_CCL_WaitingForDestinationLock,
  169, CclientError,  Sdm_EC_CCL_MailboxNoLongerExists,
  210, CclientError,  Sdm_EC_SmtpConnectionTimeout	// timout talking with smtp(sendmail) server
};

static SdmBoolean
HandleKnownMessages(SdmDpDataChanCclient *dc, char* text)
{
  char buf[4];
  int n, errorCode;

  // First extract the leading 3 digit cclient error code
  if (!text)
    return Sdm_False;
  if (!(buf[0] = text[0]) || !(buf[1] = text[1]) || !(buf[2] = text[2]))
    return Sdm_False;
  buf[3] = '\0';

  n = sscanf(buf, "%d", &errorCode);

  if (n != 1 || errorCode < 105 || errorCode > 167)
    return Sdm_False;

  SdmBoolean handled = Sdm_False;
  SdmError error;

  for (int i = 0; i < sizeof(logMessageTable) / sizeof(logMessageTable[0]); i++) {
    if (logMessageTable[i].cclientCode == errorCode) {
      switch (logMessageTable[i].disposition) {
      case CclientError:
        // Set an potential error, which is returned to the caller only if the call fails.
        dc->_dcCclientError = logMessageTable[i].sdmCode;
        handled = Sdm_True;

        // if the mailbox was changed by another user, then we need to set the
        // flag in the data channel that indicates this.  After the c-client
        // call is done, this flag will be checked and if set, the front end
        // will be informed that the mailbox has been changed.
        if (dc->IsOpen() && (logMessageTable[i].sdmCode == Sdm_EC_CCL_IllegalMailbox ||
            logMessageTable[i].sdmCode == Sdm_EC_CCL_MailboxShrank)) 
        {
            dc->SetMailboxChangedByOtherUser(Sdm_True);
        }

        break;
      case CclientDebug:
        dc->CallDebugLogCallback(SdmString(text));
        handled = Sdm_True;
        break;
      case CclientNotify:
        assert(logMessageTable[i].sdmCode);
        // Assign the sdm error code to an error object, the text of which
        // should be internationalized.
        error = logMessageTable[i].sdmCode;
        dc->CallNotifyLogCallback(SdmString(error.ErrorMessage()));
        handled = Sdm_True;
        break;
      }
    }
  }

  return (handled);
}

void mm_cache_size (MAILSTREAM *stream, unsigned long size,int done) {}

void mm_checksum(MAILSTREAM *stream, unsigned short checksum, long nbytes, char *driver) {}

void mm_critical (MAILSTREAM *stream) {}

long mm_diskerror (MAILSTREAM *stream,long errcode,long serious) { return 0L; }

// Debug logging
void mm_dlog (MAILSTREAM *stream, char *string)
{
  SdmDpDataChanCclient *dc = stream ? (SdmDpDataChanCclient *)stream->userdata : 0;
  if (!dc)
    dc = SdmDpDataChanCclient::LastDataChanUsed();
  if (dc) {
    dc->CallDebugLogCallback(SdmString(string));
  }
}

void mm_exists (MAILSTREAM *stream,unsigned long number) {}

void mm_expunged (MAILSTREAM *stream,unsigned long number)
{
  assert(stream);
  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;
  assert(dc);
  assert(dc->_dcExpungeCollector);

  SdmMessageNumberL &r_msgnums = *dc->_dcExpungeCollector;
  r_msgnums(-1) = number;
}

void mm_fatal (char *string, MAILSTREAM *mstream)
{
  SdmDpDataChanCclient *dc = SdmDpDataChanCclient::LastDataChanUsed();
  if (dc) {
    // Allow the front end to handle the error message
    dc->CallErrorLogCallback(SdmString(string));
    // Add this message to the cclient message cache
    if (string[0] != '\n')
      dc->_dcCclientMessages += "\n";
    dc->_dcCclientMessages += string;
  }
}

void mm_flags (MAILSTREAM *stream,unsigned long number) {}

void mm_io_callback(MAILSTREAM *stream, long nchars, int type)
{
  if (stream) {
    SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;
    assert(dc);

    // only set the server connection broken if the data channel is
    // not already closed.  This callback could be called as a result
    // of a close.  In this case, we don't need to handle the server
    // disconnect.
    if (type == CBSTRDEAD && dc->IsOpen() == Sdm_True) {
      //fprintf(stderr, "** Stream is dead!!\n");
      dc->SetServerConnectionBroken(Sdm_True);
    }
  }
}

void mm_list (MAILSTREAM *stream,char delimiter,char *mailbox,long attributes)
{
  assert(stream);
  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;
  assert(dc);
  assert(dc->_dcScanCollector);

  SdmNamespaceFlag nsflag = 0;
  int offset = 0;

  // If the mailbox is something like "{grimsby}/foobar", we need to skip
  // over the hostname and the curly braces.
  if (dc->_dcRemote) {
    assert (dc->_dcHostname.Length());
    offset = strlen((const char*)dc->_dcHostname) + 2;
  }

  if (attributes & LATT_NOINFERIORS)  nsflag |= Sdm_NSA_noInferiors;
  if (attributes & LATT_NOSELECT)     nsflag |= Sdm_NSA_cannotOpen;
  if (attributes & LATT_MARKED)       nsflag |= Sdm_NSA_changed;
  if (attributes & LATT_UNMARKED)     nsflag |= Sdm_NSA_unChanged;

  SdmIntStrL &r_names = *dc->_dcScanCollector;
  r_names[r_names.AddElementToVector()].SetNumberAndString
    (nsflag, mailbox + offset);
}

void mm_log_stream (MAILSTREAM *stream, char *text)
{
  SdmDpDataChanCclient *dc = stream ? (SdmDpDataChanCclient *)stream->userdata: 0;
  if (!dc) {
    dc->CallDebugLogCallback(SdmString("mm_log_stream called with no userdata"));
    // dc = SdmDpDataChanCclient::LastDataChanUsed();
    return;
  }

  if (!HandleKnownMessages(dc, text)) {
    SdmString s("Cclient Log (IGNORED): ");
    s += text;
    dc->CallDebugLogCallback(s);
  }

  // Add this message to the cclient message cache
  if (text[0] != '\n')
    dc->_dcCclientMessages += "\n";
  dc->_dcCclientMessages += text;
}

void mm_log (char *string, long errflg, MAILSTREAM *mstream)
{
  SdmDpDataChanCclient *dc = SdmDpDataChanCclient::LastDataChanUsed();
  char *category;

  switch (errflg) {
  case WARN:
    category = "(WARNING): ";
    break;
  case ERROR:
    category = "(ERROR): ";
    break;
  case PARSE:
    category = "(PARSE): ";
    break;
  case NIL:
  default:
    category = "(FYI): ";
    break;
  }

  // This function is not passed a stream argument, so the mailbox to which
  // the message applies cannot reliably be determined in a multi-threaded
  // environment.  Therefore, the message is only emitted to the debug log.
  // See mm_log_stream for errors that can be properly detected.
  if (dc) {
    size_t len;

    SdmString theMessage("mm_log ");
    theMessage += category;
    theMessage += string;
    dc->CallDebugLogCallback(theMessage);
    
    // hack: need to cache messages from imap server that may be errors
    if (errflg == NIL || errflg == ERROR) {
      if (string && *string) {
	if (string[0] != '\n')
	  dc->_dcCclientMessages += "\n";
	dc->_dcCclientMessages += string;
      }
    }

    // hack:  check for invalid mailbox format.  when this error occurs, 
    // mm_log_stream does not get called.  only mm_log gets called so we
    // need to detect this error here for now until c-client fixes their
    // error reporting.
    //
    if (errflg == ERROR && string && ((len = strlen(string)) > 30) && 
	strcmp(string+len-30, "is not in valid mailbox format") == 0) {
      dc->_dcCclientError = Sdm_EC_CCL_FileNotInMailboxFormat;
    }
  }
}

void mm_login (MAILSTREAM *stream, NETMBX *mbx,char *user,char *pwd,long trial)
{
  assert(stream);

  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;

  assert(dc);

  strcpy(user, (char *)(const char *)dc->_dcUsername);
  strcpy(pwd, (char *)(const char *)dc->_dcPassword);
}

// Called for each subscribed mailbox that matches the pattern
// provided to mail_lsub.
void mm_lsub (MAILSTREAM *stream,char delimiter,char *mailbox,long attributes)
{
  // Just call mm_list to accumulate the names
  mm_list(stream, delimiter, mailbox, attributes);
}

void mm_nocritical (MAILSTREAM *stream) {}

void mm_notify (MAILSTREAM *stream,char *string,long errflg)
{
  SdmDpDataChanCclient *dc;

  if (stream && stream->userdata)
    dc = (SdmDpDataChanCclient *)stream->userdata;
  else
    dc = SdmDpDataChanCclient::LastDataChanUsed();

  // If the notify message begins with a 3 digit string, then it
  // the status area.

  // Yikes - these notification messages are NOT internationalized.  The
  // most useful (131) relates to informing the user about the progress of
  // the open, in terms of number of headers parsed, etc.  This is "partly"
  // delivered to mm_log_stream (the useful data is omitted in the string
  // that's sent there).  Once the c-client is fixed to send the complete
  // string to mm_log_stream, mm_notify messages should probably be sent to
  // the debug log and mm_log_stream should be used to capture such
  // notifications (the position of the digits will have to be used to
  // extract the data for internationalization purposes).

  // Ce gross merde est encroyable, n'est pas?  JSC MV21 Can you believe it's 14 Nov 96?

  if (dc) {
    if (string && isdigit(string[0]) && isdigit(string[1]) &&
        isdigit(string[2]) && string[3] == ' ' && string[4])
      dc->CallNotifyLogCallback(SdmString(string+4));
    else
      dc->CallDebugLogCallback(SdmString(string));

    // Add this message to the cclient message cache
    if (string[0] != '\n')
      dc->_dcCclientMessages += "\n";
    dc->_dcCclientMessages += string;

    // supreme hack: need to detect the fact that the mailbox has been thrown
    // into read only mode. This comes back as an 'FYI' (NIL) that starts with
    // the text '[READ-ONLY]', such as: 
    //   [READ-ONLY] 128 Now READ-ONLY, mailbox lock surrendered
    // we need to recognize this and if it is present need to set a special
    // flag so that this information can be reflected back to the caller
    //
    if (string && (strlen(string) >= 11) 
	&& (strncasecmp(string, "[READ-ONLY]", 11) == 0)) {
      dc->SetMailboxConnectionReadOnly(Sdm_True);
    }
  }
}

void mm_restore_callback (void *userdata, unsigned long size, unsigned long count) {}

void mm_searched (MAILSTREAM *stream,unsigned long number)
{
  assert(stream);
  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;
  assert(dc);
  assert(dc->_dcSearchCollector);

  SdmMessageNumberL &r_msgnums = *dc->_dcSearchCollector;
  r_msgnums(-1) = number;
}

void mm_set_ignore (void) {}

void mm_status (MAILSTREAM *stream,char *mailbox,MAILSTATUS *status)
{
  assert(stream);

  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)stream->userdata;

  if (dc && dc->_dcStatusCollector) {
    dc->_dcStatusCollector->flags =          status->flags;
    dc->_dcStatusCollector->messages =       status->messages;
    dc->_dcStatusCollector->recent =         status->recent;
    dc->_dcStatusCollector->unseen =         status->unseen;
    dc->_dcStatusCollector->uidnext =        status->uidnext;
    dc->_dcStatusCollector->uidvalidity =    status->uidvalidity;
    dc->_dcStatusCollector->checksum =       status->checksum;
    dc->_dcStatusCollector->checksum_bytes = status->checksum_bytes;
  }
}

void mm_tcp_log (MAILSTREAM *stream, char *err_msg)
{
  SdmDpDataChanCclient *dc = stream ? (SdmDpDataChanCclient *)stream->userdata : 0;
  if (!dc)
    dc = SdmDpDataChanCclient::LastDataChanUsed();
    
  if (dc) {
    dc->CallDebugLogCallback(SdmString(err_msg));

    // Add this message to the cclient message cache
    if (err_msg[0] != '\n')
      dc->_dcCclientMessages += "\n";
    dc->_dcCclientMessages += err_msg;
  }
}
