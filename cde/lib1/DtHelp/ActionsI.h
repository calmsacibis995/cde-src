/* $XConsortium: ActionsI.h /main/cde1_maint/1 1995/07/17 17:20:17 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ActionsI.h
 **
 **   Project:     Display area routines
 **
 **   Description: Header file for Actions.c
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpActionsI_h
#define _DtHelpActionsI_h

/*****************************************************************************
 *              Semi Public Routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	void	_DtHelpActivateLink ();
extern	void	_DtHelpCopyAction ();
extern	void	_DtHelpDeSelectAll ();
extern	void	_DtHelpNextLink ();
extern	void	_DtHelpPageLeftOrRight ();
extern	void	_DtHelpPageUpOrDown ();
extern	void	_DtHelpSelectAll ();
#else
extern	void	_DtHelpActivateLink (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpCopyAction (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpDeSelectAll (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpNextLink (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpPageLeftOrRight (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpPageUpOrDown (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
extern	void	_DtHelpSelectAll (
			Widget		widget,
			XEvent		*event,
			String		*params,
			Cardinal	*num_params);
#endif

#endif /* _DtHelpActionsI_h */
