/* $XConsortium: hashlib.h /main/cde1_maint/3 1995/10/14 00:38:07 montyb $ */
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
 * hash table library private definitions
 */

#ifndef _HASHLIB_H
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

#define _HASHLIB_H

#include <ast.h>

typedef __V_*		(*HASHalloc) __PROTO__((size_t));
typedef int		(*HASHcompare) __PROTO__((const char*, const char*, ...));
typedef unsigned int	(*HASHhash) __PROTO__((const char*, ...));
typedef void		(*HASHfree) __PROTO__((__V_*));
typedef __V_*		(*HASHregion) __PROTO__((__V_*, __V_*, size_t, int));

typedef struct				/* last lookup cache		*/
{
	struct Hash_table*  table;	/* last lookup table		*/
	struct Hash_bucket* bucket;	/* last lookup bucket		*/
	const char*	name;		/* last lookup name		*/
	unsigned int	hash;		/* last lookup hash		*/
} Hash_last_t;

typedef struct				/* root local pointers		*/
{
	HASHhash	hash;		/* name hash routine		*/
	HASHcompare	compare;	/* name comparision routine	*/
	HASHalloc	alloc;		/* value allocation routine	*/
	HASHfree	free;		/* value free routine		*/
	HASHregion	region;		/* region alloc/free routine	*/
	__V_*		handle;		/* region handle arg		*/
} Hash_local_t;

#define _HASH_INFO_PRIVATE_ \
	Hash_root_t*	list;		/* root table list		*/

#define _HASH_POSITION_PRIVATE_ \
	Hash_table_t*	tab;		/* table pointer		*/ \
	Hash_table_t*	top;		/* top scope table pointer	*/ \
	int		flags;		/* scan flags			*/ \
	Hash_bucket_t**	slot;		/* table slot			*/ \
	Hash_bucket_t**	limit;		/* slot limit			*/

#define _HASH_ROOT_PRIVATE_ \
	int		namesize;	/* fixed name size: 0 => string	*/ \
	int		meanchain;	/* resize mean chain length	*/ \
	Hash_last_t	last;		/* last lookup info		*/ \
	Hash_local_t*	local;		/* root local pointers		*/ \
	Hash_root_t*	next;		/* next in list	of all roots	*/ \
	Hash_table_t*	references;	/* referencing table list	*/

#define _HASH_TABLE_PRIVATE_ \
	unsigned char	frozen;		/* table freeze nesting		*/ \
	unsigned char	bucketsize;	/* min bucket size in char*'s	*/ \
	Hash_bucket_t**	table;		/* hash slot table		*/ \
	Hash_table_t*	next;		/* root reference list link	*/

#include <hash.h>

#define HASHMINSIZE	(1<<4)		/* min table slots (power of 2)	*/
#define HASHMEANCHAIN	2		/* def resize mean chain len	*/

#define HASHMOD(t,h)	(h &= (t->size - 1))
#define HASHVAL(x)	((x)&~HASH_FLAGS)

#define HASH(r,n,h)	if (r->local->hash) h = r->namesize ? (*r->local->hash)(n, r->namesize) : (*r->local->hash)(n);\
			else\
			{\
				register const char*	_hash_s1 = n;\
				h = 0;\
				if (r->namesize)\
				{\
					register const char*	_hash_s2 = _hash_s1 + r->namesize;\
					while (_hash_s1 < _hash_s2) HASHPART(h, *_hash_s1++);\
				}\
				else while (*_hash_s1) HASHPART(h, *_hash_s1++);\
			}

#endif
