/* $XConsortium: VirtFuncsI.h /main/cde1_maint/1 1995/07/17 17:54:06 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   VirtFunctsI.h
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
#ifndef _DtHelpVirtFuncsI_h
#define _DtHelpVirtFuncsI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Private Defines Declarations    ********/

/********    Private Typedef Declarations    ********/

/********    Private Structures Declarations    ********/

/********    Private Define Declarations    ********/

/********    Private Macro Declarations    ********/

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
extern	VStatus	_DtHelpCeLoadGraphic();
extern	VStatus	_DtHelpCeResolveSpc();
extern	Unit	_DtHelpCeGetStringWidth();
extern	void	_DtHelpCeDestroyGraphic();
extern	void	_DtHelpCeDrawString();
extern	void	_DtHelpCeGetFontMetrics();
#else
extern	void	_DtHelpCeDestroyGraphic(
			CECanvasStruct	*canvas,
			GraphicPtr	 graphic_handle);
extern	void	_DtHelpCeDrawString(
			CECanvasStruct	*canvas,
			Unit		 base_x,
			Unit		 base_y,
			char		*string,
			int		 len,
			int		 char_size,
			FontPtr		 font_handle,
			Unit		 box_x,
			Unit		 box_y,
			Unit		 box_height,
			int		 link_type,
			Flags		 old_flag,
			Flags		 new_flag);
extern	void	_DtHelpCeGetFontMetrics(
			CECanvasStruct	*canvas,
			FontPtr		 font_handle,
			Unit		*ret_ascent,
			Unit		*ret_descent,
			Unit		*ret_ave,
			Unit		*ret_super_y,
			Unit		*ret_sub_y);
extern	VStatus	_DtHelpCeLoadGraphic(
			CECanvasStruct	 *canvas,
			char		 *topic_xid,
			CEElement	 *pGraphEl,
			GraphicPtr	 *ret_handle,
			Unit		 *ret_width,
			Unit		 *ret_height);
extern	VStatus	_DtHelpCeResolveSpc(
			CECanvasStruct	 *canvas,
			char		 *spc_name,
			char		 *lang,
			char		 *char_set,
			CanvasFontSpec	 *font_specs,
			CESpecial	**ret_seg);
extern	Unit	_DtHelpCeGetStringWidth(
			CECanvasStruct	 *canvas,
			CESegment	 *segment,
			char		 *lang,
			char		 *charset,
			CanvasFontSpec	 *font_spec,
			char		 *string,
			int		  len);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHelpVirtFuncsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
