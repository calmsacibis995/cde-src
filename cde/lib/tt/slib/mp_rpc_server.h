/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * @(#)mp_rpc_server.h	1.8 93/09/07
 *
 * Copyright (c) 1990,1993 by Sun Microsystems, Inc.
 *
 * Implementation of _Tt_rpc_server which represents a provider of an
 * RPC service and _Tt_rpc_client which represents the corresponding
 * RPC client.
 */
#ifndef _TT_MP_RPC_SERVER_H
#define _TT_MP_RPC_SERVER_H

#include "mp/mp_rpc.h"

class _Tt_rpc_server : public _Tt_object {
      public:
	_Tt_rpc_server() {};
	_Tt_rpc_server(int program,
		       int version,
		       int Rsocket,
		       _Tt_rpc_auth_ptr auth);
	virtual ~_Tt_rpc_server();
	int			init(void (*service_fn)(svc_req *, SVCXPRT *));
	_Tt_rpcsrv_err		run_until(int *stop, int sec_timeout,
				    _Tt_int_rec_list_ptr &efds);
	int			program() { return _program; };
	int			version() { return _version; };
      private:
	_Tt_rpc_auth_ptr        _auth;
	int			_program;
	int			_version;
	int			_socket;
	int			_rpc_fd;
	SVCXPRT			*_transp;
};

#endif				/* _TT_MP_RPC_SERVER_H */
