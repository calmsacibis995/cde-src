/* $XConsortium: LayoutUtilI.h /main/cde1_maint/1 1995/07/17 17:46:27 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:       LayoutUtilI.h
 **
 **   Project:    Cde DtHelp
 **
 **   Description:
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
#ifndef _CELayoutUtilI_h
#define _CELayoutUtilI_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Private Defines Declarations    ********/
#define	_CEFORMAT_ALL	-1

/********    Private Typedef Declarations    ********/

/********    Private Structures Declarations    ********/
typedef	struct	_ceLayoutStruct {
	CESegment	*line_seg;
	unsigned int	 line_start;
	unsigned int	 line_bytes;
	Unit		 cur_len;
	Unit		 max_x_pos;
	Unit		 y_pos;
	Unit		 text_x_pos;
	Unit		 label_x_pos;
	int		*gr_list;
	int		 gr_cnt;
	int		 gr_max;
	int		 lst_hyper;
	int		 format_y;
	CEBoolean	 lst_vis;
	CEBoolean	 join;
} CELayoutInfo;

/*****************************************************************************
 *			Private Macro Declarations
 *****************************************************************************/

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
extern	void		_DtHelpCeAddLines ();
extern	void		_DtHelpCeAddToInLineList ();
extern	int		_DtHelpCeAllocGraphicStruct();
extern	void		_DtHelpCeCheckAddToHyperList ();
extern	CEBoolean	_DtHelpCeCheckLineSyntax ();
extern	CEBoolean	_DtHelpCeCheckOneByteCantBeginList ();
extern	CEBoolean	_DtHelpCeCheckOneByteCantEndList ();
extern	int		_DtHelpCeGetNextWidth ();
extern	int		_DtHelpCeGetTraversalWidth ();
extern	void		_DtHelpCeInitLayoutInfo ();
extern	void		_DtHelpCeLoadMultiInfo ();
extern	int		_DtHelpCeProcessStringSegment();
extern	void		_DtHelpCeSaveInfo ();
#else
extern	void		_DtHelpCeAddLines (
				CECanvasStruct	*canvas,
				int		 number,
				CEBoolean	 force,
				int		*ret_y);
extern	void		_DtHelpCeAddToInLineList (
				CELayoutInfo	*info,
				int		 index);
extern	int		_DtHelpCeAllocGraphicStruct (
				CECanvasStruct      *canvas);
extern	void		_DtHelpCeCheckAddToHyperList (
				CECanvasStruct      *canvas,
				CESegment           *p_seg,
				CEBoolean           *lst_vis,
				int                 *lst_hyper,
				Unit                *cur_len);
extern	CEBoolean	_DtHelpCeCheckLineSyntax (
				CECanvasStruct	 *canvas,
				CESegment	 *pSeg,
				int		  start,
				int		  str_len);
extern	CEBoolean	_DtHelpCeCheckOneByteCantBeginList (
				char		*string,
				char		*cant_begin_list);
extern	CEBoolean	_DtHelpCeCheckOneByteCantEndList (
				char		*string,
				char		*cant_end_list);
extern	int		_DtHelpCeGetNextWidth (
				CECanvasStruct	 *canvas,
				int		  old_type,
				int		  lst_hyper,
				CESegment	 *pSeg,
				int		  start,
				char		 *lang,
				char		 *char_set,
				CanvasFontSpec	 *font_spec,
				CESegment	 *prev_seg,
				CESegment	 *toss,
				CEElement	 *snb,
				CESegment	**nextSeg,
				int		 *nextStart,
				int		 *widthCount);
extern	int		_DtHelpCeGetTraversalWidth (
				CECanvasStruct	 *canvas,
				CESegment        *p_seg,
				int		  lst_hyper);
extern	void		_DtHelpCeInitLayoutInfo (
				CECanvasStruct	 *canvas,
				CELayoutInfo	 *layout);
extern	void		_DtHelpCeLoadMultiInfo (
				CECanvasStruct	 *canvas);
extern	int		_DtHelpCeProcessStringSegment(
				CECanvasStruct	 *canvas,
				CELayoutInfo	 *lay_info,
				Unit		  max_width,
				Unit		  l_margin,
				Unit		  r_margin,
				CESegment	 *toss,
				CEElement	 *snb,
				CESegment	 *cur_seg,
				unsigned int	 *cur_start,
				char		 *lang,
				char		 *char_set,
				CanvasFontSpec	 *font_specs,
				enum SdlOption	  txt_justify,
				CEBoolean	  stat_flag);
extern	void		_DtHelpCeSaveInfo (
				CECanvasStruct	 *canvas,
				CELayoutInfo	 *layout,
				Unit              max_width,
				Unit              r_margin,
				enum SdlOption    txt_justify);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _CELayoutUtilI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
