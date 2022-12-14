//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_xdr_utils.C /main/cde1_maint/5 1995/10/07 19:10:39 pascale $ 			 				
/*
 *
 * tt_xdr_utils.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#if defined(ultrix)
#include <rpc/types.h>
#define bool_t int
#endif
#include <rpc/rpc.h>
#include <util/tt_xdr_utils.h>
#include <memory.h>
#include "tt_options.h"

typedef bool_t (*local_xdrproc_t)(XDR *, caddr_t *);

static bool_t
tt_x_putlong(XDR *xp, long *)
{
    xp->x_handy += 4;
    return TRUE;
}

static bool_t
/* The second argument is caddr_t and not void * in both SUN, DEC headers
 * files.
 * The third agrument is also wrong. Both SUN and DEC system header files
 * expect int instead of u_int.
 */
tt_x_putbytes(XDR *xp, caddr_t, int len)
{
    xp->x_handy += RNDUP (len);
    return TRUE;
}

#if defined(ultrix) || defined(__osf__)
static int*
#else
static long *
#endif
tt_x_inline(XDR *xp, int len)
{
	/* Be paranoid -- some code really expects inline to
	 * always succeed, so we keep a small buffer around
	 * just in case.  Not too paranoid, though -- it's
	 * legal to not support inline!
	 */
    /* It is better to promote len to caddr_t than demote x_base to
       int for 64 bit arch.
    */
    if (len > 0 && (caddr_t) len < xp->x_base) {
	xp->x_handy += RNDUP (len);
#if defined(ultrix) || defined(__osf__)
	return (int *) xp->x_private;
#else
	return (long *) xp->x_private;
#endif
    } else
	return 0;
}


unsigned long
_tt_xdr_sizeof(xdrproc_t f, void *data)
{
	_Tt_xdr_size_stream x;

	if ((*(local_xdrproc_t)f) ((XDR *)x, (caddr_t *)data) == TRUE) {
		return x.getsize();
	} else {
		return 0;
	}
}

_Tt_xdr_size_stream::
_Tt_xdr_size_stream() {
	memset ((char *)&ops, 0, sizeof ops);
#if defined(OPT_BUG_SUNOS_4) || defined(OPT_BUG_HPUX)
	ops.x_putlong = (int (*)(...))tt_x_putlong;
	ops.x_putbytes = (int (*)(...))tt_x_putbytes;
	ops.x_inline = (long *(*)(...))tt_x_inline;
#elif defined(OPT_BUG_AIX)
	ops.x_putlong = (int (*)(XDR *, long *))tt_x_putlong;
	ops.x_putbytes = (int (*)(XDR *, caddr_t, u_int))tt_x_putbytes;
	ops.x_inline = (long *(*)(XDR *, u_int))tt_x_inline;
#elif defined(OPT_BUG_SUNOS_5)
	ops.x_putlong = tt_x_putlong;
	ops.x_putbytes = (bool_t (*)(XDR *, caddr_t, int))tt_x_putbytes;
	ops.x_inline = tt_x_inline;
#elif defined(CRAY)
	ops.x_putlong = tt_x_putlong;
	ops.x_putbytes = tt_x_putbytes;
	ops.x_inline = (inline_t *(*)(...))tt_x_inline;
#elif defined(OPT_BUG_USL) || defined(OPT_BUG_UXP)
	ops.x_putlong = tt_x_putlong;
	ops.x_putbytes = (bool_t (*)(XDR *, caddr_t, u_int))  tt_x_putbytes;
#if defined(OPT_BUG_UW_1)
	ops.x_inline = (long *(*)(struct __XDR *, const int)) tt_x_inline;
#else
	ops.x_inline = (long *(*)(struct XDR *, int)) tt_x_inline;
#endif /* OPT_BUG_UW1 */
#else
	ops.x_putlong = tt_x_putlong;
	ops.x_putbytes = tt_x_putbytes;
	ops.x_inline = tt_x_inline;
#endif	
	xdrstream.x_op = XDR_ENCODE;
	xdrstream.x_ops = &ops;
	xdrstream.x_handy = 0;
	xdrstream.x_private = (caddr_t) buf;
	xdrstream.x_base = (caddr_t) sizeof buf;
}

_Tt_xdr_size_stream::
operator XDR *() {
	return(&xdrstream);
}
