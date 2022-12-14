//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * @(#)mp_pat_context.C	1.9 93/09/07
 *
 * Tool Talk Pattern Passer (MP) - mp_pat_context.cc
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * Implementation of the _Tt_pat_context: a context slot in a pattern.
 */

#include "mp/mp_arg.h"
#include "mp/mp_msg_context.h"
#include "mp/mp_pat_context.h"
#include "util/tt_iostream.h"

_Tt_pat_context::
_Tt_pat_context()
{
	_values = new _Tt_arg_list;
}

_Tt_pat_context::
_Tt_pat_context(const _Tt_context &c) : _Tt_context(c)
{
	_values = new _Tt_arg_list;
}

_Tt_pat_context::
~_Tt_pat_context()
{
}

Tt_status _Tt_pat_context::
addValue(
	const _Tt_string &value
)
{
	_Tt_arg_ptr newVal = new _Tt_arg( TT_INOUT, "string" );
	if (newVal.is_null()) {
		return TT_ERR_NOMEM;
	}
	Tt_status status = newVal->set_data_string( value );
	if (status != TT_OK) {
		return status;
	}
	_values->append( newVal );
	return TT_OK;
}

Tt_status _Tt_pat_context::
addValue(
	int value
)
{
	_Tt_arg_ptr newVal = new _Tt_arg( TT_INOUT, "integer" );
	if (newVal.is_null()) {
		return TT_ERR_NOMEM;
	}
	Tt_status status = newVal->set_data_int( value );
	if (status != TT_OK) {
		return status;
	}
	_values->append( newVal );
	return TT_OK;
}

Tt_status _Tt_pat_context::
addValue(
	const _Tt_msg_context &msgCntxt
)
{
	_Tt_arg_ptr newVal = new _Tt_arg( msgCntxt.value() );
	if (newVal.is_null()) {
		return TT_ERR_NOMEM;
	}
	_values->append( newVal );
	return TT_OK;
}

Tt_status _Tt_pat_context::
deleteValue(
	const _Tt_msg_context &msgCntxt
)
{
	_Tt_arg_list_cursor argC( _values );
	while (argC.next()) {
		if (msgCntxt.value() == **argC) {
			argC.remove();
			return TT_OK;
		}
	}
	return TT_WRN_NOTFOUND;
}

void _Tt_pat_context::
print(
	const _Tt_ostream &os
) const
{
	_Tt_context::print( os );
	os << ":\n";
	if (! _values.is_null()) {
		_Tt_string indent = os.indent();
		os.set_indent( indent.cat( "\t" ));
		_Tt_arg_list_cursor argC( _values );
		while (argC.next()) {	
			argC->print( os );
		}
		os.set_indent( indent );
	}
}

void _Tt_pat_context::
print_(
	const _Tt_ostream &os,
	const _Tt_object *obj
)
{
	((_Tt_pat_context *)obj)->print( os );
}

bool_t _Tt_pat_context::
xdr(
	XDR *xdrs
)
{
	if (! _Tt_context::xdr( xdrs )) {
		return 0;
	}
	return _values.xdr( xdrs );
}
