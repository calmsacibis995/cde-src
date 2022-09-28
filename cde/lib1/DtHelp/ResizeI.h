/* $XConsortium: ResizeI.h /main/cde1_maint/1 1995/07/17 17:50:25 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ResizeI.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for CallbacksTG.h
 **
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
#ifndef	_DtResizeI_h
#define	_DtResizeI_h

/*****************************************************************************
 *                 Defines
 *****************************************************************************/
/*****************************************************************************
 *                 Variables Global to the library.
 *****************************************************************************/
/*****************************************************************************
 *                 Routine Declarations
 *****************************************************************************/
#ifdef	_NO_PROTO
extern	void	_DtHelpResizeDisplayArea ();
#else
extern	void	_DtHelpResizeDisplayArea (
			Widget	parent,
			DtHelpDispAreaStruct *pDAS,
			int	rows,
			int	columns );
#endif

#endif /* _DtResizeI_h */
