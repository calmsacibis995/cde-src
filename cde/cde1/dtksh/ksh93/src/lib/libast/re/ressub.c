/* $XConsortium: ressub.c /main/cde1_maint/3 1995/10/14 01:04:42 montyb $ */
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
#include "relib.h"

#undef	next

#include <ctype.h>

/*
 * do a single substitution
 */

static void
sub __PARAM__((register Sfio_t* dp, register const char* sp, register Match_t* mp, register int flags), (dp, sp, mp, flags)) __OTORP__(register Sfio_t* dp; register const char* sp; register Match_t* mp; register int flags;){
	register int	c;
	char*		s;
	char*		e;

	flags &= (RE_LOWER|RE_UPPER);
	for (;;) switch (c = *sp++)
	{
	case 0:
		return;
	case '\\':
		switch (c = *sp++)
		{
		case 0:
			sp--;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			c -= '0';
			goto group;
		default:
			sfputc(dp, chresc(sp - 2, &s));
			sp = s;
			break;
		}
		break;
	case '&':
		c = 0;
	group:
		if (s = mp[c].sp)
		{
			e = mp[c].ep;
			while (s < e)
			{
				c = *s++;
				switch (flags)
				{
				case RE_UPPER:
					if (islower(c)) c = toupper(c);
					break;
				case RE_LOWER:
					if (isupper(c)) c = tolower(c);
					break;
				}
				sfputc(dp, c);
			}
		}
		break;
	default:
		switch (flags)
		{
		case RE_UPPER:
			if (islower(c)) c = toupper(c);
			break;
		case RE_LOWER:
			if (isupper(c)) c = tolower(c);
			break;
		}
		sfputc(dp, c);
		break;
	}
}

/*
 * ed(1) style substitute using matches from last reexec()
 */

void
ressub __PARAM__((Re_program_t* re, Sfio_t* dp, register const char* op, const char* sp, int flags), (re, dp, op, sp, flags)) __OTORP__(Re_program_t* re; Sfio_t* dp; register const char* op; const char* sp; int flags;){
	register Match_t*	mp;

	mp = re->subexp.m;
	do
	{
		sfwrite(dp, op, mp->sp - op);
		sub(dp, sp, mp, flags);
		op = mp->ep;
	} while ((flags & RE_ALL) && *op && mp->sp != mp->ep && reexec(re, op));
	sfputr(dp, op, -1);
}
