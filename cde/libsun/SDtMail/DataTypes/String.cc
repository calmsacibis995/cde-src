/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmString class.
 
#pragma ident "@(#)String.cc	1.58 97/06/17 SMI"

//-----------------------------------------------------------------------------
//[Include Files
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/RepSWMR.hh>
#include <DataTypes/StringRep.hh>
#include <Utils/TnfProbe.hh>
#include <SDtMail/SystemUtility.hh>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

const unsigned int SdmBoyerMoore::delta_size = 256;
SdmStringRep *Sdm_NullReturn = 0;

//----------------------------------------------------------------------------

//
// Initialize globals used by all string objects
//
void Sdm_StringInit()
{
  Sdm_NullReturn = new SdmStringRep("");
}
#pragma init(Sdm_StringInit)

// Copy constructor
SdmString::SdmString (const SdmString& s) : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  p = s.p;
  if (p != Sdm_NullReturn)
    p->count++;
  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Additional constructor
SdmString::SdmString (const char* s)  : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  // If a non-null string pointer is specified AND if the first byte is
  // not a null character, then create a rep for this new string

  if (s && *s) {
    p = new SdmStringRep ((const char*)s);
    if (!p) {
      p = Sdm_NullReturn;
      _memoryError = Sdm_True;
    }
    else {
      p->count = 1;
    }
  }
  else {
    p = Sdm_NullReturn;
  }

  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Constructor to init size n with char c
SdmString::SdmString (char c, SdmStringLength n) : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  // If a non-zero count is specified, create the new string rep

  if (n) {
    p = new SdmStringRep (c, n);
    if (!p) {
      p = Sdm_NullReturn;
      _memoryError = Sdm_True;
    }
    else {
      p->count = 1;
    }
  }
  else {
    p = Sdm_NullReturn;	
  }

  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Constructor to set string to n characters in c
SdmString::SdmString (const char* c, SdmStringLength n)  : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  // If a non-zero count is specified, create the new string rep

  if (n) {
    p = new SdmStringRep (c, n);
    if (!p) {
      p = Sdm_NullReturn;
      _memoryError = Sdm_True;
    }
    else {
      p->count = 1;
    }
  }
  else {
    p = Sdm_NullReturn;
  }

  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Constructor to set string to n string s with length stringLength and size allocationSize

SdmString::SdmString (char** s, SdmStringLength stringLength, size_t allocationSize)  : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  // If a non-zero count is specified, create the new string rep

  if (stringLength) {
    p = new SdmStringRep (s, stringLength, allocationSize, SdmStringRep::_MS_MALLOC);
    if (!p) {
      p = Sdm_NullReturn;
      _memoryError = Sdm_True;
    }
    else {
      p->count = 1;
    }
  }
  else {
    p = Sdm_NullReturn;
  }

  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Given a pointer to an array of wide characters and the number of
// wide characters in that wide character stream, construct a string
// that converts those wide characters back into a multi-byte standard
// character stream

SdmString::SdmString (const wchar_t* ws, SdmStringLength n) : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_widecharp_length_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_widecharp_length_start");

  // Algorithm: copy the wide character fragment into a separate
  // buffer (nws), determine the number of 8-bit bytes necessary to
  // represent the wide character fragment as a multi-byte string,
  // allocate storage for such a multi-byte string, convert the wide
  // character fragment to the multi-byte string, and then cause a rep
  // to be allocated that has the multi-byte string as its contents.

  if (n) {

    int nwsbytes = n*sizeof(wchar_t);
    int nwsbytesP = nwsbytes+sizeof(wchar_t);
    wchar_t* nws = (wchar_t*)malloc(nwsbytesP);
    if (!nws) {
      _memoryError = Sdm_True;
    }
    else {
      memset(nws, 0L, nwsbytesP);
      memcpy((void*)nws, (const void*)ws, nwsbytes);
      size_t nbytes = wcstombs(NULL, nws, nwsbytes);
      if (nbytes <= 0) {
	// wcstombs failed - invalid characters in the multibyte stream! Great...
      }
      else {
	char* nmb = (char *)malloc(nbytes);
	if (!nmb) {
	  _memoryError = Sdm_True;
	  free(nws);
	}
	else {
	  size_t tbytes = wcstombs(nmb, nws, nbytes);
	  p = new SdmStringRep (nmb, tbytes);
	  free(nmb);
	  free(nws);
	  if (!p) {
	    p = Sdm_NullReturn;
	    _memoryError = Sdm_True;
	  }
	  else {
	    p->count = 1;
	  }
	}
      }
    }
  }
  else {
    p = Sdm_NullReturn;
  }

  assert(p);

  TNF_PROBE_0 (SdmString_constructor_widecharp_length_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_widecharp_length_end");
}

// Private constructor
SdmString::SdmString (size_t allocationSize, char** r) : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  p = new SdmStringRep (r, allocationSize);
  if (!p) {
    p = Sdm_NullReturn;
    _memoryError = Sdm_True;
  }
  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

// Private constructor
SdmString::SdmString (SdmStringLength n, size_t allocationSize, char** r)  : _memoryError(Sdm_False)
{
  TNF_PROBE_0 (SdmString_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_start");

  p = new SdmStringRep (r, n, allocationSize, SdmStringRep::_MS_ADREP);
  if (!p) {
    p = Sdm_NullReturn;
    _memoryError = Sdm_True;
  }
  assert(p);

  TNF_PROBE_0 (SdmString_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_constructor_end");
}

SdmString::~SdmString ()
{
  TNF_PROBE_0 (SdmString_destructor_start,
	       "api SdmString",
	       "sunw%debug SdmString_destructor_start");

  assert(p);
  if (p != Sdm_NullReturn && (--(p->count) <= 0))
    {
      delete p;
      p = Sdm_NullReturn;
    }

  TNF_PROBE_0 (SdmString_destructor_end,
	       "api SdmString",
	       "sunw%debug SdmString_destructor_end");
}

// overload new and delete
void*
SdmString::operator new(size_t size)
{
  TNF_PROBE_0 (SdmString_new_start,
	       "api SdmString",
	       "sunw%debug SdmString_new_start");

  void* return_value = SdmMemoryRepository::_ONew(size, Sdm_MM_String);

  TNF_PROBE_0 (SdmString_new_end,
	       "api SdmString",
	       "sunw%debug SdmString_new_end");

  return(return_value);
}

void*
SdmString::operator new(size_t size, void* mAdd)
{
  TNF_PROBE_0 (SdmString_new_start,
	       "api SdmString",
	       "sunw%debug SdmString_new_start");

  void* return_value = SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_String);

  TNF_PROBE_0 (SdmString_new_end,
	       "api SdmString",
	       "sunw%debug SdmString_new_end");

  return(return_value);
}

void
SdmString::operator delete(void* mAdd)
{
  TNF_PROBE_0 (SdmString_delete_start,
	       "api SdmString",
	       "sunw%debug SdmString_delete_start");

  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_String);

  TNF_PROBE_0 (SdmString_delete_end,
	       "api SdmString",
	       "sunw%debug SdmString_delete_end");
}

#if 0
// Boolean not operator
int SdmString::operator! () const
{
  TNF_PROBE_0 (SdmString_notoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_notoperator_start");

  assert(p);
  int ret_val = !(p->len);

  TNF_PROBE_0 (SdmString_notoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_notoperator_end");

  return (ret_val);
}
#endif

// Assignment operator - assign to an existing SdmString object
SdmString &SdmString::operator=(const SdmString& q)
{
  TNF_PROBE_0 (SdmString_assignoperatorr_start,
	       "api SdmString",
	       "sunw%debug SdmString_assignoperatorr_start");

  assert(p);

  // check for assigment to self
  if (this == &q)
    return *this;

  if (p != Sdm_NullReturn && (--(p->count) <= 0) && p != q.p)
    delete p;
  p = q.p;
  if (p != Sdm_NullReturn)
    p->count++;
  assert(p);

  TNF_PROBE_0 (SdmString_assignoperatorr_end,
	       "api SdmString",
	       "sunw%debug SdmString_assignoperatorr_end");
  return *this;
}

// Assignment operator - assign to an existing const char* object
SdmString &SdmString::operator=(const char* q)
{
  TNF_PROBE_0 (SdmString_assignoperatorr_start,
	       "api SdmString",
	       "sunw%debug SdmString_assignoperatorr_start");

  assert(p);

  // remove reference to this rep and remove rep if last
  if (p != Sdm_NullReturn && (--(p->count) <= 0))
    delete p;

  // create new stringrep from given char*
  if (q)
    {
      p = new SdmStringRep ((char*) q);
      if (!p) {
	p = Sdm_NullReturn;
	_memoryError = Sdm_True;
      }
      else {
	p->count = 1;
      }
    }
  else
    {
      p = Sdm_NullReturn;
    }
  assert(p);

  TNF_PROBE_0 (SdmString_assignoperatorr_end,
	       "api SdmString",
	       "sunw%debug SdmString_assignoperatorr_end");

  return *this;
}

// Concatenate two strings
SdmString SdmString::operator+ (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_plusoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_plusoperator_start");

  assert(p);
  if (p != Sdm_NullReturn)
    {
      if (s.p != Sdm_NullReturn)
	{
	  size_t allocationSize = s.p->len + p->len + 1;
	  char* buf = (char *)SdmStringRep::_AllocateRep(allocationSize);
	  if (!buf) {
	    SdmString retval (*this);
	    retval._memoryError = Sdm_True;
	    TNF_PROBE_0 (SdmString_plusoperator_end,
			 "api SdmString",
			 "sunw%debug SdmString_plusoperator_end");
	    return retval;
	  }
	  ::memcpy (buf, p->rep, p->len);
	  ::memcpy (buf+p->len, s.p->rep, s.p->len);
	  buf[s.p->len + p->len] = '\0';
	  SdmString retval (s.p->len + p->len, allocationSize, &buf);
	  // don't delete buf.  It is now owned by retval.
	  return retval;
	}
      else
	{
	  SdmString retval (*this);
	  return retval;
	}
    }
  else
    {
      if (s.p != Sdm_NullReturn)
	{
	  SdmString retval (s);
	  return retval;
	}
      else
	{
	  SdmString retval ((char *)NULL);	// GMG WHAT TO DO HERE?
	  return retval;
	}
    }

  TNF_PROBE_0 (SdmString_plusoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_plusoperator_end");
}

// Concatenate two strings
SdmString SdmString::operator+ (const char* s) const
{
  TNF_PROBE_0 (SdmString_plusoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_plusoperator_start");

  assert(p);
  if (p != Sdm_NullReturn)
    {
      if (s)
	{
	  SdmStringLength slen = strlen(s);
	  size_t allocationSize = slen + p->len + 1;
	  char* buf = (char *)SdmStringRep::_AllocateRep(allocationSize);
	  if (!buf) {
	    SdmString retval (*this);
	    retval._memoryError = Sdm_True;
	    TNF_PROBE_0 (SdmString_plusoperator_end,
			 "api SdmString",
			 "sunw%debug SdmString_plusoperator_end");
	    return retval;
	  }
	  ::memcpy (buf, p->rep, p->len);
	  ::memcpy (buf+p->len, s, slen);
	  buf[p->len + slen] = '\0';
	  SdmString retval (p->len+slen, allocationSize, &buf);
	  // don't delete buf.  It is now owned by retval.
	  return retval;
	}
      else
	{
	  SdmString retval (*this);
	  return retval;
	}
    }
  else
    {
      if (s)
	{
	  SdmString retval (s);
	  return retval;
	}
      else
	{
	  SdmString retval ((char *)NULL);	// GMG WHAT TO DO HERE
	  return retval;
	}
    }

  TNF_PROBE_0 (SdmString_plusoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_plusoperator_end");
}

// Concatenate and assign two strings
SdmString& SdmString::operator+= (const SdmString &s)
{
  TNF_PROBE_0 (SdmString_plusassignoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_plusassignoperator_start");

  assert(p);
  SdmString q(*this + s);
  if (p != Sdm_NullReturn && (--(p->count) <= 0))
    delete p;
  p = q.p;
  if (p != Sdm_NullReturn)
    p->count++;
  assert(p);

  TNF_PROBE_0 (SdmString_plusassignoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_plusassignoperator_end");

  return (*this);
}

// Concatenate and assign two strings
SdmString& SdmString::operator+= (const char* s)
{
  TNF_PROBE_0 (SdmString_plusassignoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_plusassignoperator_start");

  assert(p);
  SdmString q(*this + (char*)s);
  if (p != Sdm_NullReturn && (--(p->count) <= 0))
    delete p;
  p = q.p;
  if (p != Sdm_NullReturn)
    p->count++;
  assert(p);

  TNF_PROBE_0 (SdmString_plusassignoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_plusassignoperator_end");
  return (*this);
}

// This method provides for initializing a string that is represented by
// a file in a read only fashion. This allows efficient mechanisms (such as mmap)
// to be used as opposed to malloc() and read()
SdmErrorCode SdmString::MapAgainstFile(SdmError& err, 
				   const int fd,
				   const size_t numberOfBytes)
{
  TNF_PROBE_0 (SdmString_MapAgainstFile_start,
	       "api SdmString",
	       "sunw%debug SdmString_MapAgainstFile_start");

  // Reset the error object
  err = Sdm_EC_Success;

  assert(p);

  // remove reference to this rep and remove rep if last
  if (p != Sdm_NullReturn && (--(p->count) <= 0))
    delete p;

  // create new stringrep 
  p = new SdmStringRep ();
  if (!p) {
    p = Sdm_NullReturn;
    _memoryError = Sdm_True;
    err = Sdm_EC_NoMemory;
  }
  else {
    p->count = 1;

    // Map this rep against the file
    p->MapAgainstFile(err, fd, numberOfBytes);

  }
  assert(p);

  TNF_PROBE_0 (SdmString_MapAgainstFile_end,
	       "api SdmString",
	       "sunw%debug SdmString_MapAgainsdtFile_end");
  return(err);
}

// Concatenate two strings
void SdmString::Append (const char* str, SdmStringLength strLength)
{
  TNF_PROBE_0 (SdmString_Append_start,
	       "api SdmString",
	       "sunw%debug SdmString_Append_start");

  assert(p);
  if (strLength > 0) {
    if (p != Sdm_NullReturn)
      {
	// create new string buffer with appended string.
	SdmStringLength newLength = strLength + p->len;
	size_t allocationSize = newLength + 1;
	char* buf = (char *)SdmStringRep::_AllocateRep(allocationSize);
	if (!buf) {
	  _memoryError = Sdm_True;
	  TNF_PROBE_0 (SdmString_Append_end,
		       "api SdmString",
		       "sunw%debug SdmString_plusoperator_end");
	  return;
	}
	::memcpy (buf, p->rep, p->len);
	::memcpy (buf+p->len, str, strLength);
	buf[newLength] = '\0';
      
	// decrement reference count to current string.
	if (--(p->count) <= 0)
	  {
	    delete p;
	    p = NULL;
	  }
      
	// create string rep using out new string buffer.
	p = new SdmStringRep (&buf, newLength);
	if (!p) {
	  p = Sdm_NullReturn;
	  _memoryError = Sdm_True;
	  SdmStringRep::_DeallocateRep(buf, allocationSize);
	}
	else {
	  p->count = 1;
	}

	// DON"T DELETE BUFFER.  it's referenced by StringRep now.
      }
    else {
      p = new SdmStringRep (str, strLength);
      if (!p) {
	p = Sdm_NullReturn;
	_memoryError = Sdm_True;
      }
      else {
	p->count = 1;
      }
    }
  }
  assert(p);

  TNF_PROBE_0 (SdmString_Append_end,
	       "api SdmString",
	       "sunw%debug SdmString_plusoperator_end");
}

#if 0
// Compare strings - less than
int SdmString::operator< (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_ltoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_ltoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Less);

  TNF_PROBE_0 (SdmString_ltoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_ltoperator_end");

  return (ret_val);
}

// Compare strings - less than
int SdmString::operator< (const char* s) const
{
  TNF_PROBE_0 (SdmString_ltoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_ltoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Less);

  TNF_PROBE_0 (SdmString_ltoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_ltoperator_end");

  return (ret_val);
}

// Compare strings - greater than
int SdmString::operator> (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_gtoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_gtoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Greater);

  TNF_PROBE_0 (SdmString_gtoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_gtoperator_end");

  return (ret_val);
}

// Compare strings - greater than
int SdmString::operator> (const char* s) const
{
  TNF_PROBE_0 (SdmString_gtoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_gtoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Greater);

  TNF_PROBE_0 (SdmString_gtoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_gtoperator_end");

  return (ret_val);
}

// Compare strings - less than or equal to
int SdmString::operator<= (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_lteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_lteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Greater);

  TNF_PROBE_0 (SdmString_lteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_lteqoperator_end");

  return (ret_val);
}

// Compare strings - less than or equal to
int SdmString::operator<= (const char* s) const
{
  TNF_PROBE_0 (SdmString_lteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_lteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Greater);

  TNF_PROBE_0 (SdmString_lteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_lteqoperator_end");

  return (ret_val);
}

// Compare strings - greater than or equal to
int SdmString::operator>= (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_gteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_gteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Less);

  TNF_PROBE_0 (SdmString_gteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_gteqoperator_end");

  return (ret_val);
}

// Compare strings - greater than or equal to
int SdmString::operator>= (const char* s) const
{
  TNF_PROBE_0 (SdmString_gteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_gteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Less);

  TNF_PROBE_0 (SdmString_gteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_gteqoperator_end");

  return (ret_val);
}

// Compare strings - equal
int SdmString::operator== (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_eqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_eqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Equal);

  TNF_PROBE_0 (SdmString_eqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_eqoperator_end");

  return (ret_val);
}

// Compare strings - equal
int SdmString::operator== (const char* s) const
{
  TNF_PROBE_0 (SdmString_eqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_eqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) == SdmString::Sdm_Equal);

  TNF_PROBE_0 (SdmString_eqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_eqoperator_end");

  return (ret_val);
}

// Compare strings - not equal
int SdmString::operator!= (const SdmString &s) const
{
  TNF_PROBE_0 (SdmString_noteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_noteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Equal);

  TNF_PROBE_0 (SdmString_noteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_noteqoperator_end");

  return (ret_val);
}

// Compare strings - not equal
int SdmString::operator!= (const char* s) const
{
  TNF_PROBE_0 (SdmString_noteqoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_noteqoperator_start");

  assert(p);
  int ret_val = (Compare (*this, s) != SdmString::Sdm_Equal);

  TNF_PROBE_0 (SdmString_noteqoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_noteqoperator_end");

  return (ret_val);
}
#endif

// indexing
char SdmString::operator() (SdmStringLength pos) const
{
  TNF_PROBE_0 (SdmString_indexoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_indexoperator_start");

  assert(p);
  if (p != Sdm_NullReturn)
    {
      if (pos >= p->len)
	{
	  TNF_PROBE_0 (SdmString_indexoperator_end,
		       "api SdmString",
		       "sunw%debug SdmString_indexoperator_end");
	  return '\x00';
	}
      else
	{
	  TNF_PROBE_0 (SdmString_indexoperator_end,
		       "api SdmString",
		       "sunw%debug SdmString_indexoperator_end");
	  return p->rep[pos];
	}
    }
  else
    {
      TNF_PROBE_0 (SdmString_indexoperator_end,
		   "api SdmString",
		   "sunw%debug SdmString_indexoperator_end");
      return '\x00';
    }
}

// indexing
char &SdmString::operator() (SdmStringLength pos)
{
  TNF_PROBE_0 (SdmString_indexoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_indexoperator_start");

  // Make a copy of the string before
  // returning the index which can
  // be modified.
  assert(p);
  if (p != Sdm_NullReturn)
    {
      if (p->count > 1)
	{
	  SdmString q(p->rep);
	  if (--(p->count) <= 0)
	    delete p;
	  p = q.p;
	  if (p != Sdm_NullReturn)
	    p->count++;
	}
      assert(p);
      TNF_PROBE_0 (SdmString_indexoperator_end,
		   "api SdmString",
		   "sunw%debug SdmString_indexoperator_end");
      return (p->rep[pos]);
    }
  else
    assert(0);	// can't recover from this one

  assert(p);
  TNF_PROBE_0 (SdmString_indexoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_indexoperator_end");
}

// indexing
SdmString SdmString::operator() (SdmStringLength pos, SdmStringLength length) const
{
  TNF_PROBE_0 (SdmString_indexoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_indexoperator_start");

  assert(p);
  if (p != Sdm_NullReturn)
    {
      size_t allocationSize = length+1;
      char* buf = (char *)SdmStringRep::_AllocateRep(allocationSize);
      if (!buf) {
	SdmString retval ((char *)NULL);
	retval._memoryError = Sdm_True;
	TNF_PROBE_0 (SdmString_indexoperator_end,
		     "api SdmString",
		     "sunw%debug SdmString_indexoperator_end");
	return retval;
      }
      ::memset(buf, 0, length+1);
      ::strncpy (buf, &(p->rep[pos]), length);
      buf[length] = '\0';
      SdmString retval (allocationSize, &buf);
      TNF_PROBE_0 (SdmString_indexoperator_end,
		   "api SdmString",
		   "sunw%debug SdmString_indexoperator_end");
      return retval;
    }
  assert(p);

  TNF_PROBE_0 (SdmString_indexoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_indexoperator_end");
}

// overload the output operator
ostream & operator << (ostream &strm, const SdmString &s)
{
  assert(s.p);
  if (s.p != Sdm_NullReturn)
    strm << *(s.p);

  return strm;
}
 
/* To be implemented
istream & operator << (istream &strm, const SdmString &s)
{
return strm >> s;
}
*/

void SdmString::Print() const
{
  assert(p);
  if (p != Sdm_NullReturn)
    cout << p->rep << endl;
  else
    cout << Sdm_NullReturn << endl;
}

// Remove all occurrences of character c from the given string.
// Returns the number of characters removed.
int SdmString::StripCharacter(char *s, char c)
{
  char character, *p = s;
  int count = 0;

  if (s == NULL)
    return count;

  while (character = *p++) {
    if (character == c) {
      ++count;
      continue;
    }
    *s++ = character;
  }

  *s = '\0';

  return count;
}

// Remove all occurrences of character c from the given string and it's length.
// Returns the number of characters removed.
int SdmString::StripCharacter(char *s, char c, SdmStringLength len)
{
  char character, *p = s;
  int count = 0;

  if (s == NULL)
    return count;

  for (SdmStringLength i=0; i<len; i++) {
    character = *p++;
    if (character == c) {
      ++count;
      continue;
    }
    *s++ = character;
  }

  return count;
}


// Remove all occurrences of character c from the given string.
//
void SdmString::StripCharacter(char c)
{
  int count;

  assert(p);
  if (p == Sdm_NullReturn)
    return;

  if (p->count > 1) {
    SdmString q(p->rep, p->len);  // create copy of string.
    if (--(p->count) <= 0)
      delete p;
    p = q.p;
    if (p != Sdm_NullReturn)
      p->count++;
  }

  int charRemoved  = StripCharacter(p->rep, c, p->len);
  if (charRemoved) {
    p->len -= charRemoved;
    p->rep[p->len] = '\0';  // null terminate new string.  this is not done in StripCharacter.
  }

  // If we have removed the last character from this string, then deallocate the rep
  if (p->len == 0) {
    SdmStringRep::_DeallocateRep(p->rep,p->_allocation_size);
    p->rep = NULL;
    p->_allocation_size = 0;
  }
}

void SdmString::ConvertToLowerCase()
{
  TNF_PROBE_0 (SdmString_ConvertToLowerCase_start,
	       "api SdmString",
	       "sunw%debug SdmString_ConvertToLowerCase_start");

  assert(p);
  if (p != Sdm_NullReturn && p->len) {
    // See if we need to create a new rep: if the reference count is > 1 and we must change
    // the string, then create a new rep
    if (p->count > 1) {
      assert(p->rep);
      assert(p->len);
      char *pp = p->rep;
      for (int i = 0; i < p->len; i++, pp++)
	if (isupper(*pp)) {
	  SdmString q(p->rep);
	  if (--(p->count) <= 0)
	    delete p;
	  p = q.p;
	  if (p != Sdm_NullReturn) 
	    p->count++;
	  break;
	}
    }

    assert(p->rep);
    assert(p->len);
    char *pp = p->rep;
    for (int i = 0; i < p->len; i++, pp++) {
      if (isupper(*pp)) {
	*pp = tolower(*pp);
      }
    }
  }
  assert(p);

  TNF_PROBE_0 (SdmString_ConvertToLowerCase_end,
	       "api SdmString",
	       "sunw%debug SdmString_ConvertToLowerCase_end");

  return;
}

void SdmString::ConvertToUpperCase()
{
  TNF_PROBE_0 (SdmString_ConvertToUpperCase_start,
	       "api SdmString",
	       "sunw%debug SdmString_ConvertToUpperCase_start");

  assert(p);
  if (p != Sdm_NullReturn && p->len) {
    // See if we need to create a new rep: if the reference count is > 1 and we must change
    // the string, then create a new rep
    if (p->count > 1) {
      assert(p->rep);
      assert(p->len);
      char *pp = p->rep;
      for (int i = 0; i < p->len; i++, pp++)
	if (islower(*pp)) {
	  SdmString q(p->rep);
	  if (--(p->count) <= 0)
	    delete p;
	  p = q.p;
	  if (p != Sdm_NullReturn) 
	    p->count++;
	  break;
	}
    }

    assert(p->rep);
    assert(p->len);
    char *pp = p->rep;
    for (int i = 0; i < p->len; i++, pp++) {
      if (islower(*pp)) {
	*pp = toupper(*pp);
      }
    }
  }
  assert(p);

  TNF_PROBE_0 (SdmString_ConvertToUpperCase_end,
	       "api SdmString",
	       "sunw%debug SdmString_ConvertToUpperCase_end");

  return;
}

void SdmString::SetRange(SdmStringLength pos, char *s)
{
  TNF_PROBE_0 (SdmString_SetRange_start,
	       "api SdmString",
	       "sunw%debug SdmString_SetRange_start");

  assert(p);

  if (!s)
    return;

  SdmStringLength slen = strlen (s);
  if ((slen + pos) > p->len)
    assert(0);  // overwrote buffer

  // Make a copy of the string that
  // is being modified.
  if (p != Sdm_NullReturn)
    {
      if (p->count > 1)
	{
	  SdmString q(p->rep);
	  if (--(p->count) <= 0)
	    delete p;
	  p = q.p;
	  if (p != Sdm_NullReturn)
	    p->count++;
	}
      ::memcpy (&(p->rep[pos]), s, slen);
    }
  else
    assert(0);	// can't recover from this one
  assert(p);

  TNF_PROBE_0 (SdmString_SetRange_end,
	       "api SdmString",
	       "sunw%debug SdmString_SetRange_end");
}

// search for substrings
SdmStringLength SdmString::Find (const SdmString& s, unsigned int nth_match, 
	  SdmString::SdmComparisonMode caseChk) const
{
  TNF_PROBE_0 (SdmString_Find_start,
	       "api SdmString",
	       "sunw%debug SdmString_Find_start");

  if (s.p == Sdm_NullReturn || (!(s.p->len)))
    {
      TNF_PROBE_0 (SdmString_Find_end,
		   "api SdmString",
		   "sunw%debug SdmString_Find_end");
      return Sdm_NPOS;
    }

  SdmStringLength ret_pos;
  SdmBoyerMoore bm (s, caseChk);

  if (BoyerMooreFind (bm, nth_match, ret_pos, caseChk))
    {
      TNF_PROBE_0 (SdmString_Find_end,
		   "api SdmString",
		   "sunw%debug SdmString_Find_end");
      return ret_pos;
    }
  else
    {
      TNF_PROBE_0 (SdmString_Find_end,
		   "api SdmString",
		   "sunw%debug SdmString_Find_end");
      return Sdm_NPOS;
    }
}

// Overload -> for reference counting
//	overloading -> causes this function to be
//	called and its results to be dereferenced.
//	eg. for a->b 	
//		invoke A::operator->() on a
//		return temporary x
//		evaluate x->b
SdmStringRep *SdmString::operator-> () const
{
  TNF_PROBE_0 (SdmString_derefoperator_start,
	       "api SdmString",
	       "sunw%debug SdmString_derefoperator_start");

  assert(p);

  TNF_PROBE_0 (SdmString_derefoperator_end,
	       "api SdmString",
	       "sunw%debug SdmString_derefoperator_end");
  return p;
}

SdmBoolean SdmString::BoyerMooreFind (const SdmBoyerMoore &bm,
	unsigned int nth_match, SdmStringLength& ret_pos,
	SdmString::SdmComparisonMode caseChk) const
{
  TNF_PROBE_0 (SdmString_BoyerMooreFind_start,
	       "api SdmString",
	       "sunw%debug SdmString_BoyerMooreFind_start");

  int nmatch;
  assert(p);
  const unsigned char *bp = (unsigned char *)p->rep;
  const unsigned char *ep = (unsigned char *)(p->rep + p->len);
  const unsigned char *cp = (unsigned char *)p->rep;
  for (nmatch = 0; nmatch < nth_match;) {
    unsigned char *mp = SdmString::BoyerMooreSearch(bm, cp, (ep-cp)+1);
    if (!mp)
      break;
    if (++nmatch == nth_match) {
      ret_pos = mp-bp-bm.patternLength+1;
      return Sdm_True;
    }
    cp = mp+1;
    if (cp > ep)
      break;
  }
  
  TNF_PROBE_0 (SdmString_BoyerMooreFind_end,
	       "api SdmString",
	       "sunw%debug SdmString_BoyerMooreFind_end");
  ret_pos = -1;
  return Sdm_False;
}

unsigned char* SdmString::BoyerMooreSearch (const SdmBoyerMoore& bm, 
			const unsigned char* target,
			SdmStringLength targetLength) const
{
  const unsigned char *k, *strend, *s, *targetLimit;
  int    t;
  int             j;

  if (bm.patternLength > targetLength)
    return NULL;

  targetLimit = target + targetLength;
  if (targetLength > bm.patternLength * 4)
    strend = targetLimit - bm.patternLength * 4;
  else
    strend = target;

  s = target;
  k = target + bm.patternLength - 1;

  for (;;)
    {
      /* The dreaded inner loop, revisited. */
      while (k < strend && (t = bm.delta[*k]))
	{
	  k += t;
	  k += bm.delta[*k];
	  k += bm.delta[*k];
	}
      while (k < targetLimit && bm.delta[*k])
	++k;
      if (k == targetLimit)
	break;

      j = bm.patternLength - 1;
      s = k;
      while (--j >= 0 && bm.cmap[*--s] == bm.pmap[j])
	;

      /*
       * delta-less shortcut for literati, but short shrift for genetic engineers.
       */
      if (j >= 0)
	k++;
      else							/* submatch */
	return ((unsigned char *) k);
    }
  return (NULL);
}

//-----------------------------------------------------------------------------

// Default constructor
//	allocate an empty string
SdmStringRep::SdmStringRep () 
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  len = 0;
  size_t allocationSize = len+1;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
  rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
  if (!rep) {
    _memoryError = Sdm_True;
    _allocation_size = 0;
  }
  else {
    *rep = '\0';
    _allocation_size = allocationSize;
  }

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Copy constructor
SdmStringRep::SdmStringRep (const SdmStringRep& s)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = s.len;
  size_t allocationSize = len+1;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
  rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
  if (!rep) {
    _memoryError = Sdm_True;
    len = 0;
    _allocation_size = 0;
  }
  else {
    ::memcpy (rep, s.rep, len + 1);
    rep[len] = '\0';
    _allocation_size = allocationSize;
  }

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Constructor to create an SdmStringRep from a char* 
SdmStringRep::SdmStringRep (const char* s)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  if (s)
    {
      // allocate a new string and copy s to it
      // using the outermost global strlen & strcpy functions
      len = ::strlen (s);
      size_t allocationSize = len + 1;
      _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
      rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
      if (!rep) {
	_memoryError = Sdm_True;
	len = 0;
	_allocation_size = 0;
      }
      else {
	::memcpy (rep, s, len);
	rep[len] = '\0';
	_allocation_size = allocationSize;
      }
    }
  else
    {
      len = 0;
      size_t allocationSize = len+1;
      _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
      rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
      if (!rep) {
	_memoryError = Sdm_True;
	_allocation_size = 0;
      }
      else {
	rep[len] = '\0';
	_allocation_size = allocationSize;
      }
    }
  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Constructor to init size n with char c
SdmStringRep::SdmStringRep (char c, SdmStringLength n)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = n;
  size_t allocationSize = len + 1;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
  rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
  if (!rep) {
    _memoryError = Sdm_True;
    len = 0;
    _allocation_size = 0;
  }
  else {
    ::memset (rep, c, len);
    rep[len] = '\0';
    _allocation_size = allocationSize;
  }

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Constructor to set string to n characters in c
SdmStringRep::SdmStringRep (const char* c, SdmStringLength n)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = n;
  size_t allocationSize = len+1;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
  rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
  if (!rep) {
    _memoryError = Sdm_True;
    len = 0;
    _allocation_size = 0;
  }
  else {
    ::memcpy (rep, c, len);
    rep[len] = '\0';
    _allocation_size = allocationSize;
  }

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Constructor to create a reference copy
SdmStringRep::SdmStringRep (char**  const r, size_t allocationSize)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  rep = *r;
  *r = 0;
  count = 1;
  len = strlen(rep);
  _allocation_size = allocationSize;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}

// Constructor to create a reference copy
SdmStringRep::SdmStringRep (char** r, SdmStringLength theLength, size_t allocationSize, MemAllocationStyle ms)
  : _memoryError(Sdm_False), _map_length(0)
{
  TNF_PROBE_0 (SdmStringRep_constructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_start");

  rep = *r;
  *r = 0;
  count = 1;
  len = theLength;
  _allocation_size = allocationSize;
  _memAllocStyle = ms;	// allocated via _AllocRep

  TNF_PROBE_0 (SdmStringRep_constructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_constructor_end");
}


// Destructor
SdmStringRep::~SdmStringRep ()
{
  TNF_PROBE_0 (SdmStringRep_destructor_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_destructor_start");
  if (rep) {
    switch (_memAllocStyle) {
    case SdmStringRep::_MS_MMAP:
      // delete the internal string
      assert(_map_length);
      // This rep is based on a mmap() region of a file - unmap it
      (void) munmap(rep, _map_length);
      break;
    case SdmStringRep::_MS_ADREP:
      // This rep is based on a new[] area - delete it
      SdmStringRep::_DeallocateRep((void *)rep, _allocation_size);
      break;
    case SdmStringRep::_MS_MALLOC:
      free((void *)rep);
      break;
    default:
      assert(0);
    }
  }

  TNF_PROBE_0 (SdmStringRep_destructor_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_destructor_end");
}

// overload new and delete
void*
SdmStringRep::operator new(size_t size)
{
  TNF_PROBE_0 (SdmStringRep_new_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_new_start");

  void* result = SdmMemoryRepository::_ONew(size, Sdm_MM_StringRep);

  TNF_PROBE_0 (SdmStringRep_new_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_new_end");

  return(result);
}
 
void*
SdmStringRep::operator new(size_t size, void* mAdd)
{
  TNF_PROBE_0 (SdmStringRep_new_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_new_start");

  void* result = SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_StringRep);

  TNF_PROBE_0 (SdmStringRep_new_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_new_end");

  return (result);
}
 
void
SdmStringRep::operator delete(void* mAdd)
{
  TNF_PROBE_0 (SdmStringRep_delete_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_delete_start");

  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_StringRep);

  TNF_PROBE_0 (SdmStringRep_delete_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_delete_end");
}

// overload the output operator
ostream & operator << (ostream &strm, const SdmStringRep &s)
{
  strm << s.rep;
  return strm;
}

void* SdmStringRep::_AllocateRep(const size_t allocationSize)
{
  TNF_PROBE_0 (SdmStringRep__AllocationRep_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep__AllocationRep_start");

  void* mAdd = SdmMemoryRepository::_ONew(allocationSize, 
					  allocationSize >= 2048 ? Sdm_MM_AnonymousAllocation : Sdm_MM_General);

  TNF_PROBE_0 (SdmStringRep__AllocationRep_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep__AllocationRep_end");

  return(mAdd);
}

void SdmStringRep::_DeallocateRep(void* mAdd, const size_t allocationSize)
{
  TNF_PROBE_0 (SdmStringRep__DeallocateRep_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep__DeallocateRep_start");

  if (mAdd) {
    SdmMemoryRepository::_ODel(mAdd, 
			       allocationSize >= 2048 ? Sdm_MM_AnonymousAllocation : Sdm_MM_General);
  }

  TNF_PROBE_0 (SdmStringRep__DeallocateRep_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep__DeallocateRep_end");
  return;
}

// This method provides for initializing a string that is represented by
// a file in a read only fashion. This allows efficient mechanisms (such as mmap)
// to be used as opposed to malloc() and read()
SdmErrorCode SdmStringRep::MapAgainstFile(SdmError& err, 
				   const int fd,
				   const size_t numberOfBytes)
{
  TNF_PROBE_0 (SdmStringRep_MapAgainstFile_start,
	       "api SdmString",
	       "sunw%debug SdmStringRep_MapAgainstFile_start");

  assert(!_map_length);

  // Deallocate previous contents of this rep (if any)
  if (rep) {
    switch (_memAllocStyle) {
    case SdmStringRep::_MS_MMAP:
      // delete the internal string
      assert(_map_length);
      // This rep is based on a mmap() region of a file - unmap it
      (void) munmap(rep, _map_length);
      _map_length = 0;
      break;
    case SdmStringRep::_MS_ADREP:
      // This rep is based on a new[] area - delete it
      SdmStringRep::_DeallocateRep((void *)rep, _allocation_size);
      break;
    case SdmStringRep::_MS_MALLOC:
      free((void *)rep);
      break;
    default:
      assert(0);
    }
    rep = 0;
    _memAllocStyle = SdmStringRep::_MS_ADREP;
    assert(_map_length == 0);
  }

  // Map the file range as indicated into a region in memory
  int page_size = SdmSystemUtility::HardwarePageSize();
  size_t map_size = (size_t) (numberOfBytes + (page_size - (numberOfBytes % page_size)));
  void* map_region = mmap(NULL, map_size, PROT_READ, MAP_PRIVATE|MAP_NORESERVE, fd, (off_t)0);
  if (map_region != MAP_FAILED) {
    // Remember what we have just mapped
    rep = (char*)map_region;			// location of mmap()ed data
    len = numberOfBytes;			// number of bytes of "data" mapped
    _allocation_size = len;
    _map_length = map_size;			// number of bytes in the mmaped region
    _memAllocStyle = SdmStringRep::_MS_MMAP;
    // Provide advice to the VM system on how we are going to reference the region:
    // MADV_SEQUENTIAL: Tell the system that addresses in this range are likely
    // to be accessed only once, so the system will free resources mapping the 
    // address range as quickly as possible (e.g. aggressive free behind, as well 
    // as sufficient read ahead to allow quick processing).
    (void) madvise(rep, _map_length, MADV_SEQUENTIAL);
    // Indicate that no error occurred
    TNF_PROBE_0 (SdmStringRep_MapAgainstFile_end,
		 "api SdmString",
		 "sunw%debug SdmStringRep_MapAgainstFile_end");
    return(err = Sdm_EC_Success);			// no error encountered
  }

  // The mmap failed - try and allocate the storage and read the data directly in.
  len = numberOfBytes;
  size_t allocationSize = len + 1;
  _memAllocStyle = SdmStringRep::_MS_ADREP;	// allocated via _AllocRep
  rep = (char *)SdmStringRep::_AllocateRep(allocationSize);
  if (!rep) {
    _memoryError = Sdm_True;
    err = Sdm_EC_NoMemory;
    len = 0;
    _allocation_size = 0;
  }
  else {
    if ( (SdmSystemUtility::SafeLseek(fd, 0L, SEEK_SET) == 0)
	 && SdmSystemUtility::SafeRead(fd, (void*)rep, (size_t)len) == len) {
      rep[len] = '\0';
      _allocation_size = allocationSize;
      err = Sdm_EC_Success;
    }
    else {
      err = (errno == ENOMEM ? Sdm_EC_NoMemory : Sdm_EC_Fail);
      _memoryError = Sdm_True;
      SdmStringRep::_DeallocateRep((void *)rep, allocationSize);
      rep = 0;
      len = 0;
      _allocation_size = 0;
    }
  }

  TNF_PROBE_0 (SdmStringRep_MapAgainstFile_end,
	       "api SdmString",
	       "sunw%debug SdmStringRep_MapAgainstFile_end");

  return(err);
}

SdmString::SdmComparisonValue SdmString::Compare (
	const SdmString &r, const char *s, 
	SdmString::SdmComparisonMode caseChk) 
{
  TNF_PROBE_0 (SdmString_Compare_start,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_start");

  SdmString::SdmComparisonValue result;

  result = SdmString::Sdm_Equal;
  if (r.p)
    if (s)
      result = Compare (r.p->rep, s, caseChk);
    else
      result = SdmString::Sdm_Greater;
  else
    if (s)
      result = SdmString::Sdm_Less;
    else
      result = SdmString::Sdm_Equal;

  TNF_PROBE_0 (SdmString_Compare_end,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_end");
  return (result);
}

SdmString::SdmComparisonValue SdmString::Compare (
	const char *r, const SdmString &s, 
	SdmString::SdmComparisonMode caseChk) 
{
  TNF_PROBE_0 (SdmString_Compare_start,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_start");

  SdmString::SdmComparisonValue result;

  result = SdmString::Sdm_Equal;
  if (r)
    if (s.p)
      result = Compare (r, s.p->rep, caseChk);
    else
      result = SdmString::Sdm_Greater;
  else
    if (s.p)
      result = SdmString::Sdm_Less;
    else
      result = SdmString::Sdm_Equal;

  TNF_PROBE_0 (SdmString_Compare_end,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_end");
  return (result);
}

SdmString::SdmComparisonValue SdmString::Compare (
	const SdmString &r, const SdmString &s, 
	SdmString::SdmComparisonMode caseChk) 
{
  TNF_PROBE_0 (SdmString_Compare_start,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_start");

  SdmString::SdmComparisonValue result;

  result = SdmString::Sdm_Equal;
  if (r.p)
    if (s.p)
      result = Compare (r.p->rep, s.p->rep, caseChk);
    else
      result = SdmString::Sdm_Greater;
  else
    if (s.p)
      result = SdmString::Sdm_Less;
    else
      result = SdmString::Sdm_Equal;

  TNF_PROBE_0 (SdmString_Compare_end,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_end");
  return (result);
}

SdmString::SdmComparisonValue SdmString::Compare (
	const char *r, const char *s, 
	SdmString::SdmComparisonMode caseChk) 
{
  TNF_PROBE_0 (SdmString_Compare_start,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_start");

  SdmStringLength minlen;
  char c1, c2;
  SdmStringLength i;
  SdmStringLength rlen, slen;
  SdmString::SdmComparisonValue result;
  int comp;

  rlen = ::strlen (r);
  slen = ::strlen (s);
  minlen = (rlen < slen ? rlen : slen);

  // Determine case sensitivity & compare
  if (caseChk == SdmString::Sdm_Ignore)
    comp = ::strncasecmp (r, s, minlen);
  else
    comp = ::strncmp (r, s, minlen);

  // Determine the results of the compare
  result = SdmString::Sdm_Equal;  // Start out equal
  if (comp < 0)
    result = SdmString::Sdm_Less;
  else if (comp > 0)
    result = SdmString::Sdm_Greater;
  else if (rlen < slen)
    result = SdmString::Sdm_Less;
  else if (slen < rlen)
    result = SdmString::Sdm_Greater;
  else if (slen == rlen)
    result = SdmString::Sdm_Equal;

  TNF_PROBE_0 (SdmString_Compare_end,
	       "api SdmString",
	       "sunw%debug SdmString_Compare_end");
  return (result);
}


//-----------------------------------------------------------------------------

SdmBoyerMoore::SdmBoyerMoore (const SdmString &pat, SdmString::SdmComparisonMode caseChk)
  : pattern (pat)
{
  TNF_PROBE_0 (SdmBoyerMoore_constructor_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_constructor_start");

  SdmStringLength i;

  // get length of pattern
  patternLength = pattern.p->len;

  // allocate pattern map
  pmap = new unsigned char [patternLength];

  // allocate delta table
  delta = new unsigned int [delta_size];

  // allocate character map
  cmap = new unsigned char [delta_size];

  // setup the pattern
  if (caseChk == SdmString::Sdm_Ignore) {
    // fold case while saving pattern
    for (i = 0; i < patternLength; i++)
      pmap[i] = (isupper((int) pattern[i]) ?
		 (char) tolower((int) pattern[i]) : pattern[i]);
  }
  else {
    ::memcpy(pmap, pat, patternLength);
  }

  // clear table
  for (i=0; i<delta_size; ++i) {
    delta[i] = patternLength;
    cmap[i] = (char) i;
  }

  // set table values
  for (i = 0; i < patternLength - 1; i++)
    delta[pmap[i]] = patternLength - i - 1;
  delta[pmap[patternLength - 1]] = 0;

  // set value for last pattern character
  delta[(unsigned int)pattern(patternLength-1)] = 0;

  if (caseChk == SdmString::Sdm_Ignore) {
    for (i = 0; i < patternLength - 1; i++)
      if (islower((int) pmap[i]))
	delta[toupper((int) pmap[i])] = patternLength - i - 1;
    if (islower((int) pmap[patternLength - 1]))
      delta[toupper((int) pmap[patternLength - 1])] = 0;
    for (i = 'A'; i <= 'Z'; i++)
      cmap[i] = (char) tolower((int) i);
  }

  TNF_PROBE_0 (SdmBoyerMoore_constructor_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_constructor_end");
}

SdmBoyerMoore::SdmBoyerMoore (const SdmBoyerMoore &bm)
  : pattern(bm.pattern)
{
  TNF_PROBE_0 (SdmBoyerMoore_constructor_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_constructor_start");

  // allocate delta table
  delta = new unsigned int [delta_size];

  // copy contents of source
  memcpy (delta, bm.delta, delta_size);

  TNF_PROBE_0 (SdmBoyer_constructor_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyer_constructor_end");
}

SdmBoyerMoore::~SdmBoyerMoore()
{
  TNF_PROBE_0 (SdmBoyerMoore_destructor_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_destructor_start");

  if (delta)
    delete [] delta;

  if (pmap)
    delete [] pmap;

  if (cmap)
    delete [] cmap;
  TNF_PROBE_0 (SdmBoyerMoore_destructor_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_destructor_end");
}

// overload new and delete
void*
SdmBoyerMoore::operator new(size_t size)
{
  TNF_PROBE_0 (SdmBoyerMoore_new_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_new_start");

  TNF_PROBE_0 (SdmBoyerMoore_new_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_new_end");
  return SdmMemoryRepository::_ONew(size, Sdm_MM_General);
}
 
void*
SdmBoyerMoore::operator new(size_t size, void* mAdd)
{
  TNF_PROBE_0 (SdmBoyerMoore_new_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_new_start");

  TNF_PROBE_0 (SdmBoyerMoore_new_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_new_end");
  return SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_General);
}
 
void
SdmBoyerMoore::operator delete(void* mAdd)
{
  TNF_PROBE_0 (SdmBoyerMoore_delete_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_delete_start");

  TNF_PROBE_0 (SdmBoyerMoore_delete_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_delete_end");

  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_General);
}

SdmBoyerMoore &SdmBoyerMoore::operator= (const SdmBoyerMoore &bm)
{
  TNF_PROBE_0 (SdmBoyerMoore_assignoperator_start,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyerMoore_assignoperator_start");

  // check for assigment to self
  if (this == &bm) return *this;

  // destroy old delta
  if (delta) 
    delete [] delta;

  // allocate delta table
  delta = new unsigned int [bm.delta_size];

  // copy contents of source
  memcpy (delta, bm.delta, delta_size*sizeof(unsigned int));

  // destroy old cmap
  if (cmap) 
    delete [] cmap;

  // allocate cmap table
  cmap = new unsigned char [bm.delta_size];

  // copy contents of source
  memcpy (cmap, bm.cmap, delta_size);

  // destroy old pattern map, then create a new copy of the original
  if (pmap)
    delete [] pmap;

  patternLength = bm.patternLength;
  pmap = new unsigned char [patternLength];

  memcpy(pmap, bm.pmap, patternLength);

  TNF_PROBE_0 (SdmBoyer_assignoperator_end,
	       "api SdmBoyerMoore",
	       "sunw%debug SdmBoyer_assignoperator_end");
}
