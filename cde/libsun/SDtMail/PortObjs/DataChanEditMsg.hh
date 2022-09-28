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

#ifndef _SDM_DATACHANEDITMSG_H
#define _SDM_DATACHANEDITMSG_H

#pragma ident "@(#)DataChanEditMsg.hh	1.19 97/03/27 SMI"

#include <PortObjs/DataChan.hh>
#include <stdio.h>

// ---> edit message message component
// This class defines the message component, which contains the "real data"
// associated with each component of the message. A message is defined by a
// series of message structure objects, with each such object having a 
// message component that holds the data for that part of the message
//

const unsigned long EditMsgComponentId = 0xdeadbeef;

class SdmDpEditMsgMessageComponent : public SdmDpMessageComponent {
public:
  SdmDpEditMsgMessageComponent(long componentIdentifier);
  virtual ~SdmDpEditMsgMessageComponent();
  virtual SdmDpMessageComponent *DeepCopy();
  virtual int operator== (const SdmDpMessageComponent& mcmp) const;
  void AdjustOrigMessageNumber(SdmMessageNumber newMsgnum) { _emccOrigMessageNumber = newMsgnum; }
  virtual SdmBoolean IsComponentDeleted() const { return _emccIsDeleted; } 
  virtual void Print() const;	// for testing only

protected:
  friend class SdmDpDataChanEditMsg;

  unsigned long		_emccId;		// id that uniquely identifies an edit msg message component
  SdmBoolean		_emccIsDeleted;		// indicates if this body part has been marked for deletion
  SdmStrStrL		_emccAttributes;	// "attributes" for this component
  SdmContentBuffer	_emccContents;		// "contents" for this component
  SdmBoolean		_emccContentsFetched;	// Indicates if contents have been fetched for this component
  SdmDpMessageComponent* _emccOrigComponent;	// the real message component for the derived message
  SdmDataPort*		_emccOrigPort;		// the port where the derived message is located
  SdmDpDataChan*	_emccOrigChan;		// the channel where the derived message is located
  SdmMessageNumber	_emccOrigMessageNumber;	// the original message number for the derived message
  long			_emccIdentifier;	// unique identifier for this editmsg component
};

// ---> edit message data channel class
//

class SdmDpDataChanEditMsg : public SdmDpDataChan {
  
public:
  SdmDpDataChanEditMsg(SdmDataPort* thisDp);
  SdmDpDataChanEditMsg(SdmDataPort* thisDp, SdmDataPort* srcDp, const SdmMessageNumber srcMsgnum);
  virtual ~SdmDpDataChanEditMsg();
  SdmErrorCode StartUp(SdmError& err);
  SdmErrorCode ShutDown(SdmError& err);
  
public:

// DATA CHANNEL API
// This is the API that is exported to the users of the data channel

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

// Create a new message in a message store
//
SdmErrorCode CreateNewMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum	// message number of newly created message
  );

// Create a new message in a message store derived from an existing message
//
SdmErrorCode CreateDerivedMessage
  (
  SdmError& err,
  const SdmMessageNumber newmsgnum,	// message number of newly created message
  SdmDataPort& msgdp,			// data port where message to derive from is located
  const SdmMessageNumber msgnum		// message number on data port of message to derive from
  );

// Delete a message body from an existing message
//
SdmErrorCode DeleteMessageBodyFromMessage
  (
  SdmError& err,
  SdmBoolean& r_previousState,		// previous state of message body
  const SdmBoolean newState,		// true to delete, false to undelete
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent& mcmp	// component of message to delete
  );

// Discard any pending changes to a message
//
SdmErrorCode DiscardPendingMessageChanges
  (
  SdmError& err,
  const SdmMessageNumber msgnum	// message number to discard changes for
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
virtual SdmErrorCode GetMessageCacheStatus
  (
  SdmError &err,
  SdmBoolean &r_cached,                 // augmented cached status
  const SdmMessageNumber msgnum,        // message number to get contents of
  const SdmDpMessageComponent &mcmp     // component of message to operate on
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

// open a connection to an object given a token stream
//
SdmErrorCode Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  );

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

#ifdef INCLUDE_UNUSED_API

// Set a specific attribute for a specific component of a message
//
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

SdmErrorCode		ReviseOriginalMessageNumber(SdmError& err, const SdmMessageNumber newMsgnum, const SdmMessageNumber newSrcMsgnum);
SdmErrorCode		FetchAllContents(SdmError& err, const SdmMessageNumber msgnum);
SdmErrorCode		FetchAllContents(SdmError& err, SdmDpMessageStructureL& msgstr);

private:

  // The "message" that this instance of the edit message channel adapter is working on

  SdmBoolean			_emcEditedMessage;	// indicates if this is an edited replacement for an existing message
  SdmMessageNumber		_emcSrcMsgnum;		// derived message number reference from data port
  SdmDataPort*			_emcSrcPort;		// data port where derived message exists
  SdmDpDataChan*		_emcSrcChan;		// data channel where derived message exists
  SdmDataPort*			_emcPort;		// data port where this message exists
  SdmMessageNumber		_emcMsgnum;		// message number for this message

  SdmDpMessageStructureL	_emcMessageStructure;	// structure for this message
  SdmMessageFlagAbstractFlags	_emcMessageFlags;	// flags for this message
  SdmStrStrL			_emcMessageHeaders;	// headers for this message
  long				_emcComponentIdentifierCeiling;	// ceiling for component indexes

protected:

  void AdjustMessageStructure(SdmDpMessageStructureL& msgstr,SdmMessageNumber newMsgnum);
  void DopeUpMessageStructure(SdmDpMessageStructureL& msgstr, const SdmDataPort* dp, const SdmDpDataChan* dc, const SdmMessageNumber msgnum);
  SdmDpMessageStructure* CreateNewMessageBody(const SdmMsgStrType bodytype, const SdmString& bodysubtype);
  SdmErrorCode FetchContents(SdmError& err, SdmDpMessageStructure& emstr, SdmDpEditMsgMessageComponent& emcmp);
  SdmErrorCode LocalizeMessage(SdmError& err, SdmDpEditMsgMessageComponent*& r_emcmp, SdmDpMessageStructure*& r_emstr, const SdmDpMessageComponent& mcmp, const SdmDpMessageStructureL& msgstr, const SdmMessageNumber msgnum);
};

#endif	// _SDM_DATACHANEDITMSG_H
