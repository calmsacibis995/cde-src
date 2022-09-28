/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "Smtp" data channel subclass
// --> It implements an interface to smtp compatible transports

#pragma ident "@(#)DataChanSmtp.cc	1.31 97/02/25 SMI"

// Include Files.

#include <string.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChanSmtp.hh>
#include <Utils/CclientUtility.hh>
#include <SDtMail/MessageUtility.hh>
#include <SDtMail/SystemUtility.hh>
#include <PortObjs/DataChanCclient.hh>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

// Constants used by this module

static const char* HEADER_TO = "To";
static const char* HEADER_CC = "Cc";
static const char* HEADER_BCC = "Bcc";
static const char* HEADER_REPLY_TO = "Reply-To";

int SdmDpDataChanSmtp::_dcOneTimeFlag = 0;
SdmToken *SdmDpDataChanSmtp::_dcRefToken;

// Constructors

SdmDpDataChanSmtp::SdmDpDataChanSmtp(SdmDataPort* parentDataPort) :
  SdmDpDataChan(Sdm_CL_DataChannel, parentDataPort)
{
  _dcOpen = Sdm_False;		// no channel is open
  _dcAttached = Sdm_False;	// no channels are attached
}

// Destructor

SdmDpDataChanSmtp::~SdmDpDataChanSmtp() 
{
  if (_dcOpen) {
    SdmError err;
    Close(err);
  }
  if (_IsStarted) {
    SdmError err;
    ShutDown(err);
  }
}

SdmErrorCode SdmDpDataChanSmtp::StartUp(SdmError& err)
{
  assert(err == Sdm_EC_Success);

  if (_IsStarted)
    return(err = Sdm_EC_Success);

  if (!_dcOneTimeFlag) {
    // setup the reference token used to filter out incompatible requests
    //
    _dcRefToken = new SdmToken;		// create the reference token
    _dcRefToken->SetValue("servicechannel","smtp");
    _dcRefToken->SetValue("serviceclass","messagetransport");
    _dcRefToken->SetValue("servicetype","smtp");
    _dcOneTimeFlag++;
  }

  _IsStarted = Sdm_True;			// Elle est chaude! VF.

 return(err = Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChanSmtp::ShutDown(SdmError& err)
{
  SdmError localError;

  assert(err == Sdm_EC_Success);

  if (!_IsStarted)
    return(err = Sdm_EC_Success);

  _IsStarted = Sdm_False;

 return(err = Sdm_EC_Success);
}

// function SdmDataChanSmtp factory : creates a SdmDpDataChanSmtp object
//
SdmDpDataChan *SdmDpDataChanSmtpFactory(SdmDataPort* parentDataPort)
{
  return(new SdmDpDataChanSmtp(parentDataPort));
}

// --------------------------------------------------------------------------------
// DATA CHANNEL API - t client implementation
// This is the API that is exported to the users of the data channel
// --------------------------------------------------------------------------------

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode SdmDpDataChanSmtp::Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to attach to
  )
{
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);	// attach is not supported
}

// Cancel any operations pending on the data port
//
SdmErrorCode SdmDpDataChanSmtp::CancelPendingOperations
  (
  SdmError& err
  )
{
  assert(_IsStarted);

  return(err = Sdm_EC_Success);	// cancel is a nop so let it appear to succeed
}

// Close the current connection down
//
SdmErrorCode SdmDpDataChanSmtp::Close
  (
  SdmError& err
  )
{
  assert(_dcOpen);
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  if (_dcStream == NULL)
    return (Sdm_EC_Fail);

  _dcStream = smtp_close(_dcStream);
  _dcOpen = Sdm_False;

  return(err = Sdm_EC_Success);
}


// open a connection to an object given a token stream
//
SdmErrorCode SdmDpDataChanSmtp::Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(!_dcOpen);
  assert(!_dcAttached);

  r_readOnly = Sdm_False;

  // if already open, this is an error
  if (_dcOpen)
    return (err = Sdm_EC_Open);

  // clean up from any previous attempts (e.g. connects)
  if (_dcAttached) {
    if (Close(err))
      return (err);
  }

  // See if the token describing the object to open loosely matches
  // this data channel reference token for opening - if it does not
  // it means the object to open is incompatible with this data channel

  if (tk.CompareLoose(*_dcRefToken) != Sdm_True)
    return (err = Sdm_EC_Fail);

  char **hostnames = SdmMessageUtility::GetMailSendingHostNamesList();
  assert(hostnames);

  void *mstream = NULL;
  SdmString tokenParm;
  SdmDataPort *msgStoreDataPort = NULL;
  // smtp_open takes a MAILSTREAM* argument which is passed to mm_log
  // whenever error or other notification needs to be made.  If a
  // dataportobject token field is present, then this represents a pointer
  // to an SdmDataPort object that contains an open c-client channel adapter
  // with a valid MAILSTREAM*.
  tk.FetchValue(tokenParm, "dataportobject");
  const char *p = (const char*)tokenParm;
  if (p && *p) {
    sscanf(p, "%x", &msgStoreDataPort);
    msgStoreDataPort->MailStream(err, mstream);
  }
  else {
    // No dataportobject in the token - just create a temporary stream.
    mstream = mail_stream_create(NIL, NIL, NIL, NIL, NIL);
  }
  assert(mstream);
  _dcStream = smtp_open(hostnames, SOP_DEBUG, (MAILSTREAM*)mstream);

  if (!(p && *p))
    mail_stream_flush((MAILSTREAM*)mstream);

  SdmMessageUtility::DestroyMailSendingHostNamesList(hostnames);

  if (_dcStream == NULL)
    return (err = Sdm_EC_CannotConnectToSmtpMailServer);

  _dcOpen = Sdm_True;

  return (err = Sdm_EC_Success);	// open is not supported
}

// Determine if a message has any addressess that a transport can handle
// and if so resolve (dealias) and produce a list of them - preparing
// the message for transmission
//
SdmErrorCode SdmDpDataChanSmtp::ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  )
{
  SdmStrStrL toHeaders, ccHeaders, bccHeaders, replyToHeaders;
  SdmStringL toAddressList, ccAddressList, bccAddressList, replyToAddressList;
  SdmString toHeader, ccHeader, bccHeader, replyToHeader;
  SdmString allHeaders;

  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  // Return an error if the target data port is not open
  //
  if (dataPort.CheckOpen(err) != Sdm_EC_Success)
    return(err);

  // Fetch the three addressing headers that we know about
  // At least one "to" header is required, while the cc and bcc headers are optional
  // We check this requirement at the end when we know that this message is meant
  // to be sent via smtp.  It could be that this message is entirely meant for another
  // transport (eg. NNTP only).

  dataPort.GetMessageHeader(err, toHeaders, HEADER_TO, msgnum);
  if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound)
    return(err);

  // reset error in case it was set to Sdm_EC_RequestedDataNotFound.
  err = Sdm_EC_Success;
  dataPort.GetMessageHeader(err, ccHeaders, HEADER_CC, msgnum);
  if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound)
    return(err);
    
  // reset error in case it was set to Sdm_EC_RequestedDataNotFound.
  err = Sdm_EC_Success;
  dataPort.GetMessageHeader(err, bccHeaders, HEADER_BCC, msgnum);
  if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound)
    return(err);

  // reset error in case it was set to Sdm_EC_RequestedDataNotFound.
  err = Sdm_EC_Success;
  dataPort.GetMessageHeader(err, replyToHeaders, HEADER_REPLY_TO, msgnum);
  if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound)
    return(err);

  // reset error in case it was set to Sdm_EC_RequestedDataNotFound.
  err = Sdm_EC_Success;
  
  // return if the addressee lists are empty.
  if (toHeaders.ElementCount() == 0 && ccHeaders.ElementCount() == 0)
    {
      if (bccHeaders.ElementCount() == 0)
	err = Sdm_EC_NoAddressFoundInMessage;
      else
	// We are not allowing addressees to *only* be specified on the "bcc" line.
	// See bugs #1253368 and #1253261
	err = Sdm_EC_InsufficientAddresseeHeaders;

      return err;
    }
  
  // Reduce multiple header fields to a single header field each

  SdmMessageUtility::MergeHeaders(toHeader, toHeaders);
  SdmMessageUtility::MergeHeaders(ccHeader, ccHeaders);
  SdmMessageUtility::MergeHeaders(bccHeader, bccHeaders);
  SdmMessageUtility::MergeHeaders(replyToHeader, replyToHeaders);

  // Insert ghost commas into the headers;
  // These become permanent and appear in the final e-mail message sent.

  SdmMessageUtility::InsertGhostCommasInHeader(toHeader, toHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(ccHeader, ccHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(bccHeader, bccHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(replyToHeader, replyToHeader);

  // Parse each header field to see if the contents are grokable

  int errorIndex;

  if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, toHeader) != Sdm_EC_Success)
    return(err);
  if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, ccHeader) != Sdm_EC_Success)
    return(err);
  if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, bccHeader) != Sdm_EC_Success)
    return(err);
  if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, replyToHeader) != Sdm_EC_Success)
    return(err);

  // Expand the list of headers into an address list for each one

  SdmMessageUtility::ConvertHeadersToAddressList(toAddressList, toHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(ccAddressList, ccHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(bccAddressList, bccHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(replyToAddressList, replyToHeader);

  // Expand any aliases in the address list

  SdmMessageUtility::ExpandRFC822LocalAddressAliases(toAddressList);
  SdmMessageUtility::ExpandRFC822LocalAddressAliases(ccAddressList);
  SdmMessageUtility::ExpandRFC822LocalAddressAliases(bccAddressList);
  // don't need to expand aliases for reply-to header.

  // After alias expansion, extract any addressees that are obviously "files"

  SdmMessageUtility::ExtractFolderNamesFromAddressList(r_filenameList, toAddressList);
  SdmMessageUtility::ExtractFolderNamesFromAddressList(r_filenameList, ccAddressList);
  SdmMessageUtility::ExtractFolderNamesFromAddressList(r_filenameList, bccAddressList);
  // note: remove folder names for reply-to header but put them in a temporary list.
  SdmStringL tmpFileList;
  SdmMessageUtility::ExtractFolderNamesFromAddressList(tmpFileList, replyToAddressList);

  // Now that all of the address headers have been resolved and rewritten, we
  // need to reverify that sufficient addresses have been entered to satisfy RFC
  // 822 - return if the addressee lists are empty.

  if (toAddressList.ElementCount() == 0 && ccAddressList.ElementCount() == 0) {
    if (bccAddressList.ElementCount() == 0) {
      // No bcc's found - if the original to/cc headers had contents this means a file only
      // recipient was specified - allow that to happen
      if (toHeaders.ElementCount() == 0 && ccHeaders.ElementCount() == 0) {
	err = Sdm_EC_NoAddressFoundInMessage;
	return(err);
      }
    }
    else {
      // We are not allowing addressees to *only* be specified on the "bcc" line.
      // See bugs #1253368 and #1253261
      err = Sdm_EC_InsufficientAddresseeHeaders;
      return err;
    }
  }
  
  // Make sure that there is an X-Mailer header present in the message; if
  // not, put in a default X-Mailer header

  if (SdmMessageUtility::AddXMailerHeader(err, SdmString("libsdtmail"), dataPort,
                                          msgnum, Sdm_False) != Sdm_EC_Success)
    return(err);

  // Cause the address headers in the message to be rewritten
  // to reflect the final resolved addresses

  SdmMessageUtility::RewriteAddressHeader(err, HEADER_TO, toAddressList, dataPort, msgnum);
  SdmMessageUtility::RewriteAddressHeader(err, HEADER_CC, ccAddressList, dataPort, msgnum);
  SdmMessageUtility::RewriteAddressHeader(err, HEADER_BCC, bccAddressList, dataPort, msgnum);
  SdmMessageUtility::RewriteAddressHeader(err, HEADER_REPLY_TO, replyToAddressList, dataPort, msgnum);

  // Merge the individual address lists into one single address list
  // that is used for the actual sending process

  SdmMessageUtility::CompressAddressList(r_addressList, toAddressList);
  SdmMessageUtility::CompressAddressList(r_addressList, ccAddressList);
  SdmMessageUtility::CompressAddressList(r_addressList, bccAddressList);
  // don't need to include reply-to header to address list returned to caller.

  return (err = Sdm_EC_Success);
}

// Submit a fully constructed message for transmission
//
SdmErrorCode SdmDpDataChanSmtp::Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL& customHeaders
  )
{
  // Construct the envelope and body
  ENVELOPE *env = mail_newenvelope();
  SdmBoolean sunAttach = (msgFormat == Sdm_MSFMT_SunV3) ? Sdm_True : Sdm_False;

  if (SdmCclientUtility::ConstructMessageEnvelope(err, env, dataPort, msgnum, sunAttach)
      != Sdm_EC_Success) {
    mail_free_envelope(&env);
    return err;
  }
  BODY *body = mail_newbody();
  if (SdmCclientUtility::ConstructMessageBody(err, body, dataPort, msgnum, sunAttach)
      != Sdm_EC_Success) {
    mail_free_envelope(&env);
    mail_free_body(&body);
    return err;
  }

  HEADERLIST *headerList = NULL;
  if (customHeaders.ElementCount()) {
    SdmStrStrL hdrs;
    SdmErrorCode ec = dataPort.GetMessageHeaders(err, hdrs, customHeaders, msgnum);
    switch (ec) {
    case Sdm_EC_RequestedDataNotFound:
      err = Sdm_EC_Success;
      break;
    case Sdm_EC_Success:
      {
        STRINGLIST *fields = NULL;
        STRINGLIST *data = NULL;
        for (int i = 0; i < hdrs.ElementCount(); i++) {
          if (!fields) {
            headerList = mail_newheaderlist();
            headerList->fields = fields = mail_newstringlist();
            headerList->data = data = mail_newstringlist();
            headerList->n_entries = hdrs.ElementCount();
          }
          else {
            fields = fields->next = mail_newstringlist();
            data = data->next = mail_newstringlist();
          }
          // Use data but don't transmit the 2nd "From:"
          char *s = (char *)(const char *)hdrs[i].GetFirstString();
          if (!strcmp(s, "From")) {
            fields->text = data->text = NULL;
            continue;
          } else {
            fields->text = strdup(hdrs[i].GetFirstString());
            data->text = strdup(hdrs[i].GetSecondString());
          }
        }
      }
    break;
    default:
      return ec;
    }
  }

  // TODO: What do we pass as the name for the outgoing_log?
  void *mstream = NULL;
  dataPort.MailStream(err, mstream);
  assert(mstream);
  SdmDpDataChanCclient *dc = (SdmDpDataChanCclient *)(((MAILSTREAM*)mstream)->userdata);
  if (dc) {
    // Reset the c-client error and message objects
    dc->_dcCclientError.Reset();	// initialize(clear) cclient error before cclient call.
    dc->_dcCclientMessages = "";	// reset any cclient messages cached from last call.
  }
  int returnReceipt = 0;
  if (smtp_mail(_dcStream, "MAIL", env, body, NULL,
                msgFormat == Sdm_MSFMT_SunV3 ? SUNATTBF : MIMEBF,
                returnReceipt, headerList, (MAILSTREAM*)mstream) != T) {
    if (dc && dc->_dcCclientError != Sdm_EC_Success) {
      err = dc->_dcCclientError;
      dc->_dcCclientError.Reset();	// reset this now that we've saved it.
    }
    else {
      err = Sdm_EC_CannotSendMessage;	// Generic message for now, specifics later
    }
    // If c-client messages were trapped, save them in the error object
    if (dc && dc->_dcCclientMessages.Length()) {
      err.AddMinorErrorCodeAndMessage(Sdm_EC_SmtpServerErrorMessage, (const char *)dc->_dcCclientMessages);
      dc->_dcCclientMessages = "";	// reset this now that we've saved it
    }
    assert(err != Sdm_EC_Success);
  }
  else {
    err = Sdm_EC_Success;
  }

  mail_free_headerlist(&headerList);
  mail_free_envelope(&env);
  mail_free_body(&body);

  return err;
}

// --------------------------------------------------------------------------------
// Utility functions that are not part of the Data Channel API and are private to
// this interface are included below.
// --------------------------------------------------------------------------------

