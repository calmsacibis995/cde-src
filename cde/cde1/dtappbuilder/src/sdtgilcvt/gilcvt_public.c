
/*
 *	$Revision: 1.1 $
 *
 *	@(#)gilcvt_public.c	1.3 01/26/95	cde_app_builder/src/dtgilcvt
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
#pragma ident "@(#)gilcvt_public.c	1.3 95/01/26 SMI"

/*
 *  gilvct_public.c - public routines
 */

#include <sys/param.h>
#include <ab_private/obj.h>
#include <ab_private/trav.h>
#include <ab_private/bil.h>
#include "gilcvt.h"

/*************************************************************************
**                                                                      **
**       Constants (#define and const)					**
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Private Functions (C declarations and macros)			**
**                                                                      **
**************************************************************************/
static int	write_file(ABObj obj, STRING fileName);

/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

int	
gilcvt_write_bil_files(ABObj project)
{
    int			return_value = 0;
    int			rc = 0;			/* return code */
    ABObj		module = NULL;
    AB_TRAVERSAL	trav;
    char		fileName[MAXPATHLEN+1];
    *fileName = 0;

    if (obj_get_write_me(project))
    {
	sprintf(fileName, "%s.bip", obj_get_name(project));
	if ((rc = write_file(project, fileName)) < 0)
	{
	    return_value = rc;
	    goto epilogue;
	}
    }
    for (trav_open(&trav, project, AB_TRAV_MODULES);
	(module = trav_next(&trav)) != NULL; )
    {
	if (obj_get_write_me(module))
	{
	    sprintf(fileName, "%s.bil", obj_get_name(module));
	    if ((rc = write_file(module, fileName)) < 0)
	    {
		return_value = rc;
		break;
	    }
	}
    }
    trav_close(&trav);

epilogue:
    return return_value;
}


static int
write_file(ABObj obj, STRING fileName)
{
    int		rc = 0;

    if (!util_be_silent())
    {
	util_printf("writing %s\n", fileName);
    }

    rc = bil_save_tree(obj, fileName, NULL, BIL_SAVE_FILE_PER_MODULE);
    return rc;
}

