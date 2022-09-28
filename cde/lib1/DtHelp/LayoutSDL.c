/* $XConsortium: LayoutSDL.c /main/cde1_maint/2 1995/08/26 22:43:17 barstow $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        LayoutSDL.c
 **
 **   Project:     CDE Help System
 **
 **   Description: Lays out the information retrieved from an SDL volume
 **                on the 'canvas'.
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

/*
 * system includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "CleanUpI.h"
#include "FormatSDLI.h"
#include "LayoutSDLI.h"
#include "LayoutUtilI.h"
#include "SnrefI.h"
#include "StringFuncsI.h"
#include "UtilSDLI.h"
#include "VirtFuncsI.h"

extern  CEElement       *_DtHelpCeFindSdlElement(
#ifndef _NO_PROTO
				CESegment         *seg_list,
				enum SdlElement    target,
				CEBoolean          depth
#endif
);

#ifdef NLS16
#include <nl_types.h>
#endif

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/
#define	JUSTIFY_CENTER	CE_CAPTION_CENTER
#define	JUSTIFY_LEFT	CE_CAPTION_LEFT
#define	JUSTIFY_RIGHT	CE_CAPTION_RIGHT

/*
 * Defines for the dimension arrays
 */
#define	DIMS_LEFT	0
#define	DIMS_RIGHT	1

#define	DIMS_LM		0
#define	DIMS_CENTER	1
#define	DIMS_RM		2

#define	DIMS_TOP	0
#define	DIMS_BOTTOM	2

#define	DIMS_WIDTH	0
#define	DIMS_HEIGHT	1
#define	DIMS_YPOS	1

#define	DIMS_TC		0
#define	DIMS_BC		1

/*
 * top/bottom dimension array
 *                  -------------------------------------------
 *                 / DIMS_HEIGHT / DIMS_HEIGHT / DIMS_HEIGHT /|
 *                /   DIMS_YPOS /   DIMS_YPOS /   DIMS_YPOS / |
 *               /-------------/-------------/-------------/  |
 *              / DIMS_WIDTH  / DIMS_WIDTH  / DIMS_WIDTH  /| /|
 *             --------------|-------------|-------------| |/ |
 * DIMS_TOP    |   DIMS_LM   | DIMS_CENTER |   DIMS_RM   | | /|
 *             |-------------|-------------|-------------|/|/ |
 *  unused     |             |             |             | | /
 *             |-------------|-------------|-------------|/|/
 * DIMS_BOTTOM |   DIMS_LM   | DIMS_CENTER |   DIMS_RM   | /
 *             ------------------------------------------|/
 */
typedef	Unit	TopDims[DIMS_BOTTOM+1][DIMS_RM+1][DIMS_HEIGHT+1];
/* 
 * left/right side dimension array - contains only the height or y_pos.
 *             ----------------------------
 * DIMS_TOP    |  DIMS_LEFT | DIMS_RIGHT  |
 *             |------------|-------------|
 * DIMS_CENTER |  DIMS_LEFT | DIMS_RIGHT  |
 *             |------------|-------------|
 * DIMS_BOTTOM |  DIMS_LEFT | DIMS_RIGHT  |
 *             ----------------------------
 */
typedef	Unit	SideDims[DIMS_BOTTOM+1][DIMS_RIGHT+1];
/*
 * corner dimension array - contains only the height or y_pos.
 *            ----------------------------
 * DIMS_TC    |  DIMS_LEFT | DIMS_RIGHT  |
 *            |------------|-------------|
 * DIMS_BC    |  DIMS_LEFT | DIMS_RIGHT  |
 *            ----------------------------
 */
typedef	Unit	CornerDims[DIMS_BC+1][DIMS_RIGHT+1];
/*
 * flow dimension array
 *            -------------|--------------
 * DIMS_LEFT  | DIMS_WIDTH | DIMS_HEIGHT |
 *            |            |  DIMS_YPOS  |
 *            |------------|-------------|
 * DIMS_RIGHT | DIMS_WIDTH | DIMS_HEIGHT |
 *            |            |  DIMS_YPOS  |
 *            ----------------------------
 */
typedef	Unit	FlowDims[DIMS_RIGHT+1][DIMS_HEIGHT+1];

/*
 */
#define	HEADW_DIVISOR	10000

#define	GROW_SIZE	10

#define CheckAddToHyperList(a, b) \
	_DtHelpCeCheckAddToHyperList(a, b, &(layout->info.lst_vis), \
			&(layout->info.lst_hyper), &(layout->info.cur_len))

typedef	struct	_dataPoint {
	Unit	left;
	Unit	right;
	Unit	y_pos;
	Unit	x_units;
	struct _dataPoint *next_pt;
} DataPoint;

typedef	struct	{
	Unit		 max_width;
	Unit		 f_margin;
	Unit		 l_margin;
	Unit		 r_margin;

	Unit		 id_Ypos;

	Unit		 left;
	Unit		 right;
	Unit		 first;
	Unit		 string_end;
	Unit		 sub_end;
	Unit		 super_end;
	CEBoolean	 id_found;
	CEBoolean	 super_script;
	CEBoolean	 sub_script;
	CEBoolean	 stat_flag;
	enum SdlOption	 txt_justify;
	unsigned int	 cur_start;
	CESegment	*toss;
	CESegment	*lst_rendered;
	CEElement	*snb;
	CEElement	*parent_el;
	char		*target_id;
        DataPoint	*data_pts;
	CELayoutInfo	 info;
} LayoutInfo;

/*****************************************************************************
 *		Private Function Declarations
 *****************************************************************************/
#ifdef _NO_PROTO
static	void		AdjustHeadPosition();
static	void		AdjustObjectPosition();
static	void		CheckSaveInfo ();
static	void		CreateTables ();
static	void		DetermineFlowConstraints();
static	void		DetermineHeadPositioning();
static	void		DetermineMaxDims();
static	void		InitDimArrays();
static	void		SdlCanvasLayout ();
#else
static	void		AdjustHeadPosition(
				CECanvasStruct	*canvas,
				CESegment	*p_seg,
				TopDims		*top_bot,
				SideDims	*side,
				CornerDims	*corner,
				FlowDims	*flow,
				Unit		 base_y,
				Unit		 base_left,
				Unit		 block_width,
				Unit		 left_margin,
				Unit		 right_margin);
static	void		AdjustObjectPosition(
				CECanvasStruct	*canvas,
				enum SdlOption	 justify,
				int		 start_txt,
				int		 start_gr,
				int		 start_ln,
				int		 end_txt,
				int		 end_gr,
				int		 end_ln,
				int		 brdr_cnt,
				Unit		 height_adj,
				Unit		 y_adj);
static	void		CheckSaveInfo (
				CECanvasStruct	*canvas,
				LayoutInfo	*layout);
static	void		DetermineFlowConstraints(
				LayoutInfo	*layout,
				FlowDims	 flow_dims,
				Unit		 left_margin,
				Unit		 right_margin,
				Unit		 start_y,
				DataPoint	*left_pt,
				DataPoint	*right_pt);
static	void		DetermineHeadPositioning(
				TopDims	*top_bot,
				SideDims	*side,
				CornerDims	*corner,
				FlowDims	*flow,
				Unit		 start_y,
				Unit		 max_top,
				Unit		 block_size,
				Unit		*ret_side_size);
static	void		DetermineMaxDims(
				TopDims		*top_bot,
				CornerDims	*corner,
				Unit		 left_margin,
				Unit		 right_margin,
				Unit		*top_height,
				Unit		*bot_height,
				Unit		*max_width);
static	void		InitDimArrays(
				TopDims		*top_bot,
				SideDims	*side,
				CornerDims	*corner,
				FlowDims	*flow);
static	CEBoolean	ProcessElement(
				CECanvasStruct	*canvas,
				LayoutInfo	*layout,
				CESegment	*cur_seg,
				CESegment	*join_seg,
				Unit		 min_y,
				TopDims		*top_bot,
				SideDims	*side,
				CornerDims	*corner,
				FlowDims	*flow,
				Unit		*max_left,
				Unit		*max_right,
				Unit		*ret_width,
				int		*ret_cnt,
				int		*join_line);
static	void		ProcessSegmentList(
				CECanvasStruct	*canvas,
				LayoutInfo	*layout,
				CESegment	*cur_seg,
				CESegment	*join_seg);
static	void		SdlCanvasLayout (
				CanvasHandle	 canvas_handle,
				CESegment	*toss,
				LayoutInfo	*layout,
				char		*target_id);
#endif /* _NO_PROTO */

/******************************************************************************
 *
 * Private Macros
 *
 *****************************************************************************/
#define	ObjHorizOrient(x)	((x)->attributes.orient)
#define	ObjVertOrient(x)	((x)->attributes.vorient)
#define	FlowAttr(x)		((x)->attributes.flow)
#define	TxtHorizJustify(x)	((x)->attributes.frmt_specs.justify)
#define	TxtVertJustify(x)	((x)->attributes.frmt_specs.vjust)
#define	Border(x)		((x)->attributes.frmt_specs.border)
#define	FrmtInfoPtr(x)		(&((x)->attributes.frmt_info))
#define	IdString(x)		_DtHelpCeSdlIdString(x)

/******************************************************************************
 *
 * Private variables
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	SkipToNumber
 *
 * Returns:
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
SkipToNumber (string)
    char	**string;
#else
SkipToNumber (
    char	**string)
#endif /* _NO_PROTO */
{
    if (string != NULL)
      {
        char *str = *string;

        while (*str == ' ' && *str != '\0')
	    str++;
        *string = str;
      }
}

/******************************************************************************
 * Function:	GetValueFromString
 *
 * Returns:
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GetValueFromString (string, def_num)
    char	**string;
    int		  def_num;
#else
GetValueFromString (
    char	**string,
    int		  def_num)
#endif /* _NO_PROTO */
{
    int  value = def_num;

    if (string != NULL && *string != NULL && **string != '\0')
      {
        char *str = *string;

	if ('0' <= *str && *str <= '9')
	    value = atoi(str);

	while ('0' <= *str && *str <= '9')
	    str++;

	while (*str != ' ' && *str != '\0' && (*str < '0' || *str > '9'))
	    str++;

        *string = str;
      }

    return value;
}

/******************************************************************************
 * Function:	GetJustifyFromString
 *
 * Returns:
 *****************************************************************************/
static	enum SdlOption
#ifdef _NO_PROTO
GetJustifyFromString (string, def_num)
    char		**string;
    enum SdlOption	  def_num;
#else
GetJustifyFromString (
    char		**string,
    enum SdlOption	  def_num)
#endif /* _NO_PROTO */
{
    enum SdlOption  value = def_num;

    if (string != NULL && *string != NULL && **string != '\0')
      {
        char *str = *string;

	while (*str == ' ' && *str != '\0')
	    str++;

	if (*str != '\0')
	  {
	    switch (*str)
	      {
		case 'L':
		case 'l': value = SdlJustifyLeft;
			  break;
		case 'R':
		case 'r': value = SdlJustifyRight;
			  break;
		case 'C':
		case 'c': value = SdlJustifyCenter;
			  break;
		case 'D':
		case 'd': value = SdlJustifyNum;
			  break;
	      }
	 
	    while (*str != ' ' && *str != '\0')
	        str++;
	  }

        *string = str;
      }

    return value;
}

/******************************************************************************
 * Function:	PushDataPoint
 *
 * Returns:
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
PushDataPoint (layout, data_pt)
    LayoutInfo		*layout;
    DataPoint		*data_pt;
#else
PushDataPoint (
    LayoutInfo		*layout,
    DataPoint		*data_pt)
#endif /* _NO_PROTO */
{
    data_pt->x_units = 0;
    data_pt->next_pt = layout->data_pts;
    layout->data_pts = data_pt;
}

/******************************************************************************
 * Function:	InsertDataPoint
 *
 * Returns:
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
InsertDataPoint (layout, data_pt)
    LayoutInfo		*layout;
    DataPoint		*data_pt;
#else
InsertDataPoint (
    LayoutInfo		*layout,
    DataPoint		*data_pt)
#endif /* _NO_PROTO */
{
    DataPoint	*lastPt = NULL;
    DataPoint	*nextPt = layout->data_pts;

    while (nextPt != NULL &&
	nextPt->y_pos != _CEFORMAT_ALL &&
	(data_pt->y_pos == _CEFORMAT_ALL || nextPt->y_pos < data_pt->y_pos))
      {
	lastPt = nextPt;
	nextPt = nextPt->next_pt;
      }

    data_pt->next_pt = nextPt;
    data_pt->x_units = 0;

    if (lastPt == NULL)
        layout->data_pts = data_pt;
    else
	lastPt->next_pt = data_pt;
}

/******************************************************************************
 * Function:	RemoveDataPoint
 *
 * Returns:
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
RemoveDataPoint (layout, data_pt)
    LayoutInfo		*layout;
    DataPoint		*data_pt;
#else
RemoveDataPoint (
    LayoutInfo		*layout,
    DataPoint		*data_pt)
#endif /* _NO_PROTO */
{
    DataPoint	*lastPt = NULL;
    DataPoint	*curPt  = layout->data_pts;

    while (curPt != NULL && curPt != data_pt)
      {
	lastPt = curPt;
	curPt  = curPt->next_pt;
      }

    if (curPt != NULL)
      {
        data_pt->x_units = layout->info.max_x_pos - data_pt->left;
        if (lastPt == NULL)
	    layout->data_pts = curPt->next_pt;
        else
	    lastPt->next_pt  = curPt->next_pt;
      }
}

/******************************************************************************
 * Function:	GetCurrentDataPoint
 *
 * Returns:
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
GetCurrentDataPoint (layout, data_pt)
    LayoutInfo		*layout;
    DataPoint		*data_pt;
#else
GetCurrentDataPoint (
    LayoutInfo		*layout,
    DataPoint		*data_pt)
#endif /* _NO_PROTO */
{
    data_pt->left  = 0;
    data_pt->right = 0;
    data_pt->y_pos = _CEFORMAT_ALL;

    if (layout->data_pts != NULL)
	*data_pt = *(layout->data_pts);
}

/******************************************************************************
 * Function:	SetMargins
 *
 * Purpose: Sets the margins.
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
SetMargins (layout)
    LayoutInfo		*layout;
#else
SetMargins (
    LayoutInfo		*layout)
#endif /* _NO_PROTO */
{
    layout->l_margin = 0;
    layout->r_margin = 0;
    layout->f_margin = 0;
    layout->info.format_y = _CEFORMAT_ALL;

    if (layout->data_pts != NULL)
      {
	/*
	 * base 
	 */
	layout->l_margin = layout->data_pts->left;
	layout->r_margin = layout->data_pts->right;

        layout->info.format_y = layout->data_pts->y_pos;
      }

    layout->l_margin += layout->left;
    layout->r_margin += layout->right;
    layout->f_margin  = layout->first;

}

/******************************************************************************
 * Function:	SetTextPosition
 *
 * Purpose: Sets the text beginning position to the left margin.
 *	    If 'first' is true, adds its value to the text beginning
 *	    position value.
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
SetTextPosition (layout, first)
    LayoutInfo		*layout;
    CEBoolean		 first;
#else
SetTextPosition (
    LayoutInfo		*layout,
    CEBoolean		 first)
#endif /* _NO_PROTO */
{
    layout->info.text_x_pos = layout->l_margin;
    if (first == True)
        layout->info.text_x_pos += layout->f_margin;

    layout->info.label_x_pos = layout->info.text_x_pos;
}

/******************************************************************************
 * Function:	CheckFormat
 *
 * Purpose: Checks to see if the flowing txt boundaries have been exceeded.
 *          If a boundary has been exceeded, then removes that boundary
 *	    information from the stack until it finds a valid boundary point.
 *
 *	    Calls SetMargins to set the correct margin
 *	    Calls SetTextPosition to set the text beginning position.
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
CheckFormat (layout, first)
    LayoutInfo		*layout;
    CEBoolean		 first;
#else
CheckFormat (
    LayoutInfo		*layout,
    CEBoolean		 first)
#endif /* _NO_PROTO */
{
    while (layout->data_pts != NULL &&
		layout->data_pts->y_pos != _CEFORMAT_ALL &&
				layout->data_pts->y_pos < layout->info.y_pos)
	RemoveDataPoint (layout, layout->data_pts);

    SetMargins(layout);
    SetTextPosition(layout, first);
}

/******************************************************************************
 * Function:	CalcOffset
 *
 * Returns:
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
CalcOffset (canvas, number)
    CECanvasStruct   *canvas;
    int		      number;
#else
CalcOffset (
    CECanvasStruct   *canvas,
    int		      number)
#endif /* _NO_PROTO */
{
    return (canvas->font_info.average_width * number);
}

/******************************************************************************
 * Function: SaveLine
 *
 * Initializes a line table element to the segment it should display.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
SaveLine (canvas, direction, x, y, length)
    CECanvasStruct	*canvas;
    int			 direction;
    Unit		 x;
    Unit		 y;
    Unit		 length;
#else
SaveLine (
    CECanvasStruct	*canvas,
    int			 direction,
    Unit		 x,
    Unit		 y,
    Unit		 length)
#endif /* _NO_PROTO */
{
    if (canvas->line_cnt >= canvas->line_max)
      {
	canvas->line_max += GROW_SIZE;
	if (canvas->line_lst)
	    canvas->line_lst = (CELineSegment *) realloc (
				(void *) canvas->line_lst,
				(sizeof(CELineSegment) * canvas->line_max));
	else
	    canvas->line_lst = (CELineSegment *) malloc (
				(sizeof(CELineSegment) * canvas->line_max));
/*
 * NOTE....should this routine return a value?
 * If (re)alloc error occurs, this simply ignores the problem.
 */
	if (canvas->line_lst == NULL)
	  {
	    canvas->line_max = 0;
	    canvas->line_cnt = 0;
	    return;
	  }
      }

    /*
     * save the line information, if there is a string here.
     */
    canvas->line_lst[canvas->line_cnt].dir    = direction;
    canvas->line_lst[canvas->line_cnt].pos_x  = x;
    canvas->line_lst[canvas->line_cnt].pos_y  = y;
    canvas->line_lst[canvas->line_cnt].length = length;

    canvas->line_cnt++;
}

/******************************************************************************
 * Function: SaveInfo
 *
 * Purpose: Saves the current information into the txt line struct.
 *          Checks to see if this line exceeds the flowing text
 *          boundary and resets the internal global margins.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
SaveInfo (canvas, layout)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
#else
SaveInfo (
    CECanvasStruct	*canvas,
    LayoutInfo		*layout)
#endif /* _NO_PROTO */
{
    _DtHelpCeSaveInfo (canvas, &(layout->info), \
		  layout->max_width, layout->r_margin, layout->txt_justify);

    layout->super_end    = 0;
    layout->sub_end      = 0;
    layout->super_script = False;
    layout->sub_script   = False;
    CheckFormat(layout, FALSE);
}

/******************************************************************************
 * Function: CheckSaveInfo
 *
 * Purpose: Checks to see if there is any information to save and saves
 *          it if there is any.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
CheckSaveInfo (canvas, layout)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
#else
CheckSaveInfo (
    CECanvasStruct	*canvas,
    LayoutInfo		*layout)
#endif /* _NO_PROTO */
{
    if (layout->info.line_bytes || layout->info.gr_cnt)
	SaveInfo (canvas, layout);

    layout->info.cur_len = 0;
}

/******************************************************************************
 * Function: ProcessStringSegment
 *
 * chops a string segment up until its completely used.
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
ProcessStringSegment(canvas, layout, cur_seg)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*cur_seg;
#else
ProcessStringSegment(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*cur_seg)
#endif /* _NO_PROTO */
{
    layout->cur_start = 0;
    if (!(_CEIsSuperScript(cur_seg->seg_type)
			|| _CEIsSubScript(cur_seg->seg_type))
	&& (layout->super_script == True || layout->sub_script == True))
      {
	layout->super_end = 0;
	layout->sub_end   = 0;
	layout->super_script = False;
	layout->sub_script   = False;
      }

    while (_DtHelpCeProcessStringSegment(canvas, &(layout->info),
			layout->max_width, layout->l_margin, layout->r_margin,
			layout->toss, layout->snb,
			cur_seg , &(layout->cur_start),
			layout->parent_el->attributes.language,
			layout->parent_el->attributes.charset,
/*
			&(layout->parent_el->attributes.font_specs),
 */
			cur_seg->seg_handle.str_handle->font_specs,
			layout->txt_justify, layout->stat_flag) == 1)
	CheckFormat(layout, False);

} /* End ProcessStringSegment */


/******************************************************************************
 *****************************************************************************/
#define	CellsAttribute(x)	((x)->attributes.id_vals.cells)
typedef	struct	{
	int		 col_spn;
	int		 row_spn;
	int		 begin_txt;
	int		 end_txt;
	int		 begin_gr;
	int		 end_gr;
	int		 begin_ln;
	int		 end_ln;
	int		 border;
	Unit		 pos_x;
	Unit		 my_height;
	CESegment	*cell_seg;
} CECellInfo;

typedef	struct _columnSpec {
	Unit	min;
	Unit	max;
	Unit	actual;
	enum SdlOption justify;
} ColumnSpec;

typedef	struct	_rowSpec {
	Unit	 height;
	char	*next_id;
} RowSpec;

/******************************************************************************
 * Function: Unit ResolveHeight(
 *
 * Parameters:
 *
 * Purpose: Determines the height of a row that is spanned.
 *****************************************************************************/
static Unit
#ifdef _NO_PROTO
ResolveHeight(row_info, cell_info, max_cols, max_rows, row, span)
    RowSpec  *row_info;
    CECellInfo *cell_info;
    int       max_cols;
    int       max_rows;
    int       row;
    int       span;
#else
ResolveHeight(
    RowSpec  *row_info,
    CECellInfo *cell_info,
    int       max_cols,
    int       max_rows,
    int       row,
    int       span)
#endif /* _NO_PROTO */
{
    int i;
    int col;
    int cell;
    int topRow;
    int topCell;
    int zeroed;
    int total;
    int rowHeight = 0;

    for (col = 0, cell = row * max_cols; col < max_cols; col++, cell++)
      {
	/*
	 * if we have a row spanning cell in this column,
	 * but it isn't from a previous column, try to fill out
	 * the height using it.
	 */
	if (cell_info[cell].row_spn == -1 && cell_info[cell].col_spn != -1)
	  {
	    /*
	     * back up to the cell information containing the row
	     * span value.
	     */
	    topRow  = row;
	    topCell = cell;
	    do
	      {
		topCell--;
		topRow--;
	      } while (cell_info[topCell].row_spn == -1);

	    /*
	     * from here, start calculating the height of the row
	     * spanning cell, and find out how big the row must
	     * be to contain it.
	     */
	    i      = cell_info[topCell].row_spn;
	    zeroed = span + 1;
	    total  = 0;
	    while (i > 0 && zeroed > 0)
	      {
		total += row_info[topRow].height;
		if (row_info[topRow].height == 0)
		   zeroed--;
		i--;
		topRow++;
	      }

	    /*
	     * if zeroed is greater than zero, that means only the
	     * allowd number of row heights were zeroed out (the ones
	     * we are looking for).
	     * go ahead and calculate a new height. Otherwise,
	     * it may be tried later.
	     */
	    if (zeroed > 0)
	      {
		/*
		 * make sure we get a positive value out of
		 * this for our height.
		 */
		if (cell_info[topCell].my_height > total)
		  {
		    total = cell_info[topCell].my_height - total;
		    total = total / span + (total % span ? 1 : 0);
		  }

		/*
		 * make sure we take the biggest value possible
		 * for this row. If it needs to be smaller for
		 * cells, we'll adjust the positioning within
		 * the cell.
		 */
		if (rowHeight < total)
		    rowHeight = total;
	      }
	  }
      }

   row_info[row].height = rowHeight;
   return rowHeight;
}

/******************************************************************************
 * Function: void AdjustHeight(
 *
 * Parameters:
 *
 * Purpose: Determines the height of rows that are spanned but the 
 *          the spanner needs more room than the calculated row height
 *          allow.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustHeight(canvas, cell_info,	row_specs, max_rows, max_cols, row, col)
   CECanvasStruct	*canvas;
   CECellInfo		*cell_info;
   RowSpec		*row_specs;
   int			 max_rows;
   int			 max_cols;
   int			 row;
   int			 col;
#else
AdjustHeight(
   CECanvasStruct	*canvas,
   CECellInfo		*cell_info,
   RowSpec		*row_specs,
   int			 max_rows,
   int			 max_cols,
   int			 row,
   int			 col)
#endif /* _NO_PROTO */
{
    int    cell = row * max_cols + col;
    int    i, j;
    Unit   total;
    Unit   adjustValue;
    Unit   value;

    if (cell_info[cell].col_spn == -1 || cell_info[cell].row_spn == -1)
	return;

    for (j = row, i = 0, total = 0; i < cell_info[cell].row_spn; i++, j++)
	total += row_specs[j].height;

    /*
     * adjust the row height to include all of the row spanning cell.
     */
    if (total < cell_info[cell].my_height)
      {
	Unit   totalUsed = 0;

	/*
	 * first, try to grow the cells on a percentage basis.
	 * This way, a small cell will grow the same amount relative
	 * to the larger cells.
	 */
	adjustValue = cell_info[cell].my_height - total;
	for (i = 0; i < cell_info[cell].row_spn; i++)
	  {
	    value = (((row_specs[row + i].height * 100) / total) * adjustValue)
				/ 100;
	    row_specs[row + i].height += value;
	    totalUsed                 += value;
	  }

	/*
	 * if didn't use all the size up - apply it evenly.
	 */
	if (totalUsed < adjustValue)
	  {
	    adjustValue = adjustValue - totalUsed;
	    for (i = 0, j = cell_info[cell].row_spn;
		adjustValue > 0 && i < cell_info[cell].row_spn; i++, j--)
	      {
		value = adjustValue / j + (adjustValue % j ? 1 : 0);
		row_specs[row + i].height += value;
		adjustValue               -= value;
		totalUsed                 += value;
	      }
	  }

	total = totalUsed;
      }
}

/******************************************************************************
 * Function: void FormatCell(
 *
 * Parameters:
 *		canvas		Specifies the specific information about the
 *				rendering area.
 *		layout		Specifies the currently active information
 *				affecting the layout of information.
 *		span_width	Specifies the desired size constraining
 *				the layout of information.
 *		base_pt		Specifies the base margins.
 *		this_cell	Specifies the cell structure to fill out.
 *
 * Purpose: Determines the width and height of the cell. Also the begin/end
 *          counts on text, graphics and lines.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
FormatCell(canvas, layout, cell_seg, span_width, min_height, base_pt,
		ret_ln, ret_width, ret_height)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*cell_seg;
    Unit		 span_width;
    Unit		 min_height;
    DataPoint		 base_pt;
    int			*ret_ln;
    Unit		*ret_width;
    Unit		*ret_height;
#else
FormatCell(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*cell_seg,
    Unit		 span_width,
    Unit		 min_height,
    DataPoint		 base_pt,
    int			*ret_ln,
    Unit		*ret_width,
    Unit		*ret_height)
#endif /* _NO_PROTO */
{
    Unit   left;
    Unit   right;
    Unit   saveYpos  = layout->info.y_pos;
    Unit   saveRight = layout->right;

    /*
     * set the limits/margins
     * assume the left_margin has been set by a previous call.
     */
    layout->right        = layout->max_width - base_pt.left - base_pt.right
				- layout->left - span_width;

    /*
     * If a segment was specified for this cell, format it.
     */
    if (cell_seg != NULL)
        ProcessElement (canvas, layout, cell_seg, NULL, min_height + saveYpos,
				NULL, NULL, NULL, NULL,
				&left, &right,
				&span_width, ret_ln, NULL);

    /*
     * Set the information.
     */
    *ret_width         = span_width;
    *ret_height        = layout->info.y_pos - saveYpos;

    /*
     * restore the right margin
     */
    layout->right = saveRight;
}

/******************************************************************************
 * Function: AdjustFrmtTxtOption
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustFrmtTxtOption(p_seg, option)
    CESegment		*p_seg;
    enum SdlOption	 option;
#else
AdjustFrmtTxtOption(
    CESegment		*p_seg,
    enum SdlOption	 option)
#endif /* _NO_PROTO */
{
    CEElement	*pEl;

    if (p_seg != NULL && _CEIsElement(p_seg->seg_type))
      {
	pEl = _CEElementOfSegment(p_seg);
	if (pEl->el_type != SdlElementRefItem)
	  {
	    TxtHorizJustify(pEl) = option;

	    p_seg = pEl->seg_list;
	    while (p_seg != NULL)
	      {
	        AdjustFrmtTxtOption(p_seg, option);
	        p_seg = p_seg->next_seg;
	      }
	  }
      }
}

/******************************************************************************
 * Function: ResolveCell
 *
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ResolveCell(canvas, layout, form, ret_fdata, col_specs, row_specs,
		col, row, max_cols, max_rows, ret_info)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CEElement		*form;
    ColumnSpec		*col_specs;
    RowSpec		*row_specs;
    int			 col;
    int			 row;
    int			 max_cols;
    int			 max_rows;
    CESegment		**ret_fdata;
    CECellInfo		*ret_info;
#else
ResolveCell(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CEElement		*form,
    ColumnSpec		*col_specs,
    RowSpec		*row_specs,
    int			 col,
    int			 row,
    int			 max_cols,
    int			 max_rows,
    CESegment		**ret_fdata,
    CECellInfo		*ret_info)
#endif /* _NO_PROTO */
{
    register int i;
    char  *id;
    char  *idRefs;
    char  *ptr;
    char   c;
    int    count;
    int    len;
    int    done;
    int    myCol = col;
    int	   cell  = row * max_cols + col;
    Unit   cellWidth;
    Unit   retWidth;
    Unit   retHeight;
    CECellInfo *thisCell = &ret_info[cell];
    DataPoint	 basePt;
    CEBoolean	 reformat = False;
    CESegment	*lastSeg;
    CESegment	*f_data = *ret_fdata;

    /*
     * if this column is spanned, skip
     */
    if (thisCell->col_spn == -1 || thisCell->row_spn == -1)
	return False;

    GetCurrentDataPoint(layout, &basePt);

    if (thisCell->cell_seg == NULL)
      {
	idRefs = row_specs[row].next_id;

        /*
         * find the end of the id
         */
        done      = False;

        /*
         * set the starting info
         */
        thisCell->col_spn = 1;
        thisCell->pos_x   = basePt.left + layout->left;

        while (!done && col < max_cols)
	  {
            ptr       = idRefs;
            id        = idRefs;

            /*
             * move the ptr to the next id,
	     * counting the characters in this id at the same time.
             */
	    len = 0;
            while (*ptr != ' ' && *ptr != '\0')
	      {
                ptr++;
		len++;
	      }

	    /*
	     * set idRefs to the next id.
	     */
            idRefs = ptr;
            while (*idRefs == ' ')
                idRefs++;

            /*
             * Is this id and the next the same? If so,
             * it spans the columns
             */
            if (_DtHelpCeStrNCaseCmp(id, idRefs, len) == 0 &&
				(idRefs[len] == ' ' || idRefs[len] == '\0'))
              {
                col++;
                thisCell->col_spn++;
		ret_info[++cell].col_spn = -1;
              }
            else
                done = True;
          }

	row_specs[row].next_id = idRefs;


	/*
	 * find the segment
	 */
	c       = *ptr;
        *ptr    = '\0';
	lastSeg = f_data;
	while (f_data != NULL &&
	    _DtHelpCeStrCaseCmp(IdString(_CEElementOfSegment(f_data)),id) != 0)
	    f_data  = f_data->next_seg;

	/*
	 * if we didn't find the ids in the form fdata list, look in 
	 * the async blocks.
	 */
	if (f_data == NULL)
	  {
            /*
             * look through the async blocks for a match.
             */
            f_data = _DtHelpCeResolveAsyncBlocks(canvas, layout->toss,
                                                            form, id);
            if (f_data == NULL)
              {
		CEElement *pEl;

                (void) _DtHelpCeAllocSegment(1, NULL, NULL, &f_data);
                (void) _DtHelpCeAllocElement(SdlElementBlockAsync,
						&(form->attributes), &pEl);

		pEl->attributes.str1_values = SDL_ATTR_ID;
		IdString(pEl)               = strdup(id);

		f_data->seg_type            = CESetTypeToElement(0);
		_CEElementOfSegment(f_data) = pEl;
              }

            /*
             * insert the segment into a list so it will be freed later
             */
	    if (lastSeg != NULL)
	      {
                while (lastSeg->next_seg != NULL)
                    lastSeg = lastSeg->next_seg;

                lastSeg->next_seg = f_data;
	      }
	    else
	     {
	       /*
	        * the form doesn't have any f_data elements
		* so this async block starts its segment list.
	        */
	       lastSeg    = f_data;
	       *ret_fdata = f_data;
	     }
	  }

	thisCell->cell_seg = f_data;

	if (f_data != NULL)
	    AdjustFrmtTxtOption (f_data, col_specs[myCol].justify);

	/*
	 * how many rows does this cell span?
	 */
	for (done = False, count = 1, len = strlen(id), i = row + 1;
					i < max_rows && False == done; i++)
	  {
	    done = True;
	    if (_DtHelpCeStrNCaseCmp(id, row_specs[i].next_id, len) == 0 &&
		(row_specs[i].next_id[len] == ' ' ||
					row_specs[i].next_id[len] == '\0'))
	      {
		done = False;
		count++;
		ret_info[i * max_cols + col].col_spn = -1;
		ret_info[i * max_cols + col].row_spn = -1;
		
		idRefs  = row_specs[i].next_id;
		do
		  {
		    /*
		     * skip the current id.
		     */
		    while (*idRefs != ' ' && *idRefs != '\0')
			idRefs++;

		    /*
		     * skip the space to the next id.
		     */
		    while (*idRefs == ' ')
			idRefs++;

		    /*
		     * now test to see if this is a match
		     * cycle if so. quit if the end of string
		     * or not a match.
		     */
		  } while (*idRefs != '\0' &&
			_DtHelpCeStrNCaseCmp(id, idRefs, len) == 0 &&
				(idRefs[len] == ' ' || idRefs[len] == '\0'));

		/*
		 * the next column in this row will use this id.
		 */
		row_specs[i].next_id = idRefs;
	      }
	  }

	*ptr = c;
	thisCell->row_spn = count;
      }

    for (i = 0, cellWidth = 0; i < thisCell->col_spn; i++)
	cellWidth += col_specs[myCol + i].actual;

    /*
     * Format the cell
     */
    thisCell->begin_txt = canvas->txt_cnt;
    thisCell->begin_ln  = canvas->line_cnt;

    FormatCell(canvas, layout, thisCell->cell_seg, cellWidth, 0, basePt,
				&(thisCell->border), &retWidth, &retHeight);

    thisCell->my_height = retHeight;
    thisCell->end_txt   = canvas->txt_cnt;
    if (thisCell->begin_gr == -1)
      {
        SDLFrmtInfo *frmtInfo = FrmtInfoPtr(_CEElementOfSegment(thisCell->cell_seg));
        thisCell->begin_gr = frmtInfo->beg_gr;
        thisCell->end_gr   = frmtInfo->end_gr;
      }
    thisCell->end_ln = canvas->line_cnt;


    /*
     * check the height against previous heights
     */
    if (row_specs[row].height < retHeight && thisCell->row_spn == 1)
	row_specs[row].height = retHeight;

    if (retWidth > cellWidth)
      {
	register int j;
	Unit  cellMax;
	Unit  maxSlop;
	Unit  value;
	Unit  percent;
	Unit  slopUsed = 0;
	Unit  slop   = retWidth - cellWidth;

	/*
	 * determine the maximum size the cell can occupy
	 */
	for (j = 0, cellMax = 0;
			j < thisCell->col_spn && j + myCol < max_cols; j++)
	    cellMax += col_specs[myCol+j].max;
	
	/*
	 * does this cell fit in the allowed size without pushing?
	 * If not, use up as much slop as possible.
	 */
	if (cellMax < retWidth)
	    slop = cellMax - cellWidth;

	/*
	 * determine the maximum available space to allocate.
	 */
	for (j = myCol + thisCell->col_spn, maxSlop = 0; j < max_cols; j++)
	    maxSlop = maxSlop + col_specs[j].actual - col_specs[j].min;

	/*
	 * If the slop demanded is larger than available,
	 * simply reduced the column specifications to their smallest
	 * values
	 */
	if (slop > maxSlop)
	  {
	    for (j = myCol + thisCell->col_spn; j < max_cols; j++)
	        col_specs[j].actual = col_specs[j].min;
	  }
	else if (slop > 0)
	  {
	    for (j = myCol + thisCell->col_spn; j < max_cols; j++)
	      {
	        percent   = col_specs[j].actual - col_specs[j].min;
	        value     = slop * percent / maxSlop;
	        slopUsed += value;
	        col_specs[j].actual -= value;
	      }

	    do {
	        slop -= slopUsed;
	        for (j = myCol + thisCell->col_spn, slopUsed = 0;
					slop > slopUsed && j < max_cols; j++)
	          {
	            if (col_specs[j].actual > col_specs[j].min)
	              {
	                col_specs[j].actual--;
	                slopUsed++;
	              }
	          }
	      } while (slopUsed > 0 && slop > slopUsed);
	  }

	/*
	 * set the column size.
	 */
	if (thisCell->col_spn == 1)
	    col_specs[myCol].actual = retWidth;
	else
	  {
	    /*
	     * how much to spread amoung the columns?
	     */
	    slop = cellMax - retWidth;

	    /*
	     * if the aggragate max width is smaller than required,
	     * allocate the excess amoung the columns.
	     */
	    if (slop < 0)
	      {
		/*
		 * set the desired to the max and calculate the leftover
		 * slop and the maximum desired size.
		 */
		for (j = 0, slop = retWidth, maxSlop = 0;
			myCol + j < max_cols && j < thisCell->col_spn; j++)
		  {
		    col_specs[j].actual = col_specs[myCol + j].max;
		    slop    -= col_specs[myCol + j].max;
		    maxSlop += col_specs[myCol + j].max;
		  }
		/*
		 * now allocate the leftover slop to each colum
		 * based on the maximum desired size.
		 */
		for (j = 0, slopUsed = 0;
			slop > slopUsed && myCol + j < max_cols
						&& j < thisCell->col_spn; j++)
		  {
		    value = slop * col_specs[myCol + j].max / maxSlop;
		    if (((slop*col_specs[myCol+j].max) % maxSlop) >= (maxSlop/2))
			value++;
		    col_specs[myCol + j].actual += value;
		    slopUsed += value;
		  }
	      }
	    else if (slop > 0)
	      {
	        slopUsed = 0;
	        for (j = myCol;
			j < max_cols && j < myCol + thisCell->col_spn; j++)
	          {
	            percent   = col_specs[j].max - col_specs[j].actual;
	            value     = slop * percent / maxSlop;
	            slopUsed += value;
	            col_specs[j].actual += value;
	          }

	        do {
	            slop -= slopUsed;
	            for (j = myCol, slopUsed = 0; slop > slopUsed &&
			j < max_cols && j < myCol + thisCell->col_spn; j++)
	              {
	                if (col_specs[j].actual < col_specs[j].max)
	                  {
	                    col_specs[j].actual++;
	                    slopUsed++;
	                  }
	              }
	          } while (slopUsed > 0 && slop > slopUsed);
	      }
	    else /* if (slop == 0) */
	      {
		for (j = 0; myCol + j < max_cols && j < thisCell->col_spn; j++)
		    col_specs[j].actual = col_specs[myCol + j].max;
	      }
	  }

	/*
	 * if this is a column in the first row, don't bother reformatting.
	 */
	if (row > 0)
	    reformat = True;
      }

    return reformat;
}

/******************************************************************************
 * Function: FindElement
 *
 *****************************************************************************/
static CEElement *
#ifdef _NO_PROTO
FindElement(p_seg, el_value)
    CESegment	*p_seg;
    enum SdlElement el_value;
#else
FindElement(
    CESegment	*p_seg,
    enum SdlElement el_value)
#endif /* _NO_PROTO */
{
    while (p_seg != NULL && _CEIsElement(p_seg->seg_type) &&
		_CEElementOfSegment(p_seg) != NULL &&
		_CEElementOfSegment(p_seg)->el_type != el_value)
	p_seg = p_seg->next_seg;

    return (_CEElementOfSegment(p_seg));
}

/******************************************************************************
 * Function: ProcessForm
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
ProcessForm(canvas, layout, eForm)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CEElement	*eForm;
#else
ProcessForm(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CEElement	*eForm)
#endif /* _NO_PROTO */
{
    int		   a;
    int		   b;
    int		   c;
    int		   col;
    int		   row;
    int		   cell;
    int		   divisor;
    int		   maxCols;
    int		   maxRows;
    int		   maxRowSpn;
    int		   saveLnStart  = canvas->line_cnt;
    int		   saveTxtStart = canvas->txt_cnt;
    int		   saveHyperCnt = canvas->hyper_count;
    Unit	   workWidth;
    Unit	   topHeight   = 0;
    Unit	   botHeight   = 0;
    Unit	   leftMargin  = 0;
    Unit	   rightMargin = 0;
    Unit	   saveLeft;
    Unit	   saveRight;
    Unit	   saveYpos    = layout->info.y_pos;
    Unit	   tableYpos;
    Unit	   newHeight;
    short	   anchorRow = -1;
    char	  *widthStr;
    char	  *colStr;
    enum SdlOption colJustify  = SdlJustifyLeft;
    CEBoolean	   oldFound  = layout->id_found;
    CEBoolean	   haveHeads = False;
    CEBoolean	   redo;
    CESegment	  *headList;
    CESegment	  *fstyleList;
    CESegment	  *fdataList;
    CESegment	  *rowVecList;
    CESegment	  *formList  = eForm->seg_list;

    CEElement  *fstyleEl;
    CEElement  *fdataEl;
    CECellInfo  defCell;
    CECellInfo *cellInfo = &defCell;
    ColumnSpec  defColumn;
    ColumnSpec *colSpecs = &defColumn;
    RowSpec	defRow;
    RowSpec    *rowSpecs = &defRow;
    TopDims     topBot;
    SideDims    sideDims;
    CornerDims  cornerDims;
    FlowDims    flowDims;
    DataPoint   basePt;
    DataPoint   leftPt;
    DataPoint   rightPt;

    /*
     * initialize the variables and arrays
     */
    GetCurrentDataPoint(layout, &basePt);
    leftPt  = basePt;
    rightPt = basePt;
    InitDimArrays(&topBot, &sideDims, &cornerDims, &flowDims);

    /*
     * <!ELEMENT form      - - (head*, ((fstyle, fdata) | (fdata, fstyle))) >
     *
     * If this form has one or more heads, process first.
     */
    headList = formList;
    while (formList != NULL && _CEIsElement(formList->seg_type) &&
		_CEElementOfSegment(formList)->el_type == SdlElementHead)
      {
	ProcessElement (canvas, layout, formList, NULL, -1,
			&topBot, &sideDims, &cornerDims, &flowDims,
			&leftMargin, &rightMargin,
			NULL, NULL, NULL);

	formList = formList->next_seg;
	haveHeads = True;
      }

    /*
     * Set up the information for having heads around a form
     */
    if (haveHeads == True)
      {
	DetermineMaxDims(&topBot, &cornerDims, leftMargin, rightMargin,
				&topHeight, &botHeight, &workWidth);
	layout->info.y_pos += topHeight;
	layout->left       += leftMargin;
	layout->right      += rightMargin;
	DetermineFlowConstraints(layout, flowDims, layout->left + basePt.left,
				layout->right + basePt.right,
				layout->info.y_pos,
				&leftPt, &rightPt);

	/*
	 * get rid of the leftMargin and rightMargin values in maxWidth
	 * otherwise the use of layout->left & layout->right will double
	 * the value.
	 */
	workWidth = workWidth - leftMargin - rightMargin;
	if (layout->max_width < workWidth + basePt.left + basePt.right +
					layout->left + layout->right)
	    layout->max_width = workWidth + basePt.left + basePt.right +
					layout->left + layout->right;
      }

    /*
     * Find the fstyle element.
     */
    fstyleEl   = FindElement(eForm->seg_list, SdlElementFstyle);
    fstyleList = fstyleEl->seg_list;

    /*
     * Find the fdata element.
     */
    fdataEl   = FindElement(formList, SdlElementFdata);
    fdataList = fdataEl->seg_list;

    /*
     * <!ELEMENT fstyle    - - (frowvec+) >
     * find out how many rows there are.
     */
    for (rowVecList = fstyleList, maxRows = 0; rowVecList != NULL; maxRows++)
	rowVecList = rowVecList->next_seg;

    if (maxRows == 0)
	return;

    /*
     * get the number of columns and the column widths
     */
    maxCols  = fstyleEl->attributes.num_vals.ncols;
    widthStr = eForm->attributes.strings.el_strs.colw;
    colStr   = eForm->attributes.strings.el_strs.colj;

    if (maxCols < 1)
	maxCols = 1;

    if (widthStr == NULL)
	widthStr = "";

    /*
     * turn the string specifying column widths into units.
     */
    workWidth  = layout->max_width - basePt.left - basePt.right
						- layout->left - layout->right;
    if (maxCols != 1)
      {
        colSpecs = (ColumnSpec *) malloc (sizeof(ColumnSpec) * maxCols);
	if (colSpecs == NULL)
	    return;
      }
    if (maxRows != 1)
      {
        rowSpecs = (RowSpec *) malloc (sizeof(RowSpec) * maxRows);
	if (rowSpecs == NULL)
	  {
	    if (maxCols > 1)
	        free(colSpecs);
	    return;
	  }
      }
    if (maxRows != 1 || maxCols != 1)
      {
        cellInfo = (CECellInfo *) malloc (sizeof(CECellInfo)*maxCols*maxRows);
	if (cellInfo == NULL)
	  {
	    if (maxCols > 1)
	        free(colSpecs);
	    if (maxRows > 1)
	        free(rowSpecs);
	    return;
	  }
      }

    for (col = 0, divisor = 0; col < maxCols; col++)
      {
	SkipToNumber(&widthStr);
	a = GetValueFromString(&widthStr, 1);
	b = GetValueFromString(&widthStr, 0);
	c = GetValueFromString(&widthStr, b);
	colJustify = GetJustifyFromString(&colStr, colJustify);

	colSpecs[col].min     = a - c;
	colSpecs[col].actual  = a;
	colSpecs[col].max     = a + b;
	colSpecs[col].justify = colJustify;

	divisor += colSpecs[col].actual;

	/*
	 * skip to the next set
	 */
	while (*widthStr != '\0' && *widthStr != ' ')
	    widthStr++;
	while (*widthStr != '\0' && *widthStr == ' ')
	    widthStr++;

	for (row = 0; row < maxRows; row++)
	  {
	    cellInfo[row * maxCols + col].cell_seg = NULL;
	    cellInfo[row * maxCols + col].my_height= 0;
	    cellInfo[row * maxCols + col].col_spn  = 0;
	    cellInfo[row * maxCols + col].row_spn  = 0;
	    cellInfo[row * maxCols + col].begin_gr = -1;
	    cellInfo[row * maxCols + col].end_gr   = -1;
	  }
      }

    /*
     * initialize the row specs
     */
    for (row = 0, rowVecList = fstyleList; row < maxRows;
				row++, rowVecList = rowVecList->next_seg)
      {
	rowSpecs[row].height  = 0;
	rowSpecs[row].next_id = CellsAttribute(_CEElementOfSegment(rowVecList));
      }

    /*
     * now figure the real values
     */
    if (divisor < 1)
	divisor = 1;
    for (col = 0; col < maxCols; col++)
      {
	colSpecs[col].min    = workWidth * colSpecs[col].min    / divisor;
	colSpecs[col].actual = workWidth * colSpecs[col].actual / divisor;
	colSpecs[col].max    = workWidth * colSpecs[col].max    / divisor;
      }

    /*
     * the only item(s) in the fstyle list must be the <frowvec> elements
     */
    tableYpos = layout->info.y_pos;
    maxRowSpn = 1;
    for (col = 0; col < maxCols; col++)
      {
	/*
	 * remember where this column starts.
	 */
	saveHyperCnt = canvas->hyper_count;
        saveLnStart  = canvas->line_cnt;
        saveTxtStart = canvas->txt_cnt;
        saveLeft     = layout->left;
        saveRight    = layout->right;

        do {
	    /*
	     * reset the counts to the start
	     */
	    canvas->hyper_count = saveHyperCnt;
	    canvas->line_cnt    = saveLnStart;
	    canvas->txt_cnt     = saveTxtStart;
	    layout->left        = saveLeft;
	    layout->right       = saveRight;

	    /*
	     * for each row, format the cell in this columns
	     */
	    for (row = 0, redo = False, cell = col;
				row < maxRows && redo == False;
						row++, cell += maxCols)
	      {
		/*
		 * layout all the cells jammed to the top of the table.
		 * later, they moved down to their position.
		 */
	        layout->info.y_pos = tableYpos;
	        redo = ResolveCell(canvas, layout, eForm,
				    colSpecs, rowSpecs, col, row,
				    maxCols, maxRows, &fdataList, cellInfo);
		if (maxRowSpn < cellInfo[cell].row_spn)
		    maxRowSpn = cellInfo[cell].row_spn;

	        /*
	         * check to see if the specified anchor has been found in
	         * this row. If so, save some information for later use.
	         */
	        if (anchorRow == -1 && oldFound != layout->id_found)
		    anchorRow = row;
	      }

          } while (redo == True);
	
	/*
	 * push the next column to the left by size of this column
	 */
	layout->left += colSpecs[col].actual;
      }

    /*
     * check to see if the fdata element's segment list was NULL.
     * if so, attach the new list to it so that we don't drop memory
     * on the floor.
     */
    if (fdataEl->seg_list == NULL)
        fdataEl->seg_list = fdataList;

    /*
     * Now go back and search for zeroed row heights and fill them in
     * based on spanned rows. This can only happen if maxRowSpn is
     * greater than 1! Otherwise, a row height really did end up zero!
     */
    if (maxRowSpn > 1)
      {
	/*
	 * try to resolve the zero height rows
	 */
	for (a = 1, redo = True; redo && a < maxRowSpn; a++)
	  {
	    redo = False;
	    for (row = 0; row < maxRows; row++)
	      {
	        if (rowSpecs[row].height == 0 &&
		    ResolveHeight(rowSpecs,cellInfo,maxCols,maxRows,row,a) == 0)
			redo = True;
	      }
	  }
	
	/*
	 * if any of the rows comes up unresolved, force to an average
	 * line height.
	 */
	if (redo)
	  {
	    for (row = 0; row < maxRows; row++)
	      {
	        if (rowSpecs[row].height == 0)
		    rowSpecs[row].height = canvas->font_info.ascent +
						canvas->font_info.descent + 1;
	      }
	  }

        /*
         * Now, double check that the row heights will accomodate
	 * all the cells.
         */
        for (row = 0; row < maxRows; row++)
	    for (col = 0; col < maxCols; col++)
	        AdjustHeight(canvas,cellInfo,rowSpecs,maxRows,maxCols,row,col);
      }

    /*
     * now reposition the cells based on the final row heights.
     */
    layout->info.y_pos = tableYpos;
    for (tableYpos = 0, cell = 0, row = 0; row < maxRows;
				tableYpos += rowSpecs[row++].height)
      {
	/*
	 * check to see if the specified anchor has been found in this
	 * row. If so, adjust the found position.
	 */
	if (anchorRow == row)
	    layout->id_Ypos += tableYpos;

	for (col = 0; col < maxCols; col++, cell++)
	  {
	    if (cellInfo[cell].cell_seg != NULL)
	      {
		for (newHeight = 0, a = 0; a < cellInfo[cell].row_spn; a++)
		    newHeight += rowSpecs[row + a].height;

	        AdjustObjectPosition(canvas,
				TxtVertJustify(
					_CEElementOfSegment(
						cellInfo[cell].cell_seg)),
				cellInfo[cell].begin_txt,
				cellInfo[cell].begin_gr ,
				cellInfo[cell].begin_ln ,
				cellInfo[cell].end_txt  ,
				cellInfo[cell].end_gr   ,
				cellInfo[cell].end_ln   ,
				cellInfo[cell].border   ,
				newHeight - cellInfo[cell].my_height,
				tableYpos);
	      }
	  }
      }

    /*
     * increment the maximum y.
     */
    layout->info.y_pos += tableYpos;

    /*
     * adjust the head positions
     */
    if (haveHeads == True)
      {
	DetermineHeadPositioning (&topBot, &sideDims, &cornerDims, &flowDims,
			saveYpos, topHeight,
			layout->info.y_pos - saveYpos - topHeight,
			NULL);
	while (headList != NULL && _CEIsElement(headList->seg_type) &&
		_CEElementOfSegment(headList)->el_type == SdlElementHead)
	  {
	    AdjustHeadPosition (canvas, headList, &topBot, &sideDims,
				&cornerDims, &flowDims,
				saveYpos, 0, workWidth,
				leftMargin, rightMargin);
	    headList = headList->next_seg;
	  }
	
	layout->info.y_pos += botHeight;
      }

    if (maxCols > 1)
        free(colSpecs);
    if (maxRows > 1)
        free(rowSpecs);
    if (maxRows > 1 || maxCols > 1)
        free(cellInfo);
}

/******************************************************************************
 * Function: ProcessSubHead
 *
 * Purpose: Determine the formatting of the subhead object,
 *          but *NOT* the correct x and y positioning.
 *	    The information is formatted as if the parent's margins are
 *	    all there is.
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
ProcessSubHead(canvas, layout, subSeg, head_width)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*subSeg;
    Unit		 head_width;
#else
ProcessSubHead(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*subSeg,
    Unit		 head_width)
#endif /* _NO_PROTO */
{
    Unit	   saveMaxWidth = layout->max_width;
    Unit	   saveMaxX     = layout->info.max_x_pos;
    Unit	   saveYpos     = layout->info.y_pos;
    Unit	   saveLeft     = layout->left;
    Unit	   saveRight	= layout->right;
    Unit	   saveFirst    = layout->first;
    CEBoolean	   saveStatic   = layout->stat_flag;
    CEElement     *pEl		= _CEElementOfSegment(subSeg);
    enum SdlOption orient	= ObjHorizOrient(pEl);
    enum SdlOption vOrient	= ObjVertOrient(pEl);
    SDLFrmtInfo   *frmtInfo     = FrmtInfoPtr(pEl);
    DataPoint      dataPt;


    /*
     * Save where the graphics and text for this subhead start
     */
    frmtInfo->beg_txt = canvas->txt_cnt;
    if (frmtInfo->beg_gr == -1)
        frmtInfo->beg_gr  = canvas->graphic_cnt;

    /*
     * disallow some of the combinations
     */
    if ((orient == SdlJustifyCenter && vOrient != SdlJustifyTop) ||
    	(vOrient == SdlJustifyCenter &&
			orient != SdlJustifyLeft && orient != SdlJustifyRight))
      {
	vOrient = SdlJustifyBottom;
	ObjVertOrient(pEl) = SdlJustifyBottom;
      }

    if (pEl->attributes.flow == SdlWrap &&
	(vOrient != SdlJustifyTop ||
		(vOrient == SdlJustifyTop &&
			orient != SdlJustifyLeft && orient != SdlJustifyRight)))
	pEl->attributes.flow = SdlNoWrap;

    layout->parent_el  = pEl;

    /*
     * move the formatting location offset from zero, zero.
     */
    dataPt.left   = layout->left;
    dataPt.right  = layout->right;
    dataPt.y_pos  = _CEFORMAT_ALL;

    /*
     * get the subhead specific margins.
     */
    layout->left  = CalcOffset(canvas, pEl->attributes.frmt_specs.lmargin);
    layout->right = CalcOffset(canvas, pEl->attributes.frmt_specs.rmargin);
    if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_FMARGIN))
        layout->first = CalcOffset(canvas, pEl->attributes.frmt_specs.fmargin);

    /*
     * Figure out the bounding box within the head margins for the subhead.
     * Right now, we want to just format the subhead within it's own
     * little box. Don't care about positioning.
     */
    layout->max_width        = head_width
				* atoi(pEl->attributes.headw) / HEADW_DIVISOR
				+ dataPt.left + dataPt.right;
    layout->info.max_x_pos = 0;

    /*
     * Set the text position here, because the left or right might have
     * changed via the previous code
     */
    PushDataPoint(layout, &dataPt);
    SetMargins(layout);
    SetTextPosition(layout, True);

    /*
     * add some space at the top if needed
     */
    _DtHelpCeAddLines (canvas, pEl->attributes.frmt_specs.tmargin, True,
						&(layout->info.y_pos));

    /*
     * get the new justification.
     * calculate the new starting position.
     */
    if (pEl->attributes.type == SdlTypeLiteral ||
					pEl->attributes.type == SdlTypeLined)
        layout->stat_flag = True;

    layout->txt_justify = pEl->attributes.frmt_specs.justify;

    /*
     * format the subhead element
     */
    layout->cur_start   = 0;
    ProcessSegmentList(canvas, layout, pEl->seg_list, NULL);

    /*
     * save the buffered information
     * restore to the parent element's text/info justification
     */
    CheckSaveInfo (canvas, layout);

    /*
     * add some space at the bottom if needed
     */
    _DtHelpCeAddLines (canvas, pEl->attributes.frmt_specs.bmargin, True,
							&(layout->info.y_pos));

    /*
     * Remove the data point and fill out the rest of the information
     * required to properly place the sub heading later.
     */
    RemoveDataPoint(layout, &dataPt);
    frmtInfo->end_txt = canvas->txt_cnt;
    if (frmtInfo->end_gr == -1)
        frmtInfo->end_gr = canvas->graphic_cnt;
    frmtInfo->y_units = layout->info.y_pos - saveYpos;
    frmtInfo->x_units = dataPt.x_units;

    /*
     * restore correct values and rest back to where the sub head started
     */
    layout->info.max_x_pos = saveMaxX;
    layout->info.y_pos     = saveYpos;

    /*
     * restore parent information
     */
    layout->max_width      = saveMaxWidth;
    layout->left	   = saveLeft;
    layout->right          = saveRight;
    layout->first          = saveFirst;
    layout->stat_flag      = saveStatic;
    return;
}

/******************************************************************************
 * Function: UpdateDimensionArrays
 *
 * Purpose: Based on the object's orientation and justification,
 *	    update the correct dimension array(s).
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
UpdateDimensionArrays(p_seg, top_bot, side, corner, flow, max_left, max_right)
    CESegment	*p_seg;
    TopDims	*top_bot;
    SideDims	*side;
    CornerDims	*corner;
    FlowDims	*flow;
    Unit	*max_left;
    Unit	*max_right;
#else
UpdateDimensionArrays(
    CESegment	*p_seg,
    TopDims	*top_bot,
    SideDims	*side,
    CornerDims	*corner,
    FlowDims	*flow,
    Unit	*max_left,
    Unit	*max_right)
#endif /* _NO_PROTO */
{
    int		    i;
    int		    j;
    int		   *marginPtr;
    CEElement	   *pEl      = _CEElementOfSegment(p_seg);
    SDLFrmtInfo	   *frmtInfo = FrmtInfoPtr(pEl);
    enum SdlOption  orient   = ObjHorizOrient(pEl);
    enum SdlOption  vOrient  = ObjVertOrient(pEl);

    /*
     * modify the width that headSize should be
     */
    j = DIMS_LM;
    i = DIMS_TOP;
    switch (orient)
      {
	case SdlJustifyRightMargin:
		j++;
	case SdlJustifyCenter:
		j++;
	case SdlJustifyLeftMargin:
		if (vOrient == SdlJustifyBottom)
		    i = DIMS_BOTTOM;

		if ((*top_bot)[i][j][DIMS_WIDTH] < frmtInfo->x_units)
		    (*top_bot)[i][j][DIMS_WIDTH] = frmtInfo->x_units;

		(*top_bot)[i][j][DIMS_HEIGHT] += frmtInfo->y_units;
		break;

      }

    /*
     * check left & right margins.
     */
    marginPtr = max_left;
    j = DIMS_LEFT;
    i = DIMS_TOP;
    switch(orient)
      {

	case SdlJustifyRightCorner:
	case SdlJustifyRight:
		j = DIMS_RIGHT;
		marginPtr = max_right;

	case SdlJustifyLeftCorner:
	case SdlJustifyLeft:
		if (vOrient != SdlJustifyTop)
		    i++;
		if (vOrient == SdlJustifyBottom)
		    i++;

		/*
		 * push i to zero or 4
		 */
		if (orient == SdlJustifyRightCorner ||
					    orient == SdlJustifyLeftCorner)
		  {
		    if (i) i = DIMS_BC;
		    (*corner)[i][j] += frmtInfo->y_units;
		    if (*marginPtr < frmtInfo->x_units)
		        *marginPtr = frmtInfo->x_units;
		  }
		else if (FlowAttr(pEl) != SdlWrap)
		  {
		    (*side)[i][j] += frmtInfo->y_units;
		    if (*marginPtr < frmtInfo->x_units)
		        *marginPtr = frmtInfo->x_units;
		  }
		else
		  {
		    (*flow)[j][DIMS_HEIGHT] += frmtInfo->y_units;
		    if ((*flow)[j][DIMS_WIDTH] < frmtInfo->x_units)
			(*flow)[j][DIMS_WIDTH] = frmtInfo->x_units;
		  }
		break;
      }
}

/******************************************************************************
 * Function: DetermineMaxDims
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DetermineMaxDims(top_bot, corner, left_margin, right_margin,
		top_height, bot_height, max_width)
    TopDims	*top_bot;
    CornerDims	*corner;
    Unit	 left_margin;
    Unit	 right_margin;
    Unit	*top_height;
    Unit	*bot_height;
    Unit	*max_width;
#else
DetermineMaxDims(
    TopDims	*top_bot,
    CornerDims	*corner,
    Unit	 left_margin,
    Unit	 right_margin,
    Unit	*top_height,
    Unit	*bot_height,
    Unit	*max_width)
#endif /* _NO_PROTO */
{
    register int	j;
    Unit     topWidth;
    Unit     botWidth;

    /*
     * now process all the information gathered about the (sub)headings
     * to determine the bounding box for the (head) txt. Start by figuring
     * out the maximums for the dimensions.
     *
     * figure the current top and bottom max widths.
     */
    topWidth  = left_margin + right_margin;
    botWidth  = left_margin + right_margin;
    *top_height = 0;
    *bot_height = 0;
    for (j = DIMS_LM; j < DIMS_RM; j++)
      {
	topWidth = topWidth + (*top_bot)[DIMS_TOP]   [j][DIMS_WIDTH];
	botWidth = botWidth + (*top_bot)[DIMS_BOTTOM][j][DIMS_WIDTH];

	if (*top_height < (*top_bot)[DIMS_TOP][j][DIMS_HEIGHT])
	    *top_height = (*top_bot)[DIMS_TOP][j][DIMS_HEIGHT];

	if (*bot_height < (*top_bot)[DIMS_BOTTOM][j][DIMS_HEIGHT])
	    *bot_height = (*top_bot)[DIMS_BOTTOM][j][DIMS_HEIGHT];
      }

    /*
     * for the maximum top and bottom heights, take into
     * consideration the corner values
     */
    if (*top_height < (*corner)[DIMS_TC][DIMS_LEFT])
	*top_height = (*corner)[DIMS_TC][DIMS_LEFT];
    if (*top_height < (*corner)[DIMS_TC][DIMS_RIGHT])
	*top_height = (*corner)[DIMS_TC][DIMS_RIGHT];

    if (*bot_height < (*corner)[DIMS_BC][DIMS_LEFT])
	*bot_height = (*corner)[DIMS_BC][DIMS_LEFT];
    if (*bot_height < (*corner)[DIMS_BC][DIMS_RIGHT])
	*bot_height = (*corner)[DIMS_BC][DIMS_RIGHT];

    *max_width = topWidth;
    if (*max_width < botWidth)
	*max_width = botWidth;

}

/******************************************************************************
 * Function: DetermineFlowConstraints
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DetermineFlowConstraints(layout, flow_dims, left_margin, right_margin, start_y,
			left_pt, right_pt)
    LayoutInfo	*layout;
    FlowDims	 flow_dims;
    Unit	 left_margin;
    Unit	 right_margin;
    Unit	 start_y;
    DataPoint	*left_pt;
    DataPoint	*right_pt;
#else
DetermineFlowConstraints(
    LayoutInfo	*layout,
    FlowDims	 flow_dims,
    Unit	 left_margin,
    Unit	 right_margin,
    Unit	 start_y,
    DataPoint	*left_pt,
    DataPoint	*right_pt)
#endif /* _NO_PROTO */
{
    Unit	leftSide  = flow_dims[DIMS_LEFT][DIMS_HEIGHT];
    Unit	rightSide = flow_dims[DIMS_RIGHT][DIMS_HEIGHT];

    /*
     * Now, if there is flowing text required, put points on the
     * stack to indicate them.
     */
    left_margin  += flow_dims[DIMS_LEFT][DIMS_WIDTH];
    right_margin += flow_dims[DIMS_RIGHT][DIMS_WIDTH];

    GetCurrentDataPoint(layout, left_pt);
    GetCurrentDataPoint(layout, right_pt);
    left_pt->left   += left_margin;
    left_pt->right  += right_margin;
    left_pt->y_pos   = _CEFORMAT_ALL;
    right_pt->left  += left_margin;
    right_pt->right += right_margin;
    right_pt->y_pos  = _CEFORMAT_ALL;

    while (leftSide > 0 || rightSide > 0)
      {
	if (leftSide > 0)
	  {
	    if (rightSide == 0 || leftSide <= rightSide)
	      {
		left_pt->right = right_margin;
		left_pt->y_pos = start_y + leftSide;
		if (leftSide != rightSide)
		    left_margin = 0;
		leftSide = 0;
		InsertDataPoint(layout, left_pt);
	      }
	  }

	if (rightSide > 0)
	  {
	    if (leftSide == 0 || leftSide > rightSide)
	      {
		right_pt->left  = left_margin;
		right_pt->y_pos = start_y + rightSide;
		if (leftSide != rightSide)
		    right_margin = 0;
		rightSide = 0;
		InsertDataPoint(layout, right_pt);
	      }
	  }
      }
}

/******************************************************************************
 * Function: DetermineHeadPositioning
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DetermineHeadPositioning(top_bot, side, corner, flow,
			start_y, max_top, block_size, ret_side_size)
    TopDims	*top_bot;
    SideDims	*side;
    CornerDims	*corner;
    FlowDims	*flow;
    Unit	 start_y;
    Unit	 max_top;
    Unit	 block_size;
    Unit	*ret_side_size;
#else
DetermineHeadPositioning(
    TopDims	*top_bot,
    SideDims	*side,
    CornerDims	*corner,
    FlowDims	*flow,
    Unit	 start_y,
    Unit	 max_top,
    Unit	 block_size,
    Unit	*ret_side_size)
#endif /* _NO_PROTO */
{
    int    i;
    Unit   leftSideHeight  = 0;
    Unit   rightSideHeight = 0;
    Unit   sideHeight      = 0;

    /*
     * determine the maximum side heights
     */
    for (i = DIMS_TOP; i < DIMS_BOTTOM; i++)
      {
	leftSideHeight  += (*side)[i][DIMS_LEFT];
	rightSideHeight += (*side)[i][DIMS_RIGHT];
      }

    /*
     * determine the maximum side height
     */
    sideHeight = block_size;
    if (sideHeight < leftSideHeight)
	sideHeight = leftSideHeight;
    if (sideHeight < rightSideHeight)
	sideHeight = rightSideHeight;
    if (sideHeight < (*flow)[DIMS_LEFT][DIMS_HEIGHT])
        sideHeight  = (*flow)[DIMS_LEFT][DIMS_HEIGHT];
    if (sideHeight < (*flow)[DIMS_RIGHT][DIMS_HEIGHT])
        sideHeight = (*flow)[DIMS_RIGHT][DIMS_HEIGHT];

    /*
     * calculate the starting Y position for each of the positions
     * reuse the arrays that were used to save the max dimension values.
     */
    for (i = DIMS_LM; i < DIMS_RM; i++)
      {
	(*top_bot)[DIMS_TOP]   [i][DIMS_YPOS] = start_y;
	(*top_bot)[DIMS_BOTTOM][i][DIMS_YPOS] = start_y + max_top + sideHeight;
      }

    (*corner)[DIMS_TC][DIMS_LEFT]  = start_y;
    (*corner)[DIMS_TC][DIMS_RIGHT] = start_y;

    (*corner)[DIMS_BC][DIMS_LEFT]  = start_y + max_top + sideHeight;
    (*corner)[DIMS_BC][DIMS_RIGHT] = start_y + max_top + sideHeight;

    (*side)[DIMS_TOP][DIMS_LEFT]  = start_y + max_top;
    (*side)[DIMS_TOP][DIMS_RIGHT] = start_y + max_top;

    (*flow)[DIMS_LEFT ][DIMS_YPOS] = start_y + max_top;
    (*flow)[DIMS_RIGHT][DIMS_YPOS] = start_y + max_top;

    (*side)[DIMS_CENTER][DIMS_LEFT]  = start_y + max_top +
			(sideHeight - (*side)[DIMS_CENTER][DIMS_LEFT]) / 2;
    (*side)[DIMS_CENTER][DIMS_RIGHT] = start_y + max_top +
			(sideHeight - (*side)[DIMS_CENTER][DIMS_RIGHT]) / 2;

    (*side)[DIMS_BOTTOM][DIMS_LEFT]  = start_y + max_top +
			sideHeight - (*side)[DIMS_BOTTOM][DIMS_LEFT];
    (*side)[DIMS_BOTTOM][DIMS_RIGHT] = start_y + max_top +
			sideHeight - (*side)[DIMS_BOTTOM][DIMS_RIGHT];

    if (ret_side_size != NULL)
	*ret_side_size = sideHeight;
}

/******************************************************************************
 * Function: AdjustHead
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustHeadPosition(canvas, p_seg, top_bot, side, corner, flow,
		base_y, base_left, block_width, left_margin, right_margin)
    CECanvasStruct	*canvas;
    CESegment		*p_seg;
    TopDims		*top_bot;
    SideDims		*side;
    CornerDims		*corner;
    FlowDims		*flow;
    Unit		 base_y;
    Unit		 base_left;
    Unit		 block_width;
    Unit		 left_margin;
    Unit		 right_margin;
#else
AdjustHeadPosition(
    CECanvasStruct	*canvas,
    CESegment		*p_seg,
    TopDims		*top_bot,
    SideDims		*side,
    CornerDims		*corner,
    FlowDims		*flow,
    Unit		 base_y,
    Unit		 base_left,
    Unit		 block_width,
    Unit		 left_margin,
    Unit		 right_margin)
#endif /* _NO_PROTO */
{
    int		    i        = DIMS_TOP;	/* also DIMS_TC */
    int		    j        = DIMS_LEFT;	/* also DIMS_LM */
    int		    divisor  = 2;
    Unit	    adjustX  = 0;
    Unit	    adjustY  = 0;
    Unit	    newY     = 0;
    CEElement	   *pEl      = _CEElementOfSegment(p_seg);
    SDLFrmtInfo	   *frmtInfo = FrmtInfoPtr(pEl);
    enum SdlOption  orient   = ObjHorizOrient(pEl);
    enum SdlOption  vOrient  = ObjVertOrient(pEl);

    if (atoi(pEl->attributes.headw) == 10000 && orient == SdlJustifyCenter
				&& TxtHorizJustify(pEl) == SdlJustifyLeft)
	frmtInfo->x_units = block_width - left_margin - right_margin;

    switch (orient)
      {
	case SdlJustifyRightMargin:
		divisor = 1;
		j++;
	case SdlJustifyCenter:
		adjustX = (block_width - left_margin - right_margin
					    - frmtInfo->x_units) / divisor;
		j++;
	case SdlJustifyLeftMargin:
		adjustX += left_margin;
		if (vOrient == SdlJustifyBottom)
		    i = DIMS_BOTTOM;

		newY  = (*top_bot)[i][j][DIMS_YPOS];
		(*top_bot)[i][j][DIMS_YPOS] += frmtInfo->y_units;
		break;

	case SdlJustifyRightCorner:
		adjustX = block_width + left_margin;
		j       = DIMS_RIGHT;

	case SdlJustifyLeftCorner:
		if (vOrient == SdlJustifyBottom)
		    i = DIMS_BC;

		newY = (*corner)[i][j];
		(*corner)[i][j] += frmtInfo->y_units;
		break;

	case SdlJustifyRight:
		adjustX = block_width + left_margin;
		j = DIMS_RIGHT;

	case SdlJustifyLeft:
		if (vOrient != SdlJustifyTop)
		    i++;
		if (vOrient == SdlJustifyBottom)
		    i++;

		if (FlowAttr(pEl) == SdlWrap)
		  {
		    if (orient == SdlJustifyLeft)
			adjustX += left_margin;
		    else
			adjustX -= frmtInfo->x_units;

		    newY = (*flow)[j][DIMS_YPOS];
		    (*flow)[j][DIMS_YPOS] += frmtInfo->y_units;
		  }
		else
		  {
		    newY = (*side)[i][j];
		    (*side)[i][j] += frmtInfo->y_units;
		  }
		break;
      }

    adjustY  = newY - base_y;
    adjustX += base_left;
    for (i = frmtInfo->beg_txt; i < frmtInfo->end_txt; i++)
      {
	canvas->txt_lst[i].baseline += adjustY;
	canvas->txt_lst[i].text_x   += adjustX;
      }

    for (i = frmtInfo->beg_gr; i < frmtInfo->end_gr; i++)
      {
	if (canvas->graphic_lst[i].pos_y != -1
	    && canvas->graphic_lst[i].pos_x != -1)
	  {
	    canvas->graphic_lst[i].pos_y += adjustY;
	    canvas->graphic_lst[i].pos_x += adjustX;
	  }
      }
}

/******************************************************************************
 * Function: InitDimArrays
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
InitDimArrays(top_bot, side, corner, flow)
    TopDims		*top_bot;
    SideDims		*side;
    CornerDims		*corner;
    FlowDims		*flow;
#else
InitDimArrays(
    TopDims		*top_bot,
    SideDims		*side,
    CornerDims		*corner,
    FlowDims		*flow)
#endif /* _NO_PROTO */
{
    int		    i;
    int		    j;

    for (i = DIMS_TOP; i <= DIMS_BOTTOM; i++)
      {
	for (j = DIMS_LM; j <= DIMS_RM; j++)
	  {
	    (*top_bot)[i][j][DIMS_WIDTH] = 0;
	    (*top_bot)[i][j][DIMS_HEIGHT] = 0;
	  }
	for (j = DIMS_LEFT; j <= DIMS_RIGHT; j++)
	    (*side)[i][j] = 0;
      }
    for (i = DIMS_LEFT; i <= DIMS_RIGHT; i++)
      {
	for (j = DIMS_WIDTH; j <= DIMS_HEIGHT; j++)
	 {
	    (*corner)[i][j] = 0;
	    (*flow)[i][j]   = 0;
	 }
      }
}

/******************************************************************************
 * Function: ProcessHead
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
ProcessHead(canvas, layout, head_seg, join_line)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*head_seg;
    int			*join_line;
#else
ProcessHead(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*head_seg,
    int			*join_line)
#endif /* _NO_PROTO */
{
    int		   txtCount;
    int		   hyperCnt;

    Unit	   objectSize;
    Unit	   contentSize;
    Unit	   headYpos;
    Unit	   startYpos  = layout->info.y_pos;

    Unit	   leftMargin   = 0;
    Unit	   rightMargin  = 0;

    Unit	   saveMaxX     = layout->info.max_x_pos;
    Unit	   saveMaxWidth = layout->max_width;

    Unit	   maxWidth;
    Unit	   topHeight;
    Unit	   botHeight;

    CEBoolean	   redo;
    CESegment	  *headList;
    CEElement     *pEl	        = _CEElementOfSegment(head_seg);
    enum SdlOption orient       = ObjHorizOrient(pEl);
    enum SdlOption vOrient      = ObjVertOrient(pEl);
    enum SdlOption flow         = FlowAttr(pEl);
    SDLFrmtInfo    *frmtInfo    = FrmtInfoPtr(pEl);
    DataPoint	    basePt;
    DataPoint	    leftPt;
    DataPoint	    rightPt;

    TopDims	topbotDims;
    SideDims	sideDims;
    CornerDims	cornerDims;
    FlowDims	flowDims;

    /*
     */
    InitDimArrays(&topbotDims, &sideDims, &cornerDims, &flowDims);

    /*
     * disallow some of the orientation & vOrient combinations
     */
    if ((orient == SdlJustifyCenter && vOrient != SdlJustifyBottom) ||
    	(vOrient == SdlJustifyCenter &&
			orient != SdlJustifyLeft && orient != SdlJustifyRight))
      {
	vOrient = SdlJustifyTop;
	ObjVertOrient(pEl) = SdlJustifyTop;
      }

    if (flow == SdlWrap &&
		(vOrient != SdlJustifyTop || (vOrient == SdlJustifyTop &&
			orient != SdlJustifyLeft && orient != SdlJustifyRight)))
      {
	flow = SdlNoWrap;
	pEl->attributes.flow = SdlNoWrap;
      }

    if (flow == SdlWrapJoin &&
		(vOrient != SdlJustifyTop || orient != SdlJustifyLeftMargin))
      {
	vOrient = SdlJustifyTop;
	orient  = SdlJustifyLeftMargin;
	ObjVertOrient(pEl)  = SdlJustifyTop;
	ObjHorizOrient(pEl) = SdlJustifyLeftMargin;
      }

    /*
     * the head object begins here. The head object includes the subheads.
     */
    frmtInfo->beg_txt = canvas->txt_cnt;
    if (frmtInfo->beg_gr == -1)
        frmtInfo->beg_gr  = canvas->graphic_cnt;

    /*
     * work out the amount of room the head object will occupy overall.
     */
    GetCurrentDataPoint(layout, &basePt);
    contentSize = (layout->max_width - basePt.left - basePt.right)
			* (atoi(pEl->attributes.headw)) / HEADW_DIVISOR
			- layout->left - layout->right;

    /*
     * get the new txt justification.
     * calculate the new starting position.
     */
    layout->txt_justify = pEl->attributes.frmt_specs.justify;
    layout->stat_flag   = False;
    if (pEl->attributes.type == SdlTypeLiteral ||
					pEl->attributes.type == SdlTypeLined)
	layout->stat_flag = True;

    /*
     * add some space at the top if needed
     */
    _DtHelpCeAddLines (canvas, pEl->attributes.frmt_specs.tmargin, True,
						&(layout->info.y_pos));

    /*
     * format the subhead objects. This will format the subheads
     * constrained to the room they are limited to. This will *NOT*
     * position the subheads.
     */
    headList    = pEl->seg_list;
    headYpos    = layout->info.y_pos;
    while (headList != NULL && _CEIsElement (headList->seg_type) &&
		_CEElementOfSegment(headList)->el_type == SdlElementSubHead)
      {
	/*
	 * format the subheading.
	 */
        layout->cur_start   = 0;
	ProcessSubHead(canvas, layout, headList, contentSize);
	layout->info.y_pos = headYpos;

	/*
	 * fill out the dimension arrays.
	 */
	UpdateDimensionArrays(headList, &topbotDims, &sideDims, &cornerDims,
					&flowDims, &leftMargin, &rightMargin);
	headList = headList->next_seg;
      }

    /*
     * now process all the information gathered about the subheadings
     * to determine the bounding box for the head txt. Start by figuring
     * out the maximums for the dimensions.
     *
     * figure the current top and bottom max widths.
     */
    DetermineMaxDims(&topbotDims, &cornerDims, leftMargin, rightMargin,
		&topHeight, &botHeight, &maxWidth);

    /*
     * Figure out if the currently allowed headSize is smaller
     * than the minimum size required for the top or bottom
     * headings.
     */
    if (contentSize < maxWidth)
	contentSize = maxWidth;

    /*
     * With left and right margins, this defines the bounding box
     * for the head text.
     */
    layout->info.max_x_pos  = 0;
    basePt.left            += leftMargin;
    basePt.right           += rightMargin;
    basePt.y_pos            = _CEFORMAT_ALL;
    PushDataPoint(layout, &basePt);

    objectSize = contentSize + layout->left + layout->right
						+ basePt.left + basePt.right;

    /*
     * Now, if there is flowing text required, put points on the
     * stack to indicate them.
     */
    leftPt  = basePt;
    rightPt = basePt;
    DetermineFlowConstraints(layout, flowDims, 0, 0, headYpos,
				&leftPt, &rightPt);
    /*
     * until the head txt quits overrunning its bounding box, keep
     * reformatting. Position it at it's absolute position.
     */
    do
      {
	redo = False;
        SetMargins(layout);
        SetTextPosition(layout, True);

        /*
         * now process the head itself
         */
	hyperCnt           = canvas->hyper_count;
	txtCount           = canvas->txt_cnt;
	layout->info.y_pos = headYpos + topHeight;
        layout->max_width  = objectSize;
        layout->cur_start  = 0;
        ProcessSegmentList(canvas, layout, headList, NULL);
	CheckSaveInfo(canvas, layout);

        /*
         * remove the data points from the stack
         */
        RemoveDataPoint(layout, &rightPt);
        RemoveDataPoint(layout, &leftPt);

	if (leftPt.y_pos > 0 &&
		objectSize < leftPt.left + leftPt.right + leftPt.x_units)
	  {
	    objectSize = leftPt.left + leftPt.right + leftPt.x_units;
	    redo = True;
	  }

	if (rightPt.y_pos > 0 &&
		objectSize < rightPt.left + rightPt.right + rightPt.x_units)
	  {
	    objectSize = rightPt.left + rightPt.right + rightPt.x_units;
	    redo = True;
	  }

	if (objectSize < layout->info.max_x_pos + basePt.right + layout->right)
	  {
	    objectSize = layout->info.max_x_pos + basePt.right + layout->right;
	    redo = True;
	  }

	if (redo == True)
	  {
	    if (leftPt.y_pos > 0)
	        InsertDataPoint(layout, &leftPt);
	    if (rightPt.y_pos > 0)
	        InsertDataPoint(layout, &rightPt);
	    canvas->txt_cnt     = txtCount;
	    canvas->hyper_count = hyperCnt;
	  }

      } while (redo == True);

    /*
     * remove the base point.
     */
    RemoveDataPoint(layout, &basePt);

    /*
     * and rememeber how many units in the Y direction the head extended.
     */
    frmtInfo->y_units = layout->info.y_pos - headYpos - topHeight;
    frmtInfo->x_units = basePt.x_units - leftMargin;
    frmtInfo->end_txt = canvas->txt_cnt;
    if (frmtInfo->end_gr == -1)
        frmtInfo->end_gr = canvas->graphic_cnt;

    /*
     * determine the maximum side heights and positioning.
     */
    DetermineHeadPositioning(&topbotDims, &sideDims, &cornerDims, &flowDims,
				headYpos, topHeight, frmtInfo->y_units,
				NULL);
				
    /*
     * At this point we have the head at the correct position,
     * but the subheads are jammed together. Now position the
     * items correctly around the head txt.
     */
    contentSize = objectSize - basePt.left - basePt.right
						- layout->left - layout->right;
    headList = pEl->seg_list;
    while (headList != NULL && _CEIsElement (headList->seg_type) &&
		_CEElementOfSegment(headList)->el_type == SdlElementSubHead)
      {
	AdjustHeadPosition(canvas, headList, 
			&topbotDims, &sideDims, &cornerDims, &flowDims,
			headYpos, basePt.left,
			contentSize, leftMargin, rightMargin);
	headList = headList->next_seg;
      }

    /*
     * get the ending position of the head
     */
    layout->info.y_pos += botHeight;
    _DtHelpCeAddLines (canvas, pEl->attributes.frmt_specs.bmargin, True,
							&(layout->info.y_pos));
    /*
     * fill out the numbers
     */
    frmtInfo->end_txt   = canvas->txt_cnt;
    frmtInfo->y_units   = layout->info.y_pos - startYpos;
    frmtInfo->x_units   = objectSize - basePt.left - basePt.right;

    /*
     * determine if this is the largest max x seen so far
     */
    if (saveMaxX < objectSize)
	saveMaxX = objectSize;

    /*
     * restore the max x units
     */
    layout->info.max_x_pos = saveMaxX;
    layout->max_width      = saveMaxWidth;
    layout->info.y_pos     = startYpos;

    if (join_line != NULL)
      {
        *join_line = -1;
	if (flow == SdlWrapJoin && txtCount != canvas->txt_cnt)
	    *join_line = canvas->txt_cnt - 1;
      }

    return;
}

/******************************************************************************
 * Function: AdjustForBorders
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustForBorders(canvas, layout, brdr, ret_bot)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    enum SdlOption	 brdr;
    CEBoolean		*ret_bot;
#else
AdjustForBorders(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    enum SdlOption	 brdr,
    CEBoolean		*ret_bot)
#endif /* _NO_PROTO */
{
    Unit	   pad = canvas->metrics.line_thickness
					/* + canvas->metrics.leading */;
    /*
     * check to see if this element has a border. If so, adjust the
     * boundaries.
     */
    if (brdr == SdlBorderFull || brdr == SdlBorderHorz || brdr == SdlBorderTop)
        layout->info.y_pos += pad;

    *ret_bot = False;
    if (brdr == SdlBorderFull || brdr == SdlBorderHorz || brdr==SdlBorderBottom)
        *ret_bot = True;

    if (brdr == SdlBorderFull || brdr == SdlBorderVert || brdr == SdlBorderLeft)
        layout->left += pad;

    if (brdr == SdlBorderFull || brdr == SdlBorderVert || brdr==SdlBorderRight)
        layout->right += pad;
}

/******************************************************************************
 * Function: DrawBorders
 *
 * Parameters:
 *		canvas		Specifies the virtual canvas on which
 *				lines are drawn.
 *		layout		Specifies the current layout information.
 *		brdr		Specifies the type of border.
 *		base_pt		Specifies the boundaries of the object.
 *		top_y		Specifes the top y position at which to
 *				draw a border.
 *		bot_y		Specifes the bottom y position at which
 *				to draw a border. It will be at this y
 *				position a bottom border is drawn. The
 *				side borders will extend to y + line_thickness.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
DrawBorders(canvas, layout, el_type, brdr, base_pt, top_y, bot_y, ret_width)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    enum SdlElement	 el_type;
    enum SdlOption	 brdr;
    DataPoint		 base_pt;
    Unit		 top_y;
    Unit		 bot_y;
    Unit		*ret_width;
#else
DrawBorders(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    enum SdlElement	 el_type,
    enum SdlOption	 brdr,
    DataPoint		 base_pt,
    Unit		 top_y,
    Unit		 bot_y,
    Unit		*ret_width)
#endif /* _NO_PROTO */
{
    int         mod        = 1;
    int		cnt        = canvas->line_cnt;
    Unit	line_width = canvas->metrics.line_thickness;
    Unit	width;

    switch (el_type)
      {
	case SdlElementSnb:
	case SdlElementBlockAsync:
	case SdlElementFdata:
	case SdlElementFstyle:
		if (ret_width != NULL)
		    *ret_width = 0;
		return 0;
      }

    /*
     * calculate the width of the element.
     */
    width = layout->max_width - base_pt.right;
    if (layout->info.max_x_pos + layout->right > width)
	width = layout->info.max_x_pos + layout->right;
    width -= base_pt.left;

    /*
     * If borders are specified, draw them
     */
    if (brdr != SdlBorderNone)
      {
	/*
	 * now do the horizontal borders
	 */
	switch(brdr)
	  {
	    case SdlBorderFull:
	    case SdlBorderHorz:
	    case SdlBorderBottom:
			SaveLine(canvas, CELineHorizontal, base_pt.left,
								bot_y, width);
			mod = -1;
			if (brdr == SdlBorderBottom)
			    break;
    
	    case SdlBorderTop:
			SaveLine(canvas, CELineHorizontal, base_pt.left,
								top_y, width);
	  }
    
	bot_y += line_width;
	switch(brdr)
	  {
	    case SdlBorderFull:
	    case SdlBorderVert:
	    case SdlBorderLeft:
			SaveLine(canvas, CELineVertical, base_pt.left,
							top_y, bot_y - top_y);
			if (brdr == SdlBorderLeft)
			    break;
    
	    case SdlBorderRight:
			if (layout->info.max_x_pos < base_pt.left + width)
			    layout->info.max_x_pos = base_pt.left + width;
    
			SaveLine(canvas, CELineVertical,
				base_pt.left + width - line_width,
				top_y, bot_y - top_y);
	  }
      }

    if (ret_width != NULL)
	*ret_width = width;

    return ((canvas->line_cnt - cnt) * mod);
}

/******************************************************************************
 * Function: AdjustObjectPosition
 *
 * Parameters:
 *		canvas		Specifies the virtual canvas on which
 *				lines are drawn.
 *		justify		Specifies the vertical adjustment for the
 *				object.
 *		start_txt	Specifies the start index of the text list.
 *		start_gr	Specifies the start index of the graphics list.
 *		start_ln	Specifies the start index of the line list.
 *		end_txt		Specifies the end indext of the text list.
 *		end_gr		Specifies the end indext of the graphics list.
 *		end_ln		Specifies the end indext of the line list.
 *		height_adj	Specifies the internal height adjust value.
 *		y_adj		Specifies the y position adjustment.
 *
 * Return:	nothing.
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustObjectPosition(canvas, justify, start_txt, start_gr, start_ln,
			end_txt, end_gr, end_ln, brdr_cnt, height_adj, y_adj)
    CECanvasStruct	*canvas;
    enum SdlOption	 justify;
    int			 start_txt;
    int			 start_gr;
    int			 start_ln;
    int			 end_txt;
    int			 end_gr;
    int			 end_ln;
    int			 brdr_cnt;
    Unit		 height_adj;
    Unit		 y_adj;
#else
AdjustObjectPosition(
    CECanvasStruct	*canvas,
    enum SdlOption	 justify,
    int			 start_txt,
    int			 start_gr,
    int			 start_ln,
    int			 end_txt,
    int			 end_gr,
    int			 end_ln,
    int			 brdr_cnt,
    Unit		 height_adj,
    Unit		 y_adj)
#endif /* _NO_PROTO */
{
    int   mod  = 1;
    Unit  yOff = 0;

    /*
     * If border count is negative, indicates the first line in the
     * list is bottom line. This requires special handling in adjusting
     * its position. Set flags accordingly.
     */
    if (brdr_cnt < 0)
      {
	mod = -1;
	brdr_cnt = -brdr_cnt;
      }

    /*
     * calculate the offset value within the object for other objects
     * contained in this object.
     */
    if (justify != SdlJustifyTop)
      {
	yOff = height_adj;
        if (justify == SdlJustifyCenter)
            yOff /= 2;
      }
    
    yOff += y_adj;

    /*
     * any adjustment required?
     */
    if (y_adj != 0 || yOff != 0 || height_adj != 0)
      {
	/*
	 * don't modify the border lines around this object yet.
	 */
	end_ln -= brdr_cnt;

	/*
	 * modify the border lines of the objects contained within
	 * this object.
	 */
        while (start_ln < end_ln)
          {
            canvas->line_lst[start_ln].pos_y += yOff;
            start_ln++;
          }

	/*
	 * now adjust the border lines around this object.
	 */
	end_ln += brdr_cnt;
        while (start_ln < end_ln)
          {
	    /*
	     * indicates the bottom line is the first line in the
	     * list. Move it down the height adjustment.
	     */
	    if (mod < 0)
	      {
                canvas->line_lst[start_ln].pos_y += height_adj;
		mod = 1;
	      }
	    /*
	     * stretch the vertical lines
	     */
	    else if (canvas->line_lst[start_ln].dir == CELineVertical)
                canvas->line_lst[start_ln].length += height_adj;

	    /*
	     * move the line down.
	     */
            canvas->line_lst[start_ln].pos_y += y_adj;

            start_ln++;
          }

	/*
	 * adjust the position of the graphics within this object.
	 */
        while (start_gr < end_gr)
          {
            canvas->graphic_lst[start_gr].pos_y += yOff;
            start_gr++;
          }
	/*
	 * adjust the position of the text within this object.
	 */
        while (start_txt < end_txt)
          {
            canvas->txt_lst[start_txt].baseline += yOff;
            start_txt++;
          }
      }
}

/******************************************************************************
 * Function: ProcessElement
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ProcessElement(canvas, layout, cur_seg, join_seg, min_y,
		top_bot, side, corner, flow,
		ret_left, ret_right, ret_width, ret_cnt, join_line)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*cur_seg;
    CESegment		*join_seg;
    Unit		 min_y;
    TopDims		*top_bot,
    SideDims		*side;
    CornerDims		*corner;
    FlowDims		*flow;
    Unit		*ret_left;
    Unit		*ret_right;
    Unit		*ret_width;
    int			*ret_cnt;
    int			*join_line;
#else
ProcessElement(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*cur_seg,
    CESegment		*join_seg,
    Unit		 min_y,
    TopDims		*top_bot,
    SideDims		*side,
    CornerDims		*corner,
    FlowDims		*flow,
    Unit		*ret_left,
    Unit		*ret_right,
    Unit		*ret_width,
    int			*ret_cnt,
    int			*join_line)
#endif /* _NO_PROTO */
{
    int		   getLn;
    int		   saveLn       = canvas->line_cnt;
    int		   saveTxt      = canvas->txt_cnt;
    Unit	   saveMaxX     = layout->info.max_x_pos;
    Unit	   saveY        = layout->info.y_pos;
    Unit	   saveLeft     = layout->left;
    Unit	   saveRight    = layout->right;
    Unit	   saveFirst    = layout->first;
    Unit	   pad;
    CEBoolean	   saveStatic   = layout->stat_flag;
    CEBoolean	   didHead      = False;
    CEBoolean	   junk         = False;
    enum SdlOption saveJustify  = layout->txt_justify;
    CEElement     *oldParent    = layout->parent_el;
    CEElement     *pEl          = _CEElementOfSegment(cur_seg);
    SDLFrmtInfo   *frmtInfo     = FrmtInfoPtr(pEl);
    DataPoint	   dataPt;

    /*
     * check to see if this segment is the segment we want as our first
     * visible line.
     */
    if (layout->target_id != NULL &&
		_CEIsStringAttrSet(pEl->attributes, SDL_ATTR_ID) &&
		_DtHelpCeStrCaseCmp(IdString(pEl), layout->target_id) == 0)
      {
        layout->id_Ypos  = layout->info.y_pos;
        layout->id_found = True;
      }

    if (pEl->seg_list == NULL)
	return False;

    if (frmtInfo->beg_gr == -1)
	frmtInfo->beg_gr = canvas->graphic_cnt;

    /*
     * set the parent's data point in the stack
     */
    layout->info.max_x_pos = 0;
    GetCurrentDataPoint(layout, &dataPt);
    if (layout->left > 0)
      {
	dataPt.left += layout->left;
	junk         = True;
      }
    if (layout->right > 0)
      {
	dataPt.right += layout->right;
	junk          = True;
      }

    if (junk == True)
        PushDataPoint(layout, &dataPt);

    /*
     * If the first margin attr was not specified, it gets inherited.
     */
    if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_FMARGIN))
        layout->first = CalcOffset(canvas, pEl->attributes.frmt_specs.fmargin);

    /*
     * If these values were specified, then they will get added to the
     * parent's margins. If they weren't, they default to zero.
     */
    layout->left  = CalcOffset(canvas, pEl->attributes.frmt_specs.lmargin);
    layout->right = CalcOffset(canvas, pEl->attributes.frmt_specs.rmargin);

    /*
     * remember the parent of the new segment list.
     */
    layout->parent_el = pEl;

    /*
     * check to see if this element breaks the formatting sequence.
     * If so save any information in the buffer, reset the margins and
     * add the appropriate lines, and check for going over boundaries.
     */
    if (pEl->format_break == True && (join_line == NULL || *join_line == -1))
      {
	CheckSaveInfo(canvas, layout);
	CheckFormat(layout, True);

	layout->txt_justify = pEl->attributes.frmt_specs.justify;

	if (pEl->attributes.type == SdlTypeLiteral ||
					pEl->attributes.type == SdlTypeLined)
	    layout->stat_flag = True;
	layout->lst_rendered = NULL;
      }

    /*
     * Adjust margins and y position for bordering
     */
    switch (pEl->el_type)
      {
	case SdlElementSnb:
	case SdlElementBlockAsync:
	case SdlElementFdata:
	case SdlElementFstyle:
		break;
	default:
		AdjustForBorders (canvas, layout, Border(pEl), &junk);
		break;
      }

    /*
     * determine what we are processing.
     */
    switch (pEl->el_type)
      {
	/*
	 * skip the system Notation blocks - they are just for reference.
	 */
	case SdlElementSnb:
		if (layout->snb == NULL)
		    layout->snb = pEl;
	/*
	 * skip the Async and Fdata blocks - they are referenced later.
	 */
	case SdlElementBlockAsync:
	case SdlElementFdata:
	case SdlElementFstyle:
		break;

	/*
	 * Process the snref block.
	 */
	case SdlElementSnRef:
		/*
		 * resolve the system notation reference
		 */
		_DtHelpCeResolveSdlSnref(canvas, NULL, layout->snb, cur_seg,
						layout->lst_rendered);

		/*
		 * format the resolved reference.
		 */
		ProcessSegmentList(canvas, layout, pEl->seg_list, NULL);
		break;
	/*
	 * Process heads which affect the information after them.
	 */
	case SdlElementHead:
		ProcessHead(canvas, layout, cur_seg, join_line);
		didHead = True;
		UpdateDimensionArrays(cur_seg, top_bot, side, corner,
					flow, ret_left, ret_right);
		break;

	case SdlElementForm:
		CheckSaveInfo(canvas, layout);
		_DtHelpCeAddLines(canvas, pEl->attributes.frmt_specs.tmargin,
						True, &(layout->info.y_pos));
		CheckFormat(layout, True);

		ProcessForm (canvas, layout, pEl);

		CheckSaveInfo(canvas, layout);
		_DtHelpCeAddLines(canvas, pEl->attributes.frmt_specs.bmargin,
						True, &(layout->info.y_pos));
		break;

	case SdlElementSphrase:
		if (layout->super_script == False && layout->sub_script==False)
		    layout->string_end = layout->info.cur_len;

		if (layout->info.cur_len == 0)
		  {
		    layout->info.line_seg   = cur_seg;
		    layout->info.line_start = 0;
		  }

		/*
		 * make cur_len non-zero, otherwise other routines will
		 * think the sub/super script should start the line (and
		 * reset pointer clobbering the previous unsave info).
		 */
		layout->info.cur_len = 1;

	default:
		if (pEl->format_break == True &&
					(join_line == NULL || *join_line == -1))
		  {
		    _DtHelpCeAddLines(canvas,
					pEl->attributes.frmt_specs.tmargin,
						True, &(layout->info.y_pos));
		    CheckFormat(layout, True);
		  }

		if (pEl->seg_list != NULL)
		    ProcessSegmentList(canvas, layout, pEl->seg_list, join_seg);

		if (pEl->format_break == True)
		  {
		    /*
		     * check to see if this element breaks the formatting
		     * sequence. If so save any information in the buffer,
		     * reset the margins and add the appropriate lines,
		     * and check for going over boundaries.
		     */
		    CheckSaveInfo(canvas, layout);
		    _DtHelpCeAddLines(canvas,
					pEl->attributes.frmt_specs.bmargin,
						True, &(layout->info.y_pos));
		    layout->lst_rendered = NULL;
		  }
      }

    if (pEl->el_type == SdlElementSphrase)
      {
	/*
	 * get rid of the extra length
	 */
	layout->info.cur_len--;

	/*
	 * Concatonate the super and subscripts together
	 */
	if (pEl->attributes.clan == SdlClassSuper)
	  {
	    layout->super_end    += layout->info.cur_len;
	    layout->super_script  = True;
	  }
	else
	  {
	    layout->sub_end    += layout->info.cur_len;
	    layout->sub_script  = True;
	  }

	if (layout->sub_end > layout->super_end)
	    layout->info.cur_len = layout->sub_end;
	else
	    layout->info.cur_len = layout->super_end;

	/*
	 * reset the cur_len to properly reflect the string and
	 * its sub/super scripts.
	 */
	layout->info.cur_len += layout->string_end;
      }

    /*
     * remove this element's data points from the stack.
     */
    RemoveDataPoint(layout, &dataPt);

    /*
     * Figure out the ending y position for any borders
     */
    pad = 0;
    if (junk == True)
	pad = canvas->metrics.line_thickness /*+ canvas->metrics.leading*/;

    layout->info.y_pos += pad;

    /*
     * does this object need to be adjust within its height?
     */
    if (frmtInfo->end_gr == -1)
	frmtInfo->end_gr = canvas->graphic_cnt;

    if (min_y > -1 && layout->info.y_pos < min_y)
      {
	AdjustObjectPosition(canvas, TxtVertJustify(pEl),
			saveTxt, frmtInfo->beg_gr, saveLn,
			canvas->txt_cnt, frmtInfo->end_gr, canvas->line_cnt, 0,
			min_y - layout->info.y_pos - pad, 0);
	layout->info.y_pos = min_y;
      }

    /*
     * Now draw the borders
     */
    getLn = DrawBorders (canvas, layout, pEl->el_type, Border(pEl), dataPt,
				saveY, layout->info.y_pos - pad, ret_width);

    /*
     * Restore the previous information
     */
    layout->left      = saveLeft;
    layout->right     = saveRight;
    layout->first     = saveFirst;
    layout->stat_flag = saveStatic;
    layout->parent_el = oldParent;

    if (pEl->format_break == True)
      {
	CheckFormat(layout, True);
	layout->txt_justify = saveJustify;
	layout->lst_rendered = NULL;
      }
    else if (pEl->el_type == SdlElementGraphic)
	layout->txt_justify = saveJustify;

    if (layout->info.max_x_pos < saveMaxX)
	layout->info.max_x_pos = saveMaxX;

    if (ret_cnt)
	*ret_cnt = getLn;

    return didHead;
}

/******************************************************************************
 * Function: ProcessSegmentList
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
ProcessSegmentList(canvas, layout, cur_seg, join_seg)
    CECanvasStruct	*canvas;
    LayoutInfo		*layout;
    CESegment		*cur_seg;
    CESegment		*join_seg;
#else
ProcessSegmentList(
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    CESegment		*cur_seg,
    CESegment		*join_seg)
#endif /* _NO_PROTO */
{
    int		 junk;
    int		 joinLine     = -1;
    int		 type;
    int		 bodyTxt = canvas->txt_cnt;
    Unit	 width;
    Unit	 tlen;
    Unit	 leftMargin   = 0;
    Unit	 rightMargin  = 0;
    Unit	 topHeight    = 0;
    Unit	 botHeight    = 0;
    Unit	 maxWidth     = 0;
    Unit	 saveMaxWidth = layout->max_width;
    Unit	 saveYpos     = layout->info.y_pos;
    Unit	 saveMaxX     = layout->info.max_x_pos;
    CEGraphics	*pGr;
    CESpecial	*pSpc;
    CESegment	*bodyStart = cur_seg;
    CESegment	*listStart = cur_seg;
    CELayoutInfo startInfo;
    CEBoolean    haveHeads = False;
    CEBoolean    step1 = True;
    CEBoolean    redo  = False;
    TopDims	 topBot;
    SideDims	 sideDims;
    CornerDims	 cornerDims;
    FlowDims	 flowDims;
    DataPoint	 basePt;
    DataPoint	 leftPt;
    DataPoint	 rightPt;

    InitDimArrays(&topBot, &sideDims, &cornerDims, &flowDims);
    GetCurrentDataPoint(layout, &basePt);
    leftPt  = basePt;
    rightPt = basePt;
    leftPt.y_pos  = _CEFORMAT_ALL;
    rightPt.y_pos = _CEFORMAT_ALL;

    startInfo = layout->info;
    layout->info.max_x_pos = 0;
    while (cur_seg != NULL)
      {
	/*
	 *
	 */
	type = _CEGetPrimaryType (cur_seg->seg_type);

	/*
	 * If we have head(s) and the next item starts the body of text for
	 * the head(s), do the calculations to layout the heads and new
	 * margins.
	 */
	if (haveHeads == True && step1 == True &&
		(type != CE_ELEMENT ||
		_CEElementOfSegment(cur_seg)->el_type != SdlElementHead))
	  {
	    DetermineMaxDims(&topBot, &cornerDims, leftMargin, rightMargin,
				&topHeight, &botHeight, &maxWidth);
	    layout->info.y_pos       += topHeight;
	    layout->info.max_x_pos  = 0;
	    layout->left             += leftMargin;
	    layout->right            += rightMargin;
	    DetermineFlowConstraints(layout, flowDims,
				basePt.left, basePt.right,
				layout->info.y_pos, &leftPt, &rightPt);

	    /*
	     * get rid of the leftMargin and rightMargin values in maxWidth
	     * otherwise the use of layout->left & layout->right will double
	     * the value.
	     */
	    maxWidth = maxWidth - leftMargin - rightMargin;
	    if (layout->max_width < maxWidth + basePt.left + basePt.right +
				layout->left + layout->right)
		layout->max_width = maxWidth + basePt.left + basePt.right +
						layout->left + layout->right;
	    step1     = False;
	    bodyStart = cur_seg;
	    bodyTxt   = canvas->txt_cnt;
	    
	    SetMargins(layout);
	    SetTextPosition(layout, True);
	    if (joinLine != -1)
	      {
		Unit	  tmpWidth;
		int       cnt;
		int       start = canvas->txt_lst[joinLine].byte_index;
		int       count = canvas->txt_lst[joinLine].length;
		CESegment *pSeg = canvas->txt_lst[joinLine].seg_ptr;

		canvas->txt_cnt = joinLine;
		layout->info.text_x_pos  = canvas->txt_lst[joinLine].text_x;
		layout->info.label_x_pos = canvas->txt_lst[joinLine].text_x;
		layout->info.line_seg    = pSeg;
		layout->info.line_start  = start;
		layout->info.line_bytes  = count;
		layout->info.y_pos       = canvas->txt_lst[joinLine].baseline -
					       canvas->txt_lst[joinLine].ascent;
		layout->info.cur_len = 0;
		while (pSeg != NULL && count > 0)
		  {
		    join_seg = pSeg;
		    _DtHelpCeDetermineWidthOfSegment(canvas,pSeg,start,count,
							&cnt, &tmpWidth, NULL);
		    layout->info.cur_len += tmpWidth;
		    count -= cnt;
		    start  = 0;
		    pSeg   = pSeg->next_disp;
		  }
	      }
	    startInfo = layout->info;
	  }

        width = layout->max_width - layout->info.text_x_pos
				- layout->r_margin - layout->info.cur_len;
	/*
	 * Process based on the primary type
	 */
	switch (type)
          {
	    case CE_ELEMENT:
		    if (ProcessElement(canvas, layout, cur_seg, join_seg, -1,
				&topBot, &sideDims, &cornerDims, &flowDims,
				&leftMargin, &rightMargin, NULL, &junk,
				&joinLine) == True)
		      {
		        haveHeads = True;
		      }
		    break;

	    case CE_STRING:
		    if (join_seg != NULL)
		      {
			join_seg->next_disp = cur_seg;
			join_seg = NULL;
		      }
		    ProcessStringSegment(canvas, layout, cur_seg);
		    if (_CEIsNewLine(cur_seg->seg_type))
			CheckFormat(layout, False);
		    layout->lst_rendered = cur_seg;
		    break;

	    case CE_GRAPHIC:
		    pGr  = _CEGraphicStructPtr(canvas,
						_CEGraphicIdxOfSeg(cur_seg));
		    if (layout->parent_el->attributes.clan == SdlClassInLine
			|| layout->parent_el->attributes.clan == SdlClassButton)
		      {
			/*
			 * if this is the start of the line, reset the
			 * pointers.
			 */
		        if (layout->info.cur_len == 0)
		          {
			    layout->info.line_seg   = cur_seg;
			    layout->info.line_start = 0;
		          }

		        if (join_seg != NULL)
		          {
			    join_seg->next_disp = cur_seg;
			    join_seg = NULL;
		          }
			tlen = _DtHelpCeGetTraversalWidth(canvas, cur_seg,
							layout->info.lst_hyper);

			if (pGr->width  + tlen > width)
			  {
			    CheckSaveInfo (canvas, layout);
			    layout->info.line_seg   = cur_seg;
			    layout->info.line_start = 0;
			    tlen = _DtHelpCeGetTraversalWidth(canvas, cur_seg,
							layout->info.lst_hyper);
			  }

			CheckAddToHyperList(canvas, cur_seg);
			_DtHelpCeAddToInLineList(&(layout->info),
					_CEGraphicIdxOfSeg(cur_seg));
			pGr->pos_x = layout->info.text_x_pos
							+ layout->info.cur_len;
			pGr->pos_y = layout->info.y_pos;

			layout->info.cur_len += (pGr->width + tlen);
		      }
		    else
		      {
			enum SdlOption option =
			       layout->parent_el->attributes.frmt_specs.justify;

		        if (join_seg != NULL)
			    join_seg = NULL;

			/*
			 * figures are standalone. Save any
			 * information in the buffer away.
			 */
			CheckSaveInfo (canvas, layout);

			/*
			 * check to see if this segment is
			 * a hypertext. If so, add it to the
			 * list if it hasn't been added yet.
			 */
			CheckAddToHyperList(canvas, cur_seg);

			/*
			 * Must adjust y position for hypertext
			 */
			tlen = 0;
			if (_CEIsHyperLink(cur_seg->seg_type))
			    tlen += canvas->traversal_info.space_above;
			if (_CEIsVisibleLink(cur_seg->seg_type))
			    tlen += canvas->link_info.space_above;

			/*
			 * Calculate the upper left corner position
			 */
			pGr->pos_x = layout->info.text_x_pos;
			pGr->pos_y = layout->info.y_pos + tlen;

			/*
			 * Must adjust ending position for hypertext
			 */
			tlen = 0;
			if (_CEIsHyperLink(cur_seg->seg_type))
			    tlen += canvas->traversal_info.space_below;
			if (_CEIsVisibleLink(cur_seg->seg_type))
			    tlen += canvas->link_info.space_below;

			layout->info.y_pos = pGr->pos_y + pGr->height + tlen;

			/*
			 * justify the x position in the window.
			 */
			if (SdlJustifyLeft != option)
			  {
			    tlen = layout->max_width - layout->r_margin
					- layout->l_margin - pGr->width;

			    if (SdlJustifyCenter == option)
			        tlen = tlen / 2;

			    if (tlen > 0)
			        pGr->pos_x += tlen;
			  }

			/*
			 * test for run over.
			 */
			tlen = pGr->pos_x + pGr->width;
			if (layout->info.max_x_pos < tlen)
			    layout->info.max_x_pos = tlen;

			/*
			 * clear the last visible flag since there will
			 * not be any text after a figure.
			 */
			layout->info.lst_vis   = False;
			layout->info.lst_hyper = -1;

			CheckFormat(layout, False);
		      }
		    layout->lst_rendered = cur_seg;
		    break;

	    case CE_SPECIAL:
		    if (join_seg != NULL)
		      {
			join_seg->next_disp = cur_seg;
			join_seg = NULL;
		      }
		    if (_CEIsNotInitialized(cur_seg->seg_type))
		      {
			if (_DtHelpCeResolveSpc(canvas,
				layout->parent_el->attributes.id_vals.name,
				layout->parent_el->attributes.language,
				layout->parent_el->attributes.charset,
				&(layout->parent_el->attributes.font_specs),
				&(cur_seg->seg_handle.spc_handle)) == 0)
			    cur_seg->seg_type =
					_CESetValidSpc(cur_seg->seg_type);

			cur_seg->seg_type =
					_CESetInitialized(cur_seg->seg_type);
		      }

		    if (_CEIsValidSpc(cur_seg->seg_type))
		      {
			CEBoolean flag;
			Unit  nWidth = 0;
		        pSpc = cur_seg->seg_handle.spc_handle;

			/*
			 * check to see if this segment is
			 * a hypertext. If so, add it to the
			 * list if it hasn't been added yet.
			 */
			CheckAddToHyperList(canvas, cur_seg);

			/*
			 * get the traversal width around the item
			 * if it's a standalone hypertext link.
			 */
			nWidth = _DtHelpCeGetTraversalWidth(canvas, cur_seg,
							layout->info.lst_hyper);
			/*
			 * check to see if it can end a line.
			 */
			flag = _DtHelpCeCheckLineSyntax(canvas, cur_seg, 0, 0);
			if (flag == False)
			  {
			    Unit tlen =_DtHelpCeGetNextWidth(canvas,
						CE_STRING,
						layout->info.lst_hyper,
						cur_seg->next_disp,
						0,
				layout->parent_el->attributes.language,
				layout->parent_el->attributes.charset,
				&(layout->parent_el->attributes.font_specs),
						cur_seg,
						layout->toss,
						layout->snb,
						NULL, NULL, NULL);
			    nWidth += tlen;
			    if (tlen <= 0)
				flag = True;
			  }

		        if (layout->info.join == False &&
						pSpc->width + nWidth > width)
			    CheckSaveInfo (canvas, layout);

		        if (layout->info.line_bytes == 0)
		          {
		            layout->info.line_seg   = cur_seg;
		            layout->info.line_start = 0;
		          }

		        layout->info.line_bytes += 1;
		        layout->info.cur_len    += pSpc->width;
			layout->info.join        = flag ? 0 : 1;
		      }
		    layout->lst_rendered = cur_seg;
		    break;

	    default:
		    if (_CEIsNewLine(cur_seg->seg_type))
			SaveInfo(canvas, layout);
		    break;
	  }

	cur_seg           = cur_seg->next_seg;
	layout->cur_start = 0;

	if (leftPt.y_pos > 0 &&
		leftPt.x_units > layout->max_width - leftPt.left - leftPt.right)
	  {
	    layout->max_width = leftPt.x_units + leftPt.left + leftPt.right;
	    redo = True;
	  }
	if (rightPt.y_pos > 0 &&
		rightPt.x_units > layout->max_width-rightPt.left-rightPt.right)
	  {
	    layout->max_width = rightPt.x_units + rightPt.left + rightPt.right;
	    redo = True;
	  }

	if (redo == True)
	  {
	    redo    = False;
	    cur_seg = bodyStart;
	    canvas->txt_cnt        = bodyTxt;
	    layout->info           = startInfo;
	    layout->info.max_x_pos = 0;

	    if (rightPt.y_pos > 0)
	      {
	        RemoveDataPoint(layout, &rightPt); /* make sure its gone */
	        InsertDataPoint(layout, &rightPt);
	      }
	    if (leftPt.y_pos > 0)
	      {
	        RemoveDataPoint(layout, &leftPt); /* make sure its gone */
	        InsertDataPoint(layout, &leftPt);
	      }
	  }
      }

    RemoveDataPoint(layout, &leftPt);
    RemoveDataPoint(layout, &rightPt);

    /*
     * do this incase the body of text for the head is empty
     * (like for the TOC).
     */
    if (haveHeads == True && step1 == True)
      {
	DetermineMaxDims(&topBot, &cornerDims, leftMargin, rightMargin,
				&topHeight, &botHeight, &maxWidth);
	layout->info.y_pos       += topHeight;
	layout->info.max_x_pos  = 0;
	layout->left             += leftMargin;
	layout->right            += rightMargin;
	DetermineFlowConstraints(layout, flowDims, leftMargin + basePt.left,
				rightMargin + basePt.right,
				layout->info.y_pos,
				&leftPt, &rightPt);

	/*
	 * get rid of the leftMargin and rightMargin values in maxWidth
	 * otherwise the use of layout->left & layout->right will double
	 * the value.
	 */
	maxWidth = maxWidth - leftMargin - rightMargin;
	if (layout->max_width < maxWidth + basePt.left + basePt.right +
				layout->left + layout->right)
		layout->max_width = maxWidth + basePt.left + basePt.right +
						layout->left + layout->right;
	step1     = False;
	bodyStart = cur_seg;
	bodyTxt   = canvas->txt_cnt;
      }

    /*
     * if there were heads, step1 should be false
     */
    if (step1 == False)
      {
	Unit	blockHeight;

	/*
	 * make sure all of the information in the body is saved out
	 */
	CheckSaveInfo (canvas, layout);
	blockHeight = layout->info.y_pos - saveYpos - topHeight;

	/*
	 * now figure the head positions.
	 */
	DetermineHeadPositioning(&topBot, &sideDims, &cornerDims, &flowDims,
				saveYpos, topHeight,
				blockHeight, &blockHeight);
	cur_seg = listStart;

	/*
	 * if the maximum available space was exceeded by the text
	 * calculate a new max width
	 */
	if (layout->max_width < layout->info.max_x_pos + layout->right)
	    layout->max_width = layout->info.max_x_pos + layout->right;

	maxWidth = layout->max_width - basePt.left - basePt.right
						- layout->left - layout->right;
	while (cur_seg != NULL && _CEIsElement(cur_seg->seg_type) &&
		_CEElementOfSegment(cur_seg)->el_type == SdlElementHead)
	  {
	    AdjustHeadPosition(canvas, cur_seg,
			&topBot, &sideDims, &cornerDims, &flowDims,
			saveYpos, 0, maxWidth,
			leftMargin, rightMargin);
	    cur_seg = cur_seg->next_seg;
	  }

        if (saveMaxX < layout->max_width)
	    saveMaxX = layout->max_width;

	if (layout->info.y_pos < saveYpos + topHeight + blockHeight)
	    layout->info.y_pos = saveYpos + topHeight + blockHeight;
	layout->info.y_pos += botHeight;
      }

    layout->max_width      = saveMaxWidth;

    if (saveMaxX < layout->info.max_x_pos)
	saveMaxX = layout->info.max_x_pos;

    layout->info.max_x_pos = saveMaxX;

    return;
}

/*****************************************************************************
 * Function:    static void SdlCanvasLayout (CanvasHandle canvas);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 * 
 *****************************************************************************/
static	const LayoutInfo DefInfo = {
	0,		/* Unit          max_width;    */
	0,		/* Unit		 f_margin;     */
	0,		/* Unit		 l_margin;     */
	0,		/* Unit		 r_margin;     */
	0,		/* Unit		 id_Ypos;      */
	0,		/* Unit		 left;         */
	0,		/* Unit		 right;        */
	0,		/* Unit		 first;        */
	0,		/* Unit		 string_end;   */
	0,		/* Unit		 sub_end;      */
	0,		/* Unit		 super_end;    */
	False,		/* CEBoolean	 id_found;     */
	False,		/* CEBoolean	 super_script; */
	False,		/* CEBoolean	 sub_script;   */
	False,		/* CEBoolean	 stat_flag;    */
	SdlJustifyLeft,	/* enum SdlOption txt_justify; */
	0,		/* unsigned int	 cur_start;    */
	NULL,		/* CESegment	*toss;         */
	NULL,		/* CESegment	*lst_rendered; */
	NULL,		/* CEElement	*snb;          */
	NULL,		/* CEElement	*parent_el;    */
	NULL,		/* char		*target_id;    */
        NULL,		/* DataPoint	*data_pts;     */
};

static void
#ifdef _NO_PROTO
SdlCanvasLayout (canvas_handle, toss, layout, target_id)
    CanvasHandle	 canvas_handle;
    CESegment		*toss;
    LayoutInfo		*layout;
    char		*target_id;
#else
SdlCanvasLayout (
    CanvasHandle	 canvas_handle,
    CESegment		*toss,
    LayoutInfo		*layout,
    char		*target_id)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    DataPoint		 basePt = { 0, 0, -1, 0, NULL};

    *layout           = DefInfo;
    layout->max_width = canvas->metrics.width;
    layout->left      = canvas->metrics.side_margin;
    layout->right     = canvas->metrics.side_margin;
    layout->target_id = target_id;
    layout->toss      = toss;
    layout->snb       = _DtHelpCeFindSdlElement(canvas->element_lst,
							SdlElementSnb, True);
    _DtHelpCeInitLayoutInfo(canvas, &(layout->info));

    PushDataPoint(layout, &basePt);
    SetMargins (layout);
    SetTextPosition (layout, True);

    ProcessSegmentList(canvas, layout, canvas->element_lst, NULL);

    if (layout->info.gr_list != NULL)
        free(layout->info.gr_list);

    RemoveDataPoint(layout, &basePt);

}  /* End SdlCanvasLayout */

/*****************************************************************************
 *              Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:    void _DtHelpCeResizeSDLCanvas (CanvasHandle canvas);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 * 
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeResizeSDLCanvas (canvas_handle, *ret_width, *ret_height)
    CanvasHandle	canvas_handle;
    Unit		*ret_width;
    Unit		*ret_height;
#else
_DtHelpCeResizeSDLCanvas (
    CanvasHandle	canvas_handle,
    Unit		*ret_width,
    Unit		*ret_height )
#endif /* _NO_PROTO */
{
    int			 i;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CanvasMetrics	 oldMetrics = canvas->metrics;
    LayoutInfo		layOut;

    /*
     * Re-Layout the information.
     * First step - invalidate some counters.
     */
    canvas->hyper_count = 0;	/* zero this only because we re-process */
				/* do not zero cur_hyper or we'll loose */
				/* where we are in the TOC              */
    canvas->txt_cnt     = 0;
    canvas->line_cnt    = 0;

    for (i = 0; i < canvas->graphic_cnt; i++)
      {
	canvas->graphic_lst[i].pos_y = -1;
	canvas->graphic_lst[i].pos_x = -1;
      }

    /*
     * Layout the information if there is anything to do
     */
    SdlCanvasLayout (canvas, NULL, &layOut, NULL);
    
    /*
     * return the maximum height and width used
     */
    if (ret_width != NULL)
        *ret_width  = layOut.info.max_x_pos;
    if (ret_height != NULL)
    *ret_height = layOut.info.y_pos;

    return True;

}  /* End _DtHelpCeResizeSDLCanvas */

/*****************************************************************************
 * Function:    void _DtHelpCeSetSdlTopic (CanvasHandle canvas);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 * 
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeSetSdlTopic (canvas_handle, topic_handle, scroll_y,
			*ret_width, *ret_height, *ret_y)
    CanvasHandle	 canvas_handle;
    TopicHandle		 topic_handle;
    int			 scroll_y;
    Unit		*ret_width;
    Unit		*ret_height;
    Unit		*ret_y;
#else
_DtHelpCeSetSdlTopic (
    CanvasHandle	canvas_handle,
    TopicHandle		topic_handle,
    int			 scroll_y,
    Unit		*ret_width,
    Unit		*ret_height,
    Unit		*ret_y )
#endif /* _NO_PROTO */
{
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CETopicStruct	*topic  = (CETopicStruct *) topic_handle;
    LayoutInfo		layOut;

    /*
     * attach to the canvas
     */
    canvas->name        = topic->name;
    canvas->element_lst = topic->element_list;
    canvas->para_lst    = topic->para_list;

    /*
     * up our paragraph count to the correct value
     */
    canvas->para_count += topic->para_num;

    /*
     * Attach the link information
     */
    canvas->link_data = topic->link_data;

    /*
     * Layout the information if there is anything to do
     */
    SdlCanvasLayout (canvas, topic->toss, &layOut, topic->id_str);
    
    /*
     * destroy the topic handle
     */
    if (topic->id_str != NULL)
	free(topic->id_str);
    free ((char *) topic);

    /*
     * return the maximum height and width used
     * And the location of the id.
     */
    if (ret_width != NULL)
        *ret_width = layOut.info.max_x_pos;
    if (ret_height != NULL)
        *ret_height = layOut.info.y_pos;

    if (scroll_y != -1)
	layOut.id_Ypos = scroll_y * layOut.info.y_pos / 1000;
    if (ret_y != NULL)
        *ret_y = layOut.id_Ypos;

}  /* End _DtHelpCeSetSdlTopic */
