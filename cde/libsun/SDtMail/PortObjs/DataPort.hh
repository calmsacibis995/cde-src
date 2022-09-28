/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Data Port class.
// --> It implements the "data port api" that is used to
// --> gain access to underlying channel adapters that provide
// --> interfaces to various service providers.

#ifndef _SDM_DATAPORT_H
#define _SDM_DATAPORT_H

#pragma ident "@(#)DataPort.hh	1.107 97/05/13 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Token.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/PortObjects.hh>
#include <PortObjs/DataChan.hh>
#include <DataStructs/MutexEntry.hh>
#include <DataStructs/RecursiveMutexEntry.hh>

// Forward declarations
class SdmDataPort;
class SdmDpMessageComponent;
class SdmMessage;

// SdmDataPort class: implements a data port object which is used to access
// a data channel
//
class SdmDataPort : public SdmPrim {

public:
  SdmDataPort(const SdmClassId classId);
  virtual ~SdmDataPort();
  SdmObjectSignature	Signature() { return (_dpObjSignature); }
  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

public:
  
// --------------------------------------------------------------------------------
// DATA PORT API
// This is the API that is exported to the users of the data port
// NOTE: return values that start with "r_copiesof_" return values that must be
// either explicitly deleted directly, by calling the ClearAndDestroyAllElements()
// method, or implicitly by calling the SetPointerDeleteFlag() method, which
// causes the contents of the container to be deleted when the container is destroyed.
// --------------------------------------------------------------------------------

// Add a message body to the top level of an existing message
//
SdmErrorCode AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum
  );

// Add a message body to a component of an existing message
//
SdmErrorCode AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Add additional header text to existing header for a single message
//
SdmErrorCode AddMessageHeader
  (
  SdmError& err, 
  const SdmString& hdr, 		// name of header to add
  const SdmString& newvalue, 		// new header value 
  const SdmMessageNumber msgnum	// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  );

#endif

// Append one message by raw contents to the message store
//
SdmErrorCode AppendMessage
  (
  SdmError& err,
  const SdmString& contents,		     // message raw contents to append
  const SdmString& date,		      	// message raw contents to append
  const SdmString& from,
  SdmMessageFlagAbstractFlags flags 
  );

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to attach to
  );

// Cancel any operations pending on the data port
//
SdmErrorCode CancelPendingOperations
  (
  SdmError& err
  );

// This group of apis check to see if the data port is in a particular state or not
//
SdmErrorCode CheckAttached(SdmError& err);
SdmErrorCode CheckAttachedOrOpen(SdmError& err);
SdmErrorCode CheckOpen(SdmError& err);
SdmErrorCode CheckStarted(SdmError& err);

// Close the current connection down
//
SdmErrorCode Close
  (
  SdmError& err
  );

// Commit any pending changes to a message
//
SdmErrorCode CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messageRewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  );

// Copy a message from the current mailbox to a specified mailbox
//
SdmErrorCode CopyMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to copy message to
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Copy a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Copy a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

// Check for new messages in the message store
//
SdmErrorCode CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  );

// Create a new message store from a token stream
//
SdmErrorCode Create
  (
  SdmError& err,
  const SdmString& name			// name of object to create
  );

// Create a new message in a message store
//
SdmErrorCode CreateNewMessage
  (
  SdmError& err,
  SdmMessageNumber& r_msgnum		// message number of newly created message
  );

// Create a new message in a message store derived from an existing message
//
SdmErrorCode CreateDerivedMessage
  (
  SdmError& err,
  SdmMessageNumber& r_msgnum,		// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  );

// Delete the currently open object
//
SdmErrorCode Delete
  (
  SdmError& err,
  const SdmString& name			// name of the message store to delete
  );

// Delete a message body from an existing message
//
SdmErrorCode DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,	// message number containing message body to delete
  const SdmDpMessageComponent& mcmp	// component of message to delete
  );

// Discard any pending changes to a message
//
SdmErrorCode DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  );

// Disconnect the currently open object from its server
//
SdmErrorCode Disconnect
  (
  SdmError& err
  );

// Expunge deleted messages from the currently open message store
//
SdmErrorCode ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// list of message numbers of messages deleted
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a specific attribute for a specific component of a message
//
SdmErrorCode GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a list of specific attributes for a specific component of a message
//
SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

#endif

// Returns boolean indicating whether message is currently cached.
//
SdmErrorCode GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  );

// Return complete contents for a specific component of a message
//
SdmErrorCode GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// contents of component fetched
  const SdmDpContentType  contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents from
  );

// Return complete contents for a specific component of a message
//
SdmErrorCode GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// contents of component fetched
  const SdmDpContentType  contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return flags for a single message
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Return a list of flags for a range of messages
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of flags for a list of messages
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

// Return a list of headers for a single header for a single message
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  );

SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const char* hdr,			// name of header to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  )
  {
  return (GetMessageHeader(err, r_hdr, SdmString(hdr), msgnum));
  }

// Return a list of all headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );
  
// Return a list of specific headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Return a list of headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Return a list of headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );


// Return a list of lists of headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

// Return a list of list of headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );
  
#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const char* hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  )
  {
  return (GetMessageHeader(err, r_copiesof_hdr, SdmString(hdr), startmsgnum, endmsgnum));
  }

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of header values fetched
  const char* hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  )
  {
  return (GetMessageHeader(err, r_copiesof_hdr, SdmString(hdr), msgnums));
  }
  

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,		// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );


// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdrs,		// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdrs,		// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );
  
#endif



#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Return a list of lists of headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_copiesof_hdr,			// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );


#endif


// Return the message structure for a single message
//
SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_copiesof_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of message structures for a list of messages
//
SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_copiesof_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );
  
#endif

// Return a message store status structure for the message store
//
SdmErrorCode GetMessageStoreStatus
  (
  SdmError& err,
  SdmMessageStoreStatus& r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  );

#ifdef INCLUDE_UNUSED_API

// Move a message from the current mailbox to a specified mailbox
//
SdmErrorCode MoveMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Move a range of messages from the current mailbox to a specified mailbox
//
SdmErrorCode MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Move a list of messages from the current mailbox to a specified mailbox
//
SdmErrorCode MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

#endif

// open a connection to an object given a token stream
//
SdmErrorCode Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  );

// Perform a search against all messages and return a list of messages that match
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,	// list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  );
  

// Perform a search against a range of messages and return a list of messages that match
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );


// Perform a search against a list of messages and return a list of messages that match
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );
  

// Perform a search against a single message and return an indication of the search results
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// results of search against message
  const SdmSearch& search,		// search criteria to apply to message
  const SdmMessageNumber msgnum	// single message to operate on
  );



// Ping the server.
//
SdmErrorCode PingServer
  (
  SdmError &err
  );
  
// Reconnect the currently disconnected object to its server
//
SdmErrorCode Reconnect
  (
  SdmError& err
  );

// Register a "group privilege action callback" with the data port machinery
//
static SdmErrorCode RegisterGroupPrivilegeActionCallback
  (
  SdmError& err,
  SdmDpGroupPrivilegeActionCallback callback,
  void* userdata = NULL
  );

// Register a "callback" with the data port object
//
SdmErrorCode RegisterCallback
  (
  SdmError& err,
  const SdmDpCallbackType callbacktype,
  SdmDpCallback callback,
  void* userdata = NULL
  );


// Rename the current object given a new token stream
//
SdmErrorCode Rename
  (
  SdmError& err,
  const SdmString& currentname,		// current name for message store
  const SdmString& newname		// new name for message store
  );
  

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
SdmErrorCode RemoveFromSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name			// name to be removed from the subscribed namespace
  );

#endif

// Remove a specific message header from a single message
//
SdmErrorCode RemoveMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be removed
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Replace specific header completely with new value for a single message
//
SdmErrorCode ReplaceMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be replaced
  const SdmString& newvalue,		// new header value
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Determine if a message has any addressess that a transport can handle
// and if so resolve (dealias) and produce a list of them - preparing
// the message for transmission
//
SdmErrorCode ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  );

// Cause the complete state of the message store to be saved
// 
SdmErrorCode SaveMessageStoreState
  (
  SdmError& err
  );

// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
SdmErrorCode ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

// Set a specific attribute for a specific component of a message
///
SdmErrorCode SetMessageAttribute
  (
  SdmError& err,
  const SdmString& attribute,		// name of attribute to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Set a specific attribute for a specific component of a message
//
SdmErrorCode SetMessageAttribute
  (
  SdmError& err,
  const SdmMessageAttributeAbstractFlags attribute,	// abstract attribute value to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

#endif

// Set the complete contents for a specific component of a message
//
SdmErrorCode SetMessageContents
  (
  SdmError& err,
  const SdmContentBuffer& contents,	// new contents for component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Set a set of flags for a single message
//
SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear);
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Set a set of flags for a range of messages
//
SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear);
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Set a set of flags for a list of messages
//
SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear);
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

// Set all headers for a single message, replacing all existing headers
//
SdmErrorCode SetMessageHeaders
  (
  SdmError& err,
  const SdmStrStrL& hdr,		// list of header name/values to set
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Set the message structure for a component of a message
//
SdmErrorCode SetMessageBodyStructure
  (
  SdmError& err,
  const SdmDpMessageStructure& msgstr,	// new message structure value for the specified component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Submit a fully constructed message for transmission
//
SdmErrorCode Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL& customHeaders
  );

SdmErrorCode MailStream
  (
  SdmError& err,
  void *&r_stream
  );

// --------------------------------------------------------------------------------
// Data components of a data port object
// --------------------------------------------------------------------------------

  void			CallLockActivityCallback(SdmDpEventType lockActivityEvent);
  void			CallBusyIndicationCallback();
  void			CallDebugLogCallback(SdmString& debugMessage);
  void			CallErrorLogCallback(SdmString& errorMessage);
  void			CallNotifyLogCallback(SdmString& notifyMessage);

  // These methods are static as they are not specific to a single data port object
  // but are global to all data ports

  static void		CallGroupPrivilegeActionCallback(SdmBoolean enableFlag);

typedef struct DpMsgnumToChan_T
{
  SdmMessageNumber	_mtcMsgnum;		// the message number understood by the caller (matches redirect)
  SdmDpDataChan*	_mtcChan;		// the data channel where this message exists

  SdmMessageNumber	_mtcSrcMsgnum;		// the message number of the "original source message"
  SdmDataPort*		_mtcSrcPort;		// the data port of the "original source message"

  int operator==(const struct DpMsgnumToChan_T& other) const
  {
    return (_mtcMsgnum == other._mtcMsgnum &&
           _mtcChan == other._mtcChan &&
           _mtcSrcMsgnum == other._mtcSrcMsgnum &&
           _mtcSrcPort == other._mtcSrcPort);
  }

} DpMsgnumToChan;

typedef SdmVector<DpMsgnumToChan> SdmMsgnumToChanL;

private:
  SdmDpDataChan		*_dpDataChan;		// data channel this port is open on (when _dpOpen set)
  SdmDpDataChanL	 _dpDataChanL;		// data channels that responded to an "attach" (when _dpAttach set)
  SdmBoolean		 _dpOpen;		// true if a single channel is fully open
  SdmBoolean		 _dpAttached;		// true if attached to at least one channel
  SdmObjectSignature 	 _dpObjSignature;	// signature of this data port
  mutex_t		 _dpConcurrencyLock;	// concurrency lock on data port object
  SdmDpDataChanProviderL _dpDataChanProvL;	// list of data channel providers available to open/attach
  SdmMsgnumToChanL	 _dpMsgnumToChanL;	// message number to data channel adapter mapping list
  SdmMessageNumber	 _dpLocalMessageNumberCeiling;	// lowest local message number (used to edit/create)
  SdmRecursiveMutexEntry::Data _dpMutexData;

  // Private class methods

  friend class SdmDpDataChanEditMsg;

  SdmErrorCode		RedirectMessage(SdmError& err, const SdmMessageNumber msgnum);
  SdmDpDataChan*	MsgnumToChan(const SdmMessageNumber msgnum);
  SdmDpDataChan*	MsgnumToChan(SdmBoolean& r_isRedirected, const SdmMessageNumber msgnum);
  void			FetchAllContents();
  void			ProcessExpungeResults(SdmMessageNumberL& msgnums);
  void			ShutdownRedirectedMessage(SdmMessageNumber msgnum, SdmBoolean shutdownFlag);
  void			ShutdownAllRedirectedMessages();
  void			ReleaseOpenChannels();
  void			ReleaseAttachedChannels();

  // Data Port support for callbacks
  //

  SdmDpCallback			_dpErrorLogCallback;
  void*				_dpErrorLogUserdata;

  SdmDpCallback			_dpDebugLogCallback;
  void*				_dpDebugLogUserdata;

  SdmDpCallback			_dpNotifyLogCallback;
  void*				_dpNotifyLogUserdata;

  SdmDpCallback			_dpBusyIndicationCallback;
  void*				_dpBusyIndicationUserdata;

  SdmDpCallback			_dpLockActivityCallback;
  void*				_dpLockActivityUserdata;

  // Disallow creating copies of SdmDataPort

  SdmDataPort(const SdmDataPort& copy);
  SdmDataPort& operator=(const SdmDataPort& rhs);

  // Static class data members

  static SdmObjectSignature	_dpObjSignatureCounter;	// highest signature to date
  static SdmDpGroupPrivilegeActionCallback		_dpGroupPrivilegeActionCallback;
  static void* 			_dpGroupPrivilegeActionUserData;
};

typedef struct {
  SdmMessageAttributeAbstractFlag flag;
  const char *                    abstractName;
  const char *                    realName;
} SdmAbstractAttributeMap;

typedef struct {
  SdmMessageHeaderAbstractFlag	flag;
  const char *			abstractName;
  const char *			realName;
  int			        realNameLength;
  SdmMessageHeaderAbstractFlag  *indirectHeaderList;
  SdmBoolean                    useShortInfo;
} SdmAbstractHeaderMap;

typedef struct {
  SdmMessageFlagAbstractFlag	flag;
  const char *			abstractName;
} SdmAbstractFlagMap;

// Declare the global tables that hold the abstract values.
extern SdmAbstractAttributeMap _sdmAbstractAttributes[];
extern SdmAbstractHeaderMap _sdmAbstractHeaders[];
extern SdmAbstractFlagMap _sdmAbstractFlags[];

#endif	// _SDM_DATAPORT_H
