//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * @(#)mp_s_pat_context.C	1.6 29 Jul 1993
 *
 * Tool Talk Pattern Passer (MP) - mp_s_pat_context.cc
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * _Tt_s_pat_context knows server-side context matching.
 */

#include <mp/mp_arg.h>
#include <mp/mp_message.h>
#include "mp_s_pat_context.h"

_Tt_s_pat_context::
_Tt_s_pat_context()
{
}

_Tt_s_pat_context::
_Tt_s_pat_context(const _Tt_context &c) : _Tt_pat_context(c)
{
}

_Tt_s_pat_context::
~_Tt_s_pat_context()
{
}

int _Tt_s_pat_context::
matchVal(
	const _Tt_message &msg
)
{
	_Tt_msg_context_ptr msgCntxt = msg.context( slotName() );
	if (msgCntxt.is_null()) {
		return 0;
	}
	if (_values->count() == 0) {
		// No values means we are a wild card.
		return 1;
	}
	_Tt_arg_list_cursor argC( _values );
	while (argC.next()) {
		if (**argC == msgCntxt->value()) {
			return 2;
		}
	}
	return 0;
}
