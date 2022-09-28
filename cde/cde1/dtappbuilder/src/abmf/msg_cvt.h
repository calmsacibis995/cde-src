/*
 *	$XConsortium: msg_cvt.h /main/cde1_maint/2 1995/10/16 11:31:41 rcs $
 *
 * @(#)msg_cvt.h	1.1 15 Jul 1994	cde_app_builder/src/abmf
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

#ifndef _ABMF_MSG_CVT_H_
#define _ABMF_MSG_CVT_H_

/*
 * msg_cvt.h - API for message [re]generation
 */

#include <ab/util_types.h>
#include "write_codeP.h"
#include "msg_file.h"

extern BOOL	abmfP_initialize_msg_file(
    GenCodeInfo	genCodeInfo,
    ABObj	project
    );

extern char *	abmfP_catgets_prefix_str(
    GenCodeInfo	genCodeInfo,
    ABObj	obj,
    char	*msg_string
    );

extern int	abmfP_write_catgets_call(
    GenCodeInfo	genCodeInfo,
    ABObj	obj,
    char	*msgString
    );

#endif /* !_ABMF_MSG_CVT_H_ */
