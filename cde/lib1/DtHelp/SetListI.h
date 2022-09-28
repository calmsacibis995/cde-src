/* $XConsortium: SetListI.h /main/cde1_maint/1 1995/07/17 17:51:51 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SetListTG.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for SetListTG.h
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
#ifndef _DtHelpSetListI_h
#define _DtHelpSetListI_h


#ifdef _NO_PROTO
extern	void	_DtHelpDisplayAreaDimensionsReturn ();
extern	void	_DtHelpDisplayAreaSetList ();
extern	int	_DtHelpGetScrollbarValue ();
extern	Boolean	_DtHelpSetScrollBars ();
#else
extern	void	_DtHelpDisplayAreaDimensionsReturn (
			DtHelpDispAreaStruct    *pDAS,
			short			*ret_rows,
			short			*ret_columns );
extern	void	_DtHelpDisplayAreaSetList (
			DtHelpDispAreaStruct    *pDAS,
			TopicHandle		 topicHandle,
			Boolean			 append_flag,
			int			 scroll_percent);
extern	int	_DtHelpGetScrollbarValue (
			DtHelpDispAreaStruct    *pDAS);
extern	Boolean	_DtHelpSetScrollBars (
			DtHelpDispAreaStruct    *pDAS,
			Dimension	new_width,
			Dimension	new_height );
#endif

#endif /* _DtHelpSetListI_h */
