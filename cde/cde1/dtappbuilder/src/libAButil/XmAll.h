
/*
 *	$XConsortium: XmAll.h /main/cde1_maint/2 1995/10/16 09:54:02 rcs $
 *
 * @(#)XmAll.h	1.5 01 Feb 1994	cde_app_builder/src/libAButil
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

#ifndef _AB_XMALL_H_
#define _AB_XMALL_H_
/*
 * XmAll.h - include standard header files, adding symbols for AB
 * 		files.
 */

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE 1		/* we want to be POSIX-compliant */
#endif

#include <Xm/XmAll.h>

/*
 * Make sure we don't redefine types defined in standard X include files
 */
#define _AB_BOOL_DEFINED_
#define _AB_BYTE_DEFINED_

#endif /* _AB_XMALL_H_ */
