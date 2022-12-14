//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * common.cc
 *
 * Some utility routines common to all inspect-and-repair tools (but
 * not part of any class.)
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include "dbck.h"
     
static int
do_a_directory(_Tt_string path, int (*fn)(_Tt_string))
{
	_Tt_string suffix("TT_DB/");
	
	if (path.right(1)!="/") {
		path = path.cat("/");
	}
	
	if (path.right(suffix.len())!=suffix) {
		path = path.cat(suffix);
	}
	
	return (*fn)(path);
}

int
do_directories(const _Tt_string_list_ptr &dirs, int (*fn)(_Tt_string))
{
	int failcount = 0;
	_Tt_string_list_cursor c(dirs);

	if (dirs->is_empty()) {
		failcount = !do_a_directory(".",fn);
	} else {
		while(c.next()) {
			failcount += !do_a_directory(*c,fn);
		}
	}
	return failcount;
}

		
