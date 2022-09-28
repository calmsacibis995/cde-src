/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the System Wide Memory Repository Class.

#ifndef _SDM_REPSWMR_H
#define _SDM_REPSWMR_H

#pragma ident "@(#)RepSWMR.hh	1.35 97/03/21 SMI"

#include <iostream.h>
#include <SDtMail/Sdtmail.hh>
#include <stdlib.h>
#include <stddef.h>

typedef enum SdmMemoryMgmt_T { 
  Sdm_MM_General, Sdm_MM_Message, Sdm_MM_Transaction, 
  Sdm_MM_String, Sdm_MM_StringRep, Sdm_MM_AnonymousAllocation
} SdmMemoryMgmt;

static void _SdmMemoryRepository_OpenDevZero();
static unsigned long* _SdmMemoryRepository_MallocFromMappedMemory(size_t size);
static unsigned long* _SdmMemoryRepository_MallocFromMemory(size_t size);
static unsigned long* _SdmMemoryRepository_ReallocMappedMemory(void* mAdd, size_t size);

class SdmMemoryRepository {

public:
  SdmMemoryRepository();
  virtual ~SdmMemoryRepository();

  static void* _ONew(size_t size, SdmMemoryMgmt mm=Sdm_MM_General);
  static void* _ONew(size_t size, void* mAdd, SdmMemoryMgmt
		     mm=Sdm_MM_General);
  static void _ODel(void* mAdd, SdmMemoryMgmt mm=Sdm_MM_General);
  static void* _ORealloc(void* mAdd, size_t size, SdmMemoryMgmt mm=Sdm_MM_General);
  static void _OpenDevZero();

private:
  friend _SdmMemoryRepository_OpenDevZero;
  friend _SdmMemoryRepository_MallocFromMappedMemory;
  friend _SdmMemoryRepository_ReallocMappedMemory;
  friend _SdmMemoryRepository_MallocFromMemory;
  static int 		_fd_devzero;	// fd for /dev/zero is stored here
};

#endif
