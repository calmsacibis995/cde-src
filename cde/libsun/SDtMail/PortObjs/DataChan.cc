/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Channel Class.
// --> It defines the "API" that each channel adapter is required
// --> to implement; they subclass from this base class and implement
// --> all of the virtual methods defined to interface a particular
// --> "channel" to the data port api ("channel adapter")

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DataChan.cc	1.68 97/05/15 SMI"
#endif

// Include Files.
#include <PortObjs/DataPort.hh>
#include <PortObjs/DataChan.hh>
#include <Utils/TnfProbe.hh>

SdmDpDataChan::SdmDpDataChan(const SdmClassId classId, SdmDataPort* parentDataPort)
  :SdmPrim(classId), _dcOpen(Sdm_False), _dcAttached(Sdm_False),
   _dcParentDataPort(parentDataPort)
  
{
  TNF_PROBE_0 (SdmDpDataChan_constructor_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_constructor_start");

  TNF_PROBE_0 (SdmDpDataChan_constructor_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_constructor_end");
}

SdmDpDataChan::~SdmDpDataChan()
{
  TNF_PROBE_0 (SdmDpDataChan_destructor_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_destructor_start");

  TNF_PROBE_0 (SdmDpDataChan_destructor_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_destructor_end");
}

// Methods for which defaults are provided, but for which
// overrides can be specified by subclasses
//

void SdmDpDataChan::CallBusyIndicationCallback()
{
  TNF_PROBE_0 (SdmDpDataChan_CallBusyIndicationCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallBusyIndicationCallback_start");

  assert(_dcParentDataPort);
  _dcParentDataPort->CallBusyIndicationCallback();

  TNF_PROBE_0 (SdmDpDataChan_CallBusyIndicationCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallBusyIndicationCallback_end");
}

void SdmDpDataChan::CallLockActivityCallback(SdmDpEventType lockActivityEvent)
{
  TNF_PROBE_1 (SdmDpDataChan_CallLockActivityCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallLockActivityCallback_start",
	       tnf_ulong, lockActivityEvent, (unsigned long) lockActivityEvent);

  assert(_dcParentDataPort);
  _dcParentDataPort->CallLockActivityCallback(lockActivityEvent);

  TNF_PROBE_0 (SdmDpDataChan_CallLockActivityCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallLockActivityCallback_end");
}

// This is a static function as the group privileges are not based on any
// particular data port but are global to all data ports

void SdmDpDataChan::CallGroupPrivilegeActionCallback(SdmBoolean enableFlag)
{
  TNF_PROBE_1 (SdmDpDataChan_CallGroupPrivilegeActionCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallGroupPrivilegeActionCallback_start",
	       tnf_ulong, enableFlag, (unsigned long) enableFlag);

  SdmDataPort::CallGroupPrivilegeActionCallback(enableFlag);

  TNF_PROBE_0 (SdmDpDataChan_CallGroupPrivilegeActionCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallGroupPrivilegeActionCallback_end");
}

void SdmDpDataChan::CallDebugLogCallback(SdmString& debugMessage)
{
  TNF_PROBE_1 (SdmDpDataChan_CallDebugLogCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallDebugLogCallback_start",
	       tnf_string, debugMessage, (const char *) debugMessage);

  assert(_dcParentDataPort);
  _dcParentDataPort->CallDebugLogCallback(debugMessage);

  TNF_PROBE_0 (SdmDpDataChan_CallDebugLogCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallDebugLogCallback_end");
}

void SdmDpDataChan::CallErrorLogCallback(SdmString& errorMessage)
{
  TNF_PROBE_1 (SdmDpDataChan_CallErrorLogCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallErrorLogCallback_start",
	       tnf_string, debugMessage, (const char *) errorMessage);

  assert(_dcParentDataPort);
  _dcParentDataPort->CallErrorLogCallback(errorMessage);

  TNF_PROBE_0 (SdmDpDataChan_CallErrorLogCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallErrorLogCallback_end");
}

void SdmDpDataChan::CallNotifyLogCallback(SdmString& notifyMessage)
{
  TNF_PROBE_1 (SdmDpDataChan_CallNotifyLogCallback_start,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallNotifyLogCallback_start",
	       tnf_string, notifyMessage, (const char *) notifyMessage);

  assert(_dcParentDataPort);
  _dcParentDataPort->CallNotifyLogCallback(notifyMessage);

  TNF_PROBE_0 (SdmDpDataChan_CallNotifyLogCallback_end,
	       "dcapi SdmDpDataChan",
	       "sunw%debug SdmDpDataChan_CallNotifyLogCallback_end");
}

SdmErrorCode SdmDpDataChan::CheckAttachedOrOpen(SdmError& err)
{
  TNF_PROBE_0 (SdmDpDataChan_CheckAttachedOrOpen_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttachedOrOpen_start");

  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDpDataChan_CheckAttachedOrOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttachedOrOpen_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (!_dcOpen && !_dcAttached) {
    TNF_PROBE_0 (SdmDpDataChan_CheckAttachedOrOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttachedOrOpen_end");
    return(err = Sdm_EC_Closed);
  }
  TNF_PROBE_0 (SdmDpDataChan_CheckAttachedOrOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttachedOrOpen_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChan::CheckAttached(SdmError& err)
{
  TNF_PROBE_0 (SdmDpDataChan_CheckAttached_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttached_start");

  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDpDataChan_CheckAttached_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttached_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (_dcOpen) {
    TNF_PROBE_0 (SdmDpDataChan_CheckAttached_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttached_end");
    return(err = Sdm_EC_Open);
  }
  if (!_dcAttached) {
    TNF_PROBE_0 (SdmDpDataChan_CheckAttached_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttached_end");
    return(err = Sdm_EC_Closed);
  }
  TNF_PROBE_0 (SdmDpDataChan_CheckAttached_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckAttached_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChan::CheckOpen(SdmError& err)
{
  TNF_PROBE_0 (SdmDpDataChan_CheckOpen_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckOpen_start");

  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDpDataChan_CheckOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckOpen_end");
    return(err = Sdm_EC_Shutdown);
  }
  if (_dcAttached) {
    TNF_PROBE_0 (SdmDpDataChan_CheckOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckOpen_end");
    return(err = Sdm_EC_Attached);
  }
  if (!_dcOpen) {
    TNF_PROBE_0 (SdmDpDataChan_CheckOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckOpen_end");
    return(err = Sdm_EC_Closed);
  }
  TNF_PROBE_0 (SdmDpDataChan_CheckOpen_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckOpen_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChan::CheckStarted(SdmError& err)
{
  TNF_PROBE_0 (SdmDpDataChan_CheckStarted_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckStarted_start");

  if (!_IsStarted) {
    TNF_PROBE_0 (SdmDpDataChan_CheckStarted_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckStarted_end");
    return(err = Sdm_EC_Shutdown);
  }
  TNF_PROBE_0 (SdmDpDataChan_CheckStarted_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckStarted_end");
  return(Sdm_EC_Success);
}

SdmErrorCode SdmDpDataChan::ReviseOriginalMessageNumber(SdmError& err, const SdmMessageNumber newMsgnum, const SdmMessageNumber newSrcMsgnum)
{
  return(err = Sdm_EC_Fail);
}

SdmErrorCode SdmDpDataChan::FetchAllContents(SdmError& err, SdmDpMessageStructureL& msgstr)
{
  return(err = Sdm_EC_Fail);
}

SdmErrorCode SdmDpDataChan::FetchAllContents(SdmError& err, const SdmMessageNumber msgnum)
{
  return(err = Sdm_EC_Fail);
}

// Add a message body to the top level of an existing message
//
SdmErrorCode SdmDpDataChan::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum
  )
{
  TNF_PROBE_0 (SdmDpDataChan_AddMessageBodyToMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddMessageBodyToMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_AddMessageBodyToMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddMessageBodyToMessage_end");
  return (err = Sdm_EC_Operation_Unavailable);
}

// Add a message body to a component of an existing message
//
SdmErrorCode SdmDpDataChan::AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_AddMessageBodyToMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddMessageBodyToMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_AddMessageBodyToMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddMessageBodyToMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Add additional header text to existing header for a single message
//
SdmErrorCode SdmDpDataChan::AddMessageHeader
  (
  SdmError& err, 
  const SdmString& hdr, 		// name of header to add
  const SdmString& newvalue, 		// new header value 
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode SdmDpDataChan::AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_AddToSubscribedNamespace_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddToSubscribedNamespace_start");

  TNF_PROBE_0 (SdmDpDataChan_AddToSubscribedNamespace_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AddToSubscribedNamespace_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Append one message by raw contents to the message store
//
SdmErrorCode SdmDpDataChan::AppendMessage
  (
  SdmError& err,
  const SdmString& contents,	// message raw contents to append
  const SdmString& date,
  const SdmString& from,
  SdmMessageFlagAbstractFlags flags 
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_AppendMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AppendMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_AppendMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_AppendMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Check for new messages in the message store
//
SdmErrorCode SdmDpDataChan::CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_CheckForNewMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckForNewMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_CheckForNewMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CheckForNewMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Copy a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::CopyMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to copy message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_CopyMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_CopyMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Copy a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_CopyMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_CopyMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Copy a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_CopyMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_CopyMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CopyMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Commit any pending changes to a message
//
SdmErrorCode SdmDpDataChan::CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messagerewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_CommitPendingMessageChanges_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CommitPendingMessageChanges_start");

  TNF_PROBE_0 (SdmDpDataChan_CommitPendingMessageChanges_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CommitPendingMessageChanges_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Create a new message store from a token stream
//
SdmErrorCode SdmDpDataChan::Create
  (
  SdmError& err,
  const SdmString& name			// name of object to create
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_Create_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Create_start");

  TNF_PROBE_0 (SdmDpDataChan_Create_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Create_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Create a new message in a message store
//
SdmErrorCode SdmDpDataChan::CreateNewMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum	// message number of newly created message
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_CreateNewMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CreateNewMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_CreateNewMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CreateNewMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Create a new message in a message store derived from an existing message
//
SdmErrorCode SdmDpDataChan::CreateDerivedMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum,	// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_CreateDerivedMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CreateDerivedMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_CreateDerivedMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_CreateDerivedMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Delete the currently open object
//
SdmErrorCode SdmDpDataChan::Delete
  (
  SdmError& err,
  const SdmString& name			// name of the message store to delete
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_Delete_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Delete_start");

  TNF_PROBE_0 (SdmDpDataChan_Delete_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Delete_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Delete a message body from an existing message
//
SdmErrorCode SdmDpDataChan::DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to delete
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_DeleteMessageBodyFromMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_DeleteMessageBodyFromMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_DeleteMessageBodyFromMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_DeleteMessageBodyFromMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Discard any pending changes to a message
//
SdmErrorCode SdmDpDataChan::DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_DiscardPendingMessageChanges_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_DiscardPendingMessageChanges_start");

  TNF_PROBE_0 (SdmDpDataChan_DiscardPendingMessageChanges_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_DiscardPendingMessageChanges_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Disconnect the currently open object from its server
//
SdmErrorCode SdmDpDataChan::Disconnect
  (
  SdmError& err
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_Disconnect_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Disconnect_start");

  TNF_PROBE_0 (SdmDpDataChan_Disconnect_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Disconnect_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Expunge deleted messages from the currently open message store
//
SdmErrorCode SdmDpDataChan::ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// augmented list of message numbers of messages deleted
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_ExpungeDeletedMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ExpungeDeletedMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_ExpungeDeletedMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ExpungeDeletedMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttribute_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttribute_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttribute_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttribute_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageAttributes_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageAttributes_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Return boolean indicating whether message is currently cached.
//
SdmErrorCode SdmDpDataChan::GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageCacheStatus_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageCacheStatus_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageCacheStatus_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageCacheStatus_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageContents_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageContents_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageContents_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageContents_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChan::GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageContents_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageContents_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageContents_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageContents_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return flags for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of flags for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of flags for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageFlags_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageFlags_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of all headers for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Return a list of headers for a single header for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of specific headers for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}


#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeader_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeader_end");

  return (err = Sdm_EC_Operation_Unavailable);
}




// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif


// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Return a list of headers for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of list of headers for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of lists of headers for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return the message structure for a single message
//
SdmErrorCode SdmDpDataChan::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
SdmErrorCode SdmDpDataChan::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Return a list of message structures for a list of messages
//
SdmErrorCode SdmDpDataChan::GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageStructure_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStructure_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Return a message store status structure for the message store
//
SdmErrorCode SdmDpDataChan::GetMessageStoreStatus
  (
  SdmError& err,
  SdmMessageStoreStatus& r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_GetMessageStoreStatus_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStoreStatus_start");

  TNF_PROBE_0 (SdmDpDataChan_GetMessageStoreStatus_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_GetMessageStoreStatus_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

SdmErrorCode SdmDpDataChan::MailStream(SdmError& err, void *&r_stream)
{
  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Move a message from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::MoveMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_MoveMessage_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessage_start");

  TNF_PROBE_0 (SdmDpDataChan_MoveMessage_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessage_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Move a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_MoveMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_MoveMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Move a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode SdmDpDataChan::MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_MoveMessages_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessages_start");

  TNF_PROBE_0 (SdmDpDataChan_MoveMessages_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_MoveMessages_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Perform a search against all messages and return a list of messages that match
//
SdmErrorCode SdmDpDataChan::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,	// augmented list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_start");

  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Perform a search against a range of messages and return a list of messages that match
//
SdmErrorCode SdmDpDataChan::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_start");

  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Perform a search against a list of messages and return a list of messages that match
//
SdmErrorCode SdmDpDataChan::PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_start");

  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_end");

  return (err = Sdm_EC_Operation_Unavailable);
}


// Perform a search against a single message and return an indication of the search results
//
SdmErrorCode SdmDpDataChan::PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// augmented results of search against message
  const SdmSearch& search,		// search criteria to apply to message
  const SdmMessageNumber msgnum	// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_start");

  TNF_PROBE_0 (SdmDpDataChan_PerformSearch_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PerformSearch_end");

  return (err = Sdm_EC_Operation_Unavailable);
}


// Ping the server.
//
SdmErrorCode SdmDpDataChan::PingServer
  (
  SdmError &err
  )
{
  TNF_PROBE_0 (SdmDpDataChan_PingServer_start,
      "dpapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PingServer_start");

  TNF_PROBE_0 (SdmDpDataChan_PingServer_end,
      "dpapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_PingServer_end");

  return (err = Sdm_EC_Operation_Unavailable);
}


// Reconnect the currently disconnected object to its server
//
SdmErrorCode SdmDpDataChan::Reconnect
  (
  SdmError& err
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_Reconnect_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Reconnect_start");

  TNF_PROBE_0 (SdmDpDataChan_Reconnect_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Reconnect_end");

  return (err = Sdm_EC_Operation_Unavailable);
}


// Rename the current object given a new token stream
//
SdmErrorCode SdmDpDataChan::Rename
  (
  SdmError& err,
  const SdmString& currentname,		// current name for message store
  const SdmString& newname		// new name for message store
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_Rename_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Rename_start");

  TNF_PROBE_0 (SdmDpDataChan_Rename_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Rename_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
SdmErrorCode SdmDpDataChan::RemoveFromSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name			// name to be removed from the subscribed namespace
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_RemoveFromSubscribedNamespace_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_RemoveFromSubscribedNamespace_start");

  TNF_PROBE_0 (SdmDpDataChan_RemoveFromSubscribedNamespace_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_RemoveFromSubscribedNamespace_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Remove a specific message header from a single message
//
SdmErrorCode SdmDpDataChan::RemoveMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be removed
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  return (err = Sdm_EC_Operation_Unavailable);
}

// Replace specific header completely with new value for a single message
//
SdmErrorCode SdmDpDataChan::ReplaceMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be replaced
  const SdmString& newvalue,		// new header value
  const SdmMessageNumber msgnum	// single message to operate on
  )
{
  return (err = Sdm_EC_Operation_Unavailable);
}

// Determine if a message has any addressess that a transport can handle
// and if so resolve (dealias) and produce a list of them - preparing
// the message for transmission
//
SdmErrorCode SdmDpDataChan::ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  )
{
  TNF_PROBE_0 (SdmDpDataChan_ResolveAndExtractAddresses_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ResolveAndExtractAddresses_start");

  TNF_PROBE_0 (SdmDpDataChan_ResolveAndExtractAddresses_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ResolveAndExtractAddresses_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Cause the complete state of the message store to be saved
// 
SdmErrorCode SdmDpDataChan::SaveMessageStoreState
  (
  SdmError& err
  )
{
  TNF_PROBE_0 (SdmDpDataChan_SaveMessageStoreState_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SaveMessageStoreState_start");

  TNF_PROBE_0 (SdmDpDataChan_SaveMessageStoreState_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SaveMessageStoreState_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
SdmErrorCode SdmDpDataChan::ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_ScanNamespace_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ScanNamespace_start");

  TNF_PROBE_0 (SdmDpDataChan_ScanNamespace_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ScanNamespace_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode SdmDpDataChan::ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_ScanSubscribedNamespace_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ScanSubscribedNamespace_start");

  TNF_PROBE_0 (SdmDpDataChan_ScanSubscribedNamespace_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_ScanSubscribedNamespace_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChan::SetMessageAttribute
  (
  SdmError& err,
  const SdmString& attribute,		// name of attribute to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageAttribute_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageAttribute_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageAttribute_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageAttribute_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SdmDpDataChan::SetMessageAttribute
  (
  SdmError& err,
  const SdmMessageAttributeAbstractFlags attribute,	// abstract attribute value to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageAttribute_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageAttribute_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageAttribute_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageAttribute_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

#endif

// Set the complete contents for a specific component of a message
//
SdmErrorCode SdmDpDataChan::SetMessageContents
  (
  SdmError& err,
  const SdmContentBuffer& contents,	// new contents for component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageContents_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageContents_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageContents_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageContents_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set a set of flags for a single message
//
SdmErrorCode SdmDpDataChan::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set a set of flags for a range of messages
//
SdmErrorCode SdmDpDataChan::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set a set of flags for a list of messages
//
SdmErrorCode SdmDpDataChan::SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageFlagValues_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageFlagValues_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set all headers for a single message, replacing all existing headers
//
SdmErrorCode SdmDpDataChan::SetMessageHeaders
  (
  SdmError& err,
  const SdmStrStrL& hdr,		// list of header name/values to set
  const SdmMessageNumber msgnum		// single message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageHeaders_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageHeaders_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageHeaders_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageHeaders_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Set the message structure for a component of a message
//
SdmErrorCode SdmDpDataChan::SetMessageBodyStructure
  (
  SdmError& err,
  const SdmDpMessageStructure& msgstr,	// new message structure value for the specified component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  ) 
{
  TNF_PROBE_0 (SdmDpDataChan_SetMessageBodyStructure_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageBodyStructure_start");

  TNF_PROBE_0 (SdmDpDataChan_SetMessageBodyStructure_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_SetMessageBodyStructure_end");

  return (err = Sdm_EC_Operation_Unavailable);
}

// Submit a fully constructed message for transmission
//
SdmErrorCode SdmDpDataChan::Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL& customHeaders
  )
{
  TNF_PROBE_0 (SdmDpDataChan_Submit_start,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Submit_start");

  TNF_PROBE_0 (SdmDpDataChan_Submit_end,
      "dcapi SdmDpDataChan",
      "sunw%debug SdmDpDataChan_Submit_end");

  return (err = Sdm_EC_Operation_Unavailable);
}
