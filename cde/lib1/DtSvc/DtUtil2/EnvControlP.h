/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/****************************<+>*************************************
 **
 **   File:     EnvControlP.h
 **
 **   RCS:	$XConsortium: EnvControlP.h /main/cde1_maint/1 1995/07/14 20:33:23 drk $
 **
 **   Project:  HP DT Runtime Library  -- Private header
 **
 **   Description: Defines structures, and parameters used
 **                for communication with the environment
 **
 **   (c) Copyright 1992-94 by Hewlett-Packard Company
 **
 ****************************<+>*************************************/
/*******************************************************************
	The environment variables
*******************************************************************/
#define BIN_PATH_ENVIRON "PATH"
#define NLS_PATH_ENVIRON "NLSPATH"
#define SYSTEM_APPL_PATH_ENVIRON "XFILESEARCHPATH"
#define PM_PATH_ENVIRON	"XMICONSEARCHPATH"
#define BM_PATH_ENVIRON	"XMICONBMSEARCHPATH"

/*******************************************************************
	The default DT path strings, architecture-dependent
*******************************************************************/
#define BIN_PATH_STRING	 CDE_INSTALLATION_TOP "/bin"

#define NLS_PATH_STRING  CDE_INSTALLATION_TOP "/lib/nls/msg/%L/%N.cat:" \
                         CDE_INSTALLATION_TOP "/lib/nls/msg/C/%N.cat"

/*
 *  Some notes on the behavior and use of the XFILESEARCHPATH component, as
 *  defined through the SYSTEM_APPL_PATH_STRING definition below:
 *
 *      Its precedence in the lookup of X resources is fairly low--it can
 *	be overridden by XRM resources, by resources specified in
 *      the user's $HOME/.Xdefaults-<hostname> file, and by resources
 *      found using the $XUSERFILESEARCHPATH setting.
 *
 *      The order of pathnames in the XFILESEARCHPATH is such that the first
 *      match satisfies the lookup, and the lookup stops there.
 *
 *      We place the pathname components in our XFILESEARCHPATH such that
 *      the lookup goes, from first match attempt to last match attempt, as
 *      follows:
 *
 *      - Custom resources: /etc/../$LANG
 *      - Custom resources: /etc/../C
 *      - Factory defaults: /opt/../$LANG --shipped with every localized system
 *      - Factory defaults: /opt/../C     --shipped with every system
 *
 *      These resources are used ONLY for the DT components themselves
 *      (not, for example, for MIT client resources).
 *
 *      The CDE vendors retain the right to alter, remove, append to, and
 *      ignore any settings in the factory defaults locations.  The vendors
 *      will not modify the settings in the "custom resources" locations.
 *
 */
#define SYSTEM_APPL_PATH_STRING  CDE_CONFIGURATION_TOP "/app-defaults/%L/%N:" \
                                 CDE_CONFIGURATION_TOP "/app-defaults/C/%N:" \
                                 CDE_INSTALLATION_TOP "/app-defaults/%L/%N:" \
                                 CDE_INSTALLATION_TOP "/app-defaults/C/%N"


/**********************************************************************
 * Data representation of the user's DT environment
 **********************************************************************/

typedef struct environStruct {
	char * pmPath;
	char * binPath;
	char * nlsPath;
	char * sysApplPath;
	char * bmPath;
} _environStruct;

/**********************************************************************
 * Miscellaneous
 **********************************************************************/
#define MAX_ENV_STRING  2047

#define BV_BINPATH                      1
#define BV_SYSAPPLPATH                  2
#define BV_NLSPATH                      4
#define BV_PMPATH                       8
#define BV_BMPATH                      16

/****************************        eof       **********************/
