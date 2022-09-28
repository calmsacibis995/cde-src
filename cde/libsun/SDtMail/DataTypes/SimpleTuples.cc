/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> These are the Super Simple Tuple Structures.

#pragma ident "@(#)SimpleTuples.cc	1.35 96/07/11 SMI"

// Include Files
#include <iostream.h>
#include <SDtMail/Sdtmail.hh>


//---------------------------------------------------------------

//
// Int String Class
//
SdmIntStr::SdmIntStr()
  :_num(), _str(), _collateMode(Sdm_ISCS_StrOnly)
{
}

SdmIntStr::~SdmIntStr()
{
}

SdmIntStr::SdmIntStr(const SdmIntStr& dubStr)
  :_num(dubStr._num), _str(dubStr._str), _collateMode(dubStr._collateMode)
{
}

SdmIntStr& SdmIntStr::operator=(const SdmIntStr& dubStr)
{
  _num = dubStr._num;
  _str = dubStr._str;
  _collateMode = dubStr._collateMode;
  return *this;
}

SdmBoolean SdmIntStr::operator== (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num == s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str == s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num == s._num) && (_str == s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator!= (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num != s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str != s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num != s._num) && (_str != s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator< (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num < s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str < s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num < s._num) && (_str < s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator<= (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num <= s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str <= s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num <= s._num) && (_str <= s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator> (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num > s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str > s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num > s._num) && (_str > s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator>= (const SdmIntStr &s) const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (_num >= s._num ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (_str >= s._str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((_num >= s._num) && (_str >= s._str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

SdmBoolean SdmIntStr::operator! () const
{
  switch (_collateMode) {
  case Sdm_ISCS_None:
    return(Sdm_False);
  case Sdm_ISCS_IntOnly:
    return (!_num  ? Sdm_True : Sdm_False);
  case Sdm_ISCS_StrOnly:
    return (!_str ? Sdm_True : Sdm_False);
  case Sdm_ISCS_Both:
    return ((!_num) && (!_str) ? Sdm_True: Sdm_False);
  default:
    assert(0);
    return(Sdm_False);
  }
}

ostream & operator << (ostream &strm, const SdmIntStr &ss)
{
  strm << "( " << ss._num << " , " << ss._str << " )";
    return strm;
}
     

//---------------------------------------------------------------

//
// String String Class
//
SdmStrStr::SdmStrStr()
  :_nStr(), _dStr()
{
}

SdmStrStr::~SdmStrStr()
{
}

SdmStrStr::SdmStrStr(const SdmStrStr& dubStr)
  :_nStr(dubStr._nStr), _dStr(dubStr._dStr)
{
}

SdmStrStr& SdmStrStr::operator=(const SdmStrStr& dubStr)
{
  _nStr = dubStr._nStr;
  _dStr = dubStr._dStr;
  return *this;
}

SdmBoolean SdmStrStr::operator== (const SdmStrStr &s) const
{
  if ((_nStr == s._nStr) && (_dStr == s._dStr))
    return Sdm_True;
  else
    return Sdm_False;
}

SdmBoolean SdmStrStr::operator!= (const SdmStrStr &s) const
{
  if ((_nStr != s._nStr) && (_dStr != s._dStr))
    return Sdm_True;
  else
    return Sdm_False;
}

SdmBoolean SdmStrStr::operator< (const SdmStrStr &s) const
{
  if ((_nStr < s._nStr) && (_dStr < s._dStr))
    return Sdm_True;
  else
    return Sdm_False;
}

SdmBoolean SdmStrStr::operator> (const SdmStrStr &s) const
{
  if ((_nStr > s._nStr) && (_dStr > s._dStr))
    return Sdm_True;
  else
    return Sdm_False;
}

ostream & operator << (ostream &strm, const SdmStrStr &ss)
{
  strm << "( " << ss._nStr << " , " << ss._dStr << " )";
    return strm;
}
     


