/*
 * File:         DtPStrings.h $XConsortium: DtPStrings.h /main/cde1_maint/1 1995/07/14 20:32:37 drk $
 * Language:     C
 *
 * (c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _DtPStrings_h
#define _DtPStrings_h

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/usr/dt"
#endif

#ifndef CDE_CONFIGURATION_TOP
#define CDE_CONFIGURATION_TOP "/etc/dt"
#endif

/* The following is the default tool class for applications that
 * use Dt[App]Init which do not have a tool class.
 */
#define DtDEFAULT_TOOL_CLASS		"DEFAULT_TOOL_CLASS"

#define DtDT_PROG_NAME 		"dt"	
	/* DT applications should use this name to look up DT-global 
	   resources instead of argv[0]. */

#define DtDT_PROG_CLASS		"Dt"
	/* DT applications should use this class to look up DT-global
	   resources. */

/* DT creates a bunch of directories in the user's home directory.
   The following constant specifies the permission mode for these directories. */
#define DtMODE_PERSONAL_DIRECTORIES	0755

/* The following string constants define the standard DT configuration
   directories. */
#define DtSYSTEM_CONFIG_DIRECTORY 	CDE_CONFIGURATION_TOP
#define DtLOCAL_CONFIG_DIRECTORY	"/usr/local/lib/X11/dt/config"
#define DtPERSONAL_CONFIG_DIRECTORY	".dt"
#define DtSM_SESSION_DIRECTORY			"sessions"
#define DtCURRENT_DT_VERSION			"3.0"

/* If you change the following two #defines, you must also change the
   related one below (DtDB_DIRS_DEFAULT). */
#define DtPERSONAL_DB_DIRECTORY		".dt/types"
#define DtPERSONAL_TMP_DIRECTORY	".dt/tmp"
#define DtPERSONAL_APPS_DIRECTORY	".dt/apps"

/* When a new user first logs into DT, a toolbox directory is built 
   for them.  This toolbox directory is populated with a set of default
   actions.  The following constant defines the directory in the DT
   tree where these default actions are found. */
#define DtDEFAULT_ACTIONS_DIRECTORY CDE_CONFIGURATION_TOP "/def-actions"

/* The following two constants are used for setting up the user's main "apps"
   directory that has symbolic links to all of the individual directories. */
#define DtSYM_NAME_SEPARATOR		'@'
#define DtSYM_NAME_DEFAULT		"apps_%d"

#define DtNETWORK_HOME_RESOURCE_NAME		"networkHome"
#define DtNETWORK_HOME_RESOURCE_CLASS		"NetworkHome"
#define DtFULL_NETWORK_HOME_RESOURCE_NAME	"dt.networkHome"
#define DtFULL_NETWORK_HOME_RESOURCE_CLASS	"Dt.NetworkHome"

#define DtDB_DIRS_RESOURCE_NAME			"typesDirs"
#define DtDB_DIRS_RESOURCE_CLASS		"TypesDirs"
#define DtFULL_DB_DIRS_RESOURCE_NAME		"dt.typesDirs"
#define DtFULL_DB_DIRS_RESOURCE_CLASS		"Dt.TypesDirs"

/* If you change the following two definitions, you must also change
   the related ones above. (DtPERSONAL_DB_DIRECTORY) */
#ifdef _SUN_OS
#define DtDB_DIRS_DEFAULT			\
          ".dt/types@personal_apps," \
          "/usr/local/lib/X11/dt/types@local_apps," \
          "/usr/lib/X11/dt/desk_set_types@desk_set_apps," \
          CDE_INSTALLATION_TOP "/lib/X11/types@system_apps"
#else
#define DtDB_DIRS_DEFAULT			\
          ".dt/types@personal_apps," \
          "/usr/local/lib/X11/dt/types@local_apps," \
          CDE_INSTALLATION_TOP "/lib/X11/types@system_apps"
#endif /* _SUN_OS */

#define DtTOOL_SUBDIR			"tools"
#define DtERRORLOG_FILE			"errorlog"
#define DtOLD_ERRORLOG_FILE		"errorlog.old"
#define DtOLDER_ERRORLOG_FILE   	"errorlog.older"

/* 
 * Strings for default types and icons
 */
#define DtDEFAULT_DATA_FT_NAME				"DATA"
#define DtFT_DIR_ICON_DEFAULT				"dirblue"
#define DtFT_DATA_ICON_DEFAULT				"data"

/*
 * The following string constants define the resource name,
 * resource class and default values for the action bitmaps.
 */

#define DtACTION_ICON_RESOURCE_NAME			"actionIcon"
#define DtACTION_ICON_RESOURCE_CLASS			"ActionIcon"
#define DtACTION_FULL_ICON_RESOURCE_NAME		"dt.actionIcon"
#define DtACTION_FULL_ICON_RESOURCE_CLASS		"Dt.ActionIcon"
#define DtACTION_ICON_DEFAULT				"Dtactn"

/*
 * The following string constants define the resource name,
 * resource class and default values for the Dt tmp directory path.
 */

#define DtACTION_DTTMPDIR_RESOURCE_NAME			"dtTmpDir"
#define DtACTION_DTTMPDIR_RESOURCE_CLASS		"DtTmpDir"
#define DtACTION_DTTMPDIR_DEFAULT			".dt/tmp"

/* Do not add anything after this endif. */
#endif /* _DtPStrings_h */
