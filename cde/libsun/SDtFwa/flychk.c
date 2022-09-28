/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)flychk.c	1.3 96/05/02 SMI"

/*
 * flychk.c
 * Reads and reports on the information contained in the flyweight
 * allocator's header page wrt current allocation characteristics.
 *
 * Options:
 *	flychk [-v] <-p pid> | <process name>
 *	-p pid		Pid of process to be examined
 *	-v		Verbose mode, prints usage info per-page.
 *
 * Compile:
 *	cc -v -Xa -O -o flychk flychk.c
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/procfs.h>
#include "flyalloc.h"



static void
dump_arena(ArenaDescriptor* arena, boolean_t verbose, const char* name)
{
    int ii;
    int xx;
    int	last;


    printf("Flyweight Allocator Arena\n");
    printf("-------------------------\n");
    printf("Process Name:\t\t%s\n", name);
    printf("Page Size:\t\t%d\n", arena->pagesize);

    if (verbose == B_TRUE) {
	printf("ArenaDescriptor size:\t%d\n", sizeof(ArenaDescriptor));
    }

    printf("\n");
    printf("              Bucket    Pages    Pages   Slots    Open\n");
    printf("               Size    Reserved  Used   Per-Page  Slots\n");

    for (ii=0; ii<NSIZES; ++ii) {
	int used;
	size_t unalloced;

	if (verbose == B_TRUE) {
	    printf("Bucket[%d]:\t", ii);
	}
	else {
	    printf("Bucket:\t\t");
	}
	printf("%2d%10d", arena->buckets[ii].bucketsize, arena->buckets[ii].count);

	used = 0;
	unalloced = 0;
	last = arena->buckets[ii].offset + arena->buckets[ii].count;
	for (xx=arena->buckets[ii].offset; xx<last; ++xx) {
	    if (arena->pages[xx].count == UNALLOCED) {
		break;
	    }
	    used++;
	    unalloced += arena->pages[xx].count;
	}

	printf("%8d%9d%9d\n", used,
	       (arena->pagesize / arena->buckets[ii].bucketsize), 
	       unalloced
           );
    }


    if (verbose == B_FALSE) {
	return;
    }


    /* Verbose mode, dump the page table free list information */
    printf("\n\n");
    printf("Page         Free / Of    Size\n");
    printf("------------------------------\n");
    for (ii=0; ii<NSIZES; ++ii) {
	last = arena->buckets[ii].offset + arena->buckets[ii].count;
	for (xx=arena->buckets[ii].offset; xx<last; ++xx) {
	    if (arena->pages[xx].count == UNALLOCED) {
		break;
	    }

	    printf("Page[%3d]%8d / %d%5d\n", 
		   xx, arena->pages[xx].count,
		   (arena->pagesize / arena->buckets[ii].bucketsize),
		   arena->buckets[ii].bucketsize
	       );
	}
	printf("\n");
    }
}


static int
get_proc_by_name(const char* name)
{
    int fd = -1;
    struct dirent* ent;
    char* match = NULL;
    char buf[24];
    DIR* dir;

    dir = opendir("/proc");
    if (dir == NULL) {
	return -1;
    }

    while ((ent = readdir(dir)) != NULL) {
	prpsinfo_t ps;

	sprintf(buf, "/proc/%s", ent->d_name);
	fd = open(buf, O_RDWR);
	if (fd == -1) {
	    /* don't have permission to open process image */
	    continue;
	}

	(void) ioctl(fd, PIOCPSINFO, &ps);
	close(fd);
	if (strcmp(name, ps.pr_fname) == 0) {
	    /* found a match. */
	    if (match != NULL) {
		/* Punt, the name is ambiguous... */
		closedir(dir);
		free(match);
		return -2;	/* -2 for "ambiguous" */
	    }
	    match = strdup(ent->d_name);
	}
    }

    fd = -1;
    if (match != NULL) {
	/* Completed and we have a match */
	sprintf(buf, "/proc/%s", match);
	fd = open(buf, O_RDWR);
	free(match);
    }

    closedir(dir);
    return fd;
}


void
main(int argc, char** argv)
{
    boolean_t verbose = B_FALSE;
    const char* name;
    unsigned long mask;
    int ii;
    int fd = -1;
    int count;
    prmap_t* maps;
    prpsinfo_t ps;
    ArenaDescriptor arena;


    if (argc == 1) {
	fprintf(stderr, "usage:  %s:\t[-v] [-p pid] [process name]\n", argv[0]);
	return;
    }


    /* Look at input options */
    for (ii=1; ii<argc; ++ii) {
	if (strcmp(argv[ii], "-p") == 0) {
	    char buf[24];
	    ++ii;
	    sprintf(buf, "/proc/%.5d", atoi(argv[ii]));
	    fd = open(buf, O_RDWR);
	}
	else if (strcmp(argv[ii], "-v") == 0) {
	    verbose = B_TRUE;
	}
	else {
	    fd = get_proc_by_name((const char*)argv[ii]);
	    if (fd == -2) {
		fprintf(stderr, "AMBIGUOUS:  There is more than one instance of"
			" this process running.\n"
		    );
		return;
	    }
	}
    }

    if (fd == -1) {
	fprintf(stderr, "Couldn't open this process\n");
	return;
    }

    /* make sure client is runnable when we go away. */
    mask = PR_RLC;
    ioctl(fd, PIOCSET, &mask);


    ioctl(fd, PIOCNMAP, &count);
    maps = malloc(count * sizeof(prmap_t));
    if (maps == NULL) {
	fprintf(stderr, "Out of memory allocating space for mapping information\n");
	return;
    }
    ioctl(fd, PIOCMAP, maps);

    /* first 3 are always text, data & heap */
    for (ii=3; ii<count; ++ii) {
	u_long flags = maps[ii].pr_mflags;
	if ( ! ((flags & ~MA_SHARED) && (flags & MA_READ)
		&& (flags & MA_WRITE) && !(flags & MA_EXEC))
	     ) {
	    /* Arena is always mapped r/w/private */
	    continue;
	}

	/* Stop process, don't want data changing while we read it */
	ioctl(fd, PIOCSTOP, NULL);
	if (pread(fd, &arena, sizeof(ArenaDescriptor), (off_t)maps[ii].pr_vaddr)
		    < sizeof(ArenaDescriptor)
	    ) {
	    ioctl(fd, PIOCRUN, NULL);
	    continue;
	}

	ioctl(fd, PIOCRUN, NULL);	/* Done reading, let go of process. */

	if ((arena.magic1 != FLYMAGIC1) || (arena.magic2 != FLYMAGIC2)) {
	    continue;
	}
	
	ioctl(fd, PIOCPSINFO, &ps);
	dump_arena(&arena, verbose, ps.pr_fname);
	free(maps);
	return;
    }

    printf("Flyweight Allocator not present.\n");
    free(maps);
    return;
}

/******************************************************************************/
