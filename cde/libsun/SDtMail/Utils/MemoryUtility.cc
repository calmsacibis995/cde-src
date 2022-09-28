/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Externalized Memory Manager.

#pragma ident "@(#)MemoryUtility.cc	1.24 96/05/13 SMI"

// Include Files
#include <Utils/MemoryUtility.hh>
#include <iostream.h>
#include <stdlib.h>

SdmMemoryUtility::SdmMemoryUtility()
{
}

SdmMemoryUtility::~SdmMemoryUtility()
{
}

 
SdmErrorCode
SdmMemoryUtility::VAlloc(void* dPtr, const int dTyp, const int size)
{
  if(dTyp == VCHAR) {
    char** aPtr;
    aPtr = (char**) dPtr;
    *aPtr = (char*) malloc(size * sizeof(char));
    if (*aPtr == NULL) {
      cout << "Sdm Error: Memory Alloc request for type: "
	   << "Char, size: " << size << " failed" << endl;
      exit(-2);
    }
  }
  else if (dTyp == VINT) {
    int** aPtr;
    aPtr = (int**) dPtr;
    *aPtr = (int*) malloc(size * sizeof(int));
    if (*aPtr == NULL) {
      cout << "Sdm Error: Memory Alloc request for type: "
	   << "Int, size: " << size << " failed" << endl;
      exit(-2);
    }
  }
  return Sdm_EC_Success;
}

SdmErrorCode
SdmMemoryUtility::VDlloc(void* dPtr, const int dTyp)
{
  if (dPtr == NULL)
    return Sdm_EC_Success;
  else {
    free(dPtr);
    dPtr = NULL;
    return Sdm_EC_Success;
  }
}

