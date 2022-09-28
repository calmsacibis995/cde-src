/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Sdm Primitive Class.

#pragma ident "@(#)Prim.cc	1.47 97/03/14 SMI"

// Include Files
#include <assert.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/RepSWMR.hh>
#include <CtrlObjs/RepSWER.hh>
#include <CtrlObjs/RequestQueue.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <SDtMail/Server.hh>
#include <SDtMail/Session.hh>

SdmObjectSignature SdmPrim::_ObjSignatureCounter = 0;	// 0 reserved for "none"
SdmPrim::SdmPrim(const SdmClassId clId) : _ClId(clId),
  _IsStarted(Sdm_False), _CollectorSession(NULL), _ServingThreadant(NULL),
  _ObjSignature(++_ObjSignatureCounter)
{
}

SdmPrim::SdmPrim(const SdmPrim& prim): _ClId(prim._ClId),
  _IsStarted(prim._IsStarted), _CollectorSession(prim._CollectorSession), 
  _ServingThreadant(prim._ServingThreadant),
  _ObjSignature(++_ObjSignatureCounter)  
{
}

SdmPrim::~SdmPrim()
{
  _ClId = Sdm_CL_INVALID;
  _IsStarted = Sdm_False;
}

SdmPrim&
SdmPrim::operator= (const SdmPrim& prim)
{
  if (this == &prim)
    return *this;
  _ClId = prim._ClId;
  _IsStarted = prim._IsStarted;

  _CollectorSession = prim._CollectorSession;
  _ServingThreadant = prim._ServingThreadant;
  _ObjSignature = ++_ObjSignatureCounter; // New, don't copy bozo
  return *this;
}

SdmErrorCode
SdmPrim::StartUp(SdmError& err)
{
  return Sdm_EC_Fail;
}

SdmErrorCode
SdmPrim::ShutDown(SdmError& err)
{
  return Sdm_EC_Fail;
}

void* SdmPrim::operator new(size_t size)  
{ return SdmMemoryRepository::_ONew(size); }

void* SdmPrim::operator new(size_t size, void* mAdd)  
{ return SdmMemoryRepository::_ONew(size, mAdd); }

void SdmPrim::operator delete(void* mAdd)
{ SdmMemoryRepository::_ODel(mAdd); }



SdmClassId
SdmPrim::IsA() const
{
  return _ClId;
}

SdmBoolean
SdmPrim::StartUpCheck(const char* fileName, const int lineNo) const
{
  if (!IsStarted()) {
    return Sdm_False;
  }
  return Sdm_True;
}

SdmObjectSize
SdmPrim::binaryStoreSize() const
{
  return sizeof(_ClId);
}


void 
SdmPrim::InCallDispatchPoint(const SdmRequest* reqst)
{
  // create and setup event that is returned in callback.	
  SdmEvent *sessEvent = NULL;
  GetEventForRequest(reqst, sessEvent, Sdm_False);

  // create and setup reply to return to front end.      
  SdmReply* reply = new SdmReply;
  *reply = *reqst;
  delete ((SdmRequest*)reqst);

  reply->SetEvent(sessEvent);
  reply->SetServiceFunctionType(Sdm_SVF_SolicitedEvent);
  OutCallDispatchPoint(reply);
}

void 
SdmPrim::CancelRequestDispatchPoint(const SdmRequest* reqst)
{
  // create and setup event that is returned in callback.	
  SdmEvent *sessEvent = NULL;
  GetEventForRequest(reqst, sessEvent, Sdm_True);

  // create and setup reply to return to front end.      
  SdmReply* reply = new SdmReply;
  *reply = *reqst;
  delete ((SdmRequest*)reqst);

  reply->SetEvent(sessEvent);
  reply->SetServiceFunctionType(Sdm_SVF_SolicitedEvent);
  OutCallDispatchPoint(reply);
}


void
SdmPrim::GetEventForRequest(const SdmRequest* reqst, SdmEvent *&r_event, 
  const SdmBoolean isCancelled)
{
  r_event = new SdmEvent;  
  r_event->sdmRequestError.error = new SdmError;
  r_event->sdmRequestError.type = Sdm_Event_RequestError;
  if (isCancelled) {
    *(r_event->sdmRequestError.error) = Sdm_EC_Cancel;
  } else {
    *(r_event->sdmRequestError.error) = Sdm_EC_Fail;
  }
}


void 
SdmPrim::OutCallDispatchPoint(SdmReply* reply)
{
  SdmError err;
   // everything should have a session associated with it.
  assert(_CollectorSession != NULL);
  _CollectorSession->PostReply(err, this, reply);
}

SdmRequestQueue* 
SdmPrim::GetServerRequestQueue()
{
   if (_ServingThreadant) 
	   return _ServingThreadant->GetRequestQueue();
   return NULL;
}

