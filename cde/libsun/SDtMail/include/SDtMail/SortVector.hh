/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmSortVector class.
// The SdmSortVector class is a template based vector class
// that is derived from SdmVector and provides for vectors of 
// sorted objects.
// 
// To use SdmSortVector create a template instantiation for the object:
//   SdmSortVector<MyObject> my_vector;
//
// If the object is a pointer than it will be sorted on the pointer
// address not the object itself.  To create a vector where the object
// pointed to is sorted use SdmPtrSortVector.
//

#ifndef _SDM_SORTVECTOR_H
#define _SDM_SORTVECTOR_H

#pragma ident "@(#)SortVector.hh	1.10 97/04/29 SMI"

#include <SDtMail/Vector.hh>


//---------------------------------------------------------------------------

template <class SVE>
class SdmSortVector : public SdmVector<SVE> {
public:
  // constructors and destructor
  // The number of elements in the vector may be specified, if not
  // the the vector does not initially allocate memory.
  SdmSortVector ()
    { };
  SdmSortVector (const int max_elems)
    : SdmVector<SVE>(max_elems)
    { };
  virtual ~SdmSortVector (void)
    { };

  // Add an element to the list.
  // Insert the element in the sorted position
  // and return an index to the insertion position.
  virtual int AddElementToList (const SVE &element);
};

//---------------------------------------------------------------------------

// Add an item to the vector
// If sorting is specified, insert the element in sorted position.
template <class SVE>
int SdmSortVector<SVE>::AddElementToList (const SVE &element)
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

    if (element < vector[i])
    {
      foundit = i;
      break;
    }
  }

  // Insert the element
  if (foundit > -1)
  {
    // Shift vector elements ->
    for (int j=num_elements; j>foundit; j--)
      vector[j] = vector[j-1];
    vector[j] = element;
  }
  else
  {
    vector[num_elements] = element;
  }

  num_elements++;

  return (num_elements);
}
 
//---------------------------------------------------------------------------


#endif // _SDM_SORTVECTOR_H

