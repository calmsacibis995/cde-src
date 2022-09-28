
/*
 *	$XConsortium: connectP.h /main/cde1_maint/2 1995/10/16 11:29:58 rcs $
 *
 * @(#)connectP.h	3.14 07 Mar 1994	cde_app_builder/src/abmf
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * connect.h - abmf connections
 */
#ifndef _ABMF_CONNECTION_H_
#define _ABMF_CONNECTION_H_

#include	<ab_private/obj.h>
#include	"utilsP.h"

int	abmfP_set_auto_func_names(ABObj root);
int	abmfP_tree_set_action_names(ABObj tree);
int	abmfP_get_connect_includes(StringList includeFiles, ABObj projOrModule);

#endif /* _ABMF_CONNECTION_H_ */
