/* $XConsortium: HelpCacheI.h /main/cde1_maint/1 1995/07/17 20:59:48 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **  File:        HelpCacheI.h
 **
 **  Project:     CDE dthelpdemo sample program.
 **
 **  Description: This is the header file for the HelpCache.c module.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994
 **      Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _HelpCacheI_h
#define _HelpCacheI_h



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
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Creats and displays a new help dialog w/the requested help
 *                  volume and topic.
 *
 ****************************************************************************/
extern void DisplayTopic(
    Widget  parent,
    char    *helpVolume,
    char    *locationId);




/****************************************************************************
 * Function:	    void DisplayMain(
 *                              Widget parent,
 *                              char *helpVolume,
 *                              char *locationId)
 *
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays help for helpdemo in the one helpDialog window 
 *                  created for the applicaiton.
 *
 ****************************************************************************/
extern void DisplayMain( 
    Widget  parent,
    char    *helpVolume,
    char    *locationId);



/****************************************************************************
 * Function:	    void DisplayVersion(
 *                              Widget parent,
 *                              char *helpVolume,
 *                              char *locationId)
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays the version dialog for the helpdemo program.
 *
 ****************************************************************************/
extern void DisplayVersion( 
    Widget  parent,
    char    *helpVolume,
    char    *locationId);



/*****************************************************************************
 * Function:	    void HelpMapCB()
 *                   
 *                            
 *
 * Parameters:      client_data is the widget in reference to
 *                  which widget w is placed
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Determins where a new child dialog should be mapped in
 *                  relation to its parent.
 *
 * Algorithm:       1. attempt left or right placement with no overlap
 *                  2. if fails, attempt up or down placement with no overlap
 *                  3. if fails, determines location with least
 *                     amount of overlap, and places there.
 *
 *****************************************************************************/
extern XtCallbackProc HelpMapCB(
    Widget w,
    XtPointer clientData,
    XtPointer callData);



/****************************************************************************
 * Function:	    void DisplayMan()
 *
 * Parameters:      
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Displays a UNIX man page in a quick help dialog.
 *
 ****************************************************************************/
extern void DisplayMan(
    Widget  parent,
    char    *man);



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
extern void ProcessLinkCB (
    Widget w,
    XtPointer clientData,
    XtPointer callData);


#endif /* _HelpCacheI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
















