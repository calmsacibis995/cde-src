/* $XConsortium: fmtfs.c /main/cde1_maint/3 1995/10/14 01:19:21 montyb $ */
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
#include <ls.h>

#include "FEATURE/fs"

#if _str_st_fstype || !_hdr_mntent && !_hdr_mnttab

char*
fmtfs __PARAM__((struct stat* st), (st)) __OTORP__(struct stat* st;){
#if _str_st_fstype
	return(st->st_fstype);
#else
	return(FS_default);
#endif
}

#else

#include <stdio.h>
#include <hash.h>

#if _hdr_mntent && _lib_getmntent

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide endmntent getmntent
#else
#define endmntent	______endmntent
#define getmntent	______getmntent
#endif

#include <mntent.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide endmntent getmntent
#else
#undef	endmntent
#undef	getmntent
#endif

extern __MANGLE__ int		endmntent __PROTO__((FILE*));
extern __MANGLE__ struct mntent*	getmntent __PROTO__((FILE*));

#else

/*
 * shortened for tw info
 */

#if _hdr_mnttab
#define MOUNTED		"/etc/mnttab"
#define SEP		'\t'
#else
#define MOUNTED		"/etc/mtab"
#define SEP		' '
#endif
#define	MNTTYPE_IGNORE	"ignore"
#define setmntent(n,f)	fopen(n,f)

struct	mntent
{
	char	mnt_dir[256];
	char	mnt_type[32];
};

static struct mntent*
getmntent __PARAM__((FILE* mp), (mp)) __OTORP__(FILE* mp;){
	register int		c;
	register char*		s;
	register char*		m;
	register int		q;
	static struct mntent	e;

	q = 0;
	s = m = 0;
	for (;;) switch (c = getc(mp))
	{
	case EOF:
		return(0);
	case SEP:
		switch (++q)
		{
		case 1:
			s = e.mnt_dir;
			m = s + sizeof(e.mnt_dir) - 1;
			break;
		case 2:
			*s = 0;
			s = e.mnt_type;
			m = s + sizeof(e.mnt_type) - 1;
			break;
		case 3:
			*s = 0;
			s = m = 0;
			break;
		}
		break;
	case '\n':
		if (q >= 3) return(&e);
		q = 0;
		s = m = 0;
		break;
	default:
		if (s < m) *s++ = c;
		break;
	}
}

static int
endmntent __PARAM__((FILE* mp), (mp)) __OTORP__(FILE* mp;){
	fclose(mp);
	return(1);
}

#endif

#ifndef MOUNTED
#ifdef	MNT_MNTTAB
#define MOUNTED		MNT_MNTTAB
#else
#if _hdr_mnttab
#define MOUNTED		"/etc/mnttab"
#else
#define MOUNTED		"/etc/mtab"
#endif
#endif
#endif

char*
fmtfs __PARAM__((struct stat* st), (st)) __OTORP__(struct stat* st;){
	register FILE*		mp;
	register struct mntent*	mnt;
	register char*		s;
	struct stat		rt;

	static Hash_table_t*	tab;
	static char		typ[16];

	if ((tab || (tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(dev_t), HASH_name, "fstype", 0))) && (s = (char*)hashget(tab, &st->st_dev)))
		return(s);
	s = FS_default;
	if (mp = setmntent(MOUNTED, "r"))
	{
		while ((mnt = getmntent(mp)) && (!strcmp(mnt->mnt_type, MNTTYPE_IGNORE) || stat(mnt->mnt_dir, &rt) || rt.st_dev != st->st_dev));
		endmntent(mp);
		if (mnt && mnt->mnt_type && (!tab || !(s = strdup(mnt->mnt_type))))
			return(strncpy(typ, mnt->mnt_type, sizeof(typ)));
	}
	if (tab) hashput(tab, NiL, s);
	return(s);
}

#endif