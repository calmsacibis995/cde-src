/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Data Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Data.cc	1.51 97/03/14 SMI"
#endif

// Include Files.
#include <SDtMail/Data.hh>
#include <SDtMail/ServiceFunction.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/Event.hh>
#include <SDtMail/RepSWMR.hh>
#include <CtrlObjs/ReplyQueue.hh>
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <TranObjs/SyncRequestEntry.hh>


SdmData::SdmData(const SdmClassId classId, SdmError& err)
  :SdmPrim(classId)
{
}

SdmData::SdmData(const SdmData& copy) 
  :SdmPrim(copy)
{
  *this = copy;
}

SdmData& 
SdmData::operator=(const SdmData &rhs)
{
  return *this;
}

SdmData::~SdmData()
{
}


void*
SdmData::operator new(size_t size)
{
  return SdmMemoryRepository::_ONew(size, Sdm_MM_Message);
}
 
void*
SdmData::operator new(size_t size, void* mAdd)
{
  return SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_Message);
}
 
void
SdmData::operator delete(void* mAdd)
{
  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_Message);
}

SdmErrorCode
SdmData::GetContents(SdmError& err, SdmContentBuffer& r_contents) 
{
  SdmSyncRequestEntry entry(err, *this);
  if (!err) {
    int st = _GetContents(err, r_contents);
  }
  return err;
}


 
SdmErrorCode
SdmData::SetContents(SdmError& err, const SdmContentBuffer& buf)
{
  err = Sdm_EC_Fail;
  return err;
}

#ifdef INCLUDE_UNUSED_API
                          
SdmErrorCode
SdmData::GetContents_Async(SdmError &err, const SdmServiceFunction& svf, void* clientData) 
{  
  SdmData *me = (SdmData*) this;
  SdmServiceFunction *svfClone = svf.SdmServiceFunctionFactory(clientData);

  SdmRequest* reqst = new SdmRequest(NULL, SdmData::AOP_GetContents_1, this);
  reqst->SetServiceFunction(svfClone);
  
  if (me->_CollectorSession->PostRequest(err, me, reqst)) {
    // need to clean up request and arguments;
    delete reqst;    
	}
	 
  return err;
} 

SdmErrorCode
SdmData::SetContents_Async(SdmError& err, const SdmServiceFunction& svf, void* clientData, const SdmContentBuffer& buf)
{
  err = Sdm_EC_Fail;
  return err;
}

#endif

SdmErrorCode
SdmData::_GetContents(SdmError& err, SdmContentBuffer& buf) 
{
  err = Sdm_EC_Fail;
  return err;
}
 
SdmErrorCode
SdmData::_SetContents(SdmError& err, const SdmContentBuffer& buf)
{
  err = Sdm_EC_Fail;
  return err;
}
 
