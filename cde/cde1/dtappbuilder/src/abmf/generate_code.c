
/*
 * $XConsortium: generate_code.c /main/cde1_maint/3 1995/10/16 14:23:47 rcs $
 * 
 * @(#)generate_code.c	1.37 11/08/95	cde_app_builder/src/abmf
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

#pragma ident "@(#)generate_code.c	1.37 95/11/08 SMI"

/*
 * generate_code.c - writes all files necessary for the project
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <ab_private/util.h>
#include <ab_private/abio.h>
#include "dtb_utils.h"
#include "msg_file.h"
#include "obj_namesP.h"
#include "write_codeP.h"
#include "ui_header_fileP.h"
#include "resource_fileP.h"
#include "ui_c_fileP.h"
#include "stubs_c_fileP.h"
#include "proj_c_fileP.h"
#include "proj_header_fileP.h"
#include "make_fileP.h"
#include "connectP.h"
#include "merge_cP.h"
#include "instancesP.h"
#include "utils_header_fileP.h"
#include "utils_c_fileP.h"
#include "abmf.h"
#include "msg_cvt.h"

/*************************************************************************
**                                                                      **
**       Constants (#define and const)					**
**                                                                      **
**************************************************************************/

typedef struct
{
    ISTRING             file_name;
    time_t              mod_time;
    BOOL                merged;
}                   GenLogEntryRec, *GenLogEntry;

typedef struct
{
    ISTRING             log_file;
    int                 num_entries;
    int                 entries_size;
    GenLogEntry         entries;
}                   GenLogRec, *GenLog;

static int          log_construct(GenLog);
static int          log_destruct(GenLog);
static int          log_sort_by_date(GenLog);
static GenLogEntry  log_find_entry_by_name(GenLog, STRING file_name);

static int		log_add_entry(
	      			GenLog,
	      			STRING file_name,
	      			time_t mod_date,
	      			BOOL merged
			);

#ifdef DEBUG
static int          log_dump(GenLog);
#else
#define			log_dump(x)	/* x */
#endif

/* private methods */
static int          logP_release_data(GenLog);
static int          logP_read(GenLog);
static int          logP_write(GenLog);
static int          logP_get_int_from_string(STRING * ptr_ptr);

/*************************************************************************
**                                                                      **
**       Private Functions (C declarations and macros)			**
**                                                                      **
**************************************************************************/

typedef enum
{
    ABMF_SKIP_UNDEF = 0,
    ABMF_SKIP_UP_TO_DATE,
    ABMF_SKIP_NO_CHANGES,
    ABMF_SKIP_USER_MODIFIED,
    ABMF_SKIP_ERR_OCCURRED,
    ABMF_SKIP_WHY_NUM_VALUES	/* must be last */
} ABMF_SKIP_WHY;

static int	write_make_file(
		    GenCodeInfo genCodeInfo,
		    GenLog genLog,
		    ABObj project,
		    BOOL merge_files,
		    BOOL check_dates,
		    BOOL * makeFileChangedPtr
		);

static int	write_project_files(
		    GenCodeInfo genCodeInfo,
		    GenLog genLog,
		    ABObj project,
		    BOOL merge_files,
		    BOOL check_dates,
		    BOOL force_write_c_file,
		    BOOL * headerFileChangedPtr,
		    BOOL * stubsCFileChangedPtr
		);

static int	write_module_files(
		   GenCodeInfo genCodeInfo,
		   GenLog genLog,
		   ABObj module,
		   BOOL merge_files,
		   BOOL check_dates,
		   STRING resFileName,
		   BOOL * headerFileChangedPtr,
		   BOOL * uiCFileChangedPtr,
		   BOOL * stubsCFileChangedPtr
		);

static int	write_utils_files(
			GenCodeInfo	genCodeInfo, 
			GenLog		genLog, 
			ABObj		project,
			BOOL		check_dates,
		        BOOL		*headerFileChangedPtr,
		        BOOL 		*CFileChangedPtr
		);

static int	write_app_resource_file(
			GenCodeInfo	genCodeInfo,
			GenLog		genLog,
			ABObj		project,
			BOOL		mergeFiles,
			BOOL		checkDates,
			BOOL		*fileChangedOut
		);
static int	write_msg_file(
			GenCodeInfo	genCodeInfo,
			GenLog		genLog,
			ABObj		project,
			BOOL		checkDates,
			BOOL		*msgFileChangedOut
		);
static int	print_progress_message(
			int	verbosity,
			STRING	message,
			STRING	fileName
		);
static int	print_backing_up_message(
			STRING fromFile, 
			STRING toFile
		);
static int	print_skipping_message(
			STRING		fileName, 
			ABMF_SKIP_WHY	why
		);
static int          count_possible_substruct_fields(ABObj obj);

static int compare_file_times(
		   STRING input_file,
		   STRING output_file,
		   GenLog genLog
);

static int 	should_write_file(
		  STRING output_file,
		  STRING input_file,
		  GenLog genLog,
		  BOOL merging,
		  BOOL check_dates
);

static int	check_and_merge_c_file(
			BOOL		needMerge,
			File		*codeFileInOut,
			STRING		mergeFileName,
			STRING		newFileName,
			ABMF_SKIP_WHY	*fileSkipReasonInOut,
			File		*deltaFileOut,
			File		*prevCodeFileOut,
			File		*prevCodeFileLockStreamOut,
			STRING		mainFuncAlternateName
		);
static int	check_and_replace_file(
			STRING		newFileName,
			STRING		prevFileName,
		  	GenLog		genLog,
			File		newFile, 
			File		oldFile,
			BOOL		check_for_changes,
			BOOL		fileWasMerged,
			BOOL		*fileChangedOutPtr,
			ABMF_SKIP_WHY	*fileSKippedReasonOutPtr
		);
static BOOL	file_changed_since_last_log(GenLog genLog, STRING fileName);
static time_t	get_file_mod_time(STRING fileName);
static BOOL	source_files_equal(FILE *file1, FILE *file2);
static int	move_file(STRING existingName, STRING newName, BOOL force);
static int	replace_file(
			STRING	newFileName, 
			STRING	oldFileName,
			File	fromFile, 
			BOOL	rewindFiles
		);
static time_t   mkgmtime(struct tm * tloc);
static int	touch_file(STRING fileName, BOOL printMessage);
static File	fopen_as_tmpfile(
			STRING	fileName, 
			File	*lockStreamOut
		);
static int	update_msg_set(MsgFile msgFile, ABObj obj);
static STRING	find_previous_file(
			STRING	fileName, 
			char	*bufOut, 
			int	bufOutSize
		);

#ifdef DEBUG
    static int	save_intermediate_file(File file, STRING fileName);
#else
    #define	save_intermediate_file(x, y) /* */
#endif

/* converts empty string to NULL */
#define str_or_null(s) (util_strempty(s)? ((STRING)NULL):(s))

/*
 * Various types of progress messages
 */
#define print_comparing_message(_fname) \
	 (print_progress_message(3, "comparing", _fname))

#define print_freshening_message(_fname) \
	(print_progress_message(1, \
	 catgets(Dtb_project_catd, 1, 88, "freshening (no changes)"), \
	 _fname))

#define print_processing_message(_fname) \
	 (print_progress_message(2, \
	  catgets(Dtb_project_catd, 1, 89, "processing"), \
	  _fname))

static int	print_merging_message(STRING fname, STRING newFname);

#define print_writing_message(_fname) \
	 (print_progress_message(1, \
	  catgets(Dtb_project_catd, 1, 52, "writing"), \
	  _fname))


/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

extern BOOL	freshenUnchangedFiles;
static BOOL	appResFileComplete = FALSE;

/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/



/*
 * Write all the code that the user wants written.
 */
int
abmf_generate_code(
		   ABObj 		project,
		   ABMF_CGEN_RESTRICTION restriction,
		   BOOL 		merge_files,
		   ABMF_I18N_METHOD 	i18n_method_in,
		   BOOL 		prototype_functions_in,
		   AB_ARG_CLASS_FLAGS	dumped_resources_in
)
{
    int                 return_value = 0;
    int                 iRC = 0;/* int return code */
    AB_TRAVERSAL        trav;
    ABObj               module = NULL;
    char                resFileName[MAX_PATH_SIZE];
    BOOL                writeAll = FALSE;
    GenCodeInfoRec      genCodeInfoRec;
    GenCodeInfo         genCodeInfo = &genCodeInfoRec;
    GenLogRec           genLogRec,
                       *genLog = &(genLogRec);
    BOOL                forceWriteProjectCFile = FALSE;
    BOOL                headerFileChanged = FALSE;
    BOOL                uiCFileChanged = FALSE;
    BOOL                stubsCFileChanged = FALSE;
    BOOL                makeFileChanged = FALSE;
    BOOL		msgFileChanged = FALSE;
    BOOL		resFileChanged = FALSE;
    BOOL                check_dates = FALSE;
    *resFileName = 0;
    abmfP_gencode_construct(&genCodeInfoRec);
    log_construct(&genLogRec);

    check_dates = ((restriction == ABMF_CGEN_BY_DATE) ||
		   (restriction == ABMF_CGEN_BY_DATE_AND_CONTENTS));

    /*
     * Set up GenCodeInfo data
     */
    genCodeInfo->prototype_funcs = prototype_functions_in;
    genCodeInfo->dumped_resources = dumped_resources_in;
    genCodeInfo->i18n_method = i18n_method_in;
    appResFileComplete = FALSE;

    /*
     * Determine if we're writing EVERYTHING
     */
    if (obj_get_write_me(project))
    {
	writeAll = TRUE;
	for (trav_open(&trav, project, AB_TRAV_MODULES);
	     (module = trav_next(&trav)) != NULL;)
	{
	    if (!obj_get_write_me(module))
	    {
		writeAll = FALSE;
		break;
	    }
	}
	trav_close(&trav);
    }

    abmfP_tree_set_action_names(project);

    /*
     * Open message source file if we have i18n enabled
     */
    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
	if (!abmfP_initialize_msg_file(genCodeInfo, project))
	{
	    util_dprintf(0, "Cannot generate message source file: %s\n",
			    genCodeInfo->msg_src_file_name);
	    return_value = -1;
	    goto epilogue;
	}
    }

    /*
     * Write out the modules
     */
    for (trav_open(&trav, project, AB_TRAV_MODULES);
	 (module = trav_next(&trav)) != NULL;)
    {
	if (!obj_get_write_me(module) 
	    || !obj_is_defined(module))
	{
	    continue;
	}
        
	/*
         * Open resource file (if necessary)
         */
	abmfP_get_intermediate_res_file_name(
		module, resFileName, MAX_PATH_SIZE);
        if (genCodeInfo->dumped_resources == AB_ARG_CLASS_FLAGS_NONE)
	{
	    unlink(resFileName);
	}
	else
        {
	    abmfP_get_intermediate_res_file_name(
		module, resFileName, MAX_PATH_SIZE);
	    genCodeInfo->resource_file = 
		abmfP_res_file_open(resFileName, resFileName, module, TRUE);
	    if (genCodeInfo->resource_file == NULL)
	    {
	        exit(1);
	    }
        }

	iRC = write_module_files(genCodeInfo, genLog,
			 module, merge_files, check_dates, resFileName,
		   &headerFileChanged, &uiCFileChanged, &stubsCFileChanged);
	return_if_err(iRC, iRC);
	if (headerFileChanged)
	{
	    forceWriteProjectCFile = TRUE;
	}
	abmfP_res_file_close(genCodeInfo->resource_file);
    }
    trav_close(&trav);
    if (return_value < 0)
    {
	goto epilogue;
    }


    /*
     * Write project/main files.
     */
    if (obj_get_write_me(project))
    {
	abmfP_get_intermediate_res_file_name(
		project, resFileName, MAX_PATH_SIZE);
        if (genCodeInfo->dumped_resources == AB_ARG_CLASS_FLAGS_NONE)
	{
	    unlink(resFileName);
	}
	else
        {
	    abmfP_get_intermediate_res_file_name(
		project, resFileName, MAX_PATH_SIZE);
	    genCodeInfo->resource_file = 
		abmfP_res_file_open(resFileName, resFileName, project, TRUE);
	    if (genCodeInfo->resource_file == NULL)
	    {
		exit(1);
	    }
	}

	iRC = write_project_files(genCodeInfo, genLog, project,
			    merge_files, check_dates, forceWriteProjectCFile,
			    &headerFileChanged, &stubsCFileChanged);
        return_if_err(iRC, iRC);
	abmfP_res_file_close(genCodeInfo->resource_file);
    }

    /*
     * Build the application resource file
     */
    iRC = write_app_resource_file(
			genCodeInfo, 
			genLog, 
			project, 
			merge_files, 
			check_dates, 
			&resFileChanged);
    return_if_err(iRC,iRC);

    /*
     * Write more project files
     */
    if (obj_get_write_me(project))
    {
	iRC = write_utils_files(genCodeInfo, genLog, project, check_dates,
				&headerFileChanged, &stubsCFileChanged);
        return_if_err(iRC, iRC);

        iRC = write_make_file(genCodeInfo, genLog,
		       project, merge_files, check_dates, &makeFileChanged);
        return_if_err(iRC, iRC);
    }

    iRC = write_msg_file(	genCodeInfo, 
			genLog,
			project, 
			check_dates, 
			&msgFileChanged);
    return_if_err(iRC, iRC);

    if (!appResFileComplete)
    {
	/* 
	 * REMIND: This should be a warning message the user sees. We
	 *         need permission to change the msg catalog
	 */
	if (debugging())
	{
            util_dprintf(1,
"Warning: Application resource file incomplete, because some modules\n");
            util_dprintf(1,
"         have not yet been processed.\n");
	}
    }

epilogue:
    if (return_value < 0)
    {
	util_puts_err("\n");
	util_puts_err(catgets(Dtb_project_catd, 1, 53,
	    "** Aborting due to errors **\n"));
    }
    abmfP_res_file_close(genCodeInfo->resource_file);	/* NULL OK */

    log_destruct(&genLogRec);
    abmfP_gencode_destruct(&genCodeInfoRec);
    return return_value;
}


/*
 * Write code for one module
 */
static int
write_module_files(
		   GenCodeInfo genCodeInfo,
		   GenLog genLog,
		   ABObj module,
		   BOOL merge_files,
		   BOOL check_dates,
		   STRING resFileName,
		   BOOL * headerFileChangedPtr,
		   BOOL * uiCFileChangedPtr,
		   BOOL * stubsCFileChangedPtr
)
{
#define headerFileChanged (*headerFileChangedPtr)
#define uiCFileChanged (*uiCFileChangedPtr)
#define stubsCFileChanged (*stubsCFileChangedPtr)
#define codeFile (genCodeInfo->code_file)
    int                 return_value = 0;
    int                 iRC = 0;/* int return code */
    STRING              errmsg = NULL;
    char                uiHeaderFileName[MAX_PATH_SIZE];
    char                uiHeaderFileNamePrev[MAX_PATH_SIZE];
    char                uiHeaderDefineName[MAX_PATH_SIZE];
    char                uiCFileName[MAX_PATH_SIZE];
    char                uiCFileNamePrev[MAX_PATH_SIZE];
    char                stubsFileName[MAX_PATH_SIZE];
    char                stubsFileNamePrev[MAX_PATH_SIZE];
    char                stubsBakFileName[MAX_PATH_SIZE];
    BOOL                needStubsMerge = FALSE;
    File		prevCodeFile = NULL;
    File		prevCodeFileLockStream = NULL;
    STRING              curFileName = NULL;
    STRING              curFileNamePrev = NULL; /* previous version */
    BOOL                curFileSkipped = FALSE;
    ABMF_SKIP_WHY	curFileSkipReason = ABMF_SKIP_UNDEF;
    *uiHeaderFileName = 0;
    *uiHeaderFileNamePrev = 0;
    *uiHeaderDefineName = 0;
    *uiCFileName = 0;
    *uiCFileNamePrev = 0;
    *stubsFileName = 0;
    *stubsFileNamePrev = 0;
    *stubsBakFileName = 0;

    headerFileChanged = FALSE;
    uiCFileChanged = FALSE;
    stubsCFileChanged = FALSE;

    if (!ab_c_ident_is_ok(obj_get_name(module)))
    {
	char *prog_name_string = util_get_program_name();
	char *module_name_string = obj_get_name(module);

	fprintf(stderr, catgets(Dtb_project_catd, 1, 54,
	    "%s: module filename must contain only letters\n"), 
	    prog_name_string);
	fprintf(stderr, catgets(Dtb_project_catd, 1, 55,
	    "and digits.  %s will generate C variables based\n"), 
	    prog_name_string);
	fprintf(stderr, catgets(Dtb_project_catd, 1, 56,
	    "on the module filename.  Please rename %s\n"), module_name_string);
	exit(1);
    }

    /*
     * Determine the file names
     */
    strcpy(uiCFileName, abmfP_get_ui_c_file_name(module));
    find_previous_file(uiCFileName, uiCFileNamePrev, MAX_PATH_SIZE);

    strcpy(uiHeaderFileName, abmfP_get_ui_header_file_name(module));
    strcpy(uiHeaderDefineName,
	   abmfP_get_define_from_file_name(uiHeaderFileName));
    find_previous_file(uiHeaderFileName, uiHeaderFileNamePrev, MAX_PATH_SIZE);

    sprintf(stubsFileName, abmfP_get_stubs_c_file_name(module));
    find_previous_file(stubsFileName, stubsFileNamePrev, MAX_PATH_SIZE);
    sprintf(stubsBakFileName, "%s.BAK", stubsFileName);

    /*
     * ***** WRITE UI HEADER FILE *****
     */
    curFileName = uiHeaderFileName;
    curFileNamePrev = str_or_null(uiHeaderFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    if (!should_write_file(
	curFileNamePrev, obj_get_file(module), genLog, FALSE, check_dates))
    {
	curFileSkipReason = ABMF_SKIP_UP_TO_DATE;
    }
    else
    {
	print_processing_message(uiHeaderFileName);
	if ((errmsg = abio_open_output(NULL, &codeFile)) != NULL)
	{
	    util_printf_err("%s\n", errmsg);
	    return_code(ERR_OPEN);
	}
	iRC = abmfP_write_ui_header_file(genCodeInfo,
					 module,
					 uiHeaderFileName,
					 uiHeaderDefineName);
	save_intermediate_file(genCodeInfo->code_file, uiHeaderFileName);
	return_if_err(iRC,iRC);

	iRC = check_and_replace_file(
			curFileName,
			curFileNamePrev,
			genLog,
			codeFile,
			NULL,
			check_dates,
			FALSE,
			&headerFileChanged,
			&curFileSkipReason);
	return_if_err(iRC,iRC);

	abio_close_output(codeFile);
    }				/* date check */

    /*
     * ***** WRITE UI.C FILE *****
     */
    curFileName = uiCFileName;
    curFileNamePrev = str_or_null(uiCFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    if (!should_write_file(curFileNamePrev, obj_get_file(module), genLog,
			   FALSE, check_dates))
    {
	curFileSkipReason = ABMF_SKIP_UP_TO_DATE;
    }
    else
    {
	print_processing_message(curFileName);
	if (!util_be_silent())
	{
	    if ((genCodeInfo->resource_file != NULL) && (resFileName != NULL))
	    {
		char *prog_name_string = util_get_program_name();
		fprintf(stderr, catgets(Dtb_project_catd, 1, 57,
			"%s: writing resources for %s into %s\n"),
			prog_name_string,
			uiCFileName,
			resFileName);
	    }
	}
	if ((errmsg = abio_open_output(NULL, &codeFile)) != NULL)
	{
	    util_printf_err("%s\n", errmsg);
	    return_code(ERR_OPEN);
	}
	iRC = abmfP_write_ui_c_file(genCodeInfo, uiCFileName, module);
	save_intermediate_file(genCodeInfo->code_file, uiCFileName);
	return_if_err(iRC, iRC);

	iRC = check_and_replace_file(
			curFileName, 
			curFileNamePrev,
			genLog,
			codeFile, 
			NULL,
			check_dates,
			FALSE,
			&uiCFileChanged,
			&curFileSkipReason);
	return_if_err(iRC, iRC);

	abio_close_output(codeFile);
    }				/* date check */


    /*****   WRITE STUBS.C FILE *****/

    /*
     * If the stubs file is already present, make a backup of it.
     */
    curFileName = stubsFileName;
    curFileNamePrev = str_or_null(stubsFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    if (!should_write_file(curFileNamePrev, obj_get_file(module), genLog,
			   merge_files, check_dates))
    {
	curFileSkipReason = ABMF_SKIP_UP_TO_DATE;
    }
    else
    {
	needStubsMerge = merge_files;
	if (util_strempty(curFileNamePrev))
	{
	    /* no previous version */
	    needStubsMerge = FALSE;
	}

	if (curFileSkipReason == ABMF_SKIP_UNDEF)
	{
	    /*
	     * Open the output (temp) file.
	     */
	    File	deltaFile = NULL;

	    errmsg = abio_open_output(NULL, &codeFile);	/* tmp file */
	    if (errmsg != NULL)
	    {
		util_printf_err("%s\n", errmsg);
		return_code(ERR_OPEN);
	    }

	    /*
	     * Write the output file
	     */
	    print_processing_message(curFileName);
	    iRC = abmfP_write_stubs_c_file(genCodeInfo, curFileName, module);
	    save_intermediate_file(genCodeInfo->code_file, curFileName);
	    return_if_err(iRC, iRC);

	    /*
	     * Merge the file
	     */
	    iRC = check_and_merge_c_file(
			needStubsMerge,
			&codeFile,
			curFileNamePrev, /* merge w/prev. version */
			curFileName,
			&curFileSkipReason,
			&deltaFile,
			&prevCodeFile,
			&prevCodeFileLockStream,
			NULL);
	    return_if_err(iRC,iRC);
			
	    /*
	     * Replace the file with the new version
	     */
	    iRC = check_and_replace_file(
			curFileName, 
			curFileNamePrev,
			genLog,
			codeFile, 
			prevCodeFile,
			check_dates,
			needStubsMerge,
			&stubsCFileChanged,
			&curFileSkipReason);
	    return_if_err(iRC,iRC);

	    if ((stubsCFileChanged) && (deltaFile != NULL))
	    {
		char	deltaFileName[MAXPATHLEN];
		sprintf(deltaFileName, "%s.delta", stubsFileName);
	        replace_file(deltaFileName, deltaFileName, deltaFile, TRUE);
	    }

    	    util_fclose(codeFile);
	    util_fclose(prevCodeFile);
	    util_fclose(prevCodeFileLockStream);
    	    util_fclose(deltaFile);
	} /* !curFileSkipped */
    } /* date check */

epilogue:
    abio_close_output(codeFile);
    return return_value;
#undef headerFileChanged
#undef uiCFileChanged
#undef uiStubsFileChanged
#undef codeFile
}


/*
 * Write the code for the main files of a project (project.h, project.c).
 */
static int
write_project_files(
		    GenCodeInfo genCodeInfo,
		    GenLog genLog,
		    ABObj project,
		    BOOL merge_files,
		    BOOL check_dates,
		    BOOL force_write_c_file,
		    BOOL * headerFileChangedPtr,
		    BOOL * stubsCFileChangedPtr
)
{
#define headerFileChanged (*headerFileChangedPtr)
#define stubsCFileChanged (*stubsCFileChangedPtr)
#define codeFile (genCodeInfo->code_file)
    int                 return_value = 0;
    int                 iRC = 0;/* int return code */
    STRING              errmsg = NULL;
    BOOL                needHeaderMerge = FALSE;
    BOOL                needStubsMerge = FALSE;
    char                headerFileName[MAX_PATH_SIZE];
    char                headerFileNamePrev[MAX_PATH_SIZE];
    char                headerDefineName[MAX_PATH_SIZE];
    char                headerBakFileName[MAX_PATH_SIZE];
    char                stubsFileName[MAX_PATH_SIZE];
    char                stubsFileNamePrev[MAX_PATH_SIZE];
    char                stubsBakFileName[MAX_PATH_SIZE];
    char		projInitFuncName[MAX_PATH_SIZE];
    STRING              curFileName = NULL;
    STRING              curFileNamePrev = NULL;
    ABMF_SKIP_WHY	curFileSkipReason = ABMF_SKIP_UNDEF;
    File		orgFile = NULL;
    File		prevCodeFile = NULL;
    File		prevCodeFileLockStream = NULL;
    File		mergedFile = NULL;
    File		deltaFile = NULL;
    *headerFileName = 0;
    *headerFileNamePrev = 0;
    *headerDefineName = 0;
    *headerBakFileName = 0;
    *stubsFileName = 0;
    *stubsFileNamePrev = 0;
    *stubsBakFileName = 0;
    *projInitFuncName = 0;

    headerFileChanged = FALSE;
    stubsCFileChanged = FALSE;

    strcpy(headerFileName, abmfP_get_project_header_file_name(project));
    find_previous_file(headerFileName, headerFileNamePrev, MAX_PATH_SIZE);
    strcpy(headerDefineName, abmfP_get_define_from_file_name(headerFileName));
    sprintf(headerBakFileName, "%s.BAK", headerFileName);

    strcpy(stubsFileName, abmfP_get_project_c_file_name(project));
    find_previous_file(stubsFileName, stubsFileNamePrev, MAX_PATH_SIZE);
    sprintf(stubsBakFileName, "%s.BAK", stubsFileName);
    strcpy(projInitFuncName, abmfP_get_project_init_proc_name(project));


    /***** PROJECT HEADER FILE *****/

    curFileName = headerFileName;
    curFileNamePrev = str_or_null(headerFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    needHeaderMerge = (merge_files && (!util_strempty(curFileNamePrev)));
    if (!should_write_file(
			   headerFileName, obj_get_file(project), genLog,
			   needHeaderMerge, check_dates))
    {
	curFileSkipReason = ABMF_SKIP_UP_TO_DATE;
    }
    else
    {
	/*
	 * Write the beastie
	 */
	print_processing_message(curFileName);
	if ((errmsg = abio_open_output(NULL, &codeFile)) != NULL)
	{
	    util_printf_err("%s\n", errmsg);
	    return ERR_OPEN;
	}
	iRC = abmfP_write_project_header_file(
					      genCodeInfo,
					      project,
					      headerFileName);
	save_intermediate_file(genCodeInfo->code_file, headerFileName);
	return_if_err(iRC, iRC);

  	/*
         * Merge the file
         */
	iRC = check_and_merge_c_file(
			needHeaderMerge,
			&codeFile,
			curFileNamePrev,	/* merge w/previous version */
			curFileName,
			&curFileSkipReason,
			&deltaFile,
			&prevCodeFile,
			&prevCodeFileLockStream,
			NULL);
	return_if_err(iRC,iRC);
			
	/*
	 * Replace the old version with the new
	 */
	iRC = check_and_replace_file(
			curFileName, 
			curFileNamePrev,
			genLog,
			codeFile, 
			prevCodeFile,
			check_dates,
			needHeaderMerge,
			&headerFileChanged,
			&curFileSkipReason);
	return_if_err(iRC, iRC);
    }				/* compare_file_times */

    abio_close_output(codeFile);
    util_fclose(prevCodeFile);
    util_fclose(prevCodeFileLockStream);
    util_fclose(orgFile);
    util_fclose(mergedFile);
    util_fclose(deltaFile);


    /*
     * ***** PROJECT .C  *****
     */
    curFileName = stubsFileName;
    curFileNamePrev = str_or_null(stubsFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    needStubsMerge = (merge_files && (!util_strempty(curFileNamePrev)));
    if (!(force_write_c_file ||
	  should_write_file(curFileName, obj_get_file(project),
			    genLog, merge_files, check_dates)))
    {
	curFileSkipReason = ABMF_SKIP_UP_TO_DATE;
    }
    else
    {
	if (util_be_verbose())
	{
	    if (check_dates && force_write_c_file)
	    {
		util_printf(catgets(Dtb_project_catd, 1, 58,
		    "generating %s because of changes in modules\n"),
		    stubsFileName);
	    }
	}

	print_processing_message(curFileName);
	errmsg = abio_open_output(NULL, &codeFile);
	if (errmsg != NULL)
	{
	    return_value = ERR_OPEN;
	    util_printf_err("%s\n", errmsg);
	    goto epilogue;
	}
	iRC = abmfP_write_project_c_file(
		       genCodeInfo, curFileName, needStubsMerge, project);
	save_intermediate_file(genCodeInfo->code_file, stubsFileName);
	return_if_err(iRC, iRC);

  	/*
         * Merge the file
         */
	iRC = check_and_merge_c_file(
			needStubsMerge,
			&codeFile,
			curFileNamePrev,	/* merge w/prev. version */
			curFileName,
			&curFileSkipReason,
			&deltaFile,
			&prevCodeFile,
			&prevCodeFileLockStream,
			projInitFuncName);
	return_if_err(iRC,iRC);
			
	/*
	 * Replace the old version with the new
	 */
	iRC = check_and_replace_file(
			curFileName,
			curFileNamePrev,
			genLog,
			codeFile, 
			prevCodeFile,
			check_dates,
			needStubsMerge,
			&stubsCFileChanged,
			&curFileSkipReason);
	return_if_err(iRC, iRC);

    }				/* compare_file_times */

epilogue:
    abio_close_output(codeFile);
    util_fclose(prevCodeFile);
    util_fclose(prevCodeFileLockStream);
    util_fclose(orgFile);
    util_fclose(mergedFile);
    util_fclose(deltaFile);
    return return_value;
#undef headerFileChanged
#undef stubsCFileChanged
#undef codeFile
}


static int
write_make_file(
		GenCodeInfo	genCodeInfo,
		GenLog		genLog,
		ABObj		project,
		BOOL		merge_files,
		BOOL		check_dates,
		BOOL		*makeFileChangedPtr
)
{
#define makeFileChanged (*makeFileChangedPtr)
#define makeFile (genCodeInfo->code_file)
    int                 return_value = 0;
    int                 rc = 0;
    STRING              errmsg = NULL;
    int                 intOSMin = ((int) AB_OS_UNDEF) + 1;
    int                 intOSMax = ((int) AB_OS_TYPE_NUM_VALUES) - 1;
    AB_OS_TYPE          actualOSType = util_get_os_type();
    AB_OS_TYPE          curOSType = AB_OS_UNDEF;
    STRING              curOSTypeString = NULL;
    char                curMakeFileName[256];
    BOOL		curMakeFileIsActual = FALSE;
    BOOL		writeCurMakeFile = FALSE;
    STRING              actualMakeFileName = "Makefile";
    BOOL		actualMakeFileModified = FALSE;
    BOOL		actualMakeFileIsOurs = FALSE;
    int                 i = 0;
    STRING		curFileName = NULL;
    ABMF_SKIP_WHY	curFileSkipReason = ABMF_SKIP_UNDEF;
    *curMakeFileName = 0;
    merge_files = merge_files;	/* ignored, for now */

    makeFileChanged = FALSE;

    /* if it's not there, we own it */
    actualMakeFileIsOurs = !util_file_exists(actualMakeFileName);

    for (i = intOSMin; i <= intOSMax; ++i)
    {
	writeCurMakeFile = TRUE;
	curMakeFileIsActual = FALSE;
	curOSType = (AB_OS_TYPE) i;
	curOSTypeString = util_os_type_to_ident(curOSType);
	if (curOSTypeString == NULL)
	{
	    continue;
	}
	sprintf(curMakeFileName, "%s.%s", actualMakeFileName, curOSTypeString);
	curFileName = curMakeFileName;
        curFileSkipReason = ABMF_SKIP_UNDEF;

	if (util_paths_are_same_file(curMakeFileName, actualMakeFileName))
	{
	    curMakeFileIsActual = TRUE;
	    actualMakeFileIsOurs = TRUE;
	}
        if (file_changed_since_last_log(genLog, curMakeFileName))
	{
	    writeCurMakeFile = FALSE;
	    if (curMakeFileIsActual)
	    {
		actualMakeFileModified = TRUE;
	    }
	    if (util_be_verbose())
	    {
		char *prog_name_string = util_get_program_name();
		fprintf(stderr, catgets(Dtb_project_catd, 1, 59,
		    "%s: skipping user-defined %s\n"),
		    prog_name_string, curMakeFileName);
	    }
	}

	if (writeCurMakeFile)
	{
	    print_processing_message(curMakeFileName);
	    if ((errmsg = abio_open_output(NULL, &makeFile)) != NULL)
	    {
	        util_printf_err("%s\n", errmsg);
	        goto epilogue;
	    }
	    abmfP_write_make_file(genCodeInfo, project, curOSType, FALSE);
	    save_intermediate_file(makeFile, curMakeFileName);

	    rc = check_and_replace_file(
			curFileName,
			curFileName,
			genLog,
			makeFile, 
			NULL,
			check_dates,
			FALSE,			/* merge?! YEAH, RIGHT! */
			&makeFileChanged,
			&curFileSkipReason);
	    return_if_err(rc,rc);

	    if (rc >= 0)
	    {
	        abio_close_output(makeFile);
	        makeFileChanged = TRUE;
	        log_add_entry(genLog, curFileName, -1, FALSE);
	    }
	} /* if writeCurMakeFile */
    } /* for i=osType */

    if (actualOSType == AB_OS_UNDEF)
    {
	util_printf_err(catgets(Dtb_project_catd, 1, 60,
	    "Could not determine OS type of this machine\n"));
	goto epilogue;
    }

    sprintf(curMakeFileName, "%s.%s",
	    actualMakeFileName, util_os_type_to_ident(actualOSType));
    if (! (actualMakeFileIsOurs && (!actualMakeFileModified)) )
    {
	/* The user has modified this file since we wrote it last */
	if (util_be_verbose())
	{
	    util_printf(catgets(Dtb_project_catd, 1, 61,
	       "skipping user-defined %s\n"), actualMakeFileName);
	}
    }
    else if (!util_paths_are_same_file(curMakeFileName, actualMakeFileName))
    {
	/* make link from os-specific makefile to 'Makefile' */
	if (!util_be_silent())
	{
	    util_printf(catgets(Dtb_project_catd, 1, 62,
		 "linking %s => %s\n"), curMakeFileName, actualMakeFileName);
	}
	rc = unlink(actualMakeFileName);
	if (util_file_exists(actualMakeFileName))
	{
	    util_printf_err(catgets(Dtb_project_catd, 1, 63,
		"Could not remove %s\n"), actualMakeFileName);
	    return_code(-1);
	}
	rc = link(curMakeFileName, actualMakeFileName);
	if (rc != 0)
	{
	    util_printf_err(catgets(Dtb_project_catd, 1, 64,
		"Could not create link to %s\n"), curMakeFileName);
	    return_code(-1);
	}
	log_add_entry(genLog, actualMakeFileName, -1, FALSE);
    }

epilogue:
    abio_close_output(makeFile);
    return return_value;
#undef makeFileChanged
#undef makeFile
}


static int
write_utils_files(
			GenCodeInfo	genCodeInfo, 
			GenLog		genLog, 
			ABObj		project,
			BOOL		check_dates,
		        BOOL		*headerFileChangedPtr,
		        BOOL 		*CFileChangedPtr
)
{
#define headerFileChanged (*headerFileChangedPtr)
#define CFileChanged (*CFileChangedPtr)
#define codeFile (genCodeInfo->code_file)
    int			return_value = 0;
    int			iRC = 0;		/* int return code */
    ABMF_SKIP_WHY	curFileSkipReason = ABMF_SKIP_UNDEF;
    STRING	utilHFileName = abmfP_get_utils_header_file_name(project);
    char	utilHFileNamePrev[MAX_PATH_SIZE];
    STRING 	utilCFileName = abmfP_get_utils_c_file_name(project);
    char	utilCFileNamePrev[MAX_PATH_SIZE];
    STRING		errmsg = NULL;
    STRING		curFileName = NULL;
    STRING		curFileNamePrev = NULL;
    *utilHFileNamePrev = 0;
    *utilCFileNamePrev = 0;

    find_previous_file(utilCFileName, utilCFileNamePrev, MAX_PATH_SIZE);
    find_previous_file(utilHFileName, utilHFileNamePrev, MAX_PATH_SIZE);

    /***** UTILITY HEADER FILE *****/

    curFileName = utilHFileName;
    curFileNamePrev = str_or_null(utilHFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    /*
     * Write the beastie
     */
    print_processing_message(curFileName);
    if ((errmsg = abio_open_output(NULL, &codeFile)) != NULL)
    {
        util_printf_err("%s\n", errmsg);
        return ERR_OPEN;
    }
    iRC = abmfP_write_utils_header_file(genCodeInfo, curFileName, project);
    save_intermediate_file(genCodeInfo->code_file, curFileName);
    return_if_err(iRC, iRC);

    /*
     * Replace the old version with the new
     */
    iRC = check_and_replace_file(
		curFileName, 
		curFileNamePrev,
		genLog,
		codeFile, 
		NULL,
		check_dates,
		FALSE,
		&headerFileChanged,
		&curFileSkipReason);
    return_if_err(iRC, iRC);
    abio_close_output(codeFile);


    /*
     * ***** PROJECT .C  *****
     */
    curFileName = utilCFileName;
    curFileNamePrev = str_or_null(utilCFileNamePrev);
    curFileSkipReason = ABMF_SKIP_UNDEF;
    {
	print_processing_message(curFileName);
	errmsg = abio_open_output(NULL, &codeFile);
	if (errmsg != NULL)
	{
	    return_value = ERR_OPEN;
	    util_printf_err("%s\n", errmsg);
	    goto epilogue;
	}
        iRC = abmfP_write_utils_c_file(genCodeInfo, curFileName, project);
        save_intermediate_file(genCodeInfo->code_file, curFileName);
	return_if_err(iRC, iRC);

	/*
	 * Replace the old version with the new
	 */
	iRC = check_and_replace_file(
			curFileName, 
			curFileNamePrev,
			genLog,
			codeFile, 
			NULL,
			check_dates,
			FALSE,
			&CFileChanged,
			&curFileSkipReason);
	return_if_err(iRC, iRC);

    }				/* compare_file_times */

epilogue:
    abio_close_output(codeFile);
    return return_value;
#undef headerFileChanged
#undef CFileChanged
#undef codeFile
}


static int	
write_app_resource_file(
			GenCodeInfo	genCodeInfo,
			GenLog		genLog,
			ABObj		project,
			BOOL		merge_files,
			BOOL		check_dates,
			BOOL		*fileChangedOut
)
{
    int		return_value = 0;
    int		rc = 0;			/* return code */
    File	newResFile = NULL;
    char	appResFileName[MAXPATHLEN+1];
    STRING	appResFileNamePrev = NULL;
    STRING	curFileName = NULL;
    ABMF_SKIP_WHY	curFileSkipReason = ABMF_SKIP_UNDEF;
    *appResFileName = 0;

    /*
     * Write new vanilla resource file
     */
    abmfP_get_app_res_file_name(project, appResFileName, MAXPATHLEN+1);
    print_processing_message(appResFileName);
    newResFile = abmfP_res_file_open(NULL, appResFileName, project, FALSE);
    curFileName = appResFileName;
    abio_printf(newResFile, 
	"! All CDE applications should include the standard Dt resource file\n"
	"#include \"Dt\"\n"
	"\n");
    rc = abmfP_write_app_res_file(newResFile, project, appResFileName);
    save_intermediate_file(newResFile, appResFileName);
    if (rc < 0)
    {
	/* error */
    }
    else if (rc == 0)
    {
	appResFileComplete = TRUE;
    }
    else if (rc == 1)
    {
	/* file built successfully, but is incomplete */
	if (!util_be_silent())
	{
	    /*
	     * We don't want to merge this file. Its pointless, since it's
	     * incomplete, and it may make us overwrite an existing  
	     * complete resource file.
	     */
	    abmfP_res_file_close(newResFile);
	    goto epilogue;
	}
    }

    /*
     * Merge the files, if necessary
     */
    if (merge_files)
    {
	File	oldResFile = util_fopen_locked(appResFileName, "r");
	File	mergedResFile = NULL;
	int	rc = 0;

	if (oldResFile != NULL)
	{
	    appResFileNamePrev = appResFileName;
	    print_merging_message(curFileName, NULL);
	    abmfP_res_file_merge(newResFile, oldResFile, &mergedResFile);
	    assert(rc >= 0);
	    if (mergedResFile != NULL)
	    {
	        util_fclose(newResFile);
	        newResFile = mergedResFile;
	        mergedResFile = NULL;
	    }
	}
    }

    /*
     * See if anything changed
     */
    rc = check_and_replace_file(
		appResFileName,
		appResFileNamePrev,
		genLog,
		newResFile,
		NULL,
		check_dates,
		merge_files,
		fileChangedOut,
		&curFileSkipReason);
    return_if_err(rc,rc);

epilogue:
    return return_value;
}


static int
write_msg_file(
			GenCodeInfo	genCodeInfo,
			GenLog		genLog,
			ABObj		project,
			BOOL		check_for_changes,
			BOOL		*msgFileChangedOutPtr
)
{
    int			return_value = 0;
    int			rc = 0;			/* return code */
    File		newMsgFile = NULL;
    ABMF_SKIP_WHY	msgFileSkipReason = ABMF_SKIP_UNDEF;
    STRING		curFileName = genCodeInfo->msg_src_file_name;
    AB_TRAVERSAL	trav;
    ABObj		module = NULL;

    update_msg_set(genCodeInfo->msg_file_obj, project);
    for (trav_open(&trav, project, AB_TRAV_MODULES);
	(module = trav_next(&trav)) != NULL; )
    {
	update_msg_set(genCodeInfo->msg_file_obj, module);
    }
    trav_close(&trav);

    /*
     * Build the message catalog
     */
    if (genCodeInfo->i18n_method == ABMF_I18N_XPG4_API)
    {
	print_processing_message(curFileName);
	rc = MsgFile_save(genCodeInfo->msg_file_obj, &newMsgFile);
	return_if_err(rc,rc);

   	rc = check_and_replace_file(
			curFileName,
			curFileName,
			genLog,
			newMsgFile,
			NULL,
			check_for_changes,
			TRUE,
			msgFileChangedOutPtr,
			&msgFileSkipReason);
	return_if_err(rc,rc);
    }

epilogue:
    util_fclose(newMsgFile);
    return return_value;
}


/*
 * Ensures that all the messages for the given object have been 
 * created, if the object is being generated.
 *
 * For objects that are not being code-generated, makes sure that no
 * messages will be deleted.
 */
static int
update_msg_set(MsgFile msgFile, ABObj obj)
{
#define do_string(_s) \
	    if ((string = (_s)) != NULL) \
	    { \
		MsgSet_sure_find_msg(msgSet, string); \
	    }

    int			return_value = 0;
    STRING		string = NULL;
    MsgSet		msgSet = NULL;
    ABObj		stringObj = NULL;
    BOOL		objIsProject = obj_is_project(obj);
    AB_TRAVERSAL	trav;
    assert(obj_is_module(obj) || obj_is_project(obj));

    msgSet = MsgFile_obj_sure_find_msg_set(msgFile, obj);
    if (msgSet == NULL)
    {
	return ERR_INTERNAL;
    }

    if (obj_get_write_me(obj))
    {
	MsgSet_set_is_referenced(msgSet, TRUE);
	MsgSet_set_allow_msg_delete(msgSet, TRUE);
	for (trav_open(&trav, obj, AB_TRAV_ALL);
		(stringObj = trav_next(&trav)) != NULL; )
	{
	    if (objIsProject && (obj_get_module(stringObj) != NULL))
	    {
		continue;
	    }
	    do_string(obj_get_arg_string(stringObj));
	    do_string(obj_get_menu_title(stringObj));
	    do_string(obj_get_ok_label(stringObj));
	    do_string(obj_get_msg_string(stringObj));
	    do_string(obj_get_action1_label(stringObj));
	    do_string(obj_get_action2_label(stringObj));
	    do_string(obj_get_initial_value_string(stringObj));
	    do_string(obj_get_icon_label(stringObj));
	    do_string(obj_get_label(stringObj));
	}
	trav_close(&trav);
    }
    else
    {
	MsgSet_set_allow_msg_delete(msgSet, FALSE);
    }

    return return_value;
#undef do_string
}


/*
 * Returns TRUE if the output file should be written
 */
static int
should_write_file(
		  STRING output_file,
		  STRING input_file,
		  GenLog genLog,
		  BOOL merge_files,
		  BOOL check_times
)
{
    output_file = output_file;	/* avoid warnings */
    input_file = input_file;
    genLog = genLog;
    merge_files = merge_files;
    check_times = check_times;
    return TRUE;

/*
 * This is supposed to check to see generation parameters have changed
 * that should cause the file to be regenerated. Unfortunately, it's gotten
 * out-of-date and does not catch all cases. 
 *
 * Don't delete this, it can be revamped and re-used later.
 */
#ifdef BOGUS		/* save this - we will need it again */
    static BOOL			initialized = FALSE;
    static time_t		exeDate = (time_t)-1;
    GenLogEntry         	logEntry = NULL;
    struct stat			fileInfo;

    if (!initialized)
    {
	char	exePath[MAX_PATH_SIZE];
	STRING	exeDir = NULL;
	STRING	exeFile = NULL;
	*exePath = 0;
	initialized = TRUE;

	exeDir = dtb_get_exe_dir();
	exeFile = util_get_program_name();
	if (exeDir != NULL)
	{
	    strcpy(exePath, exeDir);
	}
	if (exeFile != NULL)
	{
	    strcat(exePath, "/");
	    strcat(exePath, exeFile);
	}

        exeDate = (time_t)0;
        if (stat(exePath, &fileInfo) == 0)
	{
	    exeDate = fileInfo.st_mtime;
	}
    }

    if (!check_times)
    {
	return TRUE;
    }

    /*
     * See if the generation parameters have changed.
     */
    logEntry = log_find_entry_by_name(genLog, output_file);
    if ((logEntry != NULL) && (util_xor(logEntry->merged, merge_files)))
    {
	return TRUE;
    }

    /*
     * See if the code generator executable is newer than this file
     */
    if (stat(output_file, &fileInfo) == 0)
    {
        if (exeDate > fileInfo.st_mtime)
        {
	    return TRUE;
        }
    }

    /* 
     * Check input vs. output timestamps
     */
    return (compare_file_times(input_file, output_file, genLog) > 0);
#endif /* BOGUS */
}


static int
check_and_merge_c_file(
			BOOL		needMerge,
			File		*codeFileInOut,
			STRING		codeFileName,
			STRING		newCodeFileName,
			ABMF_SKIP_WHY	*fileSkipReasonInOut,
			File		*deltaFileOut,
			File		*prevCodeFileOut,
			File		*prevCodeFileLockStreamOut,
			STRING		mainFuncAlternateName
)
{
    int		return_value = 0;
    int		rc = 0;			/* return code */
    File	codeFile = *codeFileInOut;
    File	mergedFile = NULL;
    File	deltaFile = NULL;
    File	*deltaFilePtr = NULL;
    File	orgFile = NULL;
    File	orgFileLockStream = NULL;	/* holds lock */

    #ifdef DEBUG
    if (debugging())
    {
	deltaFilePtr = &deltaFile;	/* delta file is primarily debugging */
    }
    #endif /* DEBUG */

    (*deltaFileOut) = NULL;
    (*prevCodeFileOut) = NULL;
    (*prevCodeFileLockStreamOut) = NULL;

    if (   ((*fileSkipReasonInOut) != ABMF_SKIP_UNDEF)
	|| (!needMerge) )
    {
	return 0;
    }

    print_merging_message(codeFileName, newCodeFileName);
    orgFile = fopen_as_tmpfile(codeFileName, &orgFileLockStream);
    if (orgFile == NULL)
    {
        *fileSkipReasonInOut = ABMF_SKIP_ERR_OCCURRED;
        util_printf_err("%s: %s\n", codeFileName, strerror(errno));
        return_code(-1);
    }
    rewind(codeFile);
    rewind(orgFile);
    rc = abmfP_merge_c_files(
	    codeFile, newCodeFileName, TRUE,
	    orgFile, codeFileName, FALSE,
	    &mergedFile, deltaFilePtr,
	    mainFuncAlternateName);
    if (rc < 0)
    {
	*fileSkipReasonInOut = ABMF_SKIP_ERR_OCCURRED;
        return_code(-1);
    }
    
    if (mergedFile == NULL)
    {
        *fileSkipReasonInOut = ABMF_SKIP_NO_CHANGES;
    }
    else
    {
        /* changes were detected and merged */
        util_fclose(codeFile);
        codeFile = mergedFile; mergedFile = NULL;
    }

epilogue:
    if (prevCodeFileOut != NULL)
    {
	if (debugging()) {assert(prevCodeFileLockStreamOut != NULL);}
	(*prevCodeFileOut) = orgFile;
	(*prevCodeFileLockStreamOut) = orgFileLockStream;
    }
    else
    {
        util_fclose(orgFile);
        util_fclose(orgFileLockStream);
    }
    *deltaFileOut = deltaFile;
    *codeFileInOut = codeFile;
    return return_value;
}


/*
 * Replaces oldFileName with newFileName, backing up oldFilename, if
 * it exists.
 *
 * genLog may be NULL
 * oldFile may be NULL - if it is, oldFileName will be opened for
			 reading.
 * oldFileName may also be NULL - if it is, the old file is ignored
 * 				  completely.
 */
static int
check_and_replace_file(
			STRING		newFileName,
			STRING		oldFileName,
			GenLog		genLog,
			File		newFile, 
			File		oldFileIn,
			BOOL		check_for_changes,
			BOOL		fileWasMerged,
			BOOL		*fileChangedOutPtr,
			ABMF_SKIP_WHY	*fileSkippedReasonOutPtr
)
{
#define fileSkippedReasonOut (*fileSkippedReasonOutPtr)
    int		return_value = 0;
    int		rc = 0;			/* return code */
    BOOL	fileChanged = FALSE;
    BOOL	updateLog = FALSE;

    if (debugging())
    {
	assert(newFileName != NULL);
        assert(newFile != NULL);
    }

    fileSkippedReasonOut = ABMF_SKIP_UNDEF;

    if (   (!check_for_changes) 
	|| (!util_streq(newFileName, oldFileName))
	|| (!util_file_exists(oldFileName))
       )
    {
	/* name changed, file is new, or we're not checking */
	fileChanged = TRUE;
    }
    else
    {
        File 	oldFile = oldFileIn;
	if (oldFile == NULL)
	{
	    /* no stream passed in - open it */
	    oldFile = util_fopen_locked(oldFileName, "r");
	}
	else
	{
	    rewind(oldFile);
	}

	if (oldFile == NULL)
	{
	    if (errno == ENOENT)
	    {
	        /* brand new file */
	        fileChanged = TRUE;
	    }
	    else
            {
	        fileSkippedReasonOut = ABMF_SKIP_ERR_OCCURRED;
	        util_printf_err(catgets(Dtb_project_catd, 1, 65, "%s: %s\n"),
		    oldFileName, strerror(errno));
	        return_code(-1);
            }
	}
	else
	{
            print_comparing_message(oldFileName);
            fileChanged = !source_files_equal(newFile, oldFile);
	}

	if (oldFileIn == NULL)
	{
	    /* we opened the file, so we must close it */
            util_fclose(oldFile);
	}
    }
    
    if (fileChanged)
    {
	print_writing_message(newFileName);

	/*
	 * Check write permission on file
	 */
	if (util_file_exists(newFileName))
	{
	    File	file = util_fopen_locked(newFileName, "a+");
	    if (file == NULL)
	    {
	        fileSkippedReasonOut = ABMF_SKIP_ERR_OCCURRED;
	        util_printf_err(catgets(Dtb_project_catd, 1, 65, "%s: %s\n"),
		    newFileName, strerror(errno));
		return_code(ERR_OPEN);
	    }
	    util_fclose(file);
	}

	/*
	 * Replace the old with the new
	 */
        rc = replace_file(newFileName, oldFileName, newFile, TRUE);
	if (rc < 0)
	{
	    fileChanged = FALSE;
	    fileSkippedReasonOut = ABMF_SKIP_ERR_OCCURRED;
	    return_code(rc);
	}
	else
	{
	    /* successful creation of new file */
	    updateLog = TRUE;
	}
    }
    else
    {
	/* no changes - it's up-to-date */
	fileSkippedReasonOut = ABMF_SKIP_NO_CHANGES;
	if (freshenUnchangedFiles)
	{
	    touch_file(newFileName, TRUE);	/* update the timestamp */
	}
	else
	{
	    print_skipping_message(oldFileName, fileSkippedReasonOut);
	}

	updateLog = TRUE;
    }

epilogue:
    if (updateLog && (genLog != NULL))
    {
        log_add_entry(genLog, newFileName, -1, fileWasMerged);
    }
    *fileChangedOutPtr = fileChanged;
    return return_value;
#undef fileSkippedReasonOut
} /* check_and_replace_file */


static int
move_file(STRING fileName, STRING newFileName, BOOL force)
{
    int		rc = 0;

    if (force)
    {
	unlink(newFileName);
    }

    rc = link(fileName, newFileName);
    if (rc != 0)
    {
	return -1;
    }

    rc = unlink(fileName);
    if (rc != 0)
    {
	return -1;
    }

    return 0;
}


static int
replace_file(
			STRING	newFileName, 
			STRING	oldFileName,
			File	fromFile, 
			BOOL	rewindFiles
)
{
    int		c;
    char	oldBakFileName[MAXPATHLEN];
    File	toFile = NULL;


    if (rewindFiles)
    {
	rewind(fromFile);
    }

    /*
     * Back up "old" file
     */
    if (util_file_exists(oldFileName))
    {
        sprintf(oldBakFileName, "%s.BAK", oldFileName);
        print_backing_up_message(oldFileName, oldBakFileName);
        move_file(oldFileName, oldBakFileName, TRUE);
    }

    /*
     * Open new file
     */
    toFile = util_fopen_locked(newFileName, "w");
    if (toFile == NULL)
    {
	util_printf_err(catgets(Dtb_project_catd, 1, 65, "%s: %s\n"),
	    newFileName, strerror(errno));
	return ERR_OPEN;
    }

    /*
     * Copy the sucker!
     */
    while ((c = fgetc(fromFile)) != EOF)
    {
	fputc(c, toFile);
    }

    util_fclose(toFile);
    return 0;
}


static int
print_progress_message(int verbosity, STRING message, STRING fileName)
{
    if (util_get_verbosity() >= verbosity)
    {
	util_printf_err(catgets(Dtb_project_catd, 1, 90, "%s %s\n"),
		message, fileName);
    }
    return 0;
}


static int
print_merging_message(
			STRING	fileName,
			STRING	newFileName
)
{
    BOOL	fileNameChanged = (   (newFileName != NULL) 
				   && (!util_streq(newFileName, fileName)));

    if (fileNameChanged && (!util_be_silent()))
    {
	util_printf(catgets(Dtb_project_catd, 1, 87, "merging %s and %s\n"),
		fileName, newFileName);
    }
    else if (util_be_verbose())
    {
	util_printf(catgets(Dtb_project_catd, 1, 86, "merging %s\n"),
		fileName);
    }
    return 0;
}

static int
print_skipping_message(STRING fileName, ABMF_SKIP_WHY why)
{
    if (util_be_verbose())
    {
	switch (why)
	{
	    case ABMF_SKIP_NO_CHANGES: 
		util_printf(catgets(Dtb_project_catd, 1, 68,
		    "skipping (no changes) %s\n"), fileName);
		break;
	    case ABMF_SKIP_UP_TO_DATE: 
		util_printf(catgets(Dtb_project_catd, 1, 69,
		    "skipping (up-to-date) %s\n"), fileName);
		break;
	    case ABMF_SKIP_ERR_OCCURRED: 
		util_printf(catgets(Dtb_project_catd, 1, 70,
		    "skipping due to errors %s\n"), fileName);
		break;
	}
    }
    return 0;
}


static int
print_backing_up_message(STRING fileFromName, STRING fileToName)
{
    if (util_be_verbose())
    {
	util_printf(catgets(Dtb_project_catd, 1, 71, 
	  "saving previous %s to %s\n"), fileFromName, fileToName);
    }
    return 0;
}


/*
 * Returns: < 0 if file1 < file2 (file1 is older) 0 if file1 == file2 (files
 * have same mod. time) > 0 if file1 > file2 (file1 is newer)
 */
static int
compare_file_times(STRING input_file, STRING output_file, GenLog genLog)
{
    int                 return_value = 0;
    GenLogEntry         logEntry = log_find_entry_by_name(genLog, output_file);
    struct stat         fileInfo;

    if (logEntry == NULL)
    {
	/* we don't have any record of output file, so input file is newer */
	return 1;
    }
    if (!util_file_exists(output_file))
    {
	/* output file doesn't exit - input file is newer */
	return 1;
    }

    if (stat(input_file, &fileInfo) != 0)
    {
	/* the input file doesn't exist? return 1 to force the issue */
	return 1;
    }

    return_value = 0;
    if (fileInfo.st_mtime > logEntry->mod_time)
    {
	return_value = 1;
    }
    else if (fileInfo.st_mtime < logEntry->mod_time)
    {
	return_value = -1;
    }

    return return_value;
} 


/*
 * See if the user has fiddled with this file
 */
static BOOL
file_changed_since_last_log(GenLog genLog, STRING fileName)
{
    BOOL	fileWasModified = FALSE;
    GenLogEntry	logEntry = NULL;
    time_t	lastLogWriteTime = 0;

    logEntry = log_find_entry_by_name(genLog, fileName);
    lastLogWriteTime = (logEntry == NULL? 0: logEntry->mod_time);
    fileWasModified = (get_file_mod_time(fileName) > lastLogWriteTime);

    return fileWasModified;
}


/*
 * Returns (time_t)-1 if file not found, or other error
 */
static time_t
get_file_mod_time(STRING fileName)
{
    time_t	fileModTime = (time_t)-1;
    struct stat	fileInfo;
    if (stat(fileName, &fileInfo) == 0)
    {
	fileModTime = fileInfo.st_mtime;
    }
    return fileModTime;
}


/*
 * POSIX gives us struct -> local time, but no struct -> UTC, so we've to to
 * write our own.
 */
static              time_t
mkgmtime(struct tm * localTimeStruct)
{
    static long         timeDiff = 0;
    time_t              gmTime = (time_t) - 1;
    time_t              curTime = 0;
    time_t              localTime = 0;
    time_t              adjustedTime = 0;
    struct tm          *gmTimeStruct = NULL;
    int                 localTimeIsDST = 0;

    /*
     * Determine time difference between localtime and gmtime
     */
    curTime = time(NULL);
    localTimeIsDST = localTimeStruct->tm_isdst;
    if (localTimeIsDST < 0)
    {
	/* we're supposed to figure out DST */
	struct tm *curTimeStruct = localtime(&curTime);
	localTimeIsDST = curTimeStruct->tm_isdst;
    }
    gmTimeStruct = gmtime(&curTime);
    if (gmTimeStruct == NULL)
    {
	goto epilogue;
    }
    gmTimeStruct->tm_isdst = localTimeIsDST;
    adjustedTime = mktime(gmTimeStruct);
    if (adjustedTime == (time_t) - 1)
    {
	goto epilogue;
    }
    timeDiff = ((long) curTime) - ((long) adjustedTime);

    /*
     * Convert given time as a local time and adjust
     */
    localTime = mktime(localTimeStruct);
    if (localTime == (time_t) - 1)
    {
	goto epilogue;
    }
    gmTime = (time_t) (((long) localTime) + ((long) timeDiff));

epilogue:
    return gmTime;
}


static int
touch_file(STRING fileName, BOOL printMessage)
{
    int rc = utime(fileName, NULL);
    if (printMessage)
    {
        print_freshening_message(fileName);
    }
    return (rc == 0? OK:ERR);
}


/*
 * Opens file, copying it to a temp file. The file is opened locked,
 * and for reading only.
 *
 * The file pointer returned can always be used to read the file
 * contents. The lockfile return parameter should be closed
 * when done (both should be closed) to free any lock on the
 * file.
 *
 */
static File
fopen_as_tmpfile(STRING fileName, File *lockStreamOut)
{
    File	file = util_fopen_locked(fileName, "r");
    File	tmpFile = NULL;
    int		c = 0;

    (*lockStreamOut) = NULL;
    if (file == NULL)
    {
	return NULL;
    }

    if ((tmpFile = tmpfile()) == NULL)
    {
	return NULL;
    }
       
    while ((c = fgetc(file)) != EOF)
    {
	fputc(c, tmpFile);
    }
    rewind(tmpFile);		/* make it look like it was just opened */

    if (lockStreamOut == NULL)
    {
	util_fclose(file);
    }
    else
    {
	(*lockStreamOut) = file;
    }

    return tmpFile;
}


/*
 * Modifies: current position on file1,file2
 * parameters may be NULL
 */
static BOOL
source_files_equal(FILE *file1, FILE *file2)
{
    BOOL	filesSame = TRUE;
    BOOL	done = FALSE;
    int		file1Char = ' ';
    int		file2Char = ' ';

    if (file1 == file2)
    {
	return TRUE;
    }
    else if ((file1 == NULL) || (file2 == NULL))
    {
	/* one is NULL, the other is not */
	return FALSE;
    }

    rewind(file1);
    rewind(file2);

    while (!done)
    {
	if (file1Char != EOF)
	{
	    file1Char = fgetc(file1); 
	}
	if (file2Char != EOF)
	{
	    file2Char = fgetc(file2); 
	}

	filesSame = (file1Char == file2Char);
	done =  ((!filesSame) || ((file1Char == EOF) && (file2Char == EOF)));
    }

    return filesSame;
}


/*
 * Attempts to find the "previous" version of this file. Looks for
 * the same file with different extensions, in case the user has
 * changed the file extension.
 *
 * Returns bufOut if successful, NULL otherwise.
 * bufOut will always be valid, but will contain a 0-length string if
 * no previous file found.
 */
static STRING
find_previous_file(STRING fileName, char *bufOut, int bufOutSize)
{
    STRING	returnPtr = NULL;
    char	*dotPtr = NULL;
    int		i = 0;
    const char	*(cExts[]) = {".c", ".cc", ".C", ".c++", ".cpp", ".cxx", NULL};
    const char 	*(hExts[]) = {".h", ".hh", ".H", ".h++", ".hpp", ".hxx", NULL};
    const char	**extList = NULL;
    *bufOut = 0;

    util_strncpy(bufOut, fileName, bufOutSize);
    if (util_file_exists(bufOut))
    {
	/* This file exists, exactly as is! */
	goto epilogue;
    }
    dotPtr = strrchr(bufOut, '.');
    if ((dotPtr == NULL) || (!isalpha(*(dotPtr+1))))
    {
	/* forget it - no extension on this file. */
	*bufOut = 0;
	goto epilogue;
    }

    /*
     * Determine the list of extensions to try
     */
    if (tolower(*(dotPtr+1)) == 'h')
    {
	extList = hExts;
    }
    else if (tolower(*(dotPtr+1)) == 'c')
    {
	extList = cExts;
    }
    else
    {
	*bufOut = 0;
	goto epilogue;
    }

    /*
     * Try out all the extensions
     */
    for (i = 0; extList[i] != NULL; ++i)
    {
	strcpy(dotPtr, extList[i]);
	if (util_file_exists(bufOut))
	{
	    goto epilogue;
	}
    }
    *bufOut = 0;		/* didn't find any of them */

epilogue:
#ifdef DEBUG
    util_dprintf(1, "DEBUG: find previous(%s) -> '%s'\n", fileName, bufOut);
#endif
    if ((*bufOut) != 0)
    {
	return bufOut;
    }
    return NULL;
}


/*************************************************************************
 **									**
 **			Debugging					**
 **									**
 *************************************************************************/

#ifdef DEBUG
static int
save_intermediate_file(File file, STRING fileName)
{
    static int	fileCount = -1;
    int		return_value = 0;
    fpos_t	fpos;
    File	outFile = NULL;
    char	outFilePath[MAXPATHLEN+1];

    ++fileCount;
    if (debug_level() < 2)
    {
	return 0;
    }
    if (file == NULL)
    {
	return -1;
    }
    fgetpos(file, &fpos);

    /*
     * Make sure tmp directory exists
     */
    if (!util_directory_exists("./tmp"))
    {
	errno = 0;
	mkdir("./tmp", 0755);
        if (!util_directory_exists("./tmp"))
        {
	    fprintf(stderr, 
	    "%s: DEBUG:: Could not create ./tmp for debugging output: %s\n", 
		util_get_program_name(),
		strerror(errno));
	    return_value = -1;
	    goto epilogue;
	}
    }

    /*
     * Determine output path
     */
    {
	int	outFilePathLen = 0;
	strcpy(outFilePath, "./tmp/");
	outFilePathLen = strlen(outFilePath);
	if (fileName == NULL)
	{
	    sprintf(outFilePath+outFilePathLen, "tmp-%d-%d", 
			getpid(), fileCount);
	}
	else
	{
	    strcat(outFilePath, fileName);
	}
	if (util_file_exists(outFilePath))
	{
	    unlink(outFilePath);
	}
    }

    /*
     * Write the output file
     */

    if ((outFile = fopen(outFilePath, "w")) == NULL)
    {
        fprintf(stderr, "%s: DEBUG: could not open %s: %s\n",
	    util_get_program_name(),
	    outFilePath,
	    strerror(errno));
	    return_value = -1;
	    goto epilogue;
    }
    
    {
	int	c = 0;
	errno = 0;
        rewind(file);
        if (ferror(file))
        {
            fprintf(stderr, "%s: DEBUG: could not rewind tmp file: %s\n",
	        util_get_program_name(),
	        strerror(errno));
	        return_value = -1;
	        goto epilogue;
        }

        /*
	 * FINALLY! Actually save the tmp file to a permanent file
	 */
	util_dprintf(2, "%s: DEBUG: saving intermediate file: %s\n",
	        util_get_program_name(), outFilePath);

        while ((c = fgetc(file)) != EOF)
        {
            fputc(c, outFile);
        }
    }
    
epilogue:
    fsetpos(file, &fpos);
    if (outFile != NULL)
    {
	fclose(outFile); outFile = NULL;
    }
    return return_value;
}
#endif /* DEBUG */

/*************************************************************************
 **									**
 **			GenLog						**
 **									**
 *************************************************************************/

static int
log_construct(GenLog log)
{
    log->log_file = NULL;
    log->num_entries = 0;
    log->entries_size = 0;
    log->entries = NULL;
    return logP_read(log);
}


static int
log_destruct(GenLog log)
{
    logP_write(log);
    logP_release_data(log);
    return 0;
}


static int
logP_release_data(GenLog log)
{
    int                 i;

    istr_destroy(log->log_file);

    for (i = 0; i < log->entries_size; ++i)
    {
	istr_destroy(log->entries[i].file_name);
	log->entries[i].mod_time = (time_t) 0;
    }
    log->num_entries = 0;
    log->entries_size = 0;

    util_free(log->entries);

    return 0;
}

static              GenLogEntry
log_find_entry_by_name(GenLog log, STRING file_name)
{
    GenLogEntry         return_value = NULL;
    ISTRING             istr_file_name = istr_dup_existing(file_name);
    int                 i = 0;

    if (istr_file_name == NULL)
    {
	return_code(NULL);
    }

    for (i = 0; i < log->num_entries; ++i)
    {
	if (istr_equal(istr_file_name, log->entries[i].file_name))
	{
	    return_value = &(log->entries[i]);
	    break;
	}
    }


epilogue:
    istr_destroy(istr_file_name);
    return return_value;
}


/*
 * If mod_time is (time_t)-1, the file itself is checked for modification
 * time.
 */
static int
log_add_entry(GenLog log, STRING file_name, time_t mod_time, BOOL merged)
{
    int                 return_value = 0;
    GenLogEntry         entry = log_find_entry_by_name(log, file_name);
    int                 new_entries_size = 0;
    int                 new_num_entries = 0;
    GenLogEntry         new_entries = NULL;

    if (mod_time == (time_t)-1)
    { 
	mod_time = get_file_mod_time(file_name);
    }

    if (entry != NULL)
    {
	entry->mod_time = mod_time;
	entry->merged = merged;
	return_code(0);
    }

    /*
     * Allocate space to append
     */
    new_entries_size = log->entries_size;
    new_num_entries = log->num_entries + 1;
    new_entries_size = util_max(new_num_entries, new_entries_size);
    new_entries = (GenLogEntry) realloc(log->entries,
				 new_entries_size * sizeof(GenLogEntryRec));

    if (new_entries == NULL)
    {
	return_code(ERR_NO_MEMORY);
    }

    log->entries = new_entries;
    log->entries_size = new_entries_size;
    log->num_entries = new_num_entries;

    /*
     * Fill in the new data
     */
    entry = &(log->entries[(log->num_entries) - 1]);
    entry->file_name = istr_create(file_name);
    entry->mod_time = mod_time;
    entry->merged = merged;

epilogue:
    return return_value;
}



#ifdef DEBUG
static int
log_dump(GenLog log)
{
    int                 i = 0;
    GenLogEntry         entry = NULL;

    if (log == NULL)
    {
	util_dprintf(0, "NULL log\n");
	return 0;
    }
    if (log->num_entries == 0)
    {
	util_dprintf(0, "Empty log file:%s\n", istr_string_safe(log->log_file));
	return 0;
    }
    util_dprintf(0, "===== Gen Log (file %s) =====\n",
		 istr_string_safe(log->log_file));
    for (i = 0; i < log->num_entries; ++i)
    {
	entry = &(log->entries[i]);
	util_dprintf(0, "%s %ld merged:%d\n",
	       istr_string_safe(entry->file_name), (long) (entry->mod_time),
		     entry->merged);
    }
    util_dprintf(0, "===================\n");
    return 0;
}
#endif /* DEBUG */


static int
logP_read(GenLog log)
{
    int                 return_value = 0;
    char                log_file_name[MAXPATHLEN] = "";
    char                line_buf[MAXPATHLEN] = "";
    GenLogEntry         next_entry = NULL;
    int                 i = 0;
    int                 num_lines = 0;
    File                file = NULL;
    char               *token_start = NULL;
    char               *line_ptr = NULL;
    char               *line_tmp_ptr = NULL;
    int                 c = 0;
    int                 year = 0;
    int                 month = 0;
    int                 day = 0;
    int                 hour = 0;
    int                 minute = 0;
    int                 second = 0;
    struct tm           time_struct;

    logP_release_data(log);	/* re-init */

    sprintf(log_file_name, ".%s.log", util_get_program_name());
    log->log_file = istr_create(log_file_name);
    file = util_fopen_locked(log_file_name, "rt");
    if (file == NULL)
    {
	return_code(0);
    }

    num_lines = 0;
    while ((c = getc(file)) != EOF)
    {
	if ((c == '\n') || (c == '\r'))
	{
	    ++num_lines;
	}
    }

    if (num_lines < 1)
    {
	return_code(0);
    }

    log->entries_size = num_lines;
    log->entries = (GenLogEntry) util_malloc(
				log->entries_size * sizeof(GenLogEntryRec));
    if (log->entries == NULL)
    {
	log->entries_size = 0;
	return_code(ERR_NO_MEMORY);
    }
    for (i = 0; i < log->entries_size; ++i)
    {
	next_entry = &(log->entries[i]);
	next_entry->file_name = NULL;
	next_entry->mod_time = 0;
	next_entry->merged = FALSE;
    }

    rewind(file);

    for (; (!feof(file)) && (log->num_entries < log->entries_size);)
    {
	next_entry = &(log->entries[(log->num_entries)]);

	*line_buf = 0;
	fgets(line_buf, MAXPATHLEN, file);
	line_ptr = line_buf;

	/*
	 * file name
	 */
	token_start = line_ptr;
	line_ptr = strchr(line_ptr, ' ');
	if (line_ptr == NULL)
	{
	    continue;		/* error - skip this one */
	}
	*line_ptr = 0;
	if (strlen(token_start) <= (size_t) 0)
	{
	    continue;		/* an error - skip this entry */
	}
	next_entry->file_name = istr_create(token_start);
	*line_ptr = ' ';

	/*
	 * Generation time
	 */
	year = logP_get_int_from_string(&line_ptr);
	month = logP_get_int_from_string(&line_ptr);
	day = logP_get_int_from_string(&line_ptr);
	hour = logP_get_int_from_string(&line_ptr);
	minute = logP_get_int_from_string(&line_ptr);
	second = logP_get_int_from_string(&line_ptr);
	time_struct.tm_year = year - 1900;
	time_struct.tm_mon = month - 1;
	time_struct.tm_mday = day;
	time_struct.tm_hour = hour;
	time_struct.tm_min = minute;
	time_struct.tm_sec = second;
	time_struct.tm_isdst = -1;	/* let sys determine DST or no */

	if (line_ptr == NULL)
	{
	    continue;		/* an error - skip this line */
	}

	next_entry->mod_time = mkgmtime(&time_struct);
	if (next_entry->mod_time == (time_t) - 1)
	{
	    continue;
	}

	/*
	 * merged
	 */
	line_tmp_ptr = strstr(line_ptr, "merged:");
	if (line_tmp_ptr == NULL)
	{
	    continue;		/* an error - skip this entry */
	}
	else
	{
	    line_tmp_ptr += strlen("merged:");
	    next_entry->merged = (tolower(*line_tmp_ptr) == 'y');
	    if ((*line_tmp_ptr) != 0)
	    {
		++line_tmp_ptr;
	    }
	    line_ptr = line_tmp_ptr;
	}

	++(log->num_entries);
    }

epilogue:
    util_fclose(file);
    /* util_dprintf(0, "After read "); log_dump(log); */
    return return_value;
}


static int
logP_write(GenLog log)
{
    int                 return_value = 0;
    STRING              log_file_name = istr_string(log->log_file);
    File                file = NULL;
    GenLogEntry         entry = NULL;
    int                 i = 0;
    struct tm          *time_struct = NULL;
    STRING              file_name = NULL;

    if (log_file_name == NULL)
    {
	return_code(0);
    }
    file = util_fopen_locked(log_file_name, "wt");
    if (file == NULL)
    {
	return_code(ERR);
    }

    for (i = 0; i < log->num_entries; ++i)
    {
	entry = &(log->entries[i]);
	file_name = istr_string(entry->file_name);

	/* this is mostly in case we mucked things up. */
	if ((file_name == NULL) || (!util_file_exists(file_name)))
	{
	    continue;
	}

	time_struct = gmtime(&(entry->mod_time));
	fprintf(file, "%s %04d.%02d.%02d.%02d.%02d.%02d",
		file_name,
		time_struct->tm_year + 1900,
		time_struct->tm_mon + 1,
		time_struct->tm_mday,
		time_struct->tm_hour,
		time_struct->tm_min,
		time_struct->tm_sec);

	fprintf(file, " merged:%s", (entry->merged ? "y" : "n"));

	fprintf(file, "\n");
    }

epilogue:
    util_fclose(file);
    return return_value;
}


static int
logP_get_int_from_string(STRING * ptr_ptr)
{
#define ptr (*ptr_ptr)
    int                 int_value = 0;
    char               *tok_start = NULL;

    if (ptr == NULL)
    {
	return 0;
    }

    tok_start = ptr;
    while (((*tok_start) != 0) && (!isdigit(*tok_start)))
    {
	++tok_start;
    }

    if ((*tok_start) != 0)
    {
	int_value = atoi(tok_start);

	for (ptr = tok_start; ((*ptr) != 0) && isdigit(*ptr); ++ptr)
	{
	}
    }

    return int_value;
#undef ptr
}
