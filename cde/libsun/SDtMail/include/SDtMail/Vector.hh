/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmVector class.
// The SdmVector class is a template based vector class.
// The vector allocates no memory unless a size has been specified
// or until AddElementToList is called.  AddElementToList allocates
// chunks of memory and then assigns elements, only allocating more
// space when the vector is full.
// 
// To use SdmVector create a template instantiation for the object:
//   SdmVector<MyObject> my_vector;
//
// Vector indexing via the [] operator is provided but programmers
// should be careful when using this to assign values into the vector
// rather than AddElementToList.  Programmers must assure that the
// vector was either assigned a size at creation:  SdmVector (20);
// or that SetVectorSize() was called.  Also note that the sorted
// vectors derived from SdmVector use an insertion sort through
// AddElementToList.  If this is bypassed by the [] operator, then
// the vector will not be properly sorted.
//
// Vector indexing via the () operator is identical to indexing via the
// [] operator with the following two exceptions:
//   1 - it will not operate on a non-const object
//   2 - if the index is -1, then a new element is added to the end of
//       the list, and that elements index is then used in the operation;
//       this allows for the quick addition of elements to the vector:
//         For a base type vector:		vector(-1) = value
//	   For a SdmIntStrL tuple vector:	vector(-1).SetNumberAndString(0,"test")
//	   For a SdmStrStrL tuple vector:	vector(-1).SetBothStrings("first","second")
//         For a SdmStringL vector:		vector(-1) = "test"
//       While this may appear slightly "ugly", it is much more efficient
//       than:  vector[vector.AddElementToVector()] = value
//
// --> This is the SdmVectorIterator class.
// The SdmVectorIterator is provided as part of a parallel interface
// with SdmLinkedList.  This should make it easier to convert between
// the two if the need should arise.  Normally, it is easier to use
// a for loop and the [] operator to iterate through the vector.
//
 
#ifndef _SDM_VECTOR_H
#define _SDM_VECTOR_H

#pragma ident "@(#)Vector.hh	1.27 97/04/29 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDtMail/RepSWMR.hh>

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
template <class SVE>
class SdmVectorIterator;

template <class SVE>
class SdmVector {
friend SdmVectorIterator<SVE>;
public:
  // constructors
  // The number of elements in the vector may be specified, if not,
  // then the vector does not initially allocate memory.
  SdmVector ();
  SdmVector (const int max_elems);
  // copy constructor
  SdmVector (const SdmVector<SVE> &a);
  // destructor
  virtual ~SdmVector (void);
  // overload new & delete operators
  void* operator new(size_t size);
  void* operator new(size_t size, void* mAdd);
  void  operator delete(void* mAdd);

  // access functions

  // Reset the number of elements to 0, deallocate memory
  void ClearAllElements (void);

  // Allocate a vector of the specified size,
  // The vector contents will be retained up to the new size.
  // The number of elements in the vector is now considered to
  // be "size".
  // If setMaximumSizeOnly is Sdm_True, then this modifies the total number
  // of elements reserved for the vector but does not count them as added;
  // AddElementToVector can use these, and the element count does
  // not reflect them. Use this when the vector size is known before
  // it is filled up, in order to limit the number of GrowVector
  // operations necessary to fill up the vector.
  void SetVectorSize (const int size, const SdmBoolean setMaximumSizeOnly = Sdm_False);

  // Remove an element and shift the rest of the vector down.
  void RemoveElement (const int pos);

  // Insert an element into the vector. 
  void InsertElementAfter (const SVE &element, const int pos);

  // Insert an element into the vector. 
  void InsertElementBefore (const SVE &element, const int pos);

  // Append an element to the list.
  virtual int AddElementToList (const SVE &element);

  // Add an element to the end of the vector and return its [] index;
  // allows you to add like this: vec[vec.AddElementToVector()] = value;
  int AddElementToVector();

  // Match the give element with one in the vector and return the position.
  // If the element isn't found, Sdm_Not_Found (-1) is returned.
  int FindElement (const SVE &element);

  // Return the number of elements in the vector.
  int ElementCount(void) const { return num_elements; }

  // index operator
  // return a reference to the element at the given position
  // programmers should be careful when using this to assign values
  // into the vector
  SVE &operator[] (const int pos) const;

  // index operator
  // return a reference to the element at the given position
  // same as [] operator except for:
  //  -- will not work for 'const' object
  //  -- allows appending a value via: vec(-1) = x;
  SVE &operator() (const int pos);

  // assignment operator
  SdmVector &operator= (const SdmVector &a);

protected:

  // Increase the number of allocated elements in the vector
  void GrowVector (void);

  // grow the vector by this amount
  enum { Sdm_VECTOR_INCREMENT = 10 };

  SVE			*vector;	// the vector of elements
  int			num_elements;	// number of elements in vector
  int			max_elements;	// number of allocated elements
  int			increment;	// amount by which to grow the vector 


};

//---------------------------------------------------------------------------

template <class SVE>
class SdmVectorIterator {
public:
  // constructors and destructor
  SdmVectorIterator (SdmVector<SVE> *v)
    { index = 0; list = v; }
  ~SdmVectorIterator ()
    { }

  // Start at the beginning of the vector
  void SdmVectorIterator::ResetIterator()
    { index = 0; }

  // Return a reference to the next element in the vector
  SVE *SdmVectorIterator::NextListElement()
    { 
      if (index < list->ElementCount())
        return &(list->vector[index++]); 
      else
        return NULL;
    }
 
private:
  int index;			// an index to the current element
  SdmVector<SVE> *list;		// a pointer to the vector
};

//---------------------------------------------------------------------------

// This is the SdmVector template implementation

// Constructor
template <class SVE>
SdmVector<SVE>::SdmVector ()
  : num_elements(0), 
    max_elements(0),
    increment(Sdm_VECTOR_INCREMENT),
    vector(NULL)
{
}

// Alternate constructor takes the number of elements
// and allocates a vector of that size
template <class SVE>
SdmVector<SVE>::SdmVector (const int max_elems)
  : num_elements(0), 
    max_elements(max_elems),
    increment(Sdm_VECTOR_INCREMENT)
{
  vector = new SVE[max_elements];
}

// Copy constructor
template <class SVE>
SdmVector<SVE>::SdmVector (const SdmVector<SVE> &a)
{
  vector = new SVE [a.max_elements];
  for (int i=0; i<a.num_elements; i++)
    vector[i] = a.vector[i];

  max_elements = a.max_elements;
  num_elements = a.num_elements;
  increment = a.increment;
}

// Destructor
template <class SVE>
SdmVector<SVE>::~SdmVector()
{
  // Free the vector
  if (vector)
    delete [] vector; 
}

// overload new and delete
template <class SVE>
void*
SdmVector<SVE>::operator new(size_t size)
{
  return SdmMemoryRepository::_ONew(size, Sdm_MM_General);
}
 
template <class SVE>
void*
SdmVector<SVE>::operator new(size_t size, void* mAdd)
{
  return SdmMemoryRepository::_ONew(size, mAdd, Sdm_MM_General);
}
 
template <class SVE>
void
SdmVector<SVE>::operator delete(void* mAdd)
{
  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_General);
}

// assignment operator
template <class SVE>
SdmVector<SVE> &SdmVector<SVE>::operator= (const SdmVector<SVE> &a)
{
  // check for assignment to self
  if (this == &a) return *this;

  // if the a vector is empty, deallocate memory in this vector
  if (!a.max_elements)
  {
    if (max_elements)
      delete [] vector;
    max_elements = 0;
    num_elements = 0;
    increment = a.increment;
 
    return *this;
  }

  // if the vectors haven't allocated the same amount of memory,
  // delete this vector and allocate a new one with the same
  // amount of memory as the a vector.
  if (max_elements != a.max_elements)
  {
    if (max_elements)
      delete [] vector;
 
    vector = new SVE [a.max_elements];
  }

  // copy a vector's elements into this vector
  for (int i=0; i<a.num_elements; i++)
    vector[i] = a.vector[i];

  // copy remaining data
  max_elements = a.max_elements;
  num_elements = a.num_elements;
  increment = a.increment;
 
  return *this;
}

// index operators - allow element to be modified
//
template <class SVE>
SVE &SdmVector<SVE>::operator[] (const int pos) const
{
  if (pos < max_elements)
    return vector[pos];

  // access out of bounds
  assert (0);
  abort();
}

// index operators - allow element to be modified
// Special feature: an index of [-1] provides for a quick interface to
// allow a new element to be added to the end of the vector; it allows
// for the quick addition of an element by: vec[-1] = value
//
template <class SVE>
SVE &SdmVector<SVE>::operator() (const int pos)
{
  // Handle the -1 add position

  if (pos == -1) {
    // Allocate more elements if needed
    if (num_elements >= max_elements)
      GrowVector();

    // Return the index of the next element allocated on the vector
    return vector[num_elements++];
  }

  if (pos < max_elements)
    return vector[pos];

  // access out of bounds
  assert (0);
  abort();
}

// Empty the vector and deallocate memory
template <class SVE>
void SdmVector<SVE>::ClearAllElements ()
{
  num_elements = 0;
  max_elements = 0;
  if (vector)
    delete [] vector;
  vector = NULL;
}

// Allocate a vector of the specified size
template <class SVE>
void SdmVector<SVE>::SetVectorSize (const int size, const SdmBoolean setMaximumSizeOnly)
{
  int copy_num;

  // the number of elements to copy depends on
  // whether the vector is growing or shrinking
  if (num_elements < size)
    copy_num = num_elements;
  else
    copy_num = size;

  // allocate a new vector
  SVE *tmp_vector = new SVE [size];

  if (num_elements > size || setMaximumSizeOnly == Sdm_False)
    num_elements = size;
  max_elements = size;

  // copy the elements
  for (int i=0; i<copy_num; i++)
    tmp_vector[i] = vector[i];

  if (vector)
    delete [] vector;
 
  vector = tmp_vector;
}

// Remove element from list and shift vector <-
template <class SVE>
void SdmVector<SVE>::RemoveElement (const int pos)
{
  if (num_elements && (pos < num_elements))
  {
    // shift vector elements <-
    for (int i=pos; i<(num_elements-1); i++)
      vector[i] = vector[i+1];
    num_elements--;
  }
  else
  {
    assert (0);
    abort();
  }
}

// Remove element from list and shift vector <-
template <class SVE>
void SdmVector<SVE>::InsertElementAfter (const SVE &element, const int pos)
{
  if (pos > num_elements) {
    assert (0);
    abort();
  }

  // Allocate more elements if needed
  if (num_elements >= max_elements)
    GrowVector();

  // shift vector elements ->
  for (int i=(num_elements); i>(pos+1); i--)
    vector[i] = vector[i-1];
  vector[pos+1] = element;
  num_elements++;
}

// Remove element from list and shift vector <-
template <class SVE>
void SdmVector<SVE>::InsertElementBefore (const SVE &element, const int pos)
{
  if (pos > num_elements) {
    assert (0);
    abort();
  }

  // Allocate more elements if needed
  if (num_elements >= max_elements)
    GrowVector();

  // shift vector elements ->
  for (int i=(num_elements); i>(pos); i--)
    vector[i] = vector[i-1];
  vector[pos] = element;
  num_elements++;
}

template <class SVE>
void SdmVector<SVE>::GrowVector ()
{
#if 0
  // Allocate memory for "increment" elements at a time.
  // When this memory is exceeded, allocate
  // space for "increment" more.
  SVE *tmp_vector 
    = new SVE [max_elements + increment];
 
  for (int i=0; i<num_elements; i++)
    tmp_vector[i] = vector[i];

  if (vector)
    delete [] vector;
 
  vector = tmp_vector;
  max_elements += increment;
#else
  // Allocate memory for "double the number of existing elements" at a time.
  // When this memory is exceeded, allocate space for "increment" more.
  int new_increment = max_elements > 1 ? max_elements * 2 : 2;

  SVE *tmp_vector 
    = new SVE [max_elements + new_increment];
 
  for (int i=0; i<num_elements; i++)
    tmp_vector[i] = vector[i];

  if (vector)
    delete [] vector;
 
  vector = tmp_vector;
  max_elements += new_increment;
#endif
}

// Append an item to the vector
template <class SVE>
int SdmVector<SVE>::AddElementToList (const SVE &element)
{
  // Allocate more elements if needed
  if (num_elements >= max_elements)
    GrowVector();

  // Append the element to the vector
  vector[num_elements] = element;
  num_elements++;

  return (num_elements-1);
}
 
// Add an element to the end of the vector and return its [] index;
// allows you to add like this: vec[vec.AddElementToVector()] = value;
//
template <class SVE>
int SdmVector<SVE>::AddElementToVector()
{
  // Allocate more elements if needed
  if (num_elements >= max_elements)
    GrowVector();

  // Return the index of the next element allocated on the vector
  return(num_elements++);
}

//
// Find an item
//
template <class SVE>
int SdmVector<SVE>::FindElement (const SVE &element)
{
  for (int i=0; i<num_elements; i++)
  {
    if (vector[i] == element)
      return (i);
  }
  return (Sdm_Not_Found);
}


//---------------------------------------------------------------------------


#endif // _SDM_VECTOR_H

