/* $XConsortium: FormatManI.h /main/cde1_maint/2 1995/10/08 17:18:16 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FormatManI.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for FormatMan.c
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
#ifndef _DtFormatManI_h
#define _DtFormatManI_h

#ifdef _NO_PROTO
extern	int	_DtHelpFormatManPage ();
#else
extern	int	_DtHelpFormatManPage (
			CanvasHandle  canvas_handle,
			char          *man_spec,
			TopicHandle   *ret_handle );
#endif

#endif /* _DtFormatManI_h */
