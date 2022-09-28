/* $XConsortium: UtilI.h /main/cde1_maint/1 1995/07/17 21:02:48 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Util.h
 **
 **   Project:     helpviewer 3.0
 **
 **   Description: Structures and defines needed HelpUtil.h in our Preview
 **                tool (aka, helpviewer).
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
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

#ifndef _UtilI_h
#define _UtilI_h



#define DEFAULT_ROWS         20
#define DEFAULT_COLUMNS      20




/****************************************************************
 *
 *  Cache List Info Structure Definition.
 *
 ****************************************************************/
typedef struct _CacheListStruct {
    Widget                    helpDialog;
    Boolean                   inUseFlag;
    struct _CacheListStruct   *pNext;
    struct _CacheListStruct   *pPrevious;
} CacheListStruct;






/****************************************************************************
 * Function:	    void DisplayTopic(
 *                              Widget parent,
 *                              char *accessPath,
 *                              char *idString)
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays a new Cache Creek help topic in a new help dialog
 *                  widget.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void DisplayTopic(); 
#else
extern void DisplayTopic (
    Widget  parent,
    char    *accessPath,
    char    *idString,
    int     argc,
    char    **argv);
#endif /* _NO_PROTO */



/****************************************************************************
 * Function:	    void DisplayFile(
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays a ascii file in a quick help dialog.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void DisplayFile(); 
#else
extern void DisplayFile (
    Widget  parent,
    char    *file);
#endif /* _NO_PROTO */




/****************************************************************************
 * Function:	    void DisplayMan();
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays a UNIX man page in a quick help dialog.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void DisplayMan(); 
#else
extern void DisplayMan (
    Widget  parent,
    char    *man,
    int     closeOption);
#endif /* _NO_PROTO */


/****************************************************************************
 * Function:         CloseAndExitCB(
 *                              Widget w,
 *                              XtPointer clientData, 
 *                      	XtPointer callData
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Closes the helpview process when the users
 *                  closes either a man page view or a ascii
 *                  text file view.
 *
 ***************************************************************************/
#ifdef _NO_PROTO
extern void CloseAndExitCB(); 
#else
extern void CloseAndExitCB(
    Widget w,
    XtPointer clientData,
    XtPointer callData);
#endif /* _NO_PROTO */



/****************************************************************************
 * Function:         CloseHelpCB(
 *                              Widget w,
 *                              XtPointer clientData, 
 *                      	XtPointer callData
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Process close requests on all Help Dialog widgets 
 *                  created and managed by this application.
 *
 ***************************************************************************/
#ifdef _NO_PROTO
extern void CloseHelpCB (); 
#else
extern void CloseHelpCB (
    Widget w,
    XtPointer clientData,
    XtPointer callData);


#endif /* _NO_PROTO */


/****************************************************************************
 * Function:	    void ProcessLinkCB(
 *                              Widget w,
 *                              XtPointer  clientData, 
 *                      	XtPointer callData
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Process JUMP-NEW and APP-LINK hypertext requests in a 
 *                  given Help Dialog Window.
 *                 
 *                  This is the callback used for the DtNhyperLinkCallback
 *                  on each of the help dialog widges created.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void ProcessLinkCB (); 
#else
extern void ProcessLinkCB (
    Widget w,
    XtPointer clientData,
    XtPointer callData);
#endif /* _NO_PROTO */


/****************************************************************************
 * Function:	    CacheListStruct GetFromCache(
 *                                  Widget parent);
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Gets a free help node form our cache list.  If none are
 *                  free, it will return fallse and the calling routine will
 *                  create a new help dialog widget.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern Boolean GetFromCache();
#else
Boolean GetFromCache(
    Widget parent,
    CacheListStruct **pCurrentNode);
#endif /* _NO_PROTO */





#endif /* _UtilI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */








