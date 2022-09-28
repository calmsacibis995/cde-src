
/*
 *	$Revision: 1.1 $
 *
 *	@(#)gilcvt_utils.c	1.3 02/01/95	cde_app_builder/src/dtgilcvt
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
#pragma ident "@(#)gilcvt_utils.c	1.3 95/02/01 SMI"

/*
 *  gilcvt_utils.c - utils c file
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ab_private/trav.h>
#include "gilcvt_utilsP.h"

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

/*
 * Finds the primary window for the application.
 * Finds the base window whose name is alphabetically first
 */
ABObj
gilcvtP_get_root_window(ABObj project)
{
    static ABObj	root_window = NULL;
    AB_TRAVERSAL	trav;
    ABObj		window= NULL;

    if (root_window != NULL)
    {
	goto epilogue;
    }

    if (obj_get_root_window(project) != NULL)
    {
	root_window = obj_get_root_window(project);
	goto epilogue;
    }

    for (trav_open(&trav, project, AB_TRAV_WINDOWS);
	    (window= trav_next(&trav)) != NULL; )
    {
	if (root_window == NULL)
	{
	    root_window = window;
	}

	if (obj_is_base_win(window))
	{
	    /* base window */
	    if (obj_is_base_win(root_window))
	    {
	        if (obj_is_initially_visible(window))
	        {
		    if (obj_is_initially_visible(root_window))
		    {
			/* both are visible main window */
			if (util_strcmp(obj_get_name(window),
				obj_get_name(root_window)) < 0)
			{
			    root_window = window;
			}
			else
			{
			    /* no change */
			}
		    }
		    else
		    {
		        /* window is visible. root_window is not */
		        root_window = window;
		    }
		}
		else
		{
		    if (obj_is_initially_visible(root_window))
		    {
			/* window is not visible, root_window is */
		    }
		    else
		    {
			/* both are invisible main windows */
			if (util_strcmp(obj_get_name(window),
				obj_get_name(root_window)) < 0)
			{
			    root_window = window;
			}
			else
			{
			    /* no change */
			}
		    }
		}
	    }
	    else
	    {
		/* window is a main window, root_window is not */
		root_window = window;
	    }
        }
	else
	{
	    /* popup window */
	    if (obj_is_base_win(root_window))
	    {
		/* window is a popup, root_window is a base window */
		/* no change */
	    }
	    else
	    {
		if (obj_is_initially_visible(window))
		{
		    if (obj_is_initially_visible(root_window))
		    {
			/* both are visible popups */
			if (util_strcmp(obj_get_name(window),
				obj_get_name(root_window)) < 0)
			{
			    root_window = window;
			}
			else
			{
			    /* no change */
			}
		    }
		    else
		    {
			/* window is visible. root_window is not */
			root_window = window;
		    }
		}
		else
		{
		    if (obj_is_initially_visible(root_window))
		    {
			/* window is not visible. root_window is */
			/* no change */
		    }
		    else
		    {
			/* both are invisible popups */
			if (util_strcmp(obj_get_name(window),
				obj_get_name(root_window)) < 0)
			{
			    root_window = window;
			}
			else
			{
			    /* no change */
			}
		    }
		}
	    }
	}
    }
    trav_close(&trav);

epilogue:
    return root_window;
}

