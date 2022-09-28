/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmStr class.
//      The SdmStr and SdmStrRep classes organize the system's
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

#pragma ident "@(#)Str.hh	1.5 96/07/16 SMI"

#ifndef _SDM_STR_H
#define _SDM_STR_H

#include <SDtMail/Sdtmail.hh>

// if returned, means the string search was bad.
//static const unsigned int Sdm_NPOS            = ~(unsigned int)0;


//---------------------------------------------------------------------------
class SdmStrRep;
class ostream;
class istream;
class BM;

class SdmStr : public SdmLinkedListElement {
friend class StringRep;
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
  SdmStr ();
  SdmStr (const SdmStr& s);
  SdmStr (const char *s);
  SdmStr (char, int);
  virtual ~SdmStr ();

  // equal operator
  SdmStr operator= (const SdmStr &q);

  // concatenation operators
  //	the two + operators are a performance consideration so that
  //	the copy constructor call is avoided.
  SdmStr operator+ (const SdmStr &s) const;
  SdmStr operator+ (const char *s) const;
  SdmStr operator+= (const SdmStr &s);
  SdmStr operator+= (const char *s);

  // comparison operators
  int operator< (const SdmStr &s) const;
  int operator> (const SdmStr &s) const;
  int operator<= (const SdmStr &s) const;
  int operator>= (const SdmStr &s) const;
  int operator== (const SdmStr &s) const;
  int operator!= (const SdmStr &s) const;

  // subscripting operators
  // return a copy of a character in the string
  char operator() (int pos) const;
  // return a reference to a character in the string
  //   so the string can be modified
  char &operator() (int pos);
  // return the string subscripted by this range
  SdmStr operator() (int pos, int length) const; 

  // conversion operator
  operator const char * () const;

  // access to SdmStrRep parts
  SdmStrRep* operator-> () const;

  // stream operators
  friend ostream & operator<< (ostream &strm, const SdmStr &s);
  // add istream operator if needed
  // friend istream & operator<< (istream &strm, const SdmStr &s);

public:
  SdmStr (char** r);
  SdmStrRep *p;

};

//---------------------------------------------------------------------------

class SdmStrRep {
friend SdmStr;
public:
  // constructors and destructor
  SdmStrRep ();				// default constructor
  SdmStrRep (const SdmStrRep& s);		// copy constructor
  ~SdmStrRep ();				// destructor
  SdmStrRep (const char *s);			// constructor from char *
  SdmStrRep (char, int);			// cons size int, init to char
  // SdmStrRep (char fill_char, int count);	// fixed size constructor
  // SdmStrRep (int max_size, const char *format, ...); // formatted

  // concatenation operators
  SdmStr operator+ (const SdmStr& s) const;
  SdmStr operator+ (const char *s) const;

  // comparison operators
  int operator< (const SdmStr& s) const;
  int operator> (const SdmStr& s) const;
  int operator<= (const SdmStr& s) const;
  int operator>= (const SdmStr& s) const;
  int operator== (const SdmStr& s) const;
  int operator!= (const SdmStr& s) const;

  // subscripting operators
  char operator() (int pos) const;
  char &operator() (int pos);
  SdmStr operator() (int pos, int length) const;

  // conversion operator
  operator const char * () const;

  // stream operators
  friend ostream & operator<< (ostream &strm, const SdmStrRep &s);

  // other member functions
  int length () const;
  int size() const;
  void print () const;

  // compare strings
  SdmStr::SdmComparisonValue compare (const SdmStr &s, 
    SdmStr::SdmComparisonMode caseChk = SdmStr::Sdm_Sensitive) const;
  // search for substrings
  int find (const SdmStr &s, int nth_match = 0, 
    SdmStr::SdmComparisonMode caseChk = SdmStr::Sdm_Sensitive) const;
  SdmBoolean BMFind (const BM &bm,
    unsigned int nth_match, unsigned int &ret_pos,
    SdmStr::SdmComparisonMode caseChk = SdmStr::Sdm_Sensitive) const;

private:
  // internal constructor
  SdmStrRep (char ** const r);

  char *rep;					// the string
  int count;					// reference count
  int len;					// actual number of chars
};

//---------------------------------------------------------------------------

class BM
{
  public:
    // constructors
    BM (const SdmStr &pattern);
    BM (const BM &bm);

    // destructor
    ~BM();

    // assignment operator
    BM operator= (const BM &bm);

    // get character from pattern string
    char operator() (unsigned int index) const;

    // get delta value form table
    unsigned int GetDelta (char ch) const;

    // get length of pattern used to create table
    unsigned int GetPatternLength() const;

  private:
    // size of delta table
    static const unsigned int delta_size;
    // pointer to delta table
    unsigned int *delta;
    SdmStr pattern;
};

#endif
