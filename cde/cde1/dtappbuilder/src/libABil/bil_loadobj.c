
/*
 *	$XConsortium: bil_loadobj.c /main/cde1_maint/3 1995/10/16 14:50:10 rcs $
 *
 * @(#)bil_loadobj.c	1.23 02 Feb 1995
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
 *  bil_loadobj.c - Load object from BIL file
 *
 */

#include <stdio.h>
#include "bilP.h"
#include "loadP.h"

STRING
bilP_load_end_of_obj(BIL_TOKEN objClass)
{
    switch (objClass)
    {
	case AB_BIL_PROJECT:
	    obj_set_file(bilP_load.project, istr_string(bilP_load.fileName));

	    /* I'm not sure why this check is here, but I'm afraid to 
	     * delete it. -jjd 
	     */
	    if (   (bilP_load.module != NULL)
		&& (obj_is_module(bilP_load.module)) )
	    {
		obj_tree_set_flag(bilP_load.module, BeingDestroyedFlag);
	    }

	    bilP_load.module = NULL;	/* may have been project */
	    bilP_load.obj = NULL;	/* was project */
	    bilP_load.objType = AB_TYPE_UNDEF;
	break;

	case AB_BIL_MODULE:
	    obj_set_file(bilP_load.module, istr_string(bilP_load.fileName));
/*
	    bilP_load.module= NULL;
*/
	    bilP_load.obj = NULL;
	    bilP_load.objType = AB_TYPE_UNDEF;
	break;

	default:
	{
	    if (bilP_load.obj->parent != NULL)
	    {
		/*
		 * parent is valid, object is complete.
		 */
		bilP_load.obj = NULL;
		bilP_load.objType = AB_TYPE_UNDEF;
	    }
	    else
	    {
		/*
		 * Who is its parent?
		 */
	        ABObj	parent= bilP_load.module;
	        if (parent == NULL)
	        {
	            parent= bilP_load.project;
	        }
	        if (parent != NULL)
	        {
		    obj_append_child(bilP_load.module, bilP_load.obj);
	            bilP_load.obj = NULL;
		    bilP_load.objType = AB_TYPE_UNDEF;
	        }
	    }
	}
	break;
    }

    /*
     * If bilP_load.obj is found to be complete, it is moved to
     * its proper place, and the pointer to it is set to NULL.
     *
     * We set destroy flags above, in case obj is child of something
     * (we won't try to delete it twice!)
     */
    if (bilP_load.obj != NULL)
    {
	obj_destroy(bilP_load.obj); bilP_load.obj = NULL;
	bilP_load.objType = AB_TYPE_UNDEF;
    }
    if (   (bilP_load.module != NULL)
	&& obj_has_flag(bilP_load.module, BeingDestroyedFlag) )
    {
	obj_destroy(bilP_load.module); bilP_load.module= NULL;
    }
    bilP_load.att= AB_BIL_UNDEF;

    /*
     * Clear load msg
     */
    abil_loadmsg_set_object(NULL);
    abil_loadmsg_set_att(NULL);

    return NULL;
}

