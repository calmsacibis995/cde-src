/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 *
 * @(#)mp_xdr_functions.h	1.17 93/07/30
 *
 * Tool Talk Message Passer (MP) - mp_xdr_functions.h
 *
 * Copyright (c) 1990,1992 by Sun Microsystems, Inc.
 *
 * This file implements any XDR functions that are needed and not
 * directly related to any object (in which case they would be
 * implemented as methods)
 */
#ifndef MP_XDR_FUNCTIONS_H
#define MP_XDR_FUNCTIONS_H

#include <stdlib.h>
#include <rpc/rpc.h>
#include <api/c/tt_c.h>
#include <util/tt_string.h>
#include <mp/mp_file_utils.h>
#include <mp/mp_msg_context_utils.h>
#include <mp/mp_message_utils.h>
#include <mp/mp_procid_utils.h>
#include <mp/mp_pattern_utils.h>
#include <mp/mp_file_utils.h>

/* 
 * Used by the rpc call to set an fd channel on a procid.
 */
struct _Tt_fd_args: public _Tt_allocated {
	_Tt_procid_ptr	procid;
	int		fd;
	_Tt_string	start_token;
};

struct _Tt_add_pattern_args: public _Tt_allocated {
	_Tt_procid_ptr	procid;
	_Tt_pattern_ptr	pattern;
};

struct _Tt_del_pattern_args: public _Tt_allocated {
	_Tt_procid_ptr	procid;
	_Tt_string	pattern_id;
};

struct _Tt_declare_ptype_args: public _Tt_allocated {
	_Tt_procid_ptr	procid;
	_Tt_string	ptid;
};

struct _Tt_update_args: public _Tt_allocated {
	_Tt_message_ptr	message;
	Tt_state	newstate;
};

struct _Tt_prop_args: public _Tt_allocated {
	_Tt_string	prop;
	_Tt_string	value;
	int		num;
};

struct _Tt_otype_args: public _Tt_allocated {
	_Tt_string	base_otid;	
	_Tt_string	derived_otid;
	int		num;
	int		num2;
};

struct _Tt_rpc_result: public _Tt_allocated {
	Tt_status	status;
	_Tt_string	str_val;
	int		int_val;
};

struct _Tt_file_join_args: public _Tt_allocated {
	_Tt_procid_ptr	procid;
	_Tt_string	path;
};

struct _Tt_context_join_args: public _Tt_allocated {
	_Tt_procid_ptr		procid;
	_Tt_msg_context_ptr	context;
};

struct _Tt_next_message_args: public _Tt_allocated {
	_Tt_message_list_ptr	msgs;
	int			clear_signal;
};

struct _Tt_dispatch_reply_args: public _Tt_allocated {
	Tt_status		status;
	_Tt_qmsg_info_ptr	qmsg_info;
};


struct _Tt_load_types_args: public _Tt_allocated {
	_Tt_string	xdrtypes;
};

struct _Tt_gss_info_args: public _Tt_allocated {
	_Tt_string	mechanism;
	_Tt_string	principal;
	_Tt_string	qop;
};

bool_t		tt_xdr_bstring(XDR *xdrs, char **bstrp);
bool_t		tt_xdr_fd_args(XDR *xdrs, _Tt_fd_args *args);
bool_t		tt_xdr_declare_ptype_args(XDR *xdrs,
					  _Tt_declare_ptype_args
					  *args);
bool_t		tt_xdr_file_join_args(XDR *xdrs, _Tt_file_join_args *args);
bool_t		tt_xdr_context_join_args(XDR *xdrs, _Tt_context_join_args *args);
bool_t		tt_xdr_prop_args(XDR *xdrs, _Tt_prop_args *args);
bool_t		tt_xdr_add_pattern_args(XDR *xdrs,
					_Tt_add_pattern_args *args);
bool_t		tt_xdr_del_pattern_args(XDR *xdrs,
					_Tt_del_pattern_args *args);
bool_t		tt_xdr_otype_args(XDR *xdrs, _Tt_otype_args *args);
bool_t		tt_xdr_rpc_result(XDR *xdrs, _Tt_rpc_result *args);
bool_t		tt_xdr_update_args(XDR *xdrs, _Tt_update_args *args);
bool_t		tt_xdr_next_message_args(XDR *xdrs,
					 _Tt_next_message_args *args);
bool_t		tt_xdr_dispatch_reply_args(XDR *xdrs,
					   _Tt_dispatch_reply_args *args);
bool_t		tt_xdr_load_types_args(XDR *xdrs,
				       _Tt_load_types_args *args);
bool_t		tt_xdr_gss_info(XDR *xdrs, _Tt_gss_info_args *args);
#endif				/*  MP_XDR_FUNCTIONS_H */


