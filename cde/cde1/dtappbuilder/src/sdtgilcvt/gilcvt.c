
/*
 *	$Revision: 1.1 $
 *
 *	@(#)gilcvt.c	1.29 11/08/95	cde_app_builder/src/dtgilcvt
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
#pragma ident "@(#)gilcvt.c	1.29 95/11/08 SMI"

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/times.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <nl_types.h>
#include <ab_private/util.h>
#include <ab_private/abio.h>
#include <ab_private/obj.h>
#include <ab_private/gil.h>
#include <ab_private/bil.h>
#include "dtb_utils.h"
#include "gilcvt.h"
#include "gilcvt_utilsP.h"
#include "gilcvtP.h"


/*
 * REMIND: move util_ functions to libAButil
 */
#ifndef util_dassert
#ifdef DEBUG
    #define util_dassert(_debugLevel, _boolExpr) \
	    {if (debug_level() >= (_debugLevel)) {assert((_boolExpr));}}
#else
    #define util_dassert(_debugLevel, _boolExpr) \
	    /* ignore this */;
#endif
#endif
#ifndef util_dabort
#ifdef DEBUG
    #define util_dabort(_lvl) {if (debug_level() >= (_lvl)) {abort();}}
#else
    #define util_dabort(_lvl) /* ignore this */;
#endif
#endif

static int	util_symlink(STRING oldPath, STRING newPath);

#ifdef __cplusplus
extern "C" {
#endif
extern int fsync(int fildes);		/* non-POSIX function */
#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus
extern "C" {
#endif
extern long _sysconf(int name);		/* CLK_TCK uses this */
#ifdef __cplusplus
} // extern "C"
#endif

#define INT_UNDEF	(INT_MIN)

typedef enum
{
    GILCVT_PROFILE_UNDEF = 0,
    GILCVT_PROFILE_LOAD,
    GILCVT_PROFILE_CONFIG,
    GILCVT_PROFILE_TRAVERSALS,
    GILCVT_PROFILE_WHAT_NUM_VALUES
} GILCVT_PROFILE_WHAT;


typedef struct
{
    BOOL			force_load_all;
    GILCVT_PROFILE_WHAT		debug_profile_what;
    BOOL			dump_tree;
    ISTRING            		proj_file;
    StringList			files;
    BOOL               		write_project;
    BOOL			use_default_project;
    int                		verbosity;
} CmdlineArgsRec, *CmdlineArgs;


/*
 * Private Functions
 */
static int      gilcvt_init(void);
static int      gilcvt_usage(void);
static int      parse_args(int argc, char *argv[], CmdlineArgs args);
static int      find_proj_file(CmdlineArgs args);
static int	mark_modules_to_load_and_write(
			ABObj		project, 
			BOOL		loadAllModules, 
			BOOL		genAllModules, 
			BOOL		genNoModules,
			StringList	fileNames
		);
static int	load_marked_modules(ABObj project);
static int	load_module(ABObj module);
static BOOL     write_required(ABObj tree);
static int      examine_tree(ABObj project);
static int	munge_ensure_win_parent(ABObj win);
static int	munge_ensure_defined(ABObj obj);
static int	munge_ensure_item_selected(ABObj obj);
static int	munge_set_bil_file_name(ABObj obj);
static int	munge_move_action(ABObj obj);
static int	munge_group_geometry(ABObj obj);
static int	munge_check_geometry(ABObj obj);
static int	munge_check_glyph_files(ABObj obj);
static int	munge_check_layered_pane(ABObj layeredPane);
static int	gilcvtP_tree_determine_new_geometry(
			ABObj		root, 
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
		);
static int	gilcvtP_obj_determine_new_geometry(
			ABObj		obj,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
		);
static int	gilcvtP_tree_adjust_geometry(
			ABObj		tree,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
		);
static int	gilcvtP_obj_adjust_geometry(
			ABObj		obj,
			int		deltaX,
			int		deltaY,
			int		deltaWidth,
			int		deltaHeight,
			BOOL		deltaIsFromOrg,
			BOOL		adjustOthers,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
		);
static int	link_file(STRING oldPath, STRING newPath);
/* static int	munge_liberate_menu(ABObj menu); REMIND: remove */
/* static int	dup_all_menu_refs(ABObj project); REMIND: remove */
static int	gilcvtP_dup_menu_ref_tree(ABObj obj);
static int	gilcvtP_do_dup_menu_ref_tree(ABObj obj);
static int	dump_tree(ABObj tree);
static int	print_tree(ABObj root, int indent);
static int	gilcvtP_prepare_tree(ABObj project);
static int	gilcvtP_create_obj_data_for_project(ABObj project);
static int	gilcvtP_create_obj_data_for_module(ABObj objInModule);
static int	ensure_data_for_module_obj(ABObj module);
static BOOL	gil_proj_file_has_modules(
			STRING		projFileName, 
			StringList	fileNames
		);
static int	replace_string_shorter(
			STRING buf, 
			STRING subStr, 
			STRING replaceStr);

/*
 * REMIND: These routines should be in libABil somewhere
 */
static BOOL	util_file_name_has_gil_extension(STRING path);

/*
 * Debugging routines
 */
#ifdef DEBUG
static int	dump_callbacks(ABObj project);
static int	examine_tree_debug(ABObj root);

/* performance testing */
static int	time_traversal(ABObj root);
static int	get_cur_time(double *realTimeOut, double *cpuTimeOut);
#endif /* DEBUG */


/* Internationalization defines */
nl_catd Dtb_project_catd;

/* Workaround for XPG4 API compatibility */
#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif


/* ARGSUSED */
main(int argc, STRING *argv)
{
    int			exitValue = 0;
    int                 iRC= 0;         /* int return code */
    CmdlineArgsRec      cmdlineRec;
    CmdlineArgs         cmdline = &cmdlineRec;
    ABObj               project = NULL;
    ABObj               module = NULL;
    int                 num_modules_processed = 0;
    BOOL		genAllFiles = FALSE;
    BOOL		genMain = FALSE;
    BOOL		genMainOnly = FALSE;
    BOOL		useDefaultProject = FALSE;
    STRING		errmsg = NULL;

#ifdef DEBUG	/* performance testing */
    double		progStartSeconds = 0.0;
    double		progEndSeconds = 0.0;
    double		progSeconds = 0.0;
    double		progStartCPUSeconds = 0.0;
    double		progEndCPUSeconds = 0.0;
    double		progCPUSeconds = 0.0;
    double		configStartSeconds = 0.0;
    double		configEndSeconds = 0.0;
    double		configSeconds = 0.0;
    double		configStartCPUSeconds = 0.0;
    double		configEndCPUSeconds = 0.0;
    double		configCPUSeconds = 0.0;
    double		loadStartSeconds = 0.0;
    double		loadEndSeconds = 0.0;
    double		loadSeconds = 0.0;
    double		loadStartCPUSeconds = 0.0;
    double		loadEndCPUSeconds = 0.0;
    double		loadCPUSeconds = 0.0;
    double		startupStartSeconds = 0.0;
    double		startupEndSeconds = 0.0;
    double		startupSeconds = 0.0;
    double		startupStartCPUSeconds = 0.0;
    double		startupEndCPUSeconds = 0.0;
    double		startupCPUSeconds = 0.0;
    get_cur_time(&progStartSeconds, &progStartCPUSeconds);
    startupStartSeconds = loadStartSeconds = configStartSeconds
				= progStartSeconds;
    startupStartCPUSeconds = loadStartCPUSeconds = configStartCPUSeconds
				= progStartCPUSeconds;
#endif /* DEBUG */

    /*
     * Open the standard message catalog for the project.
     */
    Dtb_project_catd = catopen("sdtgilcvt", NL_CAT_LOCALE);
    if (Dtb_project_catd == (nl_catd)-1)
    {
        fprintf(stderr, 
	    "WARNING: Could not open message catalog: sdtgilcvt.cat\n");
    }

    util_init(&argc, &argv);
    dtb_save_command(argv[0]);
    /*objxm_init(NULL);*/
    gilcvt_init();

    if ((iRC = parse_args(argc, argv, cmdline)) < 0)
    {
        exit(1);
    }
    util_set_verbosity(cmdline->verbosity);

    /*********************************************************************
     **									**
     **		 Figure out what the user wants				**
     **									**
     *********************************************************************/

    /*
     * Find the project file
     */
    if (cmdline->use_default_project)
    {
	useDefaultProject = TRUE;
    }
    else if (cmdline->proj_file == NULL)
    {
        int numProjFiles = find_proj_file(cmdline);

	if (   (numProjFiles < 0)
	    || (numProjFiles > 1))
	{
	    /* assume find_project_file() complained */
	    exitValue = 1;
	    goto epilogue;
	}
	else if (numProjFiles == 0)
	{
	    if (   (cmdline->files == NULL) 
		|| (strlist_get_num_strs(cmdline->files) == 0))
	    {
		util_printf(
		    catgets(Dtb_project_catd, 1, 2,
		      "No files specified, and no project file found\n"));
		gilcvt_usage();
	    }
	    else
	    {
		useDefaultProject = TRUE;
	        if (!util_be_silent())
	        {
	            util_printf(
			catgets(Dtb_project_catd, 1, 1,
	    "No project file that references module(s) - using defaults\n"));
	        }
	    }
	}
	else if (numProjFiles == 1)
	{
	    util_dassert(1, (cmdline->proj_file != NULL));
	}
    }

    genMain = FALSE;
    genMainOnly = FALSE;
    genAllFiles = FALSE;
    if (strlist_is_empty(cmdline->files))
    {
	if (cmdline->write_project)
	{
	    /* only -main was specified */
	    genMain = TRUE;
	    genMainOnly = TRUE;
	}
	else
	{
	    /* no specific files requested - gen them all */
	    genAllFiles = TRUE;
	    genMain = TRUE;
	}
    }
    else
    {
	/* files specified */
	if (cmdline->write_project)
	{
	    genMain = TRUE;
	    if (useDefaultProject)
	    {
		/* dtcodegen -np module.G -main : only gen main */
		genMainOnly = TRUE;
	    }
	}
	else
	{
	    if (useDefaultProject)
	    {
		genMain = cmdline->write_project;
		genAllFiles = FALSE;
	    }
	}
    }

    util_dprintf(1, "main:%d mainonly:%d all:%d defaultproj:%d\n",
	genMain, genMainOnly, genAllFiles, useDefaultProject);

    if ((!util_be_silent()) && (cmdline->proj_file != NULL))
    {
	char *proj_file_name = istr_string_safe(cmdline->proj_file);
        util_printf(
	    catgets(Dtb_project_catd, 1, 3, "Reading project %s.\n"),
                proj_file_name);
    }

#ifdef DEBUG	/* performance testing */
    get_cur_time(&loadStartSeconds, &loadStartCPUSeconds);
#endif

    /*********************************************************************
     **									**
     **			Load or create the project			**
     **									**
     *********************************************************************/

    if (cmdline->proj_file != NULL)
    {
	util_dassert(1,(!useDefaultProject));
	/* project = gil_load_file(istr_string(cmdline->proj_file), NULL, project);*/

	fflush(stdout); fsync(fileno(stdout));
        iRC = gil_load_project_file_and_resolve_all(
				istr_string(cmdline->proj_file), NULL, &project);
	util_dassert(1, (project != NULL));
	if (iRC < 0)
	{
	    exitValue = 1;
	    goto epilogue;
	}
    }

    if (useDefaultProject)
    {
	int	i = 0;
	int	numModuleFiles = 0;
	STRING	moduleFileName = NULL;
	ABObj	fileObj = NULL;
	char	*dotPtr = NULL;
	char	projNameBuf[1024];
	*projNameBuf = 0;

	/*
	 * Create the default project
	 */
	if (project != NULL)
	{
	    ABObj oldProject = project;
	    project = obj_create(AB_TYPE_PROJECT, NULL);
	    obj_move_children(project, oldProject);
	    obj_destroy(oldProject);
	}
	else
	{
	    project = obj_create(AB_TYPE_PROJECT, NULL);
	}
	util_strncpy(projNameBuf, 
			strlist_get_str(cmdline->files, 0, NULL), 1024);
	dotPtr = strrchr(projNameBuf, '.');
	if (dotPtr != NULL)
	{
	    *dotPtr = 0;
	}
	obj_set_name(project, projNameBuf);
	obj_set_file(project, strlist_get_str(cmdline->files, 0, NULL));
	obj_set_is_default(project, TRUE);

	/*
	 * Create "files" to get converted to undefined modules
	 */
	numModuleFiles = strlist_get_num_strs(cmdline->files);
	for (i = 0; i < numModuleFiles; ++i)
	{
	    moduleFileName = strlist_get_str(cmdline->files, i, NULL);
	    fileObj = obj_create(AB_TYPE_FILE, project);
	    obj_set_file(fileObj, moduleFileName);
	}
    }

    /*
     * Load the appropriate files
     */
    if (genMain)
    {
	mfobj_set_flags(project, CGenFlagLoadMe|CGenFlagIsReferenced);
	obj_set_write_me(project, TRUE);
    }
    mark_modules_to_load_and_write(
	project, cmdline->force_load_all, genAllFiles, genMainOnly, cmdline->files);
    load_marked_modules(project);

#ifdef DEBUG	/* performance testing */
    get_cur_time(&loadEndSeconds, &loadEndCPUSeconds);
    loadSeconds = loadEndSeconds - loadStartSeconds;
    loadCPUSeconds = loadEndCPUSeconds - loadStartCPUSeconds;
    printf("load real:%lg  CPU:%lg\n", loadSeconds, loadCPUSeconds);
#endif /* DEBUG */

    if (iRC < 0)
    {
	char *prog_name_string = util_get_program_name();

        fprintf(stderr, 
	    catgets(Dtb_project_catd, 1, 4,
	        "%s: exiting due to error loading project.\n"), 
		prog_name_string);
        exit(1);
    }
    if (cmdline->debug_profile_what == GILCVT_PROFILE_LOAD)
    {
	exit(0);
    }

    /*
     * See if we actually have anything to write
     */
    if (!write_required(project))
    {
	char *prog_name_string = util_get_program_name();

        fprintf(stderr, 
	    catgets(Dtb_project_catd, 1, 5,
	      "%s: Nothing to do!\n"), prog_name_string);
        exit(1);
    }

    /*
     * Configure the raw objects into AB objects
     */

#ifdef DEBUG	/* performance testing */
    get_cur_time(&configStartSeconds, &configStartCPUSeconds);
#endif

    examine_tree(project);

#ifdef DEBUG	/* performance testing */
    get_cur_time(&configEndSeconds, &configEndCPUSeconds);
    configSeconds = configEndSeconds - configStartSeconds;
    configCPUSeconds = configEndCPUSeconds - configStartCPUSeconds;
    get_cur_time(&startupEndSeconds, &startupEndCPUSeconds);
    startupSeconds = startupEndSeconds - startupStartSeconds;
    startupCPUSeconds = startupEndCPUSeconds - startupStartCPUSeconds;

    /*
     * Print out startup stats (if debugging build)
     */
    util_printf("Startup real time: %lg s (%lg s load, %lg s config)\n",
		startupSeconds, loadSeconds, configSeconds);
    util_printf("Startup CPU time: %lg s (%lg s load, %lg s config)\n",
		startupCPUSeconds, loadCPUSeconds, configCPUSeconds);
#endif /* DEBUG */

#ifdef DEBUG

    util_dprintf(1, "after configure_tree\n");

    /* print out the tree, if debugging level is high enough */
    if (!cmdline->dump_tree)
    {
	int	numObjs = trav_count(project, AB_TRAV_ALL);
	if (   ((debug_level() >= 2) && (numObjs <= 100))
	    && (debug_level() >= 7) )
	{
	    printf("Dumping tree: %d objects\n", numObjs);
	    obj_tree_print(project);
	}
    }

    /* make sure we didn't muss anything up */
    if (debugging())
    {
	examine_tree_debug(project);
    }
#endif /* DEBUG */

    /*
     * The tree has been loaded, examined, mangled, and generally messed with.
     * Now, do what the user has requested.
     */

    if (cmdline->dump_tree)
    {
	dump_tree(project);
    }
    else
    {
	AB_ARG_CLASS_FLAGS dumpedRes = obj_get_res_file_arg_classes(project);

        /*
         * Generate the code!
         */
        iRC = gilcvt_write_bil_files(project);
	if (iRC < 0)
	{
	    exitValue = 1;
	}
    }

#ifdef DEBUG	/* performance testing */
    get_cur_time(&progEndSeconds, &progEndCPUSeconds);
    progSeconds = progEndSeconds - progStartSeconds;
    progCPUSeconds = progEndCPUSeconds - progStartCPUSeconds;
    util_printf("Total real time: %lg s   CPU time: %lg s\n",
	progSeconds, progCPUSeconds);
#endif /* DEBUG */

epilogue:
#ifdef DEBUG
    if (debugging() && (exitValue >= 0))
    {
	examine_tree_debug(project);
    }
#endif /* DEBUG */

    if (exitValue != 0)
    {
	util_printf_err("Quitting due to errors.\n");
    }
    return exitValue;
}


static int
parse_args(int argc, char *argv[], CmdlineArgs cmdline)
{
    int                 iReturn = 0;
    int                 argCount = 0;

    /*
     * Set default values
     */
    cmdline->force_load_all = TRUE;
    cmdline->debug_profile_what = GILCVT_PROFILE_UNDEF;
    cmdline->dump_tree = FALSE;
    cmdline->proj_file = NULL;
    cmdline->files = strlist_create();
    cmdline->write_project = FALSE;
    cmdline->use_default_project = FALSE;
    cmdline->verbosity = util_get_verbosity();

    for (argCount = 1; argCount < argc; ++argCount)
    {
        char               *arg = argv[argCount];
        if (*arg == '-')
        {
            if (strcmp(arg, "-s") == 0 ||
                strcmp(arg, "-silent") == 0)
            {
                cmdline->verbosity = 0;
            }
	    else if (util_streq(arg, "-tree"))
	    {
		cmdline->dump_tree = TRUE;
	    }
            else if (strcmp(arg, "-main") == 0)
            {
                cmdline->write_project = TRUE;
            }
            else if (strcmp(arg, "-P") == 0 ||
                     strcmp(arg, "-p") == 0 ||
                     strcmp(arg, "-project") == 0)
            {
                if ((argCount + 1) >= argc)
                {
                    util_error( catgets(Dtb_project_catd, 1, 6,
		    "Missing project name for -p option"));
                    gilcvt_usage();
                }
                else
                {

                    /*
                     * get the project file name
                     */
                    STRING      projArg;
                    ++argCount;
                    projArg= argv[argCount];
                    if (   util_file_name_has_extension(projArg, "bip")
                        || util_file_name_has_extension(projArg, "bix"))
                    {
                        cmdline->proj_file = istr_const(projArg);
                    }
                    else
                    {
                        char                projFile[MAX_PATH_SIZE];
                        sprintf(projFile, "%s.P", projArg);
			if (!util_file_exists(projFile))
			{
			    char	encapsFile[MAXPATHLEN];
			    sprintf(encapsFile, "%s.bix", projArg);
			    if (util_file_exists(encapsFile))
			    {
				strcpy(projFile, encapsFile);
			    }
			}
                        cmdline->proj_file = istr_create(projFile);
                    }
                }
            }
	    else if (   util_streq(arg, "-np")
		     || util_streq(arg, "-noproject"))
	    {
	        cmdline->use_default_project = TRUE;
	    }
            else if (strcmp(arg, "-help") == 0)
            {
                gilcvt_usage();
            }
            else if (strncmp(arg, "-v", 2) == 0)
            {
		cmdline->verbosity = 2;
            }

	    /* 
	     * debugging options 
	     */
#ifdef DEBUG
	    else if (strncmp(arg, "-V", 2) == 0)
	    {
                int                 i;
                for (i = 1; arg[i] == 'V'; ++i) 
		    { /* empty */ }
                --i;
                cmdline->verbosity = i;
	    }
	    else if (strncmp(arg, "-time", 5) == 0)
	    {
		if (argCount < (argc-1))
		{
		    arg = argv[++argCount];
		    if (util_streq(arg, "load"))
		    {
			cmdline->debug_profile_what = GILCVT_PROFILE_LOAD;
		    }
		}
	    }
	    else if (strncmp(arg, "-loadall", 8) == 0)
	    {
		cmdline->force_load_all = TRUE;
	    }
#endif /* DEBUG */
            else
            {
		char *prog_name_string = util_get_program_name();

                fprintf(stderr, catgets(Dtb_project_catd, 1, 7,
		    "%s: Illegal option \"%s\"\n\n"), prog_name_string, arg);
                gilcvt_usage();
            }

        }                       /* arg == '-' */
        else
        {

            /*
             * It's a file name
             */
            if (abio_is_gil_proj_path(arg))
            {
                if (cmdline->proj_file != NULL)
                {
                    util_error(
			catgets(Dtb_project_catd, 1, 8,
		    "Only one project file may be specified."));
                    gilcvt_usage();
                }
                else
                {
                    cmdline->proj_file = istr_const(arg);
                }
            }
            else
            {
                ISTRING             newFile = NULL;

                if (util_file_name_has_gil_extension(arg))
                {
                    newFile = istr_const(arg);
                }
                else
                {
                    char                fileName[MAX_PATH_SIZE];
                    strcpy(fileName, arg);
                    strcat(fileName, ".G");
                    newFile = istr_create(fileName);
                }

		strlist_add_istr(cmdline->files, newFile, NULL);
            }                   /* not project file */
        }                       /* ! flag arg */
    } /* for argCount */


    /*
     * Make the comand-line args consistent
     */
    if (cmdline->use_default_project && (cmdline->proj_file != NULL))
    {
	char *proj_file_name = istr_string_safe(cmdline->proj_file);

        util_printf_err(
	  catgets(Dtb_project_catd, 1, 9,
          "ignoring -noproject (-np) because project file was specified: %s\n"),
	       proj_file_name);
	cmdline->use_default_project = FALSE;
    }

    return iReturn;
}


static int
mark_modules_to_load_and_write(
			ABObj		project, 
			BOOL		loadAllModules, 
			BOOL		genAllModules,
			BOOL		genNoModules,
			StringList	fileNames
)
{
    int			return_value = 0;
    AB_TRAVERSAL	fileTrav;
    ABObj		file = NULL;
    ISTRING		fileName = NULL;
    char		newModuleName[1024];
    STRING		dotPtr = NULL;
    AB_TRAVERSAL	allTrav;
    ABObj		obj = NULL;
    ABObj		obj1 = NULL;
    ABObj		obj2 = NULL;
    AB_TRAVERSAL	moduleTrav;
    ABObj		module = NULL;
    ABObj		winParent = NULL;
    *newModuleName = 0;

    /*
     * Convert remaining files into undefined modules
     */
    for (trav_open(&fileTrav, project, AB_TRAV_FILES | AB_TRAV_MOD_SAFE);
	(file = trav_next(&fileTrav)) != NULL; )
    {
	fileName = istr_create(obj_get_file(file));
	util_get_file_name_from_path(
			istr_string(fileName), newModuleName, 1024);
	dotPtr = strrchr(newModuleName, '.');
	if (dotPtr != NULL)
	{
	    *dotPtr = 0;
	}

	module = obj_find_module_by_name(project, newModuleName);
	if (module != NULL)
	{
	    /* this module already exists, probably because of a forward */
	    /* reference */
	    obj_destroy(file);
	}
	else
	{
	    obj_set_type(file, AB_TYPE_MODULE);
	    module = file; file = NULL;
	    obj_set_name(module, newModuleName);
	    obj_set_is_defined(module, FALSE);
	    mfobj_set_flags(module, CGenFlagLoadMe|CGenFlagIsReferenced);
	}

	/*
	 * Objects/modules created as a forward reference will
	 * not have their file names set, so we're going to
	 * check it.
	 */
	if (obj_get_file(module) == NULL)
	{
	    obj_set_file(module, istr_string(fileName));
	}
	istr_destroy(fileName);

	load_module(module);
    }
    trav_close(&fileTrav);


    /*
     * Mark the modules that are to be written
     */
    {
	char	explodedModuleFile[MAXPATHLEN+1];
	*explodedModuleFile = 0;

        for (trav_open(&moduleTrav, project, AB_TRAV_MODULES|AB_TRAV_MOD_SAFE);
	    (module = trav_next(&moduleTrav)) != NULL; )
        {
	    sprintf(explodedModuleFile, "%s.G", obj_get_name(module));

	    if (   (genAllModules && obj_is_defined(module))
	        || (   strlist_str_exists(fileNames, explodedModuleFile)
		    && (!genNoModules))
	       )
	    {
		if (obj_is_defined(module))	/* must be defined!! */
		{
	            strlist_set_str_data(fileNames, 
				explodedModuleFile, (void*)TRUE);
	            obj_set_write_me(module, TRUE);
		}
            }
        }
        trav_close(&moduleTrav);
    }

    /*
     * Make sure all the modules on the command line were found
     */
    {
	int	i = 0;
	int	numFiles = strlist_get_num_strs(fileNames);
	STRING	fileName = NULL;
	BOOL	wasFound = FALSE;
	void	*wasFoundVoidPtr;
	BOOL	errOccurred = FALSE;
        for (i = 0; i < numFiles; ++i)
	{
	    fileName = strlist_get_str(fileNames, i, &wasFoundVoidPtr);
	    util_dassert(1, (fileName != NULL));
	    wasFound = (BOOL)(unsigned)wasFoundVoidPtr;
	    if (!wasFound)
	    {
		util_printf_err("module not found in project: %s\n", fileName);
		errOccurred = TRUE;
	    }
	}
	if (errOccurred)
	{
	    exit(1);
	}
    }

    /*
     * Determine dependencies necessary to write the requested modules
     */
    /* if loadAll... */
    {
        for (trav_open(&allTrav, project, 
		AB_TRAV_ALL | AB_TRAV_MOD_PARENTS_FIRST | AB_TRAV_MOD_SAFE);
	    (obj = trav_next(&allTrav)) != NULL; )
        {
	    switch (obj_get_type(obj))
	    {
	    case AB_TYPE_MODULE:
	        if (obj_get_write_me(obj))
	        {
		    mfobj_set_flags(obj, CGenFlagIsReferenced);
	        }
	    break;
    
	    /*
	     * win-parent can be in another module
	     */
	    case AB_TYPE_DIALOG:
	    case AB_TYPE_BASE_WINDOW:
	        if (   obj_is_defined(obj)
		    && obj_is_popup(obj)
		    && obj_get_write_me(obj_get_module(obj)))
	        {
		    if ((winParent = obj_get_win_parent(obj)) != NULL)
		    {
		        mfobj_set_flags(obj_get_module(winParent), 
					CGenFlagIsReferenced);
		    }
	        }

		/* visible windows must be loaded to write main */
		if (   obj_get_write_me(project)
		    && obj_is_defined(obj) && obj_is_initially_visible(obj))
		{
		    mfobj_set_flags(obj_get_module(obj), 
					CGenFlagIsReferenced);
		}
	    break;
    
	    /*
	     *
	     */
	    case AB_TYPE_ACTION:
		obj1 = obj_get_from(obj);
		if (obj_is_project(obj1))
		    break;
		obj2 = obj_get_to(obj);
		if (  (   (obj1 != NULL) 
		       && obj_is_defined(obj1)
		       && obj_get_write_me(obj_get_module(obj1)))
		    ||(   (obj2 != NULL) 
		       && obj_is_defined(obj2)
		       && obj_get_write_me(obj_get_module(obj2)))
		   )
		{
		        /*
		         * At least one object will be written, make sure
		         * both have data.
		         */
			if (obj1 != NULL)
			{
		            mfobj_set_flags((obj_get_module(obj1)), 
					CGenFlagIsReferenced);
			}
			if (obj2 != NULL)
			{
		            mfobj_set_flags((obj_get_module(obj2)), 
			   		 CGenFlagIsReferenced);
			}
		}

		/*
		 * to/from of cross-module conns must be available to write
		 * main
		 */
		if (   obj_get_write_me(project)
		    && obj_is_defined(obj)
		    && obj_is_cross_module(obj))
		{
			if ((obj1 != NULL) && obj_is_defined(obj1))
			{
		            mfobj_set_flags((obj_get_module(obj1)), 
					CGenFlagIsReferenced);
			}
			if ((obj2 != NULL) && obj_is_defined(obj2))
			{
		            mfobj_set_flags((obj_get_module(obj2)), 
			   		 CGenFlagIsReferenced);
			}
		}
	    break;
	    }
        }
        trav_close(&allTrav);
    }

    return 0;
}


static int
load_marked_modules(ABObj project)
{
    ABObj		module = NULL;
    AB_TRAVERSAL	moduleTrav;
    char	fileName[MAXPATHLEN+1] = "";

    for (trav_open(&moduleTrav, project, AB_TRAV_MODULES|AB_TRAV_MOD_SAFE);
	(module = trav_next(&moduleTrav)) != NULL; )
    {
	if (   mfobj_has_flags(module, CGenFlagLoadMe) 
	    && (!obj_is_defined(module)))
	{
	    load_module(module);
	}
    }
    trav_close(&moduleTrav);

    return 0;
}


static int
load_module(ABObj module)
{
    char	fileName[MAXPATHLEN+1];
    ABObj	newProject = NULL;
    ABObj	project = obj_get_project(module);
    *fileName = 0;

	if (obj_get_file(module) != NULL)
	{
	    strcpy(fileName, obj_get_file(module));
	}
	else
	{
	    strcpy(fileName, obj_get_name(module));
	    if (!util_file_name_has_gil_extension(fileName))
	    {
	        strcat(fileName, ".G");
	    }
	}

	fflush(stdout); fsync(fileno(stdout));
	newProject = gil_load_file(fileName, NULL, project);
	if (newProject == NULL)
	{
	    /* a failure ocurred. The error has already been reported */
	    util_printf_err(
	        catgets(Dtb_project_catd, 1, 86,
			"%s: exiting due to error loading file %s.\n"),
		util_get_program_name(), fileName);
	    exit(1);
	}
	if (!obj_is_project(newProject))
	{
	    /* interface was returned */
	    newProject = project;
	}
	if (newProject != project)
	{
	    util_printf_err(
	        catgets(Dtb_project_catd, 1, 10,
	             "Unexpected project in file %s. Aborting\n"), fileName);
	    exit(1);
	}
	if (!obj_is_defined(module))
	{
	    char *module_name_string = obj_get_name(module);
    
	    util_printf_err(
		    catgets(Dtb_project_catd, 1, 11,
		        "Module %s not found in file %s. Aborting\n"),
		        module_name_string, fileName);
	    exit(1);
	}

    return 0;
}


/*
 * Print usage message
 */
static int
gilcvt_usage(void)
{
    char *program_name_string = util_get_program_name();

    fprintf(stderr,"\n");
    fprintf(stderr,
       catgets(Dtb_project_catd, 1, 12,
"Usage: %s [options] [project-file] [interface-file [interface-file] ...]\n\n"),
	program_name_string);

    fprintf(stderr, catgets(Dtb_project_catd, 1, 13,
"A .bil file is generated for each interface specified on the command line,\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 14,
"or for all interfaces in the project, if no interfaces are specified. If no\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 15,
"project file is specified, a project file containing the specified interface(s)\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 16,
"is searched for in the current directory.\n\n"));

    fprintf(stderr, catgets(Dtb_project_catd, 1, 17,
"Files with extension .P are assumend to be GIL project files, and files\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 18,
"a .G extenstion are assumed to be GIL interface files.\n"));

    fprintf(stderr, catgets(Dtb_project_catd, 1, 20,
"Options:\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 21,
"  -help (-h)        Print out this help message\n"));

/* CRT#00610
 * [ dunn 9 June, 1995 ]
 * The message catalog entry for this line is completely incorrect, and it
 * causes this line to print out a description of -nomerge, which is not
 * supported by sdtgilcvt. We're too close to FCS to fix it now, so we'll
 * just eliminate this line, altogether. An identical problem exists in
 * dtcodegen.

    fprintf(stderr, catgets(Dtb_project_catd, 1, 26,
"* -project (-p)     Specify a project to generate code for\n"));

 */

    fprintf(stderr, catgets(Dtb_project_catd, 1, 27,
"  -noproject (-np)  Use default project settings, ignore project file\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 30,
"  -silent (-s)      Silent mode, no messages written\n"));
    fprintf(stderr, catgets(Dtb_project_catd, 1, 31,
"  -verbose (-v)     Verbose mode, detailed progress messages\n\n"));


/* "-a (-ansi)      Write ANSI C code\n" */
/* "-k (-kandr)     Write K&R C code, no function prototypes\n" */
/* "-r (-resources) Write all resources into a resource file\n" */

    exit(1);
    return ERR_INTERNAL;
}


/*
 * Searches the current directory for a project file
 * Sets the cmdline->proj_file member
 *
 * Returns the total number of project files found
 *    
 */
static int
find_proj_file(CmdlineArgs cmdline)
{
    DIR                 *dir= NULL;
    struct dirent       *dirEntry= NULL;
    int			numProjFiles = 0;
    StringListRec	projFiles;
    strlist_construct(&projFiles);

    dir= opendir(".");
    if (dir == NULL)
    {
        perror(catgets(Dtb_project_catd,1,32,"Couldn't open '.'"));
        return -1;
    }

    while ((dirEntry= readdir(dir)) != NULL)
    {
        if (util_file_name_has_extension(dirEntry->d_name, "P"))
        {
            /* found a project file! */
	    strlist_add_str(&projFiles, dirEntry->d_name, NULL);
        }
    }

    numProjFiles = strlist_get_num_strs(&projFiles);
    if (numProjFiles > 0)
    {
	int	num_strings = strlist_get_num_strs(&projFiles);
	int	i = 0;
	STRING	fileName = NULL;

	for (i = 0; i < strlist_get_num_strs(&projFiles); ++i)
	{
	    fileName = strlist_get_str(&projFiles, i, NULL);
	    if (!gil_proj_file_has_modules(fileName, cmdline->files))
	    {
		strlist_remove_index(&projFiles, i);
		--i;
	    }
	}
    }

    if (strlist_get_num_strs(&projFiles) == 1)
    {
	cmdline->proj_file = istr_create(strlist_get_str(&projFiles, 0, NULL));
    }
    else if (strlist_get_num_strs(&projFiles) > 1)
    {
	int	i = 0;
	char    *prog_name_string = util_get_program_name();

	fprintf(stderr, catgets(Dtb_project_catd, 1, 33,
	    "%s: Please specify project file (e.g."), prog_name_string);
  	for (i = 0; i < strlist_get_num_strs(&projFiles); ++i)
	{
	    fprintf(stderr, ", %s", strlist_get_str(&projFiles, i, NULL));
	}
	fprintf(stderr, catgets(Dtb_project_catd, 1, 34, ")\n"));
	if (!strlist_is_empty(cmdline->files))
	{
	    fprintf(stderr,
		catgets(Dtb_project_catd, 1, 35,
    		"%s: More than one project contains specified module(s)\n"),
		prog_name_string);
	}
    }

    closedir(dir); dir= NULL;

    strlist_destruct(&projFiles);
    return numProjFiles;
}


static BOOL
gil_proj_file_has_modules(STRING projFileName, StringList moduleFileNames)
{
    BOOL	hasModules = FALSE;
    char	word[1024] = "";
    int		modulesFound = 0;
    int		wordLen = 0;
    File	file = NULL;
    int		c = 0;
    fpos_t	fpos;
    int		numFiles = strlist_get_num_strs(moduleFileNames);

    if (numFiles < 1)
    {
	return TRUE;
    }

    file = util_fopen_locked(projFileName, "r");
    if (file == NULL)
    {
	perror(projFileName);
	return FALSE;
    }

    while ((c = fgetc(file)) != EOF)
    {
	if (c == ':')
	{
	    fgetpos(file, &fpos);
	    *word = 0;
	    if (   (fgets(word, 11, file) != NULL) 
		&& (strncmp(word, "interfaces", 10) == 0))
	    {
		/* :files <file-list> */
		BOOL	filesDone = FALSE;
		while (((c = fgetc(file)) != EOF) && (c != '(')) {}
		filesDone = (c == EOF);
		while (!filesDone)
		{
		    while (((c = fgetc(file)) != EOF) && isspace(c)) {}
		    ungetc(c, file);
		    *word = 0;
		    wordLen = 0;
		    while (((c = fgetc(file)) != EOF) && (!isspace(c)))
		    {
			if (c == ')')
			{
			    filesDone = TRUE;
			    break;
			}
			word[wordLen++] = c;
			word[wordLen] = 0;
		    }
		    if (c == EOF)
		    {
			filesDone = TRUE;
		    }

		    if (*word != 0)
		    {
			int	i = 0;
			ISTRING	temp_istr = istr_create(word);
			for (i = 0; i < numFiles; ++i)
			{
			    if (istr_equal(temp_istr, 
				    strlist_get_istr(moduleFileNames, i, NULL)))
			    {
				++modulesFound;
				break;
			    }
			}
			istr_destroy(temp_istr);
		    }
		}
	    }
	    else if (   (strncmp(word, "modul", 5) == 0)
		     && (fgets(word, 2, file) != NULL)
		     && (strncmp(word, "e", 1) == 0) )
	    {
		/* :module <module-name */
		/* REMIND: we need to have this for .bix files to work! */
	    }
	    else
	    {
		fsetpos(file, &fpos);
	    }
	} /* c == ':' */
    } /* while c != EOF */

    util_fclose(file);
    hasModules = (modulesFound >= numFiles);

    return hasModules;
}


static BOOL
write_required(ABObj tree)
{
    BOOL        writeSomething= FALSE;

    if (obj_get_write_me(tree))
    {
        writeSomething= TRUE;
    }
    else
    {
        AB_TRAVERSAL    trav;
        ABObj           module= NULL;

        for(trav_open(&trav, tree, AB_TRAV_MODULES);
            (module= trav_next(&trav)) != NULL; )
        {
            if (obj_get_write_me(module))
            {
                writeSomething= TRUE;
                break;
            }
        }
        trav_close(&trav);
    }

    return writeSomething;
}


/*
 * Initializes all subsytems used by gilcvt
 */
static int
gilcvt_init(void)
{
    gil_init();
    return 0;
}


/*
 * Replaces all occurences of substr with replacestr. 
 * replaceStr *must* be shorter than or the same length as subStr
 */
static int
replace_string_shorter(STRING buf, STRING subStr, STRING replaceStr)
{
    int		numReplaced = 0;
    char	*bufPtr = buf;
    char	*bufEnd = buf + strlen(buf) - 1;
    int		subStrLen = strlen(subStr);
    char	*subStrPtr = subStr;
    int		replaceStrLen = strlen(replaceStr);
    int		replaceDiffLen = subStrLen - replaceStrLen;

    util_dassert(1, (((int)strlen(subStr)) >= ((int)strlen(replaceStr))));

    while ((subStrPtr = strstr(bufPtr, subStr)) != NULL)
    {
	memmove(subStrPtr+replaceStrLen, 
		subStrPtr+subStrLen,
		((int)(bufEnd - (subStrPtr+replaceStrLen))) + 1);
	strncpy(subStrPtr, replaceStr, replaceStrLen);	/* no NULL! */
	bufPtr = subStrPtr+replaceStrLen;
	bufEnd -= replaceDiffLen;
	++numReplaced;
    }
    return numReplaced;
}


#ifdef DEBUG

static int
examine_tree_debug(ABObj root)
{
    int			return_value = 0;
    int			rc = 0;
    AB_TRAVERSAL	trav;
    ABObj		obj = NULL;
    int			errCount = 0;
    char		objName[MAXPATHLEN+1];

    if (!debugging())
    {
	return 0;
    }

    util_dprintf(1, "Examining tree for inconsisties.\n");
    if ((rc = obj_tree_verify(root)) < 0)
    {
	util_dprintf(1, "CORRUPT TREE DETECTED. %s:%d\n", __FILE__, __LINE__);
	++errCount;
	goto epilogue;
    }

    for (trav_open(&trav, root, AB_TRAV_ALL | AB_TRAV_MOD_PARENTS_FIRST);
	(obj = trav_next(&trav)) != NULL; )
    {
	if (!obj_is_defined(obj))
	{
	    util_dprintf(1, "UNDEFINED: %s\n", 
		obj_get_safe_name(obj, objName, MAXPATHLEN+1));
	    ++errCount;
	}
	if (   (obj_get_type(obj) == AB_TYPE_UNDEF)
	    || (obj_get_type(obj) == AB_TYPE_UNKNOWN) )
   	{
	    util_dprintf(1, " BAD TYPE: %s\n", 
		obj_get_safe_name(obj, objName, MAXPATHLEN+1));
	    ++errCount;
	}
    }
    trav_close(&trav);

    if (errCount == 0)
    {
	util_dprintf(1, "Debug verify tree: OK!\n");
    }

epilogue:
    if ((return_value >= 0) && (errCount > 0))
    {
	return_value = -1;
    }
    if (debugging() && (return_value < 0))
    {
	util_dprintf(1,
		"CORRUPT TREE DETECTED %s:%d\n", __FILE__, __LINE__);
	    util_dabort(1);
    }
    return return_value;
}

#endif /* DEBUG */


/* 
 *
 */
static int
examine_tree(ABObj project)
{
    int                 returnValue = 0;
    AB_TRAVERSAL        allTrav;
    ABObj               obj = NULL;
    int			debugI = 0;

    util_dprintf(1, "Manipulating tree to make a valid BIL file\n");

    /* objxm_obj_configure(project, OBJXM_CONFIG_CODEGEN, TRUE); */
    munge_set_bil_file_name(project);

    for (trav_open(&allTrav, project, 
		AB_TRAV_ALL | AB_TRAV_MOD_PARENTS_FIRST | AB_TRAV_MOD_SAFE);
        (obj = trav_next(&allTrav)) != NULL; )
    {
	++debugI;
	/*
	 * We need to check to see if the object has been destroyed, after
	 * each "munging." If it has, it will belong to no project.
	 */
	if (obj_get_project(obj) == NULL) {continue;}
        munge_set_bil_file_name(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_ensure_win_parent(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_ensure_defined(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_move_action(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_check_glyph_files(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_check_layered_pane(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_group_geometry(obj);

	if (obj_get_project(obj) == NULL) {continue;}
	munge_ensure_item_selected(obj);

    } /* trav modules */
    trav_close(&allTrav);

    if (obj_get_root_window(project) == NULL)
    {
	obj_set_root_window(project, gilcvtP_get_root_window(project));
    }

    gilcvtP_prepare_tree(project);	/* creates extra data for each obj */

    return returnValue;
}


static int
munge_set_bil_file_name(ABObj obj)
{
    ABObj	fileNameObj = NULL;
    char	fileName[MAXPATHLEN+1];
    int		fileNameLen = 0;
    *fileName = 0;
    
    if ((fileNameObj = obj_get_module(obj)) == NULL)
    {
	fileNameObj = obj_get_project(obj);
	if (fileNameObj == NULL)
	{
	    return -1;
	}
    }
    if (obj_get_file(fileNameObj) == NULL)
    {
	return -1;
    }

    util_strncpy(fileName, obj_get_file(fileNameObj), MAXPATHLEN+1);
    fileNameLen = strlen(fileName);
    if (   (fileNameLen >= 2)
        && (   (util_streq(&(fileName[fileNameLen-2]), ".P"))
            || (util_streq(&(fileName[fileNameLen-2]), ".G"))))
    {
        fileName[fileNameLen-2] = 0;
    }
    if (obj_is_project(fileNameObj))
    {
	abio_expand_bil_proj_path(fileName);
    }
    else
    {
	abio_expand_bil_module_path(fileName);
    }

    return obj_set_file(obj, fileName);
}


/*
 * Groups have their x,y values set relative to their non-group parents.
 * For ABObj trees, the x,y values should be relative to the group. Since
 * we don't know the actual position of the group, itself, we'll just
 * move all the objects so that they butt up against 0,0, keeping their
 * relative positions to each other.
 */
static int
munge_group_geometry(ABObj obj)
{
    if (!obj_is_group(obj))
    {
	return 0;
    }
    {
	AB_TRAVERSAL	memberTrav;
	ABObj		member = NULL;
	int		xMin = INT_MAX;
	int		yMin = INT_MAX;

	for (trav_open(&memberTrav, obj, AB_TRAV_CHILDREN|AB_TRAV_MOD_SAFE);
		(member = trav_next(&memberTrav)) != NULL; )
	{
	    if (!obj_is_ui(member))
	    {
		continue;
	    }
	    xMin = util_min(xMin, obj_get_x(member));
	    yMin = util_min(yMin, obj_get_y(member));
	}
	/* don't close traversal, yet */

	if (xMin == INT_MAX) xMin = 0;
	if (yMin == INT_MAX) yMin = 0;

	for (trav_reset(&memberTrav); 
		(member = trav_next(&memberTrav)) != NULL; )
        {
	    if (!obj_is_ui(member))
	    {
		continue;
	    }
	    obj_move(member, obj_get_x(member)-xMin, obj_get_y(member)-yMin);
	}
	trav_close(&memberTrav);
    }

    return 0;
}


/*
 * Some types of objects require that an item be initially selected. GIL
 * files will often not specify which this is and default to the first item.
 * We're going to force the first item to be selected (if none are already).
 */
static int
munge_ensure_item_selected(ABObj obj)
{
    BOOL		selectionRequired = FALSE;

    switch (obj_get_type(obj))
    {
    case AB_TYPE_CHOICE:
    {
        switch (obj_get_choice_type(obj))
        {
	case AB_CHOICE_EXCLUSIVE:
	    selectionRequired = TRUE;
	break;

	case AB_CHOICE_NONEXCLUSIVE:
	    selectionRequired = obj_get_selection_required(obj);
	break;

	case AB_CHOICE_OPTION_MENU:
	    selectionRequired = TRUE;
	break;
        }
    }
    break;

    case AB_TYPE_LIST:
	selectionRequired = obj_get_selection_required(obj);
    break;
    } /* switch obj_get_type() */

    if (selectionRequired)
    {
	AB_TRAVERSAL	trav;
	ABObj		item = NULL;
	ABObj		firstItem = NULL;
	ABObj		selectedItem = NULL;

	for (trav_open(&trav, obj, AB_TRAV_ITEMS);
		(item = trav_next(&trav)) != NULL; )
	{
	    if (firstItem == NULL)
	    {
		firstItem = item;
	    }
	    if (obj_is_initially_selected(item))
	    {
		selectedItem = item;
		break;
	    }
	}
	trav_close(&trav);

	if ((selectedItem == NULL) && (firstItem != NULL))
	{
	    obj_set_is_initially_selected(firstItem, TRUE);
	}
    }
    return 0;
}

/*
 * Checks windows and messages for proper parents
 */
static int
munge_ensure_win_parent(ABObj win)
{
    ABObj	project = NULL;
    ABObj	module = NULL;
    ABObj	root_window = NULL;
    ABObj	win_parent = NULL;
    ABObj	parent = NULL;

    if (! (obj_is_window(win) || obj_is_message(win)) )
    {
	return 0;
    }

    project = obj_get_project(win);
    module = obj_get_module(win);
    root_window = gilcvtP_get_root_window(obj_get_project(win));
    win_parent = obj_get_win_parent(win);
    parent = obj_get_parent(win);

    if (obj_is_window(win))
    {
	/* messages don't have win-parent attribute */
        /* all windows are win-parented to the root window */
        if ((win != root_window) && (win_parent != root_window))
        {
	    obj_set_win_parent(win, root_window);
        }
    }

    /* all windows are parented by a module */
    if (!obj_is_module(parent))
    {
	obj_reparent(win, module);
    }

    return 0;
}


/*
 * GIL specifies some objects (e.g., :owner) using a simple name, even
 * though the object is in another interface. Look for undefined objects
 * in one module and try to resolve them to objects in another module.
 */
static int
munge_ensure_defined(ABObj obj)
{
    ABObj	actualObj = NULL;
    BOOL	hasNonWindowChildren = FALSE;
    STRING	objName = obj_get_name(obj);
    ABObj	foundObj = NULL;
    ABObj	scopeObj = NULL;
    ABObj	module = obj_get_module(obj);
    ABObj	project = obj_get_project(obj);
    AB_TRAVERSAL	trav;
    ABObj		child = NULL;

    if (obj_is_defined(obj))
    {
	return 0;
    }

    if (   (actualObj == NULL)
	&& (module != NULL)
	&& ((foundObj = obj_find_by_name(module, objName)) != NULL)
	&& (foundObj != obj) )
    {
	actualObj = foundObj;
    }

    if (   (actualObj == NULL)
	&& (project != NULL)
	&& ((foundObj = obj_find_by_name(project, objName)) != NULL)
	&& (foundObj != obj) )
    {
	actualObj = foundObj;
    }

    if ((actualObj == NULL) && (project != NULL))
    {
	AB_TRAVERSAL	trav;
	ABObj		scopeModule = NULL;
	for (trav_open(&trav, project, AB_TRAV_MODULES);
		(scopeModule = trav_next(&trav)) != NULL; )
	{
	    if (   ((foundObj = obj_find_by_name(scopeModule, objName)) != NULL)
	        && (foundObj != obj) )
	    {
		actualObj = foundObj;
		break;
	    }
	}
	trav_close(&trav);
    }

    hasNonWindowChildren = FALSE;
    if (obj_has_child(obj))
    {
	for (trav_open(&trav, obj, AB_TRAV_SALIENT_CHILDREN);
		(child = trav_next(&trav)) != NULL; )
	{
	    if (!obj_is_window(child))
	    {
		hasNonWindowChildren = TRUE;
		break;
	    }
	}
	trav_close(&trav);
    }

    if (actualObj == NULL)
    {
	util_printf_err(
	    "WARNING: destroying references to undefined object %s\n",
	    obj_get_name(obj));
	obj_destroy_one(obj);
    }
    else if (hasNonWindowChildren)
    {
	util_printf_err(
	    "WARNING: invalid cross-interface reference to object %s\n",
	    obj_get_name(obj));
    }
    else
    {
	for (trav_open(&trav, obj, AB_TRAV_SALIENT_CHILDREN);
		(child = trav_next(&trav)) != NULL; )
	{
	    util_dassert(1, (obj_is_window(child)));
	    obj_set_win_parent(child, actualObj);
	    util_dprintf(3,"set win parent %s <- %s\n",
		obj_get_name(child), obj_get_name(actualObj));
	}
	trav_close(&trav);
	obj_destroy_one(obj);	/* destroy undefined reference */
    }

    return 0;
}


static int
munge_move_action(ABObj action)
{
    ABObj	project = NULL;
    ABObj	module = NULL;

    if (!obj_is_action(action))
    {
	return 0;
    }

    project = obj_get_project(action);
    module = obj_get_module(action);

    if (obj_is_intra_module(action))
    {
	if (obj_get_module(action) == NULL)
	{
	    obj_add_action(module, action);
	}
    }
    else
    {
	if (obj_get_module(action) != NULL)
	{
	    obj_add_action(project, action);
	}
    }

    return 0;
}


static int
munge_check_geometry(ABObj obj)
{
    if (obj == NULL)
    {
	return -1;
    }

    switch (obj_get_type(obj))
    {
	case AB_TYPE_BUTTON:
	case AB_TYPE_LABEL:
	    obj_resize(obj, -1, -1);
	break;

	case AB_TYPE_TEXT_FIELD:
	    if (obj_get_num_columns(obj))
	    {
		/* size is set by # characters across, rather than pixels */
	        obj_resize(obj, -1, -1);
	    }
	break;

	case AB_TYPE_SPIN_BOX:
	{
	    /* spin boxes become HUGE if allowed free reign with their width.
	     * GIL used the width of the whole construct (label and all), but
	     * BIL only stores the width of the spinbox, itself.
	     */
	    int 	width = obj_get_width(obj);
	    if (width >= 0)
	    {
		width -= (util_strlen(obj_get_label(obj)) * 7);
		width += 12;		/* GIL did not count arrows */
		width = util_max(80, width);
	    }
	    obj_resize(obj, width, -1);
	}
	break;

	case AB_TYPE_CONTAINER:
	case AB_TYPE_BASE_WINDOW:
	case AB_TYPE_DIALOG:
	{
	    ABObj	parent = obj_get_parent(obj);
	    if (   (obj_get_child_cond(obj, 0, obj_is_ui) != NULL)
		&& (   (parent != NULL) 
		    && (obj_get_type(parent) != AB_TYPE_LAYERS))
	       )
	    {
		/* the object is a container with children - set it to
		 * "fit contents."
		 *
		 * don't allow layered panes' sizes to float, though - all
		 * panes must be exactly the same size
		 */
		obj_resize(obj, -1, -1);
	    }
	}
	break;
    }

    /*
     * Check for OpenWindows BG0,1,2, FG0,1,2,..., which should be translated
     * to the default color values.
     */
    if (   (obj_get_bg_color(obj) != NULL)
	&& (util_strncasecmp(obj_get_bg_color(obj), "bg", 2) == 0) )
    {
	obj_set_bg_color(obj, NULL); 
    }
    if (   (obj_get_fg_color(obj) != NULL)
	&& (util_strncasecmp(obj_get_fg_color(obj), "fg", 2) == 0) )
    {
	obj_set_fg_color(obj, NULL);
    }

    /*
     * Every object should have left and top attachments
     */
    if (obj_is_ui(obj) && (!obj_is_window(obj)))
    {
	if (obj_get_x(obj) != -1)
	{
	    obj_set_attachment(obj, AB_CP_WEST, AB_ATTACH_POINT, 
			(void*)obj_get_x(obj), 0);
	}
	if (obj_get_y(obj) != -1)
	{
	    obj_set_attachment(obj, AB_CP_NORTH, AB_ATTACH_POINT, 
			(void*)obj_get_y(obj), 0);
	}
    }

    return 0;
}


static int
munge_check_glyph_files(ABObj obj)
{
    char	oldFileName[MAXPATHLEN+1];
    int		oldFileNameLen = 0;
    *oldFileName = 0;

    if (obj_get_label_type(obj) != AB_LABEL_GLYPH)
    {
	return 0;
    }

    util_strncpy(oldFileName, obj_get_label(obj), MAXPATHLEN+1);
    oldFileNameLen = strlen(oldFileName);
    if (   (oldFileNameLen >= 3) 
	&& util_streq(&(oldFileName[oldFileNameLen-3]), ".pr"))
    {
        char	newFileName[MAXPATHLEN+1];
        char	newFileNameLen = 0;
        *newFileName = 0;

	newFileNameLen = oldFileNameLen - 3;
	util_strncpy(newFileName, oldFileName, newFileNameLen+1);
	obj_set_label(obj, newFileName);
	obj_set_name_from_label(obj, NULL);
	strcat(newFileName, ".bm");
	link_file(oldFileName, newFileName);
    }

    return 0;
}


/*
 * Makes sure that no more than one layer in a layers object is visible
 * sets the width and height of all panes to be equal.
 */
static int
munge_check_layered_pane(ABObj layeredPane)
{
    if ((layeredPane == NULL) || (obj_get_type(layeredPane) != AB_TYPE_LAYERS))
    {
	return 0;
    }

    {
	AB_TRAVERSAL	trav;
	ABObj		firstVisible = NULL;
	ABObj		pane = NULL;
	int		i = 0;
	int		paneWidth = 0;
	int		paneHeight = 0;

        for (trav_open(&trav, layeredPane, AB_TRAV_SALIENT_CHILDREN);
	    (pane = trav_next(&trav)) != NULL; )
 	{
	    if (obj_is_ui(pane))
	    {
		paneWidth = util_max(paneWidth, obj_get_width(pane));
		paneHeight= util_max(paneHeight, obj_get_height(pane));
		if (obj_is_initially_visible(pane))
		{
		    if (firstVisible == NULL)
		    {
		        firstVisible = pane;
		    }
		    else
		    {
			obj_set_is_initially_visible(pane, FALSE);
		    }
		}
	    }
	}
	/* don't close trav */

	if ((paneWidth > 0) && (paneHeight > 0))
	{
	    for (trav_reset(&trav); (pane = trav_next(&trav)) != NULL; )
	    {
	        if (obj_is_ui(pane))
	        {
		    obj_resize(pane, paneWidth, paneHeight);
	        }
	    }
	    trav_close(&trav);
        }

	trav_close(&trav);
    }

    return 0;
}


static int
link_file(STRING oldPath, STRING newPath)
{
    int		return_value = 0;
    int		rc = 0;			/* return code */
    char	*oldFileNamePtr = NULL;
    char	*slashPtr = NULL;

    oldFileNamePtr = oldPath;
    if ((slashPtr = strrchr(oldPath, '/')) != NULL)
    {
	/* use only simple file name - no directory info */
	oldFileNamePtr = slashPtr + 1;
	if ((*oldFileNamePtr) == 0)
	{
	    /* there is no file = only a directory */
	    oldFileNamePtr = oldPath;
	}
    }

    /*
     * Check for existing link
     */
    if (util_file_exists(newPath))
    {
	if (util_paths_are_same_file(oldPath, newPath))
	{
	    /* the link exists, already */
	    goto epilogue;
	}
	else
	{
	    /* the target path exists, and is not what we want */
	    return_value = -1;
	    goto epilogue;
	}
    }
    else
    {
	/* link may exist, but it is bogus */
	unlink(newPath);
    }

    /* make the link */
    if (util_be_verbose())
    {
        util_printf("linking %s -> %s\n", newPath, oldFileNamePtr);
    }
    if ((rc = util_symlink(oldFileNamePtr, newPath)) != 0)
    {
	return_value = -1;
    }

epilogue:
    return return_value;
}


#ifdef BOGUS
static int
munge_liberate_menu(ABObj menu)
{
    if (!obj_is_menu(menu))
    {
	return -1;
    }
    {
	ABObj		compRoot = obj_get_root(menu);
	AB_OBJECT_TYPE	compType = AB_TYPE_UNDEF;
	
	if (compRoot != NULL)
	{
	    compType = obj_get_type(compRoot);
	}
	if (   (compRoot != menu)
	    && (   (compType == AB_TYPE_ITEM)
	        || (compType == AB_TYPE_BUTTON)) )
	{
	    menu->part_of = NULL;
	}
    }

    return 0;
}
#endif /* BOGUS */


#ifdef BOGUS
static int
dup_all_menu_refs(ABObj project)
{
    AB_TRAVERSAL	moduleTrav;
    AB_TRAVERSAL	menuTrav;
    ABObj		module = NULL;
    ABObj		menu = NULL;

    for (trav_open(&moduleTrav, project, AB_TRAV_MODULES);
	(module = trav_next(&moduleTrav)) != NULL; )
    {
	if (!obj_get_write_me(module))
	{
	    continue;
	}
        for (trav_open(&menuTrav, module, AB_TRAV_MENUS);
	    (menu = trav_next(&menuTrav)) != NULL; )
        {
    	    if (obj_is_ref(menu))
	    {
		gilcvtP_dup_menu_ref_tree(menu);
#ifdef BOGUS
		if (obj_is_popup(menu))
		{
		   STRING title = obj_get_menu_title(obj_get_parent(menu));
		   if (!util_strempty(title))
		    	objxm_create_popup_menu_title(menu, OBJXM_CONFIG_CODEGEN,
				title);
		}
#endif /* BOGUS */
	    }
        }
        trav_close(&menuTrav);
    }
    trav_close(&moduleTrav);

    return 0;
}
#endif /* BOGUS */


#ifdef BOGUS
static int
gilcvtP_dup_menu_ref_tree(ABObj obj)
{
    int		rc = 0;		/* return code */
    int			return_value = 0;
    ABObj	refObj = NULL;
    AB_TRAVERSAL	trav;

    if (!obj_is_menu_ref(obj))
    {
	return 0;
    }
    rc = gilcvtP_do_dup_menu_ref_tree(obj);
    if (rc < 0)
    {
	return rc;
    }

    /*
     * We can't handle multi-layered references, so we are going to collapse
     * any reference lists to only reference the final (actual) object.
     */
    for (trav_open(&trav, obj, AB_TRAV_UI);
	(obj = trav_next(&trav)) != NULL; )
    {
	refObj = obj_get_actual_obj(obj);
	if (refObj != NULL)
	{
	    while (obj_is_ref(refObj))
	    {
	        refObj = obj_get_actual_obj(refObj);
	    }
	    obj_cvt_to_ref(obj, refObj);
	}
    }
    trav_close(&trav);
    return return_value;
}
#endif /* BOGUS */


#ifdef BOGUS
static int
gilcvtP_do_dup_menu_ref_tree(ABObj obj)
{
    int		return_value = 0;
    int		rc = 0;
    ABObj	actualMenu = NULL;
    if (!obj_is_menu_ref(obj))
    {
	return 0;
    }
    actualMenu = obj_get_actual_obj(obj);
    if (actualMenu == NULL)
    {
	return -1;
    }

    if (obj_get_num_items(obj) != obj_get_num_items(actualMenu))
    {
	ABObj dupTree = obj_tree_create_ref(actualMenu);
	if (dupTree == NULL)
	{
	    return_value = -1;
	    goto epilogue;
	}
	rc = obj_move_children(obj, dupTree);
	if (rc < 0)
	{
	    return_value = rc;
	}
	obj_destroy(dupTree);

	{
	    AB_TRAVERSAL	trav;
	    ABObj		descendant = NULL;

	    for (trav_open(&trav, obj, 
			AB_TRAV_UI|AB_TRAV_MOD_PARENTS_FIRST|AB_TRAV_MOD_SAFE);
		(descendant = trav_next(&trav)) != NULL; )
	    {
		if (descendant == obj)
		{
		    continue;
		}
		if (obj_is_menu_ref(descendant))
		{
		    gilcvtP_do_dup_menu_ref_tree(descendant);
		}
	    }
	    trav_close(&trav);
	}
    }
epilogue:
    return return_value;
}
#endif /* BOGUS */


static int
dump_tree(ABObj tree)
{
    int		return_value = 0;
    int		old_verbosity = util_get_verbosity();

    switch (old_verbosity)
    {
	case 0: /* FALLTHROUGH */
	case 1: util_set_verbosity(3);
	break;

	case 2: util_set_verbosity(4); 
	break;
    }

    print_tree(tree, 0);

epilogue:
    util_set_verbosity(old_verbosity);
    return return_value;
}


static int
print_tree(ABObj root, int indent)
{
    AB_TRAVERSAL	trav;
    ABObj		child = NULL;
    int			numPrinted = 0;
    int			travType = AB_TRAV_SALIENT_CHILDREN;

    if (util_get_verbosity() >= 4)
    {
	travType = AB_TRAV_CHILDREN;
    }

    if (   (travType == AB_TRAV_CHILDREN)
	|| (obj_is_salient(root)) )
    {
        obj_print_indented(root, indent, util_get_verbosity());
        ++numPrinted;
    }
    
    if (obj_is_module(root) && (!obj_get_write_me(root)))
    {
	goto epilogue;
    }
    for (trav_open(&trav, root, travType);
	(child = trav_next(&trav)) != NULL; )
    {
	numPrinted += print_tree(child, indent+4);
    }
    trav_close(&trav);

    /*
     * Look for non-salient children that need their children printed
     */
    if (travType == AB_TRAV_SALIENT_CHILDREN)
    {
	for (trav_open(&trav, root, AB_TRAV_CHILDREN);
	    (child = trav_next(&trav)) != NULL; )
	{
	    if ((!obj_is_salient(child)) && (obj_get_root(child) != root))
	    {
		numPrinted += print_tree(child, indent);
	    }
	}
    }

epilogue:
    return numPrinted;
}


#ifdef DEBUG
static int
time_traversal(ABObj root)
{
	int		return_value = 0;
	AB_TRAVERSAL	trav;
	ABObj		obj = NULL;
	int		i = 0;
	double		startTime = 0;
	double		endTime = 0;
	double		totalTravTime = 0;
	double		oneTravTime = 0;
	int		numTravs = 1000;
	struct tms	timeBuf;
	long		ticks_per_second = sysconf(_SC_CLK_TCK);
	int		oldVerbosity = util_get_verbosity();

	if (ticks_per_second <= 0)
	{
	    util_dprintf(0, "Couldn't get the value of _SC_CLK_TCK!\n");
	    return_code(ERR_INTERNAL);
	}
	
	util_set_verbosity(0);		/* no expensive error-checking */

	/*
	 *  all
	 */
	util_printf("Beginning trav test\n");
	startTime = (times(&timeBuf) *1.0) / ticks_per_second;
	for (i = 0; i < numTravs; ++i)
	{
	    for (trav_open(&trav, root, AB_TRAV_ALL);
		(obj = trav_next(&trav)) != NULL; )
	    {
	    }
	    trav_close(&trav);
	}
	endTime = (times(&timeBuf) *1.0) / ticks_per_second;
	util_printf("end of trav test\n");
	totalTravTime = (endTime - startTime);
	oneTravTime = totalTravTime/numTravs;
	util_printf("one ALL traversal time: %g\n", oneTravTime);

	/*
	 * salient
	 */
	startTime = (times(&timeBuf) *1.0) / ticks_per_second;
	for (i = 0; i < numTravs; ++i)
	{
	    for (trav_open(&trav, root, AB_TRAV_SALIENT);
		(obj = trav_next(&trav)) != NULL; )
	    {
	    }
	    trav_close(&trav);
	}
	endTime = (times(&timeBuf) *1.0) / ticks_per_second;
	util_printf("end of trav test\n");
	totalTravTime = (endTime - startTime);
	oneTravTime = totalTravTime/numTravs;
	util_printf("one SALIENT traversal time: %g\n", oneTravTime);

epilogue:
    util_set_verbosity(oldVerbosity);
    return return_value;
}
#endif /* DEBUG */


/*
 * Actually sets the tree up for code generation.
 */
static int
gilcvtP_prepare_tree(ABObj project)
{
    ABObj		module = NULL;
    AB_TRAVERSAL	trav;
    StringList		proj_callbacks = NULL;
    ABObj		obj = NULL;
    AB_TRAVERSAL	allTrav;
    ABObj		callbackScopeObj;
    STRING		funcName = NULL;
    ABObj		parent = NULL;

    /*
     * Create extra cgen data for each object
     */
    gilcvtP_create_obj_data_for_project(project);
    for(trav_open(&trav, project, AB_TRAV_MODULES);
	(module = trav_next(&trav)) != NULL; )
    {
	gilcvtP_create_obj_data_for_module(module);
    }
    trav_close(&trav);

    proj_callbacks = mfobj_get_proj_data(project)->callbacks;

    for (trav_open(&allTrav, project, 
                AB_TRAV_ALL | AB_TRAV_MOD_PARENTS_FIRST | AB_TRAV_MOD_SAFE);
	(obj = trav_next(&allTrav)) != NULL; )
    {
	module = obj_get_module(obj);

	if ((!(mfobj_get_proj_data(project)->has_ui_obj)) && obj_is_ui(obj))
	{
	    mfobj_get_proj_data(project)->has_ui_obj = TRUE;
	}
	if (obj_get_type(obj) == AB_TYPE_TERM_PANE)
	{
	    mfobj_get_proj_data(project)->has_terminal = TRUE;
	}
    } /* trav modules */
    trav_close(&allTrav);

    /*
     * Adjust for objects that change size in the translation
     * They all get BIGGER! Motif LIKES screen space!!
     */
    {
        ABObjList	adjustedList = objlist_create();
	int		numAdjusted = 0;
	int		objCount = 0;
	ABObj		obj = NULL;
	BOOL		adjustmentMade = FALSE;

        objlist_set_is_unique(adjustedList, TRUE);
        gilcvtP_tree_determine_new_geometry(
				project, adjustedList, &adjustmentMade);
	if (adjustmentMade)
	{
	    adjustmentMade = FALSE;
            gilcvtP_tree_adjust_geometry(
				project, adjustedList, &adjustmentMade);
   	}

	util_dprintf(1, "# of adjusted objects: %d\n",
		objlist_get_num_objs(adjustedList));

	numAdjusted = objlist_get_num_objs(adjustedList);
        for (objCount = 0; objCount < numAdjusted; ++objCount)
	{
	    obj = objlist_get_obj(adjustedList, objCount, NULL);

	    /* set undefined adjustments to 0 */
	    if (mfobj_data(obj)->adjustX == INT_UNDEF)
	        mfobj_data(obj)->adjustX = 0;
	    if (mfobj_data(obj)->adjustY == INT_UNDEF)
	        mfobj_data(obj)->adjustY = 0;
	    if (mfobj_data(obj)->adjustWidth == INT_UNDEF)
	        mfobj_data(obj)->adjustWidth = 0;
	    if (mfobj_data(obj)->adjustHeight == INT_UNDEF)
	        mfobj_data(obj)->adjustHeight = 0;

	    obj_set_geometry(obj, 
		obj_get_x(obj) + mfobj_data(obj)->adjustX,
		obj_get_y(obj) + mfobj_data(obj)->adjustY,
		obj_get_width(obj) + mfobj_data(obj)->adjustWidth,
		obj_get_height(obj) + mfobj_data(obj)->adjustHeight);

	    /*
	    if (debugging())
	    {
		char buf[1000];
	        util_dprintf(2, "%s(%d,%d,%d,%d) + (%d,%d,%d,%d)\n",
		    obj_get_safe_name(obj, buf, 1000),
		    obj_get_x(obj),
		    obj_get_y(obj),
		    obj_get_width(obj),
		    obj_get_height(obj),
		    mfobj_data(obj)->adjustX,
		    mfobj_data(obj)->adjustY,
		    mfobj_data(obj)->adjustWidth,
		    mfobj_data(obj)->adjustHeight);
	    }
	    */
	}

	objlist_destroy(adjustedList);
    }

    /*
     * We need to remove geometry info, now.
     */
    for (trav_open(&trav, project, AB_TRAV_UI);
	    (obj = trav_next(&trav)) != NULL; )
    {
	munge_check_geometry(obj);
    }
    trav_close(&trav);

    return 0;
}


/*
 * Adjust the sizes of each object, from the bottom up
 *
 * Does not actually changes the object geometry. Sets the adjustX,Y,Width,
 * Height fields in mfobj_data().
 */
static int
gilcvtP_tree_determine_new_geometry(
			ABObj		root,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
)
{
    int			return_value = 0;
    AB_TRAVERSAL	childTrav;
    ABObj		child = NULL;

    for (trav_open(&childTrav, root, AB_TRAV_CHILDREN);
	(child = trav_next(&childTrav)) != NULL; )
    {
	if (gilcvtP_tree_determine_new_geometry(
		child, adjustedList, adjustmentMadeOut) < 0)
	{
	    return_value = -1;
	    break;
	}
    }
    trav_close(&childTrav);

    gilcvtP_obj_determine_new_geometry(root, adjustedList, adjustmentMadeOut);

    return return_value;
}


static int
gilcvtP_obj_determine_new_geometry(
			ABObj 		obj, 
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
)
{
    int		returnValue = 0;
    int		x = obj_get_x(obj);
    int		y = obj_get_y(obj);
    int		width = obj_get_width(obj);
    int		height = obj_get_height(obj);
    int		deltaX = INT_UNDEF;
    int		deltaY = INT_UNDEF;
    int		deltaWidth = INT_UNDEF;
    int		deltaHeight = INT_UNDEF;

    /* if (debugging())
    {
	char	buf[1000];
        util_dprintf(2, "obj_determine_new_geo(%s)\n",
	    obj_get_safe_name(obj, buf, 1000));
    } */

    switch (obj_get_type(obj))
    {
	case AB_TYPE_CHOICE:
	{
	    switch (obj_get_choice_type(obj))
	    {
		case AB_CHOICE_OPTION_MENU:
		    /*deltaY = y/10;*/
		    deltaWidth = -8;
		    deltaHeight = 4;
		break;

		case AB_CHOICE_EXCLUSIVE:
		case AB_CHOICE_NONEXCLUSIVE:
		{
		    int		numItems = obj_get_num_items(obj);
		    int		numAcross = 0;
		    int 	numDown = 0;

		    /* in OL, exclusive items are packed right up against each
		     * other.
		     */
		    if (obj_get_orientation(obj) == AB_ORIENT_VERTICAL)
		    {
		        /* this is objects laid out horizontally... */
		        numAcross = obj_get_num_columns(obj);
		        if (numAcross >= 1)
		        {
			    numAcross = util_min(numAcross, obj_get_num_items(obj));
		        }
		        else
		        {
			    numAcross = obj_get_num_items(obj);
		        }
		        if (numItems > 0)
		        {
			    numDown = ((numItems-1)/numAcross) + 1;
		        }
		    }
		    else if (obj_get_orientation(obj) == AB_ORIENT_HORIZONTAL)
		    {
			numDown = obj_get_num_rows(obj);
			if (numDown >= 1)
			{
			    numDown = util_min(numDown, numItems);
			}
			else
			{
			    numDown = numItems;
			}
			if (numItems > 0)
			{
			    numAcross = ((numItems-1)/numDown)+1;
			}
		    }
		    else
		    {
		        abort();
		    }

	    if (debugging())
	    {
		char	buf[1000];
	        util_dprintf(1, "%s - orientation:%s   across:%d down:%d\n",
			obj_get_safe_name(obj, buf, 1000),
			(obj_get_orientation(obj) == AB_ORIENT_HORIZONTAL?
			    "horiz"
			    :
			    (obj_get_orientation(obj) == AB_ORIENT_VERTICAL?
				"vertical"
				:
				"?????")),
			numAcross,
			numDown);
	    }

		    if (obj_get_choice_type(obj) == AB_CHOICE_EXCLUSIVE)
		    {
			/* exclusives in OL were crammed together */
		        deltaWidth = 15 * numAcross;
		        deltaHeight = 2 * numDown;
		    }
		    else if (obj_get_choice_type(obj) == AB_CHOICE_NONEXCLUSIVE)
		    {
			/* nonexclusives in OL had space between options */
		        deltaWidth = 0;
		        deltaHeight = 5 * numDown;
		    }
		    else
		    {
			abort();
		    }
		}
		break;
	    } /* switch obj_get_choice_type(obj) */
	}
	break;

	case AB_TYPE_BUTTON:
	    switch (obj_get_button_type(obj))
	    {
		case AB_BUT_MENU:
		    deltaWidth = 0;
		    deltaHeight = 2;
		break;
	    }
	break;

	case AB_TYPE_CONTAINER:
	    deltaWidth = 2;
	    deltaHeight = 2;
	break;

	case AB_TYPE_SPIN_BOX:
	    deltaHeight = 9;
	break;

	case AB_TYPE_TEXT_FIELD:
	    /* add border */
	    deltaWidth = 6;
	    deltaHeight = 4;
	break;
    } /* switch obj_get_type() */

    /*
     * If the change resolved to less than one pixel, make it one pixel.
     */
    if (deltaX != INT_UNDEF)
    {
	deltaX = util_max(1, deltaX);
    }
    if (deltaY != INT_UNDEF)
    {
	deltaY = util_max(1, deltaY);
    }
    if (deltaWidth != INT_UNDEF)
    {
	deltaWidth = util_max(1, deltaWidth);
    }
    if (deltaHeight != INT_UNDEF)
    {
	deltaHeight = util_max(1, deltaHeight);
    }


    if (debugging())
    {
	char name[1000];
        util_dprintf(1, "%s + (%d,%d,%d,%d)\n",
		obj_get_safe_name(obj, name, 1000),
		deltaX, deltaY, deltaWidth, deltaHeight);
    }
    if (   (deltaX != INT_UNDEF)
	|| (deltaY != INT_UNDEF)
	|| (deltaWidth != INT_UNDEF)
	|| (deltaHeight != INT_UNDEF) )
    {
	gilcvtP_obj_adjust_geometry(obj, 
		deltaX, deltaY, deltaWidth, deltaHeight,
		TRUE, FALSE, adjustedList,
		adjustmentMadeOut);
    }

epilogue:
    return returnValue;
}


static int
gilcvtP_tree_adjust_geometry(
			ABObj		tree,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
)
{
    AB_TRAVERSAL	trav;
    ABObj		obj = NULL;
    int			i = 0;
    BOOL		adjustmentMade = TRUE;
    char		busyChars[] = "|/-\\";

    (*adjustmentMadeOut) = FALSE;
    if ((util_be_verbose()) && (debug_level() < 2))
    {
        printf("adjusting geometry... "); fflush(stdout);
    }
    for (i = 0; (adjustmentMade) && (i < 10); ++i)
    {
	if ((util_be_verbose()) && (debug_level() < 2))
	{
	    printf("\b%c", busyChars[i%4]); fflush(stdout);
	}
	util_dprintf(2, "********** TOP OF LOOP (%d) **********\n", i);

	adjustmentMade = FALSE;
        for (trav_open(&trav, tree, AB_TRAV_UI);
	    (obj = trav_next(&trav)) != NULL; )
        {
	    gilcvtP_obj_adjust_geometry(obj,
		    INT_UNDEF, INT_UNDEF, INT_UNDEF, INT_UNDEF,
		    TRUE, TRUE, adjustedList, &adjustmentMade);
        }
        trav_close(&trav);

	if (adjustmentMade)
	{
	    (*adjustmentMadeOut) = TRUE;
	}
    }
    printf("\n");

    return 0;
}


static int
gilcvtP_obj_adjust_geometry(
			ABObj		obj,
			int		deltaX,
			int		deltaY,
			int		deltaWidth,
			int		deltaHeight,
			BOOL		deltaIsFromOrg,
			BOOL		adjustOthers,
			ABObjList	adjustedList,
			BOOL		*adjustmentMadeOut
)
{
#define ALIGN_THRESH	(5)
    int			orgX = obj_get_x(obj);
    int			orgY = obj_get_y(obj);
    int			orgRight = obj_get_x(obj) + obj_get_width(obj);
    int			orgBottom = obj_get_y(obj) + obj_get_height(obj);
    int			adjustRight = 0;
    int			adjustBottom = 0;

    /* delta delta values */
    int			ddX = 0;
    int			ddY = 0;
    int			ddWidth = 0;
    int			ddHeight = 0;
    int			ddRight = 0;
    int			ddBottom = 0;
    AB_TRAVERSAL	trav;
    ABObj		sibling = NULL;
    int			siblingRightMax = 0;
    int			siblingBottomMax = 0;

    if (deltaX != INT_UNDEF)
    {
	if (mfobj_data(obj)->adjustX == INT_UNDEF)
	{
	    mfobj_data(obj)->adjustX = 0;
	    ddX = deltaX;
	}
	else
	{
	    if (deltaIsFromOrg)
	    {
	        ddX = util_max(0, deltaX - mfobj_data(obj)->adjustX);
	    }
	    else
	    {
		ddX = deltaX;
	    }
	}
    }
    if (deltaY != INT_UNDEF)
    {
	if (mfobj_data(obj)->adjustY == INT_UNDEF)
	{
	    mfobj_data(obj)->adjustY = 0;
	    ddY = deltaY;
	}
	else
	{
	    if (deltaIsFromOrg)
	    {
	        ddY = util_max(0, deltaY - mfobj_data(obj)->adjustY);
	    }
	    else
	    {
		ddY = deltaY;
	    }
	}
    }
    if (deltaWidth != INT_UNDEF)
    {
	if (mfobj_data(obj)->adjustWidth == INT_UNDEF)
	{
	    mfobj_data(obj)->adjustWidth = 0;
	    ddWidth = deltaWidth;
	}
	else
	{
	    if (deltaIsFromOrg)
	    {
	        ddWidth = 
		    util_max(0, deltaWidth - mfobj_data(obj)->adjustWidth);
	    }
	    else
	    {
		ddWidth = deltaWidth;
	    }
	}
    }
    if (deltaHeight != INT_UNDEF)
    {
	if (mfobj_data(obj)->adjustHeight == INT_UNDEF)
	{
	    mfobj_data(obj)->adjustHeight = 0;
	    ddHeight = deltaHeight;
	}
	else
	{
	    if (deltaIsFromOrg)
	    {
	        ddHeight = 
		    util_max(0, deltaHeight - mfobj_data(obj)->adjustHeight);
	    }
	    else
	    {
		ddHeight = deltaHeight;
	    }
	}
    }

    ddRight = ddX + ddWidth;
    ddBottom = ddY + ddHeight;

    /*if (debugging())
    {
	char	buf[1000];
        util_dprintf(2, "obj_adjust_geometry(%s, %s:[%d,%d,%d,%d])\n",
	    obj_get_safe_name(obj, buf, 1000),
	    (deltaIsFromOrg? "abs":"rel"),
	    deltaX, deltaY, deltaWidth, deltaHeight);
        if (   util_streq(obj_get_name(obj), "message9")
	    || util_streq(obj_get_name(obj), "message5")
	    || util_streq(obj_get_name(obj), "col_bg_def_set")
	   )
        {
	    util_dprintf(2,"yeah, so what?\n");
        }
    }*/
    mfobj_data(obj)->adjustX += ddX;
    mfobj_data(obj)->adjustY += ddY;
    mfobj_data(obj)->adjustWidth += ddWidth;
    mfobj_data(obj)->adjustHeight += ddHeight;

    if ((ddX | ddY | ddWidth | ddHeight) != 0)
    {
        (*adjustmentMadeOut) = TRUE;
    }

    /* must have new adjust* values to calculate these */
    adjustRight = mfobj_data(obj)->adjustX + mfobj_data(obj)->adjustWidth;
    adjustBottom = mfobj_data(obj)->adjustY + mfobj_data(obj)->adjustHeight;

    if (   (adjustedList != NULL)
	&& ((ddX | ddY | ddWidth | ddHeight) != 0) )
    {
        objlist_add_obj(adjustedList, obj, NULL);
    }

    /*
     * Adjust our siblings that are located below or to the right
     */
    if (adjustOthers)
    {
	int	siblingDeltaX = INT_UNDEF;
	int	siblingDeltaY = INT_UNDEF;
	int	siblingRight = 0;
	int	siblingBottom = 0;
	int	dist = 0;

	siblingRightMax = 0;
	siblingBottomMax = 0;

	/*
	 * REMIND: AB_TRAV_SIBLINGS does *not* work!
	 */
        for (trav_open(&trav, obj_get_parent(obj), AB_TRAV_CHILDREN);
	    (sibling = trav_next(&trav)) != NULL; )
        {
	    /*
	     * Find max right,bottom
	     */
	    siblingRight = obj_get_x(sibling) + obj_get_width(sibling);
	    if (mfobj_data(sibling)->adjustX != INT_UNDEF)
	    {
	        siblingRight += mfobj_data(sibling)->adjustX;
	    }
	    if (mfobj_data(sibling)->adjustWidth != INT_UNDEF)
	    {
	        siblingRight += mfobj_data(sibling)->adjustWidth;
	    }
	    siblingRightMax = util_max(siblingRightMax, siblingRight);
    
	    siblingBottom = obj_get_y(sibling) + obj_get_height(sibling);
	    if (mfobj_data(sibling)->adjustY != INT_UNDEF)
	    {
	        siblingBottom += mfobj_data(sibling)->adjustY;
	    }
	    if (mfobj_data(sibling)->adjustHeight != INT_UNDEF)
	    {
	        siblingBottom += mfobj_data(sibling)->adjustHeight;
	    }
	    siblingBottomMax = util_max(siblingBottomMax, siblingBottom);

	    if ((sibling == obj) || (!obj_is_ui(sibling)))
	    {
	       continue;
	    }
    
	    siblingDeltaX = INT_UNDEF;
	    siblingDeltaY = INT_UNDEF;
	    dist = 0;

	    if (obj_get_width(obj) > ALIGN_THRESH)
	    {
	        if ((dist = (obj_get_x(sibling) - orgRight)) >= -ALIGN_THRESH)
	        {
		    /* sibling to right of obj: dist is positive */
		    siblingDeltaX = 
		        (orgRight + adjustRight + dist) - obj_get_x(sibling);
		}
	    }

	    if (obj_get_height(obj) > ALIGN_THRESH)
	    {
	        if ((dist = (obj_get_y(sibling) - orgBottom)) >= -ALIGN_THRESH)
	        {
		    /* sibling below obj: dist is positive */
		    siblingDeltaY =
		        (orgBottom + adjustBottom + dist) - obj_get_y(sibling);
	        }
	    }

	    if (   (siblingDeltaX != INT_UNDEF)
		|| (siblingDeltaY != INT_UNDEF) )
	    {
	        gilcvtP_obj_adjust_geometry(sibling,
			siblingDeltaX,
		        siblingDeltaY, 
		        INT_UNDEF,
		        INT_UNDEF,
			TRUE,
		        FALSE,
		        adjustedList,
			adjustmentMadeOut);

	    }
        }
        trav_close(&trav);
    }

    /*
     * Adjust our parent
     */
    if (adjustOthers && ((siblingRightMax > 0) && (siblingBottomMax > 0)))
    {
	ABObj	parent = obj_get_parent(obj);
        while ((parent != NULL) && (!obj_is_ui(parent)))
	{
	    parent = obj_get_parent(parent);
	}
	if (parent != NULL)
	{
            gilcvtP_obj_adjust_geometry(parent,
	        INT_UNDEF,
		INT_UNDEF,
	        siblingRightMax - obj_get_width(parent),
	        siblingBottomMax - obj_get_height(parent),
		TRUE,
		TRUE,
		adjustedList,
		adjustmentMadeOut);
	}
    }

epilogue:
    return 0;
#undef ALIGN_THRESH
}


static BOOL
gilcvtP_obj_has_no_module(ABObj obj)
{
    return ((obj == NULL) || (obj_get_module(obj) == NULL));
}


/*
 * Creates data ONLY for project-specific things that are not covered
 * by the individual modules.
 *
 * Assumes: project is a project
 */
static int
gilcvtP_create_obj_data_for_project(ABObj project)
{
    AB_TRAVERSAL	trav;
    ABObj		obj = NULL;
    CGenProjData	projData = NULL;
    CGenData		objData = NULL;
    size_t		objDataNumBytes = 0;
    int			numObjects = 0;
    int			objCount = 0;

    numObjects = 0;
    for (trav_open_cond(&trav, project, AB_TRAV_ALL|AB_TRAV_MOD_SAFE, 
						gilcvtP_obj_has_no_module);
	(obj = trav_next(&trav)) != NULL; )
    {
	++numObjects;
    }
    /* don't close trav, yet */

    /*
     * Create data space for all children
     */
    objDataNumBytes = numObjects * sizeof(CGenDataRec);
    objData= (CGenData)util_malloc(objDataNumBytes);
    memset(objData, 0, objDataNumBytes);

    /*
     * Assign each obj it's own struct
     */
    for (trav_reset(&trav), objCount = 0; 
	(obj = trav_next(&trav)) != NULL; )
    {
	obj->cgen_data = (CGenData)&(objData[objCount++]);
	mfobj_set_flags(obj, CGenFlagDataIsObj);
	mfobj_data(obj)->adjustX = INT_UNDEF;
	mfobj_data(obj)->adjustY = INT_UNDEF;
	mfobj_data(obj)->adjustWidth = INT_UNDEF;
	mfobj_data(obj)->adjustHeight = INT_UNDEF;
    }
    trav_close(&trav);	/* close it, now */
    util_dassert(1, (objCount == numObjects));

    /*
     * Create project-specific data
     */
    projData = (CGenProjData)calloc(1, sizeof(CGenProjDataRec));
    projData->callbacks = strlist_create();
    projData->children_data = objData;
    mfobj_set_proj_data(project, projData);

    return 0;
}

/*
 * Accepts any value (NULL, obj w/o module, ...)
 */
static int
gilcvtP_create_obj_data_for_module(ABObj objInModule)
{
    ABObj		module = NULL;
    int			numObjects = 0;
    CGenData		objData = NULL;
    size_t		objDataBytes = 0;
    ABObj		obj = NULL;
    AB_TRAVERSAL	trav;
    int			objCount = 0;

    if (   (objInModule == NULL) 
	|| (mfobj_has_data(objInModule))
	|| ((module = obj_get_module(objInModule)) == NULL)
       )
    {
	return 0;
    }


    /*
     * object data is an array of all data for the module.
     */
    numObjects = trav_count(module, AB_TRAV_ALL);
    objDataBytes = numObjects * sizeof(CGenDataRec);
    objData = (CGenData)util_malloc(objDataBytes);
    memset(objData, 0, objDataBytes);
    if (objData == NULL)
    {
	util_printf_err(catgets(Dtb_project_catd, 1, 36,
	    "Could not allocate cgen data\n"));
	return -1;
    }

    /*
     * Assign a structure to each object
     */
    for (trav_open(&trav, module, AB_TRAV_ALL), objCount = 0;
	(obj = trav_next(&trav)) != NULL; )
    {
	if (obj->cgen_data == NULL)
	{
	    obj->cgen_data = (CGenData)&(objData[objCount++]);
	    mfobj_set_flags(obj, CGenFlagDataIsObj);
	}
    }
    trav_close(&trav);
    util_dassert(1, (objCount <= numObjects));

    /*
     * Create module-specific data
     */
    ensure_data_for_module_obj(module);
    mfobj_get_module_data(module)->children_data = objData;

    return 0;
}


/*
 * Makes sure the module has a cgen data associated with it. Will load
 * the objects in the module if necessary, if loadFile is TRUE.
 */
static int
ensure_data_for_module_obj(ABObj module)
{
    CGenModuleData	moduleData = NULL;
    ABObj		project = NULL;
    ABObj		newProject = NULL;

    if (!obj_is_module(module))
    {
	return -1;
    }
    project = obj_get_project(module);
    if (module->cgen_data == NULL)
    {
        module->cgen_data = (CGenData)calloc(sizeof(CGenDataRec), 1);
    }

    if (mfobj_get_module_data(module) == NULL)
    {
	moduleData = (CGenModuleData)calloc(sizeof(CGenModuleDataRec), 1);
        moduleData->callbacks = strlist_create();
        mfobj_set_module_data(module, moduleData);
    }

    return 0;
}


#ifdef DEBUG
static int
dump_callbacks(ABObj project)
{
    ABObj		module = NULL;
    AB_TRAVERSAL	moduleTrav;
    StringList		callbacks = NULL;

    util_dprintf(0, "\n***** CALLBACKS *****\n");
    util_dprintf(0, "project callbacks\n");
    callbacks = mfobj_get_proj_data(project)->callbacks;
    if (callbacks != NULL)
    {
        strlist_dump(callbacks);
    }
    for (trav_open(&moduleTrav, project, AB_TRAV_MODULES);
	(module = trav_next(&moduleTrav)) != NULL; )
    {
	util_dprintf(0, "module '%s' callbacks\n", util_strsafe(obj_get_name(module)));
	callbacks = mfobj_get_module_data(module)->callbacks;
        if (callbacks != NULL)
        {
            strlist_dump(callbacks);
        }
    }
    trav_close(&moduleTrav);
    util_dprintf(0,"\n");
    return 0;
}
#endif

#ifdef DEBUG	/* performance testing */
static int	
get_cur_time(double *realTimeOut, double *cpuTimeOut)
{
	long		ticks_per_second = sysconf(_SC_CLK_TCK);
    struct tms	timeInfo;
    double	realTime = times(&timeInfo);
    double	cpuTime = timeInfo.tms_utime + timeInfo.tms_stime
			+ timeInfo.tms_cutime + timeInfo.tms_cstime;
    *realTimeOut  = realTime / CLK_TCK;
    *cpuTimeOut = cpuTime / CLK_TCK;

    return 0;
}
#endif /* DEBUG */

/*
 * REMIND: These routines should be in libABil somewhere
 */
static BOOL	
util_file_name_has_gil_extension(STRING path)
{
    int		len = util_strlen(path);
    return (   (len >= 2) 
	    && (   util_streq(&(path[len-2]), ".G")
	        || util_streq(&(path[len-2]), ".P")) );
}


/*
 * REMIND: move to libAButil
 */
/*
 * Makes a symbolic link, newPath, that references oldPath
 * This is a non-POSIX function, and this function hides the differences
 * on each of the platforms.
 */

#ifdef __cplusplus
extern "C" {
#endif
int symlink(const char *oldPath, const char *newPath);
#ifdef __cplusplus
} //extern "C"
#endif

util_symlink(STRING oldPath, STRING newPath)
{
    return symlink(oldPath, newPath);
}

