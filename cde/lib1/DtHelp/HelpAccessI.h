/* $XConsortium: HelpAccessI.h /main/cde1_maint/1 1995/07/17 17:35:26 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   HelpAccessI.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Internal header file for HelpAccess.c
 **  -----------
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
#ifndef _HelpAccessI_h
#define _HelpAccessI_h




/* The following Defines are for all the F1 key help requests */

/* Help Dialog */
#define DtHELP_dialogShell_STR  "_HOMETOPIC"

/* Menubar help access */
#define DtHELP_onHelpMenu_STR                "_HOMETOPIC"
#define DtHELP_onTableOfContenseMenu_STR     "TABLE-OF-CONTENTS"
#define DtHELP_onTasksMenu_STR               "TASKS"
#define DtHELP_onReferenceMenu_STR           "REFERENCE"
#define DtHELP_onMouseAndKeysMenu_STR        "MOUSE-AND-KEYBOARD"
#define DtHELP_onVersionMenu_STR             "_COPYRIGHT"

#define DtHELP_ExecutionPolicy_STR           "EXECUTION-POLICY"

/* Menu Item Id's (F1 Key) */
#define DtHELP_FileMenu_STR           "HELP-FILE-MENU"
#define DtHELP_EditMenu_STR           "HELP-EDIT-MENU"
#define DtHELP_SearchMenu_STR         "HELP-SEARCH-MENU"
#define DtHELP_NavigateMenu_STR       "HELP-NAVIGATE-MENU"
#define DtHELP_HelpMenu_STR           "HELP-HELP-MENU"
#define DtHELP_PopupMenu_STR          "HELP-POPUP-MENU"

/* Quick Help Dialog help access defines */
#define DtHELP_quickHelpShell_STR   "QUICK-HELP"
#define DtHELP_quickHelpBtn_STR     "QUICK-HELP"

/* History Dialog help access defines */
#define DtHELP_historyShell_STR       "HISTORY"
#define DtHELP_historyHelpBtn_STR     "HISTORY"

/* Printing Dialog help access defines */
#define DtHELP_printShell_STR       "PRINTING"
#define DtHELP_printHelpBtn_STR     "PRINTING"

/* Index Search Dialog help access defines */
#define DtHELP_srchIndexShell_STR   "INDEX-SEARCH"
#define DtHELP_srchIndexHelpBtn_STR "INDEX-SEARCH"

/* Volume Selection Dialog help access defines */
#define DtHELP_volSelectShell_STR    "VOLUME-SELECT"
#define DtHELP_volSelectHelpBtn_STR  "VOLUME-SELECT"



/****************************************************************************
 * Function:          void  _DtHelpCB (widget, clientData, callData) 
 *                                    Widget	widget;
 *                                    XtPointer	clientData;
 *                                    XtPointer   callData;
 * 
 * Parameters:           widget
 *                       clientData
 *                       callData
 *   
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Catches any "F1" key presses in a help dialog widget, or 
 *                  selects any of the help buttons in any of the help dialogs
 *                  dialogs.
 *
 ***************************************************************************/
#ifdef _NO_PROTO
extern void  _DtHelpCB (); 
#else
extern void  _DtHelpCB(
    Widget widget,
    XtPointer clientData,
    XtPointer callData);
#endif /* _NO_PROTO */





/*****************************************************************************
 * Function:	    void _DtHelpListFree (DtHelpListStruct *pHead);  
 *
 *
 * Parameters:      pHead   Specifies the head pointer to the help list.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Frees all elements in a Help List.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpListFree();
#else
extern void _DtHelpListFree(
    DtHelpListStruct * * pHead);
#endif 





/*****************************************************************************
 * Function:	    DtHelpListStruct * _DtHelpListAdd(
 *                                       char *locationId,
 *                                       Widget widget,
 *                                       int widgetType,
 *                                       DtHelpListStruct *pHead)
 *                            
 *
 * Parameters:  
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Adds an element to the top of the given topicList.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern DtHelpListStruct * _DtHelpListAdd();
#else
extern DtHelpListStruct * _DtHelpListAdd(
    char *locationId,
    Widget widget,
    _DtHelpCommonHelpStuff * help,
    DtHelpListStruct **pHead);
#endif 



#endif /* _HelpAccessI_h */
/* Do not add anything after this endif. */

