/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Data Class.

#ifndef _SDM_DATA_H
#define _SDM_DATA_H

#pragma ident "@(#)Data.hh	1.46 97/01/09 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/String.hh>
#include <SDtMail/Event.hh>
#include <SDtMail/Time.hh>
#include <SDtMail/ContentBuffer.hh>

class SdmServiceFunction;

class SdmData : public SdmPrim {

public:
  enum OperationList {AOP_SyncOp=100, AOP_GetContents_1, AOP_GetContents_2, 
    AOP_SetContents_1, AOP_SetContents_2 };


public: // Memory Operator Overloading
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

public:
  virtual SdmErrorCode GetContents(SdmError &err, SdmContentBuffer &r_contents);
 
  virtual SdmErrorCode SetContents(SdmError& err, const SdmContentBuffer& buf);

#ifdef INCLUDE_UNUSED_API
  virtual SdmErrorCode GetContents_Async(SdmError &err, const SdmServiceFunction& svf, void* clientData);
  virtual SdmErrorCode SetContents_Async(SdmError& err, const SdmServiceFunction& svf, 
                          void* clientData, const SdmContentBuffer& buf);
#endif

protected:
  SdmData(const SdmClassId classId, SdmError& err);
  virtual ~SdmData();
  
  SdmData(const SdmData& copy);
  SdmData& operator=(const SdmData &rhs);

protected:
  virtual SdmErrorCode _GetContents(SdmError& err, SdmContentBuffer& buf);     
  virtual SdmErrorCode _SetContents(SdmError& err, const SdmContentBuffer& buf);
};

#endif
