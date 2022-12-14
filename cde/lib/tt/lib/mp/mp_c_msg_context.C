//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * @(#)mp_c_msg_context.C	1.4 30 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_c_msg_context.cc
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * _Tt_c_msg_context knows the client side of the context RPC interface.
 */

#include <mp/mp_arg.h>
#include <mp/mp_c_msg_context.h>
#include <mp/mp_rpc_interface.h>
#include <mp/mp_xdr_functions.h>

_Tt_c_msg_context::_Tt_c_msg_context()
{
}

_Tt_c_msg_context::~_Tt_c_msg_context()
{
}

Tt_status
_Tt_c_msg_context::c_join(
	_Tt_session &session,
	_Tt_procid_ptr &procID
)
{
	Tt_status		status;
	Tt_status		rstatus;
	_Tt_context_join_args	args;

	args.procid = procID;
	args.context = this;
	rstatus = session.call( TT_RPC_JOIN_CONTEXT,
			        (xdrproc_t)tt_xdr_context_join_args,
			        (char *)&args,
			        (xdrproc_t)xdr_int,
			        (char *)&status );
	return (rstatus == TT_OK) ? status : rstatus;
}

Tt_status
_Tt_c_msg_context::c_quit(
	_Tt_session &session,
	_Tt_procid_ptr &procID
)
{
	Tt_status		status;
	Tt_status		rstatus;
	_Tt_context_join_args	args;

	args.procid = procID;
	args.context = this;
	rstatus = session.call( TT_RPC_QUIT_CONTEXT,
			        (xdrproc_t)tt_xdr_context_join_args,
			        (char *)&args,
			        (xdrproc_t)xdr_int,
			        (char *)&status );
	return (rstatus == TT_OK) ? status : rstatus;
}
