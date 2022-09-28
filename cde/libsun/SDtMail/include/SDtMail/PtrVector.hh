/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmPtrVector class.
// The SdmPtrVector class is a template based vector class
// that is derived from SdmVector and provides for vectors of 
// pointers to objects.
// 
// To use SdmPtrVector create a template instantiation
// for the pointer to the object:
//   SdmPtrVector<MyObject *> my_vector;
//
// To sort the vector, the SdmPtrSortVector must be used.
//

#ifndef _SDM_PTRVECTOR_H
#define _SDM_PTRVECTOR_H

#pragma ident "@(#)PtrVector.hh	1.9 96/05/13 SMI"

#include <SDtMail/Vector.hh>


//---------------------------------------------------------------------------

template <class SVE>
class SdmPtrVector : public SdmVector<SVE> {
public:
  // constructors and destructor
  // The number of elements in the vector may be specified, if not
  // the the vector does not initially allocate memory.
  SdmPtrVector () : ptr_delete(Sdm_False)
    { };
  SdmPtrVector (const int max_elems)
    : SdmVector<SVE>(max_elems),
    ptr_delete(Sdm_False)
    { };
  virtual ~SdmPtrVector (void);

  // Reset the number of elements to 0, delete what the vector
  // elements point to, and free the memory of the vector itself.
  void ClearAndDestroyAllElements (void);

  // Set the ptr_delete flag to True or False.  The objects pointed to 
  // will be deleted automatically by the destructor.
  void SetPointerDeleteFlag ()
    { ptr_delete = Sdm_True; }
  void ClearPointerDeleteFlag ()
    { ptr_delete = Sdm_False; }

private:
  SdmBoolean ptr_delete;

};

//---------------------------------------------------------------------------

// destructor
template <class SVE>
SdmPtrVector<SVE>::~SdmPtrVector (void)
{
  if (ptr_delete)
    ClearAndDestroyAllElements();
}

// Empty the vector, deallocate,
// delete what elements point to
template <class SVE>
void SdmPtrVector<SVE>::ClearAndDestroyAllElements ()
{
  // Array elements must be pointers
  for (int i=0; i<num_elements; i++)
  {
    // Array elements must be pointers
    if (vector[i] != NULL)
      delete vector[i];

    vector[i] = NULL;
  }
 
  if (max_elements)
  {
    delete [] vector;
    vector = NULL;
    max_elements = 0;
  }
 
  num_elements = 0;
}

//---------------------------------------------------------------------------


#endif // _SDM_PTRVECTOR_H

