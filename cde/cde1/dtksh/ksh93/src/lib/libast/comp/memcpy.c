/* $XConsortium: memcpy.c /main/cde1_maint/3 1995/10/14 00:27:16 montyb $ */
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
#include <ast.h>

#ifdef _lib_memcpy

NoN(memcpy)

#else

#undef	memcpy

#ifdef _lib_bcopy

extern __MANGLE__ void	bcopy __PROTO__((__V_*, __V_*, size_t));

__V_*
memcpy __PARAM__((__V_* s1, __V_* s2, size_t n), (s1, s2, n)) __OTORP__(__V_* s1; __V_* s2; size_t n;){
	bcopy(s2, s1, n);
	return(s1);
}

#else

__V_*
memcpy __PARAM__((__V_* as1, const __V_* as2, register size_t n), (as1, as2, n)) __OTORP__(__V_* as1; const __V_* as2; register size_t n;){
	register char*		s1 = (char*)as1;
	register const char*	s2 = (const char*)as2;

	while (n-- > 0)
		*s1++ = *s2++;
	return(as1);
}

#endif

#endif