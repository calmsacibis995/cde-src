/*
 * $XConsortium: msg_cvt.c /main/cde1_maint/2 1995/10/16 14:26:51 rcs $
 * 
 * @(#)msg_cvt.c	1.1 15 Jul 1994	cde_app_builder/src/abmf
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


/*
 * msg_cvt.c - Handles [re]generation of message source file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ab_private/obj.h>
#include <ab_private/abio.h>
#include "write_codeP.h"
#include "msg_file.h"
#include "msg_cvt.h"

static MsgSet	get_cached_msg_set(
		    MsgFile	msg_file,
		    ABObj	obj
		);


extern BOOL
abmfP_initialize_msg_file(
    GenCodeInfo	genCodeInfo,
    ABObj	project
    )
{
    BOOL	ret_val = (BOOL)FALSE;

    char	msg_file_name[BUFSIZ];
    MsgFile	msg_file;

    if (project == NULL)
        goto cret;

    if (genCodeInfo->msg_file_obj != NULL)
    {
        ret_val = TRUE;
        goto cret;
    }
    
    /*
     * Make path name of message src file
     */
    sprintf(msg_file_name, "%s%s", obj_get_name(project), ".msg");

    if ((msg_file = MsgFile_create(obj_get_name(project), msg_file_name))
	!= NULL)
    {
	genCodeInfo->msg_src_file_name = strdup(msg_file_name);
	genCodeInfo->msg_file_obj = msg_file;
        ret_val = (BOOL)TRUE;
    }
    
cret:
    return(ret_val);
}


extern char *
abmfP_catgets_prefix_str(
    GenCodeInfo	genCodeInfo,
    ABObj	obj,
    char	*msg_string
    )
{
    static char		str_catg[BUFSIZ];
    char		*ret_val = msg_string;
    MsgSet		msg_set;
    int			msg_num;

    if ((msg_set = get_cached_msg_set(genCodeInfo->msg_file_obj, obj))== NULL)
        goto cret;

    if ((msg_num = MsgSet_sure_find_msg(msg_set, msg_string)) <= 0)
        goto cret;
    MsgSet_set_msg_is_referenced(msg_set, msg_num, TRUE);

    sprintf(str_catg, "catgets(Dtb_project_catd, %d, %d, ",
            MsgSet_get_number(msg_set),
            msg_num);
    
    ret_val = str_catg;
    
cret:    
    return(ret_val);
}


extern int
abmfP_write_catgets_call(
    GenCodeInfo	genCodeInfo,
    ABObj	obj,
    char	*msgString
)
{
    char	*prefix = NULL;
    File	fp = genCodeInfo->code_file;

    if (   (msgString == NULL)
	|| ((prefix = abmfP_catgets_prefix_str(genCodeInfo, obj, msgString)) 
								      == NULL)
       )
    {
        abio_printf(fp, "((char*)NULL)");
	return 0;
    }
    else
    {
        abio_puts(fp, prefix);
        abio_put_string(fp, msgString);
        abio_puts(fp, ")");
    }

    return 0;
}


static MsgSet
get_cached_msg_set(
    MsgFile	msg_file,
    ABObj	obj
)
{
    static ABObj	prev_obj = (ABObj)NULL;
    static MsgSet	prev_msg_set = (MsgSet)NULL;

    MsgSet		msg_set;

    if (obj == prev_obj)
        msg_set = prev_msg_set;
    else if (obj_is_project(obj))
    {
	char	*project_name = obj_get_name(obj);
	char	proj_suffixed_name[BUFSIZ];

	strcpy(proj_suffixed_name, project_name);
	strcat(proj_suffixed_name, "_project");

	msg_set = MsgFile_sure_find_msg_set(msg_file, proj_suffixed_name);
	prev_msg_set = msg_set;
    }
    else
    {
	static ABObj	prev_module = (ABObj)NULL;
	ABObj		module;
        char		*module_name;
        
        module = obj_get_module(obj);

        if (module == prev_module)
            msg_set = prev_msg_set;
        else
        {
            prev_module = module;

            module_name = obj_get_name(module);
            msg_set = MsgFile_sure_find_msg_set(msg_file, module_name);

            prev_msg_set = msg_set;
        }
    }
    return(msg_set);
}
