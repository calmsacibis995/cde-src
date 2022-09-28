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
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide printf
#else
#define printf		______printf
#endif

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE	1
#endif

#include <sys/types.h>

#include "FEATURE/lib"
#include "FEATURE/limits.lcl"
#include "FEATURE/unistd.lcl"
#include "FEATURE/param"
#include "FEATURE/types"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide printf
#endif

#if defined(__STDPP__hide) || defined(printf)
#undef	printf
extern __MANGLE__ int		printf __PROTO__((const char*, ...));
#endif

#include "conflib.h"

main()
{
	register int	i;
	register int	n;

	char		c;
	unsigned char	uc;
	unsigned short	us;
	unsigned int	ui;
	unsigned long	ul;
	unsigned long	val;

	/*
	 * <limits.h> with *constant* valued macros
	 */

	printf("\n");
#ifdef CHAR_BIT
	val = CHAR_BIT;
	printf("#undef	CHAR_BIT\n");
#else
	uc = 0;
	uc = ~uc;
	val = 1;
	while (uc >>= 1) val++;
#endif
	printf("#define CHAR_BIT	%lu\n", val);
#ifdef MB_LEN_MAX
	val = MB_LEN_MAX;
	printf("#undef	MB_LEN_MAX\n");
#else
	val = 1;
#endif
	printf("#define MB_LEN_MAX	%lu\n", val);

	c = 0;
	c = ~c;
	uc = 0;
	uc = ~uc;
	us = 0;
	us = ~us;
	ui = 0;
	ui = ~ui;
	ul = 0;
	ul = ~ul;

#ifdef UCHAR_MAX
	val = UCHAR_MAX;
	printf("#undef	UCHAR_MAX\n");
#else
	val = uc;
#endif
	printf("#if defined(__STDC__)\n");
	printf("#define UCHAR_MAX	%luU\n", val);
	printf("#else\n");
	printf("#define UCHAR_MAX	%lu\n", val);
	printf("#endif\n");

#ifdef SCHAR_MIN
	val = -(SCHAR_MIN);
	printf("#undef	SCHAR_MIN\n");
#else
	val = (unsigned char)(uc >> 1) + 1;
#endif
	printf("#define SCHAR_MIN	-%lu\n", val);

#ifdef SCHAR_MAX
	val = SCHAR_MAX;
	printf("#undef	SCHAR_MAX\n");
#else
	val = (unsigned char)(uc >> 1);
#endif
	printf("#define SCHAR_MAX	%lu\n", val);

	if (c < 0)
	{
#ifdef CHAR_MIN
		printf("#undef	CHAR_MIN\n");
#endif
		printf("#define CHAR_MIN	SCHAR_MIN\n");

#ifdef CHAR_MAX
		printf("#undef	CHAR_MAX\n");
#endif
		printf("#define CHAR_MAX	SCHAR_MAX\n");
	}
	else
	{
#ifdef CHAR_MIN
		printf("#undef	CHAR_MIN\n");
#endif
		printf("#define CHAR_MIN	0\n");

#ifdef CHAR_MAX
		printf("#undef	CHAR_MAX\n");
#endif
		printf("#define CHAR_MAX	UCHAR_MAX\n");
	}

#ifdef USHRT_MAX
	val = USHRT_MAX;
	printf("#undef	USHRT_MAX\n");
#else
	val = us;
#endif
	printf("#if defined(__STDC__)\n");
	printf("#define USHRT_MAX	%luU\n", val);
	printf("#else\n");
	printf("#define USHRT_MAX	%lu\n", val);
	printf("#endif\n");

#ifdef SHRT_MIN
	val = -(SHRT_MIN);
	printf("#undef	SHRT_MIN\n");
#else
	val = (unsigned short)(us >> 1) + 1;
#endif
	printf("#define SHRT_MIN	-%lu\n", val);

#ifdef SHRT_MAX
	val = SHRT_MAX;
	printf("#undef	SHRT_MAX\n");
#else
	val = (unsigned short)(us >> 1);
#endif
	printf("#define SHRT_MAX	%lu\n", val);

	if (ui == us)
	{
#ifdef UINT_MAX
		printf("#undef	UINT_MAX\n");
#endif
		printf("#define UINT_MAX	USHRT_MAX\n");

#ifdef INT_MIN
		printf("#undef	INT_MIN\n");
#endif
		printf("#define INT_MIN		SHRT_MIN\n");

#ifdef INT_MAX
		printf("#undef	INT_MAX\n");
#endif
		printf("#define INT_MAX		SHRT_MAX\n");
	}
	else
	{
#ifdef UINT_MAX
		val = UINT_MAX;
		printf("#undef	UINT_MAX\n");
#else
		val = ui;
#endif
		printf("#if defined(__STDC__)\n");
		printf("#define UINT_MAX	%luU\n", val);
		printf("#else\n");
		printf("#define UINT_MAX	%lu\n", val);
		printf("#endif\n");

#ifdef INT_MIN
		val = -(INT_MIN);
		printf("#undef	INT_MIN\n");
#else
		val = (unsigned int)(ui >> 1) + 1;
#endif
		if (ui == ul) printf("#define INT_MIN		(-%lu-1)\n", val - 1);
		else printf("#define INT_MIN		-%lu\n", val);

#ifdef INT_MAX
		val = INT_MAX;
		printf("#undef	INT_MAX\n");
#else
		val = (unsigned int)(ui >> 1);
#endif
		printf("#define INT_MAX		%lu\n", val);
	}

	if (ul == ui)
	{
#ifdef ULONG_MAX
		printf("#undef	ULONG_MAX\n");
#endif
		printf("#define ULONG_MAX	UINT_MAX\n");

#ifdef LONG_MIN
		printf("#undef	LONG_MIN\n");
#endif
		printf("#define LONG_MIN	INT_MIN\n");

#ifdef LONG_MAX
		printf("#undef	LONG_MAX\n");
#endif
		printf("#define LONG_MAX	INT_MAX\n");
	}
	else
	{
#ifdef ULONG_MAX
		val = ULONG_MAX;
		printf("#undef	ULONG_MAX\n");
#else
		val = ui;
#endif
		printf("#if defined(__STDC__)\n");
		printf("#define ULONG_MAX	%luU\n", val);
		printf("#else\n");
		printf("#define ULONG_MAX	%lu\n", val);
		printf("#endif\n");

#ifdef LONG_MIN
		val = -(LONG_MIN);
		printf("#undef	LONG_MIN\n");
#else
		val = (unsigned long)(ul >> 1) + 1;
#endif
		printf("#define LONG_MIN	(-%lu-1)\n", val - 1);

#ifdef LONG_MAX
		val = LONG_MAX;
		printf("#undef	LONG_MAX\n");
#else
		val = (unsigned long)(ul >> 1);
#endif
		printf("#define LONG_MAX	%lu\n", val);
	}

#ifdef ULONGLONG_MAX
	printf("#undef	ULONGLONG_MAX\n");
	printf("#if defined(__STDC__)\n");
	printf("#define ULONGLONG_MAX	%lluLLU\n", ULONGLONG_MAX);
	printf("#else\n");
	printf("#define ULONGLONG_MAX	%llu\n", ULONGLONG_MAX);
	printf("#endif\n");
#endif

#ifdef LONGLONG_MIN
	printf("#undef	LONGLONG_MIN\n");
	printf("#if defined(__STDC__)\n");
	printf("#define LONGLONG_MIN	%lldLL\n", LONGLONG_MIN);
	printf("#else\n");
	printf("#define LONGLONG_MIN	%lld\n", LONGLONG_MIN);
	printf("#endif\n");
#endif

#ifdef LONGLONG_MAX
	printf("#undef	LONGLONG_MAX\n");
	printf("#if defined(__STDC__)\n");
	printf("#define LONGLONG_MAX	%lldLL\n", LONGLONG_MAX);
	printf("#else\n");
	printf("#define LONGLONG_MAX	%lld\n", LONGLONG_MAX);
	printf("#endif\n");
#endif

	printf("\n");
#include "conflim.h"
	printf("\n");

	/*
	 * pollution control
	 */

	printf("/*\n");
	printf(" * pollution control\n");
	printf(" */\n");
	printf("\n");
	printf("#if defined(__STDPP__directive) && defined(__STDPP__ignore)\n");
	printf("__STDPP__directive pragma pp:ignore \"limits.h\"\n");
	printf("#else\n");
#ifdef	_limits_h
	printf("#define _limits_h\n");
#endif
#ifdef	__limits_h
	printf("#define __limits_h\n");
#endif
#ifdef	_sys_limits_h
	printf("#define _sys_limits_h\n");
#endif
#ifdef	__sys_limits_h
	printf("#define __sys_limits_h\n");
#endif
#ifdef	_LIMITS_H_
	printf("#define _LIMITS_H_\n");
#endif
#ifdef	__LIMITS_H
	printf("#define __LIMITS_H\n");
#endif
#ifdef	__LIMITS_INCLUDED
	printf("#define __LIMITS_INCLUDED\n");
#endif
#ifdef	_MACH_MACHLIMITS_H_
	printf("#define _MACH_MACHLIMITS_H_\n");
#endif
#ifdef	_SYS_LIMITS_H_
	printf("#define _SYS_LIMITS_H_\n");
#endif
#ifdef	__SYS_LIMITS_H
	printf("#define __SYS_LIMITS_H\n");
#endif
#ifdef	__SYS_LIMITS_INCLUDED
	printf("#define __SYS_LIMITS_INCLUDED\n");
#endif
#ifdef	_SYS_SYSLIMITS_H_
	printf("#define _SYS_SYSLIMITS_H_\n");
#endif
#ifdef	_H_LIMITS
	printf("#define _H_LIMITS\n");
#endif
#ifdef	__H_LIMITS
	printf("#define __H_LIMITS\n");
#endif
#ifdef	_H_SYS_LIMITS
	printf("#define _H_SYS_LIMITS\n");
#endif
#ifdef	__H_SYS_LIMITS
	printf("#define __H_SYS_LIMITS\n");
#endif
	printf("#endif\n");
	printf("\n");

	return(0);
}