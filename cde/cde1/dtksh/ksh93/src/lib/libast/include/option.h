/* $XConsortium: option.h /main/cde1_maint/3 1995/10/14 00:41:55 montyb $ */
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * command line option parse assist definitions
 */

#ifndef _OPTION_H
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

#define _OPTION_H

#if _DLL_INDIRECT_DATA && !_DLL
#define opt_info	(*_opt_info_)
#else
#define opt_info	_opt_info_
#endif

/*
 * obsolete interface mappings
 */

#define opt_again	opt_info.again
#define opt_arg		opt_info.arg
#define opt_argv	opt_info.argv
#define opt_char	opt_info.offset
#define opt_index	opt_info.index
#define opt_msg		opt_info.msg
#define opt_num		opt_info.num
#define opt_option	opt_info.option
#define opt_pindex	opt_info.pindex
#define opt_pchar	opt_info.poffset

typedef struct
{
	int		again;		/* see optjoin()		*/
	char*		arg;		/* {:,#} string argument	*/
	char**		argv;		/* most recent argv		*/
	int		index;		/* argv index			*/
	char*		msg;		/* error/usage message buffer	*/
	long		num;		/* # numeric argument		*/
	int		offset;		/* char offset in argv[index]	*/
	char		option[3];	/* current flag {-,+} + option  */
	int		pindex;		/* prev index for backup	*/
	int		poffset;	/* prev offset for backup	*/
#ifdef _OPT_INFO_PRIVATE
	_OPT_INFO_PRIVATE
#endif
} Opt_info_t;

extern __MANGLE__ Opt_info_t	opt_info;	/* global option state		*/

extern __MANGLE__ int		optget __PROTO__((char**, const char*));
extern __MANGLE__ int		optjoin __PROTO__((char**, ...));
extern __MANGLE__ char*		optusage __PROTO__((const char*));

#endif
