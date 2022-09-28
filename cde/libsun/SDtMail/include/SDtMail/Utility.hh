/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Utils Base class.

#ifndef _SDM_UTILITY_H
#define _SDM_UTILITY_H

#pragma ident "@(#)Utility.hh	1.42 96/09/27 SMI"

#include <iostream.h>
#include <pwd.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ClassIds.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Prim.hh>
#include <thread.h>

class SdmMailRc;

typedef unsigned long SdmObjectKey;

class SdmUtility {

public:

  static const char* SdmUtility::kFolderResourceName;
  static const char* SdmUtility::kImapFolderResourceName;


  // The expandPath method will turn a relative path, with
  // variable's into an absolute path, relative to the current
  // network node.
  //
  static SdmErrorCode ExpandAndMakeAbsolutePath(SdmError& err, SdmString& r_pathName, const SdmString& pathName, const SdmBoolean followLinks);

  static SdmErrorCode ExpandPath(SdmError &err, char *&r_absolutePath,
				 const char *relativePath, const SdmMailRc &mailRc,
				 const char *folderResourceName = SdmUtility::kFolderResourceName);
                         
  static SdmErrorCode GetRelativePath(SdmError &err, char *&r_relatePath, 
				      const char * path, const SdmMailRc &mailRc,
				      const char *folderResourceName = SdmUtility::kFolderResourceName);
                  
  static SdmBoolean   ValidObjectKey(SdmObjectKey);
  static SdmObjectKey NewObjectKey(void);
  static void         RemoveObjectKey(SdmObjectKey);
  
  static SdmString    ErrnoMessage(int systemErrorNumber);
  static void         LogError(SdmBoolean criticalError, const char *format, ...);
  static const char*  MessageCatalogLookup(int setnum, int msgnum, const char* defaultMsg);

private:

  // this class should never be instantiated.  clients should
  // call static methods directly.
  SdmUtility();
  ~SdmUtility();

  static SdmVector<SdmObjectKey>	_ValidKeys;
  static SdmObjectKey _CurrentKey;
};

#endif
