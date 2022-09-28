/* $XConsortium: sfextern.c /main/cde1_maint/3 1995/10/14 01:08:35 montyb $ */
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

/*	External variables used by sfio
**	Written by Kiem-Phong Vo (06/27/90)
*/

/* global variables used internally to the package */
Sfext_t _Sfextern =
{	{ NIL(Sfpool_t*), 0, 0, 0, NIL(Sfio_t**) },	/* _Sfpool	*/
	NIL(Sfio_t*),					/* _Sffree	*/
	NIL(Fa_t*),					/* _Fafree	*/
	0,						/* _Sfpage	*/
	NIL(int(*)_ARG_((Sfio_t*,int))),		/* _Sfpmove	*/
	NIL(Sfio_t*(*)_ARG_((Sfio_t*, Sfio_t*))),	/* _Sfstack	*/
	NIL(void(*)_ARG_((Sfio_t*, int, int))),		/* _Sfnotify	*/
	NIL(int(*)_ARG_((Sfio_t*))),			/* _Sfstdio	*/
	{ NIL(Sfread_f),				/* _Sfudisc	*/
	  NIL(Sfwrite_f),
	  NIL(Sfseek_f),
	  NIL(Sfexcept_f),
	  NIL(Sfdisc_t*)
	},
	NIL(void(*)_ARG_((void)) ),			/* _Sfcleanup	*/
	0						/* _Sfexiting	*/
};

/* accessible to application code for a few fast macro functions */
int	_Sfi = -1;

Sfio_t _Sfstdin  = SFNEW(NIL(char*),-1,0,(SF_READ|SF_STATIC),NIL(Sfdisc_t*));
Sfio_t _Sfstdout = SFNEW(NIL(char*),-1,1,(SF_WRITE|SF_STATIC),NIL(Sfdisc_t*));
Sfio_t _Sfstderr = SFNEW(NIL(char*),-1,2,(SF_WRITE|SF_STATIC),NIL(Sfdisc_t*));
