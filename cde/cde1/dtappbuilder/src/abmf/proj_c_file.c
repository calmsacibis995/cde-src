/*
 * $XConsortium: proj_c_file.c /main/cde1_maint/3 1995/10/16 14:28:08 rcs $
 * 
 * @(#)proj_c_file.c	1.14 16 Feb 1994	cde_app_builder/src/abmf
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
 * proj_c_file.c - write project.c file
 */

#include <ab_private/util.h>
#include <ab_private/abio.h>
#include <ab_private/objxm.h>
#include "write_codeP.h"
#include "cdefsP.h"
#include "instancesP.h"
#include "obj_namesP.h"
#include "utilsP.h"
#include "connectP.h"
#include "argsP.h"
#include "write_cP.h"
#include "stubs_c_fileP.h"
#include "lib_func_stringsP.h"
#include "proj_c_fileP.h"

/*************************************************************************
**                                                                      **
**       Constants (#define and const)					**
**                                                                      **
*************************************************************************/

/*************************************************************************
**                                                                      **
**       Private Functions (C declarations and macros)			**
**                                                                      **
**************************************************************************/

#define		app_needs_tt_init(project) \
		    (   ((project) != NULL) \
		     && (obj_get_tooltalk_level(project) != AB_TOOLTALK_NONE) \
		    )

#define		main_needs_app_context_var(project) \
		    (   obj_get_write_main_func(project) \
		     || (app_needs_tt_init(project)) )

#define		main_needs_display_var(project) \
		    (app_needs_tt_init(project))

		/* only need counter for parsing args - i.e., no main() */
#define		main_needs_counter_var(project) \
		    (   abmfP_proj_needs_session_restore(project) \
		     && (!obj_get_write_main_func(project)) \
		    )

#define		main_needs_get_resources_vars(project) \
		    (   obj_get_write_main_func(project) \
		     || abmfP_proj_needs_session_restore(project))

static int	write_includes(GenCodeInfo genCodeInfo, ABObj project);
static int      write_main_or_init_func(
			GenCodeInfo genCodeInfo, 
			ABObj project
		);
static int 	write_main_register_save_yourself(
			GenCodeInfo	genCodeInfo, 
			ABObj		project, 
			char		*atom_name
			);
static int write_main_begin(GenCodeInfo genCodeInfo, ABObj project);
static int write_main_session_restore(GenCodeInfo genCodeInfo, ABObj project);
static int write_main_i18n_local_vars(
			GenCodeInfo	genCodeInfo, 
			ABObj 		project
		);
static int write_main_xt_i18n(GenCodeInfo genCodeInfo, ABObj project);
static int write_main_msg_i18n(GenCodeInfo genCodeInfo, ABObj project);
static int 	write_main_local_vars(
			GenCodeInfo 	genCodeInfo, 
			ABObj		project,
			ABObj		mainWindow
		);
static int write_init_xt_vars(
			GenCodeInfo	genCodeInfo, 
			ABObj		project
		);
static int write_option_desc_list(GenCodeInfo genCodeInfo, ABObj project);
static int write_app_resources(GenCodeInfo genCodeInfo, ABObj project);
static int write_app_resource_var(GenCodeInfo genCodeInfo, ABObj project);
static int write_i18n_var_defs(GenCodeInfo genCodeInfo, ABObj project);
static int write_main_tooltalk_local_vars(GenCodeInfo genCodeInfo, ABObj project);
static int write_parse_command_line(GenCodeInfo genCodeInfo, ABObj project);
static int write_main_tooltalk_init(GenCodeInfo genCodeInfo, ABObj project);
static int	write_map_window(
			GenCodeInfo	genCodeInfo, 
			ABObj		window,
			BOOL		show
		);
static int	write_map_initially_visible_windows(
			GenCodeInfo	genCodeInfo, 
			ABObj		project
		);

/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

/*
 * Main file includes.
 */
static char        *Includes[] =
{
    "<unistd.h>",
    "<stdlib.h>",
    "<stdio.h>",
    "<sys/param.h>",
    "<sys/types.h>",
    "<sys/types.h>",
    "<sys/stat.h>",
    "<X11/Intrinsic.h>",
    "<Xm/Xm.h>",
    "<Xm/MwmUtil.h>",
    "<Xm/Protocols.h>",
    "<Dt/Help.h>",
    "<Dt/HelpQuickD.h>",
    "<Dt/HelpDialog.h>",
    NULL,
};

/*
 * Main file includes for sessioning.
 */
static char        *Session_Includes[] =
{
    "<unistd.h>",
    "<Dt/Session.h>",
    NULL,
};

/*
 * Main file includes for i18n
 */
static char        *I18n_Includes[] =
{
    "<nl_types.h>",
    NULL,
};


/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

/*
 * Write the project.c file.
 */
int
abmfP_write_project_c_file(
			   GenCodeInfo genCodeInfo,
			   STRING codeFileName,
			   BOOL prepareForMerge,
			   ABObj project
)
{
    File                codeFile = genCodeInfo->code_file;
    STRING              errmsg = NULL;
    STRING             *p = NULL;
    ABObj               win_obj = NULL;
    ABObj               obj = NULL;
    ABObj               action = NULL;
    AB_ACTION_INFO     *action_info = NULL;
    AB_TRAVERSAL        trav;
    ABObj		module = NULL;
    int			numFuncsWritten = 0;
    char		projectName[1024];
    char		commentBuf[1024];
    char		*comment = NULL;
    *projectName = 0;
    *commentBuf = 0;

    /*
     * Write file header.
     */
    abmfP_write_user_header_seg(genCodeInfo);
    abio_puts(codeFile, nlstr);

    if (obj_get_write_main_func(project))
    {
        comment = " * Contains: main() and cross-module connections";
    }
    else
    {
	sprintf(commentBuf, 
	    " * Contains: %s and cross-module connections",
	    	abmfP_get_project_init_proc_name(project));
	comment = commentBuf;
    }
    sprintf(projectName, "project %s", obj_get_name(project));
    abmfP_write_file_header(
		genCodeInfo, 
		codeFileName, 
		FALSE,
		projectName,
		util_get_program_name(), 
		ABMF_MODIFY_USER_SEGS,
		comment
		);

    /*
     * Write includes.
     */
    write_includes(genCodeInfo, project);

    /*
     * Write user segment
     */
    abmfP_write_user_file_top_seg(genCodeInfo);
   
    /*
     * i18n variable definitions. We need to write some of these,
     * even if we are not generating main, because they are public and
     * used by other files
     */
    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
        write_i18n_var_defs(genCodeInfo, project);
    }

    write_option_desc_list(genCodeInfo, project);
    write_app_resources(genCodeInfo, project);
    write_app_resource_var(genCodeInfo, project);

    /*
     * Write main().
     */
    write_main_or_init_func(genCodeInfo, project);

    /*
     * Write callback funcs.
     */
    numFuncsWritten = 0;
    for (trav_open(&trav, project, AB_TRAV_ACTIONS);
	 (action = trav_next(&trav)) != NULL;)
    {
	action_info = &(action->info.action);

	/* If the function name for the action is not NULL AND
	 * the action is a cross-module connection OR the action
	 * is a shared connection, then write it out.
	 */
	if ( 	(abmfP_get_action_name(action) != NULL) && 
		(   obj_is_cross_module(action) || 
		    ( (!mfobj_has_flags(action, CGenFlagIsDuplicateDef))
	              && (mfobj_has_flags(action, CGenFlagWriteDefToProjFile))
	              && (obj_get_func_type(action) == AB_FUNC_USER_DEF)
		    )
		)
	   )

	{
	    ++numFuncsWritten;
	    abmfP_write_action_function(genCodeInfo, action);
	}
    }
    trav_close(&trav);

    if (numFuncsWritten > 0)
    {
	abio_puts(codeFile, nlstr);
    }
    abmfP_write_user_file_bottom_seg(genCodeInfo);

    return OK;
}


static int
write_includes(GenCodeInfo genCodeInfo, ABObj project)
{
    File	 codeFile = genCodeInfo->code_file;
    STRING	 *p       = NULL;
    AB_TRAVERSAL trav;
    ABObj	 module   = NULL;
    StringList   includes = strlist_create();
    char         buf[MAXPATHLEN+1];
    int          i;
    
    strlist_set_is_unique(includes, TRUE);
    *buf = 0;

    /* standard system includes */
    for (p = Includes; *p; p++)
    {
	strlist_add_str(includes, *p, NULL);
    }

    /*
     * Includes for sessioning.
     * These include files are needed only if sessioning
     * is used.
     */
    if (abmfP_proj_needs_session_save(project) || 
	abmfP_proj_needs_session_restore(project))
    {
        for (p = Session_Includes; *p; p++)
        {
	    strlist_add_str(includes, *p, NULL);
        }
    }

    /*
     * Includes for i18n.
     * These include files are needed only if i18n
     * is enabled.
     */
    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
        for (p = I18n_Includes; *p; p++)
        {
	    strlist_add_str(includes, *p, NULL);
        }
    }

    /* module includes */
    for (trav_open(&trav, project, AB_TRAV_MODULES);
	(module = trav_next(&trav)) != NULL; )
    {
	if (!obj_is_defined(module))
	{
	    continue;
	}
	sprintf(buf, "\"%s\"", abmfP_get_ui_header_file_name(module));
	strlist_add_str(includes, buf, NULL);
    }
    trav_close(&trav);

    /* project include */
    sprintf(buf, "\"%s\"", abmfP_get_project_header_file_name(project));
    strlist_add_str(includes, buf, NULL);

    sprintf(buf, "\"%s\"", abmfP_get_utils_header_file_name(project));
    strlist_add_str(includes, buf, NULL);

    abmfP_get_connect_includes(includes, project);

    /* Write the includes */
    for (i = 0; i < strlist_get_num_strs(includes); ++i)
    {
	abio_printf(codeFile, "#include %s\n",
	    strlist_get_str(includes, i, NULL));
    }
    
    abio_puts(codeFile, nlstr);

    strlist_destroy(includes);
    return 0;
}



static int
write_option_desc_list(GenCodeInfo genCodeInfo, ABObj project)
{
    File	codeFile;

    if (   (genCodeInfo == NULL)
	|| (project == NULL)
	|| (!obj_get_write_main_func(project))	/* only for main() */
       )
	return (0);
    
    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile, "\n");
    abio_puts(codeFile, "/*\n");
    abio_puts(codeFile, " * command line options...\n");
    abio_puts(codeFile, " */\n");
    abio_puts(codeFile, "static XrmOptionDescRec optionDescList[] = {\n");
    abio_indent(codeFile);
    abio_puts(codeFile,     "{\"-session\", \"*session\", XrmoptionSepArg, (XPointer)NULL}\n");
    abio_puts(codeFile, "\n");
    abmfP_write_user_struct_fields_seg(genCodeInfo);
    abio_outdent(codeFile);
    abio_printf(codeFile, "};\n");

    return (0);
}

static int
write_app_resources(GenCodeInfo genCodeInfo, ABObj project)
{
    File	codeFile;

    if (   (genCodeInfo == NULL) 
	|| (project == NULL)
       )
	return (0);
    
    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile, "\n");
    abio_puts(codeFile, "/*\n");
    abio_puts(codeFile, " * Application Resources\n");
    abio_puts(codeFile, " */\n");
    abio_puts(codeFile, "static XtResource resources[] = {\n");
    abio_indent(codeFile);
    abio_puts(codeFile,     "{\"session\", \"Session\", XtRString, sizeof(String),\n");
    abio_indent(codeFile);
    abio_puts(codeFile,         "XtOffsetOf(DtbAppResourceRec, session_file), XtRImmediate, (XtPointer)NULL}\n");
    abio_outdent(codeFile);
    abio_puts(codeFile, "\n");
    
    abmfP_write_user_struct_fields_seg(genCodeInfo);
    abio_outdent(codeFile);
    abio_puts(codeFile, "};\n");

    return (0);
}


static int
write_app_resource_var(
    GenCodeInfo	genCodeInfo, 
    ABObj	project
)
{
    File	codeFile;

    if (!genCodeInfo || !project)
	return (0);

    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile,"\n");
    abio_puts(codeFile,"DtbAppResourceRec\tdtb_app_resource_rec;\n");

    return (0);
}

static int
write_i18n_var_defs(
    GenCodeInfo genCodeInfo,
    ABObj project
    )
{
    File	codeFile;
    int		ret_val = 0;
    

    if (!genCodeInfo || !project)
	goto cret;

    codeFile = genCodeInfo->code_file;

    /*
     * Write out the NL_CAT_LOCALE workaround
     */
    if (obj_get_write_main_func(project))
    {
        abio_puts(codeFile, "\n");
        abio_puts(codeFile, "/* Workaround for XPG4 API compatibility */\n");
        abio_puts(codeFile, "#if !defined(NL_CAT_LOCALE)\n");
        abio_puts(codeFile, "#define NL_CAT_LOCALE 0\n");
        abio_puts(codeFile, "#endif\n");
        abio_puts(codeFile, "\n");
    }

    /*
     * Always written - used by all modules
     */
    abio_puts(codeFile,
              "/* Handle for standard message catalog for the project */\n");
    
    abio_puts(codeFile, "nl_catd\tDtb_project_catd = (nl_catd)-1;\n");

cret:
    return(ret_val);
}


static int
write_map_window(
			GenCodeInfo	genCodeInfo, 
			ABObj 		window,
			BOOL		show
)
{
    File	codeFile = genCodeInfo->code_file;
    ABObj	project = obj_get_project(window);
    ABObj	proj_root_window = abmfP_get_root_window(project);
    ABObjRec	showActionRec;
    ABObj	showAction = &showActionRec;
    ABObj	winParent = NULL;
    char	winParentName[1024];
    obj_construct(showAction, AB_TYPE_ACTION, NULL);
    *winParentName = 0;

    obj_set_from(showAction, NULL);
    obj_set_to(showAction, window);
    obj_set_func_type(showAction, AB_FUNC_BUILTIN);
    obj_set_func_builtin(showAction, AB_STDACT_SHOW);

    winParent = obj_get_win_parent(window);
    if (winParent == NULL)
    {
	sprintf(winParentName, "%s()", abmfP_lib_get_toplevel_widget->name);
    }
    else
    {
        strcpy(winParentName, abmfP_get_c_name(genCodeInfo, winParent));
    }
    abio_printf(codeFile, "%s(%s, %s);\n",
	abmfP_get_init_proc_name(window),
	abmfP_get_c_struct_ptr_name(genCodeInfo, window),
	winParentName);

    if (show)
    	abmfP_write_builtin_action(genCodeInfo, showAction, FALSE);

    obj_destruct(showAction);
    return 0;
}


/*
 * Write main() for the given project, or project_initialize(), if we
 * are not generating main().
 */
static int
write_main_or_init_func(GenCodeInfo genCodeInfo, ABObj project)
{ /* line 503 */
    int                 returnValue = 0;
    BOOL		writingMain = obj_get_write_main_func(project);
    File                codeFile = genCodeInfo->code_file;
    ABObj               window = NULL;
    int                 initialized = FALSE;
    ABObj		main_window= NULL;
    AB_TRAVERSAL        trav;

    abmfP_gencode_enter_func(genCodeInfo);

    main_window = abmfP_get_root_window(project);
    if (main_window != NULL)
    {
	mfobj_set_flags(main_window, CGenFlagTreatAsAppShell);
    }

    write_main_begin(genCodeInfo, project);
    write_main_local_vars(genCodeInfo, project, main_window);

    abmfP_write_user_long_seg(genCodeInfo,
    " No initialization has been done.\n"
"     ***\n"
"     *** Add local variables and code."
    );

    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
        write_main_xt_i18n(genCodeInfo, project);
    }

    /*
     * Create a parent shell for every other shell and don't realize it. This
     * way, we can be consistent with our hierarchy.
     */
    write_init_xt_vars(genCodeInfo, project);

    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
        write_main_msg_i18n(genCodeInfo, project);
    
    /* Save the toplevel widget so it can be fetched later as needed */
    abmfP_write_c_comment(genCodeInfo, FALSE,
      "Save the toplevel widget so it can be fetched later as needed.");
    abio_puts(codeFile, "dtb_save_toplevel_widget(toplevel);\n");

    /* Save the command used to invoke the application */
    abio_puts(codeFile,"\n");
    abmfP_write_c_comment(genCodeInfo, FALSE,
      "Save the command used to invoke the application.");
    abio_puts(codeFile, "dtb_save_command(argv[0]);\n");

    if (main_needs_get_resources_vars(project)) {
	/*
     	 * Get application resources
     	 */
    	abio_puts(codeFile,"\n");
    	abio_puts(codeFile, "XtGetApplicationResources(toplevel, (XtPointer)&dtb_app_resource_rec,\n");
    	abio_indent(codeFile);
    	abio_puts(codeFile, "resources, XtNumber(resources),\n");
    	abio_puts(codeFile, "get_resources_args, num_get_resources_args);\n");
    	abio_outdent(codeFile);
    }
    write_parse_command_line(genCodeInfo, project);

    /*
     * User segment after initialization
     */
    abmfP_write_user_long_seg(genCodeInfo,
"     A connection to the X server has been established, and all\n"
"     *** initialization has been done.\n"
"     ***\n"
"     ***  Add extra initialization code after this comment."
    );

    /*
     * Call session restore callback (if needed)
     */
    write_main_session_restore(genCodeInfo, project);

    /*
     * Initialize all global variables
     */
    abmfP_write_c_comment(genCodeInfo, FALSE, 
	"Initialize all global variables.");
    for (trav_open(&trav, project, AB_TRAV_SALIENT_UI);
	(window= trav_next(&trav)) != NULL; )
    {
	if (!obj_is_defined(window))
	{
	    continue;
	}

	if (obj_is_message(window))
 	{
	    abio_printf(codeFile, "%s(&%s);\n",
		abmfP_get_msg_clear_proc_name(window),
		abmfP_get_c_struct_global_name(window));
	}
	else if (obj_is_window(window))
	{
	    abio_printf(codeFile, "%s(&%s);\n",
		abmfP_get_clear_proc_name(window),
		abmfP_get_c_struct_global_name(window));
	}
    }
    trav_close(&trav);
    abio_puts(codeFile, "\n");

    if (main_window != NULL)
    {
	abmfP_write_c_comment(genCodeInfo, FALSE,
		"Set up the application's root (primary) window.");
        abio_printf(codeFile, "%s = toplevel;\n",
	    abmfP_get_c_name_global(main_window));

	if (obj_get_icon(main_window) != NULL)
	{
	    abio_printf(codeFile, "%s(%s,\n", 
		abmfP_lib_cvt_image_file_to_pixmap->name,
	        abmfP_get_c_name_global(main_window));
	    abio_indent(codeFile);
	    abio_put_string(codeFile, obj_get_icon(main_window));
	    abio_printf(codeFile, ", &%s);\n",
		istr_string(abmfP_icon_pixmap_var(genCodeInfo)));
	    abio_outdent(codeFile);
	    abmfP_icon_pixmap_var_has_value(genCodeInfo) = TRUE;
	}
	if (obj_get_icon_mask(main_window) != NULL)
	{
	    abio_printf(codeFile, "%s(%s,\n", 
		abmfP_lib_cvt_image_file_to_pixmap->name,
	        abmfP_get_c_name_global(main_window));
	    abio_indent(codeFile);
	    abio_put_string(codeFile, obj_get_icon_mask(main_window));
	    abio_printf(codeFile, ", &%s);\n",
		istr_string(abmfP_icon_mask_pixmap_var(genCodeInfo)));
	    abio_outdent(codeFile);
	    abmfP_icon_mask_pixmap_var_has_value(genCodeInfo) = TRUE;
	}

	if (abmfP_get_num_args_of_classes(main_window, ABMF_ARGCLASS_ALL) > 0)
	{
	    abmfP_xt_va_list_open_setvalues(genCodeInfo, main_window);
            abmfP_obj_spew_args(genCodeInfo, 
		main_window, ABMF_ARGCLASS_ALL, ABMF_ARGFMT_VA_LIST);
    	    if (!obj_get_resizable(main_window))
	    {
        	abio_printf(codeFile,
            	    "XmNmwmDecorations, MWM_DECOR_ALL | MWM_DECOR_RESIZEH,\n");
        	abio_printf(codeFile, 
            	    "XmNmwmFunctions, MWM_FUNC_ALL | MWM_FUNC_RESIZE,\n"); 
	    }
	    abmfP_xt_va_list_close(genCodeInfo);
	}
    }
    abio_puts(codeFile, nlstr);
    if ((main_window != NULL) && (obj_is_initially_visible(main_window)))
    {
	/* ApplicationShell doesn't require "show" action for mapping */
        write_map_window(genCodeInfo, main_window, False);
    }

    write_map_initially_visible_windows(genCodeInfo, project);
    write_main_register_save_yourself(genCodeInfo, project, "save_yourself_atom");

    /*
     * User seg before realize
     */
    abmfP_write_user_long_seg(genCodeInfo,
"     All initially-mapped widgets have been created, but not\n"
"     *** realized. Set resources on widgets, or perform other operations\n"
"     *** that must be completed before the toplevel widget is\n"
"     *** realized."
    );

    /*
     * Realize the widget hierarchy
     */
    if (writingMain)
    {
        abio_puts(codeFile, "XtRealizeWidget(toplevel);\n\n");
    }

    /*
     * Write ToolTalk Initialization if needed
     */
    write_main_tooltalk_init(genCodeInfo, project);

    /*
     * User seg before event loop (or return, for init func)
     */
    abmfP_write_user_long_seg(genCodeInfo,
"     All automatic initialization is complete. Add any additional\n"
"     *** code that should be executed before the Xt main loop is entered."
    );

    if (writingMain)
    {
        abmfP_write_c_comment(genCodeInfo, FALSE, "Enter event loop");
        abio_puts(codeFile, "XtAppMainLoop(app);\n");
    }

    /*
     * Write func footer.
     */
    /* { vi hack */
    abio_printf(codeFile, "return 0;\n");
    if (!writingMain)
    {
	abio_puts(codeFile, "#undef argc\n");
	abio_puts(codeFile, "#undef argv\n");
    }
    abio_set_indent(codeFile, 0);
    abio_puts(codeFile, "}\n\n");

    abmfP_gencode_exit_func(genCodeInfo);
    return returnValue;
} /* line 743 */


static int
write_main_begin(GenCodeInfo genCodeInfo, ABObj project)
{
    File	codeFile = genCodeInfo->code_file;

    if (obj_get_write_main_func(project))
    {
	/* main() */
        abio_puts(codeFile, "\n\n");
        abio_puts(codeFile, abmfP_comment_begin);
        abio_puts(codeFile, abmfP_comment_continue);
        abio_printf(codeFile, "main for application %s\n",
		    obj_get_name(project));
        abio_puts(codeFile, abmfP_comment_end);
    }
    else
    {
	/* project init func */
        abio_puts(codeFile, "\n\n");
        abio_puts(codeFile, abmfP_comment_begin);
        abio_puts(codeFile, abmfP_comment_continue);
        abio_printf(codeFile, "initialization for application %s\n",
		    obj_get_name(project));
        abio_puts(codeFile, abmfP_comment_continue);
        abio_printf(codeFile, 
			"This should be called from user-defined main()\n");
        abio_puts(codeFile, abmfP_comment_end);
    }

    /*
     * Write func name
     */
    if (obj_get_write_main_func(project))
    {
	abmfP_write_c_func_begin(
		genCodeInfo,
		FALSE,
		abmfP_str_int,		"main",
		abmfP_str_int,		"argc",
		"char",			"**argv",
		NULL);
    }
    else
    {
	abmfP_write_c_func_begin(
		genCodeInfo,
		FALSE,
		abmfP_str_int,	abmfP_get_project_init_proc_name(project),
		abmfP_str_int,		"*argc_in_out",
		"char",			"**argv_in_out[]",
		abmfP_str_widget,	"toplevel",
		NULL);
	abio_puts(codeFile, "#define argc (*argc_in_out)\n");
	abio_puts(codeFile, "#define argv (*argv_in_out)\n");
    }
    abio_set_indent(codeFile, 1);
 
    return 0;
}


/*
 * mainWindow may be NULL;
 */
static int
write_main_local_vars(
			GenCodeInfo	genCodeInfo, 
			ABObj 		project,
			ABObj		mainWindow
)
{
    File	codeFile = genCodeInfo->code_file;
    BOOL	mainWindowHasIcon = FALSE;
    BOOL	writingMain = obj_get_write_main_func(project);

    if (writingMain)
    {
        abio_puts(codeFile, "Widget\t\ttoplevel = (Widget)NULL;\n");
    }
    if (main_needs_display_var(project))
    {
        abio_puts(codeFile, "Display\t\t*display = (Display*)NULL;\n");
    }
    if (main_needs_app_context_var(project))
    {
        abio_puts(codeFile, "XtAppContext\tapp = (XtAppContext)NULL;\n");
    }
    if (writingMain)
    {
        abio_puts(codeFile, "String\t\t*fallback_resources = (String*)NULL;\n");
        abio_puts(codeFile, "ArgList\t\tinit_args = (ArgList)NULL;\n");
        abio_puts(codeFile, "Cardinal\t\tnum_init_args = (Cardinal)0;\n");
    }
    if (main_needs_get_resources_vars(project))
    {
        abio_puts(codeFile, "ArgList\t\tget_resources_args = (ArgList)NULL;\n");
        abio_puts(codeFile, "Cardinal\t\tnum_get_resources_args = (Cardinal)0;\n");
    }
    if (abmfP_proj_needs_session_save(project))
    {
        abio_puts(codeFile, "Atom\t\tsave_yourself_atom = (Atom)NULL;\n");
    }
    write_main_i18n_local_vars(genCodeInfo, project);
    write_main_tooltalk_local_vars(genCodeInfo, project);
    if (main_needs_counter_var(project))
    {
        abio_puts(codeFile, "int\t\t\ti = 0;\n");
    }

    mainWindowHasIcon = 
	(   (mainWindow != NULL) 
	 && (   (obj_get_icon(mainWindow) != NULL)
	     || (obj_get_icon_mask(mainWindow) != NULL)) );
    if (mainWindowHasIcon)
    {
	abmfP_icon_pixmap_var(genCodeInfo) = istr_const("icon_pixmap");
	abmfP_icon_mask_pixmap_var(genCodeInfo) = 
					istr_const("icon_mask_pixmap");
	abio_puts(codeFile, "Pixmap\ticon_pixmap = NULL;\n");
	abio_puts(codeFile, "Pixmap\ticon_mask_pixmap = NULL;\n");
    }

    abio_puts(codeFile, nlstr);
    return 0;
}


/*
 * If we are writing main(), this will open the display and initialize Xt.
 * If we are writing project_initialize() instead, it gets the xt values
 * from the toplevel widget that is passed in.
 *
 * Sets variables: toplevel, display, app
 */
static int
write_init_xt_vars(
			GenCodeInfo	genCodeInfo, 
			ABObj		project
)
{
    File	codeFile = genCodeInfo->code_file;
    BOOL	writingMain = obj_get_write_main_func(project);
    STRING	displayNotOpenedMsg = NULL;

    if (writingMain)
    {
        abio_printf(codeFile,"toplevel = XtAppInitialize(&app, \"%s\",\n",
    	    abmfP_capitalize_first_char(obj_get_name(project)));
        abio_indent(codeFile);
        abio_puts(codeFile, "optionDescList, XtNumber(optionDescList),\n");
        abio_puts(codeFile, "&argc, argv, fallback_resources,\n");
        abio_puts(codeFile, "init_args, num_init_args);\n\n");
        abio_outdent(codeFile);
    }
       
    abmfP_write_c_comment(genCodeInfo, FALSE,
	"Get display and verify initialization was successful.");
    abio_puts(codeFile, "if (toplevel == NULL)\n");
    abmfP_write_c_block_begin(genCodeInfo);

    /* toplevel is NULL - why? */
    if (writingMain)
    {
        abio_puts(codeFile, 
	    "fprintf(stderr, \"Could not open display.\\n\");\n");
        abio_puts(codeFile, "exit(1);\n");
    }
    else
    {
	/*
	 * print:
	 *
	 * WARNING: dtb_<project>_initialize() called with invalid widget.
	 * Initialization failed.
	 *
	 */
	char	msg[1024];
	sprintf(msg, "\"\\nWARNING: %s() called with invalid widget.\\n\"",
		abmfP_get_project_init_proc_name(project));
        abio_printf(codeFile, "fprintf(stderr, %s);\n", msg);
        abio_puts(codeFile, 
	    "fprintf(stderr, \"Initialization failed.\\n\\n\");\n");
        abio_puts(codeFile, "return -1;\n");
    }

    abmfP_write_c_block_end(genCodeInfo);

    if (main_needs_app_context_var(project) && (!writingMain))
    {
	/* since, we're not writing main, we haven't called XtAppInitialize(),
	 * so we need to get the app context that was created elsewhere.
	 */
        abio_puts(codeFile, 
		"app = XtWidgetToApplicationContext(toplevel);\n");
    }
    if (main_needs_display_var(project))
    {
        abio_puts(codeFile, "display = XtDisplayOfObject(toplevel);\n\n");

    /* rhale - comments uneven by design because esc("\") - 1 == lost sp */
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"************************************************************************");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"vvvvvvvv Programmer may comment in to initalize libDtSvc vvvvvvvvvvvvvvv");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
" if (!(DtAppInitialize(&app, XtDisplay(toplevel),                       ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"                       toplevel, \"toplevel\", 0) ))                      ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
" {                                                                      ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"    printf(\"Error intializing Desktop Services Library \\n\");            ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"    exit(1);                                                            ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
" }                                                                      ");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"^^^^^^^^ Programmer may comment in to initalize libDtSvc ^^^^^^^^^^^^^^^");
    abmfP_write_c_comment(genCodeInfo, TRUE ,
"************************************************************************");
    abio_printf(codeFile," \n");
    /* rhale */

    }
    abio_puts(codeFile, nlstr);

    return 0;
}


static int
write_map_initially_visible_windows(
			GenCodeInfo	genCodeInfo, 
			ABObj		project
)
{
    AB_TRAVERSAL	trav;
    ABObj		window = NULL;
    ABObjList		windowList = objlist_create();
    int			numWindows = 0;
    ABObj		primaryWindow = NULL;
    int			i = 0;

    primaryWindow = abmfP_get_root_window(project);

    /*
     * Find the windows
     */
    for (trav_open(&trav, project, AB_TRAV_WINDOWS | AB_TRAV_MOD_SAFE);
         (window = trav_next(&trav)) != NULL;)
    {
        if ( (window != primaryWindow) && 	
             (obj_is_initially_visible(window)) &&
             obj_is_defined(window)
           )
        {
	    objlist_add_obj(windowList, window, NULL);
        }
    }
    trav_close(&trav);
    numWindows = objlist_get_num_objs(windowList);

    /*
     * Write a nifty comment
     */
    if (numWindows > 0)
    {
        abmfP_write_c_comment(genCodeInfo, FALSE,
            "Map initially-visible windows");
    }

    /*
     * Map 'em!
     */
    for (i = 0; i < numWindows; ++i)
    {
	window = objlist_get_obj(windowList, i, NULL);
        write_map_window(genCodeInfo, window, True);
    }
    if (numWindows > 0)
    {
	abio_puts(genCodeInfo->code_file, nlstr);
    }

    objlist_destroy(windowList);
    return numWindows;
}


/*
 * We need to parse the command line manually, if we are not able
 * to call XtAppInitialize(). I.e., if we are not generating main()
 */
static int
write_parse_command_line(GenCodeInfo genCodeInfo, ABObj project)
{
    File	codeFile = genCodeInfo->code_file;
    int		oldIndent = 0;
    char	*codeString = NULL;

    /* only parse if restoring session, and we're not writing main() */
    if (    obj_get_write_main_func(project)
	||  (!abmfP_proj_needs_session_restore(project))
       )
	return 0;

    oldIndent = abio_get_indent(codeFile);
    abio_set_indent(codeFile, 0);
    abio_puts(codeFile, nlstr);		/* needed for set_indent() to work */
    codeString = 
"    /*\n"
"     * Since XtAppInitialize() was called elsewhere, manually scan for\n"
"     * -session flag.\n"
"     */\n"
"    for (i = 1; i < argc; ++i)\n"
"    {\n"
"      	 if (strcmp(argv[i], \"-session\") == 0)\n"
"        {\n"
"            if (i >= (argc-1))\n"
"            {\n"
"                fprintf(stderr, \"%s: -session flag requires argument\\n\",\n"
"                    dtb_get_command());\n"
"                return -1;\n"
"            }\n"
"            else\n"
"            {\n"
"                /* get the argument and remove it from the arg list */\n"
"                dtb_app_resource_rec.session_file = argv[i+1];\n"
"                if ((argc-i-2) > 0)\n"
"                {\n"
"                    memmove(&(argv[i]), &(argv[i+2]),\n"
"                            (argc-i-2) * sizeof(*argv));\n"
"                }\n"
"                argc -= 2;\n"
"            }\n"
"        }\n"
"    }\n"
;
    abio_set_indent(codeFile, oldIndent);

    abio_puts(codeFile, codeString);
    return 0;
}


static int
write_main_session_restore(
    GenCodeInfo	genCodeInfo, 
    ABObj	project
)
{
    File	codeFile;

    if (   (genCodeInfo == NULL)
        || (project == NULL)
       )
	return (0);

    codeFile = genCodeInfo->code_file;

    if (abmfP_proj_needs_session_restore(project))
    {
        AB_TRAVERSAL	trav;
	ABObj		action;
	char		*ss_restore_CB_name = NULL;

        abio_puts(codeFile,"if (dtb_app_resource_rec.session_file)\n");
        abio_puts(codeFile,"{\n");

	if (!obj_is_project(project))
	    project = obj_get_project(project);

	/*
	 * Search for session restore callback in project
	 * action list
	 */
    	for (trav_open(&trav, project, AB_TRAV_ACTIONS);
             (action = trav_next(&trav)) != NULL;)
    	{
	    switch(obj_get_when(action))
	    {
		case AB_WHEN_SESSION_RESTORE:
		    /*
		     * Remember session restore callback if found
		     */
		    ss_restore_CB_name = obj_get_func_name(action);
		    break;
		default:
		    break;
	    }
	}
	trav_close(&trav);

	abio_printf(codeFile, 
	"    dtb_set_client_session_restoreCB((DtbClientSessionRestoreCB)%s);\n",
		ss_restore_CB_name ? ss_restore_CB_name : "NULL");

        abio_puts(codeFile,
	"    (void)dtb_session_restore(toplevel, dtb_app_resource_rec.session_file);\n");
        abio_puts(codeFile,"}\n");

        abio_puts(codeFile,"\n");
    }

    return (0);
}


static int 	
write_main_register_save_yourself(
			GenCodeInfo	genCodeInfo, 
			ABObj		project, 
			char		*atom_name
)
{
    File	codeFile;

    if (!genCodeInfo || !project)
	return (0);

    if (!atom_name)
	atom_name = "save_yourself_atom";

    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile,"\n");

    /*
     * Set client save session callback
     */
    if (abmfP_proj_needs_session_save(project))
    {
        AB_TRAVERSAL	trav;
	ABObj		action;
	char		*ss_save_CB_name = NULL;

        /*
         * Get WM_SAVE_YOURSELF atom
         */
        abio_puts(codeFile, "\n");
        abio_puts(codeFile, "save_yourself_atom = XmInternAtom(XtDisplay(toplevel),\n");
        abio_puts(codeFile, "\t\"WM_SAVE_YOURSELF\", False);\n");

	/*
	 * Search for session save callback in project
	 * action list
	 */
    	for (trav_open(&trav, project, AB_TRAV_ACTIONS);
             (action = trav_next(&trav)) != NULL;)
    	{
	    switch(obj_get_when(action))
	    {
		case AB_WHEN_SESSION_SAVE:
		    /*
		     * Remember session save callback if found
		     */
		    ss_save_CB_name = obj_get_func_name(action);
		    break;
		default:
		    break;
	    }
	}
	trav_close(&trav);

	abio_printf(codeFile, 
	    "dtb_set_client_session_saveCB((DtbClientSessionSaveCB)%s);\n",
		ss_save_CB_name ? ss_save_CB_name : "NULL");

	abio_puts(codeFile, "\n");

        abio_printf(codeFile,"XmAddWMProtocolCallback(toplevel, %s,\n", 
			atom_name);
        abio_puts(codeFile,"\tdtb_session_save, (XtPointer)NULL);\n");
    }

    abio_puts(codeFile,"\n");

    return (0);
}


/*
 * writes local vars for i18n, as needed
 */
static int
write_main_i18n_local_vars(
			GenCodeInfo	genCodeInfo,
			ABObj		project
)
{
    File	codeFile = genCodeInfo->code_file;
    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
        abio_puts(codeFile, 
	    "XtLanguageProc\tlanguage_proc = (XtLanguageProc)NULL;\n");
        abio_puts(codeFile, 
	    "XtPointer\t\tlanguage_proc_client_data = (XtPointer)NULL;\n");
    }
    return 0;
}


static int
write_main_xt_i18n(
    GenCodeInfo genCodeInfo,
    ABObj project
    )
{
    File	codeFile;
    int		ret_val = 0;
    
    if (!genCodeInfo || !project)
	goto cret;

    codeFile = genCodeInfo->code_file;

    if (obj_get_write_main_func(project))
    {
        abmfP_write_c_comment(genCodeInfo, TRUE,
            "NULL language_proc installs the default Xt language procedure");
        abio_puts(codeFile, "XtSetLanguageProc((XtAppContext)NULL,\n");
        abio_indent(codeFile);
        abio_puts(codeFile, "language_proc, language_proc_client_data);\n");
        abio_outdent(codeFile);
        abio_puts(codeFile, nlstr);
    }

cret:
    return(ret_val);
}


static int
write_main_msg_i18n(
    GenCodeInfo genCodeInfo,
    ABObj project
    )
{
    File	codeFile;
    int		ret_val = 0;
    
    if (!genCodeInfo || !project)
	goto cret;

    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile, abmfP_comment_begin);
    abio_puts(codeFile, abmfP_comment_continue);
    abio_puts(codeFile,
              "Open the standard message catalog for the project.\n");
    abio_puts(codeFile, abmfP_comment_end);
    
    abio_puts(codeFile,
       "Dtb_project_catd = catopen(DTB_PROJECT_CATALOG, NL_CAT_LOCALE);\n");
    abio_puts(codeFile, "if (Dtb_project_catd == (nl_catd)-1)\n");
    abmfP_write_c_block_begin(genCodeInfo);
    abio_puts(codeFile,
              "fprintf(stderr, \"WARNING: Could not open message catalog: %s. Messages will be defaults.\\n\",\n");
    abio_indent(codeFile);
    abio_puts(codeFile, "DTB_PROJECT_CATALOG);\n");
    abio_outdent(codeFile);
    abmfP_write_c_block_end(genCodeInfo);

    abio_puts(codeFile, "\n");

cret:
    return(ret_val);
}


static int
write_main_tooltalk_local_vars(
    GenCodeInfo genCodeInfo,
    ABObj project
)
{
    File        codeFile;
    int         ret_val = 0;

    if (!genCodeInfo || !project ||
	(obj_get_tooltalk_level(project) == AB_TOOLTALK_NONE))
        return 0;

    codeFile = genCodeInfo->code_file;

    abio_puts(codeFile, "char\t\t*tt_proc_id = NULL;\n");
    abio_puts(codeFile, "int\t\t\ttt_fd = -1;\n");
    abio_puts(codeFile, "Tt_status\t\ttt_status = TT_OK;\n");
    abio_puts(codeFile, "Tt_pattern\t\t*tt_session_pattern = (Tt_pattern*)NULL;\n");
    abio_puts(codeFile, "char\t\t*ttenv = NULL;\n");
    abio_puts(codeFile, "char\t\t*session = NULL;\n");

    return 0;
}


static int
write_main_tooltalk_init(
    GenCodeInfo genCodeInfo, 
    ABObj project 
) 
{ 
    AB_TOOLTALK_LEVEL	tt_level;
    AB_TRAVERSAL	trav;
    STRING		vendor, version;
    ABObj		action;
    File        	codeFile; 
    int         	ret_val = 0; 
 
    if (!genCodeInfo || !project ||
        ((tt_level = obj_get_tooltalk_level(project)) == AB_TOOLTALK_NONE))
        return 0;
 
    codeFile = genCodeInfo->code_file; 

    abio_puts(codeFile, abmfP_comment_begin);
    abio_puts(codeFile, abmfP_comment_continue);
    abio_puts(codeFile, "Initialize ToolTalk to handle Desktop Message Protocol\n");
    abio_puts(codeFile, abmfP_comment_end);

    if (tt_level == AB_TOOLTALK_DESKTOP_ADVANCED)
    {
    	for (trav_open(&trav, project, AB_TRAV_ACTIONS);
             (action = trav_next(&trav)) != NULL;)
    	{
	    switch(obj_get_when(action))
	    {
		case AB_WHEN_TOOLTALK_QUIT:
		    abio_printf(codeFile, "dtb_set_tt_msg_quitCB((DtbTTMsgHandlerCB)%s);\n",
			obj_get_func_name(action));
		    break;
		case AB_WHEN_TOOLTALK_DO_COMMAND:
                    abio_printf(codeFile, "dtb_set_tt_msg_do_commandCB((DtbTTMsgHandlerCB)%s);\n", 
                        obj_get_func_name(action)); 
                    break; 
                case AB_WHEN_TOOLTALK_GET_STATUS: 
                    abio_printf(codeFile, "dtb_set_tt_msg_get_statusCB((DtbTTMsgHandlerCB)%s);\n",  
                        obj_get_func_name(action));  
                    break; 
                case AB_WHEN_TOOLTALK_PAUSE_RESUME:
                    abio_printf(codeFile, "dtb_set_tt_msg_pause_resumeCB((DtbTTMsgHandlerCB)%s);\n",  
                        obj_get_func_name(action));
                    break;
		default:
		    break;
	    }
	}
	trav_close(&trav);
	abio_puts(codeFile, "\n");
    }

    /* Write out tt init code to deal with possible remote display session */
    abio_puts(codeFile, "ttenv = getenv(\"TT_SESSION\");\n");
    abio_puts(codeFile, "if (!ttenv || strlen(ttenv) == 0)\n");
    abio_indent(codeFile);
    abio_puts(codeFile,"ttenv = getenv(\"_SUN_TT_SESSION\");\n");
    abio_outdent(codeFile);
    abio_puts(codeFile, "if (!ttenv || strlen(ttenv) == 0)\n");
    abmfP_write_c_block_begin(genCodeInfo);
    abio_puts(codeFile, "session = tt_X_session(XDisplayString(display));\n");
    abio_puts(codeFile, "tt_default_session_set(session);\n");
    abio_puts(codeFile, "tt_free(session);\n");
    abmfP_write_c_block_end(genCodeInfo);
    
    abio_printf(codeFile, "tt_proc_id = ttdt_open(&tt_fd, \"%s\", ",
	obj_get_name(project));
    vendor = obj_get_vendor(project);
    version = obj_get_version(project);
    abio_printf(codeFile, "\"%s\", \"%s\", 1);\n",
	vendor? vendor : "NULL", version? version : "NULL");
    abio_puts(codeFile, "tt_status = tt_ptr_error(tt_proc_id);\n");
    abio_puts(codeFile, "if (tt_status != TT_OK)\n");
/*
    abio_puts(codeFile, "{\n");
    abio_indent(codeFile);
*/
    abmfP_write_c_block_begin(genCodeInfo);
    abio_puts(codeFile, 
	"fprintf(stderr,\"ttdt_open(): %s\\n\", tt_status_message(tt_status));\n");
    abio_puts(codeFile, "tt_proc_id = NULL;\n");
/*
    abio_outdent(codeFile);
    abio_puts(codeFile, "}\n");
*/
    abmfP_write_c_block_end(genCodeInfo); 
    abio_puts(codeFile, "else\n");
/*
    abio_puts(codeFile, "{\n");
    abio_indent(codeFile);
*/
    abmfP_write_c_block_begin(genCodeInfo);
    abio_puts(codeFile, "XtAppAddInput(app, tt_fd, (XtPointer)XtInputReadMask,\n");
    abio_indent(codeFile);
    abio_puts(codeFile, "tttk_Xt_input_handler, tt_proc_id);\n\n");
    abio_outdent(codeFile); 
    abio_printf(codeFile, "tt_session_pattern = ttdt_session_join(NULL, %s,\n",
	tt_level == AB_TOOLTALK_DESKTOP_ADVANCED? "dtb_tt_contractCB" : "NULL");
    abio_indent(codeFile);
    abio_puts(codeFile, "toplevel, NULL, True);\n\n");
    abio_outdent(codeFile);
    abio_puts(codeFile, "atexit(dtb_tt_close);\n");
/*
    abio_outdent(codeFile); 
    abio_puts(codeFile, "}\n");
*/
    abmfP_write_c_block_end(genCodeInfo);  

    return 0;

}

