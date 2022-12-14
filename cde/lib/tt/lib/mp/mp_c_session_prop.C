//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * mp_c_session_prop.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#include <mp/mp_global.h>
#include <mp/mp_mp.h>
#include <mp/mp_rpc_client.h>
#include <mp/mp_rpc_interface.h>
#include <mp/mp_c_session.h>
#include <mp/mp_xdr_functions.h>


/* 
 * Set the session property to the value
 */
Tt_status _Tt_c_session::
c_setprop(_Tt_string prop, _Tt_string val)
{
	Tt_status		status;
	Tt_status		rstatus;
	_Tt_prop_args		args;
		
	if (prop.len() == 0) {
		return TT_ERR_INVALID;
	}
	args.prop = prop;
	args.value = val;
	args.num = 0;
		
	rstatus = call(TT_RPC_SET_PROP,
		       (xdrproc_t)tt_xdr_prop_args, (char *)&args,
		       (xdrproc_t)xdr_int, (char *)&status);
	return((rstatus == TT_OK) ? status : rstatus);
}


/* 
 * Add the value to the session property
 */
Tt_status _Tt_c_session::
c_addprop(_Tt_string prop, _Tt_string val)
{
	Tt_status		status;
	Tt_status		rstatus;
	_Tt_prop_args		args;
		
	if (prop.len() == 0) {
		return TT_ERR_INVALID;
	}
	args.prop = prop;
	args.value = val;
	args.num = 0;
	rstatus = call(TT_RPC_ADD_PROP,
		       (xdrproc_t)tt_xdr_prop_args, (char *)&args,
		       (xdrproc_t)xdr_int, (char *)&status);
	return((rstatus == TT_OK) ? status : rstatus);
}


/* 
 * Get the ith value of the session property
 */
Tt_status _Tt_c_session::
c_getprop(_Tt_string prop, int i, _Tt_string &value)
{
	_Tt_prop_args		args;
	_Tt_rpc_result		result;
	Tt_status		rstatus;


	if (prop.len() == 0) {
		return TT_ERR_INVALID;
	}
	if (i < 0) {
		return TT_ERR_INVALID;
	}
	args.prop = prop;
	args.value = "";
	args.num = i;
	rstatus = call(TT_RPC_GET_PROP,
		       (xdrproc_t)tt_xdr_prop_args, (char *)&args,
		       (xdrproc_t)tt_xdr_rpc_result, (char *)&result);
	if (rstatus != TT_OK) {
		return(rstatus);
	}
	if (result.status == TT_OK) {
		value = result.str_val;
		return(TT_OK);
	} else {
		return result.status;
	}
}


/* 
 * Return the number of values on the session property
 */
Tt_status _Tt_c_session::
c_propcount(_Tt_string prop, int &cnt)
{
	_Tt_prop_args		args;
	_Tt_rpc_result		result;
	Tt_status		rstatus;
		
	if (prop.len() == 0) {
		return TT_ERR_INVALID;
	}
	args.prop = prop;
	args.value = "";
	args.num = 0;
	rstatus = call(TT_RPC_PROP_COUNT,
		       (xdrproc_t)tt_xdr_prop_args, (char *)&args,
		       (xdrproc_t)tt_xdr_rpc_result, (char *)&result);
	if (rstatus != TT_OK) {
		return(rstatus);
	}
	if (result.status != TT_OK) {
		return result.status;
	}
	cnt = result.int_val;
	return TT_OK;
}


/* 
 * Return the ith session property name
 */
Tt_status _Tt_c_session::
c_propname(int i, _Tt_string &prop)
{
	_Tt_prop_args		args;
	_Tt_rpc_result		result;
	Tt_status		rstatus;
		
	if (i < 0) {
		return TT_ERR_INVALID;
	}
	args.prop = "";
	args.value = "";
	args.num = i;
	rstatus =  call(TT_RPC_PROP_NAME,
			(xdrproc_t)tt_xdr_prop_args, (char *)&args,
			(xdrproc_t)tt_xdr_rpc_result, (char *)&result);
	if (rstatus != TT_OK) {
		return(rstatus);
	}
	prop = result.str_val;
	return result.status;
}


/* 
 * Return the number of session properties
 */
Tt_status _Tt_c_session::
c_propnames_count(int &cnt)
{
	_Tt_prop_args		args;
	_Tt_rpc_result		result;
	Tt_status		rstatus;
	
	args.prop = "";
	args.value = "";
	args.num = 0;
	rstatus = call(TT_RPC_PROP_NAMES_COUNT,
		       (xdrproc_t)tt_xdr_prop_args, (char *)&args,
		       (xdrproc_t)tt_xdr_rpc_result,(char *)&result);
	if (rstatus != TT_OK) {
		return(rstatus);
	}
	if (result.status != TT_OK) {
		return result.status;
	} else {
		cnt = result.int_val;
		return TT_OK;
	}
}





