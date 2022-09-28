/* $XConsortium: CallbacksI.h /main/cde1_maint/1 1995/07/17 17:22:50 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        CallbacksTG.h
 **
 **   Project:     TextGraphic Display routines
 **
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
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpCallbacksI_h
#define _DtHelpCallbacksI_h

/*****************************************************************************
 *              Semi Public Routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	Boolean	_DtHelpCancelSelection();
extern	void	_DtHelpCleanAndDrawWholeCanvas();
#else
extern	Boolean	_DtHelpCancelSelection(
			DtHelpDispAreaStruct *pDAS);
extern	void	_DtHelpCleanAndDrawWholeCanvas(
			DtHelpDispAreaStruct *pDAS);
#endif

/*****************************************************************************
 *              Public Routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	void	_DtHelpClearSelection ();
extern	void	_DtHelpClickOrSelectCB ();
extern	void	_DtHelpEndSelectionCB ();
extern  void    _DtHelpEnterLeaveCB ();
extern	void	_DtHelpExposeCB ();
extern	void	_DtHelpFocusCB ();
extern	void	_DtHelpGetClearSelection ();
extern	void	_DtHelpHorzScrollCB ();
extern	void	_DtHelpInitiateClipboard ();
extern	void    _DtHelpLoseSelectionCB ();
extern	void	_DtHelpMoveBtnFocusCB ();
extern	void	_DtHelpMouseMoveCB ();
extern	void	_DtHelpResizeCB ();
extern	void	_DtHelpVertScrollCB ();
extern	void	_DtHelpVisibilityCB ();

#else

extern	void	_DtHelpClearSelection ( DtHelpDispAreaStruct *pDAS );
extern	void	_DtHelpClickOrSelectCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern	void	_DtHelpEndSelectionCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern  void    _DtHelpEnterLeaveCB (
			Widget          widget,
			XtPointer       client_data,
			XEvent          *event );
extern	void	_DtHelpExposeCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern	void	_DtHelpFocusCB (
			Widget		widget,
			XtPointer	client_data,
			XEvent		*event );
extern	void	_DtHelpGetClearSelection (
			Widget		widget,
			XtPointer	client_data);
extern	void	_DtHelpHorzScrollCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern	void	_DtHelpInitiateClipboard ( DtHelpDispAreaStruct *pDAS );
extern	void    _DtHelpLoseSelectionCB (
			Widget   widget,
			Atom    *selection );
extern	void	_DtHelpMoveBtnFocusCB (
			Widget		widget,
			XtPointer	client_data,
			XEvent		*event );
extern	void	_DtHelpMouseMoveCB (
			Widget		widget,
			XtPointer	client_data,
			XEvent		*event );
extern	void	_DtHelpResizeCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern	void	_DtHelpVertScrollCB (
			Widget		widget,
			XtPointer	client_data,
			XtPointer	call_data );
extern	void	_DtHelpVisibilityCB (
			Widget		widget,
			XtPointer	client_data,
			XEvent		*event );
#endif

#endif /* _DtHelpCallbacksI_h */
