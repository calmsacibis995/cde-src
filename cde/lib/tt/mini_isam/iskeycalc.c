/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * iskeycalc.c
 *
 * Description:
 *	Key related calculation functions
 */

#include "isam_impl.h"


/* getkeysperleaf() - Calculate number of keys per leaf node */
int 
getkeysperleaf (keylen)
    int		keylen;
{
    int 	n;

    n = ((ISPAGESIZE - BT_KEYS_OFF) / keylen);

    return (le_odd (n));		     /* n or n-1 */
}

/* getkeyspernode() - Calculate number of keys per non-leaf node */
int 
getkeyspernode (keylen)
    int		keylen;
{
    int 	n;

    n = ((ISPAGESIZE - BT_KEYS_OFF) / (keylen + BLKNOSIZE));

    return (le_odd (n));		     /* n or n-1 */
}


/* le_odd(n) - Get next lower or equal odd number */
int 
le_odd(n)
    int		n;
{
    return ((n - 1) | 1);
}
