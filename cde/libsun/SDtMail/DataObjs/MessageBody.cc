/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Body Part Class.

#pragma ident "@(#)MessageBody.cc	1.91 97/03/14 SMI"

// Include Files.
#include <iostream.h>
#include <assert.h>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageStore.hh>
#include <CtrlObjs/RepSWER.hh>
#include <SDtMail/Session.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <PortObjs/DataPort.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <Utils/TnfProbe.hh>
#include <TranObjs/ArgumentList.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <Utils/CclientUtility.hh>

// construct a new message structure 
// It should have a empty/null contents as it is to be filled in by the creator
//
SdmMessageBodyStructure::SdmMessageBodyStructure()
{
  TNF_PROBE_0 (SdmMessageBodyStructure_constructor_start,
      "SdmMessageBodyStructure",
      "sunw%start SdmMessageBodyStructure_constructor_start");

  // Note: SdmString fields are initialized by the SdmString constructor
  mbs_type = Sdm_MSTYPE_none;		// Message type: no contents yet
  mbs_encoding = Sdm_MSENC_other;	// Message encoding: unspecified
  mbs_lines = 0;				// size in lines - none
  mbs_bytes = 0; 			// size in bytes - none
  mbs_disposition = Sdm_MSDISP_not_specified; // disposition of component: unspecified
  mbs_unix_mode = 0;			// unix mode: unspecified
  mbs_is_nested_message = Sdm_False;  // by default, not a tested message.

  TNF_PROBE_0 (SdmMessageBodyStructure_constructor_end,
      "SdmMessageBodyStructure",
      "sunw%end SdmMessageBodyStructure_constructor_end");
}

// destructor
SdmMessageBodyStructure::~SdmMessageBodyStructure()
{
  TNF_PROBE_0 (SdmMessageBodyStructure_destructor_start,
      "SdmMessageBodyStructure",
      "sunw%start SdmMessageBodyStructure_destructor_start");

  // Need to deallocate any data that is owned by this object; 
  TNF_PROBE_0 (SdmMessageBodyStructure_destructor_end,
      "SdmMessageBodyStructure",
      "sunw%end SdmMessageBodyStructure_destructor_end");
}

SdmMessageBodyStructure& 
SdmMessageBodyStructure::operator=(const SdmDpMessageStructure& mstruct) 
{
  TNF_PROBE_0 (SdmMessageBodyStructure_eqoperator_start,
      "api SdmMessageBodyStructure",
      "sunw%start SdmMessageBodyStructure_eqoperator_start");

   mbs_type = mstruct.ms_type;		
   mbs_encoding = mstruct.ms_encoding;
   mbs_subtype = mstruct.ms_subtype;
   mbs_lines = mstruct.ms_lines;
   mbs_bytes = mstruct.ms_bytes;
   mbs_disposition = mstruct.ms_disposition;
   mbs_id = mstruct.ms_id;
   mbs_description = mstruct.ms_description;
   mbs_md5 = mstruct.ms_md5;
   mbs_attachment_name = mstruct.ms_attachment_name;
   mbs_unix_mode = mstruct.ms_unix_mode;
   if(mstruct.ms_ms != NULL) 
     mbs_is_nested_message = Sdm_True;
   else
     mbs_is_nested_message = Sdm_False;
     
  TNF_PROBE_0 (SdmMessageBodyStructure_eqoperator_end,
      "api SdmMessageBodyStructure",
      "sunw%end SdmMessageBodyStructure_eqoperator_end");
   return *this;
}


void 
SdmMessageBodyStructure::CopyToDpMessageStructure(SdmDpMessageStructure& mstruct) const
{
  TNF_PROBE_0 (SdmMessageBodyStructure_CopyTo_start,
      "api SdmMessageBodyStructure",
      "sunw%start SdmMessageBodyStructure_CopyTo_start");

   mstruct.ms_type = mbs_type;		
   mstruct.ms_encoding = mbs_encoding;
   mstruct.ms_subtype = mbs_subtype;
   mstruct.ms_lines = mbs_lines;
   mstruct.ms_bytes = mbs_bytes;
   mstruct.ms_disposition = mbs_disposition;
   mstruct.ms_id = mbs_id;
   mstruct.ms_description = mbs_description;
   mstruct.ms_md5 = mbs_md5;
   mstruct.ms_attachment_name = mbs_attachment_name;
   mstruct.ms_unix_mode = mbs_unix_mode;
   
  TNF_PROBE_0 (SdmMessageBodyStructure_CopyTo_end,
      "api SdmMessageBodyStructure",
      "sunw%end SdmMessageBodyStructure_CopyTo_end");
}


SdmMessageBody::SdmMessageBody(SdmError& err, SdmMessage& messg, 
      SdmDpMessageStructure &messgStruct, int bpNum)
  :SdmData(Sdm_CL_MessageBody, err), _IsDestroyingNestedMessage(Sdm_False)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageBody_constructor_start,
      "SdmMessageBody",
      "sunw%start SdmMessageBody_constructor_start");

  err = Sdm_EC_Success;

  _MessgStore = messg._ParentStore;
  assert (_MessgStore != NULL);
  SdmSession* session = _MessgStore->_ParentSession;  
  assert (session != NULL);
  
  __SdmPRIM_REGISTERENTITY(session, _MessgStore);

  _ParentMessg = &messg;
  _ParentMessgNum = _ParentMessg->_MessgNumber;
  assert (_ParentMessgNum != 0);

  _NestedMessg = NULL;
  _MessgStruct = (SdmDpMessageStructure*) &messgStruct;
  _BodyPartNum = bpNum;
  _user_charset = NULL;

  TNF_PROBE_0 (SdmMessageBody_constructor_end,
      "SdmMessageBody",
      "sunw%end SdmMessageBody_constructor_end");
}



SdmMessageBody::SdmMessageBody(const SdmMessageBody& copy) 
  :SdmData(Sdm_CL_MessageBody, SdmError())
{
  cout << "*** Error: SdmMessageBody copy constructor called\n";
  assert(Sdm_False);
}

SdmMessageBody& 
SdmMessageBody::operator=(const SdmMessageBody &rhs)
{
  cout << "*** Error: SdmMessageBody assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmMessageBody::~SdmMessageBody()
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageBody_destructor_start,
      "SdmMessageBody",
      "sunw%start SdmMessageBody_destructor_start");

  if (_NestedMessg) {
    _IsDestroyingNestedMessage = Sdm_True;
    delete _NestedMessg;  
    _IsDestroyingNestedMessage = Sdm_False;
  }

  _NestedMessg = NULL;
  // note: don't delete _MessgStruct.  it gets deleted by parent msg.
  __SdmPRIM_UNREGISTERENTITY;

  TNF_PROBE_0 (SdmMessageBody_destructor_end,
      "SdmMessageBody",
      "sunw%end SdmMessageBody_destructor_end");
}


SdmErrorCode
SdmMessageBody::GetStructure(SdmError &err, SdmMessageBodyStructure &r_struct)
{
  TNF_PROBE_0 (SdmMessageBody_GetStructure_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetStructure_start");

  err = Sdm_EC_Success;

 SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else if (_MessgStruct == NULL) {
      err = Sdm_EC_MessageStructureDataUndefined;
    } else {
      r_struct = *_MessgStruct;
      // note: we need to pass in the message body structure argument 
      // otherwise, this call will end up as an endless loop.  this is
      // because the GetMimeTypeStringFromMessage will call GetStructure
      // if the message body structure argument is not passed in.
      SdmDataTypeUtility::GetMimeTypeStringFromMessage(err, 
          r_struct.mbs_mime_content_type, *this, &r_struct);

    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetStructure_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetStructure_end");
  return err;
}
         

SdmErrorCode
SdmMessageBody::SetStructure(SdmError &err, const SdmMessageBodyStructure &bodyStruct)
{
  TNF_PROBE_0 (SdmMessageBody_GetStructure_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetStructure_start");

  err = Sdm_EC_Success;
  
 SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else if (_MessgStruct == NULL) {
      err = Sdm_EC_MessageStructureDataUndefined;
    } else {
      // save original structure in case there is an error.
      SdmMessageBodyStructure origStruct;
      origStruct = *_MessgStruct;    

      bodyStruct.CopyToDpMessageStructure(*_MessgStruct);
      _MessgStore->GetDataPort()->SetMessageBodyStructure(err, *_MessgStruct, 
        _ParentMessgNum, *(_MessgStruct->ms_component)); 

      // restore original structure if there's an error.
      if (err) {
        origStruct.CopyToDpMessageStructure(*_MessgStruct);
      }
    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetStructure_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetStructure_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::DeleteMessageBodyFromMessage(SdmError& err, SdmBoolean& r_previousState, 
  const SdmBoolean newState)
{
  TNF_PROBE_0 (SdmMessageBody_Delete_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_Delete_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->DeleteMessageBodyFromMessage(err, r_previousState,
           newState, _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }  
  }
  
  TNF_PROBE_0 (SdmMessageBody_Delete_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_Delete_end");
  return err;
}


SdmErrorCode 
SdmMessageBody::GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  TNF_PROBE_0 (SdmMessageBody_GetContents_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetContents_start");

  err = Sdm_EC_Success;

  SdmData::GetContents(err, r_contents);

  // I18N - pass the user chosen charset to the content buffer
  if (!err)
    r_contents.SetCharacterSet(err, _user_charset);

  TNF_PROBE_0 (SdmMessageBody_GetContents_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetContents_end");
  return err;
}


SdmErrorCode 
SdmMessageBody::SetContents(SdmError &err, const SdmContentBuffer &r_contents)
{
  TNF_PROBE_0 (SdmMessageBody_SetContents_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetContents_start");

  err = Sdm_EC_Success;	
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _SetContents(err, r_contents);
    }
  }
  	 
  TNF_PROBE_0 (SdmMessageBody_SetContents_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetContents_end");
  return err;
}


SdmErrorCode 
SdmMessageBody::GetMessageCacheStatus(SdmError &err, SdmBoolean &r_status)
{
  TNF_PROBE_0 (SdmMessageBody_GetMessageCacheStatus_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetMessageCacheStatus_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->GetMessageCacheStatus(err, r_status, 
         _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetMessageCacheStatus_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetMessageCacheStatus_end");
  return err;
}

#ifdef INCLUDE_UNUSED_API                            


SdmErrorCode 
SdmMessageBody::GetContents_Async(SdmError &err, const SdmServiceFunction& svf, void* clientData)
{
  TNF_PROBE_0 (SdmMessageBody_GetContents_Async_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetContents_Async_start");

  err = Sdm_EC_Success;
  
  // just call inherited method.
  SdmData::GetContents_Async(err, svf, clientData);
	 
  TNF_PROBE_0 (SdmMessageBody_GetContents_Async_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetContents_Async_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::SetContents_Async(SdmError &err, const SdmServiceFunction& svf, 
  void* clientData, const SdmContentBuffer &r_contents)
{
  TNF_PROBE_0 (SdmMessageBody_SetContents_Async_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetContents_Async_start");

  err = Sdm_EC_Success;	

  SdmData *me = (SdmData*) this;
  SdmArgumentList* args = new SdmArgumentList; 
  args->PutArg(r_contents);

  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);

  SdmRequest* reqst = new SdmRequest(args, SdmData::AOP_SetContents_1, this);
  reqst->SetServiceFunction(svfClone);

  if (me->_CollectorSession->PostRequest(err, me, reqst)) {
    // need to clean up request and arguments;
    delete reqst;    
    delete args;
    delete svfClone;
  }
  
  TNF_PROBE_0 (SdmMessageBody_SetContents_Async_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetContents_Async_end");
  return err;
}



SdmErrorCode 
SdmMessageBody::GetAttributes(SdmError &err, SdmStrStrL &r_attribute)
{
  TNF_PROBE_0 (SdmMessageBody_GetAttributes_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetAttributes_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->GetMessageAttributes(err, r_attribute, 
         _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetAttributes_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetAttributes_end");
  return err;
}


SdmErrorCode 
SdmMessageBody::GetAttribute(SdmError &err, SdmString &r_attribute, 
                            const SdmString &attribute)
{
  TNF_PROBE_0 (SdmMessageBody_GetAttribute_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetAttribute_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
       _MessgStore->GetDataPort()->GetMessageAttribute(err, r_attribute, 
         attribute, _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetAttribute_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetAttribute_end");
  return err;
} 

SdmErrorCode 
SdmMessageBody::GetAttributes(SdmError &err, SdmStrStrL &r_attributes, 
  const SdmStringL &attributes)
{
  TNF_PROBE_0 (SdmMessageBody_GetAttributes_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetAttributes_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->GetMessageAttributes(err, r_attributes, 
         attributes, _ParentMessgNum, *(_MessgStruct->ms_component));	
    }  
  }

  TNF_PROBE_0 (SdmMessageBody_GetAttributes_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetAttributes_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::GetAttributes(SdmError &err, SdmStrStrL &r_attributes, 
     const SdmMessageAttributeAbstractFlags attributes)
{
  TNF_PROBE_0 (SdmMessageBody_GetAttributes_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetAttributes_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->GetMessageAttributes(err, r_attributes, 
          attributes, _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }  
  }

  TNF_PROBE_0 (SdmMessageBody_GetAttributes_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetAttributes_end");
  return err;
}


SdmErrorCode 
SdmMessageBody::SetAttribute(SdmError &err, const SdmString &attribute, 
    const SdmString &attribute_value)
{
  TNF_PROBE_0 (SdmMessageBody_SetAttribute_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetAttribute_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->SetMessageAttribute(err, attribute,  
            attribute_value, _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }
  }  

  TNF_PROBE_0 (SdmMessageBody_SetAttribute_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetAttribute_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::SetAttribute(SdmError &err, 
   const SdmMessageAttributeAbstractFlags attribute,
  const SdmString &attribute_value)
{
  TNF_PROBE_0 (SdmMessageBody_SetAttribute_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetAttribute_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      _MessgStore->GetDataPort()->SetMessageAttribute(err, attribute, 
          attribute_value, _ParentMessgNum, *(_MessgStruct->ms_component)); 
    }
  }  

  TNF_PROBE_0 (SdmMessageBody_SetAttribute_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetAttribute_end");
  return err;
}

#endif

SdmErrorCode
SdmMessageBody::Parent(SdmError &err, SdmMessage *&r_parent)
{
  TNF_PROBE_0 (SdmMessageBody_Parent_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_Parent_start");
      
  err = Sdm_EC_Success;
  SdmMessageBody *me = (SdmMessageBody*)this;

  SdmSyncRequestEntry entry(err, *me);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      r_parent = _ParentMessg;
    }
  }

  TNF_PROBE_0 (SdmMessageBody_Parent_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_Parent_end");
   return err;
}

SdmErrorCode
SdmMessageBody::SdmMessageFactory(SdmError& err, SdmMessage *&r_nestedMessg)
{
  TNF_PROBE_0 (SdmMessageBody_SdmMessageFactory_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SdmMessageFactory_start");

  err = Sdm_EC_Success;

  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;

    // return the nested message if it's already created.      
    } else if (_NestedMessg) {
       r_nestedMessg = _NestedMessg;

    // the body represents a nesed message if it's message structure contains 
    // a nested message structure.
    } else if (_MessgStruct->ms_ms != NULL) {
      assert (_ParentMessg != NULL);
      _NestedMessg = new SdmMessage(err, *_ParentMessg, *this);
      if (err) {
        _IsDestroyingNestedMessage = Sdm_True;
        delete _NestedMessg;
        _NestedMessg = NULL;
        _IsDestroyingNestedMessage = Sdm_False;
      } else {
        r_nestedMessg = _NestedMessg;
      }
    } else {
       r_nestedMessg = NULL;
       err = Sdm_EC_Fail;
    }
  }

  TNF_PROBE_0 (SdmMessageBody_SdmMessageFactory_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SdmMessageFactory_end");
  return err;
}

// I18N - set the user chosen charset on the body part
SdmErrorCode
SdmMessageBody::SetCharacterSet(SdmError& err, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessageBody_SetCharacterSet_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetCharacterSet_start");

  _user_charset = charset;

  if (_NestedMessg)
    _NestedMessg->SetCharacterSet(err, charset);

  TNF_PROBE_0 (SdmMessageBody_SetCharacterSet_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetCharacterSet_end");
  return err;
}

// I18N - get the user chosen charset on the body part
SdmErrorCode
SdmMessageBody::GetCharacterSet(SdmError& err, SdmString& r_charSet)
const
{
  TNF_PROBE_0 (SdmMessageBody_GetCharacterSet_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetCharacterSet_start");
 
  err = Sdm_EC_Success;
  r_charSet = _user_charset;
 
  TNF_PROBE_0 (SdmMessageBody_GetCharacterSet_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetCharacterSet_end");
  return err;
}

// I18N - set the user chosen charset on the body part
SdmErrorCode
SdmMessageBody::SetDataTypeCharSet(SdmError& err, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessageBody_SetDataTypeCharSet_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_SetDataTypeCharSet_start");

  _data_type_charset = charset;

  TNF_PROBE_0 (SdmMessageBody_SetDataTypeCharSet_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_SetDataTypeCharSet_end");
  return err;
}

// I18N - get the user chosen charset on the body part
SdmErrorCode
SdmMessageBody::GetDataTypeCharSet(SdmError& err, SdmString& r_charSet)
const
{
  TNF_PROBE_0 (SdmMessageBody_GetDataTypeCharSet_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetDataTypeCharSet_start");
 
  err = Sdm_EC_Success;
  r_charSet = _data_type_charset;
 
  TNF_PROBE_0 (SdmMessageBody_GetDataTypeCharSet_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetDataTypeCharSet_end");
  return err;
}


SdmErrorCode
SdmMessageBody::GetBodyPartNumber(SdmError& err, int& bodyPartNumber)
{
  TNF_PROBE_0 (SdmMessageBody_GetBodyPartNumber_start,
      "api SdmMessageBody",
      "sunw%start SdmMessageBody_GetBodyPartNumber_start");

  err = Sdm_EC_Success;
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      bodyPartNumber = _BodyPartNum;
    }
  }

  TNF_PROBE_0 (SdmMessageBody_GetBodyPartNumber_end,
      "api SdmMessageBody",
      "sunw%end SdmMessageBody_GetBodyPartNumber_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::_GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageBody__GetContents_start,
      "SdmMessageBody",
      "sunw%start SdmMessageBody__GetContents_start");

  err = Sdm_EC_Success;
  if (_ParentMessg->IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
  } else {
    assert (_MessgStruct != NULL);
    assert (_MessgStruct->ms_component != NULL);

    // the body does not represent a nested message. return the processed
    // contents for the body.
    if (_MessgStruct->ms_ms == NULL) {
      _MessgStore->GetDataPort()->GetMessageContents(err, 
          r_contents, DPCTTYPE_processedcomponent, _ParentMessgNum, 
          *(_MessgStruct->ms_component));

    // the body is a nested message.  return the raw contents for the body.        
    } else {
      _MessgStore->GetDataPort()->GetMessageContents(
          err, r_contents, DPCTTYPE_rawfull, _ParentMessgNum, 
          *(_MessgStruct->ms_component)); 
    }
  }


  TNF_PROBE_0 (SdmMessageBody__GetContents_end,
      "SdmMessageBody",
      "sunw%end SdmMessageBody__GetContents_end");
  return err;
}

SdmErrorCode 
SdmMessageBody::_SetContents(SdmError &err, const SdmContentBuffer &contents)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageBody__SetContents_start,
      "SdmMessageBody",
      "sunw%start SdmMessageBody__SetContents_start");

  if (_ParentMessg->IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
  } else {
    assert (_MessgStruct != NULL);
    assert (_MessgStruct->ms_component != NULL);
    
    // if this is a message/rfc822 body, then we need to check that
    // the from line is in the beginning of the contents.        
    if (_MessgStruct->ms_type == Sdm_MSTYPE_message && 
        _MessgStruct->ms_subtype.Length() > 0 &&
        ::strcasecmp((const char*)_MessgStruct->ms_subtype, "rfc822") == 0)
    {
      SdmString strContents;
      if (!contents.GetContents(err, strContents)) {
        const char* p = (const char*)strContents;
      
        // SkipPastFromLine should return a different pointer
        // from the original if a from line exists. 
        if (SdmCclientUtility::SkipPastFromLine(p) == p) {
          err = Sdm_EC_MessageRFC822MissingFromLine;
        }
      }
    }

    // proceed to set the contents only if no error has occurred.
    if (!err && !_MessgStore->GetDataPort()->SetMessageContents(
        err, contents,  _ParentMessgNum, *(_MessgStruct->ms_component)))
    {
      _MessgStruct->ms_bytes = contents.Length();
      _MessgStruct->ms_cached = Sdm_True;
    } 
  }

  TNF_PROBE_0 (SdmMessageBody__SetContents_end,
      "SdmMessageBody",
      "sunw%end SdmMessageBody__SetContents_end");
  return err;  
}

void 
SdmMessageBody::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
  const SdmBoolean isCancelled)
{
  TNF_PROBE_0 (SdmMessageBody_GetEventForRequest_start,
      "SdmMessageBody",
      "sunw%start SdmMessageBody_GetEventForRequest_start");

  switch(reqst->GetOperator()) {

    case SdmData::AOP_GetContents_1:
      {
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmGetContents.error = new SdmError;
        r_event->sdmGetContents.type = Sdm_Event_GetMessageBodyContents;  
        r_event->sdmGetContents.messageNumber = _ParentMessgNum;
        r_event->sdmGetContents.bodyPartNumber = _BodyPartNum;

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
    case SdmData::AOP_SetContents_1:
      {
        SdmArgumentList* args = reqst->GetArguments();
        SdmContentBuffer contents = *((SdmContentBuffer *) args->GetArg(SdmArgumentList::ArgT_CBuf));
        delete args;  

        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmSetContents.error = new SdmError;
        r_event->sdmSetContents.type = Sdm_Event_SetMessageBodyContents;  
        r_event->sdmSetContents.messageNumber = _ParentMessgNum;
        r_event->sdmSetContents.bodyPartNumber = _BodyPartNum;

        if (!isCancelled) {
          // do the actual setting of the contents.
          int st = _SetContents(*(r_event->sdmSetContents.error), contents);

        } else {
          *(r_event->sdmSetContents.error) = Sdm_EC_Cancel;
        }
        break;
      }
    default:
      {
        SdmData::GetEventForRequest(reqst, r_event, isCancelled);
        break;
      }
 }  

  TNF_PROBE_0 (SdmMessageBody_GetEventForRequest_end,
      "SdmMessageBody",
      "sunw%end SdmMessageBody_GetEventForRequest_end");
 return;
}

 


