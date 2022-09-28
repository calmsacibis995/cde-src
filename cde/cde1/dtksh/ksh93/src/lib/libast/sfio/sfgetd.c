/* $XConsortium: sfgetd.c /main/cde1_maint/3 1995/10/14 01:09:13 montyb $ */
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

/*	Read a portably coded double value
**
**	Written by Kiem-Phong Vo (08/05/90)
*/

#if __STD_C
double sfgetd(Sfio_t* f)
#else
double sfgetd(f)
Sfio_t	*f;
#endif
{
	reg uchar	*s, *ends, c;
	reg double	v;
	reg int		p, sign, exp;

	if((sign = sfgetc(f)) < 0 || (exp = (int)sfgetu(f)) < 0)
		return -1.;

	SFLOCK(f,0);

	v = 0.;
	for(;;)
	{	/* fast read for data */
		if(SFRPEEK(f,s,p) <= 0)
		{	f->flags |= SF_ERROR;
			v = -1.;
			goto done;
		}

		for(ends = s+p; s < ends; )
		{
			c = *s++;
			v += SFUVALUE(c);
			v = ldexp(v,-SF_PRECIS);
			if(!(c&SF_MORE))
			{	f->next = s;
				goto done;
			}
		}
		f->next = s;
	}

done:
	v = ldexp(v,(sign&02) ? -exp : exp);
	if(sign&01)
		v = -v;

	SFOPEN(f,0);
	return v;
}
