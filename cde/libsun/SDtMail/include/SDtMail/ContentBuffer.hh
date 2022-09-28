/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the Basic Character Buffer.
 

#ifndef _SDM_CONTENTBUFFER_H
#define _SDM_CONTENTBUFFER_H

#pragma ident "@(#)ContentBuffer.hh	1.8 96/12/01 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>


class SdmContentBuffer {

public:
  SdmContentBuffer();
  SdmContentBuffer(const char *string);
  SdmContentBuffer(const SdmString& string);
  virtual ~SdmContentBuffer();

  SdmContentBuffer(const SdmContentBuffer& cBuf);
  
  SdmContentBuffer& operator= (const SdmContentBuffer& cBuf);
  SdmContentBuffer& operator= (const SdmString& string);
  SdmContentBuffer& operator= (const char* string);

  SdmStringLength Length() const;
  
  SdmErrorCode GetContents(SdmError &err, SdmString &r_contents) const;

  // I18N
  static SdmErrorCode DecodeCharset(SdmError &err, char *& r_outputBp, 
                 int& r_outputLen, const char * inputBp, 
                 const unsigned long inputLen, SdmString &body_charset);

  SdmErrorCode AddEncodeType(SdmError &err, const SdmString& encodeType);
  SdmErrorCode AddEncodeType(SdmError &err, const char* encodeType);

  SdmErrorCode SetCharacterSet(SdmError &err, const SdmString& characterSet);
  SdmErrorCode GetCharacterSet(SdmError &err, SdmString& r_characterSet) const;
  SdmErrorCode SetDataTypeCharSet(SdmError &err, const SdmString& characterSet);
  SdmErrorCode GetDataTypeCharSet(SdmError &err, SdmString& r_characterSet) const;
  static SdmErrorCode GetPossibleCharacterSetList(SdmError &err, SdmStringL& r_characterSetList, const char *inputBp, SdmBoolean forSend);
  SdmErrorCode MapAgainstFile(SdmError& err, const int fd, const size_t numberOfBytes);

  static const char *Sdm_CBE_Base64;
  static const char *Sdm_CBE_QuotedPrintable;
  static const char *Sdm_CBE_Compressed;
  static const char *Sdm_CBE_UUEncoded;

protected:
  virtual SdmErrorCode Decode(SdmError &err);

  static SdmErrorCode DecodeBase64Data(SdmError &err, char *& r_outputBp,
                                       unsigned long& r_outputLen, const char * inputBp,
                                       const unsigned long inputLen);
  static SdmErrorCode DecodeQuotedPrintableData(SdmError &err, char *& r_outputBp,
                                                unsigned long& r_outputLen,
                                                const char * inputBp,
                                                const unsigned long inputLen);
  static SdmErrorCode DecodeCompressedData(SdmError &err, char *& r_outputBp,
                                           unsigned long& r_outputLen, const char * inputBp,
                                           const unsigned long inputLen);
  static SdmErrorCode DecodeUUEncodedData(SdmError &err, char *& r_outputBp,
                                          unsigned long& r_outputLen, const char * inputBp,
                                          const unsigned long inputLen);

private:
  SdmIntStrL	_DecodeSet;
  SdmString	_Data;
  SdmBoolean    _IsDecoded;
  SdmString	_CharacterSet;
  SdmString	_DataTypeCharSet;
};    

#endif
