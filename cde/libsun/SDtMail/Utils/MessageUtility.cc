/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Message Utility base class.

#pragma ident "@(#)MessageUtility.cc	1.74 97/06/05 SMI"

// Include Files
#include <SDtMail/MessageUtility.hh>
#include <SDtMail/SystemUtility.hh>
#include <Utils/LockUtility.hh>
#include <Utils/CclientUtility.hh>
#include <PortObjs/DataPort.hh>
#include <mail.h>
#include <rfc822.h>
#include <os_sv5.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/systeminfo.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <SDtMail/MailRc.hh>
#include <DataStructs/MutexEntry.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <errno.h>

// headers and definitioans used by GetRemoteMailboxHostName
#include <sys/mnttab.h> /* sysv */
#include <sys/mntent.h>
#define mntent mnttab
#define mnt_fsname mnt_special
#define mnt_dir mnt_mountp
#define mnt_type mnt_fstype
#define mnt_opts mnt_mntopts


// I18N 
#include <SDtMail/Connection.hh>
#include <lcl/lcl.h>
#include <widec.h>
#include <wctype.h>
#include <limits.h>
// I18N end

// Macros used by this module
#define ISWCSPACEORCOMMA(WC) (((WC) == ',') || ((WC) == ' ') || ((WC) == '\t'))
#define ISWCSPACE(WC) (((WC) == ' ') || ((WC) == '\t'))
#define WCGETNEXT(WC_CHAR, SB_CHAR) mbtowc(&WC_CHAR, SB_CHAR, MB_LEN_MAX)

// Constants used by this module

mutex_t SdmMessageUtility::_sendingUserNameMutex = DEFAULTMUTEX;
char* SdmMessageUtility::_sendingUserName = 0;

static const char* HEADER_TO = "To";
static const char* HEADER_CC = "Cc";
static const char* HEADER_BCC = "Bcc";
static const char* HEADER_REPLY_TO = "Reply-To";

#define CR '\015'
static const char* xmailerHeader = "X-Mailer";
static const int LINESIZE = 5120;

// forward declarations for static internal functions
static char *advancePastRfc822Comment(char *line);
static char *parse_one_addr(char *cp, int *found);
static char *parse_addr(char *cp, int *found, char **aptr);
static SdmBoolean metoo_addr(SdmString& addr, SdmStringL & alts, SdmBoolean allnet);
static void skin_comma(char * buf);
static char *strip_hosts(char * addr);
static char *phrase(char *name, int token, int comma);

SdmString* SdmMessageUtility::_remoteMailboxHostName = NULL;

SdmMessageUtility::SdmMessageUtility()
{
}

SdmMessageUtility::~SdmMessageUtility()
{
}

SdmErrorCode SdmMessageUtility::AddXMailerHeader
  (
  SdmError& err,
  const SdmString& mailerPrefix,
  SdmDataPort& dataPort,
  SdmMessageNumber msgnum,
  SdmBoolean replaceExistingHeader
  )
{
  SdmError localError;
  SdmStrStrL localHeaders;

  err = Sdm_EC_Success;

  if (replaceExistingHeader == Sdm_True || dataPort.GetMessageHeader(localError, localHeaders, xmailerHeader, msgnum) != Sdm_EC_Success) {
    // No X-Mailer header -- add one
    const int bufrsize = 128;
    char bufr[bufrsize+1];

    SdmString theHeader = mailerPrefix;
    theHeader += " ";
    if (sysinfo(SI_SYSNAME, bufr, bufrsize) != -1) {
      theHeader += bufr;
      theHeader += " ";
    }
    if (sysinfo(SI_RELEASE, bufr, bufrsize) != -1) {
      theHeader += bufr;
      theHeader += " ";
    }
    if (sysinfo(SI_MACHINE, bufr, bufrsize) != -1) {
      theHeader += bufr;
      theHeader += " ";
    }
    if (sysinfo(SI_ARCHITECTURE, bufr, bufrsize) != -1) {
      theHeader += bufr;
      theHeader += " ";
    }
    dataPort.ReplaceMessageHeader(err, xmailerHeader, theHeader, msgnum);
  }
  return(err);
}

void SdmMessageUtility::GenerateUnixFromLine
  (
  SdmString& r_fromLine
  )
{
  // Generate the Unix From line...
  //

  char from_buf[150];
  char time_buf[50];
  SdmString sendingUserName;

  SdmMessageUtility::GetMailSendingUserName(sendingUserName);
  time_t now = time(NULL);
  SdmSystemUtility::SafeCtime(&now, time_buf, sizeof(time_buf));

  sprintf(from_buf, "From %s %s", (const char *)sendingUserName, time_buf);
  assert(strlen(from_buf) < sizeof(from_buf));

  r_fromLine = from_buf;
}

// RemoveMessageStoreIfEmpty:
// This utility function takes the name of a file, and after obtaining a lock
// on the file if the file is empty, then the file is removed. This is used
// primarily to implement the "keep" .mailrc variable functionality

SdmErrorCode SdmMessageUtility::RemoveMessageStoreIfEmpty(
  SdmError& err,
  const SdmString& messageStoreName	// name of file to be removed if empty
  )
{
  char *tempExpandedFileName = NULL;
  SdmString expandedFileName;
  SdmMailRc* mrc = SdmMailRc::GetMailRc();

  // expand mailbox file name
  // Must be careful: tempExpandedFileName is filled in with a malloc()ed pointer that
  // we must free before we return, so we stuff it into an SdmString object.. yuk!

  if (SdmUtility::ExpandPath(err, tempExpandedFileName, messageStoreName, *mrc) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }
  assert(tempExpandedFileName);
  expandedFileName = tempExpandedFileName;
  free(tempExpandedFileName);

  // get the update lock on the file [ SHOULD BE LONG TERM LOCK BUT NO CODE RIGHT NOW! ]

  SdmLockUtility filelock;

  if (filelock.IdentifyLockObject(err, expandedFileName) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }

  if (filelock.ObtainLocks(err, Sdm_LKT_Update) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }

  // Got the lock on the file - now safe to manipulate

  struct stat sbuf;

  if (SdmSystemUtility::SafeStat((const char *)expandedFileName, &sbuf) == -1) {
    int lerrno = errno;
    err = Sdm_EC_ErrorUpdatingMessageStore;
    err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_CannotStatMessageStore,
				    (const char *)expandedFileName,
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  // If the message store is indeed now empty we can remove it

  if (sbuf.st_size == 0) {
    // File is empty - remove it
    if (SdmSystemUtility::SafeUnlink((const char *)expandedFileName) == -1) {
      int lerrno = errno;
      err = Sdm_EC_ErrorUpdatingMessageStore;
      err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_ErrorUpdatingMessageStore,
				      (const char *)expandedFileName,
				      (const char *)SdmUtility::ErrnoMessage(lerrno));
      return(err);
    }
  }

  // Always return success if file was not removed cause it was not empty
  // or if file was successfully removed because it was empty

  return (err = Sdm_EC_Success);
}


// Append message to file:
// This utility function takes the name of a file, and a string that represents
// a complete e-mail message as it would appear in a message store 
// (e.g. RFC822/RFC1521 compliant) and causes it to be appended to the message
// store indicated by the specified file name
//
// It uses the appropriate locking facilities to assure that the message store
// consistency is maintained.
//

SdmErrorCode SdmMessageUtility::AppendMessageToFile(
  SdmError& err, 
  const SdmString& fileName, 	// name of message store to append message to
  const SdmString& theHeaders,	// headers for the message
  const SdmString& theBody	// body for the message
  )
{
  SdmString fromLine;
  char *tempExpandedFileName = NULL;
  SdmString expandedFileName;
  SdmMailRc* mrc = SdmMailRc::GetMailRc();

  // get a Unix from line
  SdmMessageUtility::GenerateUnixFromLine(fromLine);
  assert(fromLine.Length());

  // expand mailbox file name
  // Must be careful: tempExpandedFileName is filled in with a malloc()ed pointer that
  // we must free before we return, so we stuff it into an SdmString object.. yuk!

  if (SdmUtility::ExpandPath(err, tempExpandedFileName, fileName, *mrc) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }
  assert(tempExpandedFileName);
  expandedFileName = tempExpandedFileName;
  free(tempExpandedFileName);

  // get the update lock on the mailbox

  SdmLockUtility filelock;

  if (filelock.IdentifyLockObject(err, expandedFileName) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }

  if (filelock.ObtainLocks(err, Sdm_LKT_Update) != Sdm_EC_Success) {
    if (err == Sdm_EC_Fail)
      err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    else
      err.AddMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    return(err);
  }

  // open the mailbox for appending purposes

  int mbfd = SdmSystemUtility::SafeOpen((const char *)expandedFileName, 
					O_RDWR | O_CREAT, 0600);
  if (mbfd == -1) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_CannotAccessMessageStore);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_CannotAccessMessageStore,
				    (const char *)expandedFileName, 
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  // extend the size of the mailbox
  
  off_t mbsize = SdmSystemUtility::SafeLseek(mbfd, 0L, SEEK_END);
  if (mbsize == -1) {
    int lerrno = errno;
    (void) SdmSystemUtility::SafeClose(mbfd);
    err.SetMajorErrorCode(Sdm_EC_ErrorUpdatingMessageStore);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_ErrorUpdatingMessageStore,
				    (const char *)expandedFileName, 
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  // Determine how many initiators to write at the beginning of the message
  // At least two new lines must be at the end of the file; if not, make sure there
  // are enough output

  int initiatorLength = 0;
  char readBuffer[2];
  memset(&readBuffer, 0, 2);

  if ( ( mbsize > 2)
       && (SdmSystemUtility::SafeLseek(mbfd, mbsize-2, SEEK_SET) == mbsize-2)
       && (SdmSystemUtility::SafeRead(mbfd, readBuffer, 2) == 2)) {
    if (readBuffer[0] != '\n')
      initiatorLength++;
    if (readBuffer[1] != '\n')
      initiatorLength++;
  }

  // Determine how many terminators to write at the end of the message
  // At least one newline must be specified; if the message itself does not end in a newline
  // then a second newline must be specified

  int terminatorLength = 1;
  if (theBody[theBody.Length()-1] != '\n')
    terminatorLength++;

  // Compute size of mailbox that can hold this new message

  off_t newmbsize = mbsize + initiatorLength + fromLine.Length() + theHeaders.Length() + theBody.Length() + terminatorLength;
  assert(mbsize < newmbsize);

  // write the new contents
  // Extend the file and then write out the components - if any call fails,
  // set the file size back to the original (pre-update) size before closing it

  if ( (SdmSystemUtility::SafeFTruncate(mbfd, newmbsize) == -1)
       || (SdmSystemUtility::SafeLseek(mbfd, mbsize, SEEK_SET) != mbsize)
       || (SdmSystemUtility::SafeWrite(mbfd, "\n\n", initiatorLength) != initiatorLength)
       || (SdmSystemUtility::SafeWrite(mbfd, (const char *)fromLine, fromLine.Length()) != fromLine.Length())
       || (SdmSystemUtility::SafeWrite(mbfd, (const char *)theHeaders, theHeaders.Length()) != theHeaders.Length())
       || (SdmSystemUtility::SafeWrite(mbfd, (const char *)theBody, theBody.Length()) != theBody.Length())
       || (SdmSystemUtility::SafeWrite(mbfd, "\n\n", terminatorLength) != terminatorLength)
       || (SdmSystemUtility::SafeFsync(mbfd) == -1)) {

    // One of the above operations failed - truncate the file down to the size before
    // we started operating on it and close it - effectively removing any changes we 
    // may have made before the error occurred
    int lerrno = errno;

    (void) SdmSystemUtility::SafeFTruncate(mbfd, mbsize);
    (void) SdmSystemUtility::SafeClose(mbfd);
    err.SetMajorErrorCode(Sdm_EC_ErrorUpdatingMessageStore);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_OBJ_ErrorUpdatingMessageStore,
				    (const char *)expandedFileName, 
				    (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(err);
  }

  // release any storage allocated by this method

  (void) SdmSystemUtility::SafeClose(mbfd);
  return (err = Sdm_EC_Success);
}

static long
_HdrAndBodyCollector(TCPSTREAM *stream,char *string, MAILSTREAM *mstream)
{
  SdmMessageUtility::ClientData *clientData =
    (SdmMessageUtility::ClientData*)stream;

  assert(string);

  // The first call will supply the header string; subsequent calls will
  // supply the chunks comprising the body parts.
  if (clientData->hdrs->Length() == 0) {
    *clientData->hdrs = string;
  }
  else {
    *clientData->body += string;
  }

  return 1;
}

// Construct Whole Message:
// This utility function is used to produce a string that represents a complete
// e-mail message as it would appear in a message store (e.g. RFC822/RFC1521 compliant)
// from a given message in a specified data port.
//
// This is used to convert a message from its component parts into a single ascii
// string suitable for transmission or inclusion in a message store.
//

SdmErrorCode SdmMessageUtility::ConstructWholeMessage(
  SdmError& err, 
  SdmString& r_theHeaders,		// message headers string returned in this container
  SdmString& r_theBody,			// message body string returned in this container
  SdmDataPort& dataPort, 		// data port on which message to operate on exists
  const SdmMessageNumber msgnum,        // message number in data port of message to operate on
  SdmBoolean returnReceipt,		// Sdm_True to cause return receipt to be requested
  SdmBoolean includeBcc,		// Sdm_True to include Bcc in saved message
  SdmBoolean resolveAddresses		// Sdm_True to cause addresses to be resolved
  )
{
  assert(r_theHeaders.Length() == 0);
  assert(r_theBody.Length() == 0);
  int i;

  // Return an error if the target data port is not open
  if (dataPort.CheckOpen(err) != Sdm_EC_Success)
    return(err);

  BODY *body = mail_newbody();
  if (SdmCclientUtility::ConstructMessageBody
      (err, body, dataPort, msgnum, Sdm_False) != Sdm_EC_Success) {
    mail_free_body(&body);
    return err;
  }

  // Apply any transport encodings to the body.
  // Note: During sending, the c-client uses the 8bit variant if the SMTP
  // EHLO command declares it supports 8BITMIME
  rfc822_encode_body_7bit(NULL, body);

  // Since a "dummy" envelope with no addressees is passed to rfc822_output,
  // 1K should be sufficient space for the minimal header that's output.
  ENVELOPE env = {0};
  char headerStorage[MAILTMPLEN];
  memset(&headerStorage, 0, MAILTMPLEN);
  ClientData clientData = {&r_theHeaders, &r_theBody};
  // As of c-client 0.15, the final argument to this function is a
  // MAILSTREAM*.  Unfortunately one isn't available, but from inspection of
  // the c-client code, it appears NIL may be safely provided.
  if (rfc822_output(headerStorage, &env, body, _HdrAndBodyCollector,
                    (TCPSTREAM*)&clientData, returnReceipt, NIL, NIL) == NIL) {
    mail_free_body(&body);
    return (err = Sdm_EC_Fail);
  }

  r_theBody.StripCharacter(CR);

  SdmStrStrL hdrs;
  if (dataPort.GetMessageHeaders(err, hdrs, msgnum) != Sdm_EC_Success) {
    mail_free_body(&body);
    return (err);
  }

  // If addressess need to be resolved, now is the time to do it

  if (resolveAddresses) {
    if (ResolveAddressesInHeaders(err, hdrs) != Sdm_EC_Success) {
      mail_free_body(&body);
      return (err);
    }
  }

  // Determine if any of the mandatory rfc822 headers are already present.
  // If not, we must add them ourselves in order to generate a conforming
  // message.

  const char *s;
  SdmBoolean needDate = Sdm_True;
  SdmBoolean needFrom = Sdm_True;

  for (i = 0; i < hdrs.ElementCount(); i++) {
    s = hdrs[i].GetFirstString();

    if (strcasecmp(s, "Date") == 0)
      needDate = Sdm_False;
    if (strcasecmp(s, "From") == 0)
      needFrom = Sdm_False;
  }

  r_theHeaders = NULL;

  char tmp[SMALLBUFLEN];

  if (needDate) {
    r_theHeaders += "Date: ";
    rfc822_date(tmp);
    r_theHeaders += tmp;
    r_theHeaders += "\n";
  }
  if (needFrom) {
    char *gecos = strdup(SdmSystemUtility::GetQuotedPersonalName());
    SdmString sendingUserName;
    SdmMessageUtility::GetMailSendingUserName(sendingUserName);
    SdmString sendingHostName;
    SdmMessageUtility::GetMailSendingHostName(sendingHostName);

    if (!gecos) {
      mail_free_body(&body);
      return (err = Sdm_EC_Fail);
    }
    SdmMessageUtility::EncodeHeader(&gecos, "From: ", Sdm_False, Sdm_False);
    r_theHeaders += "From: ";
    r_theHeaders += gecos;
    sprintf(tmp, " <%s@%s>\n", (const char *)sendingUserName, 
	    (const char *)sendingHostName);
    r_theHeaders += tmp;
    free(gecos);
  }

  // Copy headers from the source message, omitting any that are
  // inappropriate or that are reinserted later. Do appropriate header
  // encodings in support of i18n

  for (i = 0; i < hdrs.ElementCount(); i++) {

    s = hdrs[i].GetFirstString();

    // Weed out headers that are reinserted later or are inappropriate for a save

    if (strcasecmp(s, "X-UID") == 0 ||
        strcasecmp(s, "Message-ID") == 0 ||
        strcasecmp(s, "Mime-Version") == 0 ||
        strcasecmp(s, "Content-Type") == 0 ||
        strcasecmp(s, "Content-Length") == 0 ||
        strcasecmp(s, "Content-Transfer-Encoding") == 0 ||
        strcasecmp(s, "Content-ID") == 0 ||
        strcasecmp(s, "Content-Description") == 0 ||
        strcasecmp(s, "Content-MD5") == 0 ||
        (!includeBcc && strcasecmp(s, "Bcc") == 0))
      continue;

    // If this header requires i18n encoding, do so now, and then add the header value 
    // to the envelope

    if (::strcasecmp(s, "to")==0
        || ::strcasecmp(s, "apparently-to")==0
        || ::strcasecmp(s, "resent-to")==0
        || ::strcasecmp(s, "cc")==0
        || ::strcasecmp(s, "bcc")==0
        || ::strcasecmp(s, "reply-to")==0
        || ::strcasecmp(s, "subject")==0) {
      char* headerValue;
      SdmString headerName(s);
      headerName += ": ";
      headerValue = strdup((const char *)(hdrs[i].GetSecondString()));
      SdmMessageUtility::EncodeHeader(&headerValue, (char *)((const char *)headerName), Sdm_False, Sdm_False);
      r_theHeaders += headerName;
      r_theHeaders += headerValue;
      r_theHeaders += "\n";
      free(headerValue);
    }
    else {
      r_theHeaders += s;
      r_theHeaders += ": ";
      r_theHeaders += hdrs[i].GetSecondString();
      r_theHeaders += "\n";
    }
  }

  // Leave the generation of a message i.d. to the smtp delivery agent

  // add the mime version header.  this should come before the 
  // Content-Type header.
  r_theHeaders += "Mime-Version: 1.0\n";

  // Call rfc822_write_body_header to construct the following headers:
  // Content-Type, Content-Transfer-Encoding, Content-ID,
  // Content-Description and Content-MD5
  tmp[0] = '\0';
  char *t = tmp;
  rfc822_write_body_header(&t, body);
  r_theHeaders += tmp;

  r_theHeaders += "Content-Length: ";
  sprintf(tmp, "%d\n", r_theBody.Length());
  r_theHeaders += tmp;

  // Add the trailing LF which delimits the headers
  r_theHeaders += "\n";

  // Strip the CR characters that are inserted by rfc822_write_body_header
  r_theHeaders.StripCharacter(CR);

  mail_free_body(&body);

  return (err = Sdm_EC_Success);
}

// Given an address list, walk through it and expand any aliases that
// may be present. Prevent duplicates and guard against alias loops.
//

void SdmMessageUtility::ExpandRFC822LocalAddressAliases(SdmStringL& addressList)
{
  // --> THIS CODE NEEDS TO OPERATE IN NON-ASCII LOCALES!!

  SdmStringL aliasesVisited;

  // Here is the plan:
  // We walk down the address list and check to see if each element
  // is an alias. If it is, we check the list of visited aliases and
  // if it is not in the list, we expand the alias and cause any non
  // duplicate addresses to be appended to the address list. Since they
  // are added at the end of the list, if they contain any aliases themselves
  // we will eventually walk into them and cause them to be expanded. And
  // so on. Recursion can be to any depth and need not be checked for loops
  // because we refuse to put aliases on the list that have been visited before.

  for (int i = 0; i < addressList.ElementCount(); i++) {
    SdmError localError;
    SdmString* srcAddress = &addressList[i];
    SdmMailRc* mrc = SdmMailRc::GetMailRc();
    assert(mrc);

    // Lookup this address in the alias table
    // If it is an alias and it does not reference itself and it has
    // not been visited before, then expand the alias and add all non
    // duplicated addresses to the address list. 

    const char* alias = mrc->GetAlias(localError, (const char *)*srcAddress);
    if (localError == Sdm_EC_Success && alias && *srcAddress != alias) {
      SdmStringL localAddressList;
      int insertIndex = i;
      if (aliasesVisited.FindElement(*srcAddress) == Sdm_Not_Found) {
	aliasesVisited(-1) = *srcAddress;

	// Insert addressess from alias into the address list - cause them to be
	// sequentially inserted after the current entry so the expansion occurs
	// "in place" relative to the other addressess. Also, cause ghost commas
	// to be inserted at this time, so that aliases that do not contain complex
	// e-mail addresses be allowed to use a space to separate addresses.

	SdmString aliasHeader(alias);
	SdmMessageUtility::InsertGhostCommasInHeader(aliasHeader, aliasHeader);
	ConvertHeadersToAddressList(localAddressList, aliasHeader);
	int localNumElements = localAddressList.ElementCount();
	for (int j = 0; j < localNumElements; j++)
	  addressList.InsertElementAfter(localAddressList[j], insertIndex++);
      }

      // This is an alias, so remove this entry from the list. If it has been
      // visited before, nothing needs to be done. If it has not, we just included
      // its information ahead of us on the list so we will get around to it next.

      addressList.RemoveElement(i);
      i--;
    }
  }
}

// Given a list of headers, cause all address headers to be resolved
// This is usually done prior to writing out a message that is supposed to be
// a "copy of the message sent" meaning it has to resemble a sent message

SdmErrorCode SdmMessageUtility::ResolveAddressesInHeaders(SdmError& err, SdmStrStrL& hdrs)
{
  SdmStrStrL toHeaders, ccHeaders, bccHeaders, replyToHeaders;
  SdmStringL toAddressList, ccAddressList, bccAddressList, replyToAddressList;
  SdmString toHeader, ccHeader, bccHeader, replyToHeader;

  assert(err == Sdm_EC_Success);

  // First, collect all of the addressing headers we know about

  int numElements = hdrs.ElementCount();
  for (int i = 0; i < numElements; i++) {
    const char *headerName = (const char *)(hdrs[i].GetFirstString());
    assert(headerName);
    assert(*headerName);
    if (::strcasecmp(headerName, HEADER_TO) ==0 )
      toHeaders(-1) = hdrs[i];
    else if (::strcasecmp(headerName, HEADER_CC) == 0)
      ccHeaders(-1) = hdrs[i];
    else if (::strcasecmp(headerName, HEADER_BCC) == 0)
      bccHeaders(-1) = hdrs[i];
    else if (::strcasecmp(headerName, HEADER_REPLY_TO) == 0)
      replyToHeaders(-1) = hdrs[i];
  }

  // Reduce multiple header fields to a single header field each

  SdmMessageUtility::MergeHeaders(toHeader, toHeaders);
  SdmMessageUtility::MergeHeaders(ccHeader, ccHeaders);
  SdmMessageUtility::MergeHeaders(bccHeader, bccHeaders);
  SdmMessageUtility::MergeHeaders(replyToHeader, replyToHeaders);

  // Insert ghost commas into the headers;
  // These become permanent and appear in the final e-mail message stored.

  SdmMessageUtility::InsertGhostCommasInHeader(toHeader, toHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(ccHeader, ccHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(bccHeader, bccHeader);
  SdmMessageUtility::InsertGhostCommasInHeader(replyToHeader, replyToHeader);

  // Parse each header field to see if the contents are grokable; If
  // not in the "C" locale, we need to defer this until after aliases
  // are expanded, as aliases may be in non-ASCII and/or multi-byte
  // form

  int errorIndex;

  if (SdmSystemUtility::InCLocale()) {
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, toHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, ccHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, bccHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, replyToHeader) != Sdm_EC_Success)
      return(err);
  }

  // Expand the list of headers into an address list for each one

  SdmMessageUtility::ConvertHeadersToAddressList(toAddressList, toHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(ccAddressList, ccHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(bccAddressList, bccHeader);
  SdmMessageUtility::ConvertHeadersToAddressList(replyToAddressList, replyToHeader);

  // Expand any aliases in the address list
  // note: don't need to do alias expansion for reply-to header.

  SdmMessageUtility::ExpandRFC822LocalAddressAliases(toAddressList);
  SdmMessageUtility::ExpandRFC822LocalAddressAliases(ccAddressList);
  SdmMessageUtility::ExpandRFC822LocalAddressAliases(bccAddressList);

  // After alias expansion, remove any addressees that are obviously "files"

  SdmMessageUtility::RemoveFolderNamesFromAddressList(toAddressList);
  SdmMessageUtility::RemoveFolderNamesFromAddressList(ccAddressList);
  SdmMessageUtility::RemoveFolderNamesFromAddressList(bccAddressList);
  SdmMessageUtility::RemoveFolderNamesFromAddressList(replyToAddressList);

  // Convert the address lists back to headers

  SdmMessageUtility::ConvertAddressListToHeader(toHeader, toAddressList);
  SdmMessageUtility::ConvertAddressListToHeader(ccHeader, ccAddressList);
  SdmMessageUtility::ConvertAddressListToHeader(bccHeader, bccAddressList);
  SdmMessageUtility::ConvertAddressListToHeader(replyToHeader, replyToAddressList);

  // Parse each header field to see if the contents are grokable; If
  // not in the "C" locale, we have deferred this until after aliases
  // are expanded, as aliases may be in non-ASCII and/or multi-byte
  // form; at this point, all addresses must conform to RFC 822 rules
  // for e-mail addresses

  if (!SdmSystemUtility::InCLocale()) {
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, toHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, ccHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, bccHeader) != Sdm_EC_Success)
      return(err);
    if (SdmMessageUtility::ParseRFC822Headers(err, errorIndex, replyToHeader) != Sdm_EC_Success)
      return(err);
  }

  // Replace the respective headers in the original list

  SdmMessageUtility::ReplaceHeaders(HEADER_TO, hdrs, toHeader);
  SdmMessageUtility::ReplaceHeaders(HEADER_CC, hdrs, ccHeader);
  SdmMessageUtility::ReplaceHeaders(HEADER_BCC, hdrs, bccHeader);
  SdmMessageUtility::ReplaceHeaders(HEADER_REPLY_TO, hdrs, replyToHeader);

  return(err = Sdm_EC_Success);
}

// Given a single header, replace it in a list of headers, removing
// any extra headers by that name; the header is completely removed
// if the value to be set is blank

void SdmMessageUtility::ReplaceHeaders(const char* headerName, SdmStrStrL& hdrs, SdmString headerValue)
{
  int found = 0;

  int numElements = hdrs.ElementCount();
  for (int i = 0; i < numElements; i++) {
    // Search for the specified header
    if (::strcasecmp((const char *)(hdrs[i].GetFirstString()), headerName) == 0) {
      // Found the header - if already found it once, this is a duplicate - remove it
      // Otherwise, replace the header value with the new value specified
      if (found || (headerValue.Length()==0)) {
	hdrs.RemoveElement(i);
	i--;
	numElements--;
	found++;
      }
      else {
	found++;
	hdrs[i].SetSecondString(headerValue);
      }
    }
  }
}

// Given an address list and the name of a header, cause that header to
// be rewritten (replaced) in the specified message on the given data port
// Use to rewrite headers after addresses have been parsed and resolved
//

SdmErrorCode SdmMessageUtility::RewriteAddressHeader(
  SdmError& err, 
  const char* headerName, 
  const SdmStringL addressList, 
  SdmDataPort& dataPort, 
  const SdmMessageNumber msgnum)
{
  SdmError localError;
  SdmStrStrL localHeaders;

  // Return an error if the target data port is not open

  if (dataPort.CheckOpen(err) != Sdm_EC_Success)
    return(err);

  // Retrieve the current setting of the indicated header 
  // Use this information later to optimize out the replace/remove if its not necessary

  dataPort.GetMessageHeader(localError, localHeaders, headerName, msgnum);

  int numElements = addressList.ElementCount();
  if (!numElements) {
    // Empty address - remove the header
    // Performance fix: check to see if the header already exists; if not, do not bother doing
    // the remove because if the message has not yet been edited/changed it will cause it to be
    // even if nothing needs to be done

    if (localHeaders.ElementCount())
      dataPort.RemoveMessageHeader(err, headerName, msgnum);
  }
  else {
    // At least one addressee - convert to single string and replace header
    // Performance fix: check to see if there is exactly one header and if the header value
    // matches the value we are about to set; if so dont bother doing the replace 
    // because if the message has not yet been edited/changed it will cause it to be
    // even if nothing needs to be done

    SdmString headerValue;

    ConvertAddressListToHeader(headerValue, addressList);
    if (localHeaders.ElementCount() != 1 || headerValue != localHeaders[0].GetSecondString())
      dataPort.ReplaceMessageHeader(err, headerName, headerValue, msgnum);
  }
  return (err);
}

// Take a list of headers and comma separate merge them onto a single header
//

void SdmMessageUtility::MergeHeaders(
  SdmString& r_allHeaders, 
  const SdmStrStrL& srcHeaders)
{
  int numHeaders = srcHeaders.ElementCount();
  for (int i = 0; i < numHeaders; i++) {
    MergeHeader(r_allHeaders, srcHeaders[i].GetSecondString());
  }
}

// Take a header and comma separate merge it onto a single header
//

void SdmMessageUtility::MergeHeader(
  SdmString& r_allHeaders, 
  const SdmString& srcHeader)
{
  if (srcHeader.Length()) {
    if (r_allHeaders.Length())
      r_allHeaders += ", ";
    r_allHeaders += srcHeader;
  }
}

// This takes an address list and produces a properly formatted comma separated
// list of addressess in a single string suitable to setting a header to
//

void SdmMessageUtility::ConvertAddressListToHeader(
  SdmString& r_header, 
  const SdmStringL& addressList)
{
  r_header = "";
  int numElements = addressList.ElementCount();
  if (numElements) {
    for (int i = 0; i < numElements; i++) {
      r_header += addressList[i];
      if (i != numElements-1)
	r_header += ", ";
    }
  }
}

// This method takes an address list and if there are no "complex" e-mail addresses
// then collapse multiple spaces and commas down to a single comma and a space; we
// call these inserted commas ghost commas because they are not present in the source
// e-mail address because the user didnt specify them, yet to us we see them and make
// them real in the final e-mail to be sent.
//
// This is used so that multiple simple addresses (e.g. you me dog named sue) 
// can be specified without requiring comma's (e.g. you, me, dog, named, sue).
//
// What's a "complex" e-mail address? Well, any address that can have a space inside
// of it. We'll make it simple here and say a complex address is any address that
// contains one of these characters: "<>()[]:;
//

void SdmMessageUtility::InsertGhostCommasInHeader(
  SdmString& destHeader,
  const SdmString& srcHeader)
{
  // --> THIS CODE NEEDS TO OPERATE IN NON-ASCII LOCALES!!

  int destBufLength = 0;
  char* destBuf = 0;
  char *dcp = destBuf = 0;

  const char *srcp = (const char *)srcHeader;
  const char *scp;
  wchar_t wc_char;	// current character in "wide" format
  size_t wc_len;	// length of the current character in "wide" format

  // Scan the source header to see if any 'complex' keys are found; if so, return the
  // source header unmodified; this could be made faster with a table lookup, but it would
  // require a lot of bytes so instead it appears the compiler does best with a switch

  for (scp = srcp; (wc_len = WCGETNEXT(wc_char, scp)) != 0; scp += wc_len) {
    if (wc_len < 0) {
      destHeader = srcHeader;
      return;
    }
    if (iswascii(wc_char))
      switch (wc_char) {
	// These characters trigger a complex address
      case '"':
      case '<':
      case '>':
      case '(':
      case ')':
      case '[':
      case ']':
      case ':':
      case ';':
	destHeader = srcHeader;
	return;
      }
  }

  // This header is not "complex"; apply ghost comma insertion
  // to multiple blanks and commas

  destBufLength = (srcHeader.Length()+1)*2;	// minimum address is "x "
  destBuf = (char *)malloc(destBufLength+1);
  assert(destBuf);
  dcp = destBuf;
  scp = srcp;

  // First skip past all leading spaces and commas

  while ((wc_len = WCGETNEXT(wc_char, scp))>0 && ISWCSPACEORCOMMA(wc_char))
    scp += wc_len;
  
  // Now scan forward until a space or comma is seen; insert a comma and a space
  // and then remove all immediately following occurances of a space or a comma
  
  while ((wc_len = WCGETNEXT(wc_char, scp)) > 0) {
    if (ISWCSPACEORCOMMA(wc_char)) {
      while ((wc_len = WCGETNEXT(wc_char, scp))>0 && ISWCSPACEORCOMMA(wc_char))
	scp += wc_len;
      if ((wc_len = WCGETNEXT(wc_char, scp)) > 0) {
	*dcp++ = ',';
	*dcp++ = ' ';
      }
    }
    else {
      while (wc_len--)
	*dcp++ = *scp++;
    }
  }

  *dcp = '\0';
  assert(dcp < destBuf+destBufLength);

  // If its all nothing but whitespace, return the source header and let the
  // rfc 822 parser fail; otherwise, return the processed results

  if (dcp == destBuf)
    destHeader = srcHeader;
  else
    destHeader = destBuf;
  free((void *)destBuf);
}

// This little diddly takes a single string that consists of a properly structured
// list of addresses (that is, each address is separated by a ",") and reduces it
// to a string list where each element in the list is a one of the addresses
//
// Takes care to not insert the same address twice, and handle the cases where
// phrases and comments can be nested or contain commas or other valid cases.
//


void SdmMessageUtility::ConvertHeadersToAddressList(
  SdmStringL& r_addressList, 
  const SdmString& srcHeader)
{
  // --> THIS CODE NEEDS TO OPERATE IN NON-ASCII LOCALES!!

  typedef enum parseState_T { 
    ps_waitingforfirstbyte,	// waiting for first byte of address
    ps_inaddress,		// inside of a general address specification
    ps_incomment,		// inside of a comment (e.g. (comment))
    ps_indomainliteral,		// inside of a domain literal (e.g. [address])
    ps_ingroupspec,		// inside of a group specification (e.g. group:list)
    ps_init, 			// initialization required - no address bytes yet processed
    ps_inquotedstring,		// inside of a quoted string (e.g. "string")
    ps_inroutespec		// inside of a route specification (e.g. <route>)
  } parseState;
  parseState state = ps_init;

#if 0
  const char *srcp = (const char *)srcHeader;
  const char *firstAddressByte = 0;
  const char *lastAddressByte = 0;
  char c = 0;
  int commentCount = 0;

  // Spin as long as there are characters to process

  for (*srcp; c = *srcp; srcp++) {

    // If in the "need to initialize" state, initialize for another address
    // and enter waiting for first byte state

    if (state == ps_init) {
      firstAddressByte = lastAddressByte = 0;	
      commentCount = 0;
      state = ps_waitingforfirstbyte;
    }

    // If waiting for the first byte of an address, skip leading white space and separators;
    // otherwise, remember first byte of address and enter address parse state

    if (state == ps_waitingforfirstbyte) {
      if ((isspace(c)) || (c == ','))
	continue;
      firstAddressByte = srcp;
      state = ps_inaddress;
    }

    // HANDLE END OF LINE 

    // Not in the init or waiting for first byte state - handle this byte of the address

    switch (state) {
      // for ps_inaddress we are outside of any special constructs - look for them,
      // entering the appropriate state when found - take a comma to separate an email address
    case ps_inaddress:
      switch(c) {
      case '"':		// a single '"' begins a quoted string
	state = ps_inquotedstring;
	break;
      case '[':		// a [ begins a domain-literal
	state = ps_indomainliteral;
	break;
      case '(':		// a ( begins a comment
	commentCount = 1;
	state = ps_incomment;
	break;
      case '<':		// a < begins a route or address specification
	state = ps_inroutespec;
	break;
      case ':':		// a : begins a group specification
	state = ps_ingroupspec;
	break;
      case ',':		// a comma is an address separator outside of any special context
	lastAddressByte = srcp-1;
	while (lastAddressByte >= firstAddressByte && isspace(*lastAddressByte))
	  lastAddressByte--;
	if (lastAddressByte >= firstAddressByte) {
	SdmString newAddress(firstAddressByte, (lastAddressByte-firstAddressByte)+1);
	if (r_addressList.FindElement(newAddress) == Sdm_Not_Found)
	  r_addressList(-1) = newAddress;
	}
	state = ps_init;
	break;
      case ']':		// a [ outside of a domain literal is an error
      case '>':		// a > outside of a route spec is an error
      case ')':		// a ) outside of a comment is an error
	break;		// right now just let them pass - the RFC 822 parser will barf them up
      default:
	break;
      }
      break;
      // These states indicate we are inside of a special construct that does not take
      // a comma to separate an email address and does allow quoting.
    case ps_inquotedstring:
    case ps_indomainliteral:
    case ps_inroutespec:
      switch(c) {
      case '"':
	if (state == ps_inquotedstring)
	  state = ps_inaddress;
	break;
      case ']':
	if (state == ps_indomainliteral)
	  state = ps_inaddress;
	break;
      case '>':
	if (state == ps_inroutespec)
	  state = ps_inaddress;
	break;
      case '\\':
	if (srcp[1])
	  srcp++;
	break;
      default:
	break;
      }
      break;
      // Inside of a () comment - allow nested comments e.g. (a(b(c))) and quoting but do not
      // take a comma to separate an email address.
    case ps_incomment:
      switch(c) {
      case '(':
	commentCount++;
	break;
      case ')':
	if (commentCount--)
	  state = ps_inaddress;
	break;
      case '\\':
	if (srcp[1])
	  srcp++;
	break;
      default:
	break;
      }
      break;
      // Inside of a group spec e.g. name:r1,r2,r3; - scan until end of spec found (;)
    case ps_ingroupspec:
      switch(c) {
      case ';':
	state = ps_inaddress;
	break;
      default:
	break;
      }
      break;
      // illegal state - impossible condition - cannot get here from there
      // reset to initial state and pray
    default:
      assert(0 && state);
      state = ps_init;
      break;
    }
  }

  // Ok, we have come to the end of the header - if we are not in the init/waiting state
  // then we are in the middle of an address - save it
  if (state != ps_init && state != ps_waitingforfirstbyte && firstAddressByte) {
    lastAddressByte = srcp;
    while (lastAddressByte >= firstAddressByte && isspace(*lastAddressByte))
      lastAddressByte--;
    if (lastAddressByte >= firstAddressByte) {
      SdmString newAddress(firstAddressByte, (lastAddressByte-firstAddressByte)+1);
      if (r_addressList.FindElement(newAddress) == Sdm_Not_Found)
	r_addressList(-1) = newAddress;
    }
  }
#else
  const wchar_t *firstAddressByte = 0;
  const wchar_t *lastAddressByte = 0;
  int commentCount = 0;
  wchar_t* wcs = 0;
  if (srcHeader.Length()==0)
    return;
  wcs = (wchar_t*) malloc(((srcHeader.Length())+1)*sizeof(wchar_t));
  if (!wcs) {
    r_addressList(-1) = srcHeader;
    return;
  }
  size_t wcl = mbstowcs(wcs, (const char *)srcHeader, srcHeader.Length());
  if (wcl <= 0) {
    r_addressList(-1) = srcHeader;
    free(wcs);
    return;
  }
  wcs[wcl] = 0;
  wchar_t* wcsp = wcs;
  wchar_t wc_char;

  // Spin as long as there are characters to process

  for (*wcsp; wc_char = *wcsp; wcsp++) {

    // If in the "need to initialize" state, initialize for another address
    // and enter waiting for first byte state

    if (state == ps_init) {
      firstAddressByte = lastAddressByte = 0;	
      commentCount = 0;
      state = ps_waitingforfirstbyte;
    }

    // If waiting for the first byte of an address, skip leading white space and separators;
    // otherwise, remember first byte of address and enter address parse state

    if (state == ps_waitingforfirstbyte) {
      if ((iswspace(wc_char)) || (wc_char == ','))
	continue;
      firstAddressByte = wcsp;
      state = ps_inaddress;
    }

    // HANDLE END OF LINE 

    // Not in the init or waiting for first byte state - handle this byte of the address

    switch (state) {
      // for ps_inaddress we are outside of any special constructs - look for them,
      // entering the appropriate state when found - take a comma to separate an email address
    case ps_inaddress:
      assert(commentCount==0);
      switch(wc_char) {
      case '"':		// a single '"' begins a quoted string
	state = ps_inquotedstring;
	break;
      case '[':		// a [ begins a domain-literal
	state = ps_indomainliteral;
	break;
      case '(':		// a ( begins a comment
	commentCount = 1;
	state = ps_incomment;
	break;
      case '<':		// a < begins a route or address specification
	state = ps_inroutespec;
	break;
      case ':':		// a : begins a group specification
	state = ps_ingroupspec;
	break;
      case ',':		// a comma is an address separator outside of any special context
	lastAddressByte = wcsp-1;
	while (lastAddressByte >= firstAddressByte && iswspace(*lastAddressByte))
	  lastAddressByte--;
	if (lastAddressByte >= firstAddressByte) {
	SdmString newAddress(firstAddressByte, (lastAddressByte-firstAddressByte)+1);
	if (r_addressList.FindElement(newAddress) == Sdm_Not_Found)
	  r_addressList(-1) = newAddress;
	}
	state = ps_init;
	break;
      case ']':		// a [ outside of a domain literal is an error
      case '>':		// a > outside of a route spec is an error
      case ')':		// a ) outside of a comment is an error
	break;		// right now just let them pass - the RFC 822 parser will barf them up
      default:
	break;
      }
      break;
      // These states indicate we are inside of a special construct that does not take
      // a comma to separate an email address and does allow quoting.
    case ps_inquotedstring:
    case ps_indomainliteral:
    case ps_inroutespec:
      switch(wc_char) {
      case '"':
	if (state == ps_inquotedstring)
	  state = ps_inaddress;
	break;
      case ']':
	if (state == ps_indomainliteral)
	  state = ps_inaddress;
	break;
      case '>':
	if (state == ps_inroutespec)
	  state = ps_inaddress;
	break;
      case '\\':
	if (wcsp[1])
	  wcsp++;
	break;
      default:
	break;
      }
      break;
      // Inside of a () comment - allow nested comments e.g. (a(b(c))) and quoting but do not
      // take a comma to separate an email address.
    case ps_incomment:
      switch(wc_char) {
      case '(':
	commentCount++;
	break;
      case ')':
	if (!--commentCount)
	  state = ps_inaddress;
	assert(commentCount>=0);
	break;
      case '\\':
	if (wcsp[1])
	  wcsp++;
	break;
      default:
	break;
      }
      break;
      // Inside of a group spec e.g. name:r1,r2,r3; - scan until end of spec found (;)
    case ps_ingroupspec:
      switch(wc_char) {
      case ';':
	state = ps_inaddress;
	break;
      default:
	break;
      }
      break;
      // illegal state - impossible condition - cannot get here from there
      // reset to initial state and pray
    default:
      assert(0 && state);
      state = ps_init;
      break;
    }
  }

  // Ok, we have come to the end of the header - if we are not in the init/waiting state
  // then we are in the middle of an address - save it
  if (state != ps_init && state != ps_waitingforfirstbyte && firstAddressByte) {
    lastAddressByte = wcsp;
    while (lastAddressByte >= firstAddressByte && iswspace(*lastAddressByte))
      lastAddressByte--;
    if (lastAddressByte >= firstAddressByte) {
      SdmString newAddress(firstAddressByte, (lastAddressByte-firstAddressByte)+1);
      if (r_addressList.FindElement(newAddress) == Sdm_Not_Found)
	r_addressList(-1) = newAddress;
    }
  }
  free(wcs);
#endif
}

// Take an address list and create a new list without any duplicates
//

void SdmMessageUtility::CompressAddressList(
  SdmStringL& r_addressList, 
  const SdmStringL& srcAddressList)
{
  int numElements = srcAddressList.ElementCount();
  for (int i = 0; i < numElements; i++) {
    SdmString* srcAddress = &srcAddressList[i];
    if (r_addressList.FindElement(*srcAddress) == Sdm_Not_Found)
      r_addressList(-1) = *srcAddress;
  }
}

// Take an address list and remove any addresses that appear to be file name references,
// adding them to the file name list container argument provided
//

void SdmMessageUtility::ExtractFolderNamesFromAddressList(
  SdmStringL& r_filenameList, 
  SdmStringL& srcAddressList)
{
  int numElements = srcAddressList.ElementCount(); 
  for (int i = 0; i < numElements; i++) {
    SdmString* srcAddress = &srcAddressList[i];
    const char* address = (const char *)*srcAddress;
    // Kick out addresses that begin with a / but are really x.400 addresses
    // Even though we are RFC 822ing it here, it is possible that an x.400 gateway
    // is out there and we might send e-mail to such a gateway with an addressee
    // that is really a full x.400 address, such as:
    // /PN=joe.blo/PRMD=ibmmail/ADMD=ibmx400/C=us/@mhs-mci.ebay
    // So the key is to search the address for an = and an @ and if both are found,
    // then we can assume this is an x.400 address and not tag it as a file name
    if (*address == '/') {
      int eqfound = 0, atfound = 0;
      for (const char *cp = address; *cp; cp++) {
	if (*cp == '=')
	  eqfound = 1;
	else if (*cp == '@')
	  atfound = 1;
      }
      if (atfound && eqfound)
	continue;
    }
    if (*address == '+' || *address == '/' || *address == '~') {
      if (r_filenameList.FindElement(*srcAddress) == Sdm_Not_Found) {
	r_filenameList(-1) = *srcAddress;
	srcAddressList.RemoveElement(i);
	i--;
	numElements--;
      }
    }
  }
}

// Take an address list and remove any addresses that appear to be file name references.
//

void SdmMessageUtility::RemoveFolderNamesFromAddressList(
  SdmStringL& srcAddressList)
{
  int numElements = srcAddressList.ElementCount(); 
  for (int i = 0; i < numElements; i++) {
    SdmString* srcAddress = &srcAddressList[i];
    const char* address = (const char *)*srcAddress;
    if (*address == '+' || *address == '/') {
      srcAddressList.RemoveElement(i);
      i--;
      numElements--;
    }
  }
}

// Parse a header

SdmErrorCode SdmMessageUtility::ParseRFC822Headers(
  SdmError& err, 
  int& r_errorIndex,
  const SdmString& srcHeader)
{
  int notvalid;
  char *aptr;
  SdmString revisedHeader;

  // Cause ghost commas to be inserted into the header before parsing; normally the headers
  // should already have this done to them if the message is about to be sent or stored; 
  // however, we do this again here in case the address is being verified by the front end
  // in which case we want to allow non-complex headers to pass muster

  SdmMessageUtility::InsertGhostCommasInHeader(revisedHeader, srcHeader);
  
  err = Sdm_EC_Success;
  r_errorIndex = 0;

  if (revisedHeader.Length()) {
    char* headerPtr = (char *)((const char *)revisedHeader);
    parse_addr(headerPtr, &notvalid, &aptr);
    if (notvalid == Sdm_EC_Success) {
      err = Sdm_EC_Success;
    }
    else {
      r_errorIndex = aptr-headerPtr;
      err.SetMajorErrorCode(Sdm_EC_RFC822AddressParseFailure);
      err.AddMinorErrorCodeAndMessage(notvalid, r_errorIndex >= revisedHeader.Length() ? revisedHeader : revisedHeader+r_errorIndex);
    }
  }
  return(err);
}

// Make a reply header from a header in an existing message
// This method extracts e-mail addresses from the headers specified by
// the abstract header flags list from the specified message envelope
// object and returns a string object suitable for a "reply" to those
// e-mail addresses.
// 
// This method is used to process exiting Sender:, To:, Cc: and other
// appropriate headers into corresponding reply headers.
//
// The 'replyAll' boolean Controls the removal of the user doing the
// reply; this is used to differentiate between a "reply" and a "reply
// all".
// 
// If this is Sdm_True, then the user who is running dtmail will be
// removed from the reply addresses if the appropriate dtmail options are
// also set (metoo, allnet, usealternates mailrc variables).
// 
// If this is Sdm_False, then the user who is running dtmail will not be removed
// regardless of the mailrc variables set.
// 
SdmErrorCode
SdmMessageUtility::MakeReplyHeader(SdmError & error, SdmString & r_headerValue,
				   SdmMessageHeaderAbstractFlags headers, 
				   SdmMessageEnvelope& env, SdmBoolean replyAll)
{
  error.Reset();
  int i, k;
  SdmIntStrL headerValues;

  if (env.GetHeaders(error, headerValues, headers)) 
    return error;
    
  // I18N Decode the header values
  SdmString charset;
  env.GetCharacterSet(error, charset);
  assert(!error);
    
  SdmMailRc *mailrc = SdmMailRc::GetMailRc();
  assert (mailrc != NULL);

  // Merge all of the individual headers into a single header so it makes it easy
  // from here on out - perform header decoding at the same time, and then burst 
  // the single header that contains all of the decoded individual headers into
  // a multi-element address list

  SdmStringL theAddressList;
  SdmString theHeader;

  for (i = 0; i < headerValues.ElementCount(); i++) {
    SdmError localError;
    SdmString tempHeader;
    tempHeader = headerValues[i].GetString();
    if (tempHeader.Length() > 0) {
      if (theHeader.Length()) {
        theHeader += ", ";
      }
      env.DecodeHeader(localError, theHeader, tempHeader, charset);
    }
  }

  // I18N end

  ConvertHeadersToAddressList(theAddressList, theHeader);  // Burst the single header into an multi-element address list 

  // Now go through the multi-element address list and strip each
  // address of its complex components so all we are left with is
  // a list of simple addresses

  for (i = 0, k = theAddressList.ElementCount(); i < k; i++) {
    // we call arpa_phrase for each item to get the 
    // address part (esthert@tanzanite) from the original
    // address.  arpa_phrase will strip out the comment part
    // of the address. we don't pass in the whole list because
    // arpa_phrase will sometime misinterpret blanks as
    // separators and the same userid is placed in the list twice.
    // we should have a one to one mapping between the items in 
    // origAddressList and shortAddressList.
    //
    char *origAddr = strdup((const char*) theAddressList[i]);
    char *addr = phrase(origAddr, 0, 0);
    assert (addr != NULL);
    if (origAddr != addr) {
      theAddressList[i] = addr;
      free(origAddr);
    }
    free(addr);
  }

  // Now we have a list of all addresses that have been reduced from complex e-mail
  // addresses to their base components

  // If metoo not defined, we need to check for allnet and alternates and process
  // the address list as appropriate

  if ((replyAll == Sdm_True) && (mailrc->IsValueDefined("metoo") == Sdm_False)) {

    // We need to build a new value that has the user stripped from 
    // the reply list. This includes any alternates the user has specified.
    //
    SdmStringL alts;
    SdmString sendingUserName;
    SdmMessageUtility::GetMailSendingUserName(sendingUserName);
    const char* sendingHostName = SdmSystemUtility::GetHostName();

    char * my_addr = new char[strlen((const char *)sendingUserName) + strlen(sendingHostName) + 5];
    strcpy(my_addr, (const char *)sendingUserName);
    alts(-1) = my_addr;            // add esthert to list of alts.
    char * my_nodename = new char[strlen(sendingHostName)+ 2];
    strcpy(my_nodename, "@");
    strcat(my_nodename, sendingHostName);
    strcat (my_addr, my_nodename);
    alts(-1) = my_addr;	          // add esthert@tanzanite to list of alts.
    delete [] my_addr;

    // Check to see if the user wants us to ignore the host
    // component of the address when stripping.
    //
    SdmBoolean allnet = mailrc->IsValueDefined("allnet");

    // Fetch the alternates, if any and add them to the list to strip.
    //
    SdmStringL alternates;
    AddAlternates(alternates);
      
    // add alternates that are defined in .mailrc list to 
    // alternates list.  if allnet is not defined, then we
    // want to add alternate@hostname to the list for each 
    // of these alternates
    for (int k=0; k < alternates.ElementCount(); k++) {
      alts.AddElementToList(alternates[k]);
      if (allnet == Sdm_False) {
        alts.AddElementToList(alternates[k] + my_nodename);
      }
    }

    delete [] my_nodename;

    // check if any of the addresses need to be stripped out because of allnet
    // and/or alternates defined in .mailrc file.  Put only the addresses
    // we want into the keepers list.
    //

    for (int naddr = theAddressList.ElementCount(); naddr; naddr--) {
      if (metoo_addr(theAddressList[naddr-1], alts, allnet) == Sdm_True) {
        theAddressList.RemoveElement(naddr-1);
      }
    }
  }

  // Finally, let's build a single string from the list of header values.
  // Cause the list to be compressed so that duplicates are weeded out.
  //

  SdmStringL compressedAddressList;
  SdmMessageUtility::CompressAddressList(compressedAddressList, theAddressList);

  SdmMessageUtility::ConvertAddressListToHeader(r_headerValue, compressedAddressList);
    
  return error;
}


void
SdmMessageUtility::AddAlternates(SdmStringL &r_alternates)
{
  SdmMailRc *mailrc = SdmMailRc::GetMailRc();
  assert (mailrc != NULL);

  // Fetch the alternates, if any and add them to the list to strip.
  //
  if (mailrc->IsValueDefined("usealternates") == Sdm_True) {

    // create list of alternates that are defined in .mailrc.
    SdmError localError;
    char * others = mailrc->GetAlternates(localError);
    if (!localError && others) {
      char *token;
      if ((token = (char *) strtok(others, " "))) {
        r_alternates(-1) = token; 
        while(token = (char *)strtok(NULL, " ")) {
          r_alternates(-1) = token; 
        }
      }
    }

    if (others) 
      free (others);
  }
}
      
      
      
static SdmBoolean
metoo_addr(SdmString& addr,
		       SdmStringL & alts,
		       SdmBoolean allnet)
{
    char * str_addr_buf = strdup((const char*)addr);
    char * str_addr = str_addr_buf;

    if (allnet) {
      str_addr = strip_hosts(str_addr_buf);
    }

    for (int nalt = 0; nalt < alts.ElementCount(); nalt++) {
      char * cmp_addr_buf = strdup((const char*)alts[nalt]);
      char * cmp_addr = cmp_addr_buf;

      if (allnet) {
          cmp_addr = strip_hosts(cmp_addr_buf);
      }

      if (strcasecmp(str_addr, cmp_addr) == 0) {
          free(str_addr_buf);
          free(cmp_addr_buf);
          return(Sdm_True);
      }

      free(cmp_addr_buf);
    }

    free(str_addr_buf);
    return(Sdm_False);
}

//
// et - the phrase function was taken directly from the mailx source code.
// the only changes made was to replace salloc with malloc and equal with strcmp.
// we use this instead of the old arpa_phrase function because the old
// arpa_phrase function was buggy.
//
/*
 * Skin an arpa net address according to the RFC 822 interpretation
 * of "host-phrase."
 */
static char *
phrase(char *name, int token, int comma)
{
	register char c;
	register char *cp, *cp2;
	char *bufend, *nbufp;
	int gotlt, lastsp, didq;
	char nbuf[LINESIZE];
	int nesting;

	if (name == NULL)
		return(NULL);
	size_t name_length = strlen(name);
	if (name_length >= (unsigned)LINESIZE)
		nbufp = (char *)malloc(name_length+1);
	else
		nbufp = nbuf;
	gotlt = 0;
	lastsp = 0;
	bufend = nbufp;
	for (cp = name, cp2 = bufend; (c = *cp++) != 0;) {
		switch (c) {
		case '(':
			/*
				Start of a comment, ignore it.
			*/
			nesting = 1;
			while ((c = *cp) != 0) {
				cp++;
				switch(c) {
				case '\\':
					if (*cp == 0) goto outcm;
					cp++;
					break;
				case '(':
					nesting++;
					break;
				case ')':
					--nesting;
					break;
				}
				if (nesting <= 0) break;
			}
		outcm:
			lastsp = 0;
			break;
		case '"':
			/*
				Start a quoted string.
				Copy it in its entirety.
			*/
			didq = 0;
			while ((c = *cp) != 0) {
				cp++;
				switch (c) {
				case '\\':
					if ((c = *cp) == 0) goto outqs;
					cp++;
					break;
				case '"':
					goto outqs;
				}
				if (gotlt == 0 || gotlt == '<') {
					if (lastsp) {
						lastsp = 0;
						*cp2++ = ' ';
					}
					if (!didq) {
						*cp2++ = '"';
						didq++;
					}
					*cp2++ = c;
				}
			}
		outqs:
			if (didq)
				*cp2++ = '"';
			lastsp = 0;
			break;

		case ' ':
		case '\t':
		case '\n':
			if (token && (!comma || c == '\n')) {
			done:
				cp[-1] = 0;
				return cp;
			}
			lastsp = 1;
			break;

		case ',':
			*cp2++ = c;
			if (gotlt != '<') {
				if (token)
					goto done;
				bufend = cp2 + 1;
				gotlt = 0;
			}
			break;

		case '<':
			cp2 = bufend;
			gotlt = c;
			lastsp = 0;
			break;

		case '>':
			if (gotlt == '<') {
				gotlt = c;
				break;
			}

			/* FALLTHROUGH . . . */

		default:
			if (gotlt == 0 || gotlt == '<') {
				if (lastsp) {
					lastsp = 0;
					*cp2++ = ' ';
				}
				*cp2++ = c;
			}
			break;
		}
	}
	*cp2 = 0;
	return (token ? --cp : (!strcmp(name, nbufp)) ? name :
	    nbufp == nbuf ? strdup(nbuf) : nbufp);
}



static void
skin_comma(char * buf)
{
    char * last_c = &buf[strlen(buf) - 1];

    while(last_c > buf && isspace((unsigned char)*last_c)) {
      *last_c = 0;
      last_c -= 1;
    }

    if (last_c > buf && *last_c == ',') {
      *last_c = 0;
    }
}

static char *
strip_hosts(char * addr)
{
    char *cp, *cp2;
    
    if ((cp = strrchr(addr,'!')) != NULL)
      cp++;
    else
      cp = addr;
      
    /*
     * Now strip off all Internet-type
     * hosts.
     */
    if ((cp2 = strchr(cp, '%')) == NULL)
      cp2 = strchr(cp, '@');
    if (cp2 != NULL)
      *cp2 = '\0';
      
    return(cp);
}


// --------------------------------------------------------------------------------
// The following code was adapted from Roam Version 3 MailBox.C.
// And now for the obligatory cute yet meaningless and cryptic comment:
// 
// This code uses deceptively brilliant methods and laborious but bug
// free techniques that King Solomon would not have approved of; I have
// thought about it quite a bit, while sipping fine wine and eating ripe
// cheese, and had good reasons, that I forget right now, so it must be
// some archane case, for even though its hard to believe that this
// algorithm works, for if you saw what I see almost badly because of it,
// you would agree, that is what confuses me about this country.
// Ditto. How true.
//
// Need to translate this to Sub-Saharan dialect of choice next Monday. MV gmg 25 May 96
// --------------------------------------------------------------------------------

// The following functions are used to check the validity of the 
// address given by the user in the to, cc, or bcc line.
// Functions:
//	parse_adr, qindex, parse_addr, oneHostWord, clearspace, clearwhitespace
// ALL IMAP4 clean

const char AT = '@';
const char GROUPEND = ';';
const char ROUTEEND = ':';
const char BKSLASH = '\\';
const char HTAB = 9;
const char SP = ' ';
const char EOL = '\n';

/*
 *	Here we parse an rfc822 address. 
 *				Bill Yeager - Stanford Univ.
 *				Feb 1, 1988.
 *
 *	The code that follows is fairly laborious but bug free.
 *					(Famous Last Words...-ED)
 */

/*
 *  Q I N D E X
 *
 *  Look for the first occurence of one of the characters in
 *  the string "lookfor" in "s".  Apply quoting
 *  conventions so atoms are not looked into, and  "domain" when non-zero
 *  causes check to made for domain literals.
 *
 */

const char DQUOTE = '"';
const char LPAREN = '(';
const char RPAREN = ')';
const char LEFTSB = '[';
const char RIGHTSB = ']';

static char *qindex(
    register char 	*s,
    char 		*lookfor,
    int			domain,
    int			*flags,
    int 		terminator)  /* true if looking for termination only */
{
  register char *lp;
  register significantData= 0;	/* flags something signficant */
  int parenCount = 0;
  
  *flags = 0;
  if (s == NULL) {
    *flags = Sdm_EC_Success;
    return s;
  }
  while (*s) {				/* the while */
    switch(*s) {			/* begin switch */
    case BKSLASH:
      s++;
      if (*s == NULL) {
	*flags = Sdm_EC_PRH_MISSINGEC;
	return NULL;
      }
      break;
    case DQUOTE:
      s++;
      if (*s == NULL) {
	*flags = Sdm_EC_PRH_MISSINGQQ;
	return NULL;
      }
      while(1)  {	/* skip over the "<string>" */
	if (*s == DQUOTE) break;
        if (*s == BKSLASH) s++;
        if(*s == NULL) {	/* "string */
	  *flags = Sdm_EC_PRH_MISSINGQQ;
	  return NULL;
	}
	s++;
      }
      significantData = 1;
      break;
    case LPAREN:	  	/* (comment) */
      s++;
      if (*s == NULL) {
	*flags = Sdm_EC_PRH_MISSINGRP;
	return NULL;
      }
      parenCount = 1;
      while(1)  {		/* skip over the "(<string>)" */
	if(*s == RPAREN && (parenCount-- == 1)) break;
	if(*s == LPAREN)
	  parenCount++;
	if(*s == BKSLASH) s++;
	if(*s == NULL) {	  		/* "string */
	  *flags = Sdm_EC_PRH_MISSINGRP;
	  return NULL;
	}
	s++;
      }
      break;
  
    case LEFTSB:
      if(domain)  {
	s++;
	if (*s == NULL) {
	  *flags = Sdm_EC_PRH_MISSINGRB;
	  return NULL;
	}
	while(1)  {
	  if (*s == RIGHTSB) break;
	  if (*s == BKSLASH) s++;
	  if(*s == NULL) {
	    *flags = Sdm_EC_PRH_MISSINGRB;
	    return NULL;
	  }
	  s++;
	}
	significantData = 1;
	break;
      }
      /*  FALLTHROUGH  */
    default:
      lp = lookfor;
      while (*lp) 
	if(*lp++ == *s) {
	  /* Here if we have found ",", then we
	   * insure some other significant chars
	   * were scanned, unless we found "'" as
	   * the first char scanned. That is taken
	   * care of in our caller's logic.
	   */
	  switch (*s) {
	  case ',':
	    if (terminator || significantData) 
	      *flags = Sdm_EC_Success;
	    else
	      *flags = Sdm_EC_PRH_NULLADDRESS;
	    break;
	  default:
	    *flags = Sdm_EC_Success;
	    break;
	  }
	  return(s);
	}
      /* Have we found something significant besides a delimiter? 
       * EG: "string", is NOT significant and
       * "string" <   is  significant
       */
      switch (*s) {
      case '\t':
      case '\n':
      case ' ':
	break;	/* Nous avons trouve' rien */
      default:
	significantData = 1;
	break;
      }
      break;
    }
    s++;
  }
  if (significantData) 
    *flags = Sdm_EC_Success;
  else
    *flags = Sdm_EC_PRH_NULLADDRESS;
  return s;
}

// This function advances past any "RFC 822" style comment
// Needed to correct parse_addr "laborious but bug free" deficiency :-)
//
static char *advancePastRfc822Comment(char *line)
{
  char* p = line;
  int commentCount = 0;

  // Basic algorithm: walk the input line one character at a time;
  // -- toss out all white space (e.g. HTAB and SP)
  // -- if a '(' is encountered, advance to closing ) allowing for nested ()s

  while (*p) {
    switch (*p) {
    case '(':
      // beginning of RFC 822 comment
      commentCount = 1;	// level is '1' open parenthesis
      p++;		// skip past it

      while (*p && commentCount) {
	switch (*p++) {
	case '\\':
	  // quote character - if character follows, skip it
	  if (*p)
	    p++;
	  continue;
	case '(':
	  // nested comment character - count it 
	  commentCount++;
	  continue;
	case ')':
	  // closing comment character - decrement count, break out if done
	  if (!--commentCount)
	    break;
	  continue;
	default:
	  // any other character in a comment is skipped
	  continue;
	}
      }

      // broke out of while - if its because we hit the end of the line AND
      // we were still inside of the comment, dont skip by - let the "parser"
      // choke on it instead

      if (!*p && commentCount)
	return(line);
      continue;

    case HTAB:
    case SP:
      // Encountered horizontal tab or space - "white space"
      p++;		// skip past character (ignore)
      continue;		// continue with next character

    default:
      // A character outside of our scope - time to return
      return(p);
    }
  }

  return(p);
}

/* Remove LWSP - stopChr is a logical end-of-line */
static char *clearwhitespace(char *line, char *stopChr)
{
  register char c;

  while (1) {
    if (line == stopChr)
      break;
    c = *line;
    if (c == HTAB || c == SP)
      ++line;
    else
      break;
  }
  return(line);
}

static char *clearspace(char *line, char *stopChr)
{
  register char c;

  while (1) {
    if (line == stopChr)
      break;
    c = *line;
    if (c == HTAB || c == SP || c == EOL) 
      ++line;
    else
      break;
  }
  return(line);
}

/* We are called with at least "<word>" and no leading white space */
static int oneHostWord(char *cp, char *stopChr) 
{
  cp = clearspace(cp, stopChr);	/* remove LWSP */
  while (*cp != HTAB && *cp != SP)	/* skip first word */
    if (*cp == '\0' || cp == stopChr)
      return 1;	/* just one word */
    else
      ++cp;
  /* found A word */
  cp = clearspace(cp, stopChr);
  if (cp == stopChr)
    return 1;
  else
    return 0;
}

static char *parse_addr(char *cp, int *found, char **aptr)
{
  char *next= cp;
  char *base= cp;

  /* Don't allow ALL WHITESPACE */
  cp = clearwhitespace(cp, NULL);
  if (*cp == '\0') {
    *found = Sdm_EC_PRH_NULLADDRESS;
    *aptr = base;
    return cp;
  }

  /* Chase the address list one at a time */
  while (next = parse_one_addr(base, found)) {
    if (*found != Sdm_EC_Success)
      break;
    else
      base = next;
  }
  cp = next;
  if (*found != Sdm_EC_Success) {  // An error of some sort
    int len;
    // Here we pass back the start of the address where
    // the error occured.
    *aptr = base;
  }
  return(cp);
}

//
// We return NULL when we've been entered with a string that is empty.
// We cannot be initially called with an ALL WHITESPACE STRING.
//
static char *parse_one_addr(char *cp, int *found)
{
  register char *dp,*ep;
  int error;
  int rabRequired= 0;
  char schar;
  int re = 0;
  int *reenter = &re;

  *found = Sdm_EC_Success;	
  cp = advancePastRfc822Comment(cp);
  if (cp == NULL || *cp == NULL) {
    // DONE, or missing field - either empty or address, or ,,
    if (cp) {
      while (--cp) {
	if (*cp == '\t' || *cp == ' ')
	  continue;
	if (*cp == ',') 
	  *found = Sdm_EC_PRH_NULLADDRESS;
	else  // Done
	  cp = NULL;
	break;
      }
    }
    return(cp);
  }

  // Scan the address for the occurance of one of the characters in the
  // quoted string - if one of the characters are found then dp returns
  // a pointer to that character; otherwise, it returns a pointer to the
  // end of the address (e.g. the \0 character)

  dp = qindex(cp, ">)]:<@,;", 1, &error, 0);
  if (error != Sdm_EC_Success) {
    *found = error;
    return(dp);
  }
	
  if (*dp == NULL)			/* This is the last char */
    schar = EOL;
  else
    schar = *dp;
		
  switch(schar)  {
  case ':':
    /*
     *  We have Group: <addresses>;
     *  Discard before colon and go again.
     */
    if (*dp == NULL) {
      *found = Sdm_EC_PRH_MISSINGGEND;
      *reenter = 0;
    } else {
      ++dp;
      *reenter = 1;
    }
    return(dp);
  case '>':
    *found = Sdm_EC_PRH_MISSINGLAB;
    return((*dp == NULL) ? dp : ++dp);
  case ']':
    *found = Sdm_EC_PRH_MISSINGLB;
    return((*dp == NULL) ? dp : ++dp);
  case ')':
    *found = Sdm_EC_PRH_MISSINGLP;
    return((*dp == NULL) ? dp : ++dp);
  case '<':			/* Possible stuff < */
    rabRequired = 1;
    if (*dp == NULL) {
      *found = Sdm_EC_PRH_MISSINGRB;
      return(dp);
    }
    break;
  case GROUPEND:		/* Addr-list end */
    *reenter = 0;
  case AT:		/* mailbox@host */
  case ',':		/* mailbox, */
  case EOL:		/* mailbox<cr> */
    /* We only break out of the switch for mailbox@field */
    if ((int)(dp - cp) > 0) {	/* NON empty field */
      if (schar != EOL) {
	switch (*dp) {
	case AT: 		/* mailbox@ */
	  if (!oneHostWord(cp, dp)) {/* check phrase out */
	    *found = Sdm_EC_PRH_ILLEGALPHRASE;
	    return dp;
	  }
	  break;			/* look for host part */
	case  ',':				/* mailbox, */
	  if (!oneHostWord(cp, dp)) {/* check phrase out */
	    // if we are at a comma separator and the preceeding character is a closing 
	    // parenthesis, implement a horrific hack to check to see if 'a (b),' was
	    // entered: scan backwards until the opening parenthesis is found, then scan
	    // back until the first whitespace character is found. Then see if only
	    // one word is present, and if so, then the address is valid. We need to 
	    // handle nested parenthesis too!
	    if ((dp-cp > 2) && (*(dp-1) == ')')) {
	      char *edp = dp;
	      int parenCount = 0;

	      while ((edp > cp) && (!((*edp == '(') && (parenCount == 1)))) {
		if (*edp == ')')
		  parenCount++;
		else if (*edp == '(')
		  parenCount--;
		edp--;
	      }
	      if (edp > cp)
		edp--;
	      if ((edp > cp) && (isspace(*edp)) && (oneHostWord(cp, edp)))
		return(++dp);
	    }
	    *found = Sdm_EC_PRH_ILLEGALPHRASE;
	    return dp;
	  }
	  return(++dp);
	default:				/* GROUPEND */
	  /* skip ";"  */
	  dp = clearwhitespace(dp+1, NULL);
	  if (*dp != NULL)	
	    if (*dp == ',') ++dp;	/* skip "'" */
	  return(dp);
	}
	break;		/* mailbox@stuff check stuff */
      } else {
	// At this point we have discovered that this address does not contain
	// any special addressing; however, qindex is implemented in such a way
	// that an address like 'a (b)' returns NULL because it conveniently scans
	// past the (b) and ignores it, yet there is more than one "word" in the
	// address, so the check to see if the user entered 'a b' (which is illegal)
	// will look identical to the user having entered 'a (b)' (which is legal).
	// Implement a hack where we scan specifically looking for this case; yes,
	// I know its awful but, as they say, C'est la vie mon ami...

	if (!oneHostWord(cp, dp)) {
	  // if we are at the end of the line and the preceeding character is a closing 
	  // parenthesis, implement a horrific hack to check to see if 'a (b)' was
	  // entered: scan backwards until the opening parenthesis is found, then scan
	  // back until the first whitespace character is found. Then see if only
	  // one word is present, and if so, then the address is valid. We need to
	  // handle nested parenthesis too!
	  if ((dp > cp) && (schar == EOL) && (*(dp-1) == ')')) {
	    char *edp = dp;
	    int parenCount = 0;

	    while ((edp > cp) && (!((*edp == '(') && (parenCount == 1)))) {
		if (*edp == ')')
		  parenCount++;
		else if (*edp == '(')
		  parenCount--;
		edp--;
	    }
	    if (edp > cp)
	      edp--;
	    if ((edp > cp) && (isspace(*edp)) && (oneHostWord(cp, edp)))
	      return(dp);
	  }
	  *found = Sdm_EC_PRH_ILLEGALPHRASE;
	}
	return(dp);			/* mailbox<EOL> */
      }
    } else 				/* EMPTY field */
      if (*dp != GROUPEND) {  
	/* Not group:; */
	*found = Sdm_EC_PRH_MISSINGFLD;
	return(dp);
      }
    /* So, we must have "group:;" */
    dp = qindex(dp, ",", 1, &error, 1);
    if (error != Sdm_EC_Success) {
      *found = error;
      return(dp);
    }
    if (*dp)
      dp++;	/* skip character */
    return(dp);
  default:
    break;
  } /* end switch */
  /*
   *	Now check after our  initial delimeter (AT or "<")
   *	and we are not at NULL ..
   */
  if (*dp != AT)  {		/* then "<" */
    cp = clearwhitespace(dp+1, NULL);
    if (*cp == NULL) {			  /* need ">" */
      *found = Sdm_EC_PRH_MISSINGRAB;
      return(cp);
    }
    if (*cp == '>') {
      *found = Sdm_EC_PRH_MISSINGADDRESS;
      return(cp+1);
    }
    /*
     *  At-sign here means route spec., ie,
     *    <phrase>"<@" <domain> ":" 
     *	          check for this @
     */
    if (*cp == AT) {		/* gobble up the <domain> ":" */
      /*
       *	Do I just gobble it or save it
       *	as the routeList?
       */
      dp = qindex(cp, ":", 1, &error, 1);
      if (error != Sdm_EC_Success) {
	*found = error;
	return(dp);
      }
      if (*dp != ':') {
	*found = Sdm_EC_PRH_BADROUTESPEC;
	if (*dp != NULL) dp++;
	return(dp);
      }
      cp = dp;
      if (*cp != NULL)
	cp++;
    }
    if (*cp == NULL) {			  /* need ">" */
      *found = Sdm_EC_PRH_MISSINGRAB;
      return(cp);
    }
    /*
     *	Look for <mailbox>
     *	Also worry about <mailbox@host> barf, ie, 
     *	Found @.
     */
    dp = qindex(cp,"@,;>", 0, &error, 1);
    if (error != Sdm_EC_Success) {
      *found = error;
      return(dp);
    }
    if (*dp == NULL) {			  /* need ">" */
      *found = Sdm_EC_PRH_MISSINGRAB;
      return(dp);
    }
    if (*dp != AT)  {	/* "<" <mailbox> ">" */
      if (*dp == '>')  {			/* "<stuff>" */
	cp = dp + 1;
	/* first REAL char after ">" */
	dp = clearwhitespace(cp, NULL);
	/* So we've found "<stuff>?" */
	if (*dp != NULL) {	/* validate  */
	  if (*reenter) {		/* "group: stuff" */
	    /* new field or end */
	    if (*dp != ';' && *dp != ',') {
	      *found = Sdm_EC_PRH_MISSINGGEND;
	      return(dp);
	    }
	    if (*dp == ';')	{	/* group end */
	      *reenter = 0;
	      dp = clearwhitespace(dp+1,NULL);
	      if (*dp != NULL && *dp == ',')
		++dp;	/* skip "," */
	      return(dp);
	    }	
	  } else
	    if (*dp != ',')	/* spurious delimiter */
	      *found = Sdm_EC_PRH_SPURIOUSSC;
	  dp++;		/* skip it */
	} 
	return(dp);
      } else {
	*found = Sdm_EC_PRH_MISSINGRAB;
	return(dp);
      }
    }
    /* So we are pointing at "@" */
  }
  /*
   *	We have the @, so host should be next. Check for nullChr.
   *
   *		The code that follows was done empirically and may not
   *		be easy to follow. For that I apologize.  Bill Yeager.
   *
   *	Skip the "@"
   */
     
  if (*++dp == NULL) {			/* "...@" missing host */
    *found = Sdm_EC_PRH_MISSINGHT;
    return(dp);
  }

  /* Check for host part */
  cp = clearwhitespace(dp, NULL);	/* cp first char after @ */
  if (*cp == NULL) {				/* Just "@   " */
    *found = Sdm_EC_PRH_MISSINGHT;
    return(cp);
  }

  /* check for the terminators */
  dp = qindex(cp, "@>,;", 1, &error, 1);
  if (error != Sdm_EC_Success) {
    *found = error;
    return(dp);
  }

  /*
   * Peut etre nous avons trouve' trop des "@" */
  if (*dp == AT) {
    *found = Sdm_EC_PRH_EXTRAATSIGN;
    return(dp+1);
  }
  ep = dp;		/* possible last char */
  if (*dp != NULL) {	/* Found a terminator from ">,;"  */
    if (*dp == '>') {			/* <stuff>? */
      if (!rabRequired) {		/* No < was present */
	*found = Sdm_EC_PRH_EXTRARAB;
	return(dp + 1);
      } else				/* found ">" */
	rabRequired = 0;
      if ((dp - cp) == 0) {	/* @> */
	*found = Sdm_EC_PRH_MISSINGHT;
	return(dp);
      }
      ep = dp + 1;			/* end pointer */
      /* Point to next non linear white space char */
      dp = advancePastRfc822Comment(ep);
    } 
  } else
    if ((dp - cp) == 0) {		/* some@? or <some@? */
      *found = Sdm_EC_PRH_MISSINGHT;	/* some@ */
      return(dp);
    } 

  /* end of the list checking  */
  if (rabRequired) {
    *found = Sdm_EC_PRH_MISSINGRAB;
    return(dp);
  }
  if (*reenter) {		/* Group: ...<?sans ;> */
    if (*dp == NULL) {
      *found = Sdm_EC_PRH_MISSINGGEND;
      *reenter = 0;
      return dp;
    } else
      if (*dp == ';') {
	*reenter = 0;
	dp = clearwhitespace(dp+1,NULL);
	if (*dp != NULL && *dp == ',')
	  ++dp;				/* skip "," */
	return(dp);
      } 
  }
  if (*dp != NULL) {
    if (*dp != ',') 	/* Only other valid terminator */
      *found = Sdm_EC_PRH_SPURIOUSSC;
    dp++;			/* skip last found terminator */
  } else			/* could be other garbage at end */
    if (*ep != NULL) {	/* See if garbage terminator */
      cp = advancePastRfc822Comment(ep);
      if (*cp != NULL) {
	*found = Sdm_EC_PRH_SPURIOUSSC;
	return(cp+1);
      }
    }
  return(dp);
}

char **
SdmMessageUtility::GetMailSendingHostNamesList()
{
  char **hostnameslist = (char **)malloc(sizeof(char*)*3);
  assert(hostnameslist);
  int i = 0;

  SdmString smtpmailservername;
  SdmMessageUtility::GetSmtpMailServerName(smtpmailservername);
  if (smtpmailservername.Length())
    hostnameslist[i++] = strdup((const char *)smtpmailservername);
  hostnameslist[i++] = strdup(SdmSystemUtility::GetHostName());
  hostnameslist[i] = 0;
  return(hostnameslist);
}

void 
SdmMessageUtility::DestroyMailSendingHostNamesList(char** hostnameslist)
{
  assert(hostnameslist);
  assert(*hostnameslist);
  char **dl = hostnameslist;
  while (*dl)
    free(*dl++);
  free(hostnameslist);
  return;
}

/*
 * 1/2/97 - esthert
 * The following was copied from preflen function in the sendmail 
 * source (file sun_compat.c).  It is used by GetRemoteMailboxHostName below.
 */
/*
 * Returns: length of second argument if it is a prefix of the
 * first argument, otherwise zero.
 */
static int
preflen(char *str, char *pref)
{
        int len;
 
        len = strlen(pref);
        if (strncmp(str, pref, len) == 0)
                return (len);
        return (0);
}


/*
 * 1/2/97 - esthert
 * The following was copied from RemoteDefault function in the sendmail 
 * source (file sun_compat.c). It checks to see if /var/mail is nfs mounted 
 * and sets the variable r_remoteMboxHost to the value of the server in 
 * this case.  Otherwise, r_remoteMboxHost is set to an empty string.
 * The function has been modified so that we store the remote mailbox hostname
 * in the static variable _remoteMailboxHostName so that it is only looked 
 * up the first time GetRemoteMailboxHostName is called.
 */
SdmErrorCode 
SdmMessageUtility::GetRemoteMailboxHostName(SdmString &r_remotemboxhostname)
{
  if (_remoteMailboxHostName == NULL) {

    /*
     * Search through mtab to see which server /var/mail
     * is mounted from.  Called when remote mode is set, but no
     * server is specified.  Deliver locally if mailbox is local.
     */
    FILE *mfp;
    struct mntent *mnt;
    struct mnttab sysvmnt;
    char *endhost;                  /* points past the colon in name */
    int bestlen = 0, bestnfs = 0;
    int len;
    char mailboxdir[256];           /* resolved symbolic link */
    char bestname[256];            /* where the name ends up */
    char linkname[256];             /* for symbolic link chasing */
    struct stat stb;

    (void) strcpy(mailboxdir, "/var/mail");
    while (1) {
      if ((len =
          readlink(mailboxdir, linkname, sizeof (linkname))) < 0)
              break;
      linkname[len] = '\0';
      (void) strcpy(mailboxdir, linkname);
    }

    stat("/var/mail/:saved", &stb);
    mfp = fopen(MNTTAB, "r");

    if (mfp == NULL)  {
      fprintf(stderr,"Unable to open mount table\n");
      return (Sdm_EC_Fail);
    }
    (void) strcpy(bestname, "");

    mnt = &sysvmnt;
    while ((getmntent(mfp, mnt)) >= 0) {
      len = preflen(mailboxdir, mnt->mnt_dir);
      if ((len != 0) && (len >= bestlen) &&
          (strcmp(mnt->mnt_type, "nfs") == 0) ) {
              bestlen = len;
              (void) strcpy(bestname, mnt->mnt_fsname);
      }
    }
    fclose(mfp);
    endhost = strchr(bestname, ':');
    if (endhost && bestlen > 4) {
      /*
       * We found a remote mount-point for /var/spool/mail --
       * save the host name. The test against "4" is because we do not
       * want to be fooled by mounting "/" or "/var" only.
       */
      *endhost = 0;
      _remoteMailboxHostName = new SdmString(bestname);
    } else {
      /*
       * No remote mounts - assume local
       */
      _remoteMailboxHostName =  new SdmString("");
    }
  }

  r_remotemboxhostname = *_remoteMailboxHostName;
  return (Sdm_EC_Success);
}


void 
SdmMessageUtility::GetMailSendingHostName(SdmString& r_mailsendinghostname)
{
  char *sentmailhostname = 0;
  SdmMailRc* mrc = SdmMailRc::GetMailRc();
  SdmError localError;

  // First, see if general override is given - if so, always use that name
  // If not, then see if an smtp server has been specified - if so, use that name
  // If not, determine the local of /var/mail and use that

  mrc->GetValue(localError, "sentmailhostname", &sentmailhostname);
  if (localError == Sdm_EC_Success && sentmailhostname) {
    r_mailsendinghostname = sentmailhostname;
    free(sentmailhostname);
  }
  else {
    SdmString smtpmailservername;

    GetSmtpMailServerName(smtpmailservername);
    if (smtpmailservername.Length()) {
      r_mailsendinghostname = smtpmailservername;
    } else {
      SdmString remotemboxhostname;
      GetRemoteMailboxHostName(remotemboxhostname);
      if (remotemboxhostname.Length()) {
	r_mailsendinghostname = remotemboxhostname;
      } else {
	r_mailsendinghostname = SdmSystemUtility::GetHostName();
      }
    }
  }

  return;
}


// Return the name of the "smtpmailserver" 
// If the server is set, a non-null string is returned that must be deallocated
// when it is no longer needed; otherwise, a null is returned

void
SdmMessageUtility::GetSmtpMailServerName(SdmString& r_smtpmailservername)
{
  char* smtpmailserver = NULL;

  SdmError localError;
  SdmMailRc *mailrc = SdmMailRc::GetMailRc();	    // Get a handle on the mailrc object for this connection
  assert(mailrc);

  // Check to see if 'smtpmailserver' mailrc variable is defined;
  // if so, use that as the first host in the list; otherwise, 
  // return NULL

  mailrc->GetValue(localError, "smtpmailserver", &smtpmailserver);
  if (localError || !smtpmailserver) {
    // Could not get the value.  try imapinboxserver variable.
     mailrc->GetValue(localError, "imapinboxserver", &smtpmailserver);
  }
  
  if (localError || !smtpmailserver) {
    // punt and return a null string
    r_smtpmailservername = "";

  } else {
    // Got the name - assign it to the string and then free it up from storage
    r_smtpmailservername =  smtpmailserver;
    free(smtpmailserver);
  }

  return;
}

// This function is used to set an internal override of the sending user's name;
// typically it is called when a remote server is used to access the user's inbox,
// in which case the login name used to access that server is used; a zero argument
// causes the internal override to be removed

void
SdmMessageUtility::SetMailSendingUserName(const char *sendingUserName)
{
  SdmMutexEntry entry(_sendingUserNameMutex);

  if (_sendingUserName) {
    free(_sendingUserName);
    _sendingUserName = 0;
  }

  _sendingUserName = sendingUserName ? strdup(sendingUserName) : 0;
}

// Get the name to be used as the 'sending users name'
// This name is used to construct the user's name for replies 
// (e.g. username is username@hostname)

void
SdmMessageUtility::GetMailSendingUserName(SdmString &r_sendingUserName)
{
  SdmMutexEntry entry(_sendingUserNameMutex);

  char *sentmailusername = 0;
  SdmMailRc* mrc = SdmMailRc::GetMailRc();
  SdmError localError;

  // First, see if general override is given - if so, always use that name
  // else if the sending user name has been set via SetMailSendingUserName,
  // then use that name (its an override from opening the user's inbox), else
  // use the current user's name from the password file

  mrc->GetValue(localError, "sentmailusername", &sentmailusername);
  if (localError == Sdm_EC_Success && sentmailusername) {
    r_sendingUserName = sentmailusername;
    free(sentmailusername);
  }
  else if (_sendingUserName) {
    r_sendingUserName = _sendingUserName;
  }
  else {
    passwd pw;
    SdmSystemUtility::GetPasswordEntry(pw);
    r_sendingUserName = pw.pw_name;
  }

  assert(r_sendingUserName.Length());
}

// I18N function, locale sensitive
// Convert and encode string
void SdmMessageUtility::EncodeHeader(char **buf, char *name, SdmBoolean attachment_flag, SdmBoolean sun_v3_mail)
{
  SdmConnection   *conn;
  LCLd    lcld = (LCLd)NULL;
  LCTd    lclt = (LCTd)NULL;
  LclError        *ret = (LclError *)NULL;
  LclCharsetSegmentSet    *segs = (LclCharsetSegmentSet *)NULL;
  char	*tmp_buf, *new_buf;
  int	s_length, add_cr;
  LctNEAttribute to_form;

  /* V3 mail or not */
  if (sun_v3_mail == Sdm_True)
    to_form = LctNOutGoingStreamForm_V3;
  else
    to_form = LctNOutGoingStreamForm;

  /* get lcld */
  conn = SdmConnection::GetConnection();
  if (!conn)
    return;

  lcld = conn->GetLocaleDescriptor();
  if (!lcld)
    return;

  /*** create tmp_buf (add field name and \n if not) ***/
  /* null string check */
  if((buf == (char **)NULL) || (*buf == (char *)NULL))
    return;
  size_t buf_length = strlen(*buf);
  if (buf_length == 0)
    return;
  if(name == (char *)NULL)
    name == "";
  size_t name_length = strlen(name);

  /* allocate tmp_buf */
  s_length = name_length + buf_length + 1;
  tmp_buf = (char *)malloc(s_length + 1);
  if(tmp_buf == (char *)NULL)
    return;

  /* build tmp_buf */
  memcpy(tmp_buf, name, name_length);
  memcpy(tmp_buf + name_length, *buf, buf_length);
  if((*buf)[buf_length - 1] != '\n'){
    tmp_buf[s_length - 1] = '\n';
    tmp_buf[s_length] = char(0);
    add_cr = 1;
  }
  else{
    tmp_buf[s_length - 1] = char(0);
    add_cr = 0;
  }

  /* create lclt */
  lclt = lct_create(lcld, LctNSourceType, attachment_flag == Sdm_True ? LctNSeparatedTaggedText : LctNMsgText,
		    tmp_buf, (char *)LctNNone, LctNSourceForm, LctNDisplayForm, 
		    LctNKeepReference, LctNKeepByReference, NULL);
  if(!lclt){
    free(tmp_buf);
    return;
  }

  /* get converted text */
  ret = lct_getvalues(lclt, to_form, LctNContentOfHeaderSegment, &segs, NULL);
  if (ret){
    if((ret->error_code == LctErrorIconvNonIdenticalConversion) || (ret->error_code == LctErrorIconvHalfDone))
      lcl_destroy_segment_set(segs);
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    free(tmp_buf);
    return;
  }
  assert(segs);

  /* conversion has been done correctly? */
  s_length = segs->seg[0].size - name_length;
  if(s_length <= 0){
    lcl_destroy_segment_set(segs);
    lct_destroy(lclt);
    free(tmp_buf);
    return;
  }

  /* copy to null-terminated string */
  new_buf = (char *)malloc(s_length + 1);
  if (new_buf == (char *)NULL){
    lcl_destroy_segment_set(segs);
    lct_destroy(lclt);
    free(tmp_buf);
    return;
  }
  memcpy(new_buf, segs->seg[0].segment + name_length, s_length);
  if(add_cr && (new_buf[s_length - 1] == '\n'))
    new_buf[s_length - 1] = (char)0;
  else
    new_buf[s_length] = (char)0;

  lcl_destroy_segment_set(segs);
  lct_destroy(lclt);

  /* replace old string to new string */
  free(*buf);
  *buf = new_buf;

  /* free tmp_buf */
  free(tmp_buf);

  return;
}

int SdmMessageUtility::CheckEightBit(char *str, int len)
{
  unsigned char	*uc_ptr = (unsigned char *)str;
  while(len){
    if(*uc_ptr & 0x80)
      return 1;
    uc_ptr++;
    len--;
  }
  return 0;
}

// I18N function, locale sensitive:
// Convert string but doesn't encode
// only return encoding parameter that should be passed to c-client
char * SdmMessageUtility::ConvertBody(size_t& r_newBodyLength, char *buf, unsigned long buf_len, char **charset, 
  int *encoding, SdmBoolean attachment, const SdmString& data_type_charset, const SdmString& user_charset, 
  SdmBoolean sun_v3_mail, SdmBoolean has_attachment)
{
  LCLd    lcld = (LCLd)NULL;
  LCTd    lclt = (LCTd)NULL;
  LclError        *ret = (LclError *)NULL;
  LclCharsetSegmentSet    *segs = (LclCharsetSegmentSet *)NULL;
  char    *body_buffer = (char *)NULL;
  char	*encoding_str = NULL;
  char	*new_buf;
  SdmConnection *conn;
  LctNEAttribute to_form;

  // Initially, set the new body length to zero so that all error returns
  // will have a zero length returned also. The length will be set only
  // when data is actually returned
  r_newBodyLength = 0;

  /* V3 mail or not */
  if (sun_v3_mail == Sdm_True){
    /* the mail has attachments or not */
    if (has_attachment == Sdm_True)
      to_form = LctNOutGoingStreamForm_V3_Attach;
    else
      to_form = LctNOutGoingStreamForm_V3;
  }
  else
    to_form = LctNOutGoingStreamForm;

  conn = SdmConnection::GetConnection();
  if (!conn)
    return NULL;
  lcld = conn->GetLocaleDescriptor();
  if (!lcld)
    return NULL;

  assert(buf[buf_len] == '\0');		// input buffer must be null terminated

  /* create lclt */
  lclt = lct_create(lcld, LctNSourceType, attachment == Sdm_True ? LctNSeparatedTaggedText : LctNMsgText,
		    LctNNone, buf, LctNSourceForm, LctNDisplayForm, LctNKeepReference, LctNKeepByReference, NULL);
  if(!lclt){
    return (char *)NULL;
  }

  /* set the data typing charset */
  lct_setvalues(lclt, LctNDisplayForm, LctNBodyCharset, 
                (const char *)data_type_charset, NULL);

  /* set the user's chosen charset */
  lct_setvalues(lclt, to_form, LctNBodyCharset, 
                (const char *)user_charset, NULL);

  /* get locale sensitive encoding */
  ret = lct_getvalues(lclt, to_form, LctNBodyEncoding, &encoding_str, NULL);
  if (ret){
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    return (char *)NULL;
  }

  /* set encoding to None */
  lct_setvalues(lclt, to_form, LctNBodyEncoding, "None", NULL);
  /* get charset and converted text */
  ret = lct_getvalues(lclt, to_form, LctNBodySegment, &segs, NULL);
  if (ret){
    if((ret->error_code == LctErrorIconvNonIdenticalConversion) || (ret->error_code == LctErrorIconvHalfDone))
      lcl_destroy_segment_set(segs);
    if (encoding_str)
      free(encoding_str);
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    return (char *)NULL;
  }

  /* determine charset */

  assert(segs);
  *charset = segs->seg[0].charset;

  /* determin encoding */
  if(encoding_str && !strcmp(encoding_str, "Default")){
    free(encoding_str);
    encoding_str == (char *)NULL;
    lct_setvalues(lclt, to_form, LctNBodyCharset, *charset, NULL);
    ret = lct_getvalues(lclt, to_form, LctNBodyCharsetEncoding, &encoding_str);
    if (ret){
      lcl_destroy_error(ret);
      lcl_destroy_segment_set(segs);
      lct_destroy(lclt);
      return (char *)NULL;
    }
  }
  if(encoding_str){
    /* ENCBINARY means Base64 in MIME mode, uuencode in SunMailtool mode */
    if(!strcasecmp(encoding_str, "Base64"))
      *encoding = ENCBINARY;
    else if(!strcasecmp(encoding_str, "Quoted-Printable"))
      *encoding = ENC8BIT;
    else if(!strcasecmp(encoding_str, "UUEncode"))
      *encoding = ENCBINARY;
    else
      *encoding = ENC7BIT;
    free(encoding_str);
  }
  else
    *encoding = ENC7BIT;

  // Note: as of the 12/2/96 release of the lcl library, 
  // lcl returns null-terminated segment (segs->seg[x].segment)
  // segs->seg[x].size is equal to the result of strlen(segs->seg[x].segment).
  // If segs->seg[x].segment is set to NULL, lcl_destroy_segment_set() call
  // would not deallocate it.

#if 0	
  new_buf = (char *)malloc(segs->seg[0].size + 1);
  if(new_buf == (char *)NULL){
    lcl_destroy_segment_set(segs);
    lct_destroy(lclt);
    return (char *)NULL;
  }

  r_newBodyLength = segs->seg[0].size;	// set the length of the returned new body contents
  memcpy(new_buf, segs->seg[0].segment, r_newBodyLength);
  new_buf[segs->seg[0].size] = (char)0;
#else
  r_newBodyLength = segs->seg[0].size;	// set the length of the returned new body contents
  new_buf = segs->seg[0].segment;
  segs->seg[0].segment = (char *)NULL;
#endif
  
  segs->seg[0].charset = (char *)NULL;
  lcl_destroy_segment_set(segs);
  lct_destroy(lclt);

  return new_buf;
}
// I18N end
