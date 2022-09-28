
/*
 *	$XConsortium: resource_fileP.h /main/cde1_maint/1 1995/07/14 22:23:05 drk $
 *
 *	@(#)resource_fileP.h	1.6 19 Oct 1994	cde_app_builder/src/abmf
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
 * resource_fileP.h - functions dealing with Xt Resource files
 */
#ifndef _ABMF_RESOURCE_FILEP_H_
#define _ABMF_RESOURCE_FILEP_H_

#include "abmfP.h"

File	abmfP_res_file_open(
			STRING	resFilePath,	/* file to open */
			STRING	resFileName,	/* name to appear in header */
			ABObj	obj,
			BOOL	isIntermediateFile
			);

int	abmfP_res_file_close(
			File	resFile
			);

int	abmfP_write_app_res_file(
			File	resFile,
			ABObj	project,
			STRING	resFileName
			);

int	abmfP_res_file_merge(
			File	vanillaFile,
			File	modifiedResFileName,
			File	*mergedFileOut
			);

STRING	abmfP_get_intermediate_res_file_name(
			ABObj	obj,
			char	*fileNameOut,
			int	nameMax
		);

STRING	abmfP_get_app_res_file_name(
			ABObj	obj,
			char	*fileNameOut,
			int	nameMax
		);


/*************************************************************************
 **									**
 **	Inline implementation						**
 **									**
 *************************************************************************/

#define abmfP_res_file_close(file) (abmfP_res_file_close_impl(&(file)))
int abmfP_res_file_close_impl(File*);

#endif /* _ABMF_RESOURCE_FILEP_H_ */

