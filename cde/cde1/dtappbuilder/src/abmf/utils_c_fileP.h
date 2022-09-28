
/*
 *	$XConsortium: utils_c_fileP.h /main/cde1_maint/1 1995/07/14 22:25:55 drk $
 *
 *	@(#)utils_c_fileP.h	1.2 11 Jul 1994	cde_app_builder/src/abmf
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
 * utils_c_fileP.h - writes dtb_utils.c
 */
#ifndef _ABMF_UTILS_C_FILEP_H_
#define _ABMF_UTILS_C_FILEP_H_

#include "abmfP.h"
#include "write_codeP.h"


int abmfP_write_utils_c_file(
			GenCodeInfo	genCodeInfo,
			STRING		fileName,
			ABObj		project
			);


#endif /* _ABMF_UTILS_C_FILEP_H_ */

