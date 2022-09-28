/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Message Utility class.

#ifndef _SDM_MESSAGEUTILITY_H
#define _SDM_MESSAGEUTILITY_H

#pragma ident "@(#)MessageUtility.hh	1.24 97/03/14 SMI"

#include <iostream.h>
#include <pwd.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ClassIds.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Prim.hh>
#include <thread.h>

class SdmMailRc;
class SdmDataPort;

class SdmMessageUtility {

public:
  // Data types, etc. local to this class
  typedef struct {
    SdmString *hdrs;
    SdmString *body;
  } ClientData;

  static void           AddAlternates(SdmStringL &r_alternates);

  static SdmErrorCode 	AddXMailerHeader(SdmError& err, const SdmString& mailerPrefix, SdmDataPort& dataPort, 
                            SdmMessageNumber msgnum, SdmBoolean replaceExistingHeader = Sdm_True);
                            
  static SdmErrorCode	AppendMessageToFile(SdmError& err, const SdmString& fileName, 
					    const SdmString& theHeaders, const SdmString& theBody);

  static SdmErrorCode	AppendMessageToFile(SdmError& err, const char *fileName, 
					    const SdmString& theHeaders, const SdmString& theBody)
                        { return(AppendMessageToFile(err, SdmString(fileName), theHeaders, theBody)); }

  static int		CheckEightBit(char *str, int len);

  static void		CompressAddressList(SdmStringL& r_addressList, const SdmStringL& srcAddressList);

  static void		ConvertAddressListToHeader(SdmString& r_header, const SdmStringL& addressList);

  static char*		ConvertBody(size_t& r_newBodyLength, char *buf, unsigned long buf_len, char **charset, 
                        int *encoding, SdmBoolean attachment, const SdmString& data_type_charset, 
                        const SdmString& user_charset, SdmBoolean sun_v3_mail, SdmBoolean has_attachment);

  static void		ConvertHeadersToAddressList(SdmStringL& r_addressList, const SdmString& srcHeader);

  static SdmErrorCode	ConstructWholeMessage(SdmError& err, SdmString& r_theHeaders, SdmString& r_theBody,
					       SdmDataPort& dataPort, const SdmMessageNumber msgnum,
					       SdmBoolean returnReceipt, SdmBoolean includeBcc, SdmBoolean resolveAddresses);

  static void		DestroyMailSendingHostNamesList(char** hostnameslist);

  static void		EncodeHeader(char **buf, char *name, SdmBoolean attachment_flag, SdmBoolean sun_v3_mail);

  static void		ExpandRFC822LocalAddressAliases(SdmStringL& addressList);

  static void		ExtractFolderNamesFromAddressList(SdmStringL& r_filenamelist, SdmStringL& srcAddressList);

  static void		GenerateUnixFromLine(SdmString& r_fromLine);

  static void		GetMailSendingHostName(SdmString& r_mailsendinghostname);

  static char**		GetMailSendingHostNamesList();

  static void		GetSmtpMailServerName(SdmString& r_smtpmailservername);

  static SdmErrorCode 	GetRemoteMailboxHostName(SdmString &r_remotemboxhostname);

  static void 		GetMailSendingUserName(SdmString &r_sendingusername);

  static void		InsertGhostCommasInHeader(SdmString& destHeader, const SdmString& srcHeader);

  static SdmErrorCode 	MakeReplyHeader(SdmError & error, SdmString & r_headerValue,
					SdmMessageHeaderAbstractFlags headers, SdmMessageEnvelope& env, SdmBoolean replyAll);

  static void		MergeHeader(SdmString& r_allHeaders, const SdmString& srcHeader);

  static void		MergeHeaders(SdmString& r_allHeaders, const SdmStrStrL& srcHeaders);

  static SdmErrorCode	ParseRFC822Headers(SdmError& err, int& r_errorIndex, const SdmString& srcHeader);

  static void		RemoveFolderNamesFromAddressList(SdmStringL& srcAddressList);

  static SdmErrorCode	RemoveMessageStoreIfEmpty(SdmError& err, const SdmString& messageStoreName);

  static void		ReplaceHeaders(const char* headerName, SdmStrStrL& hdrs, SdmString headerValue);

  static SdmErrorCode	ResolveAddressesInHeaders(SdmError& err, SdmStrStrL& hdrs);

  static SdmErrorCode	RewriteAddressHeader(SdmError& err, const char* headerName, 
					     const SdmStringL addressList, SdmDataPort& dataPort, 
					     const SdmMessageNumber msgnum);

  static void		SetMailSendingUserName(const char *sendingUserName);

private:

  static SdmString*	_remoteMailboxHostName;
  static char*		_sendingUserName;
  static mutex_t	_sendingUserNameMutex;
  
  // this class should never be instantiated.  clients should
  // call static methods directly.
  SdmMessageUtility();
  ~SdmMessageUtility();

};

#endif
