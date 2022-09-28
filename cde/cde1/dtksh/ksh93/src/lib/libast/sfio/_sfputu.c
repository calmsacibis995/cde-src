/* $XConsortium: _sfputu.c /main/cde1_maint/3 1995/10/14 01:06:48 montyb $ */
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
static __sfputu(reg Sfio_t* f, reg ulong v)
#else
static __sfputu(f,v)
reg Sfio_t	*f;
reg ulong	v;
#endif
{
	return sfputu(f,v);
}

#undef sfputu

#if __STD_C
sfputu(reg Sfio_t* f, reg ulong v)
#else
sfputu(f,v)
reg Sfio_t	*f;
reg ulong	v;
#endif
{
	return __sfputu(f,v);
}
