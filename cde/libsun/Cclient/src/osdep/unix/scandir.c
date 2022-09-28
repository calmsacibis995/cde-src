/*
 * Program:	Scan directories
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
 * Last Edited:	04 October 1996
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
 * Edit Log:
 * [10-04-96, clin] convert it to readdir_r even no one seems to call it.
 */
 
#include "os_sv5.h"

#define  NAME_MAX (_PC_NAME_MAX + 1 )
/* Emulator for BSD scandir() call
 * Accepts: directory name
 *	    destination pointer of names array
 *	    selection function
 *	    comparison function
 * Returns: number of elements in the array or -1 if error
 */

int scandir (char *dirname,struct direct ***namelist,int (*select) (),
	     int (*compar) ())
{
  struct direct *p,*d,**names, *res_dir;
  int nitems;
  struct stat stb;
  long nlmax;
  DIR *dirp = opendir (dirname);/* open directory and get status poop */
  				/* get the buffer size for readdir_r. */
  long dsize = sizeof(struct dirent ) + pathconf(dirname, _PC_NAME_MAX) + 1;

  if ((!dirp) || (fstat (dirp->d_fd,&stb) < 0)) return -1;
  nlmax = stb.st_size / 24;	/* guesstimate at number of files */
  names = (struct direct **) fs_get (nlmax * sizeof (struct direct *));
  nitems = 0;			/* initially none found */

  res_dir = (struct dirent *)fs_get(dsize); /* For POSIX there is a result */
					/* needed otherwise, this is OK. */

#if (_POSIX_C_SOURCE - 0 >= 199506L)
  while (readdir_r(dirp, res_dir, &d) == 0)
#else
  while ((d = readdir_r(dirp, res_dir)) != NULL)
#endif
    {	/* read directory item */
				/* matches select criterion? */
    if (select && !(*select) (d)) continue;
				/* get size of direct record for this file */
    p = (struct direct *) fs_get (DIR_SIZE (d));
    p->d_ino = d->d_ino;	/* copy the poop */
    strcpy (p->d_name,d->d_name);
    if (++nitems >= nlmax) {	/* if out of space, try bigger guesstimate */
      nlmax *= 2;		/* double it */
      fs_resize ((void **) &names,nlmax * sizeof (struct direct *));
    }
    names[nitems - 1] = p;	/* store this file there */
  }

  fs_give((void **) &res_dir);  /* need to give up the block */
  closedir (dirp);		/* done with directory */
				/* sort if necessary */
  if (nitems && compar) qsort (names,nitems,sizeof (struct direct *),compar);
  *namelist = names;		/* return directory */
  return nitems;		/* and size */
}
