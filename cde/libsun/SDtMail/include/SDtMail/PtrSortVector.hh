/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmPtrSortVector class.
// The SdmPtrSortVector class is a template based vector class
// that is derived from SdmVector and provides for vectors of 
// pointers to sorted objects.
// 
// To use SdmPtrSortVector create a template instantiation
// for the pointer to the object:
//   SdmPtrSortVector<MyObject *> my_vector;
//
// The object must have both operator== and operator< defined.
// If these aren't provided, the vector cannot be sorted and
// SdmPtrVector should be used instead.
//
 
#ifndef _SDM_PTRSORTVECTOR_H
#define _SDM_PTRSORTVECTOR_H

#pragma ident "@(#)PtrSortVector.hh	1.10 97/04/29 SMI"

#include <SDtMail/PtrVector.hh>


//---------------------------------------------------------------------------

template <class SVE>
class SdmPtrSortVector : public SdmPtrVector<SVE> {
public:
  // constructors and destructor
  // The number of elements in the vector may be specified, if not
  // the the vector does not initially allocate memory.
  SdmPtrSortVector ()
    { };
  SdmPtrSortVector (const int max_elems)
    : SdmVector<SVE>(max_elems)
    { };
  virtual ~SdmPtrSortVector (void)
    { };

  // Match the give element with one in the vector and return the position.
  // If the element isn't found, Sdm_Not_Found (-1) is returned.
  int FindElementByValue (const SVE &element);

  // Add an element to the list.
  // Insert the element in the sorted position
  // and return an index to the insertion position
  virtual int AddElementToList (const SVE &element);
};

//---------------------------------------------------------------------------

//
// Find an item
//
template <class SVE>
int SdmPtrSortVector<SVE>::FindElementByValue (const SVE &element)
{
  for (int i=0; i<num_elements; i++)
  {
    if (*(vector[i]) == *element)
      return (i);
  }
  return (Sdm_Not_Found);
}


// Add an item to the vector
// If sorting is specified, insert the element in sorted position.
template <class SVE>
int SdmPtrSortVector<SVE>::AddElementToList (const SVE &element)
{
  // Allocate more elements if needed
  if (num_elements >= max_elements)
    GrowVector();

  // Add the element to the vector
  // Insertion Sort - using linear search
  int foundit = -1;

    // find insertion point
  for (int i=0; i<num_elements; i++)
  {
    // Array elements must be pointers
    if (!element || !vector[i]) {
      assert (0);
      abort();
    }

    if (*element < *(vector[i]))
    {
      foundit = i;
      break;
    }
  }

  // insert the element
  if (foundit > -1)
  {
    // shift vector elements ->
    for (int i=num_elements; i>foundit; i--)
      vector[i] = vector[i-1];
    vector[i] = element;
  }
  else
  {
    vector[num_elements] = element;
  }

  num_elements++;

  return (num_elements-1);
}
 
//---------------------------------------------------------------------------


#endif // _SDM_PTRSORTVECTOR_H
