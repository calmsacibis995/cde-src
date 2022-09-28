/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Message Utility base class.

#pragma ident "@(#)DataTypeUtility.cc	1.16 97/03/12 SMI"

// Include Files
#include <Utils/TnfProbe.hh>
#include <SDtMail/Connection.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/MessageBody.hh>
#include <Utils/LockUtility.hh>
#include <Utils/CclientUtility.hh>
#include <DataStructs/MutexEntry.hh>
#include <PortObjs/DataPort.hh>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/systeminfo.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <Dt/Dts.h>
#include <lcl/lcl.h>

#pragma weak DtDtsFreeDataType
#pragma weak DtDtsFreeDataTypeNames
#pragma weak DtDtsDataTypeToAttributeValue
#pragma weak DtDtsBufferToDataType
#pragma weak DtDtsFindAttribute
#pragma weak DtDtsFreeAttributeValue

// mutex to serialize data typing access as the data typing code is NOT MT-SAFE
static mutex_t* _dataTypeUtilityMutex = NULL;

//
// SdmDataTypeUtilityInitMutex function is used to initialize 
// the mutex in SdmDataTypeUtility before the program starts 
// execution.  The pragma belows specifies that this 
// function is an initialization function.  
//
// NOTE that this pragma is specific to Solaris 2.x only.
// 
static void SdmDataTypeUtilityInitMutex()
{
  // Initialize the single mutex that guards access to the CDE data typing machinery

  _dataTypeUtilityMutex = new mutex_t;
  int rc;
  rc = mutex_init(_dataTypeUtilityMutex, USYNC_THREAD, NULL);
  assert (rc == 0);
  assert (_dataTypeUtilityMutex != NULL);
}
#pragma init(SdmDataTypeUtilityInitMutex)

static int check_text(char *buf, size_t length, char **charset);


SdmDataTypeUtility::SdmDataTypeUtility()
{
}

SdmDataTypeUtility::~SdmDataTypeUtility()
{
}

extern "C" {
    extern Boolean DtDtsIsTrue(char *);
}

char *SdmDataTypeUtility::SafeDtDtsBufferToDataType(const void *buffer, const int size, const char *opt_name)
{
  TNF_PROBE_0 (SdmDataTypeUtility_SafeDtDtsBufferToDataType_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsBufferToDataType_start");

  assert(&DtDtsBufferToDataType);
  mutex_lock(_dataTypeUtilityMutex);
  char* result = DtDtsBufferToDataType(buffer, size, opt_name);
  mutex_unlock(_dataTypeUtilityMutex);

  // The CDE datatyping machinery has the deficiency that ANY text beginning
  // with the string "From " is typed as a mailbox.  The following code
  // checks whether a *valid* Unix "From " line is really present in this
  // case.
  if (strcasecmp(result, "DTMAIL_FILE") == 0) {
    const char *tmp = SdmCclientUtility::SkipPastFromLine((const char*)buffer);
    if (tmp == buffer) {  // No real "From " line present
      free(result);
      result = strdup("DATA");
    }
  }

  TNF_PROBE_1 (SdmDataTypeUtility_SafeDtDtsBufferToDataType_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsBufferToDataType_end",
	       tnf_string, dataType, result ? "(NIL)" : (const char *)result);

  return(result);
}

char *SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue(const char *datatype, const char *attr, const char *opt_name)
{
  TNF_PROBE_0 (SdmDataTypeUtility_SafeDtDtsDataTypeToAttributeValue_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsDataTypeToAttributeValue_start");

  assert(&DtDtsDataTypeToAttributeValue);
  mutex_lock(_dataTypeUtilityMutex);
  char* result = DtDtsDataTypeToAttributeValue(datatype, attr, opt_name);
  mutex_unlock(_dataTypeUtilityMutex);

  TNF_PROBE_1 (SdmDataTypeUtility_SafeDtDtsDataTypeToAttributeValue_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsDataTypeToAttributeValue_end",
	       tnf_string, attributeValue, result ? "(NIL)" : (const char *)result);

  return(result);
}

void SdmDataTypeUtility::SafeDtDtsFreeAttributeValue(char* attr_value)
{
  TNF_PROBE_0 (SdmDataTypeUtility_SafeDtDtsFreeAttributeValue_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsFreeAttributeValue_start");

  assert(&DtDtsFreeAttributeValue);
  mutex_lock(_dataTypeUtilityMutex);
  DtDtsFreeAttributeValue(attr_value);
  mutex_unlock(_dataTypeUtilityMutex);

  TNF_PROBE_0 (SdmDataTypeUtility_SafeDtDtsFreeAttributeValue_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SafeDtDtsFreeAttributeValue_end");

  return;
}

// Return an ascii string that represents the MIME type and subtype of a body part
// in the form: mimetype/mimesubtype; e.g. text/plain, multipart/mixed, etc.
//
SdmErrorCode SdmDataTypeUtility::GetMimeTypeStringFromMessage
  (
  SdmError& err,
  SdmString& r_mimeType,
  SdmMessageBody& messageBody,
  SdmMessageBodyStructure* messageBodyStructure
  )
{
  TNF_PROBE_0 (SdmDataTypeUtility_GetMimeTypeStringFromMessage_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_GetMimeTypeStringFromMessage_start");

  SdmMessageBodyStructure mbstruct;

  // If no message body structure passed in, get one

  if (!messageBodyStructure) {
    if (messageBody.GetStructure(err, mbstruct) != Sdm_EC_Success)
      return(err);
    messageBodyStructure = &mbstruct;
  }
  assert(messageBodyStructure);

  // Concoct ASCII mime type/subtype string from constituent parts

  SdmString mimeType;
  SdmString mimeSubtype = messageBodyStructure->mbs_subtype;

  switch (messageBodyStructure->mbs_type) {
  case Sdm_MSTYPE_none:
  case Sdm_MSTYPE_other:
  default:
    // type is not intelligible - force to text/plain
    //
    mimeType = "text/";
    mimeSubtype = "plain";
    break;
  case Sdm_MSTYPE_text:
    mimeType = "text/";
    break;
  case Sdm_MSTYPE_multipart:
    mimeType = "multipart/";
    break;
  case Sdm_MSTYPE_message:
    mimeType = "message/";
    break;
  case Sdm_MSTYPE_application:
    mimeType = "application/";
    break;
  case Sdm_MSTYPE_audio:
    mimeType = "audio/";
    break;
  case Sdm_MSTYPE_image:
    mimeType = "image/";
    break;
  case Sdm_MSTYPE_video:
    mimeType = "video/";
    break;
  }

  if (!mimeSubtype.Length()) {
    // no subtype -- force to text/plain
    mimeType = "text/";
    mimeSubtype = "plain";
  }

  mimeType += mimeSubtype;		// ascii string for mime type of this body part
  mimeType.ConvertToLowerCase();	// Make sure the string is in all lower case
  r_mimeType = mimeType;		// Assign it to the return value string passed in

  TNF_PROBE_1 (SdmDataTypeUtility_GetMimeTypeStringFromMessage_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_GetMimeTypeStringFromMessage_end",
	       tnf_string, mimeType, (const char *)r_mimeType);

  return(err);
}

// Return the CDE data type for a message body part
//
SdmErrorCode SdmDataTypeUtility::DetermineCdeDataTypeForMessage
  (
   SdmError& err, 
   SdmString& r_cdeDataType, 
   SdmMessageBody& messageBody,
   SdmMessageBodyStructure* messageBodyStructure,
   SdmBoolean lowBandwidthRequest	// Sdm_True to not fetch contents if not already fetched, else Sdm_False to always fetch (default)
   )
{
  TNF_PROBE_0 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_start");

  SdmMessageBodyStructure mbstruct;
  SdmString mimeType;

  assert(&DtDtsFindAttribute);

  // If no message body structure passed in, get one

  if (!messageBodyStructure) {
    if (messageBody.GetStructure(err, mbstruct) != Sdm_EC_Success) {
      TNF_PROBE_1 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
		   "datatype SdmDataTypeUtility",
		   "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
		   tnf_string, error, "messageBody.GetStructure failed");

      return(err);
    }
    messageBodyStructure = &mbstruct;
  }
  assert(messageBodyStructure);

  // Need to obtain the current MIME type and make it into a a lower case string of
  // the form: type/subtype

  if (SdmDataTypeUtility::GetMimeTypeStringFromMessage(err, mimeType, messageBody, messageBodyStructure) != Sdm_EC_Success) {
    TNF_PROBE_1 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
		 tnf_string, error, "GetMimeTypeStringForMessage failed");

    return(err);
  }

  // By default, the application/x-sun-shell-script mime type has 3 CDE data
  // type representations - Bourne, csh, and ksh.  Consequently, the
  // DtDtsFindAttribute call below would return 3 types, so for low
  // bandwidth, the actual typing would be based on file name alone, which
  // is often insufficient for shell scripts.  For this reason, under low
  // bandwidth, SHELL is special cased here.
  if (lowBandwidthRequest && mimeType == "application/x-sun-shell-script") {
    r_cdeDataType = "SHELL";
    return (err = Sdm_EC_Success);
  }

  char ** types;
  {
    SdmMutexEntry entry(*_dataTypeUtilityMutex);
    types = DtDtsFindAttribute(DtDTS_DA_MIME_TYPE, mimeType);
  }

  // We must have an exact 1:1 mapping between the mime type and the
  // CDE type to use this inverse mapping. If we have no hits then we
  // dont have any thing to use. If we have several hits, then we have
  // no idea which type is the correct type.

  if (types[0] != '\0' && types[1] == '\0') {
    r_cdeDataType = types[0];		// 1 type returned: use it
    mutex_lock(_dataTypeUtilityMutex);
    DtDtsFreeDataTypeNames(types);
    mutex_unlock(_dataTypeUtilityMutex);
    TNF_PROBE_2 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
		 tnf_string, message, "1:1 mapping between mime type and CDE type",
		 tnf_string, cdeDataType, (const char *)r_cdeDataType);
    return (err = Sdm_EC_Success);
  }

  {
    SdmMutexEntry entry(*_dataTypeUtilityMutex);
    DtDtsFreeDataTypeNames(types);
  }

  // We need the bits so we can type the buffer and get
  // a type for the object. This is where things can get
  // very slow for the user.

  SdmContentBuffer theContents;
  SdmString theStrContents;

  if (!lowBandwidthRequest) {
    if (messageBody.GetContents(err, theContents) != Sdm_EC_Success) {
      TNF_PROBE_1 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
		   "datatype SdmDataTypeUtility",
		   "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
		   tnf_string, error, "messageBody.GetContents failed");
      return(err);
    }
    if (theContents.GetContents(err, theStrContents) != Sdm_EC_Success) {
      TNF_PROBE_1 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
		   "datatype SdmDataTypeUtility",
		   "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
		   tnf_string, error, "theContents.GetContents failed");
      return(err);
    }
  }

  // Get the "name" for this body part
  // We have written a name pattern for text parts that will match
  // the name "text" as a TEXT part. If we have no name and we are
  // text/plain, then we will use this name rule to get the right
  // type for the part.

  SdmString messageBodyName;

  if (messageBodyStructure->mbs_attachment_name.Length())
    messageBodyName = messageBodyStructure->mbs_attachment_name;
  else if (strcasecmp(mimeType, "text/plain") == 0)
    messageBodyName = "text";

  // Ask the data typing system to return a type for the buffer contents and name

  char *type =
    SafeDtDtsBufferToDataType(theStrContents, theStrContents.Length(),
                              messageBodyName.Length() ? messageBodyName : NULL);

  if (!type)
    r_cdeDataType = "UNKNOWN";
  else {
    r_cdeDataType = type;
    SdmMutexEntry entry(*_dataTypeUtilityMutex);
    DtDtsFreeDataType(type);
  }

  TNF_PROBE_1 (SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_DetermineCdeDataTypeForMessage_end",
	       tnf_string, cdeDataType, (const char *)r_cdeDataType);

  return(err = Sdm_EC_Success);
}

// Return the MIME type and subtype for a message body part
//
SdmErrorCode SdmDataTypeUtility::DetermineMimeDataTypesForMessage
  (
   SdmError& err,
   SdmBoolean& r_isText,
   SdmMsgStrType& r_mimeType, 
   SdmString& r_mimeSubtype, 
   SdmMessageBody& messageBody,
   SdmMessageBodyStructure* messageBodyStructure
)
{
  TNF_PROBE_0 (SdmDataTypeUtility_DetermineMimeDataTypesForMessage_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_DetermineMimeDataTypesForMessage_start");

  SdmMessageBodyStructure mbstruct;

  assert(&DtDtsFindAttribute);

  // If no message body structure passed in, get one

  if (!messageBodyStructure) {
    if (messageBody.GetStructure(err, mbstruct) != Sdm_EC_Success) {
      TNF_PROBE_1 (SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end,
		   "datatype SdmDataTypeUtility",
		   "sunw%debug SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end",
		   tnf_string, error, "messageBody.GetStructure failed");
      return(err);
    }
    messageBodyStructure = &mbstruct;
  }
  assert(messageBodyStructure);

  SdmString cdeType;

  if (SdmDataTypeUtility::DetermineCdeDataTypeForMessage(err, cdeType, messageBody, messageBodyStructure, Sdm_False) != Sdm_EC_Success) {
    TNF_PROBE_1 (SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end",
		 tnf_string, error, "DetermineCdeDataTypeForMessage failed");
    return(err);
  }

  // Look it up in the data typing system. Hopefully we will
  // get a db based mime name.

  char * db_type;
  {
    SdmMutexEntry entry(*_dataTypeUtilityMutex);
    db_type = DtDtsDataTypeToAttributeValue(cdeType, DtDTS_DA_MIME_TYPE, NULL);
  }

  // See if we call this text. If so, then it will be text/plain,
  // if not then application/octet-stream

  char *text_type;
  {
    SdmMutexEntry entry(*_dataTypeUtilityMutex);
    text_type = DtDtsDataTypeToAttributeValue(cdeType, DtDTS_DA_IS_TEXT, NULL);
  }

  SdmString mimeType;

  r_isText = (text_type && strcasecmp(text_type, "true") == 0) ? Sdm_True : Sdm_False;

  if (db_type)
    mimeType = db_type;
  else {
    if (r_isText == Sdm_True)
      mimeType = "text/plain";
    else
      mimeType = "application/octet-stream";
  }

  const char *mime_type = (const char *)mimeType;

  // I18N
  if (strncmp(mime_type, "application/octet-stream", 24) == 0)
    {
      char *charset = NULL;
      int rc;
      SdmContentBuffer content_buf;
      SdmString contents;
  
      // If the data is marked as binary (that is, application/octet-stream), 
      // it might actually be multibyte character text.  
      // Call the lcl library function check_text to 
      // determine if the data is text or actually binary.

      messageBody.GetContents(err, content_buf);
      assert (!err);
      content_buf.GetContents (err, contents);
      assert (!err);
 
      // Determine if the contents can be decoded, if so, they are text
      rc = check_text ((char *)(const char *)contents,
		       contents.Length(), &charset);
      if (rc) {
	mimeType = "text/plain";
	// If the message can be decoded, set the charset on the messageBody
	// so that later it is decoded in the correct charset.
	messageBody.SetDataTypeCharSet(err, charset);
      } else {
	mimeType = "application/octet-stream";
      }

      // Set the return value
      mime_type = (const char *)mimeType;

      if (charset) free (charset);
    }
  // I18N end

  if (db_type)
    free(db_type);

  if (text_type)
    free(text_type);

  // At this point we have a mime type string; need to decode into the
  // constituent parts of mime type and subtype

  if (strncmp(mime_type, "text/", 5) == 0)
    r_mimeType = Sdm_MSTYPE_text;
  else if (strncmp(mime_type, "multipart/", 10) == 0)
    r_mimeType = Sdm_MSTYPE_multipart;
  else if (strncmp(mime_type, "message/", 8) == 0)
    r_mimeType = Sdm_MSTYPE_message;
  else if (strncmp(mime_type, "application/", 12) == 0)
    r_mimeType = Sdm_MSTYPE_application;
  else if (strncmp(mime_type, "audio/", 6) == 0)
    r_mimeType = Sdm_MSTYPE_audio;
  else if (strncmp(mime_type, "image/", 6) == 0)
    r_mimeType = Sdm_MSTYPE_image;
  else if (strncmp(mime_type, "video/", 6) == 0)
    r_mimeType = Sdm_MSTYPE_video;
  else
    r_mimeType = Sdm_MSTYPE_other;

  const char *subtypeindex = strchr(mime_type, '/');
  r_mimeSubtype = (subtypeindex ? subtypeindex+1 : "");

  TNF_PROBE_1 (SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_DetermineMimeDataTypesForMessage_end",
	       tnf_string, mimeSubtype, (const char *)r_mimeSubtype);

  return(err = Sdm_EC_Success);
}

// Set the content type parameters of a body part according to examination

SdmErrorCode SdmDataTypeUtility::SetContentTypesForMessage
  (
   SdmError& err, 
   SdmMessageBody& messageBody
  )
{
  TNF_PROBE_0 (SdmDataTypeUtility_SetContentTypesForMessage_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SetContentTypesForMessage_start");

  SdmMessageBodyStructure messageBodyStructure;
  SdmMsgStrType mimeType;
  SdmString mimeSubType;
  SdmBoolean isText;

  // Get message body for this message

  if (messageBody.GetStructure(err, messageBodyStructure) != Sdm_EC_Success) {
    TNF_PROBE_1 (SdmDataTypeUtility_SetContentTypesForMessage_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_SetContentTypesForMessage_end",
		 tnf_string, error, "messageBody.GetStructure failed");

    return(err);
  }

  // Get the mime types for this message

  if (SdmDataTypeUtility::DetermineMimeDataTypesForMessage(err, 
							   isText, 
							   messageBodyStructure.mbs_type, 
							   messageBodyStructure.mbs_subtype,
							   messageBody) != Sdm_EC_Success) {
    TNF_PROBE_1 (SdmDataTypeUtility_SetContentTypesForMessage_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_SetContentTypesForMessage_end",
		 tnf_string, error, "DetermineMimeDataTypesForMessage failed");
    return(err);
  }

  // Set the types in the message structure

  TNF_PROBE_0 (SdmDataTypeUtility_SetContentTypesForMessage_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_SetContentTypesForMessage_end");

  return(messageBody.SetStructure(err, messageBodyStructure));
}

// I18N function, locale sensitive
// guess the content is text or not.
// If the content is text, return > 0 and charset name
// If not text, return 0
// If error, return < 0
static int
check_text(char *buf, size_t length, char **charset)
{
  TNF_PROBE_0 (SdmDataTypeUtility_check_text_start,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_check_text_start");

  SdmConnection   *conn;
  LCLd    lcld = (LCLd)NULL;
  LCTd    lclt = (LCTd)NULL;
  LclError        *ret = (LclError *)NULL;
  LclContentInfo  *info = (LclContentInfo*)NULL;
  int   ret_value;
 
  /* initialize */
  *charset = (char *)NULL;
 
  /* get lcld */
  conn = SdmConnection::GetConnection();
  if (!conn) {
    TNF_PROBE_0 (SdmDataTypeUtility_check_text_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_check_text_end");
    return -1;
  }
 
  lcld = conn->GetLocaleDescriptor();
  if (!lcld) {
    TNF_PROBE_0 (SdmDataTypeUtility_check_text_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_check_text_end");
    return -1;
  }
 
  /* create lclt */
  lclt = lct_create(lcld, LctNSourceType, LctNSourceUnknown,
		    buf, length, LctNSourceForm, LctNDisplayForm,
		    LctNKeepReference, LctNKeepByReference, NULL);
  if(!lclt) {
    TNF_PROBE_0 (SdmDataTypeUtility_check_text_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_check_text_end");
    return -1;
  }
 
  /* get type */
  ret = lct_getvalues(lclt, LctNUnknownForm,
		      LctNQuerySourceType, &info, NULL);
  if(ret){
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    TNF_PROBE_0 (SdmDataTypeUtility_check_text_end,
		 "datatype SdmDataTypeUtility",
		 "sunw%debug SdmDataTypeUtility_check_text_end");
    return -1;
  }

  assert(info);
  ret_value = 0;
  if(info->type == LclContentText){
    if(info->charset_list->num > 0){
      *charset = (char *)malloc(strlen(info->charset_list->charset[0]) + 1);
      if(*charset != (char *)NULL){
        strcpy(*charset, info->charset_list->charset[0]);
        ret_value = 1; /* text */
      }
    }
  }
 
  /* destroy */
  lcl_destroy_content_info(info);
  lct_destroy(lclt);
 
  TNF_PROBE_0 (SdmDataTypeUtility_check_text_end,
	       "datatype SdmDataTypeUtility",
	       "sunw%debug SdmDataTypeUtility_check_text_end");

  return ret_value;
}

