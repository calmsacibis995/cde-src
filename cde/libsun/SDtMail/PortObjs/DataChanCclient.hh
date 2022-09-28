/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "C client" data channel subclass
// --> It implements the "c client channel adapter" that interfaces
// --> the c client API to the data port api

#ifndef _SDM_DATACHANCCLIENT_H
#define _SDM_DATACHANCCLIENT_H

#pragma ident "@(#)DataChanCclient.hh	1.88 97/06/19 SMI"

#include <PortObjs/DataChan.hh>
#include <Utils/LockUtility.hh>
#include <stdio.h>
#include <mail.h>
#include <misc.h>
#include <nl.h>

#define CR '\015'
#define LF '\012'

const unsigned long CclientComponentId = 0xcafedeaf;

class SdmDpCclientLookahead {
public:
  SdmDpCclientLookahead(MAILSTREAM *stream, SdmMessageNumber start, SdmMessageNumber end);
  ~SdmDpCclientLookahead();
private:
  static long defaultLookahead;

  MAILSTREAM *_stream;
  SdmBoolean _lookaheadIncreased;
};


class SdmDpCclientMessageComponent : public SdmDpMessageComponent {
public:
  SdmDpCclientMessageComponent();
  virtual ~SdmDpCclientMessageComponent();
  virtual SdmDpMessageComponent *DeepCopy();
  virtual int operator== (const SdmDpMessageComponent& mcmp) const;
  virtual void Print() const;

protected:
  friend class SdmDpDataChanCclient;

  unsigned long _id;
  SdmString _section;
  const BODY *_bodyPart;
};

class SdmDpDataChanCclient : public SdmDpDataChan {

public:
  SdmDpDataChanCclient(SdmDataPort* parentDataPort);
  virtual ~SdmDpDataChanCclient();
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

// Append one message by raw contents to the message store
//
virtual SdmErrorCode AppendMessage
  (
  SdmError& err,
  const SdmString& contents,	// contents of the message to append
  const SdmString& date,
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

// Check for new messages in the message store
//
SdmErrorCode CheckForNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
  );

// Check for new messages in the message store
//
SdmErrorCode _ReturnNumberOfNewMessages
  (
  SdmError& err,
  SdmMessageNumber& r_nummessages	// returned number of "new" messages in the message store
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

SdmErrorCode SdmDpDataChanCclient::_CopyOrMoveMessages
  (
  SdmError& err,
  const SdmToken& token,	// token naming mailbox to move message to
  const char *sequence,
  long moveFlag
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
SdmErrorCode Create
  (
  SdmError& err,
  const SdmString& name			// name of object to create
  );

// Delete the currently open object
//
SdmErrorCode Delete
  (
  SdmError& err,
  const SdmString& name			// name of the message store to delete
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

SdmErrorCode MailStream(SdmError& err, void *&r_stream);

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
  SdmMessageNumberL& r_msgnums,	// augmented list of messages that match
  const SdmSearch& search	// search criteria to apply to all messages
  );

// Perform a search against a range of messages and return a list of messages that match
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumber startmsgnum,	// first message to operate on
  const SdmMessageNumber endmsgnum	// last message to operate on
  );

// Perform a search against a list of messages and return a list of messages that match
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmMessageNumberL& r_msgnums,		// augmented list of messages that match
  const SdmSearch& search,		// search criteria to apply to messages
  const SdmMessageNumberL& msgnums	// list of messages to operate on
  );


// Perform a search against a single message and return an indication of the search results
//
SdmErrorCode PerformSearch
  (
  SdmError& err,
  SdmBoolean& r_bool,			// augmented results of search against message
  const SdmSearch& search,		// search criteria to apply to message
  const SdmMessageNumber msgnum	// single message to operate on
  );


// Ping the IMAP server.
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

  static SdmBoolean StringToDate (unsigned short *date, const char **arg);
  static SdmBoolean StringToNumber (unsigned long *number, const char **arg);
  static SdmBoolean IsDateField (const char *arg);


void SetServerConnectionBroken
  (
  SdmBoolean isBroken
  );

void SetMailboxChangedByOtherUser
  (
  SdmBoolean isBroken
  );
  
void SetMailboxConnectionReadOnly
  (
  SdmBoolean isReadOnly
  );
  
protected:

// Close the current connection down
//
SdmErrorCode _Close
  (
  SdmError& err
  );

SdmErrorCode _CopyBodyStructure
  (
  SdmError &err,
  SdmDpMessageStructure *dpms,
  const SdmMessageNumber msgnum,
  BODY *body,
  char *section
  );

SdmErrorCode _CreateStringList
  (
  SdmError& err,
  STRINGLIST*& lines,
  const SdmStringL* str_hdrs,
  const SdmMessageHeaderAbstractFlags abst_hdrs,
  SdmBoolean using_abstract
  );

SdmErrorCode _ExtractHeaders
  (
  SdmError& err,
  SdmStrStrL* r_strstr_hdrs,             // list of header name/values fetched
  const SdmStringL* str_hdrs,             // ptr to list of names of headers to fetch
  SdmIntStrL* r_intstr_hdrs,             // list of header name/values fetched
  SdmMessageHeaderAbstractFlags abst_hdrs, // abstract headers to fetch
  const SdmMessageNumber msgnum          // single message to operate on
  );

SdmErrorCode _GenerateUnixFromLine
  (
  SdmError &err,
  SdmString &r_fromLine,
  SdmMessageNumber msgnum
  );

SdmErrorCode _GetMessageStructure
  (
  SdmError &err,
  SdmDpMessageStructureL &r_structure,
  const SdmMessageNumber msgnum,
  PART *part,
  char *section
  );

SdmErrorCode _ScanNamespace
  (
  SdmError& err,
  SdmIntStrL& r_names,		        // augmented list of names returned
  const SdmString& reference,	        // reference portion of names to scan for
  const SdmString& pattern,	        // pattern portion of names to scan for
  void (*func)(MAILSTREAM* stream, char* ref, char* pat)  // function pointer - either mail_list or mail_lsub
  );

SdmErrorCode _HandleErrorExitConditions
  (
  SdmError &err,
  const SdmErrorCode defaultErrorCode
  );

SdmErrorCode _SetMessageFlagValues
  (
  SdmError &err,
  const SdmBoolean flagValue, 
  const SdmMessageFlagAbstractFlags aflags,
  char *sequence
  );

SdmErrorCode _HandleInvalidMailboxState
  (
  SdmError& err
  );

void _HandleMailboxConnectionReadOnly
  (
  );
  
void _GetStringFromFlags
  (
  SdmString &r_flagsString, 
  SdmMessageFlagAbstractFlags flags
  ); 

protected:
  // 
  // Below are all the methods used to handle searching in the cclient.
  //
  
  // method that calls cclient to perform search with given SEARCHPGM.
  SdmErrorCode _PerformSearch(SdmError &err, SEARCHPGM *pgm);

  // creates search program from leaf obtained from search object.
  SdmErrorCode CreateSearchPgmForLeaf(SdmError err, SEARCHPGM *&pgm, SdmSearchLeaf *result );

  // Methods for expanding dynamic predicates (eg. "Today")
  char* UpdateDynamicPredicates(SdmSearchLeaf *f);
  
  // Methods for creating search program.
  SEARCHPGM *CreatePgmForAndExpression(SdmSearchLeaf **l, SEARCHPGM *pgm);
  SdmSearchLeaf* DeMorganizeExpression(SdmSearchLeaf **l);
  SEARCHOR* CreatePgmForOrExpression(SdmSearchLeaf **l, SEARCHPGM *first);
  SEARCHOR* CreateOrPgm(SdmSearchLeaf **l, SEARCHPGM *first, SdmBoolean *exp_end);
  void AddToSearchProgram(const char *pred, SdmBoolean NOT, SEARCHPGM *pgm);
  void AddToSearchProgram(SdmSearchLeaf *leaf, SEARCHPGM *pgm, 
          SdmString *savePredicate = NULL, SdmBoolean *saveNot = NULL);
  
  //
  // Methods for parsing strings to data types in cclient search structures.
  static char *CreateNonQuotedToken(const char **str, int *len);
  static SdmBoolean AddStringToList (STRINGLIST **string,  const char **arg);
  static void StringToSearchSet (SEARCHSET **set, const char **arg, long maxima);
  static SdmBoolean HandleStringToDate (unsigned short *date, const char **arg);
  
  //
  // Methods for creating cclient search structures.
  static SEARCHPGM* CreateNewSearchPgm(void);
  static void FlushSearchPgm(SEARCHPGM *pgm);
  static SEARCHOR* CreateNewSearchOr(void);
  static SEARCHPGMLIST* CreateNewSearchPgmList(void);
  static SEARCHHEADER* CreateNewSearchHeader(const char *line);
  static SEARCHSET* CreateNewSearchSet(void);
  static STRINGLIST *InitializeStringList(const char *str);
  static void FreeStringList(STRINGLIST *str);


  // Methods to implement the session locking and group permissions callbacks
  // We need to provide a series of callbacks (that are passed in to the mail_open
  // call) that allow the c-client to call us when critical locking/permissions
  // functionality is required.
#if defined(INCLUDE_SESSIONLOCKING)
  static void _SessionLockCallback(const SdmLockEventType lockEvent, void* userdata);
  static int _ObtainSessionLock(char *mailbox, void *userdata);
  static int _ReleaseSessionLock(char *mailbox, void *userdata);
#endif // defined (INCLUDE_SESSIONLOCKING)

  static int _EnableGroupPrivileges(void);
  static int _DisableGroupPrivileges(void);
  

#if defined(INCLUDE_SESSIONLOCKING)
  SdmError		    _dcLockError;	// error when lock object is manipulated
  SdmLockUtility*	_dcLockObject;	// lock object used for session locking
  SdmBoolean		_dcLockGrabIfOwnedByOther;	// grab lock if owned by other
  SdmBoolean		_dcLockIgnoreIfOwnedByOther;	// ignore if owned by other
  SdmBoolean		_dcLockNoSessionLocking;	// do not do any session locking
#endif // defined(INCLUDE_SESSIONLOCKING)

public:
  // These need to public so they may be accessed from the mm_ callbacks
  SdmString             _dcUsername;
  SdmString             _dcPassword;
  SdmIntStrL            *_dcScanCollector;
  SdmMessageStoreStatus *_dcStatusCollector;
  SdmMessageNumberL     *_dcExpungeCollector;
  SdmMessageNumberL     *_dcSearchCollector;
  SdmString             _dcHostname;
  SdmBoolean            _dcRemote;
  SdmError		_dcCclientError;	// error when c-client is called,
  SdmString		_dcCclientMessages;	// collector for messages from c-client
  SdmBoolean            _dcDebug;
  static SdmDpDataChanCclient* LastDataChanUsed();

private:

  SdmBoolean _OutOfBounds(SdmMessageNumber msgnum) const {
    return (msgnum < 1 || msgnum > _dcNmsgs) ? Sdm_True : Sdm_False;
  }

  SdmBoolean _OutOfBounds(SdmMessageNumber start, SdmMessageNumber end) const {
    return (start > end || start < 1 || end > _dcNmsgs) ?
      Sdm_True : Sdm_False;
  }

  SdmBoolean _OutOfBounds(const SdmMessageNumberL &msgnums) const {
    int len = msgnums.ElementCount();

    for (int i = 0; i < len; i++)
      if (msgnums[i] < 1 || msgnums[i] > _dcNmsgs)
        return Sdm_True;
    return Sdm_False;
  }
  
  static SdmToken	*_dcRefToken;		// reference token used to filter out incompatible requests
  static int		_dcOneTimeFlag;		// global one time initialization flag
  static SdmDpDataChanCclient* _dcLastDataChanUsed;
  MAILSTREAM            *_dcStream;
  unsigned long         _dcNmsgs;
  SdmBoolean            _dcReadonly;
  SdmBoolean            _dcDisconnected;
  SdmBoolean            _dcServerConnectionBroken;
  SdmBoolean		    _dcMailboxConnectionReadOnly;
  SdmBoolean            _dcMailboxChangedByOtherUser;
};

#endif	// _SDM_DATACHANCCLIENT_H
