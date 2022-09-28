/*
 * File:	pathutils.h $XConsortium: pathutils.h /main/cde1_maint/1 1995/07/14 20:05:47 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef __PATHUTILS_H_
#define __PATHUTILS_H_

#include <stddef.h>
#include <codelibs/boolean.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
    char *pathdir(const char *src, char *dst = NULL);
    char *pathcollapse(const char *src, char *dst = NULL,
	    boolean show_dir = FALSE);
    char *pathbase(char *src);
#else
#ifdef __STDC__
    char *pathdir(const char *src, char *dst);
    char *pathcollapse(const char *src, char *dst, boolean show_dir);
    char *pathbase(const char *src);
#endif
#endif
#if defined(__cplusplus) || defined(__STDC__)

#else /* C */
extern char *pathbase();
extern char *pathdir();
extern char *pathcollapse();

#endif /* C */
#ifdef __cplusplus
}

inline const char *pathbase(const char *src)
{
    return (const char *)pathbase((char *)src);
}
#endif

#endif /* __PATHUTILS_H_ */
