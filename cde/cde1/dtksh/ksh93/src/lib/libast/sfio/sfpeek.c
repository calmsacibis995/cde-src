/* $XConsortium: sfpeek.c /main/cde1_maint/3 1995/10/14 01:11:18 montyb $ */
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

/*	Safe access to the internal stream buffer.
**	This function is obsolete. sfreserve() should be used.
**
**	Written by Kiem-Phong Vo (06/27/90).
*/

#if __STD_C
sfpeek(reg Sfio_t* f, Void_t** bp, reg int size)
#else
sfpeek(f,bp,size)
reg Sfio_t*	f;	/* file to peek */
Void_t**	bp;	/* start of data area */
reg int		size;	/* size of peek */
#endif
{	reg int	n;

	/* query for the extent of the remainder of the buffer */
	if(!bp || size == 0)
	{	if(f->mode&SF_INIT)
			(void)_sfmode(f,0,0);

		if((f->flags&SF_RDWRSTR) == SF_RDWRSTR)
		{	SFSTRSIZE(f);
			n = (f->data+f->here) - f->next;
		}
		else	n = f->endb - f->next;

		if(!bp)
			return n;
		else if(n > 0)	/* size == 0 */
		{	*bp = (Void_t*)f->next;
			return 0;
		}
		/* else fall down and fill buffer */
	}

	if(!(n = f->flags&SF_READ) )
		n = SF_WRITE;
	if(f->mode != n && _sfmode(f,n,0) < 0)
		return -1;

	*bp = sfreserve(f, size <= 0 ? 0 : size > f->size ? f->size : size, 0);

	if(*bp && size >= 0)
		return size;

	if((n = sfslen()) > 0)
	{	*bp = (Void_t*)f->next;
		if(size < 0)
		{	f->mode |= SF_PEEK;
			f->endr = f->endw = f->data;
		}
		else
		{	if(size > n)
				size = n;
			f->next += size;
		}
	}

	return (size >= 0 && n >= size) ? size : n;
}
