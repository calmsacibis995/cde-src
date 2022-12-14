/* $XConsortium: dirlib.h /main/cde1_maint/3 1995/10/14 00:32:23 montyb $ */
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
                  
/*
 * AT&T Bell Laboratories
 *
 * directory stream access library private definitions
 * library routines should include this file rather than <dirent.h>
 */

#ifndef _DIRLIB_H
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

#define _DIRLIB_H

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getdents
#else
#define getdents	______getdents
#endif

#include <ast.h>
#include <errno.h>

#if _lib_opendir && ( _hdr_dirent || _hdr_ndir || _sys_dir )

#define _dir_ok		1

#include <ls.h>
#ifndef _DIRENT_H
#if _hdr_dirent
#include <dirent.h>
#else
#if _hdr_ndir
#include <ndir.h>
#else
#include <sys/dir.h>
#endif
#ifndef dirent
#define dirent	direct
#endif
#endif
#endif

#else

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide DIR closedir dirent opendir readdir seekdir telldir
#else
#define DIR		______DIR
#define closedir	______closedir
#define dirent		______dirent
#define opendir		______opendir
#define readdir		______readdir
#define seekdir		______seekdir
#define telldir		______telldir
#endif

#include <ast_param.h>

#include <ls.h>
#include <limits.h>
#include <sys/dir.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide DIR closedir dirent opendir readdir seekdir telldir
#else
#undef	DIR
#undef	closedir
#undef	dirent
#undef	opendir
#undef	readdir
#undef	seekdir
#undef	telldir
#endif

#define _DIR_PRIVATE_ \
	int		dd_loc;		/* offset in block		*/ \
	int		dd_size;	/* valid data in block		*/ \
	char*		dd_buf;		/* directory block		*/

#include "dirstd.h"

#ifndef	DIRBLKSIZ
#ifdef	DIRBLK
#define DIRBLKSIZ	DIRBLK
#else
#ifdef	DIRBUF
#define DIRBLKSIZ	DIRBUF
#else
#define DIRBLKSIZ	8192
#endif
#endif
#endif

#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getdents
#else
#undef	getdents
#endif

#ifndef errno
extern __MANGLE__ int	errno;
#endif

extern __MANGLE__ ssize_t		getdents __PROTO__((int, __V_*, size_t));

#endif
