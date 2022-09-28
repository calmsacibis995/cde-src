/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Transaction Class.

#pragma ident "@(#)ArgumentList.cc	1.47 96/08/21 SMI"

// Include Files.
#include <stdio.h>
#include <TranObjs/ArgumentList.hh>
#include <SDtMail/RepSWMR.hh>
#include <SDtMail/Token.hh>

/*[
  Do we really need int* and other type-pointers, Why ???
  Is NULL a valid value, at any time in the ArgBay.
  */
SdmArgumentList::SdmArgumentList()
  :SdmPrim(Sdm_CL_ArgumentList)
{
  _CurPutSeg = -1;
  _CurGetSeg = -1;
  _ArgCount = 0;

  for (int i = 0; i < SdmTran_MaxArgSet_Size; i++) {
    _ArgBay[i] = NULL;
    _ArgSpec[i] = ArgT_Inv;
  }
}  


SdmArgumentList::~SdmArgumentList()
{
  // We are deleting only our allocated pointers
  // Note: we need to cast the value of _ArgBay item to the appropriate
  // pointer in order to delete it properly.  We can't delete void pointers.
  for (int i = 0; i < _ArgCount; i++) {
    if (_DeleteArg[i]) {
      switch (_ArgSpec[i]) {
      case ArgT_String:
        delete (SdmString*) _ArgBay[i];
        break;
      case ArgT_StringL:
        delete (SdmStringL*) _ArgBay[i];
        break;
      case ArgT_CBuf:
        delete (SdmContentBuffer*) _ArgBay[i];
        break;
      case ArgT_Int:
        delete (int*) _ArgBay[i];
        break;
      case ArgT_Booleon:
        delete (SdmBoolean*) _ArgBay[i];
        break;
      default:
        break;
      }
    }
  }
}

SdmArgumentList::SdmArgumentList(const SdmArgumentList& tran)
  :SdmPrim(tran)
{
  cout << "SdmArgumentList::SdmArgumentList-Copy Constructor Called$$$$$$$$$$$$$$$$" << endl;
  assert(0);
}

SdmArgumentList&
SdmArgumentList::operator= (const SdmArgumentList& tran)
{ 
  cout << "SdmArgumentList::SdmArgumentList-Assign Operator Called$$$$$$$$$$$$$$$$" << endl;
  assert(0);
  return *this;
}

void*
SdmArgumentList::operator new(size_t size)
{
  return SdmMemoryRepository::_ONew(size, Sdm_MM_Transaction);
}

void*
SdmArgumentList::operator new(size_t size, void* mAdd)
{
  return SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_Transaction);
}

void
SdmArgumentList::operator delete(void* mAdd)
{
  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_Transaction);
}

void
SdmArgumentList::PutArg(const SdmString& arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) new SdmString(arg);
  _ArgSpec[_CurPutSeg] = ArgT_String;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(const SdmStringL& arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) new SdmStringL(arg);
  _ArgSpec[_CurPutSeg] = ArgT_StringL;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(const SdmContentBuffer& arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) new SdmContentBuffer(arg);
  _ArgSpec[_CurPutSeg] = ArgT_CBuf;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(const SdmToken& arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  SdmToken *token = new SdmToken(arg);
  _ArgBay[_CurPutSeg] = (void *) token;
  _ArgSpec[_CurPutSeg] = ArgT_Token;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(const int arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) new int;
  *((int* ) (_ArgBay[_CurPutSeg])) = arg;
  _ArgSpec[_CurPutSeg] = ArgT_Int;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(const SdmBoolean arg)
{
  ++_CurPutSeg;
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) new SdmBoolean;
  *((int* ) (_ArgBay[_CurPutSeg])) = arg;
  _ArgSpec[_CurPutSeg] = ArgT_Booleon;
  _DeleteArg[_CurPutSeg] = Sdm_True;
  ++_ArgCount;
}

void
SdmArgumentList::PutArg(sema_t *arg)
{
  ++_CurPutSeg; // Hit occurs not in this new but in the copy constructor
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) arg;
  _ArgSpec[_CurPutSeg] = ArgT_SemaP;
  _DeleteArg[_CurPutSeg] = Sdm_False;
  ++_ArgCount;
}


void
SdmArgumentList::PutArg(SdmError *arg)
{
  ++_CurPutSeg; // Hit occurs not in this new but in the copy constructor
  assert(_CurPutSeg < SdmTran_MaxArgSet_Size);
  _ArgBay[_CurPutSeg] = (void *) arg;
  _ArgSpec[_CurPutSeg] = ArgT_ErrorP;
  _DeleteArg[_CurPutSeg] = Sdm_False;
  ++_ArgCount;
}


void*
SdmArgumentList::GetArg(const SdmArgumentList::ArgT argT)
{

  ++_CurGetSeg;

  if (_ArgCount <= 0 || _CurGetSeg >= _ArgCount)
    { 
		cout << "SdmArgumentList::GetArg Nothing returned.\n";
		return NULL; 
	}; // Throw Excp, Inavlid Type Request

  if (_ArgSpec[_CurGetSeg] != argT)
    { 
		cout << "SdmArgumentList::GetArg Nothing returned.\n";
		return NULL; 
	}; // Throw Excp, Inavlid Type Request

  return _ArgBay[_CurGetSeg];
}

void
SdmArgumentList::ResetGetSegment()
{
	_CurGetSeg = -1;
}
