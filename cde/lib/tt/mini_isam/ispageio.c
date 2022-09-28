/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * ispageio.c
 *
 * Description: 
 *	I/O functions for file pages.
 *	
 *
 */

#include "isam_impl.h"

/*
 * _isseekpg(fd, pgno)
 *
 * Set current file pointer to the page pgno.
 */

void
_isseekpg(fd, pgno)
    int		fd;
    Blkno	pgno;
{
    register long	n;
    char 		mesg[255];
    off_t		offset = pgno * ISPAGESIZE;

    if ((n = lseek(fd, offset, 0)) != offset) {
	sprintf(mesg, "lseek failed: got %ld instead of %ld", n, offset);
	_isfatal_error(mesg, strerror(errno));
    }
}

/*
 * _isreadpg(fd, buf)
 *
 * Read eon block from UNIX file into a buffer.
 */

void
_isreadpg(fd, buf)
    int		fd;
    char	*buf;
{
    register int	n;
    char 		mesg[255];

    if ((n = read(fd, buf, ISPAGESIZE)) != ISPAGESIZE) {
	sprintf(mesg, "read failed: requested %d bytes, got %d", ISPAGESIZE, n);
	_isfatal_error(mesg, strerror(errno));
    }
}

/*
 * _iswritepg(fd, buf)
 *
 * Write one block to UNIX file.
 */

void
_iswritepg(fd, buf)
    int		fd;
    char	*buf;
{
    register int	n;
    char 		mesg[255];

    if ((n = write(fd, buf, ISPAGESIZE)) != ISPAGESIZE) {
	sprintf(mesg, "write failed: tried %d bytes, wrote %d", ISPAGESIZE, n);
	_isfatal_error(mesg, strerror(errno));
    }
}
