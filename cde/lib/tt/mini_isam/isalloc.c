/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isalloc.c
 *
 * Description:
 *	Functions that allocate and deallocate memory.
 *	All errors are treated as fatal.
 */

#include "isam_impl.h"
#include <malloc.h>

/* strdup is not defined in Ultrix' string.h, so it is implemented in a separate
 * file named strdup.ultrix.c
 */
#ifdef __ultrix__
extern char *strdup();
#endif

/*
 * _ismalloc(nbytes)
 *
 * Allocate nbytes.
 */

char *_ismalloc(nbytes)
    unsigned int	nbytes;
{
    register char	*p;
    
    if ((p = (char *) malloc (nbytes)) == NULL) {
    	char                mesg[255];

	sprintf(mesg, "malloc() failed: got NULL instead of %d bytes", nbytes);
	_isfatal_error(mesg, strerror(errno));
    }

    return (p);
}

char *_isrealloc(oldaddr, nbytes)
    char		*oldaddr;
    unsigned int	nbytes;
{
    register char	*p;
    
    if ((p = (char *) realloc (oldaddr, nbytes)) == NULL) {
    	char                mesg[255];

	sprintf(mesg, "realloc() failed: got NULL instead of %d bytes", nbytes);
	_isfatal_error(mesg, strerror(errno));
    }

    return (p);
}


/*
 * _isallocstring(str)
 *
 * Create a duplicate of string in dynamic memory.
 */

char *
_isallocstring(str)
    char	*str;
{
    register char	*p;

    if ((p = strdup(str)) == NULL) {
	_isfatal_error("strdup() failed", (char *) 0);
    }

    return (p);
}

/*
 * _isfreestring(str)
 *
 * Free dynamically allocated string.
 */

void
_isfreestring(str)
    char	*str;
{
    assert(str != NULL);
    free(str);
}
