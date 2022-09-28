/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#ifndef _FLYWEIGHT_ALLOCATOR_H
#define _FLYWEIGHT_ALLOCATOR_H

#pragma ident "@(#)flyalloc.h	1.3 96/05/02 SMI"

/*
 *  The flyweight allocator is a malloc/realloc/free interposer
 *  which is tailored to the way CDE uses memory.  CDE tends to
 *  allocate very large numbers of very small blocks -- the vast
 *  majority being 32 bytes and under.
 */


/* Hex representation of the name "flyalloc" 8^|
 * Has the advantage that in a dump of the raw data
 * for a segment, "flyalloc" as the first 8 bytes is
 * easilly identifiable.
 */
enum {
    FLYMAGIC1 = 0x666c7961,
    FLYMAGIC2 = 0x6c6c6f63
};


/* Arena size in pages plus 1 for the header information */
enum { NBLOCKS = 400 + 1};


/* Number of bucket sizes */
enum { NSIZES = 4 };



enum {
    HGEBUCKETSIZE = 32,	/* Divides evenly into BLOCKSIZE */
    LRGBUCKETSIZE = 24,	/* Tiny bit of tail waste */
    MEDBUCKETSIZE = 16,	/* Divides evenly into BLOCKSIZE */
    WEEBUCKETSIZE = 8	/* Divides evenly into BLOCKSIZE */
};

enum {
    HGEBUCKET,
    LRGBUCKET,
    MEDBUCKET,
    WEEBUCKET
};



typedef struct _BucketRec* BucketListPtr; /* Used for free list */
struct _BucketRec {
    struct _BucketRec* next;
};


/* Flag set in page[n].count if physical page hasn't been allocated yet. */
static const unsigned short UNALLOCED = (unsigned short)-1;
typedef struct {
    BucketListPtr	freelist;	/* first free slots */
    unsigned short	count;		/* number of free slots */
} PageDescriptor;


typedef struct _BucketDescriptorRec BucketDescriptor;
struct _BucketDescriptorRec {
    unsigned char*	endAddress;	/* BUG: this is hosed */
    unsigned char*	startAddress;
    unsigned char*	nextblock;
    PageDescriptor*	hotpage;
    unsigned short	bucketsize;

    /* Pre-computed values used by external client */
    unsigned short	offset;	/* PageDescriptor start index */
    unsigned short	count;	/* # of pages at this bucket size */
};


/* The main block where all the mapping and page info is
 * tracked.  It is intended that this block be identifiable
 * and readable by an external agent to report on allocation
 * statistics.  
 *
 * More on this later...
 *
 * WARNING:	This thing is ASSumed to fit on a single
 *		page in memory!
 */
typedef struct {
    unsigned int	magic1;			/* id ourselves */
    unsigned int	magic2;
    unsigned short	pagesize;		/* need this, ULTRASparc uses 8k */


    /* Bucket information, only the count, offset & bucktsize
     * values are useful for an external reader.
     */
    BucketDescriptor	buckets[NSIZES];


    /* Used as a quick way to map a bucket size to it's cor-
     * responding BucketDescriptor.  Ordering is dependent upon
     * the initialization sequence in init_bucket_descriptors().
     */
    BucketDescriptor*	bucket_map[HGEBUCKETSIZE+1];

    PageDescriptor	pages[NBLOCKS-1];	/* the acutal page handles */
} ArenaDescriptor;




#endif	/* _FLYWEIGHT_ALLOCATOR_H */
