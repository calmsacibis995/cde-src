//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/* -*-C++-*-
 *
 * @(#)mp_pat_context_utils.C	1.4 30 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_pat_context_utils.cc
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#include <mp/mp_pat_context.h>
#include <mp/mp_context_utils.h>
#include <mp/mp_pat_context_utils.h>

implement_list_of(_Tt_pat_context)

_Tt_pat_context_list & _Tt_pat_context_list::
append_ordered(const _Tt_pat_context_ptr &e)
{
	return (_Tt_pat_context_list &)this->_Tt_context_list::append_ordered(e);
}

