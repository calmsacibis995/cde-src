/*
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

/* Emulator for BSD flock() call
 * THAT USES lockf().
 *  Bill Yeager -- 8-jun-95
 * This will be called from the solaris driver.
 * Accepts: file descriptor
 *	    operation bitmask
 * Returns: 0 if successful, -1 if failure
 * Note: this emulator does not handle shared locks
 */

int sol_flock (int fd, int operation)
{
  int func;
  off_t offset = lseek (fd,0,L_INCR);
  switch (operation & ~LOCK_NB){/* translate to lockf() operation */
  case LOCK_EX:			/* exclusive */
  case LOCK_SH:			/* shared */
    func = (operation & LOCK_NB) ? F_TLOCK : F_LOCK;
    break;
  case LOCK_UN:			/* unlock */
    func = F_ULOCK;
    break;
  default:			/* default */
    errno = EINVAL;
    return -1;
  }
  lseek (fd,0,L_SET);		/* position to start of the file */
  func = lockf (fd,func,0);	/* do the lockf() */
  lseek (fd,offset,L_SET);	/* restore prior position */
  return func;
}
