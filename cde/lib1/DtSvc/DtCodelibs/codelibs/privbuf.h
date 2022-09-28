/*
 * File:	privbuf.h $XConsortium: privbuf.h /main/cde1_maint/2 1995/10/08 22:11:01 pascale $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#ifndef __PRIVBUF_H_
#define __PRIVBUF_H_

#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif
#ifdef apollo
#include <stdlib.h>
#endif

typedef void (*privbuf_func)(void *v);

#ifdef __cplusplus

#ifdef __PRIVATE_
// this is only for internal library routines needing dynamic buffers:
#include <stddef.h>
#include <stdlib.h>

#ifndef __osf__
// The DEC C++ compiler rejects valid inline declarations, claiming
// they have both internal and external linkage.
inline static void *operator new(size_t size) { return malloc((unsigned)size); }
inline static void operator delete(void *p) { if (p) free((char*)p); }
#endif

#include <codelibs/dynarray.h>
declare_array(privbuf_charbuf, char, 128)
declare_array(privbuf_strvec, char*, 128)
struct privbuf_buffer
{
    privbuf_func func;
    privbuf_charbuf buf;
    privbuf_strvec vec;
};
extern void privbuf_freeprivbuf(void *buf);
extern privbuf_buffer *privbuf_allocprivbuf();
#undef __PRIVATE_
#endif /* __PRIVATE_ */

extern "C" {
    void privbuf_release(void **var);
}
#else
extern void privbuf_release();
#endif

#endif /* __PRIVBUF_H_ */
