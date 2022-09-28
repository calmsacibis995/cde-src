/* $XConsortium: ansi_c.h /main/cde1_maint/1 1995/07/17 19:46:43 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _ANSI_C_H
#define _ANSI_C_H

#pragma ident "@(#)ansi_c.h	1.6 96/11/12 Sun Microsystems, Inc."

#ifndef __STDC__
#error "Must have ANSI-C compiler!"
#endif
/*
**  For all function declarations, Must have an  ANSI compiler.
*/
#ifdef SunOS

extern int madvise(caddr_t, size_t, int);
#ifndef MADV_NORMAL
#define MADV_NORMAL     0               /* no further special treatment */
#define MADV_RANDOM     1               /* expect random page references */
#define MADV_SEQUENTIAL 2               /* expect sequential page references */
#define MADV_WILLNEED   3               /* will need these pages */
#define MADV_DONTNEED   4               /* don't need these pages */
#endif

#endif

#endif
