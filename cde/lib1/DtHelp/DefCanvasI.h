/* $XConsortium: DefCanvasI.h /main/cde1_maint/1 1995/07/17 17:25:32 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   DefCanvasI.h
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
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpDefCanvasI_h
#define _DtHelpDefCanvasI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern	CanvasHandle	_DtHelpCeCreateDefCanvas();
#else
extern	CanvasHandle	_DtHelpCeCreateDefCanvas (void);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _DtHelpDefCanvasI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
