/*
 * File:	stdiox.h $Revision: 1.3 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/* stdiox.h - extended stdio-like functions */


#ifndef __STDIOX_H_
#define __STDIOX_H_

#include <stdio.h>
#include <codelibs/boolean.h>

#ifdef __cplusplus
#include <stddef.h>
#include <stdarg.h>
extern "C" {
extern const char *msprintf(const char *fmt, ...);
extern char *mvsprintf(char *&buf, size_t &buflen, const char *fmt, va_list ap);
}
#else /* C */

extern char *msprintf();
extern char *mvsprintf();

#endif /* C */


#endif /* __STDIOX_H_ */
