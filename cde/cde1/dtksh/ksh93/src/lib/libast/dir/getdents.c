/* $XConsortium: getdents.c /main/cde1_maint/3 1995/10/14 00:33:01 montyb $ */
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
#include "dirlib.h"

#if _dir_ok || _lib_getdents

NoN(getdents)

#else

/*
 * getdents
 *
 * read directory entries into directory block
 *
 * NOTE: directory entries must fit within DIRBLKSIZ boundaries
 */

#ifndef MAXNAMLEN
#define MAXNAMLEN	255
#endif

#if _lib_dirread
extern __MANGLE__ int		dirread __PROTO__((int, char*, int));
#endif
#if _lib_getdirentries
extern __MANGLE__ int		getdirentries __PROTO__((int, char*, int, long*));
#endif

ssize_t
getdents __PARAM__((int fd, __V_* buf, size_t siz), (fd, buf, siz)) __OTORP__(int fd; __V_* buf; size_t siz;){
	struct stat		st;

	if (siz < DIRBLKSIZ)
	{
		errno = EINVAL;
		return(-1);
	}
	if (fstat(fd, &st)) return(-1);
	if (!S_ISDIR(st.st_mode))
	{
#ifdef ENOTDIR
		errno = ENOTDIR;
#else
		errno = EBADF;
#endif
		return(-1);
	}
#if _lib_getdirentries
	{
		long		off;
		return(getdirentries(fd, buf, siz, &off));
	}
#else
#if _lib_dirread
	{
		register char*		sp;	/* system */
		register struct dirent*	up;	/* user */
		char*			u;
		int			n;
		int			m;
		int			i;

		m = (siz * 6) / 10;
		m = roundof(m, 8);
		sp = (char*)buf + siz - m - 1;
		if (!(n = dirread(fd, sp, m))) return(0);
		if (n > 0)
		{
			up = (struct dirent*)buf;
			sp[n] = 0;
			while (sp < (char*)buf + siz - m + n)
			{
				i = 0;
				while (*sp >= '0' && *sp <= '9')
					i = 10 * i + *sp++ - '0';
				while (*sp && *sp != '\t') sp++;
				if (*sp++)
				{
					up->d_fileno = i;
					u = up->d_name;
					while ((*u = *sp++) && u < up->d_name + MAXNAMLEN) u++;
					*u = 0;
					up->d_reclen = sizeof(struct dirent) - sizeof(up->d_name) + (up->d_namlen = u - up->d_name) + 1;
					up->d_reclen = roundof(up->d_reclen, 8);
					up = (struct dirent*)((char*)up + up->d_reclen);
				}
			}
			return((char*)up - (char*)buf);
		}
	}
#else
#if _mem_d_reclen_direct
	return(read(fd, buf, siz));
#else
	{

#define MAXREC	roundof(sizeof(*up)-sizeof(up->d_name)+sizeof(sp->d_name)+1,8)

		register struct direct*	sp;	/* system */
		register struct dirent*	up;	/* user */
		register char*		s;
		register char*		u;
		int			n;
		int			m;
		char			tmp[sizeof(sp->d_name) + 1];

		/*
		 * we assume sizeof(struct dirent) > sizeof(struct direct)
		 */

		up = (struct dirent*)buf;
		n = (siz / MAXREC) * sizeof(struct direct);
		if ((!(m = n & ~511) || m < MAXREC) && (!(m = n & ~255) || m < MAXREC)) m = n;
		do
		{
			if ((n = read(fd, (char*)buf + siz - m, m)) <= 0) break;
			sp = (struct direct*)((char*)buf + siz - m);
			while (sp < (struct direct*)((char*)buf + siz - m + n))
			{
				if (sp->d_ino)
				{
					up->d_fileno = sp->d_ino;
					s = sp->d_name;
					u = tmp;
					while (s < sp->d_name + sizeof(sp->d_name) && *s)
						*u++ = *s++;
					*u = 0;
					strcpy(up->d_name, tmp);
					up->d_reclen = sizeof(struct dirent) - sizeof(up->d_name) + (up->d_namlen = u - tmp) + 1;
					up->d_reclen = roundof(up->d_reclen, 8);
					up = (struct dirent*)((char*)up + up->d_reclen);
				}
				sp++;
			}
		} while (up == (struct dirent*)buf);
		return((char*)up - (char*)buf);
	}
#endif
#endif
#endif
}

#endif
