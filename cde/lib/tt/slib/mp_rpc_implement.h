/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * mp_rpc_implement.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#ifndef _MP_RPC_IMPLEMENT_H
#define _MP_RPC_IMPLEMENT_H

#include "mp/mp_rpc_interface.h"
#include "mp/mp_rpc_auth.h"
void _tt_service_rpc(svc_req *rqstp, SVCXPRT *transp);
#endif				/* _MP_RPC_IMPLEMENT_H */
