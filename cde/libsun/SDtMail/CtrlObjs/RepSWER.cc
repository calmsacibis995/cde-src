/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the System Wide Entity Repository Class.

#pragma ident "@(#)RepSWER.cc	1.42 97/03/14 SMI"

// Include Files.
#include <iostream.h>
#include <stdio.h>
#include <assert.h>
#include <SDtMail/Sdtmail.hh>
#include <CtrlObjs/RepSWER.hh>
#include <Utils/TnfProbe.hh>


static SdmPtrSortVector<SdmEntity *> _EntityList;
static SdmPtrSortVector<SdmCollector *> _CollectorList;

SdmEntity::SdmEntity(SdmObjectSignature objSignature)
{
  _ObjSignature = objSignature;
  _ObjThis = NULL;
  _ServingCollector = NULL;
  _ServingThreadant = NULL;
  _ServingThreadId = -1;
  _EntityValid = Sdm_False;
}

SdmEntity::SdmEntity(SdmObjectSignature objSignature, SdmPrim* objThis, SdmSession* session,
		     SdmServer* server)
{
  _ObjSignature = objSignature;
  _ObjThis = objThis;
  _ServingCollector = session;
  _ServingThreadant = server;
  _ServingThreadId = -1;
  _EntityValid = Sdm_True;
}

SdmEntity::~SdmEntity()
{
}

SdmCollector::SdmCollector(SdmSession* session, SdmRequestQueue* rqQueue,
			   SdmReplyQueue* rpQueue)
{
  _ServingCollector = session;
  _CollectorRequestQueue = rqQueue;
  _CollectorReplyQueue = rpQueue;
}

SdmCollector::SdmCollector(SdmSession* session)
{
  _ServingCollector = session;
  _CollectorRequestQueue = NULL;
  _CollectorReplyQueue = NULL;
}

SdmCollector::~SdmCollector()
{
}

SdmEntityRepository::SdmEntityRepository()
{
  _EntityList.ClearAllElements();
  _CollectorList.ClearAllElements();
}

SdmEntityRepository::~SdmEntityRepository()
{
  // Destroy All Entities
  // Destroy All Collectors
}

void
SdmEntityRepository::RegisterObject(SdmPrim* objThis, const SdmObjectSignature objSignature,
			   SdmSession* collectorSession, SdmServer*
			   servingThreadant)
{
  TNF_PROBE_3 (SdmEntityRespository_RegisterObject,
      "registry trace call",
      "sunw%debug SdmEntityRepository::RegisterObject Called with args: ",
      tnf_long, object, (long)objSignature,
      tnf_long, session, (long) collectorSession,
      tnf_long, server, (long) servingThreadant);
  
  
  SdmEntity* ent = new SdmEntity(objSignature, objThis, collectorSession,
				 servingThreadant);
  if (ent == NULL)
    {}; // Throw Excp
  _EntityList.AddElementToList(ent);
}

void
SdmEntityRepository::UnRegisterObject(const SdmObjectSignature objSignature)
{
  // cout << "SdmEntityRepository::UnRegister Object Called with :" << objSignature
  //      << endl;

  TNF_PROBE_1 (SdmEntityRespository_UnregisterObject,
      "registry trace call",
      "sunw%debug SdmEntityRepository::UnregisterObject Called with args: ",
      tnf_long, object, (long)objSignature);

  SdmEntity ent1(objSignature);
  int ent_index;

  ent_index = _EntityList.FindElementByValue(&ent1);
  if (ent_index == Sdm_Not_Found) {
    assert (0); // Throw Excp : Assertion Failed
  } else {
    SdmEntity *ent = _EntityList[ent_index];
    _EntityList.RemoveElement(ent_index);    // note: Remove does not delete element.
    delete ent;
  }
}

void
SdmEntityRepository::RegisterCollector(SdmSession* session, SdmRequestQueue* rqQueue,
                              SdmReplyQueue* rpQueue)
{
  // cout << "SdmEntityRepository::Register Collector Called with :" << session << ", "
  //      << rqQueue << ", " << rpQueue << endl;

  TNF_PROBE_2 (SdmEntityRespository_RegisterCollector,
      "registry trace call",
      "sunw%debug SdmEntityRepository::Register Collector Called with args: ",
      tnf_long, session, (long)session, 
      tnf_long, queue, (long)rpQueue);


  SdmCollector* ent = new SdmCollector(session, rqQueue, rpQueue);
  if (ent == NULL)
    {}; // Throw Excp

  _CollectorList.AddElementToList(ent);

}

SdmPrim*
SdmEntityRepository::ResolveObjReference(const SdmObjectSignature objSignature)
{
  SdmEntity* ent = new SdmEntity(objSignature);
  int fnd_index;

  fnd_index = _EntityList.FindElement(ent);
  if (fnd_index == Sdm_Not_Found)
    {}; // Throw Excp : Assertion Failed

  delete ent;
  return _EntityList[fnd_index]->_ObjThis;
}

void
SdmEntityRepository::UnRegisterCollector(SdmSession* session)
{

  TNF_PROBE_1 (SdmEntityRespository_UnregisterCollector,
      "registry trace call",
      "sunw%debug SdmEntityRepository::UnegisterCollector Called with args: ",
      tnf_long, session, (long)session);

 SdmCollector ent1(session);
  int ent_index;

  ent_index = _CollectorList.FindElementByValue(&ent1);
  if (ent_index == Sdm_Not_Found) {
    assert (0); // Throw Excp : Assertion Failed
  } else {
    SdmCollector *collector = _CollectorList[ent_index];
    _CollectorList.RemoveElement(ent_index);
    delete collector;
  }
}

SdmRequestQueue*
SdmEntityRepository::GetCollectorRequestQueue(const SdmObjectSignature objSignature)
{
  SdmEntity* ent1 = new SdmEntity(objSignature);
  int fnd1_index;

  fnd1_index = _EntityList.FindElement(ent1);
  if (fnd1_index == Sdm_Not_Found)
    {}; // Throw Excp : Assertion Failed
	
  SdmCollector* ent2 
    = new SdmCollector(_EntityList[fnd1_index]->_ServingCollector);
  int fnd2_index;
	
  fnd2_index = _CollectorList.FindElement(ent2);
  if (fnd2_index == Sdm_Not_Found)
    {}; // Throw Excp : Assertion Failed

  delete ent1; delete ent2;
  return _CollectorList[fnd2_index]->_CollectorRequestQueue;
}

SdmReplyQueue*
SdmEntityRepository::GetCollectorReplyQueue(const SdmObjectSignature objSignature)
{
  SdmEntity* ent1 = new SdmEntity(objSignature);
  int fnd1_index;

  fnd1_index = _EntityList.FindElement(ent1);
  if (fnd1_index == Sdm_Not_Found)
    {}; // Throw Excp : Assertion Failed

  SdmCollector* ent2 
    = new SdmCollector(_EntityList[fnd1_index]->_ServingCollector);
  int fnd2_index;

  fnd2_index = _CollectorList.FindElement(ent2);
  if (fnd2_index == Sdm_Not_Found)
    {}; // Throw Excp : Assertion Failed

  delete ent1; delete ent2;
  return _CollectorList[fnd2_index]->_CollectorReplyQueue;
}

