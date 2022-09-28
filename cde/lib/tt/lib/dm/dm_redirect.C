//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * Tool Talk Database Manager (DM) - dm_redirect.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Definition of class _Tt_redirect which implements a <host1 -> host2:path>
 * redirection entry.
 *
 */

#include <dm/dm_redirect.h>

_Tt_string
_tt_redirect_host1(_Tt_object_ptr &o)
{
	return(((_Tt_redirect *)o.c_pointer())->host1());
}


_Tt_redirect::
_Tt_redirect()
{
}

_Tt_redirect::
_Tt_redirect(const char *h1, const char *h2, const char *p2)
{
	_host1 = h1;
	_host2 = h2;
	_path2 = p2;
}

_Tt_redirect::
~_Tt_redirect()
{
}

implement_list_of(_Tt_redirect)

implement_table_of(_Tt_redirect)

