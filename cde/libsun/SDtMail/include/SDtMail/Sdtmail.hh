/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> These are the common definitions for Sdm sources.

#ifndef _SDM_H
#define _SDM_H

#pragma ident "@(#)Sdtmail.hh	1.100 97/06/12 SMI"

#ifndef NULL
#define NULL 0
#endif

#if	defined(NDEBUG)
#undef	INCLUDE_UNUSED_API
#else
#define	INCLUDE_UNUSED_API
#endif


// Major common simple type definitions
//
typedef enum SdmBoolean { Sdm_False = 0, Sdm_True = 1 };

static const int Sdm_Not_Found = -1;
static const int Sdm_Found = 1;

typedef unsigned int SdmErrorCode;		        // "error code"

static const SdmErrorCode Sdm_EC_Success = 0;			// successful completion of request (MUST BE 0!)
static const SdmErrorCode Sdm_EC_Fail = 1; 			// Call Failure
static const SdmErrorCode Sdm_EC_Operation_Unavailable = 2;	// unavailable operation
static const SdmErrorCode Sdm_EC_Cancel = 3; 			// requested operation cancelled
static const SdmErrorCode Sdm_EC_HostnameRequired = 4;
static const SdmErrorCode Sdm_EC_Shutdown = 5;			// operation denied: object is shutdown
static const SdmErrorCode Sdm_EC_Closed = 6;			// operation denied: object is closed
static const SdmErrorCode Sdm_EC_Attached = 7;			// operation denied: object is attached
static const SdmErrorCode Sdm_EC_Open = 8;			// operation denied: object is open
static const SdmErrorCode Sdm_EC_MessageStoreNameRequired = 9;
static const SdmErrorCode Sdm_EC_CannotAccessMessageStore = 10;
static const SdmErrorCode Sdm_EC_Readonly = 11;			// operation denied: object is read only
static const SdmErrorCode Sdm_EC_UsernameRequired = 12;
static const SdmErrorCode Sdm_EC_PasswordRequired = 13;
static const SdmErrorCode Sdm_EC_MessageExpunged = 14;
static const SdmErrorCode Sdm_EC_MessageStructureDataUndefined = 15;
static const SdmErrorCode Sdm_EC_ServiceFunctionAlreadyRegistered = 16;
static const SdmErrorCode Sdm_EC_ServiceFunctionNotRegistered = 17;
static const SdmErrorCode Sdm_EC_ConnectionAlreadyStarted = 18;
static const SdmErrorCode Sdm_EC_ConnectionNotStarted = 19;
static const SdmErrorCode Sdm_EC_SessionAlreadyStarted = 20;
static const SdmErrorCode Sdm_EC_SessionNotStarted = 21;
static const SdmErrorCode Sdm_EC_SessionAlreadyAttached = 22;
static const SdmErrorCode Sdm_EC_SessionNotAttached = 23;
static const SdmErrorCode Sdm_EC_ReturnArgumentListHasContents = 24;
static const SdmErrorCode Sdm_EC_RequestedDataNotFound = 25;	// a specific "get" did not find the information requested
static const SdmErrorCode Sdm_EC_NoMemory = 26;
static const SdmErrorCode Sdm_EC_MessageStoreAlreadyStarted = 27;
static const SdmErrorCode Sdm_EC_MessageStoreNotStarted = 28;
static const SdmErrorCode Sdm_EC_LocalServerNotRegistered = 29;
static const SdmErrorCode Sdm_EC_MessageIsNested = 30;
static const SdmErrorCode Sdm_EC_ServiceTypeRequired = 31;
static const SdmErrorCode Sdm_EC_BadArgument = 32;
static const SdmErrorCode Sdm_EC_BadMessageNumber = 33;
static const SdmErrorCode Sdm_EC_MessageNotFromOutgoingStore = 34;
static const SdmErrorCode Sdm_EC_BadMessageBodyPartNumber = 35;
static const SdmErrorCode Sdm_EC_NoAddressFoundInMessage = 36;
static const SdmErrorCode Sdm_EC_InsufficientAddresseeHeaders = 37;
static const SdmErrorCode Sdm_EC_SubmitDeletedMessage = 38;
static const SdmErrorCode Sdm_EC_InvalidOutgoingStoreToken = 39;
static const SdmErrorCode Sdm_EC_CommitDisconnectedMessage = 40;
static const SdmErrorCode Sdm_EC_AlreadyConnected = 41;
static const SdmErrorCode Sdm_EC_AlreadyDisconnected = 42;
static const SdmErrorCode Sdm_EC_InvalidServiceFunction = 43;
static const SdmErrorCode Sdm_EC_InactivityIntervalNotPassed = 44;
static const SdmErrorCode Sdm_EC_RFC822AddressParseFailure = 45;	// minor codes of Sdm_EC_PRH
static const SdmErrorCode Sdm_EC_SearchStringParseFailure = 46;		// minor codes of Sdm_EC_PRS
static const SdmErrorCode Sdm_EC_CannotObtainMailboxLock = 47;		// minor codes of Sdm_EC_MBL
static const SdmErrorCode Sdm_EC_ErrorProcessingMailrc = 48;		// minor codes of Sdm_EC_MRC
static const SdmErrorCode Sdm_EC_ErrorUpdatingMessageStore = 49;
static const SdmErrorCode Sdm_EC_IncompatibleMessageStoreTypes = 50;
static const SdmErrorCode Sdm_EC_BadMessageSequence = 51;
static const SdmErrorCode Sdm_EC_MessageRFC822MissingFromLine = 52;
static const SdmErrorCode Sdm_EC_CannotAttachToService = 53;
static const SdmErrorCode Sdm_EC_CannotDetermineCurrentWorkingDirectory = 54;
static const SdmErrorCode Sdm_EC_CannotAccessMailboxPathNameComponent = 55;
static const SdmErrorCode Sdm_EC_NullsDetectedInMail = 56;
static const SdmErrorCode Sdm_EC_CannotSendMessage = 57;
static const SdmErrorCode Sdm_EC_CannotConnectToSmtpMailServer = 58;
static const SdmErrorCode Sdm_EC_CclientError = 59;
static const SdmErrorCode Sdm_EC_CannotAccessNewMail = 60;
static const SdmErrorCode Sdm_EC_CannotConnectToIMAPServer = 61;
static const SdmErrorCode Sdm_EC_CannotAppendMessageToMailbox = 62;
static const SdmErrorCode Sdm_EC_CannotCopyOrMoveMessage = 63;
static const SdmErrorCode Sdm_EC_CannotDeleteMailbox = 64;
static const SdmErrorCode Sdm_EC_CannotRenameMailbox = 65;
static const SdmErrorCode Sdm_EC_CannotCreateMailbox = 66;
static const SdmErrorCode Sdm_EC_SmtpConnectionTimeout = 67;
static const SdmErrorCode Sdm_EC_SmtpServerErrorMessage = 68;
static const SdmErrorCode Sdm_EC_DecodingError = 69;

// error codes from ParseRFC822Headers [20xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being "Sdm_EC_RFC822AddressParseFailure"

static const SdmErrorCode Sdm_EC_PRH_BADROUTESPEC = 2000;			// <@route: addr> bad
static const SdmErrorCode Sdm_EC_PRH_EXTRAATSIGN = 2001;			// extra @
static const SdmErrorCode Sdm_EC_PRH_EXTRARAB = 2002;				// extra <
static const SdmErrorCode Sdm_EC_PRH_ILLEGALPHRASE = 2003;			// X@Y corrupted/missing pieces
static const SdmErrorCode Sdm_EC_PRH_MISSINGADDRESS = 2004;			// essentially <>
static const SdmErrorCode Sdm_EC_PRH_MISSINGDOMAIN = 2005;			// stuff@
static const SdmErrorCode Sdm_EC_PRH_MISSINGEC = 2006;				// missing escapee following "\"
static const SdmErrorCode Sdm_EC_PRH_MISSINGFLD = 2007;				// );); or some such
static const SdmErrorCode Sdm_EC_PRH_MISSINGGEND = 2008;			// missing ;
static const SdmErrorCode Sdm_EC_PRH_MISSINGHT = 2009;				// missing host
static const SdmErrorCode Sdm_EC_PRH_MISSINGLAB = 2010;				// missing <
static const SdmErrorCode Sdm_EC_PRH_MISSINGLB = 2011;				// missing [
static const SdmErrorCode Sdm_EC_PRH_MISSINGLP = 2012;				// missing (
static const SdmErrorCode Sdm_EC_PRH_MISSINGQQ = 2013;				// missing "
static const SdmErrorCode Sdm_EC_PRH_MISSINGRAB = 2014;				// missing >
static const SdmErrorCode Sdm_EC_PRH_MISSINGRB = 2015;				// missing ]
static const SdmErrorCode Sdm_EC_PRH_MISSINGRP = 2016;				// missing )
static const SdmErrorCode Sdm_EC_PRH_NULLADDRESS = 2017;			// empty field
static const SdmErrorCode Sdm_EC_PRH_SPURIOUSSC = 2018;				// bare ;

// error codes from SdmSearch object [21xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being "Sdm_EC_SearchStringParseFailure"

static const SdmErrorCode Sdm_EC_PRS_EmptyFilter = 2100;			// empty specification
static const SdmErrorCode Sdm_EC_PRS_ExpectedBoolean = 2101;			// too few operators for operands
static const SdmErrorCode Sdm_EC_PRS_IllegalKeyField = 2102;			// tab/space in key field
static const SdmErrorCode Sdm_EC_PRS_IllegalLeftParenthesisField = 2103;	// field contains illegal (
static const SdmErrorCode Sdm_EC_PRS_IllegalLeftParenthesisKeyField = 2104;	// key field contains illegal (
static const SdmErrorCode Sdm_EC_PRS_IllegalNumericField = 2105;		// numeric field contains non-numeric character
static const SdmErrorCode Sdm_EC_PRS_IllegalRightParenthesisKeyField = 2106;	// key field contains illegal )
static const SdmErrorCode Sdm_EC_PRS_IllegalRightParenthesisTermination = 2107;	// key field ) terminator not followed by tab/space
static const SdmErrorCode Sdm_EC_PRS_IllegalRightParenthesisToken = 2108;	// token contains ) where illegal
static const SdmErrorCode Sdm_EC_PRS_IllegalSearchToken = 2109;			// keyword/operator unknown
static const SdmErrorCode Sdm_EC_PRS_MissingDoubleQuote = 2110;			// terminating " missing
static const SdmErrorCode Sdm_EC_PRS_MissingField = 2111;			// expected field missing
static const SdmErrorCode Sdm_EC_PRS_MissingPredicate = 2112;			// expected data for operator missing
static const SdmErrorCode Sdm_EC_PRS_ParenthesisBad = 2113;			// unbalanced parenthesis - uneven ( and ) counts
static const SdmErrorCode Sdm_EC_PRS_IllegalDateField = 2114;		// date field is not in the correct date format (DD-MMM-YYY)
static const SdmErrorCode Sdm_EC_PRS_IllegalNotBeforeField = 2115;		// not operator before field (From ~ foo@bar)

// error code for mailbox locking [22xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being "Sdm_EC_CannotObtainMailboxLock"

static const SdmErrorCode Sdm_EC_MBL_CannotCreateMailboxLockFile = 2200;	// general creation failure
static const SdmErrorCode Sdm_EC_MBL_CannotCreateMailboxLockFile_IsDirectory = 2201;
static const SdmErrorCode Sdm_EC_MBL_CannotCreateMailboxLockFile_NoPermission = 2202;
static const SdmErrorCode Sdm_EC_MBL_CannotCreateMailboxLockFile_NoSuchFile = 2203;
static const SdmErrorCode Sdm_EC_MBL_CannotCreateMailboxLockFile_RemoteAccessLost = 2204;
static const SdmErrorCode Sdm_EC_MBL_CannotRemoveMailboxLockFile = 2205;
static const SdmErrorCode Sdm_EC_MBL_CannotRemoveStaleMailboxLockFile = 2206;
static const SdmErrorCode Sdm_EC_MBL_CannotStatMailboxLockFile = 2207;
static const SdmErrorCode Sdm_EC_MBL_TooltalkLockError = 2208;
static const SdmErrorCode Sdm_EC_MBL_TooltalkLockFileJoinError = 2209;
static const SdmErrorCode Sdm_EC_MBL_TooltalkNotResponding = 2210;
static const SdmErrorCode Sdm_EC_MBL_OtherMailerOwnsWriteAccess = 2211;
static const SdmErrorCode Sdm_EC_MBL_TooltalkSessionStartupError = 2212;
static const SdmErrorCode Sdm_EC_MBL_ThisMailerOwnsWriteAccess = 2213;

// error codes from SdmMailRc object [23xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being "Sdm_EC_ErrorProcessingMailrc"

static const SdmErrorCode Sdm_EC_MRC_CannotOpenMailrcForUpdating = 2300;
static const SdmErrorCode Sdm_EC_MRC_CannotOpenMailruleForUpdating = 2301;
static const SdmErrorCode Sdm_EC_MRC_CannotRenameNewMailrcFile = 2302;
static const SdmErrorCode Sdm_EC_MRC_CannotRenameNewMailruleFile = 2303;
static const SdmErrorCode Sdm_EC_MRC_ResourceParsingNoEndif = 2304;

// error codes for object access [24xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being one of many that cause access to an object

static const SdmErrorCode Sdm_EC_OBJ_UserNameNotFound = 2401;			// getpwxxx failed
static const SdmErrorCode Sdm_EC_OBJ_HomeDirPathMissingMessageStoreName = 2402;		// ~blah specified
static const SdmErrorCode Sdm_EC_OBJ_ErrorUpdatingMessageStore = 2403;
static const SdmErrorCode Sdm_EC_OBJ_CannotAccessMessageStore = 2404;
static const SdmErrorCode Sdm_EC_OBJ_ErrorRemovingMessageStore = 2405;
static const SdmErrorCode Sdm_EC_OBJ_CannotStatMessageStore = 2406;

// Error code for network related issues [25xx]
static const SdmErrorCode Sdm_EC_NET_BadPortNumberSpecified = 2500;			
static const SdmErrorCode Sdm_EC_NET_HostNameSyntaxError = 2501;
static const SdmErrorCode Sdm_EC_NET_HostDoesNotExist = 2502;
static const SdmErrorCode Sdm_EC_NET_CannotCreateConnection = 2503;
static const SdmErrorCode Sdm_EC_NET_ServiceNotAvailable = 2504;

// error codes for the c-client [26xx]
// These are all returned as a "minor" code to the library caller, with
// the major code being "Sdm_EC_CclientError"
static const SdmErrorCode Sdm_EC_CCL_ServerErrorMessage = 2600;
static const SdmErrorCode Sdm_EC_CCL_CantCreateSessionLock = 2601;
static const SdmErrorCode Sdm_EC_CCL_WaitingForLock = 2602;
static const SdmErrorCode Sdm_EC_CCL_TryingForLock = 2603;
static const SdmErrorCode Sdm_EC_CCL_IllegalFromLine = 2604;
static const SdmErrorCode Sdm_EC_CCL_IllegalMailbox = 2605;
static const SdmErrorCode Sdm_EC_CCL_MailboxShrank = 2606;
static const SdmErrorCode Sdm_EC_CCL_LockFailure = 2607;
static const SdmErrorCode Sdm_EC_CCL_MailboxContainsNulls = 2608;
static const SdmErrorCode Sdm_EC_CCL_MailboxInUse = 2609;
static const SdmErrorCode Sdm_EC_CCL_CantSetGid = 2610;
static const SdmErrorCode Sdm_EC_CCL_MailboxReadError = 2611;
static const SdmErrorCode Sdm_EC_CCL_WaitingForDestinationLock = 2612;
static const SdmErrorCode Sdm_EC_CCL_CantLoginToIMAPServer = 2613;
static const SdmErrorCode Sdm_EC_CCL_FileNotInMailboxFormat = 2614;
static const SdmErrorCode Sdm_EC_CCL_LocalErrorMessage = 2615;
static const SdmErrorCode Sdm_EC_CCL_MailboxNoLongerExists = 2616;

// Error codes for i18n
static const SdmErrorCode Sdm_EC_LocaleNotSetCorrectly = 3001; // setlocale
static const SdmErrorCode Sdm_EC_LclOpenFailed = 3002; // lcl_open failure
static const SdmErrorCode Sdm_EC_LctCreateFailure = 3003; // lct_create failure

typedef int SdmClassId;
typedef unsigned int SdmObjectSize;
typedef unsigned long SdmObjectSignature;
 
// Set Sizes
static const int SdmD_SessionSet_Size = 2;
static const int SdmD_ServiceFunctionSet_Size = 14;

class SdmString;
class SdmMessageBody;
class SdmMessage;
class SdmIntStr;
class SdmStrStr;

//
// we need these forward declarations for the
// typedefs below.
//
template<class T> class SdmVector;
template<class T> class SdmPtrVector;
template<class T> class SdmSortVector;
template<class T> class SdmPtrSortVector;

// --> Vector types
// There are two primary types of vectors; those that hold copies of objects
// themselves (SdmVector - object vector) and those that hold pointers to 
// objects (SdmPtrVector - object pointer vector).
//
// Since object vectors hold the objects themselves, when the vector is 
// destroyed, its contents (which includes the objects) is destroyed as
// well, so the destructor for each object in the vector is called. If
// the object must survive the destruction of the vector, then a copy of
// the object must be explicitly done.
//
// Object pointer vectors hold pointers to objects; when an object pointer
// vector is destroyed, the default behavior of the vector is to *not*
// call the destructor of the objects that each element in the vector
// points to. The vector must either have the SetPointerDeleteFlag()
// method called once (which changes the pointer vector behavior to
// cause all objects pointed to to be deleted when the vector is destroyed)
// or each pointed to object must be explicitly deleted before the vector
// itself is deleted.

  // These vectors hold copies of the objects themselves

typedef SdmVector<long int>	SdmLongL;	// vector of long integers
typedef unsigned long   SdmBitMask;
typedef SdmVector<SdmBitMask>	SdmBitMaskL;
typedef SdmVector<SdmBoolean>	SdmBooleanL;	// vector of booleans
typedef SdmVector<SdmString>	SdmStringL;	// vector of strings
typedef SdmVector<SdmIntStr>	SdmIntStrL;	// vector of integer/string pairs
typedef SdmSortVector<SdmIntStr> SdmSortIntStrL; // vector of sorted integer/string pairs
typedef SdmVector<SdmStrStr>	SdmStrStrL;	// vector of string/string pairs

  // These vectors hold pointers to vectors that hold copies of the objects themselves

typedef SdmPtrVector<SdmStringL *>	SdmStringLL;	// vector of vectors of strings
typedef SdmPtrVector<SdmIntStrL *>	SdmIntStrLL;	// vector of vectors of integer/string pairs
typedef SdmPtrVector<SdmStrStrL *>	SdmStrStrLL;	// vector of vectors of string/string pairs

  // These vectors hold pointers to objects

typedef SdmPtrVector<SdmMessageBody *>	SdmMessageBodyL;// vector of body parts
typedef SdmPtrVector<SdmMessage *>	SdmMessageL;	// vector of messages

// Other types
typedef long        SdmMessageNumber;
typedef SdmLongL    SdmMessageNumberL;

// Extern Declarations
extern SdmBoolean __LogOn;

// Message Attribute Abstract Flags
// These flag bits represent individual attributes associated with
// a component of a message

typedef SdmBitMask	SdmMessageAttributeAbstractFlag;	// a single message attribute abstract flag
typedef SdmBitMask	SdmMessageAttributeAbstractFlags;	// a message attribute abstract flag mask
typedef SdmBitMaskL	SdmMessageAttributeAbstractFlagsL;	// a list of message attribute abstract flag masks

static const SdmMessageAttributeAbstractFlag Sdm_MAA_charset = (1L<<0);
static const SdmMessageAttributeAbstractFlags Sdm_MAA_ALL =
(Sdm_MAA_charset);

// Message Flag Abstract Flags
// These flag bits represent individual flags associated with a message

typedef SdmBitMask	SdmMessageFlagAbstractFlag;	// a single abstract flag
typedef SdmBitMask	SdmMessageFlagAbstractFlags;	// an abstract flag mask
typedef SdmBitMaskL	SdmMessageFlagAbstractFlagsL;	// a list of abstract flag masks

static const SdmMessageFlagAbstractFlag Sdm_MFA_Answered = (1L<<0);
static const SdmMessageFlagAbstractFlag Sdm_MFA_Deleted = (1L<<1);
static const SdmMessageFlagAbstractFlag Sdm_MFA_Draft = (1L<<2);
static const SdmMessageFlagAbstractFlag Sdm_MFA_Flagged = (1L<<3);
static const SdmMessageFlagAbstractFlag Sdm_MFA_Recent = (1L<<4);
static const SdmMessageFlagAbstractFlag Sdm_MFA_Seen = (1L<<5);
static const SdmMessageFlagAbstractFlags Sdm_MFA_ALL = 
(Sdm_MFA_Answered|Sdm_MFA_Deleted|Sdm_MFA_Draft|Sdm_MFA_Flagged|Sdm_MFA_Recent|Sdm_MFA_Seen);

// Message Header Abstract Flags
// These flag bits represent individual headers associated with a message

typedef SdmBitMask	SdmMessageHeaderAbstractFlag;	// a single message header abstract flag
typedef SdmBitMask	SdmMessageHeaderAbstractFlags;	// a header abstract flag mask
typedef SdmBitMaskL	SdmMessageHeaderAbstractFlagsL;	// a list of abstract flag masks

  // These correspond to real headers

static const SdmMessageHeaderAbstractFlag Sdm_MHA_Bcc = (1L<<0);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Cc = (1L<<1);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Date = (1L<<2);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_From = (1L<<3);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Subject = (1L<<4);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_To = (1L<<5);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Apparently_To = (1L<<6);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Resent_To = (1L<<7);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Reply_To = (1L<<8);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Return_Path = (1L<<9);
static const SdmMessageHeaderAbstractFlag Sdm_MHA_Resent_From = (1L<<10);

  // These correspond to "psuedo headers" and may map onto multiple
  // different types of headers depending upon the underlying transport;
  // Those marked with 'SI' can possibly be retrieved more economically and
  // should be given preference when creating the "message scrolling list"

static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_Bcc = (1L<<16);		// PI: list of "bcc" addressees
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_Cc = (1L<<17);		// PI: list of "cc" addressees
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_MessageFrom = (1L<<18);	// SI: from (e.g. John Public <jpublic@host>)
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_MessageSize = (1L<<19);	// SI: size
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_MessageType = (1L<<20);	// SI: bodytype one of: TEXT TEXT-OTHER MULTIPART MESSAGE APPLICATION AUDIO IMAGE VIDEO
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_ReceivedDate = (1L<<21);	// SI: date this message received (as DDD mmm dd hh:mm:ss yyy e.g. Tue Aug 27 09:10:12 1996)
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_AbbreviatedReceivedDate = (1L<<22);	// SI: date this message received (as dd-mmm-yyy hh:mm:ss -oooo e.g. 17-Feb-1995 16:17:03 -0800)
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_Sender = (1L<<24);		// SI: sender of mail (e.g. jpublic@host)
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_SenderPersonalInfo = (1L<<25);	// SI: personal (e.g. John Public)
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_SentDate = (1L<<26);	// PI: date this message was sent
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_Subject = (1L<<27);		// SI: subject
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_To = (1L<<28);		// PI: list of "to" addressees
static const SdmMessageHeaderAbstractFlag Sdm_MHA_P_SenderUsername = (1L<<29);	// SI: sender's username, i.e. the "mailbox" component in "mailbox@hostname"

static const SdmMessageHeaderAbstractFlags Sdm_MHA_ALL =
(Sdm_MHA_Bcc|Sdm_MHA_Cc|Sdm_MHA_Date|Sdm_MHA_From|Sdm_MHA_Subject|
Sdm_MHA_To|Sdm_MHA_P_Bcc|Sdm_MHA_P_Cc|Sdm_MHA_P_MessageFrom|
Sdm_MHA_P_MessageSize|Sdm_MHA_P_MessageType|Sdm_MHA_P_ReceivedDate|
Sdm_MHA_P_Sender|Sdm_MHA_P_SenderPersonalInfo|Sdm_MHA_P_SentDate|
Sdm_MHA_P_Subject|Sdm_MHA_P_To|Sdm_MHA_P_SenderUsername);

// Namespace flags
// Any "namespace" type scan returns a list of IntStrs; the "GetNumber()" portion
// is a mask of one or more of the following flags indicating the nature of
// the name returned in the "GetString()" portion
typedef int SdmNamespaceFlag;

static const SdmNamespaceFlag Sdm_NSA_noInferiors = (1L<<0);	// component not a container; cannot scan below this name (e.g. not a directory)
static const SdmNamespaceFlag Sdm_NSA_cannotOpen = (1L<<1);	// component is not a message store; cannot be opened (e.g. not a mailbox file)
static const SdmNamespaceFlag Sdm_NSA_changed = (1L<<2);	// component is marked as changed in some way (e.g. new messages since last access)
static const SdmNamespaceFlag Sdm_NSA_unChanged = (1L<<3);	// component is not marked as changed in some way since last access

// SdmMsgStrType:
// describes the "type" of a component of a message
//
typedef enum SdmMsgStrType_T {
  Sdm_MSTYPE_none,		// this indicates the structure does not have a component
  Sdm_MSTYPE_text,
  Sdm_MSTYPE_multipart,
  Sdm_MSTYPE_message,
  Sdm_MSTYPE_application,
  Sdm_MSTYPE_audio,
  Sdm_MSTYPE_image,
  Sdm_MSTYPE_video,
  Sdm_MSTYPE_other
} SdmMsgStrType;

typedef enum SdmMsgStrEncoding_T {
  Sdm_MSENC_none,
  Sdm_MSENC_7bit,
  Sdm_MSENC_8bit,
  Sdm_MSENC_binary,
  Sdm_MSENC_base64,
  Sdm_MSENC_quoted_printable,
  Sdm_MSENC_other
} SdmMsgStrEncoding;

typedef enum SdmMsgStrDisposition_T {
  Sdm_MSDISP_not_specified,	// disposition of component not specified
  Sdm_MSDISP_inline,		// component should be handled "in line" if possible
  Sdm_MSDISP_attachment		// component should be handled as "attachment" 
} SdmMsgStrDisposition;

typedef enum SdmMsgFormat_T {
  Sdm_MSFMT_Mime,		// message format: MIME
  Sdm_MSFMT_SunV3		// message format: Sun V3 (mailtool)
} SdmMsgFormat;

typedef enum SdmSessionType_T {
  Sdm_ST_InSession=0,  // incoming session. 
  Sdm_ST_OutSession    // Outgoing session.
} SdmSessionType;


typedef enum SdmServiceFunctionType_T { 
  Sdm_SVF_Any,                           // catch-all service function type.
  Sdm_SVF_AutoSave,                      // polled event: auto save of message store.
  Sdm_SVF_CheckNewMail,                  // polled event:  check for new mail in message store
  Sdm_SVF_DataPortBusy,                  // dataport event:  data port busy indication.
  Sdm_SVF_DataPortLockActivity,		       // dataport event: lock activity notification
  Sdm_SVF_DebugLog,                      // dataport event: debug message from data port.
  Sdm_SVF_GroupPrivilegeAction,		 // connection service: group privilege action request.
  Sdm_SVF_ErrorLog,                      // dataport event:  error from data port.
  Sdm_SVF_NotifyLog,                     // dataport event:  notify from data port.
  Sdm_SVF_LastInteractiveEvent,          // connection service:  return last interactive time.
  Sdm_SVF_SolicitedEvent,                // user solicited event via async method.
  Sdm_SVF_SubmitOutgoingCachedMessage,   // outgoing store reconnect event.
  Sdm_SVF_Other,                         // internal to mid-end.  do not use directly.
  Sdm_SVF_Unknown                        // internal to mid-end.  do not use directly.
} SdmServiceFunctionType;                            


// Other useful includes
#include <SDtMail/Error.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/ClassIds.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/PtrVector.hh>
#include <SDtMail/SortVector.hh>
#include <SDtMail/PtrSortVector.hh>
#include <SDtMail/LinkedList.hh>
#include <SDtMail/DoubleLinkedList.hh>
#include <SDtMail/String.hh>
#include <SDtMail/SimpleTuples.hh>
#include <SDtMail/ContentBuffer.hh>

#endif
