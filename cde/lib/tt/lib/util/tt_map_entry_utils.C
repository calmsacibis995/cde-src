//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_map_entry_utils.C /main/cde1_maint/2 1995/10/07 19:09:24 pascale $ 			 				
/*
 * Tool Talk Utility - tt_map_entry_utils.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Defines the _Tt_map_entry utils.
 */

#include "util/tt_map_entry_utils.h"
#include "util/tt_map_entry.h"

implement_derived_ptr_to(_Tt_map_entry, _Tt_object)
implement_table_of(_Tt_map_entry)

_Tt_map_entry::
_Tt_map_entry ()
{
}

_Tt_map_entry::
~_Tt_map_entry ()
{
}
