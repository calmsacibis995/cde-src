/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Envelope Class.

#ifndef _SDM_MESSAGEENVELOPE_H
#define _SDM_MESSAGEENVELOPE_H

#pragma ident "@(#)MessageEnvelope.hh	1.57 97/01/29 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Data.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/PortObjects.hh>

class SdmMessage;
class SdmMessageStore;

class SdmMessageEnvelope : public SdmData {
public:
  virtual SdmErrorCode GetContents(SdmError &err, SdmContentBuffer &r_contents);
  
#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
                          void* clientData);
#endif

   static SdmErrorCode DecodeHeader(SdmError &err, SdmStrStrL &r_hdr,
                                  const SdmString &hdr, 
                                  const SdmString &charset);
   static SdmErrorCode DecodeHeader(SdmError &err, SdmString &r_hdr,
                                  const SdmString &hdr, 
                                  const SdmString &charset);
          SdmErrorCode SetCharacterSet(SdmError& err, 
                                  const SdmString &charset);
          SdmErrorCode GetCharacterSet(SdmError& err, 
                                  SdmString& r_charSet) const;

          SdmErrorCode GetHeaders(SdmError &err, SdmIntStrL &r_retheader, 
                                  const SdmMessageHeaderAbstractFlags header);

          SdmErrorCode GetHeaders(SdmError &err,SdmStrStrL &r_hdr);

#ifdef INCLUDE_UNUSED_API
          SdmErrorCode GetHeader(SdmError &err, SdmStrStrL &r_hdr,
                                  const SdmString &hdr);
                                  
          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrL &r_retheader,  
                                  const SdmMessageHeaderAbstractFlags header);

          SdmErrorCode GetHeaders(SdmError &err, SdmStrStrL &r_hdrs,			
                                  const SdmStringL &hdrs);
#endif
                                  
          SdmErrorCode SetHeaders(SdmError &err, const SdmStrStrL &headers);
          
          SdmErrorCode AddXMailerHeader(SdmError &err, const SdmString& mailerPrefix,
                                        SdmBoolean replaceExistingHeader);

#ifdef INCLUDE_UNUSED_API 
          SdmErrorCode AddHeader(SdmError& err, const SdmString& hdr, 	
                                  const SdmString& value);
          SdmErrorCode RemoveHeader(SdmError& err, const SdmString& hdr);
          SdmErrorCode ReplaceHeader(SdmError& err, const SdmString& hdr, 	
                                  const SdmString& newvalue);
#endif
          
          SdmErrorCode Parent(SdmError &error, SdmMessage *&r_message);

protected:
  // Disallow construction and deletion of message envelope.  
  // Only parent message may delete this object.
  SdmMessageEnvelope(SdmError& err, SdmMessage& messg);

  virtual ~SdmMessageEnvelope();

  virtual SdmErrorCode _GetContents(SdmError &err, SdmContentBuffer &r_contents);
  virtual void         GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
                          const SdmBoolean isCancelled);
  static SdmBoolean	_HeaderNeedsDecoding(const SdmString &hdr, SdmBoolean in_c_locale);

private:

  // Disallow creating copies of SdmMessageEnvelope.
  SdmMessageEnvelope(const SdmMessageEnvelope& copy);
  SdmMessageEnvelope& operator=(const SdmMessageEnvelope &rhs);
  
  SdmMessage*             _ParentMessg;
  SdmMessageStore*        _MessgStore;
  SdmMessageNumber        _ParentMessgNum;
  SdmString               _user_charset;
  
  friend class SdmMessage;
  friend class SdmMessageEnvelopeTest;  // for testing.
  friend class SdmMessageStoreTest;
};

#endif
