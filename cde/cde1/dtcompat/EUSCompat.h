/*
 *+SNOTICE
 *
 *	$XConsortium: EUSCompat.h /main/cde1_maint/6 1995/10/06 20:05:01 pascale $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)EUSCompat.h	1.17 30 Sep 1993"
#endif

#ifndef _EUSCOMPAT_H
#define _EUSCOMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SunOS)
#if (RELMAJOR==5)
#if (RELMINOR<2)
#define STRCASECMP_NOT_DEFINED
#endif /* RELMINOR */
#endif /* RELMAJOR */
#endif

/*
** System V R4 based systems define the stuff we need in
** sys/types.h. Include that and then we are done.
*/
#if defined(SunOS) || defined(UNIX_SV) || defined(USL) || defined(__uxp__)
#include <sys/types.h>
#endif

#if defined(sun) && defined(_XOPEN_SOURCE)
#ifndef B_TRUE
#define B_TRUE _B_TRUE
#endif
#ifndef B_FALSE
#define B_FALSE _B_FALSE
#endif
#endif   /* sun && _XOPEN_SOURCE */

/*
** HPUX defines most of what we need, if we set the right
** include options before including the system files.
*/
#if defined(HPUX)

#ifndef _INCLUDE_POSIX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#endif

#ifndef _INCLUDE_XOPEN_SOURCE
#define _INCLUDE_XOPEN_SOURCE
#endif

#ifndef _INCLUDE_AES_SOURCE
#define _INCLUDE_AES_SOURCE
#endif

#ifndef _INCLUDE_HPUX_SOURCE
#define _INCLUDE_HPUX_SOURCE
#endif

#include <sys/types.h>

#ifndef hpV4
typedef unsigned long ulong_t;
#endif /* hpV4 */
typedef unsigned char uchar_t;
typedef enum {B_FALSE, B_TRUE} boolean_t;

#define _SC_PAGESIZE	_SC_PAGE_SIZE

#ifndef MAXPATHLEN
#include <sys/param.h>
#endif

#define MAXNAMELEN      256

#endif	/* HPUX */

/*
** AIX, like HPUX defines most of what we need.
*/
#if defined(AIX)

#ifndef KERNEL
#define KERNEL
#endif

#ifndef _BSD_INCLUDES
#define _BSD_INCLUDES
#endif

#include <sys/types.h>

#define _SC_PAGESIZE	_SC_PAGE_SIZE
#define vfork		fork

#ifndef MAXPATHLEN
#include <sys/param.h>
#endif

#define MAXNAMELEN      256

typedef enum {B_FALSE, B_TRUE} boolean_t;

#undef BIG_ENDIAN

#endif /* AIX */

#if defined(USL) || defined(__uxp__)

#include <sys/param.h>

#define STRCASECMP_NOT_DEFINED
#if !defined(S_ISLNK)
#define S_ISLNK(mode)     (((mode) & S_IFMT) == S_IFLNK)
#endif
#endif

/*
 * A bug in Solaris 2.1 and the GNU compilers, these are not defined.
 */
#ifdef STRCASECMP_NOT_DEFINED
  extern int strcasecmp(const char *, const char *);
  extern int strncasecmp(const char *, const char *, size_t);
#endif

 /*
 ** 
 */
#if defined(__osf__)

#define vfork		fork

#include <sys/types.h>

#ifndef MAXPATHLEN
#include <sys/param.h>
#endif

#ifndef MAXNAMELEN
#define MAXNAMELEN      256
#endif

typedef enum {B_FALSE, B_TRUE} boolean_t;

#undef BIG_ENDIAN

#endif /* __osf__ */

#ifdef __cplusplus
}
#endif


#endif
