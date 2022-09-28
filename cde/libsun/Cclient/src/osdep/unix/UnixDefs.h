/*
 * @(#)UnixDefs.h	1.5 97/05/19
 */
#ifndef _UNIXDEFS_H__
#define _UNIXDEFS_H__

#ifndef _POSIX_C_SOURCE
#ifdef SunOS

#if ((SunOS >= 54)&&(SunOS < 55) || ((SunOS >= 540) && (SunOS < 550)) )
#define SUNOS 24
#endif

#if ((SunOS >= 55)&&(SunOS < 56) || ((SunOS >= 550) && (SunOS < 560)) )
#define SUNOS 25
#endif

#if ((SunOS >= 56)&&(SunOS < 500)) || (SunOS >= 560)
#define SUNOS 26
#endif

#if (SUNOS == 24)
/* From the broken /usr/include/sys/mman.h header file */
extern int madvise(char *, unsigned int, int);
#define MADV_NORMAL     0               /* no further special treatment */
#define MADV_RANDOM     1               /* expect random page references */
#define MADV_SEQUENTIAL 2               /* expect sequential page references */
#define MADV_WILLNEED   3               /* will need these pages */
#define MADV_DONTNEED   4               /* don't need these pages */
#endif

#if (SUNOS == 24) || (SUNOS == 25)
extern int gethostname(char *name, int len);
#endif

#if (SUNOS == 24)
#define _POSIX_C_SOURCE		199309L
#else/*Must be SunOS >= 5.5 */
#define _POSIX_C_SOURCE		199506L
#endif

#if (DO_LARGE_FS && (SUNOS >= 26))
#ifndef _LARGE_FILE64_SOURCE
#define _LARGEFILE64_SOURCE	1
#endif /*_LARGEFILE64_SOURCE*/
#ifndef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS	64
#endif /*_FILE_OFFSET_BITS */
#endif /* DO_LARGE_FS */

#endif /* SunOS */
#endif /* _POSIX_C_SOURCE */

#include <unistd.h>
#include <sys/types.h>
#endif /*_UNIXDEFS_H__*/
