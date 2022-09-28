/* $XConsortium: sfio_t.h /main/cde1_maint/3 1995/10/14 01:10:01 montyb $ */
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
#ifndef _SFIO_T_H
#define _SFIO_T_H	1

/*	This header file is for library writers who need to know certain
**	internal info concerning the full Sfio_t structure. Including this
**	file means that you agree to track closely with sfio development
**	in case its internal architecture is changed.
**
**	Written by Kiem-Phong Vo (02/02/1993).
*/

/* the parts of Sfio_t private to sfio functions */
#define _SFIO_PRIVATE \
	long		extent;	/* current file	size		*/ \
	long		here;	/* current physical location	*/ \
	unsigned char	getr;	/* the last sfgetr separator 	*/ \
	unsigned char	tiny[1];/* for unbuffered read stream	*/ \
	unsigned short	mode;	/* current io mode		*/ \
	struct _sfdc_*	disc;	/* discipline			*/ \
	struct _sfpl_*	pool;	/* the pool containing this	*/

#include	"sfio.h"

/* mode bit to indicate that the structure hasn't been initialized */
#define SF_INIT		0000004

/* short-hand for common stream types */
#define SF_RDWR		(SF_READ|SF_WRITE)
#define SF_RDSTR	(SF_READ|SF_STRING)
#define SF_WRSTR	(SF_WRITE|SF_STRING)
#define SF_RDWRSTR	(SF_RDWR|SF_STRING)

/* macro to initialize an Sfio_t structure */
#define SFNEW(data,size,file,type,disc)	\
	{ (unsigned char*)(data),			/* next		*/ \
	  (unsigned char*)(data),			/* endw		*/ \
	  (unsigned char*)(data),			/* endr		*/ \
	  (unsigned char*)(data),			/* endb		*/ \
	  (struct _sfio_*)0,				/* push		*/ \
	  (unsigned short)((type)&SF_FLAGS),		/* flags	*/ \
	  (short)(file),				/* file		*/ \
	  (unsigned char*)(data),			/* data		*/ \
	  (int)(size),					/* size		*/ \
	  0L,						/* extent	*/ \
	  0L,						/* here		*/ \
	  0,						/* getr		*/ \
	  "",						/* tiny		*/ \
	  (unsigned short)(((type)&(SF_RDWR))|SF_INIT),	/* mode		*/ \
	  (struct _sfdc_*)(disc),			/* disc		*/ \
	  (struct _sfpl_*)0,				/* pool		*/ \
	}

#endif /* _SFIO_T_H */
