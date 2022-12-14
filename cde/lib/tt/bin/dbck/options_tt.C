//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * options_tt.cc
 *
 * ttdbck option handling routines
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbck.h"     
#include "options_tt.h"
#include "util/tt_gettext.h"

Dbck_specoptions::
Dbck_specoptions()
{
	_repair_filename_p  = 0;
	_repair_filename = (char *)0;
}


/*
 * Return the getopt valid-options string
 */
char * Dbck_specoptions::
optstring()
{
	return "vhf:k:t:bximpaIF:T:Zd:";
}

int Dbck_specoptions::
set_option(int optchar, const char *optval)
{
	switch (optchar) {
	      case 'a':
		_disp_id_p = _disp_mand_p = _disp_prop_p = 1;
		break;
	      case 'F':
		_repair_filename_p = 1;
		_repair_filename = optval;
		break;
	      default:
		return set_common_option(optchar, optval);
	}
	return 1;
}


void Dbck_specoptions::
print(FILE *f) const
{
	Dbck_options::print(f);
	if (_repair_filename_p) {
		fprintf(f,catgets(_ttcatd, 6, 19,
				  "Repair by setting to file: %s\n"),
			(char *)_repair_filename);
	}	
	fprintf(f,">\n");
}
