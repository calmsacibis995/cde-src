/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmStr class.
 
#pragma ident "@(#)Str.cc	1.5 96/07/24 SMI"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream.h>
#include <assert.h>
#include "Str.hh"

const unsigned int BM::delta_size = 256;

SdmStr::SdmStr ()
  : SdmLinkedListElement(this)
{
  p = new SdmStrRep();
  p->count = 1;
}

// Copy constructor
SdmStr::SdmStr (const SdmStr& s)
  : SdmLinkedListElement(this)
{
  p = s.p;
  p->count++;
}

// Additional constructor
SdmStr::SdmStr (const char *s)
  : SdmLinkedListElement(this)
{
  p = new SdmStrRep(s);
  p->count = 1;
}

// Constructor to init size n with char c
SdmStr::SdmStr (char c, int n)
  : SdmLinkedListElement(this)
{
  p = new SdmStrRep(c, n);
  p->count = 1;
}

// Private constructor
SdmStr::SdmStr (char **r)
  : SdmLinkedListElement(this)
{
  p = new SdmStrRep(r);
}

SdmStr::~SdmStr ()
{
  if (--(p->count) <= 0)
    delete p;
}

// Assignment operator
SdmStr SdmStr::operator=(const SdmStr& q)
{
  // check for assigment to self
  if (this == &q) return *this;

  if (--(p->count) <= 0 && p != q.p)
    delete p;
  (p = q.p)->count++;
  return *this;
}

// Concatenate two strings
SdmStr SdmStr::operator+ (const SdmStr &s) const
{
  return *p + s;
}

// Concatenate two strings
SdmStr SdmStr::operator+ (const char *s) const
{
  return *p + s;
}

// Concatenate and assign two strings
SdmStr SdmStr::operator+= (const SdmStr &s)
{
  SdmStr q(*p + s);
  if (--(p->count) <= 0)
    delete p;
  (p = q.p)->count++;
  return (*this);
}

// Concatenate and assign two strings
SdmStr SdmStr::operator+= (const char *s)
{
  SdmStr q(*p + s);
  if (--(p->count) <= 0)
    delete p;
  (p = q.p)->count++;
  return (*this);
}

// Compare strings - less than
int SdmStr::operator< (const SdmStr &s) const
{
  return *p < s;
}

// Compare strings - greater than
int SdmStr::operator> (const SdmStr &s) const
{
  return *p > s;
}

// Compare strings - less than or equal to
int SdmStr::operator<= (const SdmStr &s) const
{
  return *p <= s;
}

// Compare strings - greater than or equal to
int SdmStr::operator>= (const SdmStr &s) const
{
  return *p >= s;
}

// Compare strings - equal
int SdmStr::operator== (const SdmStr &s) const
{
  return *p == s;
}

// Compare strings - not equal
int SdmStr::operator!= (const SdmStr &s) const
{
  return *p != s;
}

// indexing - return a copy of a char in the string
char SdmStr::operator() (int pos) const
{
  return (*p)(pos);
}

// indexing - return a reference to a char in the string
char &SdmStr::operator() (int pos)
{
  // Make a copy of the string before
  // returning the index which can
  // be modified.
  if (p->count > 1)
  {
    SdmStr q(*p);
    if (--(p->count) <= 0)
      delete p;
    (p = q.p)->count++;
  }
  return (*p)(pos);
}

// indexing - return substring in range
SdmStr SdmStr::operator() (int pos, int length) const
{
  return (*p)(pos, length);
}

// Conversion operator
SdmStr::operator const char * () const
{
  return (const char *)*p;
}

// overload the output operator
ostream & operator << (ostream &strm, const SdmStr &s)
{
  strm << *(s.p);
  return strm;
}

/* To be implemented
istream & operator << (istream &strm, const SdmStr &s)
{
  return strm >> s;
}
*/

// Overload -> for reference counting
//	overloading -> causes this function to be
//	called and its results to be dereferenced.
//	eg. for a->b 	
//		invoke A::operator->() on a
//		return temporary x
//		evaluate x->b
SdmStrRep *SdmStr::operator-> () const
{
  return p;
}

// Default constructor
//	allocate an empty string
SdmStrRep::SdmStrRep ()
{
  *(rep = new char[1]) = '\0';
  if (rep == NULL)
    assert (0);
  len = 0;
}

// Copy constructor
SdmStrRep::SdmStrRep (const SdmStrRep& s)
{
  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = s.len;

  rep = new char[::strlen(s.rep) + 1];
  if (rep == NULL)
    assert (0);

  ::memcpy (rep, s.rep, len + 1);
  rep[len] = '\0';
}

// Constructor to create an SdmStrRep from a char *
SdmStrRep::SdmStrRep (const char *s)
{
  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = ::strlen (s);

  rep = new char[len + 1];
  if (rep == NULL)
    assert (0);

  ::memcpy (rep, s, len);
  rep[len] = '\0';
}

// Constructor to init size n with char c
SdmStrRep::SdmStrRep (char c, int n)
{
  // allocate a new string and copy s to it
  // using the outermost global strlen & strcpy functions
  len = n;

  rep = new char[len + 1];
  if (rep == NULL)
    assert (0);

  ::memset (rep, c, len);
  rep[len] = '\0';
}

// Constructor to create a reference copy
SdmStrRep::SdmStrRep (char ** const r)
{
  rep = *r;
  *r = 0;
  count = 1;
  len = strlen(rep);
}


// Destructor
SdmStrRep::~SdmStrRep ()
{
  // delete the internal string
  delete [] rep;
}

// Concatenation operator
SdmStr SdmStrRep::operator+ (const SdmStr& s) const
{
  char *buf = new char[s->len + len + 1];
  ::memset(buf, 0, s->len + len + 1);
  ::strcpy (buf, rep);
  ::strcat(buf, s->rep);
  SdmStr retval (&buf);
  return retval;
}

// Concatenation operator
SdmStr SdmStrRep::operator+ (const char *s) const
{
  char *buf = new char[strlen(s) + len + 1];
  ::memset(buf, 0, strlen(s) + len + 1);
  ::strcpy (buf, rep);
  ::strcat(buf, s);
  SdmStr retval (&buf);
  return retval;
}

// Compare string - less than
int SdmStrRep::operator< (const SdmStr& s) const
{
  return (compare (s) == SdmStr::Sdm_Less);
}

// Compare string - less than
int SdmStrRep::operator> (const SdmStr& s) const
{
  return (compare (s) == SdmStr::Sdm_Greater);
}

// Compare string - less than
int SdmStrRep::operator<= (const SdmStr& s) const
{
  return (compare (s) != SdmStr::Sdm_Greater);
}

// Compare string - less than
int SdmStrRep::operator>= (const SdmStr& s) const
{
  return (compare (s) != SdmStr::Sdm_Less);
}

// Compare string - less than
int SdmStrRep::operator== (const SdmStr& s) const
{
  return (compare (s) == SdmStr::Sdm_Equal);
}

// Compare string - less than
int SdmStrRep::operator!= (const SdmStr& s) const
{
  return (compare (s) != SdmStr::Sdm_Equal);
}

// indexing
char SdmStrRep::operator() (int pos) const
{
  if (pos >= len)
    return '\x00';
  else
    return rep[pos];
}

// indexing
char &SdmStrRep::operator() (int pos)
{
  return (rep[pos]);
}

// indexing
SdmStr SdmStrRep::operator() (int pos, int length) const
{
  char *buf = new char[length + 1];
  ::memset(buf, 0, length+1);
  ::strncpy (buf, &(rep[pos]), length);
  buf[length] = '\0';
  SdmStr retval (&buf);
  return retval;
}

// Conversion operator
SdmStrRep::operator const char * () const
{
  return (const char *)rep;
}

ostream & operator << (ostream &strm, const SdmStrRep &s)
{
  strm << s.rep;
  return strm;
}

int SdmStrRep::length () const
{
  return len;
}

void SdmStrRep::print () const
{
  cout << rep << endl;
}

SdmStr::SdmComparisonValue SdmStrRep::compare (const SdmStr &s, 
	SdmStr::SdmComparisonMode caseChk) const
{
  int min_len;
  char c1, c2;
  int i;

  if (s->len < len)
    min_len = s->len;
  else
    min_len = len;

  if (caseChk == SdmStr::Sdm_Ignore)
  {
    // case insensitive
    for (i=0; i<min_len; ++i)
    {
      // convert both to lowercase
      c1 = (char) ::tolower (rep[i]);
      c2 = (char) ::tolower (s->rep[i]);

      if (c1 != c2)
      {
        if (c1 < c2)
          return SdmStr::Sdm_Less;
        else
          return SdmStr::Sdm_Greater;
      }
    }
  }
  else
  {
    for (i=0; i<min_len; ++i)
    {
      c1 = rep[i];
      c2 = s->rep[i];

      if (c1 != c2)
      {
        if (c1 < c2)
          return SdmStr::Sdm_Less;
        else
          return SdmStr::Sdm_Greater;
      }
    }
  }

  // If we reach here, the strings are equal provided one isn't
  // a substring of the other
  if (len == s->len)
  {
    return SdmStr::Sdm_Equal;
  }
  else
  {
    if (len < s->len)
      return SdmStr::Sdm_Less;
    else
      return SdmStr::Sdm_Greater;
  }
}

int SdmStrRep::find (const SdmStr &s, int pos,
	SdmStr::SdmComparisonMode caseChk) const
{
  unsigned int ret_pos;
  BM bm (s);

  if (BMFind (bm, pos, ret_pos, caseChk))
  {
    return ret_pos;
  }
  else
  {
    return Sdm_NPOS;
  }
}

SdmBoolean SdmStrRep::BMFind (const BM &bm,
	unsigned int nth_match, unsigned int &ret_pos,
	SdmStr::SdmComparisonMode caseChk) const
{
  char c1, c2;
  unsigned int i, j, patlen;
  int times = 1;
  SdmBoolean found = Sdm_False;

  // store pattern length locally (it gets used a lot)
  patlen = bm.GetPatternLength();

  // i is the index into the target
  i = patlen;

  while (i <= len)
  {
    // j is an index into pattern
    j = patlen;
 
    // convert both to lowercase
    if (caseChk == SdmStr::Sdm_Ignore)
    {
      c1 = (char) ::tolower (bm(j-1));
      c2 = (char) ::tolower (rep[i-1]);
    }
    else
    {
      c1 = bm(j-1);
      c2 = rep[i-1];
    }

    // while corresponding characters match
    found = Sdm_False;
    while ((c1 == c2) && !found)
    {

      if (j>1)
      {
	// move left one character for next comparison
	--j;
	--i;
      }
      else
      {
	// we have reached the beginning of the pattern
	// pattern found!
	if (times >= nth_match)
	{
	  ret_pos = i - 1;
	  return Sdm_True;
	}
	else
	// if we don't want the first match, continue
	// until we find the nth match.
	{
	  times++;
	  i++;
	  found = Sdm_True;
	}
      }

      // convert both to lowercase
      if (caseChk == SdmStr::Sdm_Ignore)
      {
        c1 = (char) ::tolower (bm(j-1));
        c2 = (char) ::tolower (rep[i-1]);
      }
      else
      {
        c1 = bm(j-1);
        c2 = rep[i-1];
      }
    }

    // move target index by delta value of
    //   mismatched character
    i += bm.GetDelta (rep[i-1]);
  }

  return Sdm_False;
}


BM::BM (const SdmStr &pat)
  : pattern (pat)
{
  // allocate delta table
  delta = new unsigned int [delta_size];

  if (delta == NULL)
    assert (0);

  // clear table
  unsigned int i;

  // get length of pattern
  unsigned int patlen = pattern->length();

  for (i=0; i<delta_size; ++i)
    delta[i] = patlen;

  // set table values
  for (i=1; i<patlen; ++i)
    delta[(unsigned int)pattern(i-1)] = patlen - i;

  // set value for last pattern character
  delta[(unsigned int)pattern(patlen-1)] = 1;
}

BM::BM (const BM &bm)
  : pattern(bm.pattern)
{
  // allocate delta table
  delta = new unsigned int [delta_size];

  if (delta == NULL)
    assert (0);

  // copy contents of source
  memcpy (delta, bm.delta, delta_size);
}

BM::~BM()
{
  if (delta)
    delete [] delta;
}

BM BM::operator= (const BM &bm)
{
  // check for assigment to self
  if (this == &bm) return *this;

  // destroy old delta
  if (delta) 
    delete [] delta;

  // allocate delta table
  delta = new unsigned int [bm.delta_size];

  // memory test
  if (delta == NULL)
    assert (0);

  // copy contents of source
  memcpy (delta, bm.delta, delta_size);
}

// get length of pattern used to create table
unsigned int BM::GetPatternLength() const
{
  return pattern->length();
}

char BM::operator() (unsigned int index) const
{
  return pattern(index);
}

unsigned int BM::GetDelta (char ch) const
{
  return delta[(unsigned int)(ch)];
}
