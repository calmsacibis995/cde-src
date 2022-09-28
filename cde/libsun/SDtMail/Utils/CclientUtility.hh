/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the c-client utility class.

#ifndef _SDM_CCLIENTUTILITY_H
#define _SDM_CCLIENTUTILITY_H

#pragma ident "@(#)CclientUtility.hh	1.11 97/06/11 SMI"

#include <PortObjs/DataPort.hh>
#include <mail.h>

class SdmCclientUtility {

public:

  static const char * SkipPastFromLine(
    const char *s
  );

  static SdmErrorCode ConstructMessageEnvelope(
    SdmError& err,
    ENVELOPE *env,
    SdmDataPort &dataPort,
    const SdmMessageNumber msgnum,
    SdmBoolean sunAttach
    );

  static SdmErrorCode ConstructMessageBody(
    SdmError& err,
    BODY *body,
    SdmDataPort &dataPort,
    const SdmMessageNumber msgnum,
    SdmBoolean sunAttach
    );

  static void GetMD5Checksum(
    SdmString &r_digest,
    const char* buffer,
    const unsigned long length,
    SdmBoolean doCanonicalFormText
    );

  static void FreeMD5Checksum(BODY* body);   

  static SdmBoolean SdmCclientUtility::strcrlfcpy(
    char *&r_dst,		// a pointer to the destination buffer is returned here
    unsigned long &r_dstl,	// the size of the destination buffer is returned here
    const char *src,		// the source buffer to copy
    unsigned long srcl,		// the number of bytes in the source buffer
    int ensureNewlineTermination // 0 to not ensure new line termination (!text), 1 to ensure it (text)
    );

  static SdmBoolean SdmCclientUtility::strcrlfcpy_fstuff(
    char **dst, 
    unsigned long *dstl,
    char *src, 
    unsigned long srcl,
    int ensureNewlineTermination
    );

private:

  // this class should never be instantiated.  clients should
  // call static methods directly.
  SdmCclientUtility();
  ~SdmCclientUtility();
};

#endif
