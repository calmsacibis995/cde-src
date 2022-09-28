/* $XConsortium: Canvas.c /main/cde1_maint/5 1995/08/26 22:40:41 barstow $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Canvas.c
 **
 **   Project:     Cde Help System
 **
 **   Description: UI independent layer for the help system.  These
 **		   routines manage the information within a 'canvas'.
 **		   The 'canvas' routines call UI dependent code to
 **		   render the information.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <stdlib.h>
#include <string.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "LinkMgrI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "CleanUpI.h"
#include "SelectionI.h"
#include "LayoutUtilI.h"
#include "VirtFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*****************************************************************************
 *		Private Variables
 *****************************************************************************/
static	CECanvasStruct	DefaultCanvas =
  {
	0,		/* int error;                  */
	False,		/* CEBoolean toc_flag;         */
	False,		/* CEBoolean toc_on;           */
	-1,		/* int toc_loc;                */
	0,		/* int para_count;             */
	0,		/* int txt_cnt;                */
	0,		/* int txt_max;                */
	0,		/* int max_x_cnt;              */
	0,		/* int line_cnt;               */
	0,		/* int line_max;               */
	0,		/* int graphic_count;          */
	0,		/* int graphic_max;            */
	0,		/* int hyper_count;            */
	0,		/* int hyper_max;              */
	0,		/* int cur_hyper;              */
	1,		/* short mb_length;            */
	1,		/* short nl_to_space;          */
	NULL,		/* wchar_t *multi_cant_end;    */
	NULL,		/* wchar_t *multi_cant_begin;  */
	NULL,		/* char         *name;         */
	NULL,		/* VolumeHandle  volume;       */
	NULL,		/* ClientData    client_data;  */
	  {		/* CanvasMetrics metrics;      */
	    0,			/* Unit width;         */
	    0,			/* Unit height;        */
	    0,			/* Unit top_margin;    */
	    0,			/* Unit leading;       */
	    0,			/* Unit line_thickness;*/
	  },
	  {		/* CanvasFontMetrics font_info; */
	    0,			/* Unit ascent;   */
	    0,			/* Unit descent;  */
	    0,			/* average_width; */
	  },
	  {		/* CanvasLinkMetrics link_info; */
	    0,			/* Unit space_before; */
	    0,			/* Unit space_after;  */
	    0,			/* Unit space_above;  */
	    0,			/* Unit space_below;  */
	  },
	  {		/* CanvasTraversalMetrics traversal_info; */
	    0,			/* Unit space_before; */
	    0,			/* Unit space_after;  */
	    0,			/* Unit space_above;  */
	    0,			/* Unit space_below;  */
	  },
	NULL,		/* CESegment      *element_lst;   */
	NULL,		/* CEParagraph    *para_lst;      */
	NULL,		/* CELines        *txt_lst;       */
	NULL,		/* CELineSegment  *line_lst;       */
	NULL,		/* Unit           *max_x_lst;      */
	NULL,		/* CEGraphics     *graphic_lst;    */
	NULL,		/* CELinkSegments *hyper_segments; */
	  {		/* CELinkData      link_data; */
	    0,			/* int max;           */
	    NULL,		/* CELinkEntry *list; */
	  },
	  {		/* CESelection select_start; */
		-1,		/* Unit x;           */
		-1, 		/* Unit y;           */
		-1,		/* int  line_idx;    */
		-1,		/* int	char_idx;    */
	  },
	  {		/* CESelection select_end; */
		-1,		/* Unit x;           */
		-1, 		/* Unit y;           */
		-1,		/* int  line_idx;    */
		-1,		/* int	char_idx;    */
	  },
	NULL,		/* VirtualInfo virt_functions; */
  };

/*****************************************************************************
 *		Private Functions
 *****************************************************************************/
/*****************************************************************************
 * Function: FindChar
 *
 *    FindChar calculates the char that is x pixels into the string.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
FindChar (canvas, segment, string, max_len, x_pos, diff)
    CECanvasStruct	*canvas;
    CESegment		*segment;
    char		*string;
    int			 max_len;
    Unit		 x_pos;
    Unit		*diff;
#else
FindChar (
    CECanvasStruct	*canvas,
    CESegment		*segment,
    char		*string,
    int			 max_len,
    Unit		 x_pos,
    Unit		*diff)
#endif /* _NO_PROTO */
{
    int     myIndex;
    int     charSize;
    Unit    myDiff = 0;
    Unit    len;
    Unit    charWidth;
    CEBoolean triedBack    = False;
    CEBoolean triedForward = False;

    /*
     * find the size of the string
     */
    charSize = _CEGetCharLen(segment->seg_type);
    max_len  = max_len / charSize;

    /*
     * get information about the font used
     */
    _DtHelpCeGetFontMetrics(canvas, _CEFontOfSegment(segment),
					NULL, NULL, &charWidth, NULL, NULL);

    /*
     * try to get close to the correct index.
     */
    myIndex = x_pos / charWidth;
    if (myIndex >= max_len)
	myIndex = max_len - 1;
    
    while (!triedBack || !triedForward)
      {
	len = _DtHelpCeGetStringWidth(canvas, segment, NULL, NULL, NULL,
					string, ((myIndex + 1) * charSize));

	if (len > x_pos)
	  {
	    myDiff       = len - x_pos;
	    triedForward = True;
	    if (!triedBack && myIndex)
	 	myIndex--;
	    else
		triedBack = True;
	  }
	else if (len < x_pos)
	  {
	    myDiff    = x_pos - len;
	    triedBack = True;
	    myIndex++;
	    if (myIndex >= max_len)
	      {
		myIndex--;
		triedForward = True;
	      }
	  }
	else /* len == x_pos */
	  {
	    myIndex++;
	    triedBack    = True;
	    triedForward = True;
	    myDiff       = 0;
	  }
      }

    if (diff != NULL)
	*diff = myDiff;

    return (myIndex * charSize);
}

/*****************************************************************************
 * Function: DrawCanvasLines
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DrawCanvasLines(canvas, x1, y1, x2, y2)
    CECanvasStruct	*canvas;
    Unit		 x1;
    Unit		 y1;
    Unit		 x2;
    Unit		 y2;
#else
DrawCanvasLines(
    CECanvasStruct	*canvas,
    Unit		 x1,
    Unit		 y1,
    Unit		 x2,
    Unit		 y2)
#endif /* _NO_PROTO */
{
    int    i;
    Unit   endX;
    Unit   endY;
    CELineSegment *pLS;

    if (canvas->line_lst != NULL && canvas->line_cnt
				&& canvas->virt_functions._CEDrawLine != NULL)
      {
        for (i = 0; i < canvas->line_cnt; i++)
          {
            pLS = &canvas->line_lst[i];

	    endX = pLS->pos_x;
	    endY = pLS->pos_y;
	    if (pLS->dir == CELineHorizontal)
		endX += pLS->length;
	    else
		endY += pLS->length;

	    if (endY >= y1 && pLS->pos_y <= y2
					&& endX >= x1 && pLS->pos_x <= x2)
		(*(canvas->virt_functions._CEDrawLine))(
			canvas->client_data, 
			pLS->pos_x, pLS->pos_y, endX, endY);
          }
      }
} /* End DrawCanvasLines */

/*****************************************************************************
 *		Semi-Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function: _DtHelpCeGetCharIdx
 *
 * Parameters:
 *		canvas		Specifies the canvas on which to render
 *				the text.
 *		line		Specifies the line in the line table.
 *		find_x		Specifies the x position of the character.
 *
 * Returns:	??		Returns the idx of the character.
 *
 * Purpose:
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCharIdx(canvas, line, find_x)
    CECanvasStruct	*canvas;
    CELines		 line;
    Unit		 find_x;
#else
_DtHelpCeGetCharIdx(
    CECanvasStruct	*canvas,
    CELines		 line,
    Unit		 find_x)
#endif /* _NO_PROTO */
{
    char	*pChar;
    CEBoolean	 lastWasLabel = FALSE;
    CEBoolean	 done         = FALSE;
    CEBoolean	 lastLinkVisible = FALSE;
    int          count        = line.length;
    int          start        = line.byte_index;
    int          len    = -1;
    int		 lnkInd = -1;
    Unit	 segWidth;
    Unit	 xPos;
    CESegment   *pSeg;

    xPos = _DtHelpCeGetStartXOfLine(canvas, line, &pSeg, &lastWasLabel);

    /*
     * check to see if the start is in the middle of the line.
     * If so, bump the x position and start indexes to the
     * correct locations.
     */
    while (!done && find_x > xPos && count > 0)
      {
	xPos = _DtHelpCeAdvanceXOfLine(canvas, line, pSeg, xPos,
				&lnkInd, &lastWasLabel, &lastLinkVisible);

	if (xPos < find_x)
	  {
            /*
             * advance the pointer by the width
             */
            _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, count,
							&len, &segWidth, NULL);
	    if (segWidth + xPos <= find_x)
	      {
	        xPos  += segWidth;
	        pSeg   = pSeg->next_disp;
	        count -= len;
	        start  = 0;
	      }
            else  /* if (xPos < find_x && find_x < xPos + segWidth) */
              {
	        if (_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
						|| _CEIsCaption(pSeg->seg_type))
	          {
                    pChar = _CEStringOfSegment(pSeg) + start;
                    len   = strlen (pChar);

	            if (len > count)
		        len = count;

	            count -= FindChar(canvas, pSeg, pChar, len,
							find_x - xPos, NULL);
                  }
                done = True;
              }
          }

	len = line.length - count;
      }

    return len;
}

/*****************************************************************************
 * Function: _DtHelpCeGetStartXOfLine
 *
 * Purpose: Get the starting 'x' of the specified line
 *          Does *not* take into account traversal or link info.
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeGetStartXOfLine (canvas, line, p_seg, labelFlag)
    CECanvasStruct	 *canvas;
    CELines		  line;
    CESegment		**p_seg;
    CEBoolean		 *labelFlag;
#else
_DtHelpCeGetStartXOfLine (
    CECanvasStruct	 *canvas,
    CELines		  line,
    CESegment		**p_seg,
    CEBoolean		 *labelFlag)
#endif /* _NO_PROTO */
{
    Unit  xPos = line.text_x;

    *labelFlag = False;
    *p_seg     = line.seg_ptr;
    if (_CEIsLabel((line.seg_ptr)->seg_type))
      {
	xPos       = line.label_x;
	*labelFlag = True;
      }

    return xPos;
}

/*****************************************************************************
 * Function: _DtHelpCeAdvanceXOfLine
 *
 * Purpose: Move the 'x' to after the traversal and link info.
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeGetAdvanceXOfLine (canvas, line, p_seg, x_pos,
			link_idx, label_flag, link_flag)
    CECanvasStruct	*canvas;
    CELines		 line;
    CESegment		*p_seg;
    Unit		 x_pos;
    int			*link_idx;
    CEBoolean		*label_flag;
    CEBoolean		*link_flag;
#else
_DtHelpCeAdvanceXOfLine (
    CECanvasStruct	*canvas,
    CELines		 line,
    CESegment		*p_seg,
    Unit		 x_pos,
    int			*link_idx,
    CEBoolean		*label_flag,
    CEBoolean		*link_flag)
#endif /* _NO_PROTO */
{
    CEBoolean	junk;

    if (*label_flag && _CEIsNotLabel(p_seg->seg_type))
      {
        x_pos       = line.text_x;
        *label_flag = False;
      }

    /*
     * take into account the link before and after space
     */
    junk = _CEIsVisibleLink(p_seg->seg_type);
    *link_flag = _DtHelpCeModifyXpos (canvas->link_info, p_seg, junk,
                        *link_flag, *link_idx, &x_pos);

    /*
     * take into account the traversal before and after space
     */
    junk = _CEIsHyperLink(p_seg->seg_type);
    (void) _DtHelpCeModifyXpos (canvas->traversal_info, p_seg, junk,
                        ((CEBoolean) True), *link_idx, &x_pos);

    *link_idx = p_seg->link_index;

    return x_pos;
}

/******************************************************************************
 * Function: _DtHelpCeDetermineWidthOfSegment
 *
 *  DetermineWidthOfSegment determines the width of the segment.
 *  The segment must have been already initialized with the correct
 *  font (for strings), the spc resolve, the graphic loaded, etc.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDetermineWidthOfSegment(canvas, p_seg, start, max_cnt,
				ret_cnt, ret_w, ret_trimmed)
    CECanvasStruct	*canvas;
    CESegment		*p_seg;
    int			 start;
    int			 max_cnt;
    int			*ret_cnt;
    Unit		*ret_w;
    CEBoolean		*ret_trimmed;
#else
_DtHelpCeDetermineWidthOfSegment(
    CECanvasStruct	*canvas,
    CESegment		*p_seg,
    int			 start,
    int			 max_cnt,
    int			*ret_cnt,
    Unit		*ret_w,
    CEBoolean		*ret_trimmed)
#endif /* _NO_PROTO */
{
    char	*pChar;

    /*
     * return the width of the segment.
     */
    *ret_cnt = 0;
    if (ret_w != NULL)
        *ret_w = 0;

    if (ret_trimmed != NULL)
        *ret_trimmed = False;

    if (_CEIsNotNoop(p_seg->seg_type))
      {
        if (_CEIsSpecial(p_seg->seg_type))
          {
	    *ret_cnt = 1;
	    if (ret_w != NULL)
                *ret_w = _CESpcOfSegment(p_seg)->width;
          }
        else if (_CEIsGraphic(p_seg->seg_type))
	  {
	    if (ret_w != NULL)
                *ret_w = _CEGraphicWidth(canvas, _CEGraphicIdxOfSeg(p_seg));
	  }
	else 
	  {
            pChar    = _CEStringOfSegment(p_seg) + start;
            *ret_cnt = strlen (pChar);
            if (*ret_cnt > max_cnt)
	      {
	        *ret_cnt = max_cnt;
		if (ret_trimmed != NULL)
		    *ret_trimmed = True;
	      }

	    /*
	     * can give nulls for lang, charset/codeset, and font_specs
	     * since this routine *must not* be called until after the
	     * font for the string has been resolved.
	     */
	    if (ret_w != NULL)
                *ret_w = _DtHelpCeGetStringWidth(canvas, p_seg,
					NULL, NULL, NULL,
					pChar, *ret_cnt);
          }
      }
}

/******************************************************************************
 * Function: _DtHelpCeModifyXpos
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeModifyXpos (info, seg, tst_result, cur_flag, last_idx, x)
    CanvasLinkMetrics	info;
    CESegment	 *seg;
    CEBoolean	  tst_result;
    CEBoolean	  cur_flag;
    int		  last_idx;
    Unit	 *x;
#else
_DtHelpCeModifyXpos (
    CanvasLinkMetrics	info,
    CESegment	 *seg,
    CEBoolean	  tst_result,
    CEBoolean	  cur_flag,
    int		  last_idx,
    Unit	 *x)
#endif /* _NO_PROTO */
{
    int    addx = 0;

    /*
     * take into account the link before and after space
     */
    if (tst_result)
      {
	/*
	 * Ignore if the same link
	 */
	if (last_idx != seg->link_index)
	  {
	    /*
	     * if one link followed by another add the space after.
	     */
	    if (last_idx != -1)
		addx = info.space_after;

	    /*
	     * add the space before the link
	     */
	    addx += info.space_before;
	  }
	cur_flag = True;
      }
    else
      {
	if (last_idx != -1 && cur_flag == True)
	    addx = info.space_after;
	cur_flag = False;
      }

    *x += addx;
    return cur_flag;
}

/******************************************************************************
 * Function: _DtHelpCeFreeGraphics
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeFreeGraphics (canvas_handle)
    CECanvasStruct	 *canvas;
#else
_DtHelpCeFreeGraphics (
    CECanvasStruct	 *canvas)
#endif /* _NO_PROTO */
{
    int		 i;
    CEGraphics	*pGraph;

    if (canvas != NULL && canvas->graphic_cnt > 0)
      {
	for (i = 0; i < canvas->graphic_cnt; i++)
	  {
	    if (_CEIsInitialized(canvas->graphic_lst[i].type) &&
			  _CEIsNotDuplicate(canvas->graphic_lst[i].type))
	      {
		pGraph = _CEGraphicStructPtr(canvas, i);
		_DtHelpCeDestroyGraphic(canvas, pGraph->handle);
	      }
	  }
	canvas->graphic_cnt = 0;
      }
}

/*****************************************************************************
 * Function: 	int _DtHelpCeGetLineAtY (Unit y)
 *
 * Parameters:
 *
 * Returns:	the line index of the first line occupying 'y'.
 *
 * Purpose:
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetLineAtY (canvas, lines, txt_cnt, y)
    CECanvasStruct	*canvas;
    CELines *lines;
    int      txt_cnt;
    Unit     y;
#else
_DtHelpCeGetLineAtY (
    CECanvasStruct	*canvas,
    CELines *lines,
    int      txt_cnt,
    Unit      y)
#endif /* _NO_PROTO */
{
    int    i = 0;

    while (i < txt_cnt && lines[i].baseline + lines[i].descent < y)
        i++;

    return i;

} /* End _DtHelpCeGetLineAtY */

/*****************************************************************************
 * Function: _DtHelpCeAdjustForSuperSub
 *
 * Parameters:
 *		canvas		Specifies the canvas.
 *		start_x		Specifies the current text x position.
 *              script_x        Specifies the current super and sub
 *				scripting x position.  Returns the same
 *				value as start_x if the segment is not a
 *				super or sub script.
 *		super_width     Specifies the width of the previously
 *				rendered super script.  Set to 0 if the
 *				next segment is not a super or sub
 *				script.
 *		super_y         Specifies the y offset for super
 *				scripts.  Set to a new value if the last
 *				segment was not a super or sub script.
 *		sub_width       Specifies the width of the previously
 *				rendered sub script.  Set to 0 if the
 *				next segment is not a super or sub
 *				script.
 *		sub_y           Specifies the y offset for sub scripts.
 *				Set to a new value if the last segment
 *				was not a super or sub script.
 *		last_was_super	Specifies if the last item was a super
 *				script. Set to False if the segment
 *				is not a super or sub script.
 *		last_was_sub	Specifies if the last item was a sub
 *				script. Set to False if the segment
 *				is not a super or sub script.
 * Returns: new text x positon.
 *
 * Purpose: Determines the super and sub scripting positions for text.
 *	    If the last item was not a script, then the base offset for
 *	    scripting (script_x) is moved to start_x. If the current
 *	    item is a string, its scripting y position is determined
 *	    (super_y and sub_y).  If the new item is a super or sub
 *	    script, the next text placement (start_x) is moved to after
 *	    the script_x plus the super or sub script size currently
 *	    active(super_width and sub_width).  Otherwise, the the flags
 *	    are set to false and the widths are set to 0.
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeAdjustForSuperSub(canvas, pSeg, start_x, script_x,
			super_width, super_y, sub_width, sub_y,
			last_was_super, last_was_sub)
    CECanvasStruct	*canvas;
    CESegment		*pSeg;
    Unit		 start_x;
    Unit		*script_x;
    Unit		*super_width;
    Unit		*super_y;
    Unit		*sub_width;
    Unit		*sub_y;
    CEBoolean		*last_was_super;
    CEBoolean		*last_was_sub;
#else
_DtHelpCeAdjustForSuperSub(
    CECanvasStruct	*canvas,
    CESegment		*pSeg,
    Unit		 start_x,
    Unit		*script_x,
    Unit		*super_width,
    Unit		*super_y,
    Unit		*sub_width,
    Unit		*sub_y,
    CEBoolean		*last_was_super,
    CEBoolean		*last_was_sub)
#endif /* _NO_PROTO */
{
    /*
     * if the last item was not a super or sub script,
     * move the script x to the end of the last output.
     */
    if (!(*last_was_super || *last_was_sub))
        *script_x = start_x;

    /*
     * check for super and sub scripts.
     * adjust text x positioning accordingly.
     */
    if (_CEIsSuperScript(pSeg->seg_type))
      {
        start_x         = *script_x + *super_width;
        *last_was_super = True;
      }
    else if (_CEIsSubScript(pSeg->seg_type))
      {
        start_x       = *script_x + *sub_width;
        *last_was_sub = True;
      }
    else if (*last_was_super || *last_was_sub)
      {
        *sub_width      = 0;
        *super_width    = 0;
        *last_was_super = False;
        *last_was_sub   = False;
      }

    /*
     * if this wasn't a super or sub script, find out where
     * they get placed on this string.
     */
    if (!(*last_was_super || *last_was_sub) &&
	(_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
			|| _CEIsCaption(pSeg->seg_type)))
	    _DtHelpCeGetFontMetrics (canvas, _CEFontOfSegment(pSeg),
				NULL, NULL, NULL, super_y, sub_y);

    return start_x;
}

/******************************************************************************
 * Function: _DtHelpCeDrawSegments
 *
 * Parameters:
 *		canvas		Specifies the canvas on which to render
 *				the text.
 *		line		Specifies the line metrics.
 *		p_seg		Specifies the starting segment.
 *		start_char	Specifies the starting index in a string
 *				segment. 0 for all others.
 *              count           Specifies the number of characters
 *				(including special characters to
 *				render).
 *              prev_lnk        Indicates the previous link index.  Used
 *				to calculate extra spacing needed for
 *				traversal and link markup.
 *              txt_x           Specifies the starting x of the
 *				segment(s).  This does *NOT* take into
 *				account traversal or link spacing.  This
 *				routine will do that.  This is so
 *				selected links will have correct spacing
 *				indicated.
 *              sel_x           Specifies where the selection x position
 *				begins.  Usually it equals txt_x, but
 *				sometimes it will be less than it to
 *				indicate blank space has been selected.
 *		super_width	Specifies the last super script x offset.
 *		super_y		Specifies the last super script y offset.
 *		sub_width		Specifies the last sub script x offset.
 *		sub_y		Specifies the last sub script y offset.
 *		last_was_sub	Specifies if the last element was a
 *				subscript.
 *		last_was_super	Specifies if the last element was a
 *				superscript.
 *		last_was_label	Specifies if the last element was a label.
 *              last_was_vis    Specifies if the last element was a
 *				visible hypertext link.
 *		old_flag	Specifies what the line use to look like.
 *		new_flag	Specifies what the line is to look like.
 *
 * Returns:	txt_x		Returns the maximum x unit processed.
 *
 * Purpose: _DtHelpCeDrawsSegments draws one or more segments based on
 *	    the count passed in.
 *
 *          This routine adds CELink to new_flag when rendering segments
 *	    that are hypertext links.  At the same time it will
 *	    determine the correct window hint and may place in old_flag
 *	    and new_flag either CELinkPopUp or CELinkNewWindow.
 *
 *	    This routine strips the CETraversalEnd and CETocTopicEnd
 *	    from old_flag and new_flag (based on what's in new_flag). It
 *	    will restore these flags (if specified) when it renders the
 *	    last element in the count sequence.
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeDrawSegments(canvas, line, p_seg, start_char, count, prev_lnk, txt_x,
			sel_x, script_x, super_width, super_y, sub_width, sub_y,
			last_was_sub, last_was_super, last_was_label,
			last_link_vis, old_flag, new_flag)
    CECanvasStruct	*canvas;
    CELines		 line;
    CESegment		*p_seg;
    int			 start_char;
    int			 count;
    int			*prev_lnk;
    Unit		 txt_x;
    Unit		 sel_x;
    Unit		*script_x;
    Unit		*super_width;
    Unit		*super_y;
    Unit		*sub_width;
    Unit		*sub_y;
    CEBoolean		*last_was_sub;
    CEBoolean		*last_was_super;
    CEBoolean		*last_was_label;
    CEBoolean		*last_link_vis;
    Flags		 old_flag;
    Flags		 new_flag;
#else
_DtHelpCeDrawSegments(
    CECanvasStruct	*canvas,
    CELines		 line,
    CESegment		*p_seg,
    int			 start_char,
    int			 count,
    int			*prev_lnk,
    Unit		 txt_x,
    Unit		 sel_x,
    Unit		*script_x,
    Unit		*super_width,
    Unit		*super_y,
    Unit		*sub_width,
    Unit		*sub_y,
    CEBoolean		*last_was_sub,
    CEBoolean		*last_was_super,
    CEBoolean		*last_was_label,
    CEBoolean		*last_link_vis,
    Flags		 old_flag,
    Flags		 new_flag )
#endif /* _NO_PROTO */
{
    int          charSize;
    int		 linkType = -1; /* set it as in _DtHelpCeGetLinkType for default */
    int          len;
    Unit	 segWidth;
    Unit	 yPos;
    char	*pChar;
    CESpecial   *pSpc;
    Flags	 saveEnd = new_flag & (CETraversalEnd | CETocTopicEnd);

    /*
     * skip any leading no-op lines
     */
    while (p_seg != NULL && _CEIsNoop(p_seg->seg_type))
      {
	start_char = 0;
	p_seg  = p_seg->next_disp;
      }

    /*
     * strip the any end info from the flags.
     * it will be put back on with the last element that makes up the count.
     */
    old_flag &= ~saveEnd;
    new_flag &= ~saveEnd;

    /*
     * now process the segments included in 'count'
     */
    while (p_seg != NULL && count > 0)
      {
	/*
	 * reset the baseline.
	 * when processing super or sub scripts, this gets changed.
	 */
	yPos = line.baseline;

	/*
	 * if the last was a label (old CCDF), we need to
	 * move the x to the real text begining.
	 * take into account the visible link and traversal info.
	 */
	txt_x = _DtHelpCeAdvanceXOfLine(canvas, line, p_seg, txt_x,
				prev_lnk, last_was_label, last_link_vis);

	/*
	 * check for super and sub scripts.
	 * adjust text x positioning accordingly.
	 */
	txt_x = _DtHelpCeAdjustForSuperSub(canvas, p_seg, txt_x,
					script_x, super_width, super_y,
					sub_width, sub_y,
					last_was_super, last_was_sub);
	if (sel_x > txt_x)
	    sel_x = txt_x;

	/*
	 * set visible link indicator flags
	 */
	new_flag &= ~(CELinkFlag);
	if (_CEIsVisibleLink(p_seg->seg_type))
	  {
	    new_flag |= CELinkFlag;
	    linkType  = _DtHelpCeGetLinkType(canvas->link_data,
							p_seg->link_index);
	    switch (_DtHelpCeGetLinkHint(canvas->link_data, p_seg->link_index))
	      {
		case CEWindowHint_PopupWindow:
				new_flag |= CELinkPopUp;
				old_flag |= CELinkPopUp;
				break;
		case CEWindowHint_NewWindow:
				new_flag |= CELinkNewWindow;
				old_flag |= CELinkNewWindow;
				break;
	      }
	  }
	
    	/*
	 * rememeber this link index.
         */
        *prev_lnk = p_seg->link_index;

	switch (_CEGetPrimaryType(p_seg->seg_type))
	  {
	    case CE_STRING:
		if (_CEIsSuperScript(p_seg->seg_type))
		    yPos -= *super_y;
		else if (_CEIsSubScript(p_seg->seg_type))
		    yPos += *sub_y;

	    case CE_LABEL:
	    case CE_CAPTION:

		/*
		 * get the string and its length.
		 */
	        pChar = _CEStringOfSegment(p_seg) + start_char;
	        len   = strlen (pChar);

		/*
		 * if length of the string is longer than we want to
		 * output, crop.
		 */
	        if (len > count)
	            len = count;

		/*
		 * if this is the last segment to be rendered,
		 * restore the end flags.
		 */
		if (len == count)
		  {
		    new_flag |= saveEnd;
		    old_flag |= saveEnd;
		  }

		/*
		 * get the num of bytes per char and render it.
		 */
		charSize = _CEGetCharLen(p_seg->seg_type);

		_DtHelpCeDrawString(canvas, txt_x, yPos, pChar, len, charSize,
				_CEFontOfSegment(p_seg), sel_x,
				line.baseline - line.ascent,
				line.ascent + line.descent + 1,
				linkType, old_flag, new_flag);

		/*
		 * now get the width of the string to update the x positions
		 */
		segWidth = _DtHelpCeGetStringWidth(canvas, p_seg,
						NULL, NULL, NULL, pChar, len);

		if (_CEIsSuperScript(p_seg->seg_type))
		    *super_width += segWidth;
		else if (_CEIsSubScript(p_seg->seg_type))
		    *sub_width += segWidth;

		txt_x      += segWidth;
		sel_x       = txt_x;
	        count      -= len;
	        start_char  = 0;
		break;

	    case CE_GRAPHIC:
		/*
		 * skip graphics.
		 */
		if (_CEGetGraphicType(p_seg->seg_type) == CE_IN_LINE)
		    txt_x += _CEGraphicWidth(canvas,_CEGraphicIdxOfSeg(p_seg));
		break;

	    case CE_SPECIAL:
		/*
		 * if this is the last segment to be rendered,
		 * restore the end flags.
		 */
		if (count == 1)
		  {
		    new_flag |= saveEnd;
		    old_flag |= saveEnd;
		  }

		/*
		 * render the special character.
		 */

		pSpc = p_seg->seg_handle.spc_handle;
		if (_CEIsValidSpc(p_seg->seg_type) &&
				canvas->virt_functions._CEDrawSpecial != NULL)
		    (*(canvas->virt_functions._CEDrawSpecial))(
			canvas->client_data,
			pSpc->spc_handle,
			txt_x, line.baseline,
			sel_x,
			line.baseline - line.ascent,
			line.ascent + line.descent + 1,
			linkType, old_flag, new_flag);

		/*
		 * now get the width of the spc to update the x positions
		 */
	        count--;
		txt_x += pSpc->width;
		sel_x  = txt_x;
		break;
	  }

        /*
         * strip the any traversal and toc topic begin flags.
         */
        old_flag &= ~(CETraversalBegin | CETocTopicBegin);
        new_flag &= ~(CETraversalBegin | CETocTopicBegin);

	p_seg = p_seg->next_disp;
      }

    return txt_x;

} /* End _DtHelpCeDrawSegments */

/******************************************************************************
 * Function: _DtHelpCeDrawTextSubSet
 *
 * Parameters:
 *		canvas		Specifies the canvas on which to render
 *				the text.
 *		dst_x		Specifies the starting x position to use
 *				for selected text. If greater than the
 *				starting position for the segment, determine
 *				the closest character to 'sel_x' and
 *				use it.
 *		end_x		Specifies the ending x position to use
 *				for selected text. If -1, means display
 *				the entire set of segments.
 *		old_flag	Specifies....
 *		new_flag	Specifies....
 *
 * Returns:	max_x		Returns the maximum x unit processed.
 *
 * Purpose: _DtHelpCeDrawText draws text segments on one line in the
 *          line table.
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeDrawTextSubSet(canvas, line, p_seg, start_char, count,
		toc_type, traversal_type, link_idx,
		txt_x, sel_x, end_x,
		super_width, super_y, sub_width, sub_y,
		last_was_super, last_was_sub, last_was_label, last_link_vis,
		old_flag, new_flag)
    CECanvasStruct	*canvas;
    CELines		 line;
    CESegment		*p_seg;
    int			 start_char;
    int			 count;
    int			 toc_type;
    int			 traversal_type;
    int			 link_idx;
    Unit		 txt_x;
    Unit		 sel_x;
    Unit		 end_x;
    Unit		 super_width;
    Unit		 super_y;
    Unit		 sub_width;
    Unit		 super_width;
    CEBoolean		 last_was_sub;
    CEBoolean		 last_was_super;
    CEBoolean		 last_was_label;
    CEBoolean		 last_link_vis;
    Flags		 old_flag;
    Flags		 new_flag;
#else
_DtHelpCeDrawTextSubSet(
    CECanvasStruct	*canvas,
    CELines		 line,
    CESegment		*p_seg,
    int			 start_char,
    int			 count,
    int			 toc_type,
    int			 traversal_type,
    int			 link_idx,
    Unit		 txt_x,
    Unit		 sel_x,
    Unit		 end_x,
    Unit		 super_width,
    Unit		 super_y,
    Unit		 sub_width,
    Unit		 sub_y,
    CEBoolean		 last_was_sub,
    CEBoolean		 last_was_super,
    CEBoolean		 last_was_label,
    CEBoolean		 last_link_vis,
    Flags		 old_flag,
    Flags		 new_flag )
#endif /* _NO_PROTO */
{
    int          charSize;
    int		 linkType;
    int          len;
    Unit	 segWidth;
    Unit	 yPos;
    Unit	 scriptX  = 0;
    char	*pChar;
    CEBoolean	 junk;
    CESpecial   *pSpc;

    /*
     * skip any leading no-op lines
     */
    while (p_seg != NULL && _CEIsNoop(p_seg->seg_type))
      {
	start_char = 0;
	p_seg  = p_seg->next_disp;
      }

    while (p_seg != NULL && count > 0 && (end_x == -1 || txt_x < end_x))
      {
	yPos = line.baseline;

	if (last_was_label && _CEIsNotLabel(p_seg->seg_type))
	  {
	    txt_x = line.text_x;
	    last_was_label = False;
	    if (end_x != -1 && txt_x > end_x)
		break;
	  }

	/*
	 * take into account the if this is a super or sub script - or not.
	 */
	txt_x = _DtHelpCeAdjustForSuperSub(canvas, p_seg, txt_x,
					&scriptX, &super_width, &super_y,
					&sub_width, &sub_y,
					&last_was_super, &last_was_sub);

	if (sel_x > txt_x)
	    sel_x = txt_x;

	old_flag &= (~(CETraversalBegin | CELinkPopUp   | CELinkNewWindow |
			CETocTopicBegin | CETocTopicEnd));
	new_flag &= (~(CETraversalBegin | CELinkPopUp   | CELinkNewWindow |
			CETocTopicBegin | CETocTopicEnd | CELinkFlag));

	/*
	 * set flags for toc, if necessary
	 */
	if (canvas->toc_flag == True)
	  {
	    new_flag |= toc_type;
	    old_flag |= toc_type;
	    toc_type = 0;
	  }

	/*
	 * set visible link indicator flags
	 */
	linkType  = 0;
	if (_CEIsVisibleLink(p_seg->seg_type))
	  {
	    new_flag |= CELinkFlag;
	    linkType  = _DtHelpCeGetLinkType (canvas->link_data, p_seg->link_index);
	    switch (_DtHelpCeGetLinkHint(canvas->link_data, p_seg->link_index))
	      {
		case CEWindowHint_PopupWindow:
				new_flag |= CELinkPopUp;
				old_flag |= CELinkPopUp;
				break;
		case CEWindowHint_NewWindow:
				new_flag |= CELinkNewWindow;
				old_flag |= CELinkNewWindow;
				break;
	      }

	    junk = _CEIsVisibleLink(p_seg->seg_type);
	    last_link_vis = _DtHelpCeModifyXpos (canvas->link_info, p_seg,
				junk,
				last_link_vis, link_idx, &txt_x);
	  }
	
	if (CEHasTraversal(new_flag) || CEHasTraversal(old_flag))
	  {
	    new_flag |= traversal_type;
	    old_flag |= traversal_type;
	  }

	traversal_type = 0;

    	/*
	 * take into account the traversal before and after space
         */
	junk = _CEIsHyperLink(p_seg->seg_type);
	(void) _DtHelpCeModifyXpos (canvas->traversal_info, p_seg,
				junk,
				((CEBoolean) True), link_idx, &txt_x);

        link_idx = p_seg->link_index;

	switch (_CEGetPrimaryType(p_seg->seg_type))
	  {
	    case CE_STRING:
		if (_CEIsSuperScript(p_seg->seg_type))
		    yPos -= super_y;
		else if (_CEIsSubScript(p_seg->seg_type))
		    yPos += sub_y;

	    case CE_LABEL:
	    case CE_CAPTION:

	        pChar = _CEStringOfSegment(p_seg) + start_char;

	        len = strlen (pChar);

	        if (len > count)
	            len = count;

		segWidth = _DtHelpCeGetStringWidth(canvas,p_seg,NULL,NULL,NULL,pChar,len);

		if ((CEHasTraversal(old_flag) || CEHasTraversal(new_flag))
				&& 
			(end_x <= 0 || txt_x + segWidth <= end_x)
				&&
			(len == count ||
			 p_seg->next_seg == NULL ||
			 p_seg->link_index != p_seg->next_seg->link_index))
		  {
		    new_flag |= (CETraversalEnd | CETocTopicEnd);
		    old_flag |= (CETraversalEnd | CETocTopicEnd);
		  }

		charSize = _CEGetCharLen(p_seg->seg_type);

		if (end_x > 0 && txt_x + segWidth > end_x)
		  {
		    if (txt_x > end_x)
			txt_x = end_x;

		    len = FindChar(canvas, p_seg, pChar, len, end_x - txt_x,
								&segWidth);
		    if (segWidth > 0)
			len++;

		    count = len;
		  }

		_DtHelpCeDrawString(canvas, txt_x, yPos, pChar, len, charSize,
				_CEFontOfSegment(p_seg), sel_x,
				line.baseline - line.ascent,
				line.ascent + line.descent + 1,
				linkType, old_flag, new_flag);

		if (_CEIsSuperScript(p_seg->seg_type))
		    super_width += segWidth;
		else if (_CEIsSubScript(p_seg->seg_type))
		    sub_width += segWidth;

		txt_x      += segWidth;
		sel_x       = txt_x;
	        count      -= len;
	        start_char  = 0;
		break;

	    case CE_GRAPHIC:
		if (_CEGetGraphicType(p_seg->seg_type) == CE_IN_LINE)
		    txt_x += _CEGraphicWidth(canvas,_CEGraphicIdxOfSeg(p_seg));
		break;

	    case CE_SPECIAL:
		if ((CEHasTraversal(old_flag) || CEHasTraversal(new_flag))
				&&
			(count == 1 ||
			 p_seg->next_seg == NULL ||
			 p_seg->link_index != p_seg->next_seg->link_index))
		  {
		    new_flag |= (CETraversalEnd | CETocTopicEnd);
		    old_flag |= (CETraversalEnd | CETocTopicEnd);
		  }

		if (_CEIsSuperScript(p_seg->seg_type))
		  {
		    yPos -= super_y;
		  }
		else if (_CEIsSubScript(p_seg->seg_type))
		  {
		    yPos += sub_y;
		  }

		pSpc = p_seg->seg_handle.spc_handle;
		if (_CEIsValidSpc(p_seg->seg_type) &&
				canvas->virt_functions._CEDrawSpecial != NULL)
		    (*(canvas->virt_functions._CEDrawSpecial))(
			canvas->client_data,
			pSpc->spc_handle,
			txt_x, yPos,
			sel_x,
			line.baseline - line.ascent,
			line.ascent + line.descent + 1,
			linkType, old_flag, new_flag);

	        count--;
		txt_x    += pSpc->width;
		sel_x  = txt_x;
		break;
	  }
	p_seg = p_seg->next_disp;
      }

    return txt_x;

} /* End _DtHelpCeDrawTextSubSet */

/******************************************************************************
 * Function: _DtHelpCeDrawText
 *
 * Parameters:
 *		canvas		Specifies the canvas on which to render
 *				the text.
 *		line		Specifies the line in the line table.
 *		start_x		Specifies the starting x position to use
 *				for selected text. If greater than the
 *				starting position for the segment, determine
 *				the closest character to 'start_x' and
 *				use it.
 *		end_x		Specifies the ending x position to use
 *				for selected text. If -1, means display
 *				the entire set of segments.
 *		old_flag	Specifies....
 *		new_flag	Specifies....
 *
 * Returns:	max_x		Returns the maximum x unit processed.
 *
 * Purpose: _DtHelpCeDrawText draws text segments on one line in the
 *          line table.
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeDrawText(canvas, line, start_x, end_x, old_flag, new_flag)
    CECanvasStruct	*canvas;
    CELines		 line;
    Unit		 start_x;
    Unit		 end_x;
    Flags		 old_flag;
    Flags		 new_flag;
#else
_DtHelpCeDrawText(
    CECanvasStruct	*canvas,
    CELines		 line,
    Unit		 start_x,
    Unit		 end_x,
    Flags		 old_flag,
    Flags		 new_flag )
#endif /* _NO_PROTO */
{
    char	*pChar;
    CEBoolean	 lastWasLabel = FALSE;
    CEBoolean	 done         = FALSE;
    CEBoolean	 lastLinkVisible = FALSE;
    CEBoolean	 lastWasSub      = FALSE;
    CEBoolean	 lastWasSuper    = FALSE;
    int          count        = line.length;
    int          start        = line.byte_index;
    int          len;
    int		 lnkInd = -1;
    int          traversalType = CETraversalBegin;
    int          tocType       = CETocTopicBegin;
    Unit	 segWidth;
    Unit	 xPos          = line.text_x;
    Unit	 superY        = 0;
    Unit	 subY          = 0;
    Unit	 subX          = 0;
    Unit	 superX        = 0;
    Unit	 txtX          = 0;
    CESegment   *pSeg;

    xPos = _DtHelpCeGetStartXOfLine(canvas, line, &pSeg, &lastWasLabel);

    /*
     * check to see if the start is in the middle of the line.
     * If so, bump the x position and start indexes to the
     * correct locations.
     */
    while (!done && start_x > xPos && count)
      {
	int cnt;

	xPos = _DtHelpCeAdvanceXOfLine(canvas, line, pSeg, xPos,
				&lnkInd, &lastWasLabel, &lastLinkVisible);

	/*
	 * take into account the if this is a super or sub script - or not.
	 */
	xPos = _DtHelpCeAdjustForSuperSub(canvas, pSeg, xPos,
					&txtX, &superX, &superY, &subX, &subY,
					&lastWasSuper, &lastWasSub);

        /*
         * advance the pointer by the width
         */
        _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, count,
							&cnt, &segWidth, NULL);
	if (segWidth + xPos <= start_x)
	  {
            if ((_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
					|| _CEIsCaption(pSeg->seg_type))
			&& !(lastWasSuper || lastWasSub))
                _DtHelpCeGetFontMetrics (canvas, _CEFontOfSegment(pSeg),
                                    NULL, NULL, NULL, &superY, &subY);

	    else if (_CEIsSuperScript(pSeg->seg_type))
		superX += segWidth;
	    else if (_CEIsSubScript(pSeg->seg_type))
		subX += segWidth;

	    xPos  += segWidth;
	    pSeg   = pSeg->next_disp;
	    count -= cnt;
	    start  = 0;
	  }
        else 
          {
	    if (_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
						|| _CEIsCaption(pSeg->seg_type))
	      {
                pChar = _CEStringOfSegment(pSeg) + start;
                len   = strlen (pChar);

	        if (len > count)
		    len = count;

		if (xPos < start_x)
		  {
	            len      = FindChar(canvas, pSeg, pChar, len,
							start_x - xPos, NULL);
		    segWidth = _DtHelpCeGetStringWidth(canvas, pSeg,
							NULL, NULL, NULL,
							pChar,len);

		    if (_CEIsSuperScript(pSeg->seg_type))
			superX += segWidth;
		    else if (_CEIsSubScript(pSeg->seg_type))
			subX += segWidth;
		    xPos += segWidth;
		  }
		else
		    len = 0;

		start += len;
		count -= len;
              }
            done = True;
          }
	traversalType = 0;
	tocType = 0;
      }

    return (_DtHelpCeDrawTextSubSet(canvas, line, pSeg, start, count,
				tocType, traversalType, lnkInd,
				xPos, start_x, end_x,
				superX, superY, subX, subY,
				lastWasSub, lastWasSuper,
				lastWasLabel, lastLinkVisible,
				old_flag, new_flag));

} /* End _DtHelpCeDrawText */

/*****************************************************************************
 * Function: _DtHelpCeDrawGraphics
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDrawGraphics(canvas, x1, y1, x2, y2)
    CECanvasStruct	*canvas;
    Unit		 x1;
    Unit		 y1;
    Unit		 x2;
    Unit		 y2;
#else
_DtHelpCeDrawGraphics(
    CECanvasStruct	*canvas,
    Unit		 x1,
    Unit		 y1,
    Unit		 x2,
    Unit		 y2)
#endif /* _NO_PROTO */
{
    int    i;
    CEGraphics *pGS;

    if (canvas->graphic_lst != NULL && canvas->graphic_cnt &&
	canvas->virt_functions._CEDrawGraphic != NULL)
      {
	Flags  flag;

        for (i = 0; i < canvas->graphic_cnt; i++)
          {
            pGS = _CEGraphicStructPtr(canvas,i);
	    if (_CEIsInitialized(pGS->type) &&
					pGS->pos_x != -1 && pGS->pos_y != -1)
	      {
	        flag = 0;

	        if (_CEIsVisibleLink(pGS->type))
		  {
		    flag = CELinkFlag;
		    switch(_DtHelpCeGetLinkHint(canvas->link_data,pGS->lnk_idx))
		      {
			case CEWindowHint_PopupWindow:
					flag |= CELinkPopUp;
					break;
			case CEWindowHint_NewWindow:
					flag |= CELinkNewWindow;
					break;
		      }
		  }

	        if ((y1 != y2
			&& pGS->pos_y + pGS->height >= y1 && pGS->pos_y <= y2 
			&& pGS->pos_x + pGS->width  >= x1)
		    || (y1 == y2 && x1 != x2
			&& pGS->pos_x + pGS->width  >= x1 && pGS->pos_x <= x2))
		    (*(canvas->virt_functions._CEDrawGraphic))(
			canvas->client_data, pGS->handle,
			pGS->pos_x, pGS->pos_y, flag, flag);
              }
          }
      }
} /* End _DtHelpCeDrawGraphics */

/*****************************************************************************
 * Function: _DtHelpCeFreeParagraphs
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeFreeParagraphs(canvas, link_data, para_lst, para_count)
    CECanvasStruct	 *canvas;
    CELinkData		  link_data;
    CEParagraph		**para_lst;
    int			  para_count;
#else
_DtHelpCeFreeParagraphs(
    CECanvasStruct	 *canvas,
    CELinkData		  link_data,
    CEParagraph		**para_lst,
    int			  para_count)
#endif /* _NO_PROTO */
{
    CEParagraph		*pList = *para_lst;
    int			 i;

    /*
     * free each paragraph
     */
    for (i = 0; i < para_count; i++)
      {
        /*
         * free the individual CESegments
         */
	_DtHelpCeFreeSegList(canvas, link_data, -1, pList->seg_list);

        free ((char *) pList->seg_list);

        /*
         * go to the next paragraph
         */
        pList++;

      }

    /*
     * free the paragraph list
     */
    if (*para_lst != NULL)
      {
        free (((char *) *para_lst));
        *para_lst = NULL;
      }

    /*
     * free the graphics
     */
   _DtHelpCeFreeGraphics(canvas);

} /* End _DtHelpCeFreeParagraphs */

/*****************************************************************************
 * Function: _DtHelpCeFreeSegList
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeFreeSegList(canvas, link_data, cnt, seg_list)
    CECanvasStruct	 *canvas;
    CELinkData		  link_data;
    int			  cnt;
    CESegment		 *seg_list;
#else
_DtHelpCeFreeSegList(
    CECanvasStruct	 *canvas,
    CELinkData		  link_data,
    int			  cnt,
    CESegment		 *seg_list)
#endif /* _NO_PROTO */
{
    int			 oldLink = -1;
    CEBoolean		 useCnt = True;

    /*
     * free the individual CESegments
     */
    if (cnt == -1) useCnt = False;
    while (seg_list != NULL && (useCnt == False || cnt-- > 0))
      {
        if (_CEIsGraphic (seg_list->seg_type))
          {
            /*
             * If this graphic has not been initialized,
             * free the filename string.
             */
            if (_CEIsNotInitialized(seg_list->seg_type))
                free (seg_list->seg_handle.file_handle);
          }
        else if (_CEGetPrimaryType (seg_list->seg_type) != CE_NOOP)
          {
            free (_CEStringOfSegment(seg_list));
            free (seg_list->seg_handle.str_handle);
          }

        /*
         * free the hypertext specification
         */
        if (_CEIsHyperLink(seg_list->seg_type) &&
                    oldLink != seg_list->link_index && link_data.list != NULL)
            _DtHelpCeRmLinkFromList (link_data, seg_list->link_index);

        oldLink = seg_list->link_index;

        /*
         * go to the next structure or indicate the end.
         */
        seg_list = seg_list->next_seg;
      }

} /* End _DtHelpCeFreeSegList */

/*****************************************************************************
 *		Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	CanvasHandle _DtHelpCeCreateCanvas (VirtualInfo virt_info);
 *
 * Parameters:
 *		virt_info	Specifies the virtual functions to attach
 *				to the created canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:	Create a canvas and attach the appropriate virtual functions
 *		to the canvas.
 *****************************************************************************/
CanvasHandle
#ifdef _NO_PROTO
_DtHelpCeCreateCanvas (virt_info, client_data)
    VirtualInfo virt_info;
    ClientData  client_data;
#else
_DtHelpCeCreateCanvas (
    VirtualInfo	virt_info,
    ClientData  client_data)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*newCanvas;

    newCanvas = (CECanvasStruct *) malloc (sizeof (CECanvasStruct));
    if (newCanvas == NULL)
	return NULL;

    *newCanvas                = DefaultCanvas;
    newCanvas->virt_functions = virt_info;
    newCanvas->client_data    = client_data;
    newCanvas->mb_length      = MB_CUR_MAX;

    if (virt_info._CEGetCanvasMetrics != NULL)
        (*(virt_info._CEGetCanvasMetrics))(
		client_data, &(newCanvas->metrics), &(newCanvas->font_info),
		&(newCanvas->link_info), &(newCanvas->traversal_info));

    if (MB_CUR_MAX > 1)
	_DtHelpCeLoadMultiInfo(newCanvas);

    return ((CanvasHandle)(newCanvas));

} /* End _DtHelpCeCreateCanvas */

/*****************************************************************************
 * Function:	void _DtHelpCeCleanCanvas (CanvasHandle canvas);
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:	Create a canvas and attach the appropriate virtual functions
 *		to the canvas.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeCleanCanvas (canvas)
    CanvasHandle canvas;
#else
_DtHelpCeCleanCanvas (CanvasHandle canvas)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*oldCanvas = (CECanvasStruct *) canvas;

    /*
     * clean the selection
     */
    _DtHelpCeProcessSelection (canvas, 0, 0, CESelectionClear);

    /*
     * deallocate the information
     */
    if (oldCanvas->para_lst != NULL)
        _DtHelpCeFreeParagraphs (oldCanvas, oldCanvas->link_data,
					&(oldCanvas->para_lst),
					oldCanvas->para_count);
    else if (oldCanvas->element_lst != NULL)
	_DtHelpCeFreeSdlStructs(canvas, &(oldCanvas->element_lst),
						&(oldCanvas->link_data));

    if (oldCanvas->volume != NULL)
      {
	_DtHelpCeCloseVolume(canvas, oldCanvas->volume);
	oldCanvas->volume = NULL;
      }

    /*
     * zero the lists
     */
    oldCanvas->toc_flag    = False;
    oldCanvas->toc_on      = False;
    oldCanvas->para_count  = 0;
    oldCanvas->txt_cnt     = 0;
    oldCanvas->line_cnt    = 0;
    oldCanvas->graphic_cnt = 0;
    oldCanvas->hyper_count = 0;

    /*
     * reset some indicators
     */
    oldCanvas->error       = 0;
    oldCanvas->toc_flag    = False;
    oldCanvas->cur_hyper   = 0;

    /*
     * free the link data structures.
     */
    if (oldCanvas->link_data.list != NULL)
        free(oldCanvas->link_data.list);

    oldCanvas->link_data.list = NULL;
    oldCanvas->link_data.max  = 0;

} /* End _DtHelpCeCleanCanvas */

/*****************************************************************************
 * Function:	void _DtHelpCeDestroyCanvas (CanvasHandle canvas);
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:	Create a canvas and attach the appropriate virtual functions
 *		to the canvas.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDestroyCanvas (canvas)
    CanvasHandle canvas;
#else
_DtHelpCeDestroyCanvas (CanvasHandle canvas)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*myCanvas = (CECanvasStruct *) canvas;

    _DtHelpCeCleanCanvas (canvas);
    _DtHelpCeFreeGraphics(canvas);

    if (myCanvas->txt_lst != NULL)
        free ((void *) myCanvas->txt_lst);
    if (myCanvas->max_x_lst != NULL)
        free ((void *) myCanvas->max_x_lst);
    if (myCanvas->line_lst != NULL)
        free ((void *) myCanvas->line_lst);
    if (myCanvas->graphic_lst != NULL)
        free ((void *) myCanvas->graphic_lst);
    if (myCanvas->hyper_segments != NULL)
        free ((void *) myCanvas->hyper_segments);
    free ((void *) myCanvas);
    return;

} /* End _DtHelpCeDestroyCanvas */

/*****************************************************************************
 * Function:	void _DtHelpCeScrollCanvas (CanvasHandle canvas);
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeScrollCanvas (canvas, x1, y1, x2, y2)
    CanvasHandle canvas;
    Unit	 x1;
    Unit	 y1;
    Unit	 x2;
    Unit	 y2;
#else
_DtHelpCeScrollCanvas (
    CanvasHandle canvas,
    Unit	 x1,
    Unit	 y1,
    Unit	 x2,
    Unit	 y2)
#endif /* _NO_PROTO */
{
    int			 i;
    CECanvasStruct	*myCanvas = (CECanvasStruct *) canvas;
    CELines		*lines    = myCanvas->txt_lst;
    Flags		 oldFlag;
    Flags		 newFlag;

    if (lines != NULL && myCanvas->txt_cnt)
      {
	i = _DtHelpCeGetLineAtY (myCanvas, lines, myCanvas->txt_cnt, y1);

	while (i < myCanvas->txt_cnt)
	  {
	    if (lines[i].baseline - lines[i].ascent <= y2)
	      {
	        oldFlag = 0;
	        newFlag = 0;

	        if (myCanvas->toc_on == True && i == myCanvas->toc_loc)
	          {
		    oldFlag |= CETocTopicFlag;
		    newFlag |= CETocTopicFlag;
	          }

	        (void) _DtHelpCeDrawText (canvas, lines[i], lines[i].label_x, 
				-1, oldFlag, newFlag);
	      }

	    i++;
	  }
      }

    DrawCanvasLines (myCanvas, x1, y1, x2, y2);

    _DtHelpCeDrawGraphics (myCanvas, x1, y1, x2, y2);

    _DtHelpCeDrawSelection (myCanvas, y1, y2);

} /* End _DtHelpCeScrollCanvas */

