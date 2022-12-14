//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * @(#)mp_msg_context.C	1.14 94/11/17
 *
 * @(#)mp_msg_context.C	1.9 93/09/07
 *
 * Implementation of the _Tt_msg_context: a context slot in a message.
 */

#include "mp/mp_arg.h"
#include "mp/mp_msg_context.h"
#include "util/tt_iostream.h"

_Tt_msg_context::_Tt_msg_context()
{
	_value = new _Tt_arg;
}

_Tt_msg_context::~_Tt_msg_context()
{
}

Tt_status
_Tt_msg_context::setValue(
	const _Tt_string &value
)
{
	_value->set_mode( TT_INOUT );
	_value->set_type( "string" );
	return _value->set_data_string( value );
}

Tt_status
_Tt_msg_context::setValue(
	int value
)
{
	_value->set_mode( TT_INOUT );
	_value->set_type( "integer" );
	return _value->set_data_int( value );
}

int
_Tt_msg_context::isEnvEntry() const
{
	if (_slotName.len() == 0) {
		return 0;
	}
	return _slotName[0] == '$';
}

_Tt_string
_Tt_msg_context::enVarName() const
{
	_Tt_string name( ((char *)_slotName)+1 );
	return name;
}

_Tt_string
_Tt_msg_context::stringRep() const
{
	_Tt_string rep;
	char temp[ 20 ];
	switch (_value->data_type()) {
	    case _Tt_arg::UNSET:
		break;
	    case _Tt_arg::STRING:
		rep = _value->data_string();
		break;
	    case _Tt_arg::INT:
		sprintf( temp, "%d", _value->data_int());
		rep = temp;
		break;
	}
	return rep;
}

void
_Tt_msg_context::print(
	const _Tt_ostream &os
) const
{
	_Tt_context::print( os );
	os << ": ";
	if (!_value.is_null()) {
		_value->print( os );
	} else {
		os << "\n";
	}
}

void
_Tt_msg_context::print_(
	const _Tt_ostream &os,
	const _Tt_object *obj
)
{
	((_Tt_msg_context *)obj)->print( os );
}

bool_t
_Tt_msg_context::xdr(
	XDR *xdrs
)
{
	if (! _Tt_context::xdr( xdrs )) {
		return 0;
	}
	return _value.xdr( xdrs );
}
