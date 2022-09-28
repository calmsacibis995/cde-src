/* $XConsortium: HelpTermP.h /main/cde1_maint/1 1995/07/17 17:39:42 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   FormatTerm.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Public Header file for FormatTerm.c, the terminal access
 **                functions.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpFormatTermP_h
#define _DtHelpFormatTermP_h


#ifdef __cplusplus
extern "C" {
#endif

/**************************
 * Link Types and Window
 * Hints should match what
 * is in Canvas.h
 **************************/
/**************************
 * Link Types
 **************************/

#ifndef	CELinkType_Execute
#define CELinkType_Execute      3
#endif
#ifndef	CELinkType_ManPage
#define CELinkType_ManPage      4
#endif
#ifndef	CELinkType_AppDefine
#define CELinkType_AppDefine    5
#endif
#ifndef	CELinkType_SameVolume
#define CELinkType_SameVolume   6
#endif
#ifndef	CELinkType_CrossLink
#define CELinkType_CrossLink    7
#endif
#ifndef	CELinkType_TextFile
#define CELinkType_TextFile     8
#endif


/**************************
 * Window Hint Kinds
 **************************/

#ifndef	CEWindowHint_PopupWindow
#define CEWindowHint_PopupWindow        1
#endif
#ifndef	CEWindowHint_CurrentWindow
#define CEWindowHint_CurrentWindow      2
#endif
#ifndef	CEWindowHint_NewWindow
#define CEWindowHint_NewWindow          3
#endif

/********    Public Defines Declarations    ********/

/********    Public Structures Declarations    ********/


/*****************************************************************************
 * Structure:       DtHelpHyperLines
 *
 * Fields:          title    Indicates the title of the linked topic.
 *                  specification Contains the hypertext link information.
 *                  hyper_type  Specifies the hypertext links type.
 *
 * 
 *****************************************************************************/
typedef struct {
	char	*title;
	char	*specification;
	int	 hyper_type;
	int	 win_hint;
} DtHelpHyperLines;


/********    Public Function Declarations    ********/

/*****************************************************************************
 * Function:	      extern void _DtHelpFreeTopicData (
 *                                  char  **helpList,
 *                                  DtHelpHyperLines   *hyperList)
 *
 *
 * Parameters:      helpList    Specifies the parent widget ID.
 *                  hyperList   Specifies the name of the created dialog 
 *
 * Return Value:    void.
 *
 * Purpose: 	    Free up the info gotten by _DtHelpGetTopicData.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpFreeTopicData();
#else
extern void _DtHelpFreeTopicData (
    char           **helpList,
    DtHelpHyperLines   *hyperList);
#endif /* _NO_PROTO */



/*****************************************************************************
 * Function:	      extern int _DtHelpGetTopicData(
 *                                      char *helpVolume,
 *                                      char *locationID,
 *                                      int maxColumns,
 *                                      char ***helpList,
 *                                      DtHelpHyperLines  **hyperList);
 *
 * Parameters:      
 *                  
 *
 * Return Value:    int value, -1 implies that the function call failed, a 
 *                  value of 0, implies that the call was successful.
 *
 * Purpose: 	    Allows developers to extract semi-formatted ASCII data
 *                  from a Cache Creek help file.  
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern int _DtHelpGetTopicData();
#else
extern int _DtHelpGetTopicData (
    char *helpVolume,
    char *locationID,
    int maxColumns,
    char ***helpList,
    DtHelpHyperLines  **hyperList);
#endif /* _NO_PROTO */



/*****************************************************************************
 * Function:	      extern int _DtHelpProcessLinkData(
 *                                   DtHelpHyperLines *hyperLine,
 *                                   char **helpVolume,
 *                                   char **locationId )
 *
 *
 * Parameters:      
 *                  
 *
 * Return Value:    int value, -1 implies that the function call failed, a 
 *                  value of 0, implies that the call was successful.
 *
 * Purpose: 	    This function is used in conjunction with the
 *                  _DtHelpGetTopicData() function call to provide developers
 *                  with a mechanism in which they can traverse Cache Creek
 *                  hypertext links.  This  call will return the file and 
 *                  locationid information that when used with the 
 *                  DtGetTopicData function will retrrieve the help text
 *                  and new hypertext information associated with the previous
 *                  hypertext link information.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern int _DtHelpProcessLinkData();
#else
extern int _DtHelpProcessLinkData (
    char	     *ref_volume,
    DtHelpHyperLines *hyperLine,
    char **helpVolume,
    char **locationId);
#endif /* _NO_PROTO */


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _DtHelpFormatTermP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
