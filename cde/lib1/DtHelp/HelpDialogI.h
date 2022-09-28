/* $XConsortium: HelpDialogI.h /main/cde1_maint/1 1995/07/17 17:36:23 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   HelpDialogI.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Internal header file for HelpDialog Widget.
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
#ifndef _HelpDialogI_h
#define _HelpDialogI_h

#include <Dt/HelpDialog.h>
#include "HelpDialogP.h"


/* Update list defines use in _DtHelpUpdateDisplayArea */

#define DtHISTORY_AND_JUMP      1
#define DtHISTORY_ONLY          2
#define DtJUMP_ONLY             3


/* Defines for processing a request to "Top Level..." */
#define DtBROWSER_HOME          _DtHelpDefaultLocationId
#define DtBROWSER_NAME          "browser.hv"


/* Jump List Max */

#define DtJUMP_LIST_MAX         999



/****************************************************************
 *
 *  Current Color Info Structure Definition
 *
 ****************************************************************/
typedef	struct	_colorStruct {
    Pixel	background;
    Pixel	foreground;
    Pixel	topShadow;
    Pixel	bottomShadow;
    Pixel	select;
} ColorStruct;


/*****************************************************************************
 * Function:	     void _DtHelpUpdateDisplayArea(
 *                             char                  *locationId,
 *                             DtHelpDialogWidget   nw,
 *                             int                   listUpdateType, 
 *                             int                   topicUpdateType);
 *
 *
 * Parameters:      locationId
 *
 *                  nw
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Updates the History List for the current help dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpUpdateDisplayArea();
#else
extern void _DtHelpUpdateDisplayArea(
    char *locationId,
    DtHelpDialogWidget nw,
    int listUpdateType,
    int topicUpdateType);
#endif 


/*****************************************************************************
 * Function:	     void _DtHelpUpdateJumpList(
 *                                  char *topicInfo,
 *                             DtHelpDialogWidget nw);
 *
 *
 * Parameters:      topicInfo
 *
 *                  nw
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Updates the History List for the current help dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpUpdateJumpList();
#else
extern void _DtHelpUpdateJumpList(
    char *topicInfo,
    int topicType,
    Widget nw);
#endif 


/*****************************************************************************
 * Function:	    void _DtHelpDialogHypertextCB(
 *                              DtHelpDispAreaStruct *pDisplayAreaStruct,
 *                              XtPointer clientData,
 *                      	DtHelpHyperTextStruct *hyperData)
 *
 * Parameters:      pDisplayAreaStruct  Specifies the curretn display are info.
 *
 *                  hyperData           Specifies the current hypertext info
 *                                      structure.
 *
 *                  clientData          Specifies the client data passed into
 *                                      the hypertext callback. 
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Process all hypertext requests in a given Help Dialogs
 *                  display area.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDialogHypertextCB ();
#else
extern void _DtHelpDialogHypertextCB (
    DtHelpDispAreaStruct *pDisplayAreaStruct,
    XtPointer clientData,
    DtHelpHyperTextStruct *hyperData);
#endif 




/*****************************************************************************
 * Function:	    void _DtHelpSetupDisplayType(DtHelpDialogWidget nw);  
 *                                            int updateType);  
 *
 *
 * Parameters:      nw          Specifies the current help dialog widget.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Determins the type of topic the user want's to display
 *                  in the current help dialog and sets it up for display.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpSetupDisplayType();
#else
extern void _DtHelpSetupDisplayType(
     DtHelpDialogWidget nw,
     int updateType);
#endif 


/*****************************************************************************
 * Function:	    void _DtHelpTopicListFree (DtTopicListStruct *pHead);  
 *
 *
 * Parameters:      pHead   Specifies the head pointer to the topic list.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Frees all elements in a Topic List.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpTopicListFree();
#else
extern void _DtHelpTopicListFree(
    DtTopicListStruct *pHead);
#endif 


/*****************************************************************************
 * Function:	   extern void DisplayHistoryCB(
 *                            Widget w,   
 *                            XtPointer client_data,
 *                            XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine will cause the history dialog to be 
 *                 posted.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDisplayHistoryCB();
#else
extern void  _DtHelpDisplayHistoryCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data );
#endif




/*****************************************************************************
 * Function:	   extern void _DtHelpDisplayBackCB(
 *                            Widget w,   
 *                            XtPointer client_data,
 *                            XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine will cause the top element in the jump
 *                 stack to be displayed.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDisplayBackCB();
#else
extern void  _DtHelpDisplayBackCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data);
#endif



/*****************************************************************************
 * Function:	   extern void _DtHelpDuplicateWindowCB(
 *                             Widget w,   
 *                             XtPointer client_data,
 *                             XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine creates a new help widget by forcing the
 *                 equivelent of a Jump New hyper text call
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDuplicateWindowCB();
#else
extern void  _DtHelpDuplicateWindowCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data);
#endif

/*****************************************************************************
 * Function:	       void _DtHelpDisplayIndexCB(
 *                            Widget w,   
 *                            XtPointer client_data,
 *                            XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine will cause the index dialog to be 
 *                 posted.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDisplayIndexCB();
#else
extern void  _DtHelpDisplayIndexCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data);
#endif /* _NO_PROTO */


/*****************************************************************************
 * Function:	       void  _DtHelpDisplayBrowserHomeCB(
 *                            Widget w,   
 *                            XtPointer client_data,
 *                            XtPointer call_data);
 *       
 * Parameters:   
 *
 * Return Value:    
 *
 * Purpose: 	   This routine will cause the top level of the browser.hv 
 *                 help volume to be displayed.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
extern void  _DtHelpDisplayBrowserHomeCB();
#else
extern void  _DtHelpDisplayBrowserHomeCB(
    Widget w,
    XtPointer client_data,
    XtPointer call_data);
#endif /* _NO_PROTO */

/*****************************************************************************
 * Function:        _DtHelpFilterExecCmdCB
 *
 *  clientData:     The general help dialog widget
 *  cmdStr:         cmd string to filter
 *  ret_filteredCmdStr:  string after filtering.  NULL if exec denied
 *
 * Return Value:    0: ok, < 0: error
 *
 * Purpose:         filter an execution command using executionPolicy rsrc
 *
 * Memory:
 *   The caller must free memory allocated for the ret_filteredCmdStr
 *****************************************************************************/
#ifdef _NO_PROTO
extern int _DtHelpFilterExecCmdCB();
#else
extern int _DtHelpFilterExecCmdCB(
    void *   clientData,
    const char *   cmdStr,
    char * * ret_filteredCmdStr);
#endif

#endif /* _HelpDialogI_h */
/* Do not add anything after this endif. */
