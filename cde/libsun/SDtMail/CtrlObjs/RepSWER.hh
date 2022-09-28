/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the System Wide Entity Repository Class.

#ifndef _SDM_REPSWER_H
#define _SDM_REPSWER_H

#pragma ident "@(#)RepSWER.hh	1.35 97/03/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/ThreadDefs.hh>

class SdmPrim;
class SdmSession;
class SdmServer;
class SdmRequest;
class SdmReply;
class SdmRequestQueue;
class SdmReplyQueue;
/*[
	Cool thing to do here, will be to use ObjectSignature as a key into
    a hash table and make lookup really fast. But skip for now, due
    to complications and also our need not to over-rely on RogueWave.
*/
/*[
	Object Signature || SdmPrim* || Dispatching || Serving Data || Serving
									Session        Server		   Thread
*/
class SdmEntity {
public:
  SdmObjectSignature _ObjSignature;
  SdmPrim* _ObjThis;
  SdmSession* _ServingCollector;
  SdmServer* _ServingThreadant;
  SdmThreadId _ServingThreadId;
  SdmBoolean _EntityValid;

public:
  SdmEntity(SdmObjectSignature objSignature);
  SdmEntity(SdmObjectSignature objSignature, SdmPrim* objThis, SdmSession* session,
	    SdmServer* server);
  ~SdmEntity();

  // These are include here so they can be inlined in templates

  SdmBoolean operator== (const SdmEntity& entity) const 
  {
    return _ObjSignature == entity._ObjSignature ? Sdm_True : Sdm_False;
  }
  SdmBoolean operator< (const SdmEntity& entity) const 
  {
    return _ObjSignature < entity._ObjSignature ? Sdm_True : Sdm_False;
  }
};

class SdmCollector {
public:
  SdmSession* _ServingCollector;
  SdmRequestQueue* _CollectorRequestQueue;
  SdmReplyQueue* _CollectorReplyQueue;
  SdmBoolean _CollectorValid;

public:
  SdmCollector(SdmSession* session);
  SdmCollector(SdmSession* session, SdmRequestQueue* rqQueue,
	       SdmReplyQueue* rpQueue);
  ~SdmCollector();

  // These are include here so they can be inlined in templates

  SdmBoolean operator== (const SdmCollector& collector) const 
  {
    return _ServingCollector == collector._ServingCollector ? Sdm_True : Sdm_False;
  }
  SdmBoolean operator< (const SdmCollector& collector) const
  {
    return _ServingCollector < collector._ServingCollector ? Sdm_True : Sdm_False;
  }
};

class SdmEntityRepository{

public:
  SdmEntityRepository();
  virtual ~SdmEntityRepository();

  static void RegisterObject(SdmPrim* objThis, const SdmObjectSignature objSignature,
			     SdmSession* collectorSession, SdmServer*
			     servingThreadant);
  static void UnRegisterObject(const SdmObjectSignature objSignature);
  static SdmPrim* ResolveObjReference(const SdmObjectSignature objSignature);
  static void RegisterCollector(SdmSession* session, SdmRequestQueue* rqQueue,
				SdmReplyQueue* rpQueue);
  static void UnRegisterCollector(SdmSession* session);
  static SdmRequestQueue* GetCollectorRequestQueue(const SdmObjectSignature objSignature);
  static SdmReplyQueue* GetCollectorReplyQueue(const SdmObjectSignature objSignature);

private:
  //	static SdmPtrSortVector<SdmEntity> _EntityList;
  //	static SdmVector<SdmCollector> _CollectorList;
	
};

#endif
