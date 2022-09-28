/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "EditMsg" data channel subclass
// --> It implements the message editing pseudo data channel; while it
// --> provides a standard data channel interface to the data port, it is a
// --> special data channel known to the data port that is used in the
// --> creation and editing of messages. Each instance of an EditMsg data
// --> channel has the capability of editing exactly one message; multiple
// --> messages require multiple instances of this object.

#pragma ident "@(#)DataChanEditMsg.cc	1.45 97/05/13 SMI"

// Include Files.
#include <string.h>
#include <SDtMail/Sdtmail.hh>
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChanEditMsg.hh>
#include <SDtMail/MessageUtility.hh>
#include <Utils/CclientUtility.hh>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

// This macro is used to test whether or not a message number (given
// as an argument) is valid. Because there is one edit message pseudo
// channel adapter per message, the member data _emcMsgnum must match
// the actual message number of the message as it is referenced at the
// data port API level.

#define EDITMSG_MSGNUM_VALID(MSGNUM) (MSGNUM == _emcMsgnum)

static const char *CHANNEL_NAME = "datachanEditMsg";

// This table is used to define the insertion order preference of new message headers
// If a header is not found in this list it is appended to the end of the headers.
// Regrettably, this order is due in part to the way the c-client code re-orders
// headers when sending, and is done so messages "written to files" look like
// messages "sent over transports". If the c-client would only take the headers
// the way we want to set them, then this order could be independent...

static char *Header_Insertion_Order[] = {
  "return-path",
  "received",
  "date",
  "from",
  "reply-to",
  "sender",
  "subject",
  "to",
  "cc",
  "bcc",
  "message-id",
  "mime-version",
  "content-type",
  "x-attribution",
  "x-mailer",
  "content-transfer-encoding",
  "content-length",
  0
};

//-> This macro takes a C++ statement and executes it either as a method
//-> of _emcSrcChan if it is non-zero, else as a method of _emcSrcPort.
//-> It is used to gain access to the contents of the source for an
//-> edited message, due to the fact that the edited message does
//-> not have a unique message number associated with it, and any
//-> attempt to access such a message gets redirected to this adapter.
//

#define PICK_SOURCE(STATEMENT)	\
  {				\
    if (_emcSrcChan) {		\
      _emcSrcChan->STATEMENT;	\
    }				\
    else {			\
      assert(_emcSrcPort);	\
      _emcSrcPort->STATEMENT;	\
    }				\
  }

// fixupNewHeader - used to process a potential new header before insertion
// Need to strip off any trailing cr/lfs as all inserted headers must be free of
// these because the code to emit them places the proper line termination depending
// upon whether the message is being written in Unix file or SMTP RFC822 format
// NOTE: since this code removes these characters from the end of the headers, there
// may be a potential problem if the header consists of multi-byte 8-bit characters.

static SdmString fixupNewHeader(const SdmString& existingHeader)
{
  int ehlen = existingHeader.Length();
  int oehlen = ehlen;
  while (ehlen && (existingHeader(ehlen-1) == '\n' || existingHeader(ehlen-1) == '\r'))
    ehlen--;
  if (ehlen == oehlen)
    return (existingHeader);
  return SdmString((const char *)existingHeader, ehlen);
}

// Constructors

// This constructor creates an empty message that is suitable for both committment
// or discarding - it must be primed via CreateNewMessage or CreateDerivedMessage.

SdmDpDataChanEditMsg::SdmDpDataChanEditMsg
  (
   SdmDataPort* thisDp		// data port on which this new message will exist
  ) :
  SdmDpDataChan(Sdm_CL_DataChannel, thisDp), 
  _emcSrcChan(0), _emcSrcMsgnum(0), _emcSrcPort(0), _emcPort(thisDp), 
  _emcMsgnum(0), _emcEditedMessage(Sdm_False), _emcComponentIdentifierCeiling(0),
  _emcMessageFlags(0)
{
}

// This constructor creates a message derived from an existing message in a message
// store that is intended to be an "edited replacement" for the existing message.
// It can only be committed - it cannot be discarded.

SdmDpDataChanEditMsg::SdmDpDataChanEditMsg
  (
   SdmDataPort* thisDp, 		// data port on which this new message will exist
   SdmDataPort* srcDp, 			// data port on which the existing message exists
   const SdmMessageNumber srcMsgnum	// message number of existing message to derive from
   ) :
  SdmDpDataChan(Sdm_CL_DataChannel, thisDp), 
  _emcSrcChan(0), _emcSrcMsgnum(srcMsgnum), _emcSrcPort(srcDp), _emcMsgnum(0),
  _emcPort(thisDp), _emcEditedMessage(Sdm_True), _emcComponentIdentifierCeiling(0),
  _emcMessageFlags(0)
{
  // note that the contents of the derived message is created in startup
}

// Destructor

SdmDpDataChanEditMsg::~SdmDpDataChanEditMsg() 
{
  // Shutting down this object causes all storage to be freed up.
  // We set _emcEditedMessage to False so that the call to ShutDown does not 
  // insist on committing the changes to any message. The reason for this is
  // that this object is only destroyed by the data port, and the data port
  // will call ShutDown before destroying this object if its appropriate to
  // attempt to save changes.

  if (_IsStarted) {
    SdmError err;
    _emcEditedMessage = Sdm_False;
    ShutDown(err);
  }
}

SdmErrorCode SdmDpDataChanEditMsg::StartUp(SdmError& err)
{
  assert(err == Sdm_EC_Success);

  if (_IsStarted)
    return(err = Sdm_EC_Success);

  _emcMessageStructure.SetPointerDeleteFlag();

  // If this is an edited message (edited constructor called), then
  // cause a derived message of that message to be created

  if (_emcEditedMessage) {
    assert(_emcSrcPort);
    assert(_emcSrcMsgnum);
    if (CreateDerivedMessage(err, _emcSrcMsgnum, *_emcSrcPort, _emcSrcMsgnum) != Sdm_EC_Success)
      return(err);
  }

  _IsStarted = Sdm_True;			// Elle est chaude! VF.

 return(err = Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChanEditMsg::ShutDown(SdmError& err)
{
  SdmError localError;

  assert(err == Sdm_EC_Success);

  if (!_IsStarted)
    return(err = Sdm_EC_Success);

  // Either commit or discard any changes as appropriate
  // Any error is ignored because recovery is not possible due to this object
  // being shutdown after this call returns

  if (_emcEditedMessage) {
    SdmBoolean rewrittenFlag;
    (void) CommitPendingMessageChanges(localError, rewrittenFlag, _emcMsgnum, Sdm_False);
    _emcEditedMessage = Sdm_False;	// force discard to succeed
  }

  // Regardless of commit action, must discard any pending changes

  (void) DiscardPendingMessageChanges(localError, _emcMsgnum);

  // Release all control storage

  _emcSrcMsgnum = 0;
  _emcSrcPort = 0;
  _emcSrcChan = 0;
  _emcPort = 0;
  _emcMessageFlags = 0;

  _IsStarted = Sdm_False;

  return(err = Sdm_EC_Success);
}

// --------------------------------------------------------------------------------
// DATA CHANNEL API - edit message implementation
// This is the API that is exported to the users of the data channel
// --------------------------------------------------------------------------------

SdmDpMessageStructure* SdmDpDataChanEditMsg::CreateNewMessageBody
(
 const SdmMsgStrType bodytype, 
 const SdmString& bodysubtype
 )
{
  SdmDpEditMsgMessageComponent* emcmp = 
    new SdmDpEditMsgMessageComponent(++_emcComponentIdentifierCeiling);
  emcmp->_emccContentsFetched = Sdm_True;

  SdmDpMessageStructure* tmsgstr = new SdmDpMessageStructure();

  tmsgstr->ms_type = bodytype;
  tmsgstr->ms_component = emcmp;
  tmsgstr->ms_cached = Sdm_True;

  switch (bodytype) {
  case Sdm_MSTYPE_none:
  case Sdm_MSTYPE_text:
  case Sdm_MSTYPE_application:
  case Sdm_MSTYPE_audio:
  case Sdm_MSTYPE_image:
  case Sdm_MSTYPE_video:
  case Sdm_MSTYPE_other:
  case Sdm_MSTYPE_message:
    tmsgstr->ms_ms = 0;		// no nested message here
    break;
  case Sdm_MSTYPE_multipart:
    tmsgstr->ms_ms = new SdmDpMessageStructureL();
    break;
  default:
    assert(!bodytype);		// dont know what type of body this is!
    tmsgstr->ms_ms = 0;
    break;			// horrific assumption: single part added
  }

  tmsgstr->ms_subtype = bodysubtype;

  assert(tmsgstr);
  assert(tmsgstr->ms_component);
  return(tmsgstr);
}

// Add a message body to the top level of an existing message
//
SdmErrorCode SdmDpDataChanEditMsg::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Add a message body to the top level: any type is valid.

  SdmDpMessageStructure* nbp = CreateNewMessageBody(bodytype, bodysubtype);
  assert(nbp);

  _emcMessageStructure(-1) = nbp;
  r_msgstruct = *nbp;

  return (err = Sdm_EC_Success);
}

// Add a message body to a component of an existing message
//
SdmErrorCode SdmDpDataChanEditMsg::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Add a message body to a component: the component must be multipart capable

  if (!emstr->ms_ms)
    return (err = Sdm_EC_BadArgument);		// component not multipart nestable: error

  // Component is capable of holding another body - add new message body to the components end

  SdmDpMessageStructure* nbp = CreateNewMessageBody(bodytype, bodysubtype);
  assert(nbp);

  (*emstr->ms_ms)(-1) = nbp;
  r_msgstruct = *nbp;

  return (err = Sdm_EC_Success);
}

// Add additional header text to existing header for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::AddMessageHeader
  (
  SdmError& err, 
  const SdmString& hdr, 		// name of header to add
  const SdmString& newvalue, 		// new header value 
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  // Need to strip off any trailing cr/lfs as all inserted headers must be free of
  // these because the code to emit them places the proper line termination depending
  // upon whether the message is being written in Unix file or SMTP RFC822 format

  SdmString correctedNewValue = fixupNewHeader(newvalue);

  // Only perform the add if the header value is not empty

  if (correctedNewValue.Length() != 0) {

    // Determine insertion point
    // 1- if an instance of this header already exists, append the new header after the
    //    last occurance of the header
    // 2- else if this header is in the "insertion order" list,  scan backwards in the
    //    insertion order list and for each item see if the insertion order header exists; 
    //	if one is found, append this new header after the insertion header found
    // 4- else append this new header to the end of the headers list

    // Find the last occurance of this header in the header stream: if found, that becomes
    // the point after which we insert this header

    int numElements = _emcMessageHeaders.ElementCount();
    for (int i = numElements-1; i >= 0; i--) {
      if (hdr.CaseCompare(_emcMessageHeaders[i].GetFirstString()) == SdmString::Sdm_Equal) {
	// printf("found existing header: insert after %d\n", i);
	_emcMessageHeaders.InsertElementAfter(SdmStrStr(hdr,correctedNewValue), i);
	break;
      }
    }

    if (i == -1) {
      // The header has not been found, determine best place to insert based upon
      // insertion order preference table. First, find this entries place in the table.
      // If found, then find the highest existing header from the table

      for (char **p = Header_Insertion_Order; *p; p++) {
	if (hdr.CaseCompare(*p) == SdmString::Sdm_Equal)
	  break;
      }

      // If header not found in insertion list, simply append to the end of the headers list

      if (!*p) {
	// printf("not found in insertion order list: appending to end\n");
	_emcMessageHeaders(-1).SetBothStrings(hdr, correctedNewValue);
      }
      else {
	// Ok, header was found in the preferred order list
	// We now need to cycle through the list looking to see if one of the
	do {
	  for (i = numElements-1; i >= 0; i--) {
	    if (_emcMessageHeaders[i].GetFirstString().CaseCompare(*p) == SdmString::Sdm_Equal)
	      break;
	  }
	} while (i == -1 && p-- != Header_Insertion_Order);
      
	// If i is -1 there are no headers existing in the list that take precedence
	// over this header, so we can add it to the top of the list; otherwise, we
	// have found a header that should preceed this one, so add this header immediately
	// after it.

	if (i == -1) {
	  // printf("no preceeding insertion header found: inserting at beginning\n");
	  _emcMessageHeaders.InsertElementBefore(SdmStrStr(hdr,correctedNewValue), 0);
	}
	else {
	  // printf("insertion precedence header found: inserting after %d\n", i);
	  _emcMessageHeaders.InsertElementAfter(SdmStrStr(hdr,correctedNewValue), i);
	}
      }
    }
  }
  
  return (err = Sdm_EC_Success);
}

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode SdmDpDataChanEditMsg::Attach
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
SdmErrorCode SdmDpDataChanEditMsg::CancelPendingOperations
  (
  SdmError& err
  )
{
  assert(_IsStarted);

  return(err = Sdm_EC_Success);	// cancel is a nop so let it appear to succeed
}

// Close the current connection down
//
SdmErrorCode SdmDpDataChanEditMsg::Close
  (
  SdmError& err
  )
{
  assert(_IsStarted);

  return(err = Sdm_EC_Fail);	// close is not supported
}

// Commit any pending changes to a message
//
SdmErrorCode SdmDpDataChanEditMsg::CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messageRewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  SdmString theHeaders, theBody;

  // printf("Commit pending message changes for message %d\n", msgnum);

  // If this message has been deleted then dont bother committing it

  r_messageRewritten = Sdm_False;

  if (!(_emcMessageFlags & Sdm_MFA_Deleted)) {
    
    // Need to turn this message into an ascii mail message stream and cause
    // the data to be appended to the message store.
    
    r_messageRewritten = _emcEditedMessage;
    
    // Reconstruct the message from the component parts
    
    SdmMessageUtility::ConstructWholeMessage(err, theHeaders, theBody, *_emcPort, msgnum, Sdm_False, includeBcc, Sdm_False);

    // printf("commit pending changes: construct whole message returned %s\n", err.ErrorMessage());
    if (err == Sdm_EC_Success) {
      const char* hdrs = (const char*)theHeaders;
      const char* body = (const char*)theBody;
      // printf("headers: '%s'\nbody: '%s'\n", hdrs ? hdrs : "(null)", body ? body : "(null)");
      SdmString messageContents = theHeaders + theBody;
      assert(_emcPort);
      SdmString date;
      SdmString from;
      SdmMessageFlagAbstractFlags flags = 0;
      _emcPort->AppendMessage(err, messageContents, date, from, flags);
      if (err == Sdm_EC_Success && _emcEditedMessage) {
	assert(_emcSrcMsgnum == _emcMsgnum);
	assert(_emcSrcMsgnum);
	assert(_emcSrcChan);
	_emcSrcChan->SetMessageFlagValues(err, Sdm_True, Sdm_MFA_Deleted, _emcSrcMsgnum);
      }
    }
  }
  
  // If no error happened, cause this message to be discarded. If the commit failed,
  // some kind of error recovery can happen followed by another call to this method.
  // What this means on an error needs to be thought through better; however,
  // a failsafe discard occurs when this object is destroyed, as well as when 
  // this commit fails while being called from shutdown.

  if (err == Sdm_EC_Success) {
    SdmError localError;
    _emcEditedMessage = Sdm_False;		// This allows discard to not throw a fit
    DiscardPendingMessageChanges(localError, msgnum);
  }

  return(err);
}

// Create a new message in a message store
//
SdmErrorCode SdmDpDataChanEditMsg::CreateNewMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum	// message number of newly created message
  )
{
  _emcMsgnum = newmsgnum;		// Remember the message number for this new message

  // Since this is a new message, there are no headers, attributes, structures, etc.
  // so there isnt anything to do as the constructors initialize everything to
  // empty/null settings

  return (err = Sdm_EC_Success);
}

// Create a new message in a message store derived from an existing message
//
SdmErrorCode SdmDpDataChanEditMsg::CreateDerivedMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum,	// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  )
{
  SdmBoolean isRedirected = Sdm_False;
  assert(!_emcSrcPort || _emcSrcPort == &msgdp);
  assert(!_emcSrcMsgnum || _emcSrcMsgnum == msgnum);
  assert(!_emcSrcChan);

  // Return an error if the target data port is not open
  //
  if (msgdp.CheckOpen(err) != Sdm_EC_Success)
    return(err);

  _emcSrcPort = (SdmDataPort*)&msgdp;
  _emcSrcMsgnum = msgnum;
  _emcMsgnum = newmsgnum;

  // If this is an edited message, then we cannot refer to the source message by its
  // data port handle, because all such references are redirected back at this channel
  // adapter. So, if this is an edited message, we must determine the data channel on
  // which this message is derived and use that for all references to the original message.
  // The PICK_SOURCE macro is a useful crutch for this.

  if (_emcEditedMessage) {
    assert(_emcPort);
    assert(_emcPort == _emcSrcPort);
    assert(_emcSrcMsgnum == _emcMsgnum);
    _emcSrcChan = _emcSrcPort->MsgnumToChan(isRedirected, msgnum);
    assert(isRedirected == Sdm_False);	// Cannot be redirected yet as that is what is being done now
    assert(_emcSrcChan);
  } 
  else {
    assert(!_emcSrcChan);
  }

  // Need to derive this message from the original

  // Flags are easy: get complete flags bit mask

  PICK_SOURCE(GetMessageFlags(err, _emcMessageFlags, msgnum))
    if (err != Sdm_EC_Success)
      return(err);

  // Headers are easy: get complete headers list

  PICK_SOURCE(GetMessageHeaders(err, _emcMessageHeaders, msgnum));
  if (err != Sdm_EC_Success)
    return(err);

  // Message structure is messy: get the complete message structure, and then
  // "dope up" the message components so that we can insert our own data

  PICK_SOURCE(GetMessageStructure(err, _emcMessageStructure, msgnum));
  if (err != Sdm_EC_Success)
    return(err);

  DopeUpMessageStructure(_emcMessageStructure, _emcSrcPort, _emcSrcChan, _emcSrcMsgnum);

  // If this message is derived from a message that is redirected already,
  // we must fetch all of the contents so that if the redirected message
  // disappears we have a complete valid copy

  if (isRedirected)
    if (FetchAllContents(err, _emcMessageStructure) != Sdm_EC_Success)
      return(err);

  if (!_emcEditedMessage) {
    // Not an edited message - filter out certain headers which are not applicable
    // to a "derived copy" of the message (presumably because its going to be edited
    // or resent)
    SdmError localErr;

    (void) RemoveMessageHeader(localErr, "content-md5", _emcMsgnum);
    (void) RemoveMessageHeader(localErr, "x-uid", _emcMsgnum);
    (void) RemoveMessageHeader(localErr, "status", _emcMsgnum);
    (void) RemoveMessageHeader(localErr, "x-status", _emcMsgnum);
  }
  return (err = Sdm_EC_Success);
}

// Delete a message body from an existing message
//
SdmErrorCode SdmDpDataChanEditMsg::DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to delete
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Save current state and the set to the new state specified

  r_previousState = emcmp->_emccIsDeleted;
  emcmp->_emccIsDeleted = newState;

  return (err = Sdm_EC_Success);
}

// Discard any pending changes to a message
//
SdmErrorCode SdmDpDataChanEditMsg::DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // printf("Discard pending message changes for message %d\n", msgnum);

  // If this is an 'edited message' then changes cannot be discarded - return an error

  if (_emcEditedMessage)
    return (err = Sdm_EC_Operation_Unavailable);

  // Ok the message can be discarded - throw it away

  _emcMessageHeaders.ClearAllElements();
  _emcMessageStructure.ClearAndDestroyAllElements();

  return (err = Sdm_EC_Success);
}


#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Copy all attributes to returned list

  int numEntries = emcmp->_emccAttributes.ElementCount();
  for (int i = 0; i < numEntries; i++)
    r_attribute.AddElementToList(emcmp->_emccAttributes[i]);

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// augmented attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_attribute.Length() == 0);	// prove that the list is empty upon call

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Copy all matching attributes to returned list

  int numEntries = emcmp->_emccAttributes.ElementCount();
  for (int i = 0; i < numEntries; i++)
    if (::strcasecmp(attribute, emcmp->_emccAttributes[i].GetFirstString())==0) {
      r_attribute = emcmp->_emccAttributes[i].GetSecondString();
      return (Sdm_EC_Success);
    }

  // Did not find the attribute - return fail error

  return(err = Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  int numEntries = emcmp->_emccAttributes.ElementCount();
  int numTargAttributes = attributes.ElementCount();
  for (int i = 0; i < numEntries; i++) {
    for (int k = 0; k < numTargAttributes; k++)
      if (::strcasecmp((const char *)(attributes[k]), emcmp->_emccAttributes[i].GetFirstString())==0)
	r_attribute.AddElementToList(emcmp->_emccAttributes[i]);
    }

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  int numEntries = emcmp->_emccAttributes.ElementCount();

  for (SdmAbstractAttributeMap *aamp = _sdmAbstractAttributes; aamp->flag; aamp++) {
    if (attributes & aamp->flag) {
      assert(aamp->realName);
      for (int i = 0; i < numEntries; i++) {
	if (::strcasecmp(aamp->realName, emcmp->_emccAttributes[i].GetFirstString())==0)
	  r_attribute.AddElementToList(emcmp->_emccAttributes[i]);
      }
    }
  }

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif 
 
SdmErrorCode SdmDpDataChanEditMsg::GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,                 // augmented cached status
  const SdmMessageNumber msgnum,        // message number to get contents of
  const SdmDpMessageComponent &mcmp     // component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;
 
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
 
  int i = 0, j = 0, numHeaders = 0;
 
  // Transform the generic message component into the edit msg component
 
  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  r_cached = emcmp->_emccContentsFetched;

  return(err = Sdm_EC_Success);
}

// Return complete contents for the entire message, as opposed to a component
// of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_contents.Length() == 0);	// prove that return contents container is empty upon call

#if 0
  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);

  switch (contenttype) {
  case DPCTTYPE_rawfull:		// the entire message in raw format
  case DPCTTYPE_rawheader:		// the entire message header in raw format
  case DPCTTYPE_rawbody:		// the entire message body in raw format
    // Can drop down to generic "gather contents" function which knows how to
    // iterate through the pieces if needed
    //
    SdmString str_contents;
    gatherContents(str_contents, contenttype, msg->_tcmStructure);
    r_contents = str_contents;
    return(err = Sdm_EC_Success);
  case DPCTTYPE_processedcomponent:	// the "final contents" of the component in question
    return(err = Sdm_EC_Fail);
  default:
    return(err = Sdm_EC_Fail);
  }
  assert(0);
#endif
  return(err = Sdm_EC_Fail);
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_contents.Length() == 0);	// prove that return contents container is empty upon call

  int i = 0, j = 0, numHeaders = 0;

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  switch (contenttype) {
  case DPCTTYPE_rawfull:
    return(err = Sdm_EC_Fail);
  case DPCTTYPE_rawheader:		// the entire message header in raw format
    return(err = Sdm_EC_Fail);
  case DPCTTYPE_rawbody:		// the entire message body in raw format
    return(err = Sdm_EC_Fail);
  case DPCTTYPE_processedcomponent:	// the "final contents" of the component in question
    // This request is to get the "processed" contents
    // If the contents have not yet been fetched, cause them to be fetched

    if (FetchContents(err, (SdmDpMessageStructure&) *emstr, (SdmDpEditMsgMessageComponent&) *emcmp) != Sdm_EC_Success)
      return(err);

    r_contents = emcmp->_emccContents;
    return(err = Sdm_EC_Success);
  default:
    assert(0);
  }
  return(err = Sdm_EC_Fail);
}

// Return flags for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  r_aflags = _emcMessageFlags;

  return(err = Sdm_EC_Success);
}

// Return a list of flags for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return(err = Sdm_EC_Fail);		// get message flags for range is not supported
}

// Return a list of flags for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return(err = Sdm_EC_Fail);		// get message flags for list is not supported
}

// Return a list of all headers for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_hdr.ElementCount() == 0);		// prove that the list is empty upon call

  int numHeaders = _emcMessageHeaders.ElementCount();

  for (int i = 0; i < numHeaders; i++)
    r_hdr.AddElementToList(_emcMessageHeaders[i]);

  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for range is not supported
}

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a list is not supported
}

#endif

// Return a list of headers for a single header for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  int numHeaders = _emcMessageHeaders.ElementCount();

  for (int i = 0; i < numHeaders; i++)
    if (::strcasecmp((const char *)_emcMessageHeaders[i].GetFirstString(), (const char *)hdr)==0)
      r_hdr.AddElementToList(_emcMessageHeaders[i]);

  return(err = Sdm_EC_Success);
}

// Return a list of specific headers for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_hdrs.ElementCount() == 0);	// prove that the list is empty upon call

  int numHeaders = _emcMessageHeaders.ElementCount();
  int numTargHeaders = hdrs.ElementCount();

  for (int i = 0; i < numHeaders; i++) {
    for (int k = 0; k < numTargHeaders; k++)
      if (::strcasecmp((const char *)_emcMessageHeaders[i].GetFirstString(), (const char *)(hdrs[k]))==0) {
	r_hdrs.AddElementToList(_emcMessageHeaders[i]);
	break;
      }
  }

  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message header for a range is not supported
}

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message header for a list is not supported
}





// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a range is not supported
}


// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a list is not supported
}

#endif


// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  int numHeaders = _emcMessageHeaders.ElementCount();

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr & ahmp->flag) {
      if (ahmp->realName) {
	for (int i = 0; i < numHeaders; i++) {
	  if (::strcasecmp((const char *)_emcMessageHeaders[i].GetFirstString(),
                           (const char *)ahmp->realName)==0)
	    r_hdr(-1) = _emcMessageHeaders[i];
	}
      }
      else if (ahmp->indirectHeaderList) {
        SdmMessageHeaderAbstractFlag *abstHdrList = ahmp->indirectHeaderList;
        int numhdrs = r_hdr.ElementCount();

        for (int i = 0; abstHdrList[i]; i++) {
          const char *hdrnm = NULL;

          // Find the header name that corresponds to this pseudo header in
          // the indirect header list.
          for (SdmAbstractHeaderMap *hdrmap = _sdmAbstractHeaders; hdrmap->flag; hdrmap++) {
            if (hdrmap->flag == abstHdrList[i]) {
              hdrnm = hdrmap->realName;
              break;
            }
          }
          // There must exist a name entry in the header map for each
          // indirect header.
          assert(hdrnm);

          // Now search for this header in this message's header list
          for (int j = 0; j < numHeaders; j++) {
            if (::strcasecmp((const char *)_emcMessageHeaders[j].GetFirstString(),
                             hdrnm)==0)
	      r_hdr(-1) = _emcMessageHeaders[j];
          }
          // Once a matching header has been found in the indirect header list,
          // cease searching any further.
	  if (r_hdr.ElementCount() != numhdrs)
	    break;
	}
      }
    }
  }

  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a range is not supported
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a list is not supported
}



#endif

// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  int numHeaders = _emcMessageHeaders.ElementCount();

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr&  ahmp->flag) {
      if (ahmp->realName) {
	for (int i = 0; i < numHeaders; i++) {
	  if (::strcasecmp((const char *)_emcMessageHeaders[i].GetFirstString(), (const char *)ahmp->realName)==0)
	    r_hdr(-1).SetNumberAndString(ahmp->flag, (_emcMessageHeaders[i]).GetSecondString());
	}
      }
      else if (ahmp->indirectHeaderList) {
        SdmMessageHeaderAbstractFlag *abstHdrList = ahmp->indirectHeaderList;
        int numhdrs = r_hdr.ElementCount();

        for (int i = 0; abstHdrList[i]; i++) {
          const char *hdrnm = NULL;

          // Find the header name that corresponds to this pseudo header in
          // the indirect header list.
          for (SdmAbstractHeaderMap *hdrmap = _sdmAbstractHeaders; hdrmap->flag; hdrmap++) {
            if (hdrmap->flag == abstHdrList[i]) {
              hdrnm = hdrmap->realName;
              break;
            }
          }
          // There must exist a name entry in the header map for each
          // indirect header.
          assert(hdrnm);

          // Now search for this header in this message's header list
          for (int j = 0; j < numHeaders; j++) {
            if (::strcasecmp((const char *)_emcMessageHeaders[j].GetFirstString(),
                             hdrnm)==0)
	      r_hdr(-1).SetNumberAndString(abstHdrList[i],
                                           (_emcMessageHeaders[j]).GetSecondString());
          }
          // Once a matching header has been found in the indirect header list,
          // cease searching any further.
	  if (r_hdr.ElementCount() != numhdrs)
	    break;
	}
      }
    }
  }

  return(err = Sdm_EC_Success);
}

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a range is not supported
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message headers for a list is not supported
}

// Return the message structure for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(r_structure.ElementCount() == 0);	// prove that the structure list is empty upon call


  SdmDpMessageStructureL *msgstrs = &_emcMessageStructure;
  if (!msgstrs)
    return(Sdm_EC_Fail);
  int numstrs = msgstrs->ElementCount();
  if (!numstrs)				// Is there no structure to this message??
    return(Sdm_EC_Success);		// None: it means the message is newly created

  for (int j = 0; j < numstrs; j++) {
    SdmDpMessageStructure *targmsgstr = (*msgstrs)[j];
    assert(targmsgstr);
    SdmDpMessageStructure *dpms = new SdmDpMessageStructure(*targmsgstr);
    assert(dpms);
    r_structure.AddElementToList(dpms);
  }
  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message structure for a range is not supported
}

// Return a list of message structures for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);		// get message structure for a list is not supported
}

#endif

// open a connection to an object given a token stream
//
SdmErrorCode SdmDpDataChanEditMsg::Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return (err = Sdm_EC_Fail);	// open is not supported
}

// Remove a specific message header from a single message
//
SdmErrorCode SdmDpDataChanEditMsg::RemoveMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be removed
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  // spin through the list and delete all matching headers

  int numElements = _emcMessageHeaders.ElementCount();
  for (int i = numElements-1; i >= 0; i--) {
    if (hdr.CaseCompare(_emcMessageHeaders[i].GetFirstString()) == SdmString::Sdm_Equal) {
      _emcMessageHeaders.RemoveElement(i);	
    }
  }

  return (err = Sdm_EC_Success);
}

// Replace specific header completely with new value for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::ReplaceMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be replaced
  const SdmString& newvalue,		// new header value
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  // spin through the list and delete all matching headers, remembering the position of
  // of the first such header in the headers list

  int numElements = _emcMessageHeaders.ElementCount();
  int lastElement = -1;

  for (int i = numElements-1; i >= 0; i--) {
    if (hdr.CaseCompare(_emcMessageHeaders[i].GetFirstString()) == SdmString::Sdm_Equal) {
      _emcMessageHeaders.RemoveElement(i);
      lastElement = i;
    }
  }

  // If lastElement is -1 then no matching headers were found, treat this as a simple
  // AddMessageHeader; otherwise, insert this new header before the position where
  // the first such header was found

  if (i < 0) {
    AddMessageHeader(err, hdr, newvalue, msgnum);
  }
  else {
    _emcMessageHeaders.InsertElementBefore(SdmStrStr(hdr,fixupNewHeader(newvalue)),lastElement);
    err = Sdm_EC_Success;
  }

  return (err);
}

#ifdef INCLUDE_UNUSED_API

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageAttribute
  (
  SdmError& err,
  const SdmString& attribute,		// name of attribute to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // First, zip through and delete all matching attributes

  int numEntries = emcmp->_emccAttributes.ElementCount();
  for (int i = numEntries-1; i >= 0; i--)
    if (::strcasecmp(attribute, emcmp->_emccAttributes[i].GetFirstString())==0) {
      emcmp->_emccAttributes.RemoveElement(i);
    }

  // If the new value is empty, then we have done our job by just removing
  // those that may have been set: return; otherwise, need to set the attribute

  if (attribute_value.Length() != 0)
    emcmp->_emccAttributes(-1).SetBothStrings(attribute,attribute_value);

  return (err = Sdm_EC_Success);
}

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageAttribute
  (
  SdmError& err,
  const SdmMessageAttributeAbstractFlags attribute,	// abstract attribute value to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;
  SdmString realAttributeName;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Find the attribute name for this attribute

  for (SdmAbstractAttributeMap *aamp = _sdmAbstractAttributes; aamp->flag; aamp++) {
    if (attribute & aamp->flag) {
      assert(aamp->realName);
      realAttributeName = aamp->realName;
      break;
    }
  }

  assert(aamp->flag && realAttributeName.Length());

  // zip through and delete all matching attributes

  int numEntries = emcmp->_emccAttributes.ElementCount();
  for (int i = numEntries-1; i >= 0; i--)
    if (::strcasecmp(realAttributeName, emcmp->_emccAttributes[i].GetFirstString())==0) {
      emcmp->_emccAttributes.RemoveElement(i);
    }

  // If the new value is empty, then we have done our job by just removing
  // those that may have been set: return; otherwise, need to set the attribute

  if (attribute_value.Length() != 0)
    emcmp->_emccAttributes(-1).SetBothStrings(realAttributeName,attribute_value);

  return (err = Sdm_EC_Success);
}

#endif


// Set the complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageContents
  (
  SdmError& err,
  const SdmContentBuffer& contents,	// new contents for component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // Override the contents of this component

  emcmp->_emccContents = contents;		// replace existing contents with new data
  emcmp->_emccContentsFetched = Sdm_True;	// prevent contents from being brought over again
  emstr->ms_bytes = contents.Length();
  emstr->ms_cached = Sdm_True;

  return (err = Sdm_EC_Success);
}

// Set a set of flags for a single message
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  if (aflags) {
    if (flagValue)
      _emcMessageFlags |= aflags;
    else
      _emcMessageFlags &= (~aflags);
  }

  return (err = Sdm_EC_Success);
}

// Set a set of flags for a range of messages
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return(err = Sdm_EC_Fail);			// set message flag values for range not supported
}

// Set a set of flags for a list of messages
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  return(err = Sdm_EC_Fail);			// set message flag values for list not supported
}

// Set all headers for a single message, replacing all existing headers
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageHeaders
  (
  SdmError& err,
  const SdmStrStrL& hdr,		// list of header name/values to set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);

  // Clear out the old list, and then for each header in the new list, cause it to
  // be added to the list of headers - this takes care of empty headers and headers
  // with invalid characters

  _emcMessageHeaders.ClearAllElements();

  // Spin through all of the headers and remove any that have null values

  int numElements = hdr.ElementCount();

  for (int i = 0; i < numElements; i++) {
    if (AddMessageHeader(err, hdr[i].GetFirstString(), hdr[i].GetSecondString(), msgnum) != Sdm_EC_Success)
      i = numElements;
  }

  return (err = Sdm_EC_Success);
}

// Set the message structure for a component of a message
//
SdmErrorCode SdmDpDataChanEditMsg::SetMessageBodyStructure
  (
  SdmError& err,
  const SdmDpMessageStructure& msgstr,	// new message structure value for the specified component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  SdmDpEditMsgMessageComponent* emcmp;
  SdmDpMessageStructure* emstr;

  assert(EDITMSG_MSGNUM_VALID(msgnum));

  // Transform the generic message component into the edit msg component

  if (LocalizeMessage(err, emcmp, emstr, mcmp, _emcMessageStructure, msgnum) != Sdm_EC_Success)
    return(err);

  // If this is a nested component, we cannot change any fields 

  if (emstr->ms_ms)
    return (err = Sdm_EC_BadArgument);		// component is multipart nestable: error

  // Kick out those things that changed that cannot or are bad values

  if (!msgstr.ms_subtype.Length())		// subtype must not be null (must have content)
    return (err = Sdm_EC_BadArgument);

  if (emstr->ms_encoding != msgstr.ms_encoding)	// encoding cannot be changed
    return (err = Sdm_EC_BadArgument);

  // if this is a message/rfc822 body, then we need to check that
  // the from line is in the beginning of the contents.        

  if (msgstr.ms_type == Sdm_MSTYPE_message && 
      msgstr.ms_subtype.Length() > 0 &&
      ::strcasecmp((const char*)msgstr.ms_subtype, "rfc822") == 0) {
    SdmString strContents;
    if (FetchContents(err, (SdmDpMessageStructure&) *emstr, (SdmDpEditMsgMessageComponent&) *emcmp) != Sdm_EC_Success)
      return(err);
    if (!emcmp->_emccContents.GetContents(err, strContents)) {
      const char* p = (const char*)strContents;
      
      // SkipPastFromLine should return a different pointer
      // from the original if a from line exists. 
      if (SdmCclientUtility::SkipPastFromLine(p) == p) {
	return(err = Sdm_EC_MessageRFC822MissingFromLine);
      }
    }
  }

  // Ok, certain fields in the message structure are settable - copy in those
  // that we allow to be changed.

  // Allow the primary mime type to change, but only if it remains non-multi-part

  emstr->ms_subtype = msgstr.ms_subtype;

  switch (msgstr.ms_type) {
  case Sdm_MSTYPE_text:
  case Sdm_MSTYPE_application:
  case Sdm_MSTYPE_audio:
  case Sdm_MSTYPE_image:
  case Sdm_MSTYPE_video:
  case Sdm_MSTYPE_message:		
    emstr->ms_type = msgstr.ms_type;
    break;
  case Sdm_MSTYPE_other:		// do not allow changing the type to other (why?)
  case Sdm_MSTYPE_none:			// do not allow changing the type to none (why would you want to?)
  case Sdm_MSTYPE_multipart:		// cannot become a multipart
    return (err = Sdm_EC_BadArgument);
  default:
    assert(!msgstr.ms_type);		// dont know what type of body this is!
    return (err = Sdm_EC_BadArgument);
  }

  // Allow the following informational fields to be updated

  emstr->ms_disposition = msgstr.ms_disposition;	// component disposition: unspecified, inline, attachment
  emstr->ms_id = msgstr.ms_id;				// Message identification: message/content id
  emstr->ms_description = msgstr.ms_description; 	// Message description: content description
  emstr->ms_attachment_name = msgstr.ms_attachment_name;	// attachment name (if present)
  emstr->ms_unix_mode = msgstr.ms_unix_mode;		// "unix mode" hint for saving to file (if non-zero)

  return (err = Sdm_EC_Success);
}

// --------------------------------------------------------------------------------
// Utility functions that are not part of the Data Channel API and are private to
// this interface are included below.
// --------------------------------------------------------------------------------

void SdmDpDataChanEditMsg::AdjustMessageStructure
  (
   SdmDpMessageStructureL& msgstr,
   SdmMessageNumber newMsgnum
  )
{
  int numstrs = msgstr.ElementCount();
  for (int i = 0; i < numstrs; i++) {

    // Setup our message component

    SdmDpMessageStructure *tmsgstr = msgstr[i];
    SdmDpEditMsgMessageComponent *tmcmp = (SdmDpEditMsgMessageComponent*)tmsgstr->ms_component;
    tmcmp->_emccOrigMessageNumber = newMsgnum;

    // If a nested component, recursively call this method to process it

    if (tmsgstr->ms_ms)
      AdjustMessageStructure(*tmsgstr->ms_ms, newMsgnum);
  }
}

// This method takes a complete message structure imported for an existing
// message (via GetMessageStructure) and dopes up all of the message components
// so that they point to a SdmDpEditMsgMessageComponent instead. That component
// has all of the information necessary to manage the component parts for
// editing/creating purposes, and it contains a pointer to the original
// message component so that it can be accessed as required.

void SdmDpDataChanEditMsg::DopeUpMessageStructure
  (
  SdmDpMessageStructureL& msgstr,	// message structure of derived message
  const SdmDataPort* dp,		// data port where derived message exists
  const SdmDpDataChan* dc,		// data chan where derived message exists
  const SdmMessageNumber msgnum		// message number of derived message
  )
{
  // spin through the message structure doping up each component; we create
  // a EditMsgMessageComponent and then copy data or add new data so that
  // the component has all data necessary to manage this component

  // if a nested component is encountered, recursively call this method
  // to dope that nested component as well

  int numstrs = msgstr.ElementCount();
  for (int i = 0; i < numstrs; i++) {
    SdmError err;
    SdmDpEditMsgMessageComponent *nmcmp = 
      new SdmDpEditMsgMessageComponent(++_emcComponentIdentifierCeiling);
    assert(nmcmp);

    // Setup our message component

    SdmDpMessageStructure *tmsgstr = msgstr[i];
    // note that we own the component in tmgstr so we don't need to create
    // a copy of it.
    nmcmp->_emccOrigComponent = tmsgstr->ms_component;
    tmsgstr->ms_component = nmcmp;
    nmcmp->_emccOrigPort = (SdmDataPort*)dp;
    nmcmp->_emccOrigChan = (SdmDpDataChan*)dc;
    nmcmp->_emccOrigMessageNumber = msgnum;
    
    // need to set the IsDeleted flag based on the original component.
    if (nmcmp->_emccOrigComponent) {
      nmcmp->_emccIsDeleted = nmcmp->_emccOrigComponent->IsComponentDeleted();
    }


#ifdef INCLUDE_UNUSED_API
    // Fetch the attributes associated with this component
    if (nmcmp->_emccOrigChan)
      nmcmp->_emccOrigChan->GetMessageAttributes(err, 
						 nmcmp->_emccAttributes, 
						 nmcmp->_emccOrigMessageNumber, 
						 *nmcmp->_emccOrigComponent);
    else
      nmcmp->_emccOrigPort->GetMessageAttributes(err, 
						 nmcmp->_emccAttributes, 
						 nmcmp->_emccOrigMessageNumber, 
						 *nmcmp->_emccOrigComponent);
#endif

    // If a nested component, recursively call this method to process it

    if (tmsgstr->ms_ms)
      DopeUpMessageStructure(*tmsgstr->ms_ms, dp, dc, msgnum);
  }
}

// This method causes the contents of a particular component of a message
// to be fetched from the source message

SdmErrorCode SdmDpDataChanEditMsg::FetchContents
  (
  SdmError& err, 
  SdmDpMessageStructure& emstr,
  SdmDpEditMsgMessageComponent& emcmp
  )
{
  assert(emcmp._emccIdentifier);

  if (emcmp._emccContentsFetched)
    return(err = Sdm_EC_Success);

  PICK_SOURCE(GetMessageContents(err, 
				 emcmp._emccContents, 
				 DPCTTYPE_processedcomponent, 
				 emcmp._emccOrigMessageNumber, 
				 *emcmp._emccOrigComponent));

  // Question of how to handle failure: what happens if a expunge or disconnect
  // happens, and as a result we try and cache up all data but the get fails: 
  // should we act as though the get succeeded??

  if (err == Sdm_EC_Success) {
    emcmp._emccContentsFetched = Sdm_True;
    emstr.ms_cached = Sdm_True;
  }

  return(err);
}

// This method causes the contents of all components of a message
// to be fetched from the source message

SdmErrorCode SdmDpDataChanEditMsg::FetchAllContents
  (
  SdmError& err,
  SdmDpMessageStructureL& msgstr	// message structure to fetch up
  )
{
  int numstrs = msgstr.ElementCount();
  for (int i = 0; i < numstrs; i++) {
    SdmError localError;
    SdmDpMessageStructure* mstr = msgstr[i];
    assert(mstr);
    SdmDpEditMsgMessageComponent* emcmp = (SdmDpEditMsgMessageComponent*)mstr->ms_component;
    assert(emcmp);

    FetchContents(localError, *mstr, *emcmp);

    if (mstr->ms_ms)
      FetchAllContents(localError, *mstr->ms_ms);
  }

  return(err);
}

// ReviseOriginalMessageNumber
// This method is used to cause the message numbers associated with this
// message to be revised. There are two message numbers to worry about:
//  -> the original source message number; this is the message number of the
//	message that this message is derived from.
//  -> the message number of this message; this is the message number that this
//	message is known by. In the case of a new or derived message, it is unique in
//	the message number space (cause it does not exist in a message store yet); in
//	the case of an edited message, it is the message number of the original message
//	that this message is derived from (same as original source message number).
//
SdmErrorCode SdmDpDataChanEditMsg::ReviseOriginalMessageNumber
  (
  SdmError& err, 
  const SdmMessageNumber newMsgnum,
  const SdmMessageNumber newSrcMsgnum
  )
{
  // For the edit message pseudo channel adapter, ReviseOriginalMessageNumber is
  // called after an expunge or other operation has been called on the real data port
  // associated with this edited message that causes the original message number on
  // the original data port to change - we are passed in the new message number and
  // adjust our idea of this message's original number accordingly

  // If this is not a derived message, it is a new message without an original on the
  // data port associated with this message; therefore, no message number update is required

  // printf("revise called B4 : edited %s msgnum %d srcmsgnum %d\n", _emcEditedMessage ? "Y" : "N", _emcMsgnum, _emcSrcMsgnum);

  if (_emcSrcMsgnum) {
    assert(_emcSrcMsgnum > 0);

    // Always adjust the source message number for this derived message

    _emcSrcMsgnum = newSrcMsgnum;

    // If this is an edited message, also revise it

    if (_emcEditedMessage)
      _emcMsgnum = newMsgnum;

    // Now must adjust this message and all of the message components associated with this message

    AdjustMessageStructure(_emcMessageStructure, newSrcMsgnum);
  }

  // printf("revise called AF : edited %s msgnum %d srcmsgnum %d\n", _emcEditedMessage ? "Y" : "N", _emcMsgnum, _emcSrcMsgnum);

  return (err = Sdm_EC_Success);
}

// This is called by the data port when all of the contents for this message
// should be cached - probably due to the original message going away
//
SdmErrorCode SdmDpDataChanEditMsg::FetchAllContents(SdmError& err, const SdmMessageNumber msgnum)
{
  assert(EDITMSG_MSGNUM_VALID(msgnum));

  return(FetchAllContents(err, _emcMessageStructure));
}

// This method is used to convert a passed in message component
// that may be either a EditMsg message component or an original
// message component from a derived message, and find the real
// EditMsg component as used internally by this channel adapter.
// This is necessary because any call to GetMessageStructure returns
// copies of the internal structure, and the caller may be holding
// on to a copy of the structure from a derived message before
// it was redirected here.

SdmErrorCode SdmDpDataChanEditMsg::LocalizeMessage
  (
  SdmError& err, 
  SdmDpEditMsgMessageComponent*& r_emcmp,
  SdmDpMessageStructure*& r_emstr,
  const SdmDpMessageComponent& mcmp,
  const SdmDpMessageStructureL& msgstr,
  const SdmMessageNumber msgnum
  )
{
  // Reset both return values

  r_emcmp = 0;
  r_emstr = 0;

  // Scan message structure for this message looking for the target component

  int numstrs = msgstr.ElementCount();
  for (int i = 0; i < numstrs; i++) {
    SdmDpMessageStructure* mstr = msgstr[i];
    assert(mstr);
    assert(mstr->ms_component);
    SdmDpEditMsgMessageComponent* emcmp = (SdmDpEditMsgMessageComponent*)mstr->ms_component;

    // If the passed in component is an EditMsg component in the message structure,
    // or if the passed in component is an original message component from a derived message,
    // then return this EditMsg component

    if ((*mstr->ms_component == mcmp) 
	|| (emcmp->_emccOrigComponent && (mcmp == *emcmp->_emccOrigComponent))) {
      r_emstr = mstr;
      r_emcmp = (SdmDpEditMsgMessageComponent*)mstr->ms_component;
      break;
    }

    // If this is a nested component, recursively search that component

    if (mstr->ms_ms)
      if (LocalizeMessage(err, r_emcmp, r_emstr, mcmp, *mstr->ms_ms, msgnum) == Sdm_EC_Success)
	break;
  }

  // If no component was resolved, then return an error

  if (!r_emcmp || !r_emstr)
    return (err = Sdm_EC_Fail);

  // Found the right EditMsg component: verify its integrity and return success

  assert(r_emcmp->_emccIdentifier);
  return (err = Sdm_EC_Success);

}

// --------------------------------------------------------------------------------
// SdmDpEditMsgMessageComponent derivative of SdmDpMessageComponent Implementation
// --------------------------------------------------------------------------------

// Constructor

SdmDpEditMsgMessageComponent::SdmDpEditMsgMessageComponent(long componentIdentifier) :
    _emccId(EditMsgComponentId), _emccOrigMessageNumber(0), _emccOrigChan(0), _emccOrigPort(0), 
    _emccOrigComponent(0), _emccContentsFetched(Sdm_False), _emccIdentifier(componentIdentifier),
    _emccIsDeleted(Sdm_False)
{
}

// Destructor

SdmDpEditMsgMessageComponent::~SdmDpEditMsgMessageComponent()
{
  // delete our copy of the original component.
  if (_emccOrigComponent)
    delete _emccOrigComponent;
}

// Print method
// Used in testing only

void SdmDpEditMsgMessageComponent::Print() const 
{
  printf("em%d%s", _emccIdentifier, _emccOrigComponent ? ":" : ""); 
  if (_emccOrigComponent)
    _emccOrigComponent->Print();
}

// Deep Copy
// Required to make a legitimate usable copy of a message component

SdmDpMessageComponent* SdmDpEditMsgMessageComponent::DeepCopy()
{
  SdmDpEditMsgMessageComponent *mcmp = new SdmDpEditMsgMessageComponent(_emccIdentifier);
  int i = 0;
  mcmp->_emccAttributes.ClearAllElements();
  int numAttributes = _emccAttributes.ElementCount();
  mcmp->_emccAttributes.SetVectorSize(numAttributes);
  for (i = 0; i < numAttributes; i++)
    mcmp->_emccAttributes[i] = _emccAttributes[i];
  mcmp->_emccIsDeleted = _emccIsDeleted;
  mcmp->_emccContents = _emccContents;
  
  if (_emccOrigComponent) {
    // we need to create a copy of the orig component because the destructor for
    // SdmDpEditMsgMessageComponent deletes the orig component.  we don't want
    // to have multiple SdmDpEditMsgMessageComponent objects referencing the same
    // original component.
    mcmp->_emccOrigComponent = _emccOrigComponent->DeepCopy();
  } else {
    mcmp->_emccOrigComponent = NULL;
  }
  
  mcmp->_emccOrigPort = _emccOrigPort;
  mcmp->_emccOrigChan = _emccOrigChan;
  mcmp->_emccOrigMessageNumber = _emccOrigMessageNumber;
  mcmp->_emccContentsFetched = _emccContentsFetched;
  return(mcmp);
}

// Equality operator: needs to be able to identify an editmsg component
// and compare it with another to see if its identical in value

int SdmDpEditMsgMessageComponent::operator== 
  (
  const SdmDpMessageComponent& mcmp
  ) const
{
  SdmDpEditMsgMessageComponent* emcmp = (SdmDpEditMsgMessageComponent *)&mcmp;
  return(this
	 && emcmp
	 && _emccId == EditMsgComponentId 
	 && emcmp->_emccId == EditMsgComponentId 
	 && emcmp->_emccIdentifier == _emccIdentifier
	 && emcmp->_emccOrigMessageNumber == _emccOrigMessageNumber
	 && emcmp->_emccOrigChan == _emccOrigChan
	 && emcmp->_emccOrigPort == _emccOrigPort);
}
