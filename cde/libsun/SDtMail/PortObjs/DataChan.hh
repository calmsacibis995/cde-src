/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Channel Class.
// --> It defines the "API" that each channel adapter is required
// --> to implement; they subclass from this base class and implement
// --> all of the virtual methods defined to interface a particular
// --> "channel" to the data port api ("channel adapter")

#ifndef _SDM_DATACHAN_H
#define _SDM_DATACHAN_H

#pragma ident "@(#)DataChan.hh	1.90 97/05/13 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Token.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/PortObjects.hh>

// forward declarations

class SdmDpDataChan : public SdmPrim {
  
public:
  SdmDpDataChan(const SdmClassId classId, SdmDataPort* parentDataPort);
  virtual ~SdmDpDataChan();
  virtual SdmErrorCode StartUp(SdmError& err) = 0;
  virtual SdmErrorCode ShutDown(SdmError& err) = 0;
  
public:

// DATA CHANNEL API
// This is the API that is exported to the users of the data channel
//
// All methods in the data channel API are virtual methods; there are no
// native methods to the class. Those methods that each channel adapter
// must implement are pure virtual methods, while those methods that a 
// channel adapter can choose to not implement are ordinary virtual methods.

// Add a message body to the top level of an existing message
//
virtual SdmErrorCode AddMessageBodyToMessage
  (
  SdmError& err,
  SdmDpMessageStructure& r_msgstruct,
  const SdmMsgStrType bodytype,
  const SdmString& bodysubtype,
  const SdmMessageNumber msgnum
  );

// Add a message body to a component of an existing message
//
virtual SdmErrorCode AddMessageBodyToMessage
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
virtual SdmErrorCode AddMessageHeader
  (
  SdmError& err, 
  const SdmString& hdr, 		// name of header to add
  const SdmString& newvalue, 		// new header value 
  const SdmMessageNumber msgnum	// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
virtual SdmErrorCode AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  );
  
#endif

// Append one message by raw contents to the message store
//
virtual SdmErrorCode AppendMessage
  (
  SdmError& err,
  const SdmString& contents,	// message raw contents to append
  const SdmString& date,
  const SdmString& from,
  SdmMessageFlagAbstractFlags flags 
  );

// Attach to an object (as opposed to / as a precursor to / an "open")
//
virtual SdmErrorCode Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to attach to
  ) = 0;

// Cancel any operations pending on the data port
//
virtual SdmErrorCode CancelPendingOperations
  (
  SdmError& err
  ) = 0;

// Check for new messages in the message store
//
virtual SdmErrorCode CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  );

// Close the current connection down
//
virtual SdmErrorCode Close
  (
  SdmError& err
  ) = 0;

// Commit any pending changes to a message
//
virtual SdmErrorCode CommitPendingMessageChanges
  (
  SdmError& err,
  SdmBoolean& r_messagerewritten,	// Indicate if message written to different location in store
  const SdmMessageNumber msgnum,	// message number to commit changes for
  SdmBoolean includeBcc
  );

// Copy a message from the current mailbox to a specified mailbox
//
virtual SdmErrorCode CopyMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to copy message to
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Copy a range of messages from the current mailbox to a specified mailbox
//
virtual SdmErrorCode CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Copy a list of messages from the current mailbox to a specified mailbox
//
virtual SdmErrorCode CopyMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to copy messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

// Create a new message store from a token stream
//
virtual SdmErrorCode Create
  (
  SdmError& err,
  const SdmString& name			// name of object to create
  );

// Create a new message in a message store
//
virtual SdmErrorCode CreateNewMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum	// message number of newly created message
  );

// Create a new message in a message store derived from an existing message
//
virtual SdmErrorCode CreateDerivedMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum,	// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  );

// Delete the currently open object
//
virtual SdmErrorCode Delete
  (
  SdmError& err,
  const SdmString& name			// name of the message store to delete
  );

// Delete a message body from an existing message
//
virtual SdmErrorCode DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to delete
  );

// Discard any pending changes to a message
//
virtual SdmErrorCode DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  );

// Disconnect the currently open object from its server
//
virtual SdmErrorCode Disconnect
  (
  SdmError& err
  );

// Expunge deleted messages from the currently open message store
//
virtual SdmErrorCode ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// augmented list of message numbers of messages deleted
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of all attributes for a specific component of a message
//
virtual SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attribute name/values fetched
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a specific attribute for a specific component of a message
//
virtual SdmErrorCode GetMessageAttribute
  (
  SdmError& err,
  SdmString& r_attribute,		// attribute value fetched
  const SdmString& attribute,		// name of attribute to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a list of specific attributes for a specific component of a message
//
virtual SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list attributes values fetched
  const SdmStringL& attributes,		// list of names of attributes to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return a list of specific attributes for a specific component of a message
//
virtual SdmErrorCode GetMessageAttributes
  (
  SdmError& err,
  SdmStrStrL& r_attribute,		// list of attributes values fetched
  const SdmMessageAttributeAbstractFlags attributes,	// list of abstract attribute values to fetch
  const SdmMessageNumber msgnum,	// message number to get attributes from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

#endif


// Return boolean indicating whether message is currently cached.
//
virtual SdmErrorCode GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,		        // augmented cached status
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent &mcmp	// component of message to operate on
  );


// Return complete contents for a specific component of a message
//
virtual SdmErrorCode GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  );

// Return complete contents for a specific component of a message
//
virtual SdmErrorCode GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return flags for a single message
//
virtual SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Return a list of flags for a range of messages
//
virtual SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of flags for a list of messages
//
virtual SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

// Return a list of all headers for a single message
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );


#ifdef INCLUDE_UNUSED_API

// Return a list of lists of all headers for a range of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of all headers for a list of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );
#endif 


// Return a list of headers for a single header for a single message
//
virtual SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  );


// Return a list of specific headers for a single message
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdrs,			// list of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber msgnum	// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
virtual SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of headers for a single header for a list of messages
//
virtual SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );



// Return a list of lists of specific headers for a range of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of specific headers for a list of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

#endif

// Return a list of headers for a single message
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Return a list of lists of headers for a list of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

#endif

// Return a list of headers for a single message
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Return a list of list of headers for a range of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Return a list of lists of headers for a list of messages
//
virtual SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

// Return the message structure for a single message
//
virtual SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureL& r_structure,	// returned message structure describing message
  const SdmMessageNumber msgnum	// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of message structures for a range of messages
//
virtual SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );


// Return a list of message structures for a list of messages
//
virtual SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );
  
#endif

// Return a message store status structure for the message store
//
virtual SdmErrorCode GetMessageStoreStatus
  (
  SdmError& err,
  SdmMessageStoreStatus& r_status,		// returned current status of message store
  const SdmMessageStoreStatusFlags storeFlags	// flags indicating status to retrieve
  );

#ifdef INCLUDE_UNUSED_API

// Move a message from the current mailbox to a specified mailbox
//
virtual SdmErrorCode MoveMessage
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Move a range of messages from the current mailbox to a specified mailbox
//
virtual SdmErrorCode MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Move a list of messages from the current mailbox to a specified mailbox
//
virtual SdmErrorCode MoveMessages
  (
  SdmError& err,
  const SdmToken& token,		// token naming mailbox to move messages to
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

#endif

// open a connection to an object given a token stream
//
virtual SdmErrorCode Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  ) = 0;

// Perform a search against all messages and return a list of messages that match
//
virtual SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,	// augmented list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  );

// Perform a search against a range of messages and return a list of messages that match
//
virtual SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Perform a search against a list of messages and return a list of messages that match
//
virtual SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );


// Perform a search against a single message and return an indication of the search results
//
virtual SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// augmented results of search against message
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
virtual SdmErrorCode Reconnect
  (
  SdmError& err
  );


// Rename the current object given a new token stream
//
virtual SdmErrorCode Rename
  (
  SdmError& err,
  const SdmString& currentname,		// current name for message store
  const SdmString& newname		// new name for message store
  );

#ifdef INCLUDE_UNUSED_API

// Remove a name from the subscribed namespace
//
virtual SdmErrorCode RemoveFromSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name			// name to be removed from the subscribed namespace
  );

#endif

// Remove a specific message header from a single message
//
virtual SdmErrorCode RemoveMessageHeader
  (
  SdmError& err,
  const SdmString& hdr,			// name of header to be removed
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Replace specific header completely with new value for a single message
//
virtual SdmErrorCode ReplaceMessageHeader
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
virtual SdmErrorCode ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  );

// Cause the complete state of the message store to be saved
// 
virtual SdmErrorCode SaveMessageStoreState
  (
  SdmError& err
  );

// Scan the global namespace on the object and return a list of matching names
// In the "intstr" result, the "GetNumber()" portion is a SdmNamespaceFlag mask
//
virtual SdmErrorCode ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
virtual SdmErrorCode ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

// Set a specific attribute for a specific component of a message
//
virtual SdmErrorCode SetMessageAttribute
  (
  SdmError& err,
  const SdmString& attribute,		// name of attribute to set
  const SdmString& attribute_value,	// new attribute value
  const SdmMessageNumber msgnum,	// message number to set attributes of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Set a specific attribute for a specific component of a message
//
virtual SdmErrorCode SetMessageAttribute
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
virtual SdmErrorCode SetMessageContents
  (
  SdmError& err,
  const SdmContentBuffer& contents,	// new contents for component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Set a set of flags for a single message
//
virtual SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Set a set of flags for a range of messages
//
virtual SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Set a set of flags for a list of messages
//
virtual SdmErrorCode SetMessageFlagValues
  (
  SdmError& err,
  const SdmBoolean flagValue,			// new flag value (true- set, false- clear)
  const SdmMessageFlagAbstractFlags aflags,	// abstract flag values to be set
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

// Set all headers for a single message, replacing all existing headers
//
virtual SdmErrorCode SetMessageHeaders
  (
  SdmError& err,
  const SdmStrStrL& hdr,		// list of header name/values to set
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Set the message structure for a component of a message
//
virtual SdmErrorCode SetMessageBodyStructure
  (
  SdmError& err,
  const SdmDpMessageStructure& msgstr,	// new message structure value for the specified component
  const SdmMessageNumber msgnum,	// message number to get contents from
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Submit a fully constructed message for transmission
//
virtual SdmErrorCode Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL& customHeaders
  );

  virtual void			CallLockActivityCallback(SdmDpEventType lockActivityEvent);
  virtual void			CallBusyIndicationCallback();
  virtual void			CallDebugLogCallback(SdmString& debugMessage);
  virtual void			CallErrorLogCallback(SdmString& errorMessage);
  virtual void			CallNotifyLogCallback(SdmString& notifyMessage);
  static void			CallGroupPrivilegeActionCallback(SdmBoolean enableFlag);

  SdmBoolean   IsOpen() const { return _dcOpen; }

protected:
  friend class SdmDataPort;

  SdmBoolean			_dcOpen;		// this data channel is open
  SdmBoolean			_dcAttached;		// this data channel is attached
  SdmDataPort*			_dcParentDataPort;	// parent data port

  // Internal only functions for the data channel (not really part of the API per-se)

  virtual SdmErrorCode		CheckAttached(SdmError& err);
  virtual SdmErrorCode		CheckAttachedOrOpen(SdmError& err);
  virtual SdmErrorCode		CheckOpen(SdmError& err);
  virtual SdmErrorCode		CheckStarted(SdmError& err);
  virtual SdmErrorCode		FetchAllContents(SdmError& err, const SdmMessageNumber msgnum);
  virtual SdmErrorCode		FetchAllContents(SdmError& err, SdmDpMessageStructureL& msgstr);
  virtual SdmErrorCode		MailStream(SdmError& err, void *&r_stream);
  virtual SdmErrorCode		ReviseOriginalMessageNumber(SdmError& err, const SdmMessageNumber newMsgnum, const SdmMessageNumber newSrcMsgnum);
};

typedef SdmPtrVector<SdmDpDataChan *> SdmDpDataChanL;

#endif	// _SDM_DATACHAN_H
