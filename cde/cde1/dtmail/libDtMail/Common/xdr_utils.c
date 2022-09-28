/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.1 $
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
#pragma ident "@(#)xdr_utils.c	1.5 02/17/94"
#endif

#ifdef NEED_XDR_SIZEOF

#include <EUSCompat.h>
#include "xdr_utils.h"
#include <stdlib.h>
#include <rpc/rpc.h>

static bool_t
local_putlong(XDR * xp, long * extra)
{
    xp->x_handy += 4;
    return TRUE;
}

static bool_t
local_putbytes(XDR *xp, void * extra, u_int len)
{
    xp->x_handy += RNDUP(len);
    return TRUE;
}

static long *
local_inline(XDR *xp, int len)
{
    if (len > 0 && len < (int) xp->x_base) {
	xp->x_handy += RNDUP(len);
	return((long *)xp->x_private);
    } else{
	return(0);
    }
}

static XDR *
local_create_stream()
{
    XDR * x = (XDR *)calloc(1, sizeof(XDR));
    struct xdr_ops * ops = (struct xdr_ops *)calloc(1, sizeof(struct xdr_ops));

    ops->x_putlong = local_putlong;
    ops->x_putbytes = local_putbytes;
    ops->x_inline = local_inline;
    x->x_op = XDR_ENCODE;
    x->x_ops = ops;
    x->x_handy = 0;
    x->x_private = malloc(sizeof(long));
    x->x_base = x->x_private;

    return(x);
}

static void
local_free_stream(XDR * x)
{
    free(x->x_ops);
    free(x->x_base);
    free(x);
}

unsigned long
xdr_sizeof(xdrproc_t xdr_func, void * data)
{
    XDR *sizeof_stream = local_create_stream();

    bool_t result = (*xdr_func)(sizeof_stream, data);
    unsigned long size = sizeof_stream->x_handy;

    local_free_stream(sizeof_stream);
    if (result == TRUE) {
	return(size);
    }
    else {
	return(0);
    }
}

#endif /* NEED_XDR_SIZEOF */
