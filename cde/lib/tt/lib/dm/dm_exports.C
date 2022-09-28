//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/* @(#)dm_exports.C	1.9 93/09/07
 * Tool Talk Database Manager (DM) 
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Implementation of class _Tt_exports which implements a linked element
 * containing the export list (/etc/xtab) of a remote host.
 *
 */

#include <dm/dm_exports.h>

_Tt_exports::
_Tt_exports(const char *ahost, struct exports *exl)
{
	_host = ahost;
	_exps = exl;
}

_Tt_exports::
~_Tt_exports()
{
	free((MALLOCTYPE *)_exps);
}

_Tt_string& _Tt_exports::
host()
{
	return _host;
}

void _Tt_exports::
print(FILE *fs) const
{
	fprintf(fs, "Exports list for host %s -\n", (char *)_host);
	struct exports *exl = _exps;
	while (exl) {
		fprintf(fs, "  ex_name = %s\n", exl->ex_name);
		exl = exl->ex_next;
	}
}


