
/*
 * $Revision: 1.1 $
 * 
 * @(#)gil_loadobj.c	1.14 04/17/96      cde_app_builder/src/libABil
 * 
 * RESTRICTED CONFIDENTIAL INFORMATION:
 * 
 * The information in this document is subject to special restrictions in a
 * confidential disclosure agreement between HP, IBM, Sun, USL, SCO and
 * Univel.  Do not distribute this document outside HP, IBM, Sun, USL, SCO,
 * or Univel without Sun's specific written approval.  This document and all
 * copies and derivative works thereof must be returned or destroyed at Sun's
 * request.
 * 
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 * 
 */

#pragma ident "@(#)gil_loadobj.c	1.14 96/04/17 SMI"

/*
 * loadobj.c - load one GIL object
 */

#include <ab_private/util.h>
#include <ab_private/abio.h>
#include "gilP.h"
#include "gil_loadattP.h"
#include "load.h"
#include "loadP.h"


/*
 * Loads one gil object into a parent.  The object is created by this
 * routine.
 */
int
gilP_load_object(FILE * inFile, ABObj parent)
{
    ABObj obj = obj_create(AB_TYPE_UNDEF, parent);
    abil_loadmsg_set_object(NULL);
    abil_loadmsg_set_att(NULL);
    abil_loadmsg_set_action_att(NULL);
    if (obj == NULL)
    {
        abil_print_load_err(ERR_NO_MEMORY);
    }
    return gilP_load_object2(inFile, obj, parent);
}


/*
 * Loads one gil object, using the object passed in to start with.  This way,
 * arbitrary attributes may be set on an object before the load process
 * begins.
 * 
 * This is necessary because some incredible bonehead neglegted to put :type and
 * :name keys into the parent object.  Since the normal load won't set the
 * type properly, we have to do it up front.
 */
int
gilP_load_object2(FILE * inFile, ABObj obj, ABObj parent)
{
    int                 return_value = 0;
    int                 rc = 0; /* r turn code */
    AB_GIL_ATTRIBUTE    attr;
    ISTRING             keyword;
    BOOL                object_read = FALSE;

    while (!object_read)
    {
        if (abio_get_eof(inFile))
        {
            abil_print_load_err(ERR_EOF);
            goto epilogue;
        }
        if (abio_gil_get_object_end(inFile))
        {
            /*** The whole object has been read! ***/
            object_read = TRUE;
            if (obj->type == AB_TYPE_LAYERS)
            {
                if (obj_get_num_children(obj) == 0)
                {
                    obj_destroy(obj);
                    obj = NULL;
                    goto epilogue;
                }
            }
            if (obj != NULL)
            {

                /*
                 * The root node may have itself as it's parent
                 */
                if ((!obj_has_parent(obj))
                    && (obj != parent))
                {
                    obj_append_child(parent, obj);
                }
            }
        }
        else
        {
            /*** More attributes to go... ***/
	    abil_loadmsg_set_err_printed(FALSE);
            if (!abio_get_keyword(inFile, &keyword))
            {
                abil_print_load_err(ERR_WANT_KEYWORD);
                goto epilogue;
            }
            abil_loadmsg_set_att(istr_string(keyword));

            attr = gilP_string_to_att(istr_string(keyword));
            if ((rc = gilP_load_attribute_value(
                                        inFile, obj, attr, parent)) != NULL)
            {
                return_value = rc;
                goto epilogue;
            }
        }
    }                           /* while TRUE */

epilogue:
    return return_value;
}
