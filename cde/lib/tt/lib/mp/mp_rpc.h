/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * @(#)mp_rpc.h	1.15 93/09/07
 *
 * Copyright (c) 1990,1993 by Sun Microsystems, Inc.
 *
 * Common declarations for RPC implementation classes.
 */
#ifndef _TT_MP_RPC
#define _TT_MP_RPC
#include "tt_options.h"
#include <rpc/rpc.h>

#if defined(ultrix) || defined(_AIX)
extern "C" {
  void svcerr_auth (SVCXPRT *, enum auth_stat);
  SVCXPRT *svcfd_create(int,int,int);
  void svc_getreqset(fd_set *);
};
#endif

#if defined(OPT_TLI)
#	include <netconfig.h>
#	include <tiuser.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#endif /* OPT_TLI */

#include <sys/types.h>
#include "util/tt_object.h"
#include "util/tt_ptr.h"
#include "util/tt_int_rec.h"
#include "mp/mp_rpc_fns.h"
#include "mp/mp_rpc_auth.h"
#include "util/tt_host_utils.h"

enum _Tt_rpcsrv_err {
	_TT_RPCSRV_OK,
	_TT_RPCSRV_ERR,
	_TT_RPCSRV_TMOUT,
	_TT_RPCSRV_SIG,
	_TT_RPCSRV_FDERR
};
#endif				/* _TT_MP_RPC */
