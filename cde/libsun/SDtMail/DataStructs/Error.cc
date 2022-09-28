/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Error Class.

#pragma ident "@(#)Error.cc	1.87 97/06/12 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/MailRc.hh>
#include <nl_types.h>

// --------------------------------------------------------------------------------
// error class static internal utility functions
// --------------------------------------------------------------------------------

// This function takes an error code and returns a "strdup"ed copy of the error message
// string for that error message - used when the message catalog fails to provide the
// error message in question

// Note that all %s specifications are spelled out as %.2048s; This causes
// no more than 2048 characters *that may be complete multibyte
// characters* to be output for a string to be included in an error
// message - this can be up to 4 times the size, or 1K bytes. We allow
// for up to 4 of these strings to be incorporated into an error message.
// This keeps us from overflowing an error message buffer

// a+(b*4*4)+1 - a=max size of localized error message, b=max size of non-localized 
// text limit on messages inserted x 4 bytes max localized x 4 max strings inserted

#define ERRMSGLENGTH 2048	// all strings must 
#define ERRMSGBUFSIZE (2048+(ERRMSGLENGTH*4*4)+1)

static const char* ConvertErrorCodeToErrorMessage(SdmErrorCode errorCode)
{
  switch(errorCode) {
  case Sdm_EC_Success: 
    return (strdup((const char *) "Operation successful.\n"));
  case Sdm_EC_Fail:
    return (strdup((const char *) "A Mailer operation has failed. Retry.\n"));
  case Sdm_EC_Operation_Unavailable:
    return (strdup((const char *) "Internal Condition: The operation requested is not supported by the object being accessed.\n"));
  case Sdm_EC_Cancel:
    return (strdup((const char *) "The requested operation was cancelled.\n"));
  case Sdm_EC_HostnameRequired:
    return (strdup((const char *) "You did not specify a server name for this mailbox.\n"));
  case Sdm_EC_Shutdown:
    return (strdup((const char *) "Internal Condition: object is shutdown and cannot process the request.\n"));
  case Sdm_EC_Closed:
    return (strdup((const char *) "Mailer has closed this mailbox.\n"));
  case Sdm_EC_Attached:
    return (strdup((const char *) "Internal Condition: object is attached and cannot process the request.\n"));
  case Sdm_EC_Open:
    return (strdup((const char *) "Internal Condition: object is open and cannot process the request.\n"));
  case Sdm_EC_MessageStoreNameRequired:
    return (strdup((const char *) "Internal Condition: A mailbox name is required to access the requested object.\n"));
  case Sdm_EC_CannotAccessMessageStore:
    return (strdup((const char *) "Mailer cannot open this mailbox.\n"));
  case Sdm_EC_Readonly:
    return (strdup((const char *) "Internal Condition: object is opened read only and cannot process the request.\n"));
  case Sdm_EC_UsernameRequired:
    return (strdup((const char *) "You did not specify a login name to access this mailbox.\n"));
  case Sdm_EC_PasswordRequired:
    return (strdup((const char *) "You did not specify a password to access this mailbox.\n"));
  case Sdm_EC_MessageExpunged:
    return (strdup((const char *) "Internal Condition: request made on a message that is deleted and expunged.\n"));
  case Sdm_EC_MessageStructureDataUndefined:
    return (strdup((const char *) "Internal Condition: specified message is missing structure information.\n"));
  case Sdm_EC_ServiceFunctionAlreadyRegistered:
    return (strdup((const char *) "Internal Condition: Sdm_EC_ServiceFunctionAlreadyRegistered\n"));
  case Sdm_EC_ServiceFunctionNotRegistered:
    return (strdup((const char *) "Internal Condition: Sdm_EC_ServiceFunctionNotRegistered\n"));
  case Sdm_EC_ConnectionAlreadyStarted:
    return (strdup((const char *) "Internal Condition: Sdm_EC_ConnectionAlreadyStarted\n"));
  case Sdm_EC_ConnectionNotStarted:
    return (strdup((const char *) "Internal Condition: Sdm_EC_ConnectionNotStarted\n"));
  case Sdm_EC_SessionAlreadyStarted:
    return (strdup((const char *) "Internal Condition: Sdm_EC_SessionAlreadyStarted\n"));
  case Sdm_EC_SessionNotStarted:
    return (strdup((const char *) "Internal Condition: Sdm_EC_SessionNotStarted\n"));
  case Sdm_EC_SessionAlreadyAttached:
    return (strdup((const char *) "Internal Condition: Sdm_EC_SessionAlreadyAttached\n"));
  case Sdm_EC_SessionNotAttached:
    return (strdup((const char *) "Internal Condition: Sdm_EC_SessionNotAttached\n"));
  case Sdm_EC_ReturnArgumentListHasContents:
    return (strdup((const char *) "Internal Condition: the request for data provided a container object that was not empty.\n"));
  case Sdm_EC_RequestedDataNotFound:
    return (strdup((const char *) "Internal Condition: the requested data was not located.\n"));
  case Sdm_EC_NoMemory:
    return (strdup((const char *) "Internal Condition: memory allocation request failed.\n"));
  case Sdm_EC_MessageStoreAlreadyStarted:
    return (strdup((const char *) "Internal Condition: request issued to start a message store that is already started.\n"));
  case Sdm_EC_MessageStoreNotStarted:
    return (strdup((const char *) "Internal Condition: request issued on message store that has not yet been started.\n"));
  case Sdm_EC_LocalServerNotRegistered:
    return (strdup((const char *) "Internal Condition: no server process registered for specified object.\n"));
  case Sdm_EC_MessageIsNested:
    return (strdup((const char *) "Internal Condition: request to manipulate a message improperly used on a nested message.\n"));
  case Sdm_EC_ServiceTypeRequired:
    return (strdup((const char *) "Internal Condition: request to access or open a service is missing a required 'servicetype' option.\n"));
  case Sdm_EC_BadArgument:
    return (strdup((const char *) "Internal Condition: request contains an argument that specifies an invalid or illegal value.\n"));
  case Sdm_EC_BadMessageNumber:
    return (strdup((const char *) "Internal Condition: request to access an existing message that does not exist.\n"));
  case Sdm_EC_MessageNotFromOutgoingStore:
    return (strdup((const char *) "Internal Condition: request to send a message that does not exist in an outgoing message store.\n"));
  case Sdm_EC_BadMessageBodyPartNumber:
    return (strdup((const char *) "Internal Condition: request to create object for existing body part that does not exist.\n"));
  case Sdm_EC_NoAddressFoundInMessage:
    return (strdup((const char *) "You did not specify any recipients for this message.\nFill in the To, Cc, or Bcc field and send it again.\n"));
  case Sdm_EC_InsufficientAddresseeHeaders:
    return (strdup((const char *) "You cannot address an email message to Bcc recipients only.\nFill in the To or Cc field and send it again.\n"));
  case Sdm_EC_SubmitDeletedMessage:
    return (strdup((const char *) "Internal Condition: request to send a message that is deleted.\n"));
  case Sdm_EC_InvalidOutgoingStoreToken:
    return (strdup((const char *) "Internal Condition: token used in request to open outgoing store contains invalid conditions.\n"));
  case Sdm_EC_CommitDisconnectedMessage:
    return (strdup((const char *) "Internal Condition: message committed to outgoing store while disconnected state.\n"));
  case Sdm_EC_AlreadyConnected:
    return (strdup((const char *) "Internal Condition: request to reconnect to already connected service.\n"));
  case Sdm_EC_AlreadyDisconnected:
    return (strdup((const char *) "Internal Condition: request to disconnect already disconnected service.\n"));
  case Sdm_EC_InvalidServiceFunction:
    return (strdup((const char *) "Internal Condition: specified type of service function cannot be used where specified.\n"));
  case Sdm_EC_InactivityIntervalNotPassed:
    return (strdup((const char *) "Internal Condition: inactivity interval has not yet been reached.\n"));
  case Sdm_EC_RFC822AddressParseFailure:
    return (strdup((const char *) "An email address is invalid.\n"));
  case Sdm_EC_SearchStringParseFailure:
    return (strdup((const char *) "The search criteria are not valid.\n"));
  case Sdm_EC_CannotObtainMailboxLock:
    return (strdup((const char *) "Mailer cannot obtain a lock to open this mailbox.\n"));
  case Sdm_EC_ErrorProcessingMailrc:
    return (strdup((const char *) "An error occurred while Mailer was processing your mail setup.\n"));
  case Sdm_EC_ErrorUpdatingMessageStore:
    return (strdup((const char *) "An error occurred while Mailer was updating a mailbox (for example) after\nmoving messages between mailboxes or deleting messages.\n"));
  case Sdm_EC_IncompatibleMessageStoreTypes:
    return (strdup((const char *) "Internal Condition: The source and target message stores are of incompatible types.\n"));
  case Sdm_EC_BadMessageSequence:
    return (strdup((const char *) "Internal Condition: The message sequence is either empty or contains duplicates.\n"));
  case Sdm_EC_MessageRFC822MissingFromLine:
    return (strdup((const char *) "Internal Condition: The contents of a message/rfc822 message body is missing a \"From\" line from the beginning of the contents.\n"));
  case Sdm_EC_CannotAttachToService:
    return (strdup((const char *) "Mailer cannot access the specified service.\n"));
  case Sdm_EC_CannotDetermineCurrentWorkingDirectory:
    return (strdup((const char *) "Mailer cannot find the full path name of the current working directory.\n\n%.2048s\n"));
  case Sdm_EC_CannotAccessMailboxPathNameComponent:
    return (strdup((const char *) "Mailer cannot access this portion (%.2048s)\n of the path for your mailbox (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_NullsDetectedInMail:
    return (strdup((const char *) "Mailer found NULLS in new e-mail: cannot check for new mail.\n"));
  case Sdm_EC_CannotSendMessage:
    return (strdup((const char *) "Mailer cannot send the message because one of the addresses may\nbe incorrect. Make sure the addresses are valid and try again.\n"));
  case Sdm_EC_CannotConnectToSmtpMailServer:
    return (strdup((const char *) "Mailer cannot contact an SMTP mail server to send this message.\nEither you cannot send mail from your local host or\nyour SMTP mail server is not available.\n"));
  case Sdm_EC_DecodingError:
    return (strdup((const char *) "Mailer encountered an error while decoding the attachment data.\nTry asking the sender to resend the attachment to you.\n"));
  case Sdm_EC_CclientError:
    return (strdup((const char *) "Mailer has encountered an error.\n"));
  case Sdm_EC_CannotAccessNewMail:
    return (strdup((const char *) "Mailer cannot check for new email.\n"));
  case Sdm_EC_CannotConnectToIMAPServer:
    return (strdup((const char *) "Mailer cannot connect to the IMAP server because \nthat server is not running on your host (%.2048s).\n"));
  case Sdm_EC_CannotAppendMessageToMailbox:
    return (strdup((const char *) "Mailer cannot append the message to the mailbox.\n"));
  case Sdm_EC_CannotCopyOrMoveMessage:
    return (strdup((const char *) "Mailer cannot copy or move the message to your mailbox.\n"));
  case Sdm_EC_CannotDeleteMailbox:
    return (strdup((const char *) "Mailer cannot delete your mailbox.\n"));
  case Sdm_EC_CannotRenameMailbox:
    return (strdup((const char *) "Mailer cannot rename your mailbox.\n"));
  case Sdm_EC_CannotCreateMailbox:
    return (strdup((const char *) "Mailer cannot create your mailbox.\n"));
  case Sdm_EC_SmtpConnectionTimeout:
    return (strdup((const char *) "Mailer could not send your message because\nyour SMTP mail server is not responding.\nTry resending this message after a short wait.\nContact your system administrator for assistance if necessary.\n"));
  case Sdm_EC_SmtpServerErrorMessage:
    return (strdup((const char *) "Error messages from your SMTP mail server:\n%.2048s\n"));
  case Sdm_EC_CCL_ServerErrorMessage:
    return (strdup((const char *) "Error messages from your mail server:\n%.2048s\n"));
  case Sdm_EC_CCL_CantCreateSessionLock:
    return (strdup((const char *) "Mailer cannot create a session lock and will open the mailbox read-only.\n"));
  case Sdm_EC_CCL_WaitingForLock:
    return (strdup((const char *) "Mailer is awaiting the mailbox lock and will attempt to open the mailbox.\n"));
  case Sdm_EC_CCL_TryingForLock:
    return (strdup((const char *) "Mailer is attempting to obtain the mailbox lock.\n"));
  case Sdm_EC_CCL_IllegalFromLine:
    return (strdup((const char *) "The mailbox contains a message with an invalid \"From \" line format.\n"));
  case Sdm_EC_CCL_IllegalMailbox:
    return (strdup((const char *) "The mailbox format is invalid.  Try restarting Mailer.\n"));
  case Sdm_EC_CCL_MailboxShrank:
    return (strdup((const char *) "The mailbox size has shrunk due to possible data corruption.  Try restarting Mailer.\n"));
  case Sdm_EC_CCL_LockFailure:
    return (strdup((const char *) "Mailer cannot lock the mailbox.  Try restarting Mailer.\n"));
  case Sdm_EC_CCL_MailboxContainsNulls:
    return (strdup((const char *) "Mailer cannot check for new mail: it contains NULL strings.\n"));
  case Sdm_EC_CCL_MailboxInUse:
    return (strdup((const char *) "The mailbox is already in use.\n"));
  case Sdm_EC_CCL_CantSetGid:
    return (strdup((const char *) "Mailer cannot set the group id for the mailbox.\n"));
  case Sdm_EC_CCL_MailboxReadError:
    return (strdup((const char *) "Mailer cannot read the mailbox.  Restart.\n"));
  case Sdm_EC_CCL_WaitingForDestinationLock:
    return (strdup((const char *) "Mailer is awaiting the destination mailbox lock.\n"));
  case Sdm_EC_CCL_CantLoginToIMAPServer:
    return (strdup((const char *) "Mailer cannot log on to the IMAP server.  Verify that the\nserver name, login, and password are correct.\n"));
  case Sdm_EC_CCL_FileNotInMailboxFormat:
    return (strdup((const char *) "The file you specified is not in a mailbox format that can be\nrecognized by Mailer.\n"));
  case Sdm_EC_CCL_LocalErrorMessage:
    return (strdup((const char *) "Error messages from your mailbox:\n%.2048s\n"));
  case Sdm_EC_CCL_MailboxNoLongerExists:
    return (strdup((const char *) "Mailer can no longer access your mailbox; it may have been moved,\ndeleted, or replaced. Close and reopen your mailbox.\n"));
  case Sdm_EC_MBL_CannotCreateMailboxLockFile:
    return (strdup((const char *) "Mailer cannot create a lock file (%.2048s) for this mailbox.\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotCreateMailboxLockFile_IsDirectory:
    return (strdup((const char *) "Mailer cannot create a lock file for this mailbox because the lock file\nname (%.2048s) is the same as an existing folder name. %.2048s\n"));
  case Sdm_EC_MBL_CannotCreateMailboxLockFile_NoPermission:
    return (strdup((const char *) "Mailer cannot create a lock file (%.2048s) for this mailbox because you do not have\npermissions to access either the mailbox or the folder that contains the mailbox.\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotCreateMailboxLockFile_NoSuchFile:
    return (strdup((const char *) "Mailer cannot create a lock file (%.2048s) for this mailbox because part of the\npath name for the lock file is not a directory.\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotCreateMailboxLockFile_RemoteAccessLost:
    return (strdup((const char *) "Mailer cannot create a lock file (%.2048s) for this mailbox because it cannot\naccess the remote host on which the file is located.\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotRemoveMailboxLockFile:
    return (strdup((const char *) "Mailer cannot remove or release the mailbox lock (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotRemoveStaleMailboxLockFile:
    return (strdup((const char *) "Mailer cannot remove an obsolete mailbox lock (%.2048s) to open this mailbox.\nBecome root and remove the obsolete lock file.  Contact your system\nadministrator for assistance if necessary.\n\n%.2048s\n"));
  case Sdm_EC_MBL_CannotStatMailboxLockFile:
    return (strdup((const char *) "Mailer cannot obtain information on the mailbox lock (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_MBL_TooltalkLockError:
    return (strdup((const char *) "A ToolTalk error occurred while Mailer was performing a\nToolTalk task (%.2048s) on the mailbox lock.\n\n%.2048s\n"));
  case Sdm_EC_MBL_TooltalkLockFileJoinError:
    return (strdup((const char *) "A ToolTalk error occurred while Mailer was performing a\nToolTalk task on the mailbox lock (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_MBL_TooltalkNotResponding:
    return (strdup((const char *) "Mailer cannot obtain exclusive access to this mailbox because the\nhost on which it is located is not responding.\n"));
  case Sdm_EC_MBL_OtherMailerOwnsWriteAccess:
    return (strdup((const char *) "Mailer cannot obtain exclusive access to this mailbox (%.2048s)\nbecause someone else is using it.\n"));
  case Sdm_EC_MBL_TooltalkSessionStartupError:
    return (strdup((const char *) "Mailer cannot start ToolTalk to create a mailbox lock.\n\n%.2048s\n"));
  case Sdm_EC_MBL_ThisMailerOwnsWriteAccess:
    return (strdup((const char *) "Mailer cannot obtain exclusive access to this mailbox (%.2048s)\nbecause it is already open.\n"));
  case Sdm_EC_MRC_ResourceParsingNoEndif:
    return (strdup((const char *) "An error exists in your mail setup file (%.2048s).  A conditional (if)\nstatement does not have a corresponding endif statement.\n"));
  case Sdm_EC_MRC_CannotOpenMailrcForUpdating:
    return (strdup((const char *) "Mailer cannot open the file for your mail setup (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_MRC_CannotOpenMailruleForUpdating:
    return (strdup((const char *) "Cannot open mail rule file for updating.\nMail rule file: %.2048s\n\n%.2048s\n"));
  case Sdm_EC_MRC_CannotRenameNewMailrcFile:
    return (strdup((const char *) "Mailer cannot rename your mail setup file (%.2048s)\nwith the new one (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_MRC_CannotRenameNewMailruleFile:
    return (strdup((const char *) "Cannot rename new mail rule file over existing mail rule file.\nNew Mailrc file: %.2048s\nExisting Mailrc file: %.2048s\n\n%.2048s\n"));
  case Sdm_EC_PRS_EmptyFilter:
    return (strdup((const char *) "You have not typed any search criteria: To, From, and so on.\n"));
  case Sdm_EC_PRS_IllegalSearchToken:
    return (strdup((const char *) "Your search criteria contain an invalid keyword or operator.\n"));
  case Sdm_EC_PRS_MissingField:
    return (strdup((const char *) "Your search criteria are missing a description after a keyword, such\nas SUBJECT.  Type a description after the keyword.\n"));
  case Sdm_EC_PRS_MissingDoubleQuote:
    return (strdup((const char *) "Your search criteria do not contain an end quote (\") after an open quote.\n"));
  case Sdm_EC_PRS_ExpectedBoolean:
    return (strdup((const char *) "You did not specify any operator in your search criteria.\nChoose one from the Operators menu.\n"));
  case Sdm_EC_PRS_MissingPredicate:
    return (strdup((const char *) "Your search criteria contain an operator keyword for which the criterion\nis missing.  For example, if you type: SUBJECT foo AND, add a criterion\nafter AND or delete AND.\n"));
  case Sdm_EC_PRS_ParenthesisBad:
    return (strdup((const char *) "Your search criteria contain open parenthesis symbols that do not\nhave corresponding end parenthesis symbols.\n"));
  case Sdm_EC_PRS_IllegalLeftParenthesisField:
    return (strdup((const char *) "Your search criteria contain an invalid open parenthesis symbol\nin a text field.  The correct format is: (SUBJECT a or SUBJECT b).\n"));
  case Sdm_EC_PRS_IllegalRightParenthesisToken:
    return (strdup((const char *) "Your search criteria contain an end parenthesis symbol after a\nkeyword, such as SUBJECT FROM.  Type a description after the keyword.\n"));
  case Sdm_EC_PRS_IllegalRightParenthesisTermination:
    return (strdup((const char *) "Your search criteria contain an end parenthesis symbol followed\nby an invalid character.  Type a space or tab after the ) symbol.\n"));
  case Sdm_EC_PRS_IllegalLeftParenthesisKeyField:
    return (strdup((const char *) "Your search criteria contain an invalid open parenthesis symbol after a\nheader keyword, such as SUBJECT.  The correct format is SUBJECT a, *not* SUBJECT (a).\n"));
  case Sdm_EC_PRS_IllegalRightParenthesisKeyField:
    return (strdup((const char *) "Your search criteria contain an invalid end parenthesis symbol after a\nheader keyword, such as SUBJECT.  The correct format is: SUBJECT a, *not* SUBJECT )a).\n"));
  case Sdm_EC_PRS_IllegalNumericField:
    return (strdup((const char *) "Your search criteria contain an alpha character in a text field\nwhere only numeric characters are allowed.\n"));
  case Sdm_EC_PRS_IllegalKeyField:
    return (strdup((const char *) "Your search criteria contain an invalid space or tab in a text field.\n"));
  case Sdm_EC_PRS_IllegalDateField:
    return (strdup((const char *) "Your search criteria contain dates that are in an incorrect format.\nThe correct format is dd-mmm-yyy.\n"));
  case Sdm_EC_PRS_IllegalNotBeforeField:
    return (strdup((const char *) "The syntax for your search criterion is incorrect: Place the NOT (~) operator before the keyword, for example, ~ from foobar.\n"));
  case Sdm_EC_PRH_MISSINGQQ:
    return (strdup((const char *) "Your alias or email address (%.2048s) is missing an end quote (\")\nafter an open quote.\n"));
  case Sdm_EC_PRH_MISSINGRB:
    return (strdup((const char *) "Your alias or email address (%.2048s) is missing a ] symbol\nafter an initial [ symbol.\n"));
  case Sdm_EC_PRH_MISSINGHT:
    return (strdup((const char *) "Your alias or email address (%.2048s) does not contain a host name\nafter the @ symbol.\n"));
  case Sdm_EC_PRH_MISSINGRP:
    return (strdup((const char *) "Your alias or email address (%.2048s) is missing a ) symbol\nafter an initial ( symbol.\n"));
  case Sdm_EC_PRH_MISSINGRAB:
    return (strdup((const char *) "Your alias or email address (%.2048s) is missing a > symbol\nafter an initial < symbol.\n"));
  case Sdm_EC_PRH_MISSINGEC:
    return (strdup((const char *) "Your alias or email address (%.2048s) ends with a backslash (\\).  Backslashes\nmust be followed by another character.\n"));
  case Sdm_EC_PRH_EXTRARAB:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains an extra > symbol.\n"));
  case Sdm_EC_PRH_MISSINGLAB:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains a > symbol without the <\nsymbol.  A < symbol must precede the > symbol.\n"));
  case Sdm_EC_PRH_MISSINGLP:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains a ) symbol without the (\nsymbol.  A ( symbol must precede the ) symbol.\n"));
  case Sdm_EC_PRH_MISSINGLB:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains a ] symbol without the [\nsymbol.  A [ symbol must precede the ] symbol.\n"));
  case Sdm_EC_PRH_MISSINGGEND:
    return (strdup((const char *) "Your alias or email address (%.2048s) does not end with a semicolon (;).\nAlways put a semi-colon at the end, as in:\nalias-group-name: address, address;\n"));
  case Sdm_EC_PRH_BADROUTESPEC:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains a colon (:), which is an invalid character.\n"));
  case Sdm_EC_PRH_MISSINGFLD:
    return (strdup((const char *) "Your alias or email address (%.2048s) does not contain any addresses.  The\ncorrect format is: alias-group-name: address, address.\n"));
  case Sdm_EC_PRH_MISSINGDOMAIN:
    return (strdup((const char *) "Your alias or email address (%.2048s) is missing a domain name.  For\nexample, in the address mary.smith@eng.sun.com, eng.sun.com is the domain name.\n"));
  case Sdm_EC_PRH_SPURIOUSSC:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains an invalid character,\nsuch as a semicolon or comma in the middle of an incomplete alias or email address,\nor a space after the end of a complete alias or email address.\n"));
  case Sdm_EC_PRH_NULLADDRESS:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains extra commas.\nUse only one comma to separate addresses; do not end an alias or\nemail address with a comma.\n"));
  case Sdm_EC_PRH_MISSINGADDRESS:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains extra commas.\n"));
  case Sdm_EC_PRH_ILLEGALPHRASE:
    return (strdup((const char *) "The personal name placed before the address (%.2048s) is incorrect,\nas in: \"John Smith\" <john.smith@sun.com>.  Be sure to separate\nmultiple addresses or aliases with commas or commas and spaces.\n"));
  case Sdm_EC_PRH_EXTRAATSIGN:
    return (strdup((const char *) "Your alias or email address (%.2048s) contains too many @ symbols.\nUse only one @ symbol.\n"));
  case Sdm_EC_OBJ_UserNameNotFound:
    return (strdup((const char *) "The user name (%.2048s) in the path name for this mailbox (%.2048s) does not exist.\n"));
  case Sdm_EC_OBJ_HomeDirPathMissingMessageStoreName:
    return (strdup((const char *) "You must specify a mailbox name (%.2048s) after the tilde symbol (~)\nthat denotes your home directory in the path name.  The correct format\nis: ~/mailbox-name or ~user-name/mailbox-name.\n"));
  case Sdm_EC_OBJ_ErrorUpdatingMessageStore:
    return (strdup((const char *) "Mailer cannot update the mailbox (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_OBJ_CannotAccessMessageStore:
    return (strdup((const char *) "Mailer cannot open the mailbox (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_OBJ_ErrorRemovingMessageStore:
    return (strdup((const char *) "Mailer cannot remove this mailbox (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_OBJ_CannotStatMessageStore:
    return (strdup((const char *) "Mailer cannot obtain information on this mailbox (%.2048s).\n\n%.2048s\n"));
  case Sdm_EC_NET_BadPortNumberSpecified:
    return (strdup((const char *) "The service port number after the host name must be numeric characters,\nfor example, host-name:500.\n"));
  case Sdm_EC_NET_HostNameSyntaxError:
    return (strdup((const char *) "The internet address that you specified for the host name is invalid.\n"));
  case Sdm_EC_NET_HostDoesNotExist:
    return (strdup((const char *) "The host that you specified does not exist.\n"));
  case Sdm_EC_NET_CannotCreateConnection:
    return (strdup((const char *) "Mailer cannot connect to the network.\nContact your system administrator for assistance.\n"));
  case Sdm_EC_NET_ServiceNotAvailable:
    return (strdup((const char *) "The server you specified does not provide the service required, for example,\nIMAP or SMTP services.  Contact your system administrator for assistance.\n"));
  default: 
    char errmsgbuf[ERRMSGLENGTH];
    memset(&errmsgbuf, 0, ERRMSGLENGTH);
    sprintf(errmsgbuf, "Unknown error code %d\n", errorCode);
    return (strdup(errmsgbuf));
  }
}

// --------------------------------------------------------------------------------
// SdmInternalErrorObject class implementation
// --------------------------------------------------------------------------------

// An internal error object is a containter for a single error code 
// An error object might have multiple internal error objects depending on
// how many minor codes have been recorded
//
class SdmInternalErrorObject
{
private:
  friend class SdmError;

  // constructors and destructor

  SdmInternalErrorObject(SdmErrorCode errorCode) 
    : _errorCode(errorCode), _errorMessage(0), _helpIndex(0), _errorMessageCached(Sdm_False) {};
  SdmInternalErrorObject(SdmInternalErrorObject* errorObject);
  SdmInternalErrorObject(SdmInternalErrorObject& errorObject) { assert(0); abort(); }
  virtual ~SdmInternalErrorObject();

  // operators

  SdmInternalErrorObject& operator= (const SdmErrorCode val);
  operator SdmErrorCode() const { return (_errorCode); }

  // methods

  const char* ErrorMessage();
  void SetErrorMessage(const char* errorMessageText);

protected:
  void			_Reset();
  const char*		_FetchErrorMessage(SdmErrorCode errorCode);
  SdmErrorCode		_errorCode;
  SdmBoolean		_errorMessageCached;
  const char*		_errorMessage;
  const char*		_helpIndex;
};

// This is the "set number" index into the message catalog where the messages for the
// SdmError object are held

static const int LIBSDTMAIL_ERRORSET = 1;

// Fetch the error message for a given error code
// An error message is guaranteed returned, and it is placed in allocated space, so
// it must be free()ed when it is no longer needed

const char* SdmInternalErrorObject::_FetchErrorMessage(SdmErrorCode errorCode)
{
  const char *the_message = NULL;

  // First lookup in the message catalog; if it is not found, then produce from
  // internal cache of error messages

  the_message = SdmUtility::MessageCatalogLookup(LIBSDTMAIL_ERRORSET, errorCode, NULL);

  if (the_message == NULL)
    the_message = ConvertErrorCodeToErrorMessage(errorCode);
  else
    the_message = strdup(the_message);

  assert(the_message);
  return (the_message);
}

// destructor

SdmInternalErrorObject::~SdmInternalErrorObject()
{
  _Reset();
}

// Assignment operator: handling assigning an error value to this container
//

SdmInternalErrorObject& SdmInternalErrorObject::operator= (const SdmErrorCode errorCode)
{
  // Reset this container (releasing all contained errors)

  _Reset();

  // Now in the reset state (Sdm_EC_Success), cache new error code

  _errorCode = errorCode;

  return *this;
}

// reference copy constructor

SdmInternalErrorObject::SdmInternalErrorObject(SdmInternalErrorObject* errorObject)
{
  assert((errorObject->_errorMessageCached && errorObject->_errorMessage)
	 || (!errorObject->_errorMessageCached && !errorObject->_errorMessage));

  _errorCode = errorObject->_errorCode;
  _errorMessageCached = errorObject->_errorMessageCached;
  _errorMessage = (_errorMessageCached ? strdup(errorObject->_errorMessage) : 0);
  _helpIndex = (errorObject->_helpIndex ? strdup(errorObject->_helpIndex) : 0);
}

// This method returns the error message for the contained error

const char* SdmInternalErrorObject::ErrorMessage()
{
  // if the message has not yet been cached, cause it to be done

  if (!_errorMessageCached) {
    assert(!_errorMessage);
    _errorMessage = _FetchErrorMessage(_errorCode);
    _errorMessageCached = Sdm_True;
  }

  // return the current cached error message 

  assert(_errorMessageCached);
  assert(_errorMessage);
  return (_errorMessage);
}

// reset contents of this container

void SdmInternalErrorObject::_Reset() 
{
  assert((_errorMessageCached && _errorMessage) || (!_errorMessageCached && !_errorMessage));

  if (_errorMessageCached) {
    _errorMessageCached = Sdm_False; 
    free((void*)_errorMessage);
    _errorMessage = 0;
  }
  if (_helpIndex) {
    free((void*)_helpIndex);
    _helpIndex = 0;
  }
    
  assert(!_errorMessage);
  assert(!_errorMessageCached);
  _errorCode = Sdm_EC_Success;
}

// Causes the error message for the container error to be fetched and then run
// through vsprintf so that printf-style substitition can be done

void SdmInternalErrorObject::SetErrorMessage(const char* errorMessageText)
{
  assert(errorMessageText);
  assert(*errorMessageText);

  if (_errorMessageCached)
    free((void *)_errorMessage);
  else
    _errorMessageCached = Sdm_True;

  _errorMessage = strdup(errorMessageText);
}

// --------------------------------------------------------------------------------
// SdmError class implementation
// --------------------------------------------------------------------------------

// SdmError constructor
//
SdmError::SdmError() :
  _majorErrorCode(Sdm_EC_Success), 
  _minorErrorStackSize(0),
  _majorErrorObject(0),
  _minorErrorObjectStack(0)
{ 
}

SdmError::SdmError(const SdmError& error) :
  _majorErrorCode(Sdm_EC_Success), 
  _minorErrorStackSize(0),
  _majorErrorObject(0),
  _minorErrorObjectStack(0)
{ 
  // use operator= to handle copy of the object data.
  *this = error;
}

static SdmBoolean _ErrorLoggingOneTimeFlag = Sdm_False;
static SdmBoolean _ErrorLoggingFlag = Sdm_False;

SdmError::~SdmError()
{
  // If this is the first time destroying an error object, set the _ErrorLoggingFlag
  // from the users ~/.mailrc file - optimized to keep performance hit to a minimum
  // as the flag is never changed while the mail user agent is in operation

  if (!_ErrorLoggingOneTimeFlag) {
    _ErrorLoggingOneTimeFlag = Sdm_True;
    SdmMailRc *mailrc = SdmMailRc::GetMailRc();
    assert(mailrc);
    _ErrorLoggingFlag = mailrc->IsValueDefined("errorlogging");
  }

  // If the error logging flag is set, and the error container has something other
  // than "fail" or "success" or a limited number of other errors that are 
  // uninteresting, emit the contents to the dt error log

  if (_ErrorLoggingFlag) {
    switch (_majorErrorCode) {
      // These errors go unlogged
    case Sdm_EC_Success:
    case Sdm_EC_Fail:
    case Sdm_EC_RequestedDataNotFound:
    case Sdm_EC_InactivityIntervalNotPassed:
      break;
      // All other errors get logged
    default:
      SdmUtility::LogError(Sdm_False, "major code %u: %s\n", (SdmErrorCode) *this, 
			 (const char *) this->ErrorMessage());
      int i = this->MinorCount();
      while (i--) {
	SdmUtility::LogError(Sdm_False, "minor code %u: %s\n", (SdmErrorCode) (*this)[i], 
			     (const char *)this->ErrorMessage(i));
      }
    }
  }

  _Reset();
}

// Assignment operator: handling assigning a major error value to this error container
//

SdmError& SdmError::operator= (const SdmErrorCode errorCode)
{
  // If holding any errors, reset this container (releasing all contained errors)

  if (_majorErrorCode != Sdm_EC_Success)  
    _Reset();

  // Now in the reset state (Sdm_EC_Success), if setting a major error code,
  // cause it to be set

  if (errorCode != Sdm_EC_Success) {
    _majorErrorObject = new SdmInternalErrorObject(errorCode);
    _majorErrorCode = errorCode;
  }

  return *this;
}

// Assignment operator: handle assigning an existing error container to this error container
//

SdmError& SdmError::operator= (const SdmError& rhs)
{
  if (&rhs != this) {
    // First off, reset this error container and release all contained errors

    _Reset();

    // Now must copy the state of the given error container to this error container
    // Copy major error container first, the copy all minor error containers

    _majorErrorCode = rhs._majorErrorCode;
    if (_majorErrorCode != Sdm_EC_Success)
      _majorErrorObject = new SdmInternalErrorObject(rhs._majorErrorObject);

    for (int i = 0; i < rhs._minorErrorStackSize; i++)
      _PushMinorErrorCode(new SdmInternalErrorObject(rhs._minorErrorObjectStack[i]));
  }
  return *this;
}

// IsErrorContained: see if error code is set anywhere in the container
//

SdmBoolean SdmError::IsErrorContained(const SdmErrorCode ec)
{
  if (_majorErrorCode == ec)
    return (Sdm_True);
  for (int i = 0; i < _minorErrorStackSize; i++)
    if (*_minorErrorObjectStack[i] == ec)
      return (Sdm_True);
  return (Sdm_False);
}


// Index operator: fetch minor code
//

SdmErrorCode SdmError::operator[] (const int minorIndex)
{
  SdmErrorCode the_code;

  the_code = (minorIndex < _minorErrorStackSize)  ? *_minorErrorObjectStack[minorIndex] : Sdm_EC_Fail;
  return (the_code);
}

// Indexed error code method: fetch minor code
//

SdmErrorCode SdmError::ErrorCode (const int minorIndex)
{
  SdmErrorCode the_code;

  the_code = (minorIndex < _minorErrorStackSize)  ? *_minorErrorObjectStack[minorIndex] : Sdm_EC_Fail;
  return (the_code);
}

// MinorCount method: fetch the number of minor error codes contained 
//

int SdmError::MinorCount()
{
  return (_majorErrorCode == Sdm_EC_Success ? 0 : _minorErrorStackSize);
}

// Reset method: reset the contents of the error container
//

void SdmError::_Reset()
{
  // Clear out any minor errors contained

  if (_minorErrorStackSize) {
    // Delete all minor objects listed in the stack,
    // then delete the stack itself and reset the stack size

    for (int i = 0; i < _minorErrorStackSize; i++)
      delete _minorErrorObjectStack[i];
    free(_minorErrorObjectStack);
    _minorErrorStackSize = 0;
  }
  assert(!_minorErrorStackSize);

  // Clear out any major error contained

  if (_majorErrorCode != Sdm_EC_Success) {
    if (_majorErrorObject) {
      delete _majorErrorObject;
      _majorErrorObject = NULL;
    }
    _majorErrorCode = Sdm_EC_Success;
  }
  assert(_majorErrorCode == Sdm_EC_Success);
}

// ErrorMessage method: return the error message text for the current major error contained
//

const char *SdmError::ErrorMessage()
{
const char *the_message = NULL;

  if (_majorErrorCode == Sdm_EC_Success) {
    // Request of the error message when no error is present - special case
    // because there is no internal error object so we need to fake it here

    the_message = (const char *) "Operation successful.\n";
  }
  else
    the_message = _majorErrorObject->ErrorMessage();

  assert(the_message);
  return (the_message);
}

// ErrorMessage(minorIndex) method: return the error message text for a specified minor error
//

const char* SdmError::ErrorMessage(const int minorIndex)
{
  const char *the_message = NULL;

  if (minorIndex < _minorErrorStackSize)
    the_message = (_minorErrorObjectStack[minorIndex])->ErrorMessage();
  else
    the_message = (const char *) "Invalid index into minor error stack";
  assert(the_message);
  return (the_message);
}

// return all major and minor errors in one string
//

void SdmError::ErrorMessageMajorAndMinor(SdmString& r_minorMessages)
{
  // Reset the string container to the primary error message text 

  r_minorMessages = this->ErrorMessage();

  // Now for each contained minor error, append the text for that minor error

  int i = this->MinorCount();
  if (i) {
    while (i--) {
      r_minorMessages += "\n";	// Separate major and minor and all minors in list by a blank line
      r_minorMessages += this->ErrorMessage(i);
    }
  }
  return;
}

// AddMinorErrorCode method: add a new minor error to the container

void SdmError::AddMinorErrorCode(SdmErrorCode errorCode)
{
  // If the error container does not have a contained error, set it to fail

  if (_majorErrorCode == Sdm_EC_Success)
    *this = Sdm_EC_Fail;

  // Prevent duplicate minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // Add a new minor error code to the minor error object stack

    _PushMinorErrorCode(new SdmInternalErrorObject(errorCode));
  }
}

// AddMinorErrorCodeAndMessage method: add a new minor error to the container

void SdmError::AddMinorErrorCodeAndMessage(SdmErrorCode errorCode, ...)
{
  // If the error container does not have a contained error, set it to fail

  if (_majorErrorCode == Sdm_EC_Success)
    *this = Sdm_EC_Fail;

  // Prevent duplicate minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // Add a new minor error code to the minor error object stack

    SdmInternalErrorObject *eo = new SdmInternalErrorObject(errorCode);

    char message_buffer[ERRMSGBUFSIZE];
    memset(&message_buffer, 0, ERRMSGBUFSIZE);

    va_list var_args;
    va_start(var_args, errorCode);

    message_buffer[0] = '\0';

    vsprintf(message_buffer, eo->ErrorMessage(), var_args);
    assert(strlen(message_buffer));
    assert(strlen(message_buffer)<sizeof(message_buffer));

    eo->SetErrorMessage(message_buffer);
    va_end(var_args);

    _PushMinorErrorCode(eo);
  }
}

// AddMinorErrorCodeAndMessage method: add a new minor error to the container

void SdmError::AddMinorErrorCodeAndMessageWithFormat(SdmErrorCode errorCode, const char* format, ...)
{
  // If the error container does not have a contained error, set it to fail

  if (_majorErrorCode == Sdm_EC_Success)
    *this = Sdm_EC_Fail;

  // Prevent duplicate minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // Add a new minor error code to the minor error object stack

    SdmInternalErrorObject *eo = new SdmInternalErrorObject(errorCode);

    char message_buffer[ERRMSGBUFSIZE];
    memset(&message_buffer, 0, ERRMSGBUFSIZE);

    va_list var_args;
    va_start(var_args, errorCode);

    message_buffer[0] = '\0';

    vsprintf(message_buffer, format, var_args);
    assert(strlen(message_buffer));
    assert(strlen(message_buffer)<sizeof(message_buffer));

    eo->SetErrorMessage(message_buffer);
    va_end(var_args);

    _PushMinorErrorCode(eo);
  }
}

// AddErrorCode method: add a new major error code, pushing down any existing
// major error code to the list of minor errors

void SdmError::AddMajorErrorCode(SdmErrorCode errorCode)
{
  assert(errorCode != Sdm_EC_Success);

  // Prevent duplicate major/minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // If there is already a major error code set, push it down to the minor error stack

    if (_majorErrorCode != Sdm_EC_Success) {
      _PushMinorErrorCode(_majorErrorObject);
      _majorErrorObject = NULL;
    }

    // Create a new internal error object for the new major error code

    _majorErrorObject = new SdmInternalErrorObject(errorCode);
    _majorErrorCode = errorCode;
  }
}

// AddMajorErrorCodeAndMessage: add a new major error code and format the message,
// pushing down any existing major error code to the list of minor errors

void SdmError::AddMajorErrorCodeAndMessage(SdmErrorCode errorCode, ...)
{
  // Prevent duplicate major/minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // Add this major error code and cause any others to be pushed down

    AddMajorErrorCode(errorCode);

    // If the code is not success, then need to cause the error message to be
    // fetched and then run through vsprintf with the given variable arguments

    if (errorCode != Sdm_EC_Success) {
      char message_buffer[ERRMSGBUFSIZE];

      va_list var_args;
      va_start(var_args, errorCode);

      message_buffer[0] = '\0';

      vsprintf(message_buffer, _majorErrorObject->ErrorMessage(), var_args);
      assert(strlen(message_buffer));
      assert(strlen(message_buffer)<sizeof(message_buffer));

      _majorErrorObject->SetErrorMessage(message_buffer);
      va_end(var_args);
    }
  }
}

// AddMajorErrorCodeAndMessageWithFormat: add a new major error code with formatted message,
// pushing down any existing major error code to the list of minor errors

void SdmError::AddMajorErrorCodeAndMessageWithFormat(SdmErrorCode errorCode, const char* format, ...)
{
  // Prevent duplicate major/minor error codes from being added

  if (!IsErrorContained(errorCode)) {

    // Add this major error code and cause any others to be pushed down

    AddMajorErrorCode(errorCode);

    // If the code is not success, then need to cause the error message to be
    // fetched and then run through vsprintf with the given variable arguments

    if (errorCode != Sdm_EC_Success) {
      char message_buffer[ERRMSGBUFSIZE];

      va_list var_args;
      va_start(var_args, errorCode);

      message_buffer[0] = '\0';

      vsprintf(message_buffer, format, var_args);
      assert(strlen(message_buffer));
      assert(strlen(message_buffer)<sizeof(message_buffer));

      _majorErrorObject->SetErrorMessage(message_buffer);
      va_end(var_args);
    }
  }
}

void SdmError::SetMajorErrorCodeAndMessage(SdmErrorCode errorCode, ...)
{
  // Set the container major error code to the error code provided

  *this = errorCode;

  // If the code is not success, then need to cause the error message to be
  // fetched and then run through vsprintf with the given variable arguments

  if (_majorErrorCode != Sdm_EC_Success) {
    char message_buffer[ERRMSGBUFSIZE];
    memset(&message_buffer, 0, ERRMSGBUFSIZE);

    va_list var_args;
    va_start(var_args, errorCode);

    message_buffer[0] = '\0';

    vsprintf(message_buffer, _majorErrorObject->ErrorMessage(), var_args);
    assert(strlen(message_buffer));
    assert(strlen(message_buffer)<sizeof(message_buffer));

    _majorErrorObject->SetErrorMessage(message_buffer);
    va_end(var_args);
  }
}

// _PushMinorErrorCode internal method: given an internal error object, cause
// that object to be pushed onto the minor error code stack, properly expanding
// the stack and stack size

void SdmError::_PushMinorErrorCode(SdmInternalErrorObject* meo)
{  
  assert(meo);

  // Now expand the minor error stack by one entry and 
  // append the new minor error to the stack

  _minorErrorObjectStack = (SdmInternalErrorObject**)(_minorErrorStackSize ? 
						     realloc(_minorErrorObjectStack, ((_minorErrorStackSize+1)*sizeof(SdmInternalErrorObject*))) : 
						     malloc(sizeof(SdmInternalErrorObject*)));
  _minorErrorObjectStack[_minorErrorStackSize++] = meo;
}

