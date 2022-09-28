/* $XConsortium: sfprints.c /main/cde1_maint/3 1995/10/14 01:12:21 montyb $ */
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

/*	Construct a string with the given format and data.
**	This function allocates space as necessary to store the string.
**	This avoids overflow problems typical with sprintf() in stdio.
**
**	Written by Kiem-Phong Vo (06/27/90)
*/

#if __STD_C
char *sfprints(const char *form, ...)
#else
char *sfprints(va_alist)
va_dcl
#endif
{
	va_list		args;
	reg int		rv;
	static Sfio_t	*f;

#if __STD_C
	va_start(args,form);
#else
	reg char	*form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	/* make a fake stream */
	if(!f && !(f = sfnew(NIL(Sfio_t*),NIL(char*),-1,-1,SF_WRITE|SF_STRING)) )
		return NIL(char*);

	sfseek(f,0L,0);
	rv = sfvprintf(f,form,args);
	va_end(args);

	if(rv < 0 || sfputc(f,'\0') < 0)
		return NIL(char*);

	_Sfi = (f->next - f->data) - 1;
	return (char*)f->data;
}
