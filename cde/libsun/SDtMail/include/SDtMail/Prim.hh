/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Primitive Class.

#ifndef _SDM_PRIM_H
#define _SDM_PRIM_H

#pragma ident "@(#)Prim.hh	1.40 97/03/14 SMI"

#include <memory.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Event.hh>
#include <SDtMail/RepSWMR.hh>


class SdmSession;
class SdmServer;
class SdmRequest;
class SdmReply;
class SdmRequestQueue;
class SdmReplyQueue;

class SdmPrim {

public:
  SdmClassId _ClId;
  SdmBoolean _IsStarted;
  static SdmObjectSignature _ObjSignatureCounter;

public: // SdmEntityRepository Data
  SdmSession* _CollectorSession;
  SdmServer* _ServingThreadant;
  SdmObjectSignature _ObjSignature;
	
public:
  virtual ~SdmPrim();

  virtual SdmErrorCode StartUp(SdmError& err);
  virtual SdmErrorCode ShutDown(SdmError& err);

public: // Memory Operator Overloading
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

public:
  SdmClassId IsA() const;
  SdmBoolean IsStarted() const 
    { return _IsStarted; }
  
  SdmBoolean StartUpCheck(const char* fileName, const int lineNo) const;

public:
  // Rogue Wave Persistence Classes
  SdmObjectSize binaryStoreSize() const;

public: // Virtual Dispatchers
  virtual void InCallDispatchPoint(const SdmRequest* reqst);
  virtual void OutCallDispatchPoint(SdmReply* reply);
  virtual void CancelRequestDispatchPoint(const SdmRequest* reqst);
  virtual void GetEventForRequest(const SdmRequest* reqst, 
                      SdmEvent *&r_event, const SdmBoolean isCancelled);

  
public: // Queue Accessors.
  SdmRequestQueue* GetServerRequestQueue();

  
protected:
  SdmPrim(const SdmClassId clId);
  SdmPrim(const SdmPrim& prim);
  SdmPrim& operator= (const SdmPrim& prim);
};

#define __SdmPRIM_STARTUPCHECK StartUpCheck(__FILE__, __LINE__);

#define __SdmPRIM_STARTUPCHECK2(errorVar, errorCode) \
  if (!StartUpCheck(__FILE__, __LINE__)) { \
    errorVar = errorCode; \
    return errorVar; \
  }

#define __SdmPRIM_REGISTERENTITY(session, server) \
_CollectorSession = (SdmSession *) session; \
_ServingThreadant = (SdmServer *) server; \
SdmEntityRepository::RegisterObject(this, _ObjSignature, _CollectorSession,\
		     _ServingThreadant);
#define __SdmPRIM_UNREGISTERENTITY \
SdmEntityRepository::UnRegisterObject(_ObjSignature);

#define __SdmPRIM_REGISTERCOLLECTOR(session, queue1, queue2) \
SdmEntityRepository::RegisterCollector(session, queue1, queue2);
#define __SdmPRIM_UNREGISTERCOLLECTOR \
SdmEntityRepository::UnRegisterCollector(this);

#endif
