#if 0
/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// -->  This is the SdmStringRep class.
//      This class implements the "reference string" that the
//	SdmString reference objects actually point to. This class
//	is the container for the actual character string data and
//	operators on that data.
//
 

#ifndef _SDM_STRINGREP_H
#define _SDM_STRINGREP_H

#pragma ident "@(#)StringRep.hh	1.12 97/03/24 SMI"

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
#endif
