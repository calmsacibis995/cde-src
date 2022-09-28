/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)flyalloc.c	1.4 97/03/19 SMI"

/*
 * Flyweight Allocater for CDE:
 *	Cheap malloc/free/realloc interposer tailored to the way CDE
 *	allocates memory (tons of tiny blocks, <= 32 bytes).
 */

#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "flyalloc.h"

#ifdef DEBUG
#include <stdio.h>
#endif


#ifdef FLYLOG
#include <stdio.h>
#include <string.h>
#include <sys/procfs.h>

static int logfd= -1;
static int pfd = -1;
static void close_log()
{ 
    if (logfd != -1) {
	close(logfd); 
    }
    if (pfd != -1) {
	close(pfd); 
    }
}
#endif


/*	Must Total 100%! */
enum { PERCENTHGE = 20 };
enum { PERCENTLRG = 30 };
enum { PERCENTMED = 30 };
enum { PERCENTWEE = 20 };



static size_t BLOCKSIZE;	/* assigned later by sysconf() */
static size_t BLOCKSIZEBITS;	/* # of bits to represent BLOCKSIZE */
static size_t MMAPSIZE;		/* assigned later based on BLOCKSIZE */


/* Given an addr in fastmem, figure out PageDescriptor index */
#define PAGE_INDEX(x) \
	(((unsigned)(x) - (unsigned)AddressRange[STARTADDR]) >> BLOCKSIZEBITS)


enum { STARTADDR, ENDADDR };
static unsigned char*	AddressRange[2];
#define isfastmem(p) \
	((p) >= AddressRange[STARTADDR] && (p) < AddressRange[ENDADDR])
 


/* Main block where page/map info is kept */
static ArenaDescriptor*	arena;



static void*	(*real_malloc)(size_t size);
static void*	(*real_realloc)(void *, size_t size);
static void	(*real_free)(void *);



static mutex_t flylock = DEFAULTMUTEX;


static unsigned char*
set_descriptor(
	BucketDescriptor* b,
	int idx,
	unsigned char* p,
	int bucketsize,
	int arenaPercent
)
{
    b->startAddress = p;
    b->nextblock    = p;

    p += (MMAPSIZE * arenaPercent) / 100;

    /* static information kept in header block */
    b->offset = PAGE_INDEX(b->startAddress);
    b->count  = PAGE_INDEX(p) - arena->buckets[idx].offset;

#ifdef DEBUG
    if ((unsigned long)p & (BLOCKSIZE - 1)) {
	fprintf(stderr,"%s:%d: bad page alignment\n",__FILE__,__LINE__);
    }
#endif
    b->endAddress = p;
    b->bucketsize = bucketsize;
    return p;
}


/* One time initialization */
static int
init_bucket_descriptors()
{
    unsigned char* p;
    unsigned char* h;
    int fd;
    int ii;


    /* Initialize handles to real malloc functions */
    real_malloc	 = (void *(*)(size_t))		dlsym(RTLD_NEXT, "malloc");
    real_realloc = (void *(*)(void *,size_t))	dlsym(RTLD_NEXT, "realloc");
    real_free	 = (void (*)(void *))		dlsym(RTLD_NEXT, "free");

    if (real_malloc == NULL || real_realloc == NULL || real_free == NULL) {
        abort();	/* Might as well, can't dance */
    }


    /* Initialize map size based on page size */
    BLOCKSIZE	  = sysconf(_SC_PAGESIZE);
    BLOCKSIZEBITS = (BLOCKSIZE == 4096) ? 12 : 13; /* 4k or 8k page size */
    MMAPSIZE	  = BLOCKSIZE * NBLOCKS;	   /* Size of fw arena */


    fd = open("/dev/zero", O_RDWR);

    /* If the open fails, devzero is -1 and p is set to MAP_FAILED */
    p = (unsigned char *) mmap((caddr_t)0, (size_t)MMAPSIZE,
			       PROT_READ|PROT_WRITE,
			       MAP_PRIVATE|MAP_NORESERVE,
			       fd, 0
			       );
    if (p == MAP_FAILED) {
	return fd; 	/* Let malloc do the allocation */
    }

    /* Instantiate the header block right away */
    h = (unsigned char *)mmap((caddr_t)p,
			      (size_t)BLOCKSIZE,
			      PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE,
			      fd, 0
		       );
    if (h == MAP_FAILED) {
	return fd; 	/* Let malloc do the allocation */
    }
        

    /* Setup header information */
    arena		= (ArenaDescriptor*)h;
    arena->magic1	= FLYMAGIC1;
    arena->magic2	= FLYMAGIC2;
    arena->pagesize	= BLOCKSIZE;
    /* arena->pages are initialized to 0 by /dev/zero */


    /* Keep track of the range of addressed covered by our
     * "fast" memory.  Used by the 'isfastmem()' macro.
     * Note:  skip first block, reserved for header info.
     */
    AddressRange[STARTADDR] = &p[BLOCKSIZE];
    AddressRange[ENDADDR]   = &p[MMAPSIZE];	/* note, this is a "bad address" */


    /* Now bump these permanently so everything other than
     * "arena" is looking at just the bucket arenas.
     */ 
    p		+= BLOCKSIZE;
    MMAPSIZE	-= BLOCKSIZE;


    /* Note:  layout of bucket_map array depends on this ordering */
    p = set_descriptor(
		       &arena->buckets[HGEBUCKET], HGEBUCKET, p, 
		       HGEBUCKETSIZE, PERCENTHGE
	       );
    p = set_descriptor(
		       &arena->buckets[LRGBUCKET], LRGBUCKET, p, 
		       LRGBUCKETSIZE, PERCENTLRG
	       );
    p = set_descriptor(
		       &arena->buckets[MEDBUCKET], MEDBUCKET, p,
		       MEDBUCKETSIZE, PERCENTMED
	       );
    p = set_descriptor(
		       &arena->buckets[WEEBUCKET], WEEBUCKET, p,
		       WEEBUCKETSIZE, PERCENTWEE
	       );

	
    /*
     * This should be unecessary:
     * make sure highest addr bucket sizes end at end of arena
     */
#ifdef DEBUG
    if (p != AddressRange[ENDADDR]) {
	fprintf(stderr,"%s:%d:flywt alloc:p!=arena end\n",__FILE__,__LINE__);
    }
#endif

    arena->buckets[NSIZES - 1].endAddress = p;

    
    /* Initialize the global bucket map.  This is used
     * to quckly map malloc sizes into bucket sizes.
     */
    arena->bucket_map[0] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[1] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[2] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[3] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[4] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[5] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[6] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[7] = &arena->buckets[WEEBUCKET];
    arena->bucket_map[8] = &arena->buckets[WEEBUCKET];

    arena->bucket_map[9]  = &arena->buckets[MEDBUCKET];
    arena->bucket_map[10] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[11] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[12] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[13] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[14] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[15] = &arena->buckets[MEDBUCKET];
    arena->bucket_map[16] = &arena->buckets[MEDBUCKET];

    arena->bucket_map[17] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[18] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[19] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[20] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[21] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[22] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[23] = &arena->buckets[LRGBUCKET];
    arena->bucket_map[24] = &arena->buckets[LRGBUCKET];

    arena->bucket_map[25] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[26] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[27] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[28] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[29] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[30] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[31] = &arena->buckets[HGEBUCKET];
    arena->bucket_map[32] = &arena->buckets[HGEBUCKET];


    /* Tag the pages as not-instantiated for 'flychk' */
    for (ii=0; ii<NBLOCKS; ++ii) {
	arena->pages[ii].count = UNALLOCED;
    }

    return fd;
}

    
static void
getblock(BucketDescriptor *bucket)
{
    int i, n;
    unsigned char *p;
    unsigned int len;
    static int devzero = -2;

    if (devzero == -2) {	/* devzero will be >= -1 after this block */
	devzero = init_bucket_descriptors();
	if (bucket == NULL) {
	    return;
	}
    }


    if (bucket->nextblock >= bucket->endAddress) {
#ifdef DEBUG
	fprintf(stderr,"%s:%d:flywt alloc no bucket mem",__FILE__,__LINE__);
	fprintf(stderr," bucketsize=%d\n", bucket->bucketsize);
#endif /* DEBUG */
        return;
    }


    /* Map BLOCKSIZE amt of the arena with backing store */
     p = (unsigned char *)mmap((caddr_t)bucket->nextblock,
				 (size_t)BLOCKSIZE,
	                         PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE,
				 devzero, 0
		       );
    
    if (p == MAP_FAILED) {
#ifdef DEBUG
	fprintf(stderr,"%s:%d:flywt alloc failed map!\n",__FILE__,__LINE__);
#endif
	return;
    }

    n = (BLOCKSIZE / bucket->bucketsize) - 1;
    len = bucket->bucketsize;

    /* Establish new free list head */
    i = (bucket->nextblock - AddressRange[STARTADDR]) / BLOCKSIZE;
    arena->pages[i].freelist	= (BucketListPtr)p;
    arena->pages[i].count	= n + 1;
    if (bucket->hotpage == NULL) {
	bucket->hotpage = &arena->pages[i];
    }

    for(i=0; i < n; ++i) {
	((BucketListPtr)p)->next = (BucketListPtr)(p + len);
	p += len;
    }

    /* No need to null last next ptr - memory obtained from /dev/zero */
 
    bucket->nextblock += BLOCKSIZE;
    return;
}


static void *
sdt_malloc(size_t size)
{
    void *retVal;
    int i;
    BucketDescriptor *b;

    mutex_lock(&flylock);

#ifdef FLYLOG
    {
	char buf[256];

	if (logfd == -1) {
	    /* Make sure /tmp/fly directory exists. */
	    if ((pfd == -1) && (access("/tmp/fly", W_OK) == -1)) {
		(void) mkdir("/tmp/fly", 0777);
		(void) chmod("/tmp/fly", 0777);
	    }

	    sprintf(buf, "/proc/%.5d", getpid());
	    logfd = open(buf, O_RDONLY);
	    if (logfd != -1) {
		prpsinfo_t psi;

		/* Get "ps" info for program name */
		ioctl(logfd, PIOCPSINFO, &psi);
		close(logfd);

		/* Create file name from program name */
		sprintf(buf, "/tmp/fly/%s.%d.log", psi.pr_fname, getpid());
		logfd = open(buf, O_RDWR|O_APPEND|O_CREAT, 0666);
		atexit(close_log);
	    }
	}

	if (logfd != -1) {
	    sprintf(buf, "%d\n", size);
	    write(logfd, buf, strlen(buf));
	}
    }
#endif

    if (arena == NULL) {
	getblock(NULL);		/* signal to prime the allocator */
    }

    if (size > HGEBUCKETSIZE) {
	mutex_unlock(&flylock);
	return real_malloc(size); /* no fastmem */
    }

    b = arena->bucket_map[size];
    if (b->hotpage == NULL) {
        getblock(b);
        if (b->hotpage == NULL) {
	    mutex_unlock(&flylock);
	    return real_malloc(size); /* no fastmem */
	}
    }

    retVal = (void *)b->hotpage->freelist;
    b->hotpage->freelist = ((BucketListPtr)b->hotpage->freelist)->next;
    b->hotpage->count--;

    if (b->hotpage->count == 0) {
	/* Page is emptied, so scan for the page with the highest
	 * freelist count.  If this fails, the next alloc will attempt
	 * a getblock().
	 */
	int ii;
	PageDescriptor* biggest = NULL;
	int max = (BLOCKSIZE / b->bucketsize);
	int blk = (b->startAddress - AddressRange[STARTADDR]) / BLOCKSIZE;

	/* # of blocks actually instantiated */
	int nblks = blk + ((b->nextblock - b->startAddress) / BLOCKSIZE);

	for (ii=blk; ii<nblks; ++ii) {
	    int bcount = biggest ? biggest->count : 0;
	    biggest = (arena->pages[ii].count > bcount)
				? &arena->pages[ii] 
				: biggest;
	    if ((biggest != NULL) && (biggest->count == max)) {
		break;
	    }
	}

	b->hotpage = biggest;	/* if NULL, next malloc will call getblock() */
    }

    mutex_unlock(&flylock);
    return retVal;
}


void *
malloc(size_t size)
{
    return sdt_malloc( size );
}


void
free(void* p)
{
    unsigned char* ptr = (unsigned char*)p;

    if (ptr == NULL) {
	return;
    }

    if (isfastmem(ptr)) {
	int id = PAGE_INDEX(ptr);
	PageDescriptor* pg = &arena->pages[id];

	mutex_lock(&flylock);
	((BucketListPtr)ptr)->next = pg->freelist;
	pg->freelist = (BucketListPtr)ptr;
	pg->count++;
	mutex_unlock(&flylock);
	return;
    } 

    real_free(ptr);
    return;
}


void *
realloc(void* p, size_t size)
{
    unsigned char* ptr = (unsigned char*)p;

    if (ptr == NULL) {
	/* Special case, ptr is null - regular malloc semantics */
	return (void *) sdt_malloc(size);
    }


    if (isfastmem((unsigned char *)ptr)) {
	void* t;
	BucketDescriptor* b;

	if (arena == NULL) {
	    getblock(NULL);	/* signal to prime the allocator */
	}

	b = &arena->buckets[HGEBUCKET];
	if (ptr >= arena->buckets[WEEBUCKET].startAddress) {
	    b = &arena->buckets[WEEBUCKET];
	}
	else if (ptr >= arena->buckets[MEDBUCKET].startAddress) {
	    b = &arena->buckets[MEDBUCKET];
	}
	else if (ptr >= arena->buckets[LRGBUCKET].startAddress) {
	    b = &arena->buckets[LRGBUCKET];
	}

	if (size <= b->bucketsize) {
	    return ptr;		/* Could be smarter based on bucketsize */
	} 

	t = (char *)sdt_malloc(size);
	if (t != NULL) {
	    memcpy(t, ptr, b->bucketsize);
	    free(ptr);
	    return t;
	}
	else {
	    return NULL;	/* Leave data intact (malloc(1)) */
	}
    } 

    /* Not fast mem, do normal realloc stuff */
    return real_realloc(ptr, size);
}



#ifdef FLYLOG
/*
 * Create an entry in the file system to track what gets
 * forked off from the apps.
 */
int execve(const char* path, char* const argv[], char* const envp[])
{
    static int (*real_execve)(const char*, char* const*, char* const*) = NULL;

    if (pfd == -1) {
	char buf[1024];
	int fd;
	prpsinfo_t psi;

	/* make sure /tmp/fly directory exists */
	if ((logfd == -1) && (access("/tmp/fly", W_OK) == -1)) {
	    (void) mkdir("/tmp/fly", 0777);
	    (void) chmod("/tmp/fly", 0777);
	}

	real_execve
	    = (int (*)(const char*, char* const*, char* const*)) dlsym(RTLD_NEXT, "execve");

	sprintf(buf, "/proc/%.5d", getpid());
	fd = open(buf, O_RDONLY);
	ioctl(fd, PIOCPSINFO, &psi);
	close(fd);

	sprintf(buf, "/tmp/fly/execve.%s.%d", psi.pr_fname, getpid());
	pfd = open(buf, O_RDWR|O_APPEND|O_CREAT, 0666);
	assert(pfd != -1);
    }

    write(pfd, path, strlen(path));
    write(pfd, "\n", 1);

    return real_execve(path, argv, envp);
}
#endif


/******************************************************************************/
