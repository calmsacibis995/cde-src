/* $XConsortium: HistoryI.h /main/cde1_maint/1 1995/07/17 17:42:04 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   HistoryI.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Internal header file for HistoryI.c 
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
#ifndef _HistoryI_h
#define _HistoryI_h


#define DtHISTORY_LIST_MAX     999   /* ??? This should be a resource ??? */




/*****************************************************************************
 * Function:	    void _DtHelpDisplayHistoryInfo(Widget nw);
 *                             
 * 
 * Parameters:      nw        Specifies the name of the current help dialog 
 *                            widget.
 *
 * Return Value:
 *
 * Purpose: 	    Displays the pre-created history dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpDisplayHistoryInfo();
#else
extern void _DtHelpDisplayHistoryInfo(
    Widget nw);
#endif 



/*****************************************************************************
 * Function:	    void _DtHelpUpdateHistoryList(char *locationId,
 *                                 Widget nw);  
 *
 *
 * Parameters:      parent      Specifies the ID string for the new topic we
 *                              are going to display in the HelpDialog widget.
 *
 *                  helpDialogWidget  Specifies the current help dialog widget.
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Updates the Path Display area on top of the help
 *                  dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpUpdateHistoryList();
#else
extern void _DtHelpUpdateHistoryList(
    char *locationId,
    int topicType,
    Widget nw);
#endif 




#endif /* _HistoryI_h */
/* Do not add anything after this endif. */











