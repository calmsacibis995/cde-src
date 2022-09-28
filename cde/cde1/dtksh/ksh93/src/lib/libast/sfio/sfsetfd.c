/* $XConsortium: sfsetfd.c /main/cde1_maint/3 1995/10/14 01:14:24 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/
#include	"sfhdr.h"

/*	Change the file descriptor
**
**	Written by Kiem-Phong Vo (01/08/91)
*/

#if __STD_C
static _sfdup(reg int fd, reg int newfd)
#else
static _sfdup(fd,newfd)
reg int	fd;
reg int	newfd;
#endif
{
	reg int	dupfd;

#ifdef F_DUPFD	/* the simple case */
	while((dupfd = fcntl(fd,F_DUPFD,newfd)) < 0 && errno == EINTR)
		errno = 0;
	return dupfd;

#else	/* do it the hard way */
	if((dupfd = dup(fd)) < 0 || dupfd >= newfd)
		return dupfd;

	/* dup() succeeded but didn't get the right number, recurse */
	newfd = _sfdup(fd,newfd);

	/* close the one that didn't match */
	CLOSE(dupfd);

	return newfd;
#endif
}

#if __STD_C
sfsetfd(reg Sfio_t* f, reg int newfd)
#else
sfsetfd(f,newfd)
reg Sfio_t	*f;
reg int		newfd;
#endif
{
	reg int		oldfd;

	if(f->flags&SF_STRING)
		return -1;

	if((f->mode&SF_INIT) && f->file < 0)
	{	/* restoring file descriptor after a previous freeze */
		if(newfd < 0)
			return -1;
	}
	else
	{	/* change file descriptor */
		if((f->mode&SF_RDWR) != f->mode && _sfmode(f,0,0) < 0)
			return -1;
		SFLOCK(f,0);

		oldfd = f->file;
		if(oldfd >= 0)
		{	if(newfd >= 0)
			{	if((newfd = _sfdup(oldfd,newfd)) < 0)
				{	SFOPEN(f,0);
					return -1;
				}
				CLOSE(oldfd);
			}
			else
			{	/* sync stream if necessary */
				if(((f->mode&SF_WRITE) && f->next > f->data) ||
				   (f->mode&SF_READ) || f->disc == _Sfudisc)
				{	if(SFSYNC(f) < 0)
					{	SFOPEN(f,0);
						return -1;
					}
				}

				if(((f->mode&SF_WRITE) && f->next > f->data) ||
				   ((f->mode&SF_READ) && f->extent < 0 &&
				    f->next < f->endb) )
				{	SFOPEN(f,0);
					return -1;
				}

#ifdef MAP_TYPE
				if((f->flags&SF_MMAP) && f->data)
				{	(void)munmap((caddr_t)f->data,f->endb-f->data);
					f->data = NIL(uchar*);
				}
#endif

				/* make stream appears uninitialized */
				f->endb = f->endr = f->endw = f->data;
				f->extent = f->here = 0;
				f->mode = (f->mode&SF_RDWR)|SF_INIT;
				f->flags &= ~SF_HOLE;	/* off /dev/null handling */
			}
		}

		SFOPEN(f,0);
	}

	/* notify changes */
	if(_Sfnotify)
		(*_Sfnotify)(f,SF_SETFD,newfd);
	f->file = newfd;
	return newfd;
}
