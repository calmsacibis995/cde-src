/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmString class.
//      The SdmString and SdmStringRep classes organize the system's
//      string data.  They are an implementation of the counted pointer
//      idiom.  This idiom provides for reference counting and has the
//      advantages of concrete data types:  they can be passes as params,
//      assigned created, destroyed, etc.  They also bring properties
//      associated with pointers:  addressing objects dynamically allocated
//      on the heap.  They are instantiated without using new and deallocated
//      without using delete.  They behave like automatics & statics.
//
//      Counted pointers act like pointers that have constructors.  They
//      are initialized automatically to point to a newly allocated object
//      when they come into scope.
//
//      e.g.
//      String a("abcd"), b("efgh");
//      printf ("a is "); a->print();
//      printf ("length of b is %d\n", b->length());
//      printf ("concatenation is ", (a+b)->print());
//

#ifndef _SDM_STRING_H
#define _SDM_STRING_H

#pragma ident "@(#)String.hh	1.41 97/03/24 SMI"

#include <SDtMail/RepSWMR.hh>

// this typedef defines the maximum size of a string

typedef unsigned long SdmStringLength;	// full 2^32 bits: 4,294,967,296 bytes

// if returned, means the string search was bad.

static const SdmStringLength Sdm_NPOS = ~(SdmStringLength)0;

//---------------------------------------------------------------------------

class ostream;
class istream;
class SdmBoyerMoore;

//---------------------------------------------------------------------------

class SdmStringRep {
public:
  // constructors and destructor
  SdmStringRep ();				// default constructor
  SdmStringRep (const SdmStringRep& s);		// copy constructor
  ~SdmStringRep ();				// destructor
  SdmStringRep (const char* s);	// constructor from char *
  SdmStringRep (char, SdmStringLength);		// cons size int, init to char
  SdmStringRep (const char* s, SdmStringLength n);	// init to n characters from s
  // SdmStringRep (char fill_char, int count);	// fixed size constructor
  // SdmStringRep (int max_size, const char *format, ...); // formatted

  // overload new & delete operators
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

  // Map data into string object from a file
  SdmErrorCode MapAgainstFile(SdmError& err, const int fd, const size_t numberOfBytes);

  // stream operators
  friend ostream&  operator<< (ostream& strm, const SdmStringRep& s);
 
  // return indication of whether or not object is in the out of memory error state
  SdmBoolean MemoryError() { return _memoryError; }

private:
  friend class SdmString;
  friend class SdmBoyerMoore;

  typedef enum { _MS_MMAP, _MS_ADREP, _MS_MALLOC } MemAllocationStyle;

  // internal constructor
  SdmStringRep (char** const r, size_t allocationSize);
  SdmStringRep (char** r, SdmStringLength theLength, size_t allocationSize, MemAllocationStyle ms);

  // Memory Allocation Functions
  static void* _AllocateRep(const size_t allocationSize);	
  static void _DeallocateRep(void* mAdd, const size_t allocationSize);

  char* 		rep;				// the string
  int 			count;				// reference count
  SdmStringLength	len;				// actual number of chars
  size_t		_allocation_size;		// number of bytes allocated to the string
  size_t		_map_length;			// if mmap()ed: length of mapping 
  SdmBoolean		_memoryError;			// string operator out of memory error
  MemAllocationStyle	_memAllocStyle;
};

//---------------------------------------------------------------------------

class SdmStringRep;
class SdmBoyerMoore;
class ostream;
class istream;

extern SdmStringRep *Sdm_NullReturn;

class SdmString {
public:
  enum SdmComparisonValue
  {
    Sdm_Less,
    Sdm_Equal,
    Sdm_Greater,
    Sdm_Error
  };

  // Case sensitivity values
  enum SdmComparisonMode 
  { 
    Sdm_Sensitive, 
    Sdm_Ignore 
  };

  // constructors and destructor
  SdmString () {
    p = Sdm_NullReturn;
    _memoryError = Sdm_False;
    assert(p);
  }
  SdmString (const SdmString& s);
  SdmString (const char* s);
  SdmString (char, SdmStringLength);
  SdmString (const char* s, SdmStringLength n);
  SdmString (char** s, SdmStringLength stringLength, size_t allocationSize);
  SdmString (const wchar_t* ws, SdmStringLength n);
  ~SdmString ();

  // overload new & delete operators
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

  // equal operator
  SdmString& operator= (const SdmString& q);
  SdmString& operator= (const char* q);

  // concatenation operators
  //	the two + operators are a performance consideration so that
  //	the copy constructor call is avoided.
  SdmString operator+ (const SdmString& s) const;
  SdmString operator+ (const char* s) const;
  SdmString& operator+= (const SdmString& s);
  SdmString& operator+= (const char* s);

  // comparison operators
  int operator< (const SdmString& s) const {
    // Compare strings - less than
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Less);
  }

  int operator< (const char* s) const {
    // Compare strings - less than
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Less);
  }

  int operator> (const SdmString& s) const {
    // Compare strings - greater than
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Greater);
  }

  int operator> (const char* s) const {
    // Compare strings - greater than
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Greater);
  }

  int operator<= (const SdmString& s) const {
    // Compare strings - less than or equal to
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Greater);
  }

  int operator<= (const char* s) const {
    // Compare strings - less than or equal to
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Greater);
  }

  int operator>= (const SdmString& s) const {
    // Compare strings - greater than or equal to
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Less);
  }

  int operator>= (const char* s) const {
    // Compare strings - greater than or equal to
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Less);
  }

  int operator== (const SdmString& s) const {
    // Compare strings - equal
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Equal);
  }

  int operator== (const char* s) const {
    // Compare strings - equal
    assert(p);
    return (Compare (*this, s) == SdmString::Sdm_Equal);
  }

  int operator!= (const SdmString& s) const {
    // Compare strings - not equal
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Equal);
  }

  int operator!= (const char* s) const {
    // Compare strings - not equal
    assert(p);
    return (Compare (*this, s) != SdmString::Sdm_Equal);
  }

  int operator! () const {
    // Boolean not operator
    assert(p);
    return !(p->len);
  }

  // subscripting operators
  // return a copy of a character in the string
  char operator() (SdmStringLength pos) const;

  // return a reference to a character in the string
  //   so the string can be modified
  char& operator() (SdmStringLength pos);

  // return the string subscripted by this range
  SdmString operator() (SdmStringLength pos, SdmStringLength length) const; 

  // conversion operator
  operator const char*  () const {
    assert(p);
    return (const char*) p->rep;
  }

  // stream operators
  friend ostream&  operator<< (ostream& strm, const SdmString& s);

  // add istream operator if needed
  // friend istream&  operator<< (istream& strm, const SdmString& s);

  // Member functions

  // Length of string
  SdmStringLength Length () const {
    assert(p);
    return p->len;
  }

  // Map data into string object from a file
  SdmErrorCode MapAgainstFile(SdmError& err, const int fd, const size_t numberOfBytes);

  // Appends character string with given length to string.
  void Append (const char* str, SdmStringLength strLength);

  // Print contents of string to standard out
  void Print () const;

  // Force the case of the string one way or another
  void ConvertToUpperCase();
  void ConvertToLowerCase();

  // Change the value of the specified range
  void SetRange (SdmStringLength pos, char *s);

  // Remove all occurrences of character c
  static int StripCharacter(char *s, char c);
  void StripCharacter(char c);
  static int SdmString::StripCharacter(char *s, char c, SdmStringLength len);

  // compare strings
  static SdmString::SdmComparisonValue Compare (
						const SdmString &r, const SdmString &s, 
						SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive);
  static SdmString::SdmComparisonValue Compare (
						const SdmString &r, const char *s, 
						SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive);
  static SdmString::SdmComparisonValue Compare (
						const char *r, const SdmString &s, 
						SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive);
  static SdmString::SdmComparisonValue Compare (
						const char *r, const char *s, 
						SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive);

  // compare strings ignore case
  SdmString::SdmComparisonValue CaseCompare (const SdmString& s) const 
  { return(Compare(*this, s, SdmString::Sdm_Ignore)); } 

  // search for substrings
  SdmStringLength Find (const SdmString& s, unsigned int nth_match = 0, 
			SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive) const;

  // search for substrings ignore case
  SdmStringLength CaseFind (const SdmString& s, unsigned int nth_match = 0) const
  { return(Find(s, nth_match, SdmString::Sdm_Ignore)); }

  // return indication of whether or not object is in the out of memory error state
  SdmBoolean MemoryError() { return _memoryError; }

private:
  // These guys reach into the SdmStringRep item when working on an SdmString
  friend class SdmStringRep;
  friend class SdmBoyerMoore;
  friend void Sdm_StringInit();

  // search for substrings (more efficient if same substring repeated search)
  SdmBoolean BoyerMooreFind (const SdmBoyerMoore& bm,
			     unsigned int nth_match, SdmStringLength& ret_pos,
			     SdmString::SdmComparisonMode caseChk = SdmString::Sdm_Sensitive) const;
 
  // search for substrings (more efficient if same substring repeated search)
  unsigned char* BoyerMooreSearch (const SdmBoyerMoore& bm, 
				   const unsigned char* buffer,
				   SdmStringLength buflen) const;
 
  // access to SdmStringRep parts
  SdmStringRep* operator-> () const;

  // internal constructor
  SdmString (size_t allocationSize, char** r);
  SdmString (SdmStringLength n, size_t allocationSize, char** r);
  
  // This is the SdmStringRep that contains the real character string data
  SdmStringRep* p;

  // This is an indication of whether or not the string operator ran out of memory
  SdmBoolean _memoryError;
};

//---------------------------------------------------------------------------
 
class SdmBoyerMoore
{
public:
  // constructors
  SdmBoyerMoore (const SdmString& pattern, SdmString::SdmComparisonMode caseChk);
  SdmBoyerMoore (const SdmBoyerMoore& bm);
 
  // destructor
  ~SdmBoyerMoore();
 
  // overload new & delete operators
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);
 
  // assignment operator
  SdmBoyerMoore& operator= (const SdmBoyerMoore& bm);

private:
  friend class SdmString;
  friend class SdmBoyerMoore;

  // size of delta table
  static const unsigned int delta_size;
  // pointer to delta table
  unsigned int* delta;
  unsigned char* cmap;
  unsigned char* pmap;
  SdmStringLength patternLength;
  SdmString pattern;
};
 
#endif
