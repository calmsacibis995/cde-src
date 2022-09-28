/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* @(#)isaddprimary.c	1.7 93/09/07
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isaddprimary.c
 *
 * Description:
 *	Add secondary index to ISAM file
 */


#include "isam_impl.h"
#include <sys/time.h>

static int _am_addprimary(register  Fab *, struct keydesc *);


/*
 * err = isaddprimary(isfd, keydesc)
 *
 * Isaddprimary() is used to add primary index to ISAM file. 
 *
 * This call is not part of the X/OPEN sprec.
 *
 * Errors:
 *	EBADKEY	error in keydesc	
 *	EDUPL	there are duplicate keys and the keydesc does not allow
 *		duplicate keys
 *	EKEXISTS key with the same key descriptor already exists
 *	EKEXISTS the ISAM file already has a primary index.
 *	ENOTEXCL ISAM file is not open in exclusive mode
 *	ENOTOPEN the ISAM file is not open in ISINOUT mode.
 *	EACCES	Cannot create index file because of UNIX error.
 */

int 
isaddprimary(int isfd, struct keydesc * keydesc)
{
    register Fab	*fab;
    int			ret;

    /*
     * Get File Access Block.
     */
    if ((fab = _isfd_find(isfd)) == NULL) {
	_setiserrno2(ENOTOPEN, '9', '0');
	return (ISERROR);
    }

    /*
     * Check that the open mode was ISOUTPUT
     */
    if (fab->openmode != OM_INOUT) {
	_setiserrno2(ENOTOPEN, '9', '0');
	return (ISERROR);
    }

    ret = _am_addprimary(fab, keydesc);
    _seterr_errcode(&fab->errcode);

    return (ret);			     /* Successful write */
}

static int _am_addprimary(fab, keydesc)
    register Fab	*fab;
    struct keydesc	*keydesc;
{
    return (_amaddprimary(&fab->isfhandle, keydesc, &fab->errcode));
}
