/* $XConsortium: unistd.c /main/cde1_maint/1 1995/07/17 23:48:31 drk $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*          Copyright (c) 1994 AT&T Bell Laboratories           *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                   advsoft@research.att.com                   *
*                 Randy Hackbarth 908-582-5245                 *
*                  Dave Belanger 908-582-7427                  *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#line 1
                  
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * generate unistd.h definitions for posix sysconf() args
 */

#ifndef _POSIX_SOURCE
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

#line 9

#define _POSIX_SOURCE
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide printf
#else
#define printf		______printf
#endif

#include <sys/types.h>

#include "FEATURE/types"
#include "FEATURE/lcl.unistd"
#include <ast_lib.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide printf
#endif

#if defined(__STDPP__hide) || defined(printf)
#undef	printf
extern __MANGLE__ int		printf __PROTO__((const char*, ...));
#endif

main()
{
	int	sep = 0;
	long	val;

	/*
	 * some systems (could it beee aix) think empty definitions
	 * constitute symbolic constants
	 */

#ifdef	_POSIX_VERSION
	{
		static long	x[] = { 0, _POSIX_VERSION };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_VERSION\n");
			val = 199009L;
		}
		else val = x[1];
	}
#else
	val = 199009L;
#endif
	printf("#define _POSIX_VERSION		%ldL\n", val);

#ifndef _POSIX_JOB_CONTROL
#ifdef	WUNTRACED
#define	_POSIX_JOB_CONTROL	1
#endif
#endif
#ifdef	_POSIX_JOB_CONTROL
	{
		static long	x[] = { 0, _POSIX_JOB_CONTROL };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_JOB_CONTROL\n");
			val = 1;
		}
		else val = x[1];
	}
	printf("#define _POSIX_JOB_CONTROL	%ld\n", val);
#endif

#ifndef _POSIX_SAVED_IDS
#if _lib_setuid && !_lib_setreuid
#define _POSIX_SAVED_IDS	1
#endif
#endif
#ifdef	_POSIX_SAVED_IDS
	{
		static long	x[] = { 0, _POSIX_SAVED_IDS };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_SAVED_IDS\n");
			val = 1;
		}
		else val = x[1];
	}
	printf("#define _POSIX_SAVED_IDS	%ld\n", val);
#endif
	printf("\n");

#ifdef	_POSIX_CHOWN_RESTRICTED
	{
		static long	x[] = { 0, _POSIX_CHOWN_RESTRICTED };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_CHOWN_RESTRICTED\n");
			val = 1;
		}
		else val = x[1];
	}
	printf("#define _POSIX_CHOWN_RESTRICTED	%ld\n", val);
	sep = 1;
#endif

#ifdef	_POSIX_NO_TRUNC
	{
		static long	x[] = { 0, _POSIX_NO_TRUNC };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_NO_TRUNC\n");
			val = 1;
		}
		else val = x[1];
	}
	printf("#define _POSIX_NO_TRUNC		%ld\n", val);
	sep = 1;
#endif

#ifdef	_POSIX_VDISABLE
	{
		static long	x[] = { 0, _POSIX_VDISABLE };

		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	_POSIX_VDISABLE\n");
			val = 1;
		}
		else val = x[1];
	}
	printf("#define _POSIX_VDISABLE		%ld\n", val);
	sep = 1;
#endif

	if (sep)
	{
		sep = 0;
		printf("\n");
	}
#ifndef _SC_ARG_MAX
#define _SC_ARG_MAX	(-1)
#endif
	printf("#define _SC_ARG_MAX		%ld\n", (long)_SC_ARG_MAX);
#ifndef _SC_CHILD_MAX
#define _SC_CHILD_MAX	(-2)
#endif
	printf("#define _SC_CHILD_MAX		%ld\n", (long)_SC_CHILD_MAX);
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK	(-3)
#endif
	printf("#define _SC_CLK_TCK		%ld\n", (long)_SC_CLK_TCK);
#ifndef _SC_NGROUPS_MAX
#define _SC_NGROUPS_MAX	(-4)
#endif
	printf("#define _SC_NGROUPS_MAX		%ld\n", (long)_SC_NGROUPS_MAX);
#ifndef _SC_OPEN_MAX
#define _SC_OPEN_MAX	(-5)
#endif
	printf("#define _SC_OPEN_MAX		%ld\n", (long)_SC_OPEN_MAX);
#ifndef _SC_JOB_CONTROL
#define _SC_JOB_CONTROL	(-6)
#endif
	printf("#define _SC_JOB_CONTROL		%ld\n", (long)_SC_JOB_CONTROL);
#ifndef _SC_SAVED_IDS
#define _SC_SAVED_IDS	(-7)
#endif
	printf("#define _SC_SAVED_IDS		%ld\n", (long)_SC_SAVED_IDS);
#ifndef _SC_TZNAME_MAX
#define _SC_TZNAME_MAX	(-8)
#endif
	printf("#define _SC_TZNAME_MAX		%ld\n", (long)_SC_TZNAME_MAX);
#ifndef _SC_VERSION
#define _SC_VERSION	(-9)
#endif
	printf("#define _SC_VERSION		%ld\n", (long)_SC_VERSION);
	printf("\n");
#ifndef _PC_LINK_MAX
#define _PC_LINK_MAX	(-1)
#endif
	printf("#define _PC_LINK_MAX		%ld\n", (long)_PC_LINK_MAX);
#ifndef _PC_MAX_CANON
#define _PC_MAX_CANON	(-2)
#endif
	printf("#define _PC_MAX_CANON		%ld\n", (long)_PC_MAX_CANON);
#ifndef _PC_MAX_INPUT
#define _PC_MAX_INPUT	(-3)
#endif
	printf("#define _PC_MAX_INPUT		%ld\n", (long)_PC_MAX_INPUT);
#ifndef _PC_NAME_MAX
#define _PC_NAME_MAX	(-4)
#endif
	printf("#define _PC_NAME_MAX		%ld\n", (long)_PC_NAME_MAX);
#ifndef _PC_PATH_MAX
#define _PC_PATH_MAX	(-5)
#endif
	printf("#define _PC_PATH_MAX		%ld\n", (long)_PC_PATH_MAX);
#ifndef _PC_PIPE_BUF
#define _PC_PIPE_BUF	(-6)
#endif
	printf("#define _PC_PIPE_BUF		%ld\n", (long)_PC_PIPE_BUF);
#ifndef _PC_CHOWN_RESTRICTED
#define _PC_CHOWN_RESTRICTED	(-7)
#endif
	printf("#define _PC_CHOWN_RESTRICTED	%ld\n", (long)_PC_CHOWN_RESTRICTED);
#ifndef _PC_NO_TRUNC
#define _PC_NO_TRUNC	(-8)
#endif
	printf("#define _PC_NO_TRUNC		%ld\n", (long)_PC_NO_TRUNC);
#ifndef _PC_VDISABLE
#define _PC_VDISABLE	(-9)
#endif
	printf("#define _PC_VDISABLE		%ld\n", (long)_PC_VDISABLE);
#ifndef _PC_LAST
#define _PC_LAST	(-10)
#endif
	printf("#define _PC_LAST		%ld\n", (long)_PC_LAST);
	return(0);
}
