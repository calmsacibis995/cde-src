/*
 * @(#)flock_sv5.c	1.4 97/05/02
 *
 * Program:	File lock
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	1 August 1988
 * Last Edited:	26 October 1994
 *
 * Copyright 1994 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made available
 * "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * Emulator for BSD flock() call
 * THAT USES lockf().
 *
 * This will be called from the solaris driver.
 *
 * Accepts: file descriptor
 *	    operation bitmask
 *
 * Returns: 0 if successful, -1 if failure
 *
 * Note: this emulator does not handle shared locks
 */
int
sol_flock(int fd, int operation)
{
  struct flock	fcntlData;
  int		results = 0;
  int		cmd;

  if ((operation & LOCK_NB) != 0) {
    cmd = F_SETLK;	/* LOCK */
  } else {
    cmd = F_SETLKW;	/* LOCK - WAIT for other lock to go away */
  }

  fcntlData.l_whence = 0;	/* Lock the entire file */
  fcntlData.l_start = (off_t)0;
  fcntlData.l_len = (off_t)0;

  switch (operation & ~LOCK_NB) {	/* translate to lockf() operation */
  case LOCK_EX:			/* exclusive */
  case LOCK_SH:			/* shared */
    fcntlData.l_type = F_RDLCK | F_WRLCK;
    break;

  case LOCK_UN:			/* unlock */
    fcntlData.l_type = F_ULOCK | F_UNLKSYS;
    break;

  default:
    errno = EINVAL;
    results = -1;
  }

  if (results != -1) {
    results = fcntl(fd, cmd, &fcntlData);	/* do the lockf() */
    if (results != -1) {
      results = 0;
    }
  }

  return(results);
}
