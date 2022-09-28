/* $XConsortium: HelpQuickDI.h /main/cde1_maint/1 1995/07/17 17:39:05 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   HelpQuickDI.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Internal header file for QuickHelpDialog Widget.
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
#ifndef _HelpQuickDI_h
#define _HelpQuickDI_h


/* General Define for Quick Help Dialog Widget */

#define DtHELP_TOPIC            1
#define DtHELP_STRING           2
#define DtMAN_TOPIC             3
#define DtFILE_TOPIC            4


/* Access File Expantion Definitions */

#define DtACCESS_PATH_TYPE             "help"
#define DtACCESS_PATH_SUFFIX           ".rp"


#define DtNULL_TOPIC_STRING            "Null Topic"



/*****************************************************************************
 * Function:	    void _DtHelpQuickDialogHypertextCB(
 *                              DtHelpDispAreaStruct *pDisplayAreaStruct,
 *                              XtPointer clientData,
 *                      	DtHyperTextCallbackStruct *hyperData)
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
extern void _DtHelpQuickDialogHypertextCB ();
#else
extern void _DtHelpQuickDialogHypertextCB (
    DtHelpDispAreaStruct *pDisplayAreaStruct,
    XtPointer clientData,
    DtHelpHyperTextStruct *hyperData);
#endif /* _NO_PROTO */





#endif /* _HelpQuickDI_h */
/* Do not add anything after this endif. */
