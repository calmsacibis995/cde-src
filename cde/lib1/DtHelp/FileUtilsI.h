/* $XConsortium: FileUtilsI.h /main/cde1_maint/1 1995/07/17 17:27:59 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FileUtilsI.h
 **
 **   Project:     DtHelp Project
 **
 **   Description: File locating and handling utilities
 ** 
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpFileUtilsI_h
#define _DtHelpFileUtilsI_h

#define DtHelpVOLUME_TYPE "volumes"      /* dir for %T in XtResolvePathname */
#define DtHelpCCDF_VOL_SUFFIX ".hv"      /* possible basename suffix */
#define DtHelpSDL_VOL_SUFFIX ".sdl"      /* possible basename suffix */

/* array indices and size for use with _DtHelpFileGetSearchPaths() */
#define _DtHELP_FILE_USER_PATH 0
#define _DtHELP_FILE_SYS_PATH  1
#define _DtHELP_FILE_HOME_PATH 2
#define _DtHELP_FILE_NUM_PATHS 3

typedef struct _dtHelpCeDirStruct {
	char    *dir_name;
	int      type;
	int      user_flag;
	struct _dtHelpCeDirStruct *next_dir;
} _DtHelpCeDirStruct;

/* list of suffixes that help volumes may have */
extern const char * _DtHelpFileSuffixList[];

#ifdef _NO_PROTO
extern	int			 _DtHelpCeCheckAndCacheDir();
extern	void			 _DtHelpFileGetSearchPaths();
extern	char			*_DtHelpFileLocate ();
extern	Boolean			 _DtHelpFileTraceLinks ();
extern	Boolean			 _DtHelpFileTraceToFile ();
#else
extern	int			 _DtHelpCeCheckAndCacheDir(char *dir);
extern	void			 _DtHelpFileGetSearchPaths(
				         char *  paths[],
				         Boolean searchHomeDir);
extern	char			 *_DtHelpFileLocate (
				        char *       type,
				        char *       base,
				        const char * suffixList[],
				        Boolean      searchCurDir,
				        int          accessMode);
extern	Boolean			 _DtHelpFileTraceLinks (
					char * *  pPathName);
extern	Boolean			 _DtHelpFileTraceToFile (
				        char * *  pPathName,
				        int       accessMode,
				        char * *  pFoundPath);
#endif 

#endif /* _DtHelpFileUtilsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

