/* $XConsortium: _sfgetl.c /main/cde1_maint/3 1995/10/14 01:05:50 montyb $ */
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

#if __STD_C
static long __sfgetl(reg Sfio_t* f)
#else
static long __sfgetl(f)
reg Sfio_t	*f;
#endif
{
	return sfgetl(f);
}

#undef sfgetl

#if __STD_C
long sfgetl(reg Sfio_t* f)
#else
long sfgetl(f)
reg Sfio_t	*f;
#endif
{
	return __sfgetl(f);
}
