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
#pragma ident "@(#)xdr_utils.h	1.5 02/17/94"
#endif

#ifndef _XDR_UTILS_H
#define _XDR_UTILS_H

#ifdef NEED_XDR_SIZEOF

#include <rpc/rpc.h>

#if defined(__cplusplus)
extern "C" {
#endif

unsigned long xdr_sizeof(xdrproc_t func, void * data);

#if defined(__cplusplus)
}
#endif

#endif

#endif
