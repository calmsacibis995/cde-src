/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Data Type utility class.

#ifndef _SDM_DATATYPEUTILITY_H
#define _SDM_DATATYPEUTILITY_H

#pragma ident "@(#)DataTypeUtility.hh	1.4 96/11/05 SMI"

#include <iostream.h>
#include <pwd.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ClassIds.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Prim.hh>
#include <thread.h>

class SdmMailRc;
class SdmDataPort;
class SdmMessageBodyStructure;

class SdmDataTypeUtility {

public:

  static char *SafeDtDtsBufferToDataType(const void *buffer, const int size, const char *opt_name);
  static char *SafeDtDtsDataTypeToAttributeValue(const char *datatype, const char *attr, const char *opt_name);
  static void SafeDtDtsFreeAttributeValue(char* attr_value);

  static SdmErrorCode	GetMimeTypeStringFromMessage(SdmError& err, SdmString& r_mimeType, SdmMessageBody& messageBody, SdmMessageBodyStructure* messageBodyStructure = 0);
  static SdmErrorCode	DetermineCdeDataTypeForMessage(SdmError& err, SdmString& r_cdeDataType, SdmMessageBody& messageBody, SdmMessageBodyStructure* messageBodyStructure = 0, SdmBoolean lowBandwidthRequest = Sdm_False);
  static SdmErrorCode   DetermineMimeDataTypesForMessage(SdmError& err, SdmBoolean& r_isText, SdmMsgStrType& r_mimeType, SdmString& r_mimeSubtype, SdmMessageBody& messageBody, SdmMessageBodyStructure* messageBodyStructure = 0);
  static SdmErrorCode	SetContentTypesForMessage(SdmError& err, SdmMessageBody& messageBody);

private:

  // this class should never be instantiated.  clients should
  // call static methods directly.
  SdmDataTypeUtility();
  ~SdmDataTypeUtility();

};

#endif
