/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "T client" data channel subclass
// --> It implements a "testing client" that gives a canned set of
// --> responses always - used to perform a consistent test of
// --> the data channel interface.
// --> Also, the T client has many assertions that are used to check
// --> the conditions set up by the data channel that other channel
// --> adapters can take for granted (e.g. "GetMessageHeaders" will
// --> only be called if an open was successful, so a regular channel
// --> adapter will never have to test for the condition).

#pragma ident "@(#)DataChanTclient.cc	1.82 97/03/27 SMI"

// Include Files.
#include <string.h>
#include <SDtMail/Sdtmail.hh>
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChanTclient.hh>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

//
// This is a test client and it is not compiled into production versions.
// Only include if necessary.
//

#if	defined(INCLUDE_TCLIENT)

static const char *CHANNEL_NAME = "datachanTclient";

int SdmDpDataChanTclient::_dcOneTimeFlag = 0;
SdmToken *SdmDpDataChanTclient::_dcRefToken;

// Constructor 

SdmDpDataChanTclient::SdmDpDataChanTclient(SdmDataPort* parentDataPort) :
  SdmDpDataChan(Sdm_CL_DataChannel, parentDataPort), _dcReadonly(Sdm_False)
{
  _dcOpen = Sdm_False;		// no channel is open
  _dcAttached = Sdm_False;	// no channels are attached
}

// Destructor

SdmDpDataChanTclient::~SdmDpDataChanTclient() 
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

SdmErrorCode SdmDpDataChanTclient::StartUp(SdmError& err)
{
  assert(err == Sdm_EC_Success);

  if (_IsStarted)
    return(err = Sdm_EC_Success);

  if (!_dcOneTimeFlag) {
    // setup the reference token used to filter out incompatible requests
    //
    _dcRefToken = new SdmToken;		// create the reference token
    _dcRefToken->SetValue("servicechannel","tclient");
    _dcRefToken->SetValue("serviceclass","messagestore");
    _dcRefToken->SetValue("servicetype","test");
    _dcOneTimeFlag++;
  }

  // construct the pseudo mailbox for this instantiation of the t client
  _dcMessages.ClearAndDestroyAllElements();
  _dcMessages.AddElementToList(makeMessage(0));
  _dcMessages.AddElementToList(makeMessage(1));
  _dcMessages.AddElementToList(makeMessage(2));
  _dcMessages.AddElementToList(makeMessage(3));
  //  _dcMessages.AddElementToList(makeMessage(4));

  _IsStarted = Sdm_True;			// Elle est chaude! VF.

 return(err = Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChanTclient::ShutDown(SdmError& err)
{
  assert(err == Sdm_EC_Success);

  if (!_IsStarted)
    return(err = Sdm_EC_Success);

  // Destruct the pseudo mailbox for this instantiation of the t client
  _dcMessages.ClearAndDestroyAllElements();
  _dcSubscribedList.ClearAllElements();

  _IsStarted = Sdm_False;

 return(err = Sdm_EC_Success);
}

// function SdmDataChanTclient factory : creates a SdmDpDataChanTclient object
//
SdmDpDataChan *SdmDpDataChanTclientFactory(SdmDataPort* parentDataPort)
{
  return(new SdmDpDataChanTclient(parentDataPort));
}

// --------------------------------------------------------------------------------
// DATA CHANNEL API - t client implementation
// This is the API that is exported to the users of the data channel
// --------------------------------------------------------------------------------

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode SdmDpDataChanTclient::AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  )
{
  assert(_IsStarted);
  assert(_dcOpen || _dcAttached);

  _dcSubscribedList(-1).SetNumberAndString(0,name);
  return(err = Sdm_EC_Success);
}

#endif


// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode SdmDpDataChanTclient::Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to attach to
  )
{
  assert(_IsStarted);
  assert(!_dcOpen);
  assert(!_dcAttached);

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

  // Ok this token is acceptable to attempt to be attached by this channel adapter.

  _dcAttached = Sdm_True;
  return (err = Sdm_EC_Success);
}

// Cancel any operations pending on the data port
//
SdmErrorCode SdmDpDataChanTclient::CancelPendingOperations
  (
  SdmError& err
  )
{
  assert(_IsStarted);

  return(err = Sdm_EC_Success);
}

// Close the current connection down
//
SdmErrorCode SdmDpDataChanTclient::Close
  (
  SdmError& err
  )
{
  assert(_IsStarted);
  assert(_dcOpen || _dcAttached);

  _dcOpen = Sdm_False;
  _dcAttached = Sdm_False;

  return(err = Sdm_EC_Success);
}

// Commit any pending changes to a message
//
SdmErrorCode SdmDpDataChanTclient::CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messageRewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  )
{
  return (err = Sdm_EC_Success);	// commit is always successful: nothing to do
}

// Check for new messages in the message store
//
SdmErrorCode SdmDpDataChanTclient::CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  )
{
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  r_nummessages = 0;			// canned response: 0 new messages

  return(err = Sdm_EC_Success);
}

// Discard any pending changes to a message
//
SdmErrorCode SdmDpDataChanTclient::DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  )
{
  return (err = Sdm_EC_Success);	// discard is always successful: nothing to do
}

// Expunge deleted messages from the currently open message store
//
SdmErrorCode SdmDpDataChanTclient::ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// augmented list of message numbers of messages deleted
  )
{
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_msgnums.ElementCount() == 0);	// prove that the list is empty upon call

  if (_dcReadonly)
    return(err = Sdm_EC_Readonly);

  // Spin through the list and gather up all messages with the deleted flag set

  int numMessages = _dcMessages.ElementCount();
  for (int i = 0; i < numMessages; i++) {
    SdmDpTclientMessage *msg = _dcMessages[i];
    if (msg->_tcmFlags&  Sdm_MFA_Deleted)
      r_msgnums(-1) = (i+1);
  }

  // Starting from the top, work backwards and delete the actual messages in the real list

  int numDeleted = r_msgnums.ElementCount();
  if (numDeleted)
    for (i = numDeleted; i > 0; i--) {
      SdmMessageNumber mn = r_msgnums[i-1]-1;
      SdmDpTclientMessage *msg = _dcMessages[mn];
      delete msg;
      _dcMessages.RemoveElement(mn);
    }
  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call
  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  // Transform the generic message component into the tclient component

  const SdmDpTclientMessageComponent *tmcmp = (const SdmDpTclientMessageComponent *)& mcmp;
  assert(tmcmp->_tcmcIdentifier);
  if (msgnum != tmcmp->_tcmcMsgnum)
    return(Sdm_EC_Fail);

  // Copy all attributes to returned list

  int numEntries = tmcmp->_tcmcAttributes.ElementCount();
  for (int i = 0; i < numEntries; i++)
    r_attribute.AddElementToList(tmcmp->_tcmcAttributes[i]);

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// augmented attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);
  assert(r_attribute.Length() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  // Transform the generic message component into the tclient component

  const SdmDpTclientMessageComponent *tmcmp = (const SdmDpTclientMessageComponent *)& mcmp;
  assert(tmcmp->_tcmcIdentifier);
  if (msgnum != tmcmp->_tcmcMsgnum)
    return(Sdm_EC_Fail);

  // Copy all matching attributes to returned list

  int numEntries = tmcmp->_tcmcAttributes.ElementCount();
  for (int i = 0; i < numEntries; i++)
    if (::strcasecmp(attribute, tmcmp->_tcmcAttributes[i].GetFirstString())==0) {
      r_attribute = tmcmp->_tcmcAttributes[i].GetSecondString();
      return (Sdm_EC_Success);
    }

  // Did not find the attribute - return fail error

  return(err = Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  // Transform the generic message component into the tclient component

  const SdmDpTclientMessageComponent *tmcmp = (const SdmDpTclientMessageComponent *)& mcmp;
  assert(tmcmp->_tcmcIdentifier);
  if (msgnum != tmcmp->_tcmcMsgnum)
    return(Sdm_EC_Fail);
  int numEntries = tmcmp->_tcmcAttributes.ElementCount();
  int numTargAttributes = attributes.ElementCount();
  for (int i = 0; i < numEntries; i++) {
    for (int k = 0; k < numTargAttributes; k++)
      if (::strcasecmp((const char *)(attributes[k]), tmcmp->_tcmcAttributes[i].GetFirstString())==0)
	r_attribute.AddElementToList(tmcmp->_tcmcAttributes[i]);
    }

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// augmented list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_attribute.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  // Transform the generic message component into the tclient component

  const SdmDpTclientMessageComponent *tmcmp = (const SdmDpTclientMessageComponent *)& mcmp;
  assert(tmcmp->_tcmcIdentifier);
  if (msgnum != tmcmp->_tcmcMsgnum)
    return(Sdm_EC_Fail);
  int numEntries = tmcmp->_tcmcAttributes.ElementCount();

  for (SdmAbstractAttributeMap *aamp = _sdmAbstractAttributes; aamp->flag; aamp++) {
    if (attributes&  aamp->flag) {
      assert(aamp->realName);
      for (int i = 0; i < numEntries; i++) {
	if (::strcasecmp(aamp->realName, tmcmp->_tcmcAttributes[i].GetFirstString())==0)
	  r_attribute.AddElementToList(tmcmp->_tcmcAttributes[i]);
      }
    }
  }

  // Return success only if some data was found

  return(err = r_attribute.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif


// Return boolean indicating whether message is currently cached.
//
SdmErrorCode SdmDpDataChanTclient::GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);
  
  r_cached = Sdm_True;
  err = Sdm_EC_Success;
  return err;
}


// Return complete contents for the entire message, as opposed to a component
// of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);
  assert(r_contents.Length() == 0);	// prove that return contents container is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmString str_contents;
  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);

  switch (contenttype) {
  case DPCTTYPE_rawfull:		// the entire message in raw format
  case DPCTTYPE_rawheader:		// the entire message header in raw format
  case DPCTTYPE_rawbody:		// the entire message body in raw format
    // Can drop down to generic "gather contents" function which knows how to
    // iterate through the pieces if needed
    //
    gatherContents(str_contents, contenttype, msg->_tcmStructure);
    break;
  case DPCTTYPE_processedcomponent:	// the "final contents" of the component in question
    return(err = Sdm_EC_Fail);
  default:
    return(err = Sdm_EC_Fail);
  }
  r_contents = str_contents;
  return(err = Sdm_EC_Success);
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  int i = 0, j = 0, numHeaders = 0;
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);
  assert(r_contents.Length() == 0);	// prove that return contents container is empty upon call
  

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmString str_contents;
  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);

  const SdmDpTclientMessageComponent *tmcmp = (const SdmDpTclientMessageComponent *)& mcmp;
  assert(tmcmp->_tcmcIdentifier);
  if (msgnum != tmcmp->_tcmcMsgnum)
    return(Sdm_EC_Fail);

  switch (contenttype) {
  case DPCTTYPE_rawfull:
    // This request is to get the entire message in raw format; we need to zip on through
    // all of the components and build up the output
    // Attach all primary headers
    numHeaders = tmcmp->_tcmcHeaders.ElementCount();
    for (j = 0; j < numHeaders; j++) {
      str_contents += (tmcmp->_tcmcHeaders[j]).GetFirstString();
      str_contents += ": ";
      str_contents += (tmcmp->_tcmcHeaders[j]).GetSecondString();
      str_contents += "\n";
    }
    str_contents += "\n";
    // Now include the "raw" body part for the entire message; cycle through the parts
    str_contents += tmcmp->_tcmcContents;
    break;
  case DPCTTYPE_rawheader:		// the entire message header in raw format
    // This request is to get the entire message header in raw format; we need to zip on through
    // the headers and build up the output
    // Attach all primary headers
    numHeaders = tmcmp->_tcmcHeaders.ElementCount();
    for (j = 0; j < numHeaders; j++) {
      str_contents += (tmcmp->_tcmcHeaders[j]).GetFirstString();
      str_contents += ": ";
      str_contents += (tmcmp->_tcmcHeaders[j]).GetSecondString();
      str_contents += "\n";
    }
    break;
  case DPCTTYPE_rawbody:		// the entire message body in raw format
    // This request is to get the entire message body in raw format; we need to zip on through
    // the body component and build up the output
    // Now include the "raw" body part for the entire message; cycle through the parts
    str_contents += tmcmp->_tcmcContents;
    break;
  case DPCTTYPE_processedcomponent:	// the "final contents" of the component in question
    // This request is to get the "processed" contents
    str_contents += tmcmp->_tcmcContents;
    break;
  default:
    assert(0);
  }
  r_contents = str_contents;
  return(err = Sdm_EC_Success);

}

// Return flags for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  r_aflags = _dcMessages[msgnum-1]->_tcmFlags;
  return(err = Sdm_EC_Success);
}

// Return a list of flags for a range of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_aflags.ElementCount() == 0);	// prove that the list is empty upon call

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    r_aflags.AddElementToList(_dcMessages[j+(startmsgnum-1)]->_tcmFlags);
  }

  return(err = Sdm_EC_Success);
}

// Return a list of flags for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int numMessages = msgnums.ElementCount();
  assert(r_aflags.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmMessageFlagAbstractFlags aflag;
    if (GetMessageFlags(err, aflag, msgnums[j]) != Sdm_EC_Success) {
      r_aflags.ClearAllElements();
      return(err);
    }
    r_aflags.AddElementToList(aflag);
  }
  return(err = Sdm_EC_Success);
}

// Return a list of all headers for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  SdmDpMessageStructure *msgstr = 0;
  SdmDpTclientMessageComponent *mcmp = 0;

  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_hdr.ElementCount() == 0);		// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);
  msgstr = msg->_tcmStructure[0];
  assert(msgstr);
  mcmp = (SdmDpTclientMessageComponent *)msgstr->ms_component;
  assert(mcmp);
  int numHeaders = mcmp->_tcmcHeaders.ElementCount();

  for (int i = 0; i < numHeaders; i++)
    r_hdr.AddElementToList(mcmp->_tcmcHeaders[i]);

  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;

  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, startmsgnum+j) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success: Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, msgnums[j]) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }
  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif


// Return a list of headers for a single header for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  SdmDpMessageStructure *msgstr = 0;
  SdmDpTclientMessageComponent *mcmp = 0;

  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);
  msgstr = msg->_tcmStructure[0];
  assert(msgstr);
  mcmp = (SdmDpTclientMessageComponent *)msgstr->ms_component;
  assert(mcmp);
  int numHeaders = mcmp->_tcmcHeaders.ElementCount();

  for (int i = 0; i < numHeaders; i++)
    if (::strcasecmp((const char *)mcmp->_tcmcHeaders[i].GetFirstString(), (const char *)hdr)==0)
    r_hdr.AddElementToList(mcmp->_tcmcHeaders[i]);

  return(err = Sdm_EC_Success);
}

// Return a list of specific headers for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  SdmDpMessageStructure *msgstr = 0;
  SdmDpTclientMessageComponent *mcmp = 0;

  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_hdrs.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);
  msgstr = msg->_tcmStructure[0];
  assert(msgstr);
  mcmp = (SdmDpTclientMessageComponent *)msgstr->ms_component;
  assert(mcmp);
  int numHeaders = mcmp->_tcmcHeaders.ElementCount();
  int numTargHeaders = hdrs.ElementCount();

  for (int i = 0; i < numHeaders; i++) {
    for (int k = 0; k < numTargHeaders; k++)
      if (::strcasecmp((const char *)mcmp->_tcmcHeaders[i].GetFirstString(), (const char *)(hdrs[k]))==0) {
	r_hdrs.AddElementToList(mcmp->_tcmcHeaders[i]);
	break;
      }
  }

  return(err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeader
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
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;

  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeader(err, *newList, hdr, startmsgnum+j) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeader(err, *newList, hdr, msgnums[j]) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}



// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
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
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  assert(r_hdrs.ElementCount() == 0);	// prove that the list is empty upon call

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, hdrs, startmsgnum+j) != Sdm_EC_Success) {
      r_hdrs.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdrs.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();
  assert(r_hdrs.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, hdrs, msgnums[j]) != Sdm_EC_Success) {
      r_hdrs.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdrs.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif


// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  SdmDpMessageStructure *msgstr = 0;
  SdmDpTclientMessageComponent *mcmp = 0;

  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);
  msgstr = msg->_tcmStructure[0];
  assert(msgstr);
  mcmp = (SdmDpTclientMessageComponent *)msgstr->ms_component;
  assert(mcmp);
  int numHeaders = mcmp->_tcmcHeaders.ElementCount();

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr & ahmp->flag) {
      if (ahmp->realName) {
	for (int i = 0; i < numHeaders; i++) {
	  if (::strcasecmp((const char *)mcmp->_tcmcHeaders[i].GetFirstString(),
                           (const char *)ahmp->realName)==0)
	    r_hdr(-1) = mcmp->_tcmcHeaders[i];
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
            if (::strcasecmp((const char *)mcmp->_tcmcHeaders[j].GetFirstString(),
                             hdrnm)==0)
	      r_hdr(-1) = mcmp->_tcmcHeaders[j];
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
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
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
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, hdr, startmsgnum+j) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmStrStrL *newList = new SdmStrStrL;
    if (GetMessageHeaders(err, *newList, hdr, msgnums[j]) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif


// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  SdmDpMessageStructure *msgstr = 0;
  SdmDpTclientMessageComponent *mcmp = 0;

  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpTclientMessage *msg = _dcMessages[msgnum-1];
  assert(msg);
  msgstr = msg->_tcmStructure[0];
  assert(msgstr);
  mcmp = (SdmDpTclientMessageComponent *)msgstr->ms_component;
  assert(mcmp);
  int numHeaders = mcmp->_tcmcHeaders.ElementCount();

  for (SdmAbstractHeaderMap *ahmp = _sdmAbstractHeaders; ahmp->flag; ahmp++) {
    if (hdr&  ahmp->flag) {
      if (ahmp->realName) {
	for (int i = 0; i < numHeaders; i++) {
	  if (::strcasecmp((const char *)mcmp->_tcmcHeaders[i].GetFirstString(), (const char *)ahmp->realName)==0)
	    r_hdr(-1).SetNumberAndString(ahmp->flag, (mcmp->_tcmcHeaders[i]).GetSecondString());
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
            if (::strcasecmp((const char *)mcmp->_tcmcHeaders[j].GetFirstString(),
                             hdrnm)==0)
	      r_hdr(-1).SetNumberAndString(abstHdrList[i],
                                           (mcmp->_tcmcHeaders[j]).GetSecondString());
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
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
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
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmIntStrL *newList = new SdmIntStrL;
    if (GetMessageHeaders(err, *newList, hdr, startmsgnum+j) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  int foundOne = 0;
  int numMessages = msgnums.ElementCount();
  assert(r_hdr.ElementCount() == 0);	// prove that the list is empty upon call

  for (int j = 0; j < numMessages; j++) {
    SdmIntStrL *newList = new SdmIntStrL;
    if (GetMessageHeaders(err, *newList, hdr, msgnums[j]) != Sdm_EC_Success) {
      r_hdr.ClearAndDestroyAllElements();
      delete newList;
      return(err);
    }
    r_hdr.AddElementToList(newList);
    if (newList->ElementCount())
      foundOne++;
  }

  return(err = foundOne ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

// Return the message structure for a single message
//
SdmErrorCode SdmDpDataChanTclient::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_structure.ElementCount() == 0);	// prove that the structure list is empty upon call

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  SdmDpMessageStructureL *msgstrs =& _dcMessages[msgnum-1]->_tcmStructure;
  if (!msgstrs)
    return(Sdm_EC_Fail);
  int numstrs = msgstrs->ElementCount();
  if (!numstrs)
    return(Sdm_EC_Fail);

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
SdmErrorCode SdmDpDataChanTclient::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_structure.ElementCount() == 0);	// prove that the structure list is empty upon call

  int numMessages = (endmsgnum-startmsgnum)+1;
  for (int j = 0; j < numMessages; j++) {
    SdmDpMessageStructureL *newStr = new SdmDpMessageStructureL;
    if (GetMessageStructure(err, *newStr, startmsgnum+j) != Sdm_EC_Success) {
      r_structure.ClearAndDestroyAllElements();
      delete newStr;
      return(err);
    }
    r_structure.AddElementToList(newStr);
  }

  return(err = Sdm_EC_Success);
}

// Return a list of message structures for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  assert(r_structure.ElementCount() == 0);	// prove that the structure list is empty upon call
  int numMessages = msgnums.ElementCount();

  for (int j = 0; j < numMessages; j++) {
    SdmDpMessageStructureL *newStr = new SdmDpMessageStructureL;
    if (GetMessageStructure(err, *newStr, msgnums[j]) != Sdm_EC_Success) {
      r_structure.ClearAndDestroyAllElements();
      delete newStr;
      return(err);
    }
    r_structure.AddElementToList(newStr);
  }
  return(err = Sdm_EC_Success);
}

#endif

// Return a message store status structure for the message store
//
SdmErrorCode SdmDpDataChanTclient::GetMessageStoreStatus
  (
  SdmError& err,
  SdmMessageStoreStatus& r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  // First, reset the status structure

  r_status.flags = storeFlags;
  r_status.messages = 0;
  r_status.recent = 0;
  r_status.unseen = 0;
  r_status.uidnext = 0;
  r_status.uidvalidity = 0;
  r_status.checksum = 0;
  r_status.checksum_bytes = 0;

  // Now spin through tallying up the statistics

  int numMessages = _dcMessages.ElementCount();
  for (int i = 0; i < numMessages; i++) {
    SdmDpTclientMessage *msg = _dcMessages[i];

    r_status.messages += 1;
    if (!(msg->_tcmFlags&  Sdm_MFA_Deleted)) {
      if ((storeFlags && Sdm_MSS_Recent) && (msg->_tcmFlags&  Sdm_MFA_Recent))
	r_status.recent++;
      if ((storeFlags && Sdm_MSS_Unseen) && (!(msg->_tcmFlags&  Sdm_MFA_Seen)))
	r_status.unseen++;
    }
    if ((storeFlags && Sdm_MSS_Uidnext) && (msg->_tcmUid > r_status.uidnext))
      r_status.uidnext = msg->_tcmUid;
  }

  if (storeFlags && Sdm_MSS_Uidnext)
    r_status.uidnext++;
  return(err = Sdm_EC_Success);
}

// open a connection to an object given a token stream
//
SdmErrorCode SdmDpDataChanTclient::Open
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

  // Ok this token is acceptable to attempt to be opened by this channel adapter.
  // We can only open "messagestorename text.mbx".
  // Lets see what it specifies.
  SdmString storeName;

  if (!tk.FetchValue(storeName, "serviceoption", "messagestorename"))
    return (err = Sdm_EC_MessageStoreNameRequired);

  if (::strcmp((const char *)storeName, "test.mbx") != 0)
    return (err = Sdm_EC_CannotAccessMessageStore);

  // set "read only" if readonly is set and readwrite is not set
  //
  _dcReadonly = (tk.CheckValue("serviceoption", "readonly")
		  && (!tk.CheckValue("serviceoption", "readwrite"))) ? Sdm_True : Sdm_False;

  r_nmsgs = _dcMessages.ElementCount();
  r_readOnly = _dcReadonly;

  _dcOpen = Sdm_True;
  return (err = Sdm_EC_Success);
}

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
SdmErrorCode SdmDpDataChanTclient::RemoveFromSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name			// name to be removed from the subscribed namespace
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen || _dcAttached);

  int numEntries = _dcSubscribedList.ElementCount();
  for (int i = 0; i < numEntries; i++) {
    const char *targFileName = _dcSubscribedList[i].GetString();
    if (::strcmp(targFileName, (const char *)name)==0) {
      _dcSubscribedList.RemoveElement(i);
      return(err = Sdm_EC_Success);
    }
  }

  return(err = Sdm_EC_RequestedDataNotFound);
}

#endif

// Cause the complete state of the message store to be saved
//
SdmErrorCode SdmDpDataChanTclient::SaveMessageStoreState
  (
  SdmError& err
  )
{
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  return(err = Sdm_EC_Success);
}

// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
SdmErrorCode SdmDpDataChanTclient::ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  )
{
  assert(err == Sdm_EC_Success);
  char argstr[2048];
  char *bufptr = 0;
  char *argS[10];
  int argI = 0;

  assert(_IsStarted);
  assert(_dcOpen || _dcAttached);

  assert(r_names.ElementCount() == 0);	// prove that the list is empty upon call

  // Ok, for the t-client, use captureCmd to exec ls a properly combined
  // reference/pattern pair

  const char *sp = "/usr/bin/ls -1d ";
  char *tp = argstr;
  char c;
  while (c = *sp++)
    *tp++ = c;
  sp = (const char *)reference;
  while (c = *sp++)
    *tp++ = c;
  *tp++ = '/';
  sp = (const char *)pattern;
  if (!*sp)
    *tp++ = '.';
  else while (c = *sp++)
    *tp++ = (c == '%' ? '*' : c);
  *tp = '\0';

  int result = captureCmd(&bufptr, "/bin/csh", "csh", "-c", argstr, 0L);
  if (result) {
    if (bufptr)
      free(bufptr);
    return(err = Sdm_EC_Fail);
  }

  // Translate output buffer into list of intstr pairs

  tp = bufptr;
  if (tp) while (*tp) {
    char *xp = argstr;
    while (*tp && *tp != '\n') {
      *xp++ = *tp++;
    }
    *xp = '\0';

    // Got the name, stat it and set the appropriate flags

    struct stat statbuf;
    SdmNamespaceFlag nsflag = 0;

    int result = stat(argstr,& statbuf);
    if (result == -1)
      nsflag = (Sdm_NSA_noInferiors|Sdm_NSA_cannotOpen);
    else {
      if (!(statbuf.st_mode&  S_IFREG) || access(argstr, R_OK))		// Regularfile ?
	nsflag |= Sdm_NSA_cannotOpen;		// No, cannot be opened
      else {
	if (statbuf.st_atime < statbuf.st_mtime)
	  nsflag |= Sdm_NSA_changed;
	else if (statbuf.st_mtime < statbuf.st_atime)
	  nsflag |= Sdm_NSA_unChanged;
      }
      if (!(statbuf.st_mode&  S_IFDIR))		// Directory?
	nsflag |= Sdm_NSA_noInferiors;		// No, cannot be scanned into
    }
    r_names(-1).SetNumberAndString(nsflag,argstr);
    while (*tp == '\n')
      tp++;
  }
  if (bufptr)
    free(bufptr);

  return(err = r_names.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode SdmDpDataChanTclient::ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  )
{
  assert(err == Sdm_EC_Success);
  char argstr[2048];

  assert(_IsStarted);
  assert(_dcOpen || _dcAttached);

  assert(r_names.ElementCount() == 0);	// prove that the list is empty upon call

  // Ok, here is a real party hack: go through the internally maintained 
  // subscribed list and ask csh to match it against the pattern passed in

  // Create the "file name matching pattern" for the passed in reference/pattern

  char *tp = argstr;
  const char *sp;
  char c;
  sp = (const char *)reference;
  while (c = *sp++)
    *tp++ = c;
  *tp++ = '/';
  sp = (const char *)pattern;
  while (c = *sp++)
    *tp++ = c;
  *tp = '\0';
  printf("pattern is %s\n", argstr);

  int numEntries = _dcSubscribedList.ElementCount();
  for (int i = 0; i < numEntries; i++) {
    const char *targFileName = _dcSubscribedList[i].GetString();
    if (pmatch_full(targFileName, argstr, '/'))
      r_names.AddElementToList(_dcSubscribedList[i]);
  }

  return(err = r_names.ElementCount() ? Sdm_EC_Success : Sdm_EC_RequestedDataNotFound);
}

#endif

// Set a set of flags for a single message
//
SdmErrorCode SdmDpDataChanTclient::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  if (_dcReadonly)
    return(err = Sdm_EC_Readonly);

  if (msgnum < 1 || msgnum > _dcMessages.ElementCount())
    return(err = Sdm_EC_Fail);

  if (aflags) {
    if (flagValue)
      _dcMessages[msgnum-1]->_tcmFlags |= aflags;
    else
      _dcMessages[msgnum-1]->_tcmFlags &= (~aflags);
  }
  return(err = Sdm_EC_Success);
}

// Set a set of flags for a range of messages
//
SdmErrorCode SdmDpDataChanTclient::SetMessageFlagValues
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
  assert(_dcOpen);
  assert(!_dcAttached);

  if (_dcReadonly)
    return(err = Sdm_EC_Readonly);

  if ( (startmsgnum > endmsgnum) || (startmsgnum < 1) || (endmsgnum > _dcMessages.ElementCount()))
    return(err = Sdm_EC_Fail);

  int numMessages = (endmsgnum-startmsgnum)+1;
  if (aflags) {
    for (int j = 0; j < numMessages; j++) {
      if (flagValue)
	_dcMessages[j+(startmsgnum-1)]->_tcmFlags |= aflags;
      else
	_dcMessages[j+(startmsgnum-1)]->_tcmFlags &= (~aflags);
    }
  }

  return(err = Sdm_EC_Success);
}

// Set a set of flags for a list of messages
//
SdmErrorCode SdmDpDataChanTclient::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  )
{
  assert(err == Sdm_EC_Success);
  assert(_IsStarted);
  assert(_dcOpen);
  assert(!_dcAttached);

  if (_dcReadonly)
    return(err = Sdm_EC_Readonly);

  int numMessages = msgnums.ElementCount();

  if (aflags) {
    for (int j = 0; j < numMessages; j++) {
      if (SetMessageFlagValues(err, flagValue, aflags, msgnums[j]) != Sdm_EC_Success)
	return(err);
    }
  }
  return(err = Sdm_EC_Success);
}

// --------------------------------------------------------------------------------
// Utility functions that are not part of the Data Channel API and are private to
// this interface are included below.
// --------------------------------------------------------------------------------

// captureCmd - execute a system command and capture the result
// into a buffer that is malloc()ed for the caller.
//
int		SdmDpDataChanTclient::captureCmd(char **bufPtr, char * cmd, ...)
{
  va_list	ap;
  int             lpipe[2];
  int             pid;
  int             cfd;
  int             i;
  char *          buffer;
  int             bufferSize;
  int             bufferIndex;
  int             result;
  int             status;
  
  assert(bufPtr);
  
  *bufPtr = 0;
  
  /*
   * Create a pipe to be used to capture the command output
   */
  
  result = pipe(lpipe);
  assert(result == 0);
  assert(lpipe[0]);
  assert(lpipe[1]);
  
  /*
   * Fork off a process to execute the command
   */
  
  pid = fork1();							/* Fork off a new process */
  if (pid == 0)							/* Are we the child process?? */
    {
      
      /*
       * This is the forked (child) process ======================
       */
      
      (void) close(lpipe[0]);					/* close the reader side of the pipe */
      (void) dup2(lpipe[1], 1);					/* pipe writer is now standard output */
      (void) dup2(lpipe[1], 2);					/* pipe writer is now standard error */
      for (cfd = 3; cfd < 256; cfd++)				/* Close all files except standard i/o */
	(void) close(cfd);
      
      /*
       * Decide how to employ execvp: if cmd is == 0, we have been called as:
       *  captureCmd(char * *bufptr, 0L, char * *cmd, char *argS[]);
       * otherwise we have been called as:
       *  captureCmd(char * *bufptr, char * *cmd, <args to exec>
       */
      
      va_start(ap, cmd);					/* Begin variable argument processing */
      if (cmd == (char *) 0)
	{
	  char **argS;
	  cmd = va_arg(ap, char *);
	  assert(cmd);
	  argS = va_arg(ap, char **);
	  assert(cmd);
	  (void) execvp(cmd, argS);
	}
      else
	(void) execvp(cmd, (char **) ap);				/* Invoke the intended victim of this process */
      status = errno;						/* EGADS! The execvp failed?! */
      perror(cmd);						/* Emit some kind of error (which ends up in callers output buffer) */
      _exit(status ? status : -1);					/* Child bails ... */
    }
  
  /*
   * This is the forking (parent) process ====================
   */
  
  bufferSize = 128;
  bufferIndex = 0;
  buffer = (char *)malloc(bufferSize);
  assert(buffer);
  memset(buffer, 0, bufferSize);
  (void) close(lpipe[1]);					/* Close the write side of the file descriptor */
  
  /*
   * Spin reading data from the child into the buffer - when the read eofs, the child has exited
   */
  while ((i = read(lpipe[0], buffer + bufferIndex, bufferSize - bufferIndex)) > 0)
    {
      bufferIndex += i;
      if (bufferIndex < bufferSize)
	continue;
      buffer = (char *)realloc(buffer, bufferSize += 128);
      assert(buffer);
      memset(buffer + bufferIndex, 0, bufferSize - bufferIndex);
    }
  
  (void) close(lpipe[0]);					/* Close the read side of the file descriptor */
  
  result = waitpid(pid,& status, 0L);				/* Get exit status from the sub-process */
  assert(result == pid);
  
  if (!*buffer)							/* Is the output buffer void of contents?? */
    free(buffer);						/* YES: the output buffer contents are meaningless - release storage */
  else
    *bufPtr = buffer;						/* NO: the output buffer contents are passed to the caller */
  
  return (WIFEXITED(status) ? WEXITSTATUS(status) : -1);	/* return result from child if exit() else return -1 */
}

// makeMessage: create a "prepackaged" email message that can be included
// in the pseudo mailbox the tclient creates when first started

SdmDpTclientMessage *SdmDpDataChanTclient::makeMessage(SdmMessageNumber msgnum)
{
  SdmDpTclientMessage *msg = new SdmDpTclientMessage();
  SdmDpMessageStructure *msgstr;
  SdmDpMessageStructure *msgstrm;
  SdmDpMessageStructureL *msgstrL;
  SdmDpTclientMessageComponent *msgcmp;

  switch(msgnum)
    {
    case 0:	// message 0: simple text/plain
      msg->_tcmFlags = (Sdm_MFA_Recent);
      msg->_tcmUid = 1;
      // Make single message component for this message and attach to single message structure
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 1;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Return-Path", "<daemon@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Eng.Sun.COM (engmail1) by basilisk.Eng.Sun.COM (5.x/SMI-SVR4)	id AA23600; Fri, 17 Feb 1995 16:34:22 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Sun.COM (sun-barr.EBay.Sun.COM) by Eng.Sun.COM (5.x/SMI-5.3)	id AA14531; Fri, 17 Feb 1995 16:34:18 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from mocha.bunyip.com by Sun.COM (sun-barr.Sun.COM)	id AA07389; Fri, 17 Feb 95 16:34:16 PST");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "by mocha.bunyip.com (5.65a/IDA-1.4.2b/CC-Guru-2b)        id AA24308  (mail destined for Gary.Gere@eng.sun.com) on Fri, 17 Feb 95 19:32:47 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Date", "Fri, 17 Feb 95 19:32:47 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Message-Id", "<9502180032.AA24308@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("From", "paf@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("To", "MIME-tester@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Cc", "mail-testers@test.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Bcc", "tattle-tale@bcc.gov");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Mime-Version", "1.0");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Subject", "M1 - MIME-Version: 1.0 header field test");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Type", "text");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Length", "63");
      msgcmp->_tcmcContents =
	"  M1\n"
	"    Always generate a \"MIME-Version: 1.0\" header field.\n"
	"\n"
	"\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_id = "<9502180032.AA24308@mocha.bunyip.com>";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 4;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_not_specified;
      msgstr->ms_unix_mode = 0;
      msg->_tcmStructure.AddElementToList(msgstr);
      break;

    case 1:
      msg->_tcmFlags = (Sdm_MFA_Recent);
      msg->_tcmUid = 2;
      // Make single message component for this message and attach to single message structure
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 2;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Return-Path", "<daemon@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Eng.Sun.COM (engmail1) by basilisk.Eng.Sun.COM (5.x/SMI-SVR4)	id AA23697; Fri, 17 Feb 1995 16:35:45 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Sun.COM (sun-barr.EBay.Sun.COM) by Eng.Sun.COM (5.x/SMI-5.3)	id AA14664; Fri, 17 Feb 1995 16:35:42 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from mocha.bunyip.com by Sun.COM (sun-barr.Sun.COM)	id AA07716; Fri, 17 Feb 95 16:35:42 PST");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "by mocha.bunyip.com (5.65a/IDA-1.4.2b/CC-Guru-2b)        id AA24451  (mail destined for Gary.Gere@eng.sun.com) on Fri, 17 Feb 95 19:34:13 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Date", "Fri, 17 Feb 95 19:34:13 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Message-Id", "<9502180034.AA24451@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("From", "paf@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("To", "MIME-tester@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Mime-Version", "1.0");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Subject", "M3-2 - Content-Type: text/plain");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Type", "text/plain");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Length", "386");
      msgcmp->_tcmcContents = 
	"This email is marked as being text, so a normal user\n"
	"should get this information without a question.\n"
	"\n"
	"  M3   \n"
	"       Recognize and interpret the Content-Type header field, and avoid\n"
	"       showing users raw data with a Content-Type field other than text.\n"
	"       Be able to send at least text/plain messages, with the character\n"
	"       set specified as a parameter if it is not US-ASCII.\n"
	"\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "plain";
      msgstr->ms_id = "<9502180034.AA24451@mocha.bunyip.com>";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 9;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_not_specified;
      msgstr->ms_unix_mode = 0;
      msg->_tcmStructure.AddElementToList(msgstr);
      break;

    case 2:
      msg->_tcmFlags = (Sdm_MFA_Recent|Sdm_MFA_Flagged);
      msg->_tcmUid = 3;
      // Make single message component for this message and attach to single message structure
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 3;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Return-Path", "<daemon@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Eng.Sun.COM (engmail1) by basilisk.Eng.Sun.COM (5.x/SMI-SVR4)	id AA23707; Fri, 17 Feb 1995 16:35:56 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Sun.COM (sun-barr.EBay.Sun.COM) by Eng.Sun.COM (5.x/SMI-5.3)	id AA14677; Fri, 17 Feb 1995 16:35:53 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from mocha.bunyip.com by Sun.COM (sun-barr.Sun.COM)	id AA07749; Fri, 17 Feb 95 16:35:53 PST");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "by mocha.bunyip.com (5.65a/IDA-1.4.2b/CC-Guru-2b)        id AA24496  (mail destined for Gary.Gere@eng.sun.com) on Fri, 17 Feb 95 19:34:24 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Date", "Fri, 17 Feb 95 19:34:24 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Message-Id", "<9502180034.AA24496@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("From", "paf@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("To", "MIME-tester@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Mime-Version", "1.0");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Subject", "M4.1.1 - Content-Type: text/plain; charset=\"US-ASCII\"");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Type", "text/plain; charset=\"US-ASCII\"");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Length", "84");
      msgcmp->_tcmcContents = 
	"  M4.1.1\n"
	"    Recognize and display \"text\" mail with the character set \"US-ASCII.\" \n"
	"\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgcmp->_tcmcAttributes(-1).SetBothStrings("charset", "US-ASCII");
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "plain";
      msgstr->ms_id = "<9502180034.AA24496@mocha.bunyip.com>";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 3;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_not_specified;
      msgstr->ms_unix_mode = 0;
      msg->_tcmStructure.AddElementToList(msgstr);
      break;

    case 3:
      msg->_tcmFlags = (Sdm_MFA_Recent);
      msg->_tcmUid = 4;
      // Make multiple message component for this message and attach to single message structure
      msgstrL = new SdmDpMessageStructureL();
      // Create part 1
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 5;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-type", "text/plain");
      msgcmp->_tcmcPrefix = "\n--this_is_a_boundary\n";
      msgcmp->_tcmcContents = 
	"  (This is the first part out of three)\n"
	"\n"
	"  M4.3.1\n"
	"       Recognize the primary (mixed) subtype. Display all relevant\n"
	"       information on the message level and the body part header level  \n"
	"       and then display or offer to display each of the body parts\n"
	"       individually.\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "plain";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 7;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_attachment;
      msgstr->ms_unix_mode = 0544;
      msgstr->ms_attachment_name = "part-one";
      msgstrL->AddElementToList(msgstr);
      // create part 2
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 6;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-type", "text/plain");
      msgcmp->_tcmcPrefix = "\n--this_is_a_boundary\n";
      msgcmp->_tcmcContents = 
	"  (This is the second part out of three)\n"
	"\n"
	"       (Interopability comment: For maximum usage friendliness, we\n"
	"       recommend the software to open as few windows as possible, i.e. if\n"
	"       the software itself can display several different media-types,\n"
	"       parts made up of these types should be shown together without any\n"
	"       need for interaction with the user,)\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "plain";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 7;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_inline;
      msgstr->ms_unix_mode = 0544;
      msgstr->ms_attachment_name = "part-two";
      msgstrL->AddElementToList(msgstr);
      // create part 3
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 7;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-type", "text/plain");
      msgcmp->_tcmcPrefix = "\n--this_is_a_boundary\n";
      msgcmp->_tcmcContents = 
	"  (This is the third part out of three)\n"
	"\n"
	"       Note that even though all three parts is text/plain, it is\n"
	"       still three different parts, and this should be visible for\n"
	"       the user.\n";
      msgcmp->_tcmcPostfix = "\n--this_is_a_boundary--\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_text;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "plain";
      msgstr->ms_ms = (SdmDpMessageStructureL *)0;
      msgstr->ms_lines = 5;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_attachment;
      msgstr->ms_unix_mode = 0544;
      msgstr->ms_attachment_name = "part-three";
      msgstrL->AddElementToList(msgstr);
      // Create main entry
      msgcmp = new SdmDpTclientMessageComponent();
      msgcmp->_tcmcMsgnum = (msgnum+1);
      msgcmp->_tcmcIdentifier = 8;
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Return-Path", "<daemon@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Eng.Sun.COM (engmail1) by basilisk.Eng.Sun.COM (5.x/SMI-SVR4)	id AA23835; Fri, 17 Feb 1995 16:36:57 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from Sun.COM (sun-barr.EBay.Sun.COM) by Eng.Sun.COM (5.x/SMI-5.3)	id AA14751; Fri, 17 Feb 1995 16:36:54 -0800");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "from mocha.bunyip.com by Sun.COM (sun-barr.Sun.COM)	id AA07909; Fri, 17 Feb 95 16:36:53 PST");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Received", "by mocha.bunyip.com (5.65a/IDA-1.4.2b/CC-Guru-2b)        id AA24721  (mail destined for Gary.Gere@eng.sun.com) on Fri, 17 Feb 95 19:35:24 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Date", "Fri, 17 Feb 95 19:35:24 -0500");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Message-Id", "<9502180035.AA24721@mocha.bunyip.com>");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("From", "paf@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("To", "MIME-tester@bunyip.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Cc", "mail-testers@test.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Cc", "interest-lists@mailtest.com");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Bcc", "tattle-tale@bcc.gov");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Bcc", "nsa-secrets@nsa.gov");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Mime-Version", "1.0");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Subject", "M4.3.1 - Content-Type: multipart/mixed; boundary=this_is_a_boundary");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Type", "multipart/mixed; boundary=this_is_a_boundary");
      msgcmp->_tcmcHeaders(-1).SetBothStrings("Content-Length", "1124");
      msgcmp->_tcmcContents = 
	" This is a text which is NOT included in the MIME message.\n"
	" Therefore, this text should not be visible for the user.\n";
      msgcmp->_tcmcAttributes.ClearAllElements();
      msgcmp->_tcmcAttributes(-1).SetBothStrings("boundary","this_is_a_boundary");
      msgstr = new SdmDpMessageStructure();
      msgstr->ms_type = Sdm_MSTYPE_multipart;
      msgstr->ms_encoding = Sdm_MSENC_7bit;
      msgstr->ms_subtype = "mixed";
      msgstr->ms_id = "<9502180034.AA24496@mocha.bunyip.com>";
      msgstr->ms_ms = msgstrL;
      msgstr->ms_lines = 2;
      msgstr->ms_bytes = msgcmp->_tcmcContents.Length();
      msgstr->ms_component = msgcmp;
      msgstr->ms_disposition = Sdm_MSDISP_not_specified;
      msgstr->ms_unix_mode = 0;
      msg->_tcmStructure.AddElementToList(msgstr);
      break;

    case 4:
      break;

    default:
      assert(0);
    }

  return(msg);
}

void SdmDpDataChanTclient::gatherContents(SdmString& r_contents, const SdmDpContentType& contenttype, SdmDpMessageStructureL& msgstrL)
{
  int numEntries = msgstrL.ElementCount();
  SdmDpMessageStructure *mstp;
  SdmDpTclientMessageComponent *mcmp = 0;
  int i = 0, j = 0, numHeaders = 0;

  switch (contenttype) {
  case DPCTTYPE_rawfull:
    // This request is to get the entire message in raw format; we need to zip on through
    // all of the components and build up the output
    for (i = 0; i < numEntries; i++) {
      mstp = msgstrL[i];
      mcmp = (SdmDpTclientMessageComponent *)mstp->ms_component;
      // Attach all primary headers
      numHeaders = mcmp->_tcmcHeaders.ElementCount();
      r_contents += mcmp->_tcmcPrefix;
      for (j = 0; j < numHeaders; j++) {
	r_contents += (mcmp->_tcmcHeaders[j]).GetFirstString();
	r_contents += ": ";
	r_contents += (mcmp->_tcmcHeaders[j]).GetSecondString();
	r_contents += "\n";
      }
      r_contents += "\n";
      // Now include the "raw" body part for the entire message; cycle through the parts
      r_contents += mcmp->_tcmcContents;
      r_contents += mcmp->_tcmcPostfix;
      // If there is a submessage contained herein, append that too
      if (mstp->ms_ms)
	gatherContents(r_contents, contenttype, *mstp->ms_ms);
    }
    return;
  case DPCTTYPE_rawheader:		// the entire message header in raw format
    // This request is to get the entire message header in raw format; we need to zip on through
    // the headers and build up the output
    mstp = msgstrL[0];
    mcmp = (SdmDpTclientMessageComponent *)mstp->ms_component;
    // Attach all primary headers
    numHeaders = mcmp->_tcmcHeaders.ElementCount();
    r_contents += mcmp->_tcmcPrefix;
    for (j = 0; j < numHeaders; j++) {
      r_contents += (mcmp->_tcmcHeaders[j]).GetFirstString();
      r_contents += ": ";
      r_contents += (mcmp->_tcmcHeaders[j]).GetSecondString();
      r_contents += "\n";
    }
    return;
  case DPCTTYPE_rawbody:		// the entire message body in raw format
    // This request is to get the entire message body in raw format; we need to zip on through
    // the body component and build up the output
    for (i = 0; i < numEntries; i++) {
      mstp = msgstrL[i];
      mcmp = (SdmDpTclientMessageComponent *)mstp->ms_component;
      // Now include the "raw" body part for the entire message; cycle through the parts
      r_contents += mcmp->_tcmcContents;
      // If there is a submessage contained herein, append that too
      if (mstp->ms_ms)
	gatherContents(r_contents, DPCTTYPE_rawfull, *mstp->ms_ms);
    }
    return;
  case DPCTTYPE_processedcomponent:	// the "final contents" of the component in question
    assert(0);
  default:
    assert(0);
  }
}

/* Wildcard pattern match [from c-client misc.c module]
 * Accepts: base string
 *	    pattern string
 *	    delimiter character
 * Returns: T if pattern matches base, else NIL
 */

long SdmDpDataChanTclient::pmatch_full (const char *s,const char *pat,char delim)
{
  switch (*pat) {
  case '%':			/* non-recursive */
				/* % at end, OK if no inferiors */
    if (!pat[1]) return (delim && strchr (s,delim)) ? 0 : 1;
                                /* scan remainder of string until delimiter */
    do if (pmatch_full (s,pat+1,delim)) return 1;
    while ((*s != delim) && *s++);
    break;
  case '*':			/* match 0 or more characters */
    if (!pat[1]) return 1;	/* * at end, unconditional match */
				/* scan remainder of string */
    do if (pmatch_full (s,pat+1,delim)) return 1;
    while (*s++);
    break;
  case '\0':			/* end of pattern */
    return *s ? 0 : 1;	/* success if also end of base */
  default:			/* match this character */
    return (*pat == *s) ? pmatch_full (s+1,pat+1,delim) : 0;
  }
  return 0;
}
// --------------------------------------------------------------------------------
// SdmDpTclientMessageComponent derivative of SdmDpMessageComponent Implementation
// --------------------------------------------------------------------------------

// Constructor

SdmDpTclientMessageComponent::SdmDpTclientMessageComponent() : 
  _tcmcId(TclientComponentId), _tcmcMsgnum(0), _tcmcIdentifier(-1)
{
}

// Destructor

SdmDpTclientMessageComponent::~SdmDpTclientMessageComponent()
{
  _tcmcAttributes.ClearAllElements();
  _tcmcHeaders.ClearAllElements();
}

// Print method
// Used in testing only

void SdmDpTclientMessageComponent::Print() const
{
  printf("tc%d.%d", _tcmcMsgnum, _tcmcIdentifier); 
}

// Deep Copy
// Required to make a legitimate usable copy of a message component

SdmDpMessageComponent* SdmDpTclientMessageComponent::DeepCopy()
{
  SdmDpTclientMessageComponent *mcmp = new SdmDpTclientMessageComponent();
  int i = 0;
  mcmp->_tcmcAttributes.ClearAllElements();
  int numAttributes = _tcmcAttributes.ElementCount();
  mcmp->_tcmcAttributes.SetVectorSize(numAttributes);
  for (i = 0; i < numAttributes; i++)
    mcmp->_tcmcAttributes[i] = _tcmcAttributes[i];
  mcmp->_tcmcHeaders.ClearAllElements();
  int numHeaders = _tcmcHeaders.ElementCount();
  mcmp->_tcmcHeaders.SetVectorSize(numHeaders);
  for (i = 0; i < numHeaders; i++)
    mcmp->_tcmcHeaders[i] = _tcmcHeaders[i];
  mcmp->_tcmcPrefix = _tcmcPrefix;
  mcmp->_tcmcContents = _tcmcContents;
  mcmp->_tcmcPostfix = _tcmcPostfix;
  mcmp->_tcmcIdentifier = _tcmcIdentifier;
  mcmp->_tcmcMsgnum = _tcmcMsgnum;
  return(mcmp);
}

// Equality operator: needs to be able to identify an tclient component
// and compare it with another to see if its identical in value

int SdmDpTclientMessageComponent::operator==
  (
  const SdmDpMessageComponent& mcmp
  ) const 
{
  SdmDpTclientMessageComponent* tcmcmp = (SdmDpTclientMessageComponent *)&mcmp;
  return(this
	 && tcmcmp
	 && _tcmcId == TclientComponentId 
	 && tcmcmp->_tcmcId == TclientComponentId 
	 && tcmcmp->_tcmcMsgnum == _tcmcMsgnum 
	 && tcmcmp->_tcmcIdentifier == _tcmcIdentifier);
}

#endif	// defined(INCLUDE_TCLIENT)
