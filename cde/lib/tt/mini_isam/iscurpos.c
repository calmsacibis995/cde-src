/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company                   */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.     */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.                    */
/*%%  (c) Copyright 1993, 1994 Novell, Inc.                              */
/*%%  $Revision: $                                                       */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * @(#)iscurpos.c	1.3 96/05/14
 *
 * Description:
 *	Save and restore current record position functions.
 */

#include "isam_impl.h"
#include <sys/file.h>

/*
 * err = isgetcurpos(isfd, len, buf)
 *
 * Get current record position and save it in user buffer.
 *
 * The user buffer buf is filled with (if the information is N bytes):
 *   	buf[0], buf[1]		total length as u_short (set to N + 2)
 *	buf[2] .. buf[N + 1]	current record position information
 */

int 
isgetcurpos(isfd, len, buf)
    int		isfd;
    int		*len;
    char	**buf;
{
    Fab		*fab;
    u_short	total_len;

    /*
     * Get File Access Block.
     */
    if ((fab = _isfd_find(isfd)) == NULL) {
	_setiserrno2(ENOTOPEN, '9', '0');
	return (ISERROR);
    }

    total_len = sizeof(total_len) + fab->curpos.length;

    if (*buf != NULL && *len < (int)total_len) {	
	_setiserrno2(E2BIG, '9', '0');
	return (ISERROR);
    }

    if (*buf == NULL) {
	*len = total_len;
	*buf = _ismalloc((unsigned int)total_len);
    }

    memcpy(*buf, (char *) &total_len, sizeof(total_len));
    memcpy(*buf+sizeof(total_len), fab->curpos.data, (int)fab->curpos.length);

    return (ISOK);
}

/*
 * err = issetcurpos(isfd, buf)
 *
 * Get current record position and save it in user buffer.
 */

int 
issetcurpos(isfd, buf)
    int		isfd;
    char	*buf;
{
    Fab		*fab;
    u_short	len;

    /*
     * Get File Access Block.
     */
    if ((fab = _isfd_find(isfd)) == NULL) {
	_setiserrno2(ENOTOPEN, '9', '0');
	return (ISERROR);
    }

    memcpy((char *)&len, buf, sizeof(len));
    len -= sizeof (len);

    _bytearr_free(&fab->curpos);
    fab->curpos = _bytearr_new(len, buf + sizeof(len));


    return (ISOK);
}
