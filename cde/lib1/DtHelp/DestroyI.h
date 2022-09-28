/* $XConsortium: DestroyI.h /main/cde1_maint/1 1995/07/17 17:26:14 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        DestroyTG.h
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
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpDestroyI_h
#define _DtHelpDestroyI_h

#ifdef	_NO_PROTO
extern	void	_DtHelpDeallocateParagraphs ();
extern	void	_DtHelpDisplayAreaClean ();
extern	void	_DtHelpDisplayAreaDestroyCB ();
#else
extern	void	_DtHelpDisplayAreaClean (
			DtHelpDispAreaStruct  *pDAS);
extern	void	_DtHelpDisplayAreaDestroyCB (
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data);
#endif

#endif /* _DtHelpDestroyI_h */
