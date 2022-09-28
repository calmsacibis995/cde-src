/* $XConsortium: FormatManCCDFI.h /main/cde1_maint/1 1995/07/17 17:31:52 drk $ */

/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FormatManCCDFI.h
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
#ifndef _DtHelpFormatManCCDFI_h
#define _DtHelpFormatManCCDFI_h

#ifndef	_CEFormatManCCDF
#define	_CEFormatManCCDF(canvas, spec, ret_handle) \
		_DtHelpCeFormatManCcdf(canvas, spec, ret_handle)
#endif

#ifdef _NO_PROTO
extern	int	_DtHelpCeFormatManCcdf ();
#else
extern	int	_DtHelpCeFormatManCcdf (
			CanvasHandle   canvas_handle,
			char          *man_spec,
			TopicHandle   *ret_handle );
#endif

#endif /* _DtHelpFormatManCCDFI_h */
