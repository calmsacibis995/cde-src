/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> These are the Simple Tuple Structures.

#ifndef _SDM_SIMPLETUPLES_H
#define _SDM_SIMPLETUPLES_H

#pragma ident "@(#)SimpleTuples.hh	1.40 96/12/01 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>

//-------------------------------------------------------------------

//
// Int String Class
//
class ostream;
class SdmIntStr {
public:
  enum CollateMode { Sdm_ISCS_None, Sdm_ISCS_IntOnly, Sdm_ISCS_StrOnly, Sdm_ISCS_Both };

  SdmIntStr();
  ~SdmIntStr();
  SdmIntStr(int num, const SdmString& str)
    { _num = num; _str = str; _collateMode = Sdm_ISCS_StrOnly; }
  SdmIntStr(int num, const char* str)
    { _num = num; _str = str; _collateMode = Sdm_ISCS_StrOnly; }
  SdmIntStr(const SdmIntStr& dubStr);
  SdmIntStr& operator= (const SdmIntStr& dubStr);
  SdmBoolean operator== (const SdmIntStr &s) const;
  SdmBoolean operator!= (const SdmIntStr &s) const;
  SdmBoolean operator< (const SdmIntStr &s) const;
  SdmBoolean operator<= (const SdmIntStr &s) const;
  SdmBoolean operator> (const SdmIntStr &s) const;
  SdmBoolean operator>= (const SdmIntStr &s) const;
  SdmBoolean operator! () const;
 
  void Clear()
    { _num = 0; _str = SdmString(); }
  int GetNumber()
    { return _num; }
  void SetNumber(int num)
    { _num = num; }
  SdmString GetString()
    { return _str; }
  void SetString(const SdmString& str)
    { _str = str; }
  void SetString(const char* str)
    { _str = str; }
  void SetNumberAndString(int num, const SdmString& str)
    { _num = num; _str = str; }
  void SetNumberAndString(int num, const char* str)
    { _num = num; _str = str; }
  void SetCollateMode(const CollateMode collateModeOption);
  friend ostream & operator << (ostream &strm, const SdmIntStr &ss);

private:
  int _num;
  SdmString _str;
  CollateMode _collateMode;
 
};

//-------------------------------------------------------------------

//
// String String Class
//
class ostream;
class SdmStrStr {
private:
  SdmString _nStr;
  SdmString _dStr;
 
public:
  SdmStrStr();
  ~SdmStrStr();
  SdmStrStr(const SdmString& nStr, const SdmString& dStr)
    { _nStr = nStr; _dStr = dStr; }
  SdmStrStr(const char* nStr, const char* dStr)
    { _nStr = nStr; _dStr = dStr; }
  SdmStrStr(const SdmStrStr& dubStr);
  SdmStrStr& operator= (const SdmStrStr& dubStr);
  SdmBoolean operator== (const SdmStrStr &s) const;
  SdmBoolean operator!= (const SdmStrStr &s) const;
  SdmBoolean operator< (const SdmStrStr &s) const;
  SdmBoolean operator> (const SdmStrStr &s) const;
 
  void Clear()
    { _nStr = SdmString(); _dStr = SdmString(); }
  SdmString GetFirstString()
    { return _nStr; }
  void SetFirstString(const SdmString& nStr)
    { _nStr = nStr; }
  void SetFirstString(const char* nStr)
    { _nStr = nStr; }
  SdmString GetSecondString()
    { return _dStr; }
  void SetSecondString(const SdmString& dStr)
    { _dStr = dStr; }
  void SetSecondString(const char* dStr)
    { _dStr = dStr; }
  void SetBothStrings(const SdmString& nStr, const SdmString& dStr)
    { _nStr = nStr; _dStr = dStr; }
  void SetBothStrings(const char* nStr, const SdmString& dStr)
    { _nStr = nStr; _dStr = dStr; }
  void SetBothStrings(const SdmString& nStr, const char* dStr)
    { _nStr = nStr; _dStr = dStr; }
  void SetBothStrings(const char* nStr, const char* dStr)
    { _nStr = nStr; _dStr = dStr; }
  friend ostream & operator << (ostream &strm, const SdmStrStr &ss);
};

//-------------------------------------------------------------------

#endif
