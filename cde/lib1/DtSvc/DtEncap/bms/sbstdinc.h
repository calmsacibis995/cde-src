/*
 * File:         sbstdinc.h $XConsortium: sbstdinc.h /main/cde1_maint/2 1995/10/08 22:15:45 pascale $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _sbstdinc_h
#define _sbstdinc_h

#include <sys/types.h>
#include <sys/param.h>

#ifdef SVR4
#include <netdb.h>		/* MAXHOSTNAMELEN */
#endif /* SVR4 */

#ifndef howmany
#define howmany(x, y)  (((x)+((y)-1))/(y))   /* From <sys/param.h>, but not an XPG3 file */
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>

#ifdef sun
#include <strings.h>
#endif

/************************************************************************/
/* Routines not defined in include files (yet).				*/
/************************************************************************/

/* Domain/OS BSD has bzero(), bcmp(), and bcopy() defined. */
#if !defined(apollo) || !defined(__bsd)

#if defined(__STDC__)
#if !defined(__osf__) && !defined(sun)
extern void bcopy(char *b1, char *b2, int length);
extern int  bcmp(char *b1, char *b2, int length);
extern void bzero(char *b, int length);
#endif

extern char *mktemp(char *tmplate);
#elif ! defined(__cplusplus)
#if !defined(__osf__) && !defined(sun)
extern void bcopy();
extern int  bcmp();
extern void bzero();
#endif

extern char *mktemp();
#endif

#endif


#ifndef  SBSTDINC_H_NO_REDEFINE   /* sbstdinc.c turns this on */

/************************************************************************/
/* Routines from <string.h> 						*/
/*  --- These always get redefined so we can catch null ptr deref's     */
/************************************************************************/

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrcat(XeString s1, const XeChar * const s2);
#else
extern XeString Xestrcat();
#endif
#if defined(__aix)
#undef strcat
#endif
#define strcat Xestrcat

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrncat(const XeString s1, const XeChar * const s2, const size_t n);
#else
extern XeString Xestrncat();
#endif
#define strncat Xestrncat

#if defined(__STDC__) || defined(__cplusplus)
extern int Xestrcmp(const XeChar * const s1, const XeChar * const s2);
#else
extern int Xestrcmp();
#endif
#if defined(__aix)
#undef strcmp
#endif
#define strcmp Xestrcmp

#if defined(__STDC__) || defined(__cplusplus)
extern int Xestrncmp(ConstXeString s1, ConstXeString s2, size_t n);
#else
extern int Xestrncmp();
#endif
#define strncmp Xestrncmp

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrcpy(const XeString s1, const XeChar * const s2);
#else
extern XeString Xestrcpy();
#endif
#if defined(__aix)
#undef strcpy
#endif
#define strcpy Xestrcpy

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrncpy(const XeString s1, const XeChar * const s2, const size_t n);
#else
extern XeString Xestrncpy();
#endif
#define strncpy Xestrncpy

#if defined(__STDC__) || defined(__cplusplus)
extern int Xestrcoll(const XeString s1, const XeString s2);
#else
extern int Xestrcoll();
#endif
#define strcoll Xestrcoll

#if defined(__STDC__) || defined(__cplusplus)
extern size_t Xestrxfrm(XeString s1, const XeString s2, size_t n);
#else
extern size_t Xestrxfrm();
#endif
#define strxfrm Xestrxfrm

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrchr(const XeString s, int c);
#else
extern XeString Xestrchr();
#endif
#if defined(__aix)
#undef strchr
#endif
#define strchr Xestrchr

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrpbrk(const XeString s1, const XeString s2);
#else
extern XeString Xestrpbrk();
#endif
#define strpbrk Xestrpbrk

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrrchr(const XeString s, int c);
#else
extern XeString Xestrrchr();
#endif
#if defined(__aix)
#undef strrchr
#endif
#define strrchr Xestrrchr

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrstr(const XeString s1, const XeString s2);
#else
extern XeString Xestrstr();
#endif
#define strstr Xestrstr

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrtok(XeString s1, const XeString s2);
#else
extern XeString Xestrtok();
#endif
#define strtok Xestrtok

#if defined(__STDC__) || defined(__cplusplus)
extern size_t Xestrlen(const XeChar * const s);
#else
extern size_t Xestrlen();
#endif
#if defined(__aix)
#undef strlen
#endif

#ifndef __osf__
#define strlen Xestrlen
#endif

#if defined(__STDC__) || defined(__cplusplus)
extern XeString Xestrdup(XeString s);
#else
extern XeString Xestrdup();
#endif
#define strdup Xestrdup

#endif /* ifndef  SBSTDINC_H_NO_REDEFINE */

#endif /*  _sbstdinc_h */ 
