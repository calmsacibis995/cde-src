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
#include <error.h>

int
query __PARAM__((int quit, const char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(int quit; const char* format; )
	va_list		ap;
	register int	n;
	register int	c;

	static Sfio_t*	rfp;
	static Sfio_t*	wfp;

	__VA_START__(ap, format); __OTORP__(quit = va_arg(ap, int );format = va_arg(ap, const char* );)
	if (!format) return(0);
	if (!rfp)
	{
		c = errno;
		if (isatty(sffileno(sfstdin))) rfp = sfstdin;
		else if (!(rfp = sfopen(NiL, "/dev/tty", "r"))) return(-1);
		if (isatty(sffileno(sfstderr))) wfp = sfstderr;
		else if (!(wfp = sfopen(NiL, "/dev/tty", "w"))) return(-1);
		errno = c;
	}
	sfsync(sfstdout);
	sfvprintf(wfp, format, ap);
	sfsync(wfp);
	for (n = c = sfgetc(rfp);; c = sfgetc(rfp)) switch (c)
	{
	case EOF:
		n = c;
		/*FALLTHROUGH*/
	case '\n':
		switch (n)
		{
		case EOF:
		case 'q':
		case 'Q':
			if (quit >= 0) exit(quit);
			return(-1);
		case '1':
		case 'y':
		case 'Y':
		case '+':
			return(0);
		default:
			return(1);
		}
	}
	va_end(ap);
	/*NOTREACHED*/
}
