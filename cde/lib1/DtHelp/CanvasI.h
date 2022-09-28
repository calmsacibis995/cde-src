/* $XConsortium: CanvasI.h /main/cde1_maint/1 1995/07/17 17:23:59 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   CanvasI.h
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
#ifndef _CECanvasI_h
#define _CECanvasI_h


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/********    Private Defines Declarations    ********/

/********    Private Typedef Declarations    ********/

/********    Private Structures Declarations    ********/

typedef	struct {
	CEBoolean	 lnk_flag;
	CEBoolean	 processed;
	Unit		 text_x;
	Unit		 label_x;
	Unit		 baseline;
	Unit		 descent;
	Unit		 ascent;
	int		 byte_index;
	int		 length;
	CESegment	*seg_ptr;
} CELines;

typedef	struct {
	int		 h_indx;
	Unit		 x_pos;
	Unit		 y_pos;
	Unit		 width;
	Unit		 height;
	CESegment	*seg_ptr;
} CELinkSegments;

typedef	struct	{
	short	dir;
	Unit	pos_x;
	Unit	pos_y;
	Unit	length;
} CELineSegment;

typedef	struct _dthelpceselectstruct {
	Unit	x;
	Unit	y;
	int	line_idx;
	int	char_idx;
} DtHelpCeSelectStruct;

typedef	struct {
	int		error;

	CEBoolean	toc_flag;	/* Does this contain the TOC?   */
	CEBoolean	toc_on;		/* Is the marker On or Off?     */
	int		toc_loc;	/* table of contents indicator	*/

	int		para_count;	/* number of paragraphs		*/

	int		txt_cnt;	/* maximum used in txt_list	*/
	int		txt_max;	/* maximum in txt_list		*/
	int		max_x_cnt;	/* maximum used in max_x_pos	*/

	int		line_cnt;	/* maximum used in line_lst	*/
	int		line_max;	/* maximum in line_lst		*/

	int		graphic_cnt;	/* maximum used in graphic_lst	*/
	int		graphic_max;	/* maximum in graphic_list	*/

	int		hyper_count;	/* maximum used in hyper_list	*/
	int		hyper_max;	/* maximum in hyper_list	*/
	int		cur_hyper;	/* traversal indicator		*/

	short		mb_length;	/* The maximum length of a char */
	short		nl_to_space;	/* Are newlines turned into spaces? */
        wchar_t        *multi_cant_end; /* the list of multibyte characters
					   that can't end a line        */
	wchar_t        *multi_cant_begin; /* the list of multibyte characters
					     that can't begin a line    */

	char			*name;
	VolumeHandle	        volume;
	ClientData		client_data;
	CanvasMetrics		metrics;
	CanvasFontMetrics	font_info;
	CanvasLinkMetrics	link_info;
	CanvasTraversalMetrics	traversal_info;

	CESegment		*element_lst;
	CEParagraph		*para_lst; /* contains pre-formatted segments */
	CELines			*txt_lst;
	CELineSegment		*line_lst;
	Unit			*max_x_lst;
	CEGraphics		*graphic_lst;
	CELinkSegments		*hyper_segments;
	CELinkData		 link_data;
	DtHelpCeSelectStruct	 select_start;
	DtHelpCeSelectStruct	 select_end;
	VirtualInfo		 virt_functions;

} CECanvasStruct;

/********    Private Define Declarations    ********/
#define	CELineHorizontal	0
#define	CELineVertical		1

/*****************************************************************************
 *			Private Macro Declarations
 *****************************************************************************/
#ifndef	_CEGraphicStructPtr
#define	_CEGraphicStructPtr(x,y)	(&((x)->graphic_lst[(y)]))
#endif

#ifndef	_CEGraphicStruct
#define	_CEGraphicStruct(x,y)		((x)->graphic_lst[(y)])
#endif

#ifndef	_CEGraphicStructType
#define	_CEGraphicStructType(x,y)	(((x)->graphic_lst[(y)]).type)
#endif

#ifndef	_CEGraphicIdxOfSeg
#define	_CEGraphicIdxOfSeg(x)		((x)->seg_handle.grph_idx)
#endif

#ifndef	_CEGraphicWidth
#define	_CEGraphicWidth(x,y)		((x)->graphic_lst[(y)].width)
#endif

#ifndef	_CEGraphicHeight
#define	_CEGraphicHeight(x,y)		((x)->graphic_lst[(y)].height)
#endif

#ifndef	_CEGraphicYPos
#define	_CEGraphicYPos(x,y)		((x)->graphic_lst[(y)].pos_y)
#endif

#ifndef	_CEGraphicLnk
#define	_CEGraphicLnk(x,y)		((x)->graphic_lst[(y)].lnk_idx)
#endif

/********    Private Function Declarations    ********/

#ifdef _NO_PROTO
extern	Unit		_DtHelpCeAdvanceXOfLine ();
extern	void		_DtHelpCeDetermineWidthOfSegment();
extern	void		_DtHelpCeDrawGraphics();
extern	Unit		_DtHelpCeDrawSegments();
extern	Unit		_DtHelpCeDrawText();
extern	void		_DtHelpCeFreeGraphics();
extern	void		_DtHelpCeFreeParagraphs();
extern	void		_DtHelpCeFreeSegList();
extern	int		_DtHelpCeGetCharIdx();
extern	int		_DtHelpCeGetLineAtY();
extern	Unit		_DtHelpCeGetStartXOfLine();
extern	CEBoolean	_DtHelpCeModifyXpos();
extern	Unit		_DtHelpCeStartXOfLine();
#else
extern	Unit		_DtHelpCeAdvanceXOfLine (
				CECanvasStruct   *canvas,
				CELines		  line,
				CESegment        *p_seg,
				Unit		  x_pos,
				int		*link_idx,
				CEBoolean	*label_flag,
				CEBoolean	*link_flag);
extern	void		_DtHelpCeDetermineWidthOfSegment(
				CECanvasStruct	 *canvas,
				CESegment	 *p_seg,
				int		  start,
				int		  max_cnt,
				int		 *ret_cnt,
				Unit		 *ret_w,
				CEBoolean	 *ret_trimmed);
extern	void		_DtHelpCeDrawGraphics(
				CECanvasStruct	 *canvas,
				Unit		  x1,
				Unit		  y1,
				Unit		  x2,
				Unit		  y2);
extern	Unit		_DtHelpCeDrawSegments(
				CECanvasStruct	 *canvas,
				CELines		  line,
				CESegment	 *p_seg,
				int		  start_char,
				int		  count,
				int		 *prev_lnk,
				Unit		  txt_x,
				Unit		  sel_x,
				Unit		 *scriptX,
				Unit		 *super_width,
				Unit		 *super_y,
				Unit		 *sub_width,
				Unit		 *sub_y,
				CEBoolean	 *last_was_sub,
				CEBoolean	 *last_was_super,
				CEBoolean	 *last_was_label,
				CEBoolean	 *last_link_vis,
				Flags		  old_flag,
				Flags		  new_flag);
extern	Unit		_DtHelpCeDrawText(
				CECanvasStruct	 *canvas,
				CELines		  line,
				Unit		  start_x,
				Unit		  end_x,
				Flags		  old_flag,
				Flags		  new_flag );
extern	void		_DtHelpCeFreeGraphics(
				CECanvasStruct	 *canvas);
extern	void		_DtHelpCeFreeParagraphs(
				CECanvasStruct	 *canvas,
				CELinkData	  link_data,
				CEParagraph	**para_list,
				int		  para_count);
extern	void		_DtHelpCeFreeSegList(
				CECanvasStruct	 *canvas,
				CELinkData	  link_data,
				int		  cnt,
				CESegment	 *seg_list);
extern	int		_DtHelpCeGetCharIdx(
				CECanvasStruct	 *canvas,
				CELines		  line,
				Unit		  find_x);
extern	int		_DtHelpCeGetLineAtY(
				CECanvasStruct	 *canvas,
				CELines		 *lines,
				int		  txt_count,
				Unit		  y);
extern	Unit		_DtHelpCeGetStartXOfLine(
				CECanvasStruct	 *canvas,
				CELines		  line,
				CESegment	**pSeg,
				CEBoolean	 *label_flag);
extern	CEBoolean	_DtHelpCeModifyXpos(
				CanvasLinkMetrics info,
				CESegment	*seg,
				CEBoolean	 tst_result,
				CEBoolean	 old_result,
				int		 idx,
				Unit		*x_pos);
extern	Unit		_DtHelpCeStartXOfLine(
				CECanvasStruct	  *canvas,
				CELines		   line,
				CESegment	**p_seg,
				CEBoolean	 *label_flag);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _CECanvasI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
