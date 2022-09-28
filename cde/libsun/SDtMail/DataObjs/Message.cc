/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Message Class.

#pragma ident "@(#)Message.cc	1.101 97/03/21 SMI"

#include <stdio.h>
#include <iostream.h>
#include <SDtMail/Session.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Event.hh>
#include <SDtMail/DeliveryResponse.hh>
#include <PortObjs/DataPort.hh>
#include <SDtMail/PortObjects.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <CtrlObjs/RepSWER.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <TranObjs/ArgumentList.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <Utils/TnfProbe.hh>
#include <DataStructs/RWLockEntry.hh>

 
SdmMessage::SdmMessage(SdmError& err, SdmMessageStore& mStore, 
        const SdmMessage &derivedFrom)
  :SdmData(Sdm_CL_Message, err), _BodyPartList(), _Envelope(NULL),
  _IsExpunged(Sdm_False), _MessgStructList(NULL), _MessgNumber(0),
  _IsNew(Sdm_True), _HasSyncLock(Sdm_False)
{  
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage_constructor_start,
      "api SdmMessage",
      "sunw%start SdmMessage_constructor_start");

  _ParentStore = &mStore;
  SdmSession* session = _ParentStore->_ParentSession;  
  assert (session != NULL);
  
  __SdmPRIM_REGISTERENTITY(session, &mStore);

  _IsNested = Sdm_False;
  _ParentMessg = NULL;
  _ParentBody = NULL;
  _user_charset = NULL;
  
  // get message number and dataport from message we are deriving from.
  SdmMessage *derivedMsg = (SdmMessage*) &derivedFrom; 
  assert (derivedMsg->_MessgNumber != 0);
  assert (derivedMsg->_ParentStore != NULL);
  assert (derivedMsg->_ParentStore->GetDataPort() != NULL);
  
  // get new message number for derived message. 
  if (!_ParentStore->GetDataPort()->CreateDerivedMessage(err, _MessgNumber, 
      *(derivedMsg->_ParentStore->GetDataPort()), derivedMsg->_MessgNumber)) 
  {
    // get message structure for new derived message.
    _MessgStructList = new SdmDpMessageStructureL;
    assert (_MessgStructList != NULL);

    // get the message structures for the derived message.
    if (!_ParentStore->GetDataPort()->GetMessageStructure(err, *_MessgStructList, 
      _MessgNumber)) 
    {
      // create body parts and envelope for message
      if (!CreateBodyParts(err)) {
        _Envelope = new SdmMessageEnvelope(err, *this);
      }
    }
  }

  TNF_PROBE_0 (SdmMessage_constructor_end,
      "api SdmMessage",
      "sunw%end SdmMessage_constructor_end");
}


SdmMessage::SdmMessage(SdmError& err, SdmMessageStore& mStore)
  :SdmData(Sdm_CL_Message, err), _BodyPartList(), _Envelope(NULL),
   _IsExpunged(Sdm_False), _MessgStructList(NULL), _MessgNumber(0),
   _IsNew(Sdm_True), _HasSyncLock(Sdm_False)
{  
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage_constructor_start,
      "api SdmMessage",
      "sunw%start SdmMessage_constructor_start");

  err = Sdm_EC_Success; 
  
  _ParentStore = &mStore;
  SdmSession* session = _ParentStore->_ParentSession;  
  assert (session != NULL);

  __SdmPRIM_REGISTERENTITY(session, &mStore);

  _IsNested = Sdm_False;
  _ParentMessg = NULL;
  _ParentBody = NULL;

  // Do we need to fill in the structure list with some initial data?
  // No.  This is this done when new body parts are created for this message.
  _MessgStructList = new SdmDpMessageStructureL;
  
  if (!_ParentStore->GetDataPort()->CreateNewMessage(err, _MessgNumber)) {
    // create new envelope for this message.
    // Note: We must do this AFTER we get the message number.
    _Envelope = new SdmMessageEnvelope(err, *this);
  }

  TNF_PROBE_0 (SdmMessage_constructor_end,
      "api SdmMessage",
      "sunw%end SdmMessage_constructor_end");
  return;
}

SdmMessage::SdmMessage(SdmError& err, SdmMessageStore& mStore, 
      const SdmMessageNumber msgnum)
  : SdmData(Sdm_CL_Message, err), _BodyPartList(), _Envelope(NULL),
   _IsExpunged(Sdm_False), _MessgStructList(NULL), _MessgNumber(0),
   _IsNew(Sdm_False), _HasSyncLock(Sdm_False)
{ 
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage_constructor_start,
      "api SdmMessage",
      "sunw%start SdmMessage_constructor_start");

  err = Sdm_EC_Success;
  
  _ParentStore = &mStore;
  SdmSession* session = _ParentStore->_ParentSession;  
  assert (session != NULL);
  
  __SdmPRIM_REGISTERENTITY(session, &mStore);

  _IsNested = Sdm_False;
  _ParentMessg = NULL;
  _ParentBody = NULL;
  _MessgNumber = msgnum;

  // create envelope and bodies for message.
  // Note:  we must do this AFTER the message number is set.
  
  // create envelope for message.
  _Envelope = new SdmMessageEnvelope(err, *this);
  if (!err) {
    // get message structures for body parts of message.
    _MessgStructList = new SdmDpMessageStructureL;
    if (!_ParentStore->GetDataPort()->GetMessageStructure(err, *_MessgStructList, _MessgNumber)) 
    {
      // get the message structures for the this message and create the body parts.
      CreateBodyParts(err);
    }
  }

  TNF_PROBE_0 (SdmMessage_constructor_end,
      "api SdmMessage",
      "sunw%end SdmMessage_constructor_end");
}


SdmMessage::SdmMessage(SdmError& err, SdmMessage& parentMessg, 
        SdmMessageBody& parentBPart)
  :SdmData(Sdm_CL_Message, err), _BodyPartList(),
  _IsExpunged(Sdm_False), _MessgStructList(NULL), _MessgNumber(0),
  _IsNew(Sdm_False), _HasSyncLock(Sdm_False)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage_constructor_start,
      "api SdmMessage",
      "sunw%start SdmMessage_constructor_start");

  err = Sdm_EC_Success;
  
  // the message structure list in the body part better not be null.
  assert (parentBPart._MessgStruct != NULL);
  assert (parentBPart._MessgStruct->ms_ms != NULL);
  
  _ParentStore = parentMessg._ParentStore;
  assert (_ParentStore != NULL);
  SdmSession* session = _ParentStore->_ParentSession;  
  assert (session != NULL);

  __SdmPRIM_REGISTERENTITY(session, _ParentStore);

  // note: envelope is null for nested messages.  envelope is only
  // defined for parent messages.
  _Envelope = NULL;
  _IsNested = Sdm_True;
  _ParentMessg = &parentMessg;
  _ParentBody = &parentBPart;
  
  // message number for nested message is the same as the 
  // parent message.
  _MessgNumber = parentMessg._MessgNumber;
  assert (_MessgNumber != 0);

  // use the message structure list from the body to create the
  // bodies of a nested message.
  _MessgStructList = _ParentBody->_MessgStruct->ms_ms;
  assert (_MessgStructList != NULL);
  CreateBodyParts(err);

  TNF_PROBE_0 (SdmMessage_constructor_end,
      "api SdmMessage",
      "sunw%end SdmMessage_constructor_end");
}


SdmMessage& 
SdmMessage::operator=(const SdmMessage &rhs)
{
  cout << "*** Error: SdmMessage assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmMessage::SdmMessage(const SdmMessage& copy) 
  :SdmData(Sdm_CL_Message, SdmError())
{
  cout << "*** Error: SdmMessage copy constructor called\n";
  assert(Sdm_False);
}

SdmMessage::~SdmMessage()
{
  TNF_PROBE_0 (SdmMessage_destructor_start,
      "api SdmMessage",
      "sunw%start SdmMessage_destructor_start");

  SdmError err;
  
  if (!_IsNested) {
    
    if (_HasSyncLock) {
      HandleTopLevelMessageDestruction(err);
    } else {
      // get sync lock before handling destruction.
      SdmSyncRequestEntry entry(err, *this);
      err = Sdm_EC_Success;    // reset error just in case it was set.
      HandleTopLevelMessageDestruction(err);
    }

  // we only allow the deletion of the nested message if the parent body is
  // deleting this message.
  } else {
    assert (_ParentBody != NULL);
    assert (_ParentBody->_IsDestroyingNestedMessage == Sdm_True);
    DestroyBodiesAndEnvelope();
  }

  __SdmPRIM_UNREGISTERENTITY

  TNF_PROBE_0 (SdmMessage_destructor_end,
      "api SdmMessage",
      "sunw%end SdmMessage_destructor_end");
}

SdmErrorCode 
SdmMessage::GetBodyCount(SdmError& err, int &r_count) const
{
  TNF_PROBE_0 (SdmMessage_GetBodyCount_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetBodyCount_start");

  err = Sdm_EC_Success;
  SdmMessage *me = (SdmMessage*)this;
  
  SdmSyncRequestEntry entry(err, *me);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      r_count = _BodyPartList.ElementCount();
    }
  }

  TNF_PROBE_0 (SdmMessage_GetBodyCount_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetBodyCount_end");
  return err;
}

SdmErrorCode 
SdmMessage::GetFlags(SdmError &err, SdmMessageFlagAbstractFlags &r_flags)
{
  TNF_PROBE_0 (SdmMessage_GetFlags_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetFlags_start");

  err = Sdm_EC_Success;
  
  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {
        _ParentStore->GetDataPort()->GetMessageFlags(err, r_flags, _MessgNumber);
      }
    }  
  }

  TNF_PROBE_0 (SdmMessage_GetFlags_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetFlags_end");
  return err;
}

SdmErrorCode 
SdmMessage::ClearFlags(SdmError &err, const SdmMessageFlagAbstractFlags flags)
{
  TNF_PROBE_0 (SdmMessage_ClearFlags_start,
      "api SdmMessage",
      "sunw%start SdmMessage_ClearFlags_start");

  err = Sdm_EC_Success;
  
  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else   {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {
        _ParentStore->GetDataPort()->SetMessageFlagValues(err, Sdm_False, flags, _MessgNumber);
      }
    }  
  }

  TNF_PROBE_0 (SdmMessage_ClearFlags_end,
      "api SdmMessage",
      "sunw%end SdmMessage_ClearFlags_end");
  return err;
}

SdmErrorCode 
SdmMessage::SetFlags(SdmError &err, const SdmMessageFlagAbstractFlags flags)
{
  TNF_PROBE_0 (SdmMessage_SetFlags_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SetFlags_start");

  err = Sdm_EC_Success;
  
  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {
        _ParentStore->GetDataPort()->SetMessageFlagValues(err, Sdm_True, flags, _MessgNumber);
      }
    }  
  }

  TNF_PROBE_0 (SdmMessage_SetFlags_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SetFlags_end");
  return err;
}

SdmErrorCode 
SdmMessage::GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  TNF_PROBE_0 (SdmMessage_GetContents_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetContents_start");

  err = Sdm_EC_Success;
  
  // just call inherited method.
  SdmData::GetContents(err, r_contents);
   
  TNF_PROBE_0 (SdmMessage_GetContents_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetContents_end");
  return err;  
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessage::GetContents(SdmError &err, SdmContentBuffer &r_contents,
    unsigned long startOffset, unsigned long length)
{
  TNF_PROBE_0 (SdmMessage_GetContents_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetContents_start");

  err = Sdm_EC_Success;
      
  // Debugs ***
  // char buffer[2000];
  // sprintf(buffer, "SdmData::GetContents Posting request. args=%d,%d\n",
  //	  (int)startOffset, (int)length);
  // cout << buffer;
  TNF_PROBE_2 (SdmMessage_GetContents_post_request_start,
      "async SdmMessage",
      "sunw%debug SdmData::GetContents Posting request. args: ",
      tnf_long, startOffset, (int)startOffset, 
      tnf_long, length, (int)length);
  // Debugs ***
  
  // Debugs ***
  // sprintf(buffer, "SdmData::GetContents calling handle method. args=%d,%d\n",
  //   (int)startOffset, (int)length);
  // cout << buffer;

  TNF_PROBE_2 (SdmMessage_GetContents_post_request_handle,
      "async SdmMessage",
      "sunw%debug SdmData::GetContents calling handle method. args: ",
      tnf_long, startOffset, (int)startOffset, 
      tnf_long, length, (int)length);
  // Debugs ***

  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    int st = _GetContents(err, r_contents, startOffset, length);
  }

  TNF_PROBE_0 (SdmMessage_GetContents_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetContents_end");
  return err;  
}

SdmErrorCode 
SdmMessage::GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
    void* clientData)
{
  TNF_PROBE_0 (SdmMessage_GetContents_Async_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetContents_Async_start");

  err = Sdm_EC_Success;
  
  SdmData::GetContents_Async(err, svf, clientData);

  TNF_PROBE_0 (SdmMessage_GetContents_Async_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetContents_Async_end");
  return err;
}

SdmErrorCode 
SdmMessage::GetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
    void* clientData, unsigned long startOffset, unsigned long length)
{
  TNF_PROBE_0 (SdmMessage_GetContents_Async_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetContents_Async_start");

  err = Sdm_EC_Success;

  SdmMessage *me = (SdmMessage*) this;
  SdmArgumentList* args = new SdmArgumentList; 
  args->PutArg(startOffset);
  args->PutArg(length);
  
  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);
  
  SdmRequest* reqst = new SdmRequest(args, SdmData::AOP_GetContents_2, this);
  reqst->SetServiceFunction(svfClone);

  // Debug ***
  // char buffer[2000];
  // sprintf(buffer, "SdmData::GetContents_Async Posting request. args=%d,%d\n",
  //   (int)startOffset, (int)length);
  // cout << buffer;
  TNF_PROBE_2 (SdmMessage_GetContents_Async_post_request,
      "async SdmMessage",
      "sunw%debug SdmData::GetContents_Async Posting request. args: ",
      tnf_long, startOffset, (int)startOffset, 
      tnf_long, length, (int)length);
  // Debug ***
  
  if (me->_CollectorSession->PostRequest(err, me, reqst)) {
    // need to clean up request and arguments;
    // need to clean up request and arguments;
    delete reqst;    
    delete args;
    delete svfClone;
  }
  
  TNF_PROBE_0 (SdmMessage_GetContents_Async_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetContents_Async_end");
  return err;
}

#endif

SdmErrorCode 
SdmMessage::Parent(SdmError &err, SdmMessageStore *&r_store)
{
  TNF_PROBE_0 (SdmMessage_Parent_start,
      "api SdmMessage",
      "sunw%start SdmMessage_Parent_start");

  err = Sdm_EC_Success;

  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      r_store = _ParentStore;
    }
  }

  TNF_PROBE_0 (SdmMessage_Parent_end,
      "api SdmMessage",
      "sunw%end SdmMessage_Parent_end");
  return err;
}

SdmErrorCode 
SdmMessage::Parent(SdmError &err, SdmMessage *&r_message) 
{
  TNF_PROBE_0 (SdmMessage_Parent_start,
      "api SdmMessage",
      "sunw%start SdmMessage_Parent_start");

  err = Sdm_EC_Success;
  
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      r_message = _ParentMessg;
    }
  }

  TNF_PROBE_0 (SdmMessage_Parent_end,
      "api SdmMessage",
      "sunw%end SdmMessage_Parent_end");
  return err;
}

SdmErrorCode 
SdmMessage::GetMessageNumber(SdmError& err, SdmMessageNumber &r_msgnum) const
{
  TNF_PROBE_0 (SdmMessage_GetMessageNumber_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetMessageNumber_start");

  err = Sdm_EC_Success;
  SdmMessage *me = (SdmMessage*)this;
  
  SdmSyncRequestEntry entry(err, *me);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else   if (_MessgNumber == 0) {
      err = Sdm_EC_Fail;
    } else {
      r_msgnum = _MessgNumber;
    }
  }

  TNF_PROBE_0 (SdmMessage_GetMessageNumber_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetMessageNumber_end");
  return err;
}

SdmErrorCode 
SdmMessage::SdmMessageEnvelopeFactory(SdmError& err, SdmMessageEnvelope *&r_envelope) 
{
  TNF_PROBE_0 (SdmMessage_SdmMessageEnvelopeFactory_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SdmMessageEnvelopeFactory_start");

  err = Sdm_EC_Success;
  
  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {
        r_envelope = _Envelope;
        _Envelope->SetCharacterSet(err, _user_charset);
      }
    }
  }

  TNF_PROBE_0 (SdmMessage_SdmMessageEnvelopeFactory_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SdmMessageEnvelopeFactory_end");
  return err;
}


SdmErrorCode
SdmMessage::SdmMessageBodyFactory(SdmError& err, SdmMessageBody *&r_body, int bpNum)
{
  TNF_PROBE_0 (SdmMessage_SdmMessageBodyFactory_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SdmMessageBodyFactory_start");

  err = Sdm_EC_Success;
  
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      // subtract one because body part numbering starts at 1.
      int index = bpNum-1; 

      if (index < 0 || index >= _BodyPartList.ElementCount()) {
        err = Sdm_EC_BadMessageBodyPartNumber;        
      } else {
        r_body = _BodyPartList[index];
        r_body->SetCharacterSet(err, _user_charset);
      }
    }
  }

  TNF_PROBE_0 (SdmMessage_SdmMessageBodyFactory_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SdmMessageBodyFactory_end");
  return err;
}

SdmErrorCode
SdmMessage::SdmMessageBodyFactory(SdmError& err, SdmMessageBody *&r_body, 
    const SdmMsgStrType bodyType, const SdmString& bodySubtype)
{
  TNF_PROBE_0 (SdmMessage_SdmMessageBodyFactory_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SdmMessageBodyFactory_start");

  err = Sdm_EC_Success; 
  
  // get message structure for new body.  we need to get a lock because we are
  // accessing the data port to get the message body.  after message structure
  // is obtained, we create the message body using it.
  //
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
      
    } else {
      SdmDpMessageStructure *r_msgstruct = new SdmDpMessageStructure;
      assert (r_msgstruct != NULL);

      if (!_IsNested) {
         _ParentStore->GetDataPort()->AddMessageBodyToMessage(err, *r_msgstruct, bodyType, bodySubtype, _MessgNumber);

      } else  {
        // if we are a nested message created from a body part, then we need
        // to call AddMessageBodyToMessages with the message component from the body.
        assert (_ParentMessg != NULL);
        assert (_ParentMessg->_MessgNumber != 0);
        assert (_ParentBody->_MessgStruct != NULL);
        assert (_ParentBody->_MessgStruct->ms_component != NULL);
        
        _ParentStore->GetDataPort()->AddMessageBodyToMessage(
            err, *r_msgstruct, bodyType, bodySubtype, _ParentMessg->_MessgNumber, 
            *(_ParentBody->_MessgStruct->ms_component)); 
      }

      // create new body with message structure and add to list.
      if (!err) {
        int count = _BodyPartList.ElementCount();
        r_body = new SdmMessageBody(err, *this, *r_msgstruct, count+1);
        if (!err) {
           _BodyPartList.AddElementToList(r_body);
           assert (_MessgStructList != NULL);
           _MessgStructList->AddElementToList(r_msgstruct);
        } 
      }

      // clean up r_msgstruct if error occurred.
      if (err) {
        delete r_msgstruct;
      }
    }
  }
  
  TNF_PROBE_0 (SdmMessage_SdmMessageBodyFactory_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SdmMessageBodyFactory_end");
  return err;
}

SdmErrorCode
SdmMessage::Submit(SdmError& err, SdmDeliveryResponse &r_response, 
  const SdmMsgFormat format, const SdmBoolean logMessage, const SdmStringL& customHeaders)
{
  TNF_PROBE_0 (SdmMessage_Submit_start,
      "api SdmMessage",
      "sunw%start SdmMessage_Submit_start");

  err = Sdm_EC_MessageNotFromOutgoingStore;

  TNF_PROBE_0 (SdmMessage_Submit_end,
      "api SdmMessage",
      "sunw%end SdmMessage_Submit_end");
  return err;  
}

SdmErrorCode 
SdmMessage::Submit_Async(SdmError& err, const SdmServiceFunction& svf, 
    void* clientData, const SdmMsgFormat format, const SdmBoolean logMessage,
    const SdmStringL& customHeaders)
{
  TNF_PROBE_0 (SdmMessage_Submit_Async_start,
      "api SdmMessage",
      "sunw%start SdmMessage_Submit_Async_start");

  err = Sdm_EC_MessageNotFromOutgoingStore;

  TNF_PROBE_0 (SdmMessage_Submit_Async_end,
      "api SdmMessage",
      "sunw%end SdmMessage_Submit_Async_end");
  return err;
}


SdmErrorCode 
SdmMessage::CommitPendingChanges(SdmError &err, SdmBoolean &r_messageRewritten,
                                 SdmBoolean includeBcc)
{
  TNF_PROBE_0 (SdmMessage_CommitPendingChanges_start,
      "api SdmMessage",
      "sunw%start SdmMessage_CommitPendingChanges_start");

  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {  
        if (!_ParentStore->GetDataPort()->CommitPendingMessageChanges(err,
           r_messageRewritten, _MessgNumber, includeBcc))
        {
          if (!_IsNew)
            RestoreOriginalMessage(err);
        }
      }
    }  
  }

  TNF_PROBE_0 (SdmMessage_CommitPendingChanges_end,
      "api SdmMessage",
      "sunw%end SdmMessage_CommitPendingChanges_end");
  return err;
}

#ifdef INCLUDE_UNUSED_API                            

SdmErrorCode 
SdmMessage::DiscardPendingChanges(SdmError &err)
{
  TNF_PROBE_0 (SdmMessage_DiscardPendingChanges_start,
      "api SdmMessage",
      "sunw%start SdmMessage_DiscardPendingChanges_start");

  if (_IsNested) {
    err = Sdm_EC_MessageIsNested;
  } else {
    SdmSyncRequestEntry entry(err, *this);
    if (!err) {
      if (IsExpunged()) {
        err = Sdm_EC_MessageExpunged;
      } else {  
        if (!_ParentStore->GetDataPort()->DiscardPendingMessageChanges(err, _MessgNumber)) {
          if (!_IsNew)
            RestoreOriginalMessage(err);
        }
      }
    }  
  }

  TNF_PROBE_0 (SdmMessage_DiscardPendingChanges_end,
      "api SdmMessage",
      "sunw%end SdmMessage_DiscardPendingChanges_end");
  return err;
}


SdmErrorCode 
SdmMessage::PerformSearch(SdmError &err, SdmBoolean &r_bool, const SdmSearch &srch)
{
  TNF_PROBE_0 (SdmMessage_PerformSearch_start,
      "api SdmMessage",
      "sunw%start SdmMessage_PerformSearch_start");
  
 SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {  
      _ParentStore->GetDataPort()->PerformSearch(err, r_bool, srch, _MessgNumber);
    }
  }  

  TNF_PROBE_0 (SdmMessage_PerformSearch_end,
      "api SdmMessage",
      "sunw%end SdmMessage_PerformSearch_end");
  return err;
}

#endif

SdmErrorCode 
SdmMessage::_GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage__GetContents_start,
      "api SdmMessage",
      "sunw%start SdmMessage__GetContents_start");

  err = Sdm_EC_Success;
  
  // Don't need to get sync lock.  we are in the request being processed.
  if (IsExpunged()) {
    err = Sdm_EC_MessageExpunged;

  } else if (!_IsNested) {  
    _ParentStore->GetDataPort()->GetMessageContents(
      err, r_contents, DPCTTYPE_rawfull, _MessgNumber); 

  } else {
    // if we are a nested message created from a body part, then we need
    // to call GetContents with the message component from the body.
    assert (_ParentMessg != NULL);
    assert (_ParentMessg->_MessgNumber != 0);
    assert (_ParentBody->_MessgStruct != NULL);
    assert (_ParentBody->_MessgStruct->ms_component != NULL);

    _ParentStore->GetDataPort()->GetMessageContents(
        err, r_contents, DPCTTYPE_rawfull, _ParentMessg->_MessgNumber, 
        *(_ParentBody->_MessgStruct->ms_component)); 
  }

  TNF_PROBE_0 (SdmMessage__GetContents_end,
      "api SdmMessage",
      "sunw%end SdmMessage__GetContents_end");
  return err;
}

SdmErrorCode 
SdmMessage::_GetContents(SdmError &err, SdmContentBuffer &r_contents,  
    unsigned long startoffset, unsigned long length)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessage__GetContents_start,
      "api SdmMessage",
      "sunw%start SdmMessage__GetContents_start");

  err = Sdm_EC_Success;

  // Don't need to get sync lock.  we are in the request being processed.  
  if (IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
  } else {

    // *** Debugs ***
    char buffer[2000];
    memset (buffer, 0, 2000);
    sprintf(buffer, "goodbye world. args=%d,%d\n", startoffset, length);  
    // *** Debugs ***

    r_contents = buffer;  
  }

  TNF_PROBE_0 (SdmMessage__GetContents_end,
      "api SdmMessage",
      "sunw%end SdmMessage__GetContents_end");
  return err;
}

SdmErrorCode
SdmMessage::_Submit(SdmError& err, SdmDeliveryResponse &r_response, 
                    const SdmMsgFormat format, const SdmBoolean logMessage,
                    const SdmStringL& customHeaders)
{
  TNF_PROBE_0 (SdmMessage__Submit_start,
      "api SdmMessage",
      "sunw%start SdmMessage__Submit_start");

  err = Sdm_EC_MessageNotFromOutgoingStore; 	
  
  TNF_PROBE_0 (SdmMessage__Submit_end,
      "api SdmMessage",
      "sunw%end SdmMessage__Submit_end");
  return err;
}

SdmErrorCode 
SdmMessage::SetMessageNumber(SdmError& err, const SdmMessageNumber msgnum)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  TNF_PROBE_0 (SdmMessage_SetMessageNumber_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SetMessageNumber_start");

  err = Sdm_EC_Success;
  
  // Don't need sync lock.  Caller should be handling the request.
  if (IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
  } else {

    // set the message number for this message and the parent
    // message number for the envelope and body parts.  check
    // for nested message in body part.
    _MessgNumber = msgnum;
    if (_Envelope) {
      _Envelope->_ParentMessgNum = _MessgNumber;
    }
    SdmVectorIterator<SdmMessageBody*> iter(&_BodyPartList);
    SdmMessageBody **body;
    while ((body = iter.NextListElement()) != NULL) {
      assert (*body != NULL);
      (*body)->_ParentMessgNum = _MessgNumber;
      if ((*body)->_NestedMessg != NULL) {
        if ((*body)->_NestedMessg->SetMessageNumber(err, msgnum)) {
          return err;
        }
      }
    }
  }

  TNF_PROBE_0 (SdmMessage_SetMessageNumber_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SetMessageNumber_end");
  return err;
}

void 
SdmMessage::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
    const SdmBoolean isCancelled)
{
  TNF_PROBE_0 (SdmMessage_GetEventForRequest_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetEventForRequest_start");

  switch(reqst->GetOperator()) {

    case SdmData::AOP_GetContents_1:
      {
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;
        r_event->sdmGetContents.error = new SdmError;
        r_event->sdmGetContents.type = Sdm_Event_GetMessageContents;
        r_event->sdmGetContents.messageNumber = _MessgNumber;
        r_event->sdmGetContents.bodyPartNumber = -1;

        if (!isCancelled) {
          r_event->sdmGetContents.buffer = new SdmContentBuffer;
          // do the actual retrieval of the contents.
          int st = _GetContents(*(r_event->sdmGetContents.error), 
                              *(r_event->sdmGetContents.buffer));

          // if the call got cancelled, set the message ptr in
          // the event to NULL and clean up the buffer.
          if (st == Sdm_EC_Cancel) {
            delete (r_event->sdmGetContents.buffer);
            r_event->sdmGetContents.buffer = NULL;
          }
        } else {
          r_event->sdmGetContents.buffer = NULL;
          *(r_event->sdmGetContents.error) = Sdm_EC_Cancel;
        }
        break;
      }
    case SdmData::AOP_GetContents_2:
      {
        SdmArgumentList* args = reqst->GetArguments();
        int startOffset = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
        int length = *((int *) args->GetArg(SdmArgumentList::ArgT_Int));
        delete args;

        // Debugs ***
        // char buffer[2000];
        // sprintf(buffer, "SdmMessage::InCallDispatchPoint called with message. server=%lu, args=%d,%d\n",
        //       (_ServingThreadant ? ((SdmPrim*)_ServingThreadant)->_ObjSignature : 0),
        //       startOffset, length);
        // cout << buffer;
        TNF_PROBE_3 (SdmMessage_GetEventForRequest_handle_request,
            "async SdmMessage",
            "sunw%debug SdmMessage::GetEventForRequest called with message. args: ",
            tnf_ulong, server, (_ServingThreadant ? ((SdmPrim*)_ServingThreadant)->_ObjSignature : 0),
            tnf_long, startOffset, (int)startOffset, 
            tnf_long, length, (int)length);
        // Debugs ***

        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;
        r_event->sdmGetContents.error = new SdmError;
        r_event->sdmGetContents.type = Sdm_Event_GetPartialContents;
        r_event->sdmGetContents.startOffset = startOffset;
        r_event->sdmGetContents.length = length;
        r_event->sdmGetContents.messageNumber = _MessgNumber;
        r_event->sdmGetContents.bodyPartNumber = -1;

        if (!isCancelled) {
          r_event->sdmGetContents.buffer = new SdmContentBuffer;
          // do the actual retrieval of the contents.
          int st = _GetContents(*(r_event->sdmGetContents.error), 
                              *(r_event->sdmGetContents.buffer), 
                              startOffset, length);
        } else {
          r_event->sdmGetContents.buffer = NULL;
          *(r_event->sdmGetContents.error) = Sdm_EC_Cancel;
        }

        break;
      }
   default:
      {
        SdmData::GetEventForRequest(reqst, r_event, isCancelled);
        break;
      }
 }  

  TNF_PROBE_0 (SdmMessage_GetEventForRequest_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetEventForRequest_end");
 return;
}


void 
SdmMessage::SetExpunged(SdmBoolean isExpunged)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  _IsExpunged = isExpunged;
}


SdmErrorCode 
SdmMessage::CreateBodyParts(SdmError &err)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  err = Sdm_EC_Success;
        
  int totalParts = _MessgStructList->ElementCount();
  for (int i=0; i< totalParts; i++) {
    // create body part.
    SdmMessageBody *body = new SdmMessageBody(err, *this, 
        *((*_MessgStructList)[i]), i+1);
    if (err) {
      TNF_PROBE_0 (SdmMessage_constructor_end,
        "api SdmMessage",
        "sunw%end SdmMessage_constructor_end");
        return err;
    }
      
    _BodyPartList.AddElementToList(body);
  }  
  return err;
}

SdmBoolean
SdmMessage::IsExpunged() const
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
  if (!_IsNested) {
    return _IsExpunged;
  } else {
    return _ParentMessg->IsExpunged();
  }
}



SdmErrorCode
SdmMessage::HandleTopLevelMessageDestruction(SdmError &err) 
{
    
  // We will continue even if an error occurred in trying to get the
  // sync request lock to avoid large memory leak!  An error would
  // only occur if the request got cancelled but we're still going
  // to clean up anyway!

  // If this message is not expunged, discard any pending changes to the message.
  // If the discard fails, that means that this is an existing message that has
  // been edited.  In this case, we commit the pending changes.  If there is still
  // an error, we just ignore it because we can't return it back to the user from the
  // destructor.
  //
  if (!IsExpunged()) {
    // Note: we call the data port directly so that we don't end up trying
    // to get a sync lock twice.  This will happen if we call our own
    // discard and commit methods.

    if (_ParentStore) {
      SdmError localError;
      SdmBoolean r_messageRewritten;
      if (_IsNew) {
        _ParentStore->GetDataPort()->DiscardPendingMessageChanges(localError, _MessgNumber);
        
      } else if (!_ParentStore->GetDataPort()->CommitPendingMessageChanges(localError, 
                      r_messageRewritten, _MessgNumber, Sdm_False))
      {
        localError = Sdm_EC_Success;     // reset error.
        _ParentStore->GetDataPort()->DiscardPendingMessageChanges(localError, _MessgNumber);
      }

      // we put the following in it's own scope so the lock is held at a minimum.
      { 
        SdmRWLockEntry entry(_ParentStore->_MessageListLock, Sdm_False);
        _ParentStore->RemoveFromMessageList(err, this);
      }
    }
  }
 
  DestroyBodiesAndEnvelope();
  
  return err;
}

void
SdmMessage::DestroyBodiesAndEnvelope()
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  // only delete the message struct list if this is the parent message.
  if (!_IsNested && _MessgStructList) {
    _MessgStructList->ClearAndDestroyAllElements();
    delete _MessgStructList;
    _MessgStructList = NULL;
  }

  // clean up envelope.
  delete _Envelope;
  _Envelope = NULL;

  // clean up body parts.
  // note don't factor this code outside the if statement.  we want
  // to have the sync request lock when we are deleting the body 
  // parts.
  int bodyCount = _BodyPartList.ElementCount();
  for (int i=0; i<bodyCount; i++) {
    delete _BodyPartList[i];
  }
  _BodyPartList.ClearAllElements();
}


SdmErrorCode
SdmMessage::RestoreOriginalMessage(SdmError &err)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //
   err = Sdm_EC_Success;

   DestroyBodiesAndEnvelope();

  // create envelope for message.
  _Envelope = new SdmMessageEnvelope(err, *this);
  if (!err) {
    // get message structures for body parts of message.
    _MessgStructList = new SdmDpMessageStructureL;
    if (!_ParentStore->GetDataPort()->GetMessageStructure(err, *_MessgStructList, _MessgNumber)) 
    {
      // get the message structures for the this message and create the body parts.
      CreateBodyParts(err);
    }
  }

  return err;
}   
   
// I18N - set the user chosen charset on the body part
SdmErrorCode
SdmMessage::SetCharacterSet(SdmError& err, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessage_SetCharacterSet_start,
      "api SdmMessage",
      "sunw%start SdmMessage_SetCharacterSet_start");
 
  _user_charset = charset;

  // Propagate the user charset choice to message envelope
  if (_Envelope)
    _Envelope->SetCharacterSet(err, _user_charset);

  // Propagate the user charset choice to message bodies
  for (int i=0; i<_BodyPartList.ElementCount(); i++) {
    _BodyPartList[i]->SetCharacterSet(err, _user_charset);
  }
 
  TNF_PROBE_0 (SdmMessage_SetCharacterSet_end,
      "api SdmMessage",
      "sunw%end SdmMessage_SetCharacterSet_end");
  return err;
}
    
// I18N - get the user chosen charset on the body part
SdmErrorCode
SdmMessage::GetCharacterSet(SdmError& err, SdmString& r_charSet)
const
{
  TNF_PROBE_0 (SdmMessage_GetCharacterSet_start,
      "api SdmMessage",
      "sunw%start SdmMessage_GetCharacterSet_start");
 
  err = Sdm_EC_Success;
  r_charSet = _user_charset;
 
  TNF_PROBE_0 (SdmMessage_GetCharacterSet_end,
      "api SdmMessage",
      "sunw%end SdmMessage_GetCharacterSet_end");
  return err;
}

