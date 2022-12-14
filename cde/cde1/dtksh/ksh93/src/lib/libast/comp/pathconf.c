/* $XConsortium: pathconf.c /main/cde1_maint/1 1995/07/17 23:39:25 drk $ */
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

#line 3
#include <ast.h>

#if _lib_pathconf

NoN(pathconf)

#else

#include <ls.h>
#include <errno.h>
#include <ast_param.h>

#ifndef errno
extern __MANGLE__ int	errno;
#endif

static long
statconf __PARAM__((struct stat* st, int op), (st, op)) __OTORP__(struct stat* st; int op;)
#line 21
{
	NoP(st);
	switch (op)
	{
	case _PC_LINK_MAX:
		return(LINK_MAX);
	case _PC_MAX_CANON:
		return(MAX_CANON);
	case _PC_MAX_INPUT:
		return(MAX_INPUT);
	case _PC_NAME_MAX:
#ifdef NAME_MAX
		return(NAME_MAX);
#else
		return(-1);
#endif
	case _PC_PATH_MAX:
#ifdef PATH_MAX
		return(PATH_MAX);
#else
		return(-1);
#endif
	case _PC_PIPE_BUF:
#ifdef PIPE_BUF
		return(PIPE_BUF);
#else
		return(-1);
#endif
	case _PC_CHOWN_RESTRICTED:
#ifdef _POSIX_CHOWN_RESTRICTED
		return(_POSIX_CHOWN_RESTRICTED);
#else
		return(-1);
#endif
	case _PC_NO_TRUNC:
#ifdef _POSIX_NO_TRUNC
		return(_POSIX_NO_TRUNC);
#else
		return(-1);
#endif
	case _PC_VDISABLE:
#ifdef _POSIX_VDISABLE
		return(_POSIX_VDISABLE);
#else
		return(-1);
#endif
	}
	errno = EINVAL;
	return(-1);
}

long
fpathconf __PARAM__((int fd, int op), (fd, op)) __OTORP__(int fd; int op;)
#line 74
{
	struct stat	st;

	return(fstat(fd, &st) ? -1 : statconf(&st, op));
}

long
pathconf __PARAM__((const char* path, int op), (path, op)) __OTORP__(const char* path; int op;)
#line 82
{
	struct stat	st;

	return(stat(path, &st) ? -1 : statconf(&st, op));
}

#endif
