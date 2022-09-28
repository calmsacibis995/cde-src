/* $XConsortium: PrintI.h /main/cde1_maint/1 1995/07/17 17:49:56 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   PrintI.h
 **
 **  Project:  Cache Creek (Rivers) Project:
 **
 **  Description:  Internal Header file for Print.c 
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
#ifndef _PrintI_h
#define _PrintI_h

/* Printer Type Defines */

#define _DtPRINT_ALL     1
#define _DtPRINT_SUB     2
#define _DtPRINT_CURRENT 3
#define _DtPRINT_TOC     4 

/* default helpprint executable */
extern char _DtHelpDefaultHelpPrint[];
/* paper size resource names */
extern char * _DtHelpPaperSizeNames[];
/* number paper size resource names */
extern int    _DtHelpPaperSizeNamesCnt;

/*****************************************************************************
 * Function:        void _DtHelpDisplayPrintDialog();
 *
 *
 * Parameters:      new      Specifies the help widget.
 *
 * Return Value:
 *
 * Purpose:         Creates and displays an instance of the print dialog.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpDisplayPrintDialog();
#else
void _DtHelpDisplayPrintDialog(
   Widget               widget,
   _DtHelpPrintStuff *   print,
   _DtHelpDisplayWidgetStuff * display,
   _DtHelpCommonHelpStuff * help);
#endif

/*****************************************************************************
 * Function:        void _DtHelpUpdatePrintDialog();
 *
 *
 * Parameters:      new      Specifies the help widget.
 *
 * Return Value:
 *
 * Purpose:         Updates the print dialog to reflect current topic
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpUpdatePrintDialog();
#else
void _DtHelpUpdatePrintDialog(
    _DtHelpPrintStuff *         print,
    _DtHelpDisplayWidgetStuff * display,
    _DtHelpCommonHelpStuff *    help,
    Boolean                     setDefaults);
#endif

/************************************************************************
 * Function: _DtHelpPrintSetValues()
 *
 *      set print-related data
 *
 ************************************************************************/
#ifdef _NO_PROTO
void _DtHelpPrintSetValues ();
#else
void _DtHelpPrintSetValues (
    _DtHelpPrintStuff * currentPrint,
    _DtHelpPrintStuff * newPrint,
    _DtHelpDisplayWidgetStuff * newDisplay,
    _DtHelpCommonHelpStuff * newCommonHelp);
#endif /* _NO_PROTO */


/*****************************************************************************
 * Function:	    void _DtHelpPrintJob(
 *                   
 *                            
 *
 * Parameters:  
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Sets up and forks off a print job to helpprint
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpPrintJob();
#else
void _DtHelpPrintJob(
    Widget	widget,
    char *	printExec,
    char *	printer,
    int		paperSize,
    char *	copies,
    char *	helpVolume,
    int 	helpType,
    char *	helpData,
    Boolean	printAll,
    Boolean	printSub,
    Boolean	printToc,
    char *	topicTitle);
#endif /* _NO_PROTO */


#endif /* _PrintI_h */
/* Do not add anything after this endif. */






