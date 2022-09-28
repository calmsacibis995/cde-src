/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the System Wide Memory Repository Class.

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RepSWMR.cc	1.34 97/06/11 SMI"
#endif

// Include Files.
#include <SDtMail/RepSWMR.hh>
#include <SDtMail/SystemUtility.hh>
#include <sys/types.h>
#include <sys/mman.h>

// Memory allocation scheme:
// For each allocation, an additional unsigned long is allocated. This is placed
// at the beginning of each allocation, and is used to contain a value that indicates
// whether or not the allocation was done with mmap() against /dev/zero or malloc().
// allocation[MEM_MAPSIZE_OFFSET] == 0, malloc() was used to allocate this area
//               		  != 0, mmap() was used to allocate this area - this is 
//					the size of the map region used (for munmap())
// allocation[MEM_KEYVALUE_OFFSET] == MEM_KEYVALUE - if assertions enabled, this special 
//					value is placed in the preamble to verify that on
//					deallocation this region was allocated with this pkg.
// allocation[MEM_DELTA] -> value returned to caller
//

#ifndef NDEBUG
// Asserts are turned on - add code to verify new/del used on correct objects
#define MEM_DELTA 2
#define MEM_DELTA_BYTES (sizeof(unsigned long)*MEM_DELTA)
#define MEM_KEYVALUE ((unsigned long)(0x5f500ff5))
#define MEM_MAPSIZE_OFFSET 0
#define MEM_KEYVALUE_OFFSET 1
#else
// Asserts are turned off - exclude the new/del verification code
#define MEM_DELTA 1
#define MEM_DELTA_BYTES (sizeof(unsigned long)*MEM_DELTA)
#define MEM_MAPSIZE_OFFSET 0
#endif

SdmMemoryRepository::SdmMemoryRepository()
{
}

SdmMemoryRepository::~SdmMemoryRepository()
{
}

int SdmMemoryRepository::_fd_devzero = -1;		// file descriptor on which /dev/zero is opened, for anonymous mappings

// This method opens /dev/zero and places the file descriptor in the
// static variable SdmMemoryRepository::_fd_devzero - a -1 means that
// mapping against /dev/zero is not allowed. This method is called as
// part of the C++ startup initialization, so a failure of the open
// results in no attempts to map against /dev/zero.
//
static void _SdmMemoryRepository_OpenDevZero()
{
  SdmMemoryRepository::_fd_devzero = ::open("/dev/zero", O_RDWR);
}
#pragma init(_SdmMemoryRepository_OpenDevZero)

// Function to allocate memory via mmap()ing against /dev/zero
// When these allocations are freed up, the space is returned to the
// system as opposed to malloc() which, once memory is allocated via
// sbrk(), it can never be returned for the life of the process.
//
static unsigned long* _SdmMemoryRepository_MallocFromMappedMemory(size_t size)
{
  if (SdmMemoryRepository::_fd_devzero == -1)
    return NULL;
  int page_size = SdmSystemUtility::HardwarePageSize();
  if (!page_size)
    page_size = (int)sysconf(_SC_PAGESIZE);
  size_t map_size = (size_t) (size + (page_size - (size % page_size)));
  unsigned long* map_region = (unsigned long *)mmap(NULL, map_size, 
						    PROT_READ|PROT_WRITE|PROT_EXEC,
						    MAP_PRIVATE, 
						    SdmMemoryRepository::_fd_devzero, (off_t)0);
  if (map_region == (unsigned long *)-1)
    return NULL;
  map_region[MEM_MAPSIZE_OFFSET] = map_size;
  return(map_region);		// RETURN ALLOCATION+0
}

static unsigned long* _SdmMemoryRepository_ReallocMappedMemory(void* mAdd, size_t size)
{
  unsigned long* memcur = ((unsigned long*) mAdd)-MEM_DELTA;

#if defined(MEM_KEYVALUE_OFFSET)
  // Assertions are enabled -- make sure this allocation was created via
  // this package of methods.
  assert(memcur[MEM_KEYVALUE_OFFSET] == MEM_KEYVALUE);
#endif

  unsigned long* memnew = _SdmMemoryRepository_MallocFromMappedMemory(size);

  if (!memnew)
    return(NULL);

  unsigned long old_map_size = memcur[MEM_MAPSIZE_OFFSET];
  unsigned long new_map_size = memnew[MEM_MAPSIZE_OFFSET];
  memcpy((void*)memnew, (void*)memcur, old_map_size <= new_map_size ? old_map_size : new_map_size);
  SdmMemoryRepository::_ODel(mAdd, Sdm_MM_General);

  return(memnew);		// RETURN NEW ALLOCATION+0
}


// Function to allocate memory via malloc() - any memory so allocated can
// never be returned to the system during the life of the process.
//
static unsigned long* _SdmMemoryRepository_MallocFromMemory(size_t size)
{
  unsigned long* memory_region = (unsigned long *)malloc(size);
  if (memory_region)
    memory_region[MEM_MAPSIZE_OFFSET] = 0;
  return(memory_region);	// RETURN ALLOCATION+0
}

void*
SdmMemoryRepository::_ONew(size_t size, SdmMemoryMgmt mm)
{
  unsigned long* ulmem;
  void* vmem;

  // For certain allocations we perform special operations; for others we are simply
  // malloc and free

  switch (mm) {
    // These classes of memory are mapped against memory if they are large
  case Sdm_MM_AnonymousAllocation:
    // First, if the size of the allocation exceeds the size of a page on this
    // hardware, allocate against mapped memory as opposed to out of the heap.

    if (size >= SdmSystemUtility::HardwarePageSize()-MEM_DELTA_BYTES)
      ulmem = _SdmMemoryRepository_MallocFromMappedMemory(size+MEM_DELTA_BYTES);
    else
      ulmem = NULL;

    // If the allocation is small or allocation against mapped memory fails,
    // attempt to allocate via malloc()

    if (ulmem == NULL)
      ulmem = _SdmMemoryRepository_MallocFromMemory(size+MEM_DELTA_BYTES);

#if defined(MEM_KEYVALUE_OFFSET)
    // Assertions are enabled - set a key value in the memory preamble that
    // is checked on deallocation to make sure all deallocations are used
    // with memory allocated with this same method.
    if (ulmem)
      ulmem[MEM_KEYVALUE_OFFSET] = MEM_KEYVALUE;
#endif

    return (void *)(ulmem ? (ulmem+MEM_DELTA) : NULL);	// RETURN ALLOCATION+DELTA

    // These classes of memory always use malloc
  default:
    vmem = malloc(size);
    if (vmem == NULL)
      assert(0);
    return (vmem);
  }
}

void*
SdmMemoryRepository::_ONew(size_t size, void* mAdd, SdmMemoryMgmt mm)
{
  // Not implemented in-situ
  return(SdmMemoryRepository::_ONew(size, mm));
}

void
SdmMemoryRepository::_ODel(void* mAdd, SdmMemoryMgmt mm)
{
  unsigned long* ulmem;
  unsigned long map_size;

  // For certain allocations we perform special operations; for others we are simply
  // malloc and free

  switch (mm) {
    // These classes of memory are mapped against memory if they are large
  case Sdm_MM_AnonymousAllocation:
    ulmem = (unsigned long*) mAdd-MEM_DELTA;
    map_size = ulmem[MEM_MAPSIZE_OFFSET];

#if defined(MEM_KEYVALUE_OFFSET)
    // Assertions are enabled -- make sure this allocation was created via
    // this package of methods.
    assert(ulmem[MEM_KEYVALUE_OFFSET] == MEM_KEYVALUE);
#endif

    // If a map size is included in the preamble, this means the allocation was
    // created via mmap() so it is deallocated via munmap() - else this allocation
    // was created via malloc() so it is deallocated via free()

    if (map_size)
      (void) munmap((caddr_t)(ulmem), map_size);
    else
      free((void*)(ulmem));
    return;

    // These classes of memory always use malloc
  default:
    free((void*)mAdd);
    return;
  }
}

void* SdmMemoryRepository::_ORealloc(void* mAdd, size_t size, SdmMemoryMgmt mm)
{
  unsigned long* memcur;
  unsigned long* memnew;
  unsigned long  map_size;

  // For certain allocations we perform special operations; for others we are simply
  // malloc and free

  switch (mm) {
    // These classes of memory are mapped against memory if they are large
  case Sdm_MM_AnonymousAllocation:
    memcur = ((unsigned long*) mAdd)-MEM_DELTA;
    memnew = NULL;
    map_size = memcur[MEM_MAPSIZE_OFFSET];

#if defined(MEM_KEYVALUE_OFFSET)
    // Assertions are enabled -- make sure this allocation was created via
    // this package of methods.
    assert(memcur[MEM_KEYVALUE_OFFSET] == MEM_KEYVALUE);
#endif

    if (map_size)
      memnew = _SdmMemoryRepository_ReallocMappedMemory(memcur,size+MEM_DELTA_BYTES);
    else
      memnew = (unsigned long*) realloc(memcur,size+MEM_DELTA_BYTES);

    return (void *)(memnew ? (memnew+MEM_DELTA) : NULL);
    // These classes of memory always use malloc
  default:
    return((void *)realloc(mAdd, size));
  }
}
