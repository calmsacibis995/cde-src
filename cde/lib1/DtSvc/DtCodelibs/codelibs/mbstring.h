/*
 * File:	mbstring.h $XConsortium: mbstring.h /main/cde1_maint/2 1995/10/04 11:44:59 gtsang $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef	__MBSTRING_H_
#define	__MBSTRING_H_

#if defined(USL) || defined(__uxp__)
#include <stdlib.h>
#endif

#include <stddef.h>

#ifdef apollo
#define _NEED_WCHAR_T
#include <sys/stdtypes.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if defined __cplusplus || defined __STDC__

#ifdef __cplusplus2_1
    extern char *_mb_schr(char *str, wchar_t ch);
    extern char *_mb_srchr(char *str, wchar_t ch);
#else /* __cplusplus2_1 */
    extern char *_mb_schr(const char *str, wchar_t ch);
    extern char *_mb_srchr(const char *str, wchar_t ch);
#endif /* __cplusplus2_1 */

#else /* defined __cplusplus || defined __STDC__ */

    extern char *_mb_schr();
    extern char *_mb_srchr();

#endif /* defined __cplusplus || defined __STDC__ */

#ifdef __cplusplus
}

#ifdef __cplusplus2_1
inline const char *_mb_schr(const char *str, wchar_t ch)
{
    return (const char *)_mb_schr((char *)str, ch);
}

inline const char *_mb_srchr(const char *str, wchar_t ch)
{
    return (const char *)_mb_srchr((char *)str, ch);
}
#endif /* __cplusplus2_1 */
#endif /* __cplusplus */

#endif /* __MBSTRING_H_ */
