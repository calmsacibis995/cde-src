/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Body Part Class.

#ifndef _SDM_MESSAGEBODY_H
#define _SDM_MESSAGEBODY_H

#pragma ident "@(#)MessageBody.hh	1.72 97/01/09 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Data.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/PortObjects.hh>
#include <SDtMail/Time.hh>

class SdmMessage;
class SdmMessageStore;
class SdmMessageDpStructure;
class SdmMessageBodyStructure;


class SdmMessageBody : public SdmData {
public:
          SdmErrorCode GetStructure(SdmError &err, SdmMessageBodyStructure &r_struct);         
          SdmErrorCode SetStructure(SdmError &err, const SdmMessageBodyStructure &bodyStruct);         

          SdmErrorCode DeleteMessageBodyFromMessage(SdmError& err, 
                          SdmBoolean& r_previousState, const SdmBoolean newState);

  virtual SdmErrorCode GetContents(SdmError &err, SdmContentBuffer &r_contents);

  virtual SdmErrorCode SetContents(SdmError &err, const SdmContentBuffer &r_contents);

#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
                          void* clientData);
  virtual SdmErrorCode SetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
                          void* clientData, const SdmContentBuffer &r_contents);

          SdmErrorCode GetAttribute(SdmError &err, SdmString &r_attribute, 
                          const SdmString &attribute);

          SdmErrorCode GetAttributes(SdmError &err, SdmStrStrL &r_attribute);
          SdmErrorCode GetAttributes(SdmError &err, SdmStrStrL &r_attributes, 
                          const SdmStringL &attributes);
          SdmErrorCode GetAttributes(SdmError &err, SdmStrStrL &r_attributes, 
                          const SdmMessageAttributeAbstractFlags attribute);

          SdmErrorCode SetAttribute(SdmError &err, const SdmString &attribute, 
                          const SdmString &attribute_value);
          SdmErrorCode SetAttribute(SdmError &err, 
                          const SdmMessageAttributeAbstractFlags attribute,
                          const SdmString &attribute_value);

#endif
          SdmErrorCode GetMessageCacheStatus(SdmError &err, SdmBoolean &r_status);

          SdmErrorCode Parent(SdmError& err, SdmMessage *&r_parent);
  
          SdmErrorCode SdmMessageFactory(SdmError& err, SdmMessage *&r_nestedMessg);

          SdmErrorCode GetBodyPartNumber(SdmError& err, int &r_bodyPartNumber);
          SdmErrorCode SetCharacterSet(SdmError& err, const SdmString &charset);
          SdmErrorCode GetCharacterSet(SdmError& err, SdmString& r_charSet) const;
          SdmErrorCode SetDataTypeCharSet(SdmError& err, const SdmString &charset);
          SdmErrorCode GetDataTypeCharSet(SdmError& err, SdmString& r_charSet) const;


protected:
  enum OperationList{AOP_GetContents_1=400};

  // Disallow construction and deletion of message body.  
  // Only parent message may delete this object.
  SdmMessageBody(SdmError& err, SdmMessage& messg, SdmDpMessageStructure &mstruct, 
                          int bodyPartNum);
  virtual ~SdmMessageBody();

  virtual SdmErrorCode _GetContents(SdmError &err, SdmContentBuffer &r_contents);
  virtual SdmErrorCode _SetContents(SdmError &err, const SdmContentBuffer &r_contents);

  virtual void         GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
                          const SdmBoolean isCancelled);


private:

  // Disallow creating copies of SdmMessageBody.
  SdmMessageBody(const SdmMessageBody& copy);
  SdmMessageBody& operator=(const SdmMessageBody &rhs);

  SdmMessage*             _ParentMessg;
  SdmMessage*             _NestedMessg;
  SdmDpMessageStructure*  _MessgStruct;
  int                     _BodyPartNum;
  SdmMessageNumber        _ParentMessgNum;  // cache for efficiency.
  SdmMessageStore*        _MessgStore;        // cache for efficiency.
  SdmBoolean              _IsDestroyingNestedMessage; 
  SdmString               _user_charset;
  SdmString               _data_type_charset;
  
  friend class SdmMessage;
  friend class SdmOutgoingMessage;
  friend class SdmPtrVector<SdmMessageBody*>;
  friend class SdmMessageBodyTest;
  friend class SdmMessageStoreTest;
};


class SdmMessageBodyStructure {

public:
  SdmMessageBodyStructure();
  virtual ~SdmMessageBodyStructure();

public:
  // These structure parts will always be present or set appropriately

  SdmMsgStrType mbs_type;		// Message type: primary mime type (e.g. text, multipart)
  SdmMsgStrEncoding mbs_encoding;	// Message encoding: transport encoding
  SdmString mbs_subtype; 		// Message subtype: secondary mime type (e.g. plain, mixed)
  unsigned long mbs_lines;		// size in lines
  unsigned long mbs_bytes; 		// size in bytes 
  SdmMsgStrDisposition mbs_disposition; 	// component disposition: unspecified, inline, attachment
  SdmBoolean mbs_is_nested_message;  // indicates whether this body part represents a nested message.

  // These structure parts are optional and may or may not be present
  // for a particular message; they are non-zero if available

  SdmString mbs_id;			// Message identification: message/content id
  SdmString mbs_description;  		// Message description: content description
  SdmString mbs_md5;			// MD5 checksum if present
  SdmString mbs_attachment_name;	// attachment name (if present)
  unsigned short mbs_unix_mode;		// "unix mode" hint for saving to file (if non-zero)
  SdmString  mbs_mime_content_type;  // Content type: concatenation of message type and subtype.
                                     //  (eg. text/plain, multipart/mixed).

private:
 // the following assignment operator is called by SdmMessageBody.
 SdmMessageBodyStructure& operator=(const SdmDpMessageStructure&);
 void CopyToDpMessageStructure(SdmDpMessageStructure& mstruct) const;
  
 // Disallow creating copies of SdmMessageBodyStructure
 SdmMessageBodyStructure& operator=(const SdmMessageBodyStructure &rhs);
 SdmMessageBodyStructure(const SdmMessageBodyStructure &msgstr);
 
 friend class SdmMessageBody;
};


#endif
