/* $XConsortium: chmod.c /main/cde1_maint/3 1995/10/14 01:33:10 montyb $ */
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
static const char id[] = "\n@(#)chmod (AT&T Bell Laboratories) 05/09/95\0\n";

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide lchmod
#else
#define lchmod		______lchmod
#endif

#include <cmdlib.h>
#include <ls.h>
#include <ftwalk.h>

#include "FEATURE/symlink"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide lchmod
#else
#undef	lchmod
#endif

extern __MANGLE__ int	lchmod __PROTO__((const char*, mode_t));

int
b_chmod __PARAM__((int argc, char* argv[]), (argc, argv)) __OTORP__(int argc; char* argv[];){
	register int	mode;
	register int	force = 0;
	register int	recurse = 0;
	register int	resolve;
	register char*	amode;
	register char*	file;
	char*		last;
	struct stat	st;
	int		(*chmodf) __PROTO__((const char*, mode_t));
	int		(*statf) __PROTO__((const char*, struct stat*));

	NoP(id[0]);
	cmdinit(argv);
	resolve = ftwflags();

	/*
	 * NOTE: we diverge from the normal optget boilerplate
	 *	 to allow `chmod -x etc' to fall through
	 */

	while (mode = optget(argv, "fHLPR mode file ..."))
	{
		switch (mode)
		{
		case 'f':
			force++;
			continue;
		case 'H':
			resolve |= FTW_META|FTW_PHYSICAL;
			continue;
		case 'L':
			resolve &= ~(FTW_META|FTW_PHYSICAL);
			continue;
		case 'P':
			resolve &= ~FTW_META;
			resolve |= FTW_PHYSICAL;
			continue;
		case 'R':
			recurse++;
			continue;
		case '?':
			error(ERROR_usage(2), optusage(NiL));
			continue;
		}
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors || argc < 2)
		error(ERROR_usage(2), optusage(NiL));
	amode = *argv;
	mode = strperm(amode, &last, 0);
	if (*last)
		error(ERROR_exit(1), "%s: invalid mode", amode);
	if (recurse)
	{
		int	n = 0;
		char*	pfxv[3];

		if (force) pfxv[n++] = "-f";
		if (resolve & FTW_META) pfxv[n++] = "-H";
		else if (resolve & FTW_PHYSICAL) pfxv[n++] = "-P";
		pfxv[n] = 0;
		return(cmdrecurse(argc, argv, n, pfxv));
	}
	statf = (resolve & FTW_PHYSICAL) ? lstat : stat;
#if _lib_lchmod
	if (resolve & FTW_PHYSICAL)
		chmodf = lchmod;
	else
#endif
	chmodf = chmod;
	while (file = *++argv)
	{
		if (!amode || !(*statf)(file, &st))
		{
			if (amode)
				mode = strperm(amode, &last, st.st_mode);
			if ((*chmodf)(file, mode) >= 0)
				continue;
		}
		if (!force)
		{
			error_info.errors = 1;
			error(ERROR_system(0), "%s: cannot change mode", file);
		}
	}
	return(error_info.errors);
}
