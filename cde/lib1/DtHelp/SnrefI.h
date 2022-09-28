/* $XConsortium: SnrefI.h /main/cde1_maint/1 1995/07/17 17:52:26 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   SnrefI.h
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
#ifndef _DtHelpSnrefI_h
#define _DtHelpSnrefI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Semi-Public Function Declarations    ********/

#ifdef _NO_PROTO
extern	void		 _DtHelpCeResolveSdlSnref();
extern	CESegment	*_DtHelpCeResolveAsyncBlocks();
#else
extern	CESegment	*_DtHelpCeResolveAsyncBlocks(
				CECanvasStruct	*canvas,
				CESegment	*toss,
				CEElement	*parent,
				char		*id);
extern	void		 _DtHelpCeResolveSdlSnref(
				CECanvasStruct	 *canvas,
				CESegment	 *toss,
				CEElement	 *snb_el,
				CESegment	 *snref_seg,
				CESegment	 *lst_disp);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpSnrefI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
