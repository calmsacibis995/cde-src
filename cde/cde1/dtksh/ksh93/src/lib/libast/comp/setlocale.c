/* $XConsortium: setlocale.c /main/cde1_maint/3 1995/10/14 00:29:19 montyb $ */
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

#undef	setlocale
#undef	strcmp
#undef	strcoll

typedef struct
{
	int		category;
	int		set;
	char*		locale;
} Locale_t;

char*
_ast_setlocale __PARAM__((register int category, const char* locale), (category, locale)) __OTORP__(register int category; const char* locale;){
#if _hdr_locale && _lib_setlocale
	register Locale_t*	lc;
	register char*		p;

	static Locale_t		def[] =
	{
		{	LC_ALL,		~0		},
		{	LC_COLLATE,	LC_SET_COLLATE	},
		{	LC_CTYPE,	LC_SET_CTYPE	},
#ifdef LC_MESSAGES
		{	LC_MESSAGES,	LC_SET_MESSAGES	},
#endif
		{	LC_MONETARY,	LC_SET_MONETARY	},
		{	LC_NUMERIC,	LC_SET_NUMERIC	},
		{	LC_TIME,	LC_SET_TIME	},
	};

	if (!def[0].locale)
		for (lc = def; lc < def + elementsof(def); lc++)
		{
			if (!(p = setlocale(lc->category, NiL)) || !(p = strdup(p)))
				p = "C";
			lc->locale = p;
		}
	if ((p = setlocale(category, locale)) && locale)
	{
		ast.locale.serial++;
		for (lc = def; lc < def + elementsof(def); lc++)
			if (lc->category == category)
			{
				if (streq(lc->locale, p))
				{
					ast.locale.set &= ~lc->set;
					if (lc->set & LC_SET_COLLATE)
						ast.collate = strcmp;
				}
				else
				{
					ast.locale.set |= lc->set;
					if (lc->set & LC_SET_COLLATE)
						ast.collate = strcoll;
				}
				break;
			}
	}
	return(p);
#else
	return(!locale || !*locale || !strcmp(locale, "C") || !strcmp(locale, "POSIX") ? "C" : (char*)0);
#endif
}