/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Transaction Class.

#ifndef _SDM_ARGUMENTLIST_H
#define _SDM_ARGUMENTLIST_H

#pragma ident "@(#)ArgumentList.hh	1.42 96/08/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <CtrlObjs/ThreadDefs.hh>

class SdmToken;

static const int SdmTran_MaxArgSet_Size = 100;

class SdmArgumentList : public SdmPrim {

public:
  enum ArgT {ArgT_Inv = -1, ArgT_Int, ArgT_IntP, ArgT_String, ArgT_StringL,
             ArgT_Booleon, ArgT_SemaP, ArgT_Token, ArgT_ErrorP, ArgT_CBuf };

public:
  SdmArgumentList();
  virtual ~SdmArgumentList();

public: // Memory Operator Overloading
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

public:
  void PutArg(const SdmString& arg);
  void PutArg(const SdmStringL& arg);
  void PutArg(const SdmContentBuffer& arg);
  void PutArg(const int arg);
  void PutArg(const SdmBoolean arg);
  void PutArg(sema_t *arg);
  void PutArg(const SdmToken& arg);
  void PutArg(SdmError *arg);

public:
  void* GetArg(const SdmArgumentList::ArgT argT);
  void  ResetGetSegment();

protected:
  int _CurPutSeg;
  int _CurGetSeg;
  int _ArgCount;
  void* _ArgBay[SdmTran_MaxArgSet_Size];
  ArgT _ArgSpec[SdmTran_MaxArgSet_Size];
  SdmBoolean _DeleteArg[SdmTran_MaxArgSet_Size];

private:
  SdmArgumentList(const SdmArgumentList& tran);
  SdmArgumentList& operator= (const SdmArgumentList& tran);
};

#endif
