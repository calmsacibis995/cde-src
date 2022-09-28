/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Message Class.

#ifndef _SDM_MESSAGE_H
#define _SDM_MESSAGE_H

#pragma ident "@(#)Message.hh	1.77 97/01/09 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Data.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/PortObjects.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/MessageUtility.hh>

class SdmMessageStore;
class SdmMessageEnvelope;
class SdmMessageBody;
class SdmDeliveryResponse;
class SdmEncodeDecodeTransformSet;
class SdmSearch;


class SdmMessage : public SdmData {

public:

  virtual ~SdmMessage();

          SdmErrorCode GetBodyCount(SdmError& err, int &r_count) const;
  
          SdmErrorCode GetFlags(SdmError &err, SdmMessageFlagAbstractFlags &r_flags);
          SdmErrorCode ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flags);
          SdmErrorCode SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags flags);

  virtual SdmErrorCode GetContents(SdmError &err, SdmContentBuffer &r_contents);
  
#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
                          void* clientData);
#endif

          SdmErrorCode Parent(SdmError &err, SdmMessageStore *&r_store);
          SdmErrorCode Parent(SdmError &err, SdmMessage *&r_message);
    
          SdmErrorCode GetMessageNumber(SdmError& err, SdmMessageNumber &r_msgnum) const;

  // returns envelope and body part for existing message.
  virtual SdmErrorCode SdmMessageEnvelopeFactory(SdmError& err, SdmMessageEnvelope *&r_envelope);
  virtual SdmErrorCode SdmMessageBodyFactory(SdmError& err, SdmMessageBody *&r_body, int bpNum);

  // returns new body part.
  virtual SdmErrorCode SdmMessageBodyFactory(SdmError& err, SdmMessageBody *&r_body, 
                          const SdmMsgStrType bodyType, const SdmString& bodySubtype);

  // Submit sends message.  
  virtual SdmErrorCode Submit(SdmError& err, SdmDeliveryResponse &r_response,
                              const SdmMsgFormat format, const SdmBoolean logMessage,
                              const SdmStringL& customHeaders);
  virtual SdmErrorCode Submit_Async(SdmError& err, const SdmServiceFunction& svf, 
                                    void* clientData, const SdmMsgFormat format,
                                    const SdmBoolean logMessage,
                                    const SdmStringL& customHeaders);

  // CommitPendingMessageChanges saves message but does not send message. 
          SdmErrorCode CommitPendingChanges(SdmError &err, SdmBoolean &r_messageRewritten,
                                            SdmBoolean includeBcc = Sdm_False);

#ifdef INCLUDE_UNUSED_API                            
          SdmErrorCode DiscardPendingChanges(SdmError &err);
                           
  // search methods
          SdmErrorCode PerformSearch(SdmError &err, SdmBoolean &r_bool, const SdmSearch &srch);
#endif

	  // I18N
          SdmErrorCode SdmMessage::SetCharacterSet(SdmError& err, 
                  const SdmString &charset);
          SdmErrorCode GetCharacterSet(SdmError& err, 
                  SdmString& r_charSet) const;

          
protected:
  // Disallow construction of message.  Only message store and message body may 
  // create messages.
  SdmMessage(SdmError& err, SdmMessageStore& mStore);
  SdmMessage(SdmError& err, SdmMessageStore& mStore, 
            const SdmMessageNumber msgnum);
  SdmMessage(SdmError& err, SdmMessageStore& mStore, 
            const SdmMessage &derivedFrom);

  // For Nested Message, has parent BPart and Messg
  SdmMessage(SdmError& err, SdmMessage& parentMessg, SdmMessageBody& parentBPart);
	   
  enum OperationList {AOP_Submit=200};

  virtual SdmErrorCode _GetContents(SdmError &err, SdmContentBuffer &r_contents);
          SdmErrorCode _GetContents(SdmError &err, SdmContentBuffer &r_contents, 
                          unsigned long startoffset, unsigned long length);

  // Submit sends message.          
  virtual SdmErrorCode _Submit(SdmError& err, SdmDeliveryResponse& r_response,
                          const SdmMsgFormat format, const SdmBoolean logMessage,
                          const SdmStringL& customHeaders);

  virtual void GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
                          const SdmBoolean isCancelled);

          SdmErrorCode SetMessageNumber(SdmError& err, const SdmMessageNumber msgnum);
          void SetExpunged(SdmBoolean isExpunged);
          
          SdmBoolean IsNested() const { return _IsNested; } 
          
  // returns boolean indicating whether message has been expunged.
          SdmBoolean   IsExpunged() const;
          
  
private:

#ifdef INCLUDE_UNUSED_API
  // for testing only
  SdmErrorCode GetContents(SdmError &err, SdmContentBuffer &r_contents,
                           unsigned long startOffset, unsigned long length);
            
  SdmErrorCode GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
                           void* clientData, unsigned long startOffset, 
                           unsigned long length);
#endif

  // Disallow creating copies of messages.
  SdmMessage(const SdmMessage& copy);
  SdmMessage& operator=(const SdmMessage &rhs);
  
  // Method for creating body parts from message structure list.
  SdmErrorCode CreateBodyParts(SdmError &err);
  SdmErrorCode RestoreOriginalMessage(SdmError &err);
  void         DestroyBodiesAndEnvelope();
  SdmErrorCode HandleTopLevelMessageDestruction(SdmError &err);

protected:
  SdmMessageEnvelope*         _Envelope; // Top Level
  SdmVector<SdmMessageBody *> _BodyPartList;
  SdmMessageStore*            _ParentStore;
  SdmMessageNumber            _MessgNumber;
  SdmDpMessageStructureL     *_MessgStructList;

  // For Nested Message
  SdmBoolean                 _IsNested;
  SdmMessage*                _ParentMessg;
  SdmMessageBody*            _ParentBody;

  // flags.
  SdmBoolean                  _IsExpunged; // is message expunged?
  SdmBoolean                  _HasSyncLock; // does thread already have sync lock?
  SdmBoolean                  _IsNew;       // is this a newly constructed message?
  SdmString                   _user_charset; // user chosen charset
  

  // SdmMessageStore is a friend because it is the object which
  // call SetExpunged and SetMessageNumber methods.
  friend class SdmMessageStore;
  
  // SdmMessageBody and SdmMessageEnvelope are friends because they access 
  // the message number and the IsExpunged method.
  friend class SdmMessageBody;
  friend class SdmMessageEnvelope;
  
  // SdmOutgoingMessage is a friend because it accesses the msg number.
  friend class SdmOutgoingMessage;
  friend class SdmOutgoingStore;

  // the following classes are friends for testing purposes.
  friend class SdmSessionTest;    
  friend class SdmMessageStoreTest;    
  friend class SdmMessageTest;    
  friend class SdmAsyncTest;    
};

#endif
