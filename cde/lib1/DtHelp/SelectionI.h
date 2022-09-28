/* $XConsortium: SelectionI.h /main/cde1_maint/2 1995/10/08 17:21:10 pascale $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   SelectionI.h
 **
 **  Project:
 **
 **  Description:  Public Header file for Canvas.c
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
#ifndef _CESelectionI_h
#define _CESelectionI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Private Defines Declarations    ********/

/********    Private Typedef Declarations    ********/

/********    Private Structures Declarations    ********/

/********    Private Macro Declarations    ********/

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
extern	void	_DtHelpCeDrawSelection ();
#else
extern	void	_DtHelpCeDrawSelection (
			CECanvasStruct	*canvas,
			Unit		 y1,
			Unit		 y2);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _CESelectionI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
