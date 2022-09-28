/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "T client" data channel subclass
// --> It implements a "testing client" that gives a canned set of
// --> responses always - used to perform a consistent test of
// --> the data channel interface.

#ifndef _SDM_DATACHANTCLIENT_H
#define _SDM_DATACHANTCLIENT_H

#pragma ident "@(#)DataChanTclient.hh	1.56 97/03/27 SMI"

//
// This is a test client and it is not compiled into production versions.
// To facilitate this, key off of the assert mechanism. If asserts are
// enabled then include the tclient, else do not.
//

#if	defined(NDEBUG)
#undef	INCLUDE_TCLIENT
#else
#define	INCLUDE_TCLIENT
#endif

#if	defined(INCLUDE_TCLIENT)

#include <PortObjs/DataChan.hh>
#include <stdio.h>

// A "t client message" is a simple representation of a message
// that is used to manage the pseudo mailbox the tclient implements
// Its just a container
//
class SdmDpTclientMessage {
public:

  SdmDpTclientMessage() {};
  ~SdmDpTclientMessage() { 
    _tcmStructure.ClearAndDestroyAllElements();
  }

  long		_tcmUid;	// the "uid" for the message
  SdmMessageFlagAbstractFlags _tcmFlags;	// the "flags" for the message
  SdmDpMessageStructureL _tcmStructure;	// the "structure" for the message
};
typedef SdmPtrVector<SdmDpTclientMessage *> SdmDpTclientMessageL;

const unsigned long TclientComponentId = 0xfeedbed;

class SdmDpTclientMessageComponent : public SdmDpMessageComponent {
public:
  SdmDpTclientMessageComponent();
  virtual ~SdmDpTclientMessageComponent();
  virtual SdmDpMessageComponent *DeepCopy();
  virtual int operator== (const SdmDpMessageComponent& mcmp) const;
  virtual void Print() const;

protected:
  friend class SdmDpDataChanTclient;

  unsigned long _tcmcId;		// id that identifies a t client message component
  SdmString	_tcmcPrefix;		// leading data (e.g. boundary)
  SdmStrStrL	_tcmcHeaders;		// the "headers" for this component
  SdmStrStrL	_tcmcAttributes;	// "attributes" for this component
  SdmString	_tcmcContents;		// "contents" for this component
  SdmString	_tcmcPostfix;		// trailing data (e.g. boundary)
  int		_tcmcIdentifier;	// unique identifier for this component
  SdmMessageNumber _tcmcMsgnum;		// message number this component belongs to
};

class SdmDpDataChanTclient : public SdmDpDataChan {
  
public:
  SdmDpDataChanTclient(SdmDataPort* parentDataPort);
  virtual ~SdmDpDataChanTclient();
  SdmErrorCode StartUp(SdmError& err);
  SdmErrorCode ShutDown(SdmError& err);
  
public:

// DATA CHANNEL API
// This is the API that is exported to the users of the data channel

#ifdef INCLUDE_UNUSED_API

// Add a name to the subscribed namespace
//
SdmErrorCode AddToSubscribedNamespace
  (
  SdmError& err,
  const SdmString& name		// name to add to subscribed namespace
  );
  
#endif

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

// Check for new messages in the message store
//
SdmErrorCode CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  );

// Discard any pending changes to a message
//
SdmErrorCode DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
  );

// Expunge deleted messages from the currently open message store
//
SdmErrorCode ExpungeDeletedMessages
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums	// augmented list of message numbers of messages deleted
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


// Return boolean indicating whether message is currently cached.
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
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum	// message number to get contents of
  );


// Return complete contents for a specific component of a message
//
SdmErrorCode GetMessageContents
  (
  SdmError& err,
  SdmContentBuffer& r_contents,		// augmented contents of component fetched
  const SdmDpContentType contenttype,	// the type of contents to be returned
  const SdmMessageNumber msgnum,	// message number to get contents of
  const SdmDpMessageComponent& mcmp	// component of message to operate on
  );

// Return flags for a single message
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlags& r_aflags,	// augmented abstract flags value for flags fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );

// Return a list of flags for a range of messages
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of flags for a list of messages
//
SdmErrorCode GetMessageFlags
  (
  SdmError& err,
  SdmMessageFlagAbstractFlagsL& r_aflags,	// augmented list of abstract flags for flags fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

// Return a list of all headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// list of header name/values fetched
  const SdmMessageNumber msgnum	// single message to operate on
  );
  
#ifdef INCLUDE_UNUSED_API

// Return a list of lists of all headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of all headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of lists of header name/values fetched
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

#endif

// Return a list of headers for a single header for a single message
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrL& r_hdr,			// header value fetched
  const SdmString& hdr,			// name of header to fetch
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


#ifdef INCLUDE_UNUSED_API

// Return a list of lists of headers for a single header for a range of messages
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of headers for a single header for a list of messages
//
SdmErrorCode GetMessageHeader
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,			// list of header values fetched
  const SdmString& hdr,			// name of header to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );


// Return a list of lists of specific headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of lists of specific headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdrs,			// list of lists of header name/values fetched
  const SdmStringL& hdrs,		// list of names of headers to fetch
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );

#endif


// Return a list of headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );

#ifdef INCLUDE_UNUSED_API

// Return a list of list of headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Return a list of lists of headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmStrStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

#endif

// Return a list of headers for a single message
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrL& r_hdr,				// header value fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for headers to fetch
  const SdmMessageNumber msgnum		// single message to operate on
  );

// Return a list of list of headers for a range of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumber startmsgnum,		// first message to operate on
  const SdmMessageNumber endmsgnum		// last message to operate on
  );

// Return a list of lists of headers for a list of messages
//
SdmErrorCode GetMessageHeaders
  (
  SdmError& err,
  SdmIntStrLL& r_hdr,				// list of header values fetched
  const SdmMessageHeaderAbstractFlags hdr,	// abstract value for header to fetch
  const SdmMessageNumberL& msgnums		// list of messages to operate on
  );

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
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Return a list of message structures for a list of messages
//
SdmErrorCode GetMessageStructure
  (
  SdmError& err,
  SdmDpMessageStructureLL& r_structure,	// returned message structure list describing messages
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

// open a connection to an object given a token stream
//
SdmErrorCode Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
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
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

#ifdef INCLUDE_UNUSED_API

// Scan the subscribed namespace on the object and return a list of matching names
//
SdmErrorCode ScanSubscribedNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		// augmented list of names returned
  const SdmString& reference,	// reference portion of names to scan for
  const SdmString& pattern	// pattern portion of names to scan for
  );

#endif

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

private:
  int captureCmd(char * *bufPtr, char * cmd, ...);
  void gatherContents(SdmString& r_contents, const SdmDpContentType& contenttype, SdmDpMessageStructureL& msgstrL);
  SdmDpTclientMessage *makeMessage(SdmMessageNumber msgnum);
  long pmatch_full (const char *s,const char *pat,char delim);

protected:
  static SdmToken	*_dcRefToken;		// reference tokenused to filter out incompatible requests
  static int		_dcOneTimeFlag;		// global one time initialization flag
  SdmBoolean		_dcReadonly;		// is the message store open read only?
  SdmDpTclientMessageL	_dcMessages;		// messages in pseudo mailbox
  SdmIntStrL		_dcSubscribedList;	// the subscribed name list
};

#endif	// defined(INCLUDE_TCLIENT)

#endif	// _SDM_DATACHANTCLIENT_H
