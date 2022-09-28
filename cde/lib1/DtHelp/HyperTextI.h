/* $XConsortium: HyperTextI.h /main/cde1_maint/1 1995/07/17 17:43:17 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        HyperText.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for HyperText.h
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
#ifndef _DtHyperTextI_h
#define _DtHyperTextI_h

/*****************************************************************************
 * Semi-private routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	int	 __DtLinkAddToList ();
extern	void	 __DtLinkRemoveFromList ();
extern	char	*__DtLinkStringReturn ();
extern	int	 __DtLinkTypeReturn ();
#else
extern	int	 __DtLinkAddToList (
			char *link,
			int   type,
			char *description);
extern	void	 __DtLinkRemoveFromList ( int link_index );
extern	char	*__DtLinkStringReturn ( int link_index );
extern	int	 __DtLinkTypeReturn ( int link_index );
#endif

#ifdef _NO_PROTO
extern	void	_DtHelpExecProcedure ();
extern	void	_DtHelpProcessHyperSelection ();
#else
extern	void	_DtHelpExecProcedure (
			DtHelpDispAreaStruct    *pDAS,
			char *cmd );
extern	void	_DtHelpProcessHyperSelection (
			DtHelpDispAreaStruct       *pDAS,
			int              downX,
			int              downY,
			XEvent		*event );
#endif

#endif /* _DtHyperTextI_h */
