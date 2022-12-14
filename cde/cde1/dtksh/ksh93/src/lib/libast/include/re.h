/* $XConsortium: re.h /main/cde1_maint/3 1995/10/14 00:42:13 montyb $ */
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

/* : : generated by proto : : */
                  
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * regular expression library definitions
 */

#ifndef _RE_H
#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif

#define _RE_H

#include <sfio.h>

#define RE_ALL		(1<<0)	/* substitute all occurrences		*/
#define RE_EDSTYLE	(1<<1)	/* ed(1) style magic characters		*/
#define RE_LOWER	(1<<2)	/* substitute to lower case		*/
#define RE_MATCH	(1<<3)	/* record matches in Re_program_t.match	*/
#define RE_UPPER	(1<<4)	/* substitute to upper case		*/
#define RE_EXTERNAL	8	/* first external flag bit		*/

typedef struct			/* sub-expression match			*/
{
	char*	sp;		/* start in source string		*/
	char*	ep;		/* end in source string			*/
} Re_match_t;

typedef struct			/* compiled regular expression program	*/
{
#ifdef _RE_PROGRAM_PRIVATE_
	_RE_PROGRAM_PRIVATE_
#else
	Re_match_t	match['9'-'0'+1];/* sub-expression match table*/
#endif
} Re_program_t, reprogram;

/*
 * interface routines
 */

extern __MANGLE__ Re_program_t*	recomp __PROTO__((const char*, int));
extern __MANGLE__ int		reexec __PROTO__((Re_program_t*, const char*));
extern __MANGLE__ void		refree __PROTO__((Re_program_t*));
extern __MANGLE__ void		reerror __PROTO__((const char*));
extern __MANGLE__ char*		resub __PROTO__((Re_program_t*, const char*, const char*, char*, int));
extern __MANGLE__ void		ressub __PROTO__((Re_program_t*, Sfio_t*, const char*, const char*, int));

#endif
