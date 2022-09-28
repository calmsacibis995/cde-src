/* $XConsortium: shtable.h /main/cde1_maint/3 1995/10/14 00:09:40 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */
                  
#ifndef _SHTABLE_H
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


/*
 * David Korn
 * AT&T Bell Laboratories
 *
 * Interface definitions read-only data tables for shell
 *
 */

#define _SHTABLE_H	1

typedef struct shtable1
{
	const char	*sh_name;
	unsigned	sh_number;
} Shtable_t;

struct shtable2
{
	const char	*sh_name;
	unsigned	sh_number;
	const char	*sh_value;
};

struct shtable3
{
	const char	*sh_name;
	unsigned	sh_number;
	int		(*sh_value) __PROTO__((int, char*[], __V_*));
};

#define sh_lookup(name,value)	sh_locate(name,(Shtable_t*)(value),sizeof(*(value)))
extern __MANGLE__ const Shtable_t		shtab_testops[];
extern __MANGLE__ const Shtable_t		shtab_options[];
extern __MANGLE__ const Shtable_t		shtab_attributes[];
extern __MANGLE__ const Shtable_t		shtab_limits[];
extern __MANGLE__ const struct shtable2	shtab_variables[];
extern __MANGLE__ const struct shtable2	shtab_aliases[];
extern __MANGLE__ const struct shtable2	shtab_signals[];
extern __MANGLE__ const struct shtable3	shtab_builtins[];
extern __MANGLE__ const Shtable_t		shtab_reserved[];
extern __MANGLE__ const Shtable_t		shtab_config[];
extern __MANGLE__ int	sh_locate __PROTO__((const char*, const Shtable_t*, int));

#endif /* SH_TABLE_H */