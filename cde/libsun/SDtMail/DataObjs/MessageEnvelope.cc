/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Envelope Class.

#pragma ident "@(#)MessageEnvelope.cc	1.74 97/01/29 SMI"

// Include Files.
#include <iostream.h>
#include <assert.h>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/MessageStore.hh>
#include <CtrlObjs/RepSWER.hh>
#include <SDtMail/Session.hh>
#include <TranObjs/SyncRequestEntry.hh>
#include <PortObjs/DataPort.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <SDtMail/Connection.hh>
#include <SDtMail/SystemUtility.hh>
#include <Utils/TnfProbe.hh>
// I18N - include the lcl library
#include <lcl/lcl.h>


SdmMessageEnvelope::SdmMessageEnvelope(SdmError& err, SdmMessage& messg)
  :SdmData(Sdm_CL_MessageEnvelope, err)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageEnvelope_constructor_start,
      "SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_constructor_start");

  err = Sdm_EC_Success;
  
  _ParentMessg = &messg;
  _ParentMessgNum = messg._MessgNumber;
  assert (_ParentMessgNum != 0);
  _user_charset = NULL;

  _MessgStore = _ParentMessg->_ParentStore;
  assert (_MessgStore != NULL);
  SdmSession* session = _MessgStore->_ParentSession;  
  assert (session != NULL);
  
  __SdmPRIM_REGISTERENTITY(session, _MessgStore);
 
  TNF_PROBE_0 (SdmMessageEnvelope_constructor_end,
      "SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_constructor_end");
}


SdmMessageEnvelope& 
SdmMessageEnvelope::operator=(const SdmMessageEnvelope &rhs)
{
  cout << "*** Error: SdmMessageEnvelope assignment operator called\n";
  assert(Sdm_False);
  return *this;
}


SdmMessageEnvelope::SdmMessageEnvelope(const SdmMessageEnvelope& copy) 
  :SdmData(Sdm_CL_MessageEnvelope, SdmError())
{
  cout << "*** Error: SdmMessageEnvelope copy constructor called\n";
  assert(Sdm_False);
}

SdmMessageEnvelope::~SdmMessageEnvelope()
{
  TNF_PROBE_0 (SdmMessageEnvelope_destructor_start,
      "SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_destructor_start");

  __SdmPRIM_UNREGISTERENTITY;

  TNF_PROBE_0 (SdmMessageEnvelope_destructor_end,
      "SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_destructor_end");
}


SdmErrorCode 
SdmMessageEnvelope::GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetContents_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetContents_start");

  err = Sdm_EC_Success;
  
  SdmData::GetContents(err, r_contents);

  TNF_PROBE_0 (SdmMessageEnvelope_GetContents_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetContents_end");
  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageEnvelope::GetContents_Async(SdmError &err, const SdmServiceFunction& svf, void* clientData)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetContents_Async_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetContents_Async_start");

  err = Sdm_EC_Success;
  
  // just call inherited method.
  SdmData::GetContents_Async(err, svf, clientData);
  
  TNF_PROBE_0 (SdmMessageEnvelope_GetContents_Async_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetContents_Async_end");
  return err;
}

#endif

// I18N
// This method is passed a header that may need decoding, and an
// indication of whether or not we are running in the "C" 7-bit ascii
// locale. It returns Sdm_True if the header should be processed by
// the lcl library, and Sdm_False if the header can be used
// as-is. This is done so that the overhead of decoding headers can be
// minimized in situtations where it is not required.
//

SdmBoolean SdmMessageEnvelope::_HeaderNeedsDecoding(const SdmString &hdr, SdmBoolean in_c_locale)
{
  const unsigned char* p = (const unsigned char*)((const char*) hdr);
  unsigned char c;

  // If we are not in the "C" locale, then we must always decode headers as some
  // encodings (such as ISO-2022-7) are NOT us-ascii, NOT encoded, and contain only
  // 7-bit characters in the printable Ascii range.. fooey!
  if (!in_c_locale)
    return(Sdm_True);

  // Ok, in the "C" locale - can assume US-Ascii for all non-encoded headers; if 
  // the header contains only printable US-Ascii 7-bit characters and the RFC 1522
  // MIME encoding sequence =? is not found, the header does not need encoding

  while (c = *p++) {
    if ((c >= 0x7f) || (c < 0x20) || (c == '=' && *p == '?')) {
	return(Sdm_True);
    }
  }

  // Just plain 7-bit US-Ascii - done decode

  return(Sdm_False);
}

//
// I18N
// The user must pass in the value of a header.  A list of converted
// values and their corresponding codesets are returned. In case of
// failure to convert, return the original header unmodified.
//
SdmErrorCode
SdmMessageEnvelope::DecodeHeader(SdmError &err, SdmStrStrL &r_hdr,
    const SdmString &hdr, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_start,
	       "api SdmMessageEnvelope",
	       "sunw%start SdmMessageEnvelope_DecodeHeader_start");

  SdmConnection *conn;
  LclError *ret;
  LCLd lcld;
  LCTd lclt;
  LclCharsetSegmentSet *segs = (LclCharsetSegmentSet *)NULL; 
  err = Sdm_EC_Success;
  int i;
  const char *str;
  SdmStrStrL ct_hdr;

#if 0 // debugging -- returns the source header undecoded
  r_hdr(-1).SetBothStrings("", hdr);
  return Sdm_EC_Success;
#endif

  // Get the locale descriptor from the connection object
  conn = SdmConnection::GetConnection();
  if (!conn) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_ConnectionNotStarted);
  }

  // First off, see if the header really needs to be decoded - if not, then dont
  // bother with the lcl song and dance as its very costly even if there is no
  // conversions to be done.

  if (!_HeaderNeedsDecoding(hdr,SdmSystemUtility::InCLocale())) {
    r_hdr(-1).SetBothStrings("", hdr);
    return (Sdm_EC_Success);
  }

  lcld = conn->GetLocaleDescriptor();
  if (!lcld) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_LclOpenFailed);
  }

  lclt = lct_create (lcld,
		     LctNSourceType, LctNMsgText, (const char *)hdr, LctNNone,
		     LctNSourceForm, LctNInComingStreamForm,
		     LctNKeepReference, LctNKeepByReference,
		     NULL);
  if (!lclt) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_LctCreateFailure);
  }

  // set source charset
  if(charset > "")
    lct_setvalues(lclt, LctNInComingStreamForm,
                  LctNHeaderCharset, (const char *)charset, NULL);

  ret = lct_getvalues (lclt,
		       LctNDisplayForm,
		       LctNHeaderSegment, &segs,
		       NULL);

  // If the header can't be converted, just use the original header,
  // otherwise return the converted header segments
  if (ret) {
    if ((ret->error_code != LctErrorIconvNonIdenticalConversion)
	&& (ret->error_code != LctErrorIconvHalfDone)) {
      r_hdr(-1).SetBothStrings("", hdr);
    }
    lcl_destroy_error(ret);
  } 
  else {
    assert(segs);
    for(i = 0; i < segs->num; i++){
      // Note: null terminate the segment by specifying explicit size
      // Note: as of the 12/2/96 release of the lcl library, 
      // lcl returns null-terminated segment (segs->seg[x].segment)
      // segs->seg[x].size is equal to the result of strlen(segs->seg[x].segment).
      // If segs->seg[x].segment is set to NULL, lcl_destroy_segment_set() call
      // would not deallocate it.
      r_hdr(-1).SetBothStrings(segs->seg[i].charset, segs->seg[i].segment);
    }
    lcl_destroy_segment_set(segs);
  }

  lct_destroy(lclt);

  TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
	       "api SdmMessageEnvelope",
	       "sunw%end SdmMessageEnvelope_DecodeHeader_end");
  return err;
}

//
// I18N
// The user must pass in the value of a header.  All resulting converted
// header segments are appended to the end of the string passed in. In case of
// failure to convert, the original header is appended.
//
SdmErrorCode
SdmMessageEnvelope::DecodeHeader(SdmError &err, SdmString &r_hdr,
    const SdmString &hdr, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_start,
	       "api SdmMessageEnvelope",
	       "sunw%start SdmMessageEnvelope_DecodeHeader_start");

  SdmConnection *conn;
  LclError *ret;
  LCLd lcld;
  LCTd lclt;
  LclCharsetSegmentSet *segs = (LclCharsetSegmentSet *)NULL; 
  err = Sdm_EC_Success;
  int i;
  const char *str;
  SdmStrStrL ct_hdr;

#if 0 // debugging -- returns the source header undecoded
  r_hdr += hdr;
  return Sdm_EC_Success;
#endif

  // Get the locale descriptor from the connection object
  conn = SdmConnection::GetConnection();
  if (!conn) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_ConnectionNotStarted);
  }

  // First off, see if the header really needs to be decoded - if not, then dont
  // bother with the lcl song and dance as its very costly even if there is no
  // conversions to be done.

  if (!_HeaderNeedsDecoding(hdr,SdmSystemUtility::InCLocale())) {
    r_hdr += hdr;
    return (Sdm_EC_Success);
  }

  lcld = conn->GetLocaleDescriptor();
  if (!lcld) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_LclOpenFailed);
  }

  lclt = lct_create (lcld,
		     LctNSourceType, LctNMsgText, (const char *)hdr, LctNNone,
		     LctNSourceForm, LctNInComingStreamForm,
		     LctNKeepReference, LctNKeepByReference,
		     NULL);
  if (!lclt) {
    TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
		 "api SdmMessageEnvelope",
		 "sunw%end SdmMessageEnvelope_DecodeHeader_end");
    return(err = Sdm_EC_LctCreateFailure);
  }

  // set source charset
  if(charset > "")
    lct_setvalues(lclt, LctNInComingStreamForm,
                  LctNHeaderCharset, (const char *)charset, NULL);

  ret = lct_getvalues (lclt,
		       LctNDisplayForm,
		       LctNHeaderSegment, &segs,
		       NULL);

  // If the header can't be converted, just use the original header,
  // otherwise return the converted header segments
  if (ret) {
    if ((ret->error_code != LctErrorIconvNonIdenticalConversion)
	&& (ret->error_code != LctErrorIconvHalfDone)) {
      r_hdr += hdr;
    }
    lcl_destroy_error(ret);
  } 
  else {
    assert(segs);
    for(i = 0; i < segs->num; i++){
      // Note: null terminate the segment by specifying explicit size
      // Note: as of the 12/2/96 release of the lcl library, 
      // lcl returns null-terminated segment (segs->seg[x].segment)
      // segs->seg[x].size is equal to the result of strlen(segs->seg[x].segment).
      // If segs->seg[x].segment is set to NULL, lcl_destroy_segment_set() call
      // would not deallocate it.
      r_hdr += segs->seg[i].segment;
    }
    lcl_destroy_segment_set(segs);
  }

  lct_destroy(lclt);

  TNF_PROBE_0 (SdmMessageEnvelope_DecodeHeader_end,
	       "api SdmMessageEnvelope",
	       "sunw%end SdmMessageEnvelope_DecodeHeader_end");
  return err;
}




SdmErrorCode 
SdmMessageEnvelope::GetHeaders(SdmError &err,SdmStrStrL &r_hdr)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetHeaders_start");

  err = Sdm_EC_Success;
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->GetMessageHeaders(err, r_hdr, _ParentMessgNum);
    }  
  }
  
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetHeaders_end");
  return err;
}

SdmErrorCode 
SdmMessageEnvelope::GetHeaders(SdmError &err, SdmIntStrL &r_hdrs, 
  const SdmMessageHeaderAbstractFlags header)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetHeaders_start");

  err = Sdm_EC_Success;

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->GetMessageHeaders(err, r_hdrs, header, _ParentMessgNum);
    }  
  }

  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetHeaders_end");
  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageEnvelope::GetHeader(SdmError &err, SdmStrStrL &r_hdr,
    const SdmString &hdr)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeader_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetHeader_start");

  err = Sdm_EC_Success;
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->GetMessageHeader(err, r_hdr, hdr, _ParentMessgNum);
    }  
  }
  
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeader_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetHeader_end");
  return err;
}

                                  
SdmErrorCode  
SdmMessageEnvelope::GetHeaders(SdmError &err, SdmStrStrL &r_hdrs,  
  const SdmMessageHeaderAbstractFlags header)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetHeaders_start");

  err = Sdm_EC_Success;

  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st =  _MessgStore->GetDataPort()->GetMessageHeaders(err, r_hdrs, header, _ParentMessgNum);
    }  
  }
  
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetHeaders_end");
  return err;
}


SdmErrorCode 
SdmMessageEnvelope::GetHeaders(SdmError &err, SdmStrStrL &r_hdrs,			
    const SdmStringL &hdrs)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetHeaders_start");

  err = Sdm_EC_Success;
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->GetMessageHeaders(err, r_hdrs, hdrs, _ParentMessgNum);
    }  
  }
  
  TNF_PROBE_0 (SdmMessageEnvelope_GetHeaders_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetHeaders_end");
  return err;
}

#endif
SdmErrorCode 
SdmMessageEnvelope::SetHeaders(SdmError &err, const SdmStrStrL &headers)
{
  TNF_PROBE_0 (SdmMessageEnvelope_SetHeaders_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_SetHeaders_start");

  err = Sdm_EC_Success;  
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->SetMessageHeaders(err, headers, _ParentMessgNum);
    }  
  }
    
  TNF_PROBE_0 (SdmMessageEnvelope_SetHeaders_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_SetHeaders_end");
  return err;
}


SdmErrorCode 
SdmMessageEnvelope::AddXMailerHeader(SdmError &err, const SdmString& mailerPrefix,
                                     SdmBoolean replaceExistingHeader)
{
  TNF_PROBE_0 (SdmMessageEnvelope_AddXMailerHeader_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_AddXMailerHeader_start");

  err = Sdm_EC_Success;

  SdmMessageUtility::AddXMailerHeader(err, mailerPrefix, *_MessgStore->GetDataPort(),
                                      _ParentMessgNum, replaceExistingHeader);

  TNF_PROBE_0 (SdmMessageEnvelope_AddXMailerHeader_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_AddXMailerHeader_end");
  return err;
}

#ifdef INCLUDE_UNUSED_API

SdmErrorCode 
SdmMessageEnvelope::AddHeader(SdmError &err, const SdmString& hdr, 	
    const SdmString& value)
{
  TNF_PROBE_0 (SdmMessageEnvelope_AddHeader_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_AddHeader_start");

  err = Sdm_EC_Success;  
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->AddMessageHeader(err, hdr, value, _ParentMessgNum);
    }  
  }
    
  TNF_PROBE_0 (SdmMessageEnvelope_AddHeader_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_AddHeader_end");
  return err;
}



SdmErrorCode 
SdmMessageEnvelope::RemoveHeader(SdmError &err, const SdmString& hdr)
{
  TNF_PROBE_0 (SdmMessageEnvelope_RemoveHeader_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_RemoveHeader_start");

  err = Sdm_EC_Success;  
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->RemoveMessageHeader(err, hdr, _ParentMessgNum);
    }  
  }
    
  TNF_PROBE_0 (SdmMessageEnvelope_RemoveHeader_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_RemoveHeader_end");
  return err;
}


SdmErrorCode 
SdmMessageEnvelope::ReplaceHeader(SdmError &err, const SdmString& hdr, 	
    const SdmString& value)
{
  TNF_PROBE_0 (SdmMessageEnvelope_ReplaceHeader_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_ReplaceHeader_start");

  err = Sdm_EC_Success;  
  
  SdmSyncRequestEntry entry(err, *this);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
      int st = _MessgStore->GetDataPort()->ReplaceMessageHeader(err, hdr, value, _ParentMessgNum);
    }  
  }
    
  TNF_PROBE_0 (SdmMessageEnvelope_ReplaceHeader_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_ReplaceHeader_end");
  return err;
}

#endif
 
SdmErrorCode 
SdmMessageEnvelope::Parent(SdmError &err, SdmMessage *&r_message)
{
  TNF_PROBE_0 (SdmMessageEnvelope_Parent_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_Parent_start");

  err = Sdm_EC_Success;
  SdmMessageEnvelope *me = (SdmMessageEnvelope*)this;
  
  SdmSyncRequestEntry entry(err, *me);  
  if (!err) {  		  
    if (_ParentMessg->IsExpunged()) {
      err = Sdm_EC_MessageExpunged;
    } else {
       r_message = _ParentMessg;
    }  
  }

  TNF_PROBE_0 (SdmMessageEnvelope_Parent_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_Parent_end");
   return err;
}

SdmErrorCode 
SdmMessageEnvelope::_GetContents(SdmError &err, SdmContentBuffer &r_contents)
{
  //
  // Caller should be handling the request before calling this method.
  // It should either have a sync request lock or is a "handle" method that
  // is processing a async request.
  //

  TNF_PROBE_0 (SdmMessageEnvelope__GetContents_start,
      "SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope__GetContents_start");

  err = Sdm_EC_Success;
  
  if (_ParentMessg->IsExpunged()) {
    err = Sdm_EC_MessageExpunged;
  } else {
    _MessgStore->GetDataPort()->GetMessageContents(
        err, r_contents, DPCTTYPE_rawheader, _ParentMessgNum);
  }

  TNF_PROBE_0 (SdmMessageEnvelope__GetContents_end,
      "SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope__GetContents_end");
  return err;
}


void 
SdmMessageEnvelope::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event,
  const SdmBoolean isCancelled)
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetEventForRequest_start,
      "SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetEventForRequest_start");

  switch(reqst->GetOperator()) {

    case SdmData::AOP_GetContents_1:
      {
        // create and setup event that is returned in callback.	
        r_event = new SdmEvent;

        r_event->sdmGetContents.error = new SdmError;
        r_event->sdmGetContents.type = Sdm_Event_GetMessageEnvelopeContents;  
        r_event->sdmGetContents.messageNumber = _ParentMessgNum;
        r_event->sdmGetContents.bodyPartNumber = -1;;

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
    default:
      {
        SdmData::GetEventForRequest(reqst, r_event, isCancelled);
        break;
      }
 }  

  TNF_PROBE_0 (SdmMessageEnvelope_GetEventForRequest_end,
      "SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetEventForRequest_end");
 return;
}
                         

// I18N - set the user chosen charset
SdmErrorCode
SdmMessageEnvelope::SetCharacterSet(SdmError& err, const SdmString &charset)
{
  TNF_PROBE_0 (SdmMessageEnvelope_SetCharacterSet_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_SetCharacterSet_start");
 
  _user_charset = charset;
 
  TNF_PROBE_0 (SdmMessageEnvelope_SetCharacterSet_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_SetCharacterSet_end");
  return err;
}

// I18N - get the user chosen charset
SdmErrorCode
SdmMessageEnvelope::GetCharacterSet(SdmError& err, SdmString& r_charSet) const
{
  TNF_PROBE_0 (SdmMessageEnvelope_GetCharacterSet_start,
      "api SdmMessageEnvelope",
      "sunw%start SdmMessageEnvelope_GetCharacterSet_start");
 
  err = Sdm_EC_Success;
  r_charSet = _user_charset;
 
  TNF_PROBE_0 (SdmMessageEnvelope_GetCharacterSet_end,
      "api SdmMessageEnvelope",
      "sunw%end SdmMessageEnvelope_GetCharacterSet_end");
  return err;
}

