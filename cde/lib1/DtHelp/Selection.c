/* $XConsortium: Selection.c /main/cde1_maint/2 1995/10/08 17:20:57 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     Selection.c
 **
 **   Project:  Cde DtHelp
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
#include <string.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "SelectionI.h"
#include "VirtFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	void	AdjustSelection ();
static	int	GetSelectedText();
static	void	SetSelected ();
#else
static	void	AdjustSelection (
			CECanvasStruct	*canvas,
			DtHelpCeSelectStruct	 next);
static	int	GetSelectedText(
			CECanvasStruct	*canvas,
			DtHelpCeSelectStruct	 start,
			DtHelpCeSelectStruct	 end,
			SelectionInfo	*ret_data);
static	void	SetSelected (
			CECanvasStruct	*canvas,
			DtHelpCeSelectStruct	 start,
			DtHelpCeSelectStruct	 end,
			CEBoolean	 flag);
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    Private Defines                 ********/
#define	GROW_SIZE	5
/********    End Private Defines             ********/

/********    Macros                          ********/
#define	ClearProcessed(x)	(x).processed = False
#define	SetProcessed(x)		(x).processed = True
#define	Processed(x)		(x).processed
#define	NotProcessed(x)		(!(x).processed)

#define	Equal(a,b)		(a.y == b.y && a.x == b.x)
#define	LessThan(a,b)		((a.y < b.y) || (a.y == b.y && a.x < b.x))
#define	GreaterThan(a,b)	((a.y > b.y) || (a.y == b.y && a.x > b.x))
#define	LessThanEq(a,b)		(Equal(a,b) || LessThan(a,b))
#define	GreaterThanEq(a,b)	(Equal(a,b) || GreaterThan(a,b))

#define	StraddlesPt(pt,min,max)		((min) <= (pt)  && (pt)  <= (max))
#define	InRegion(top,bot,min,max)	((min) <= (bot) && (top) <= (max))

/********    End Macros                      ********/

/********    Private Variable Declarations    ********/
static	const DtHelpCeSelectStruct	defaultSelect = { -1, -1, -1, -1};

/********    End Private Variable Declarations    ********/

/******************************************************************************
 *                             Private Functions
 ******************************************************************************/
/******************************************************************************
 * Function: StartXOfLine
 *
 * Purpose: Determine the start of a line, takes into consideration
 *          the traversal and link before values. The 'x' returned is
 *          exactly where the text/graphic is to be placed on the canvas.
 *****************************************************************************/
static Unit
#ifdef _NO_PROTO
StartXOfLine(canvas, line)
    CECanvasStruct	*canvas;
    CELines		 line;
#else
StartXOfLine(
    CECanvasStruct	*canvas,
    CELines		 line)
#endif /* _NO_PROTO */
{
    CEBoolean	 lastLinkVisible = FALSE;
    CEBoolean	 junk;
    int		 lnkInd = -1;
    Unit	 xPos;
    CESegment   *pSeg;

    xPos = _DtHelpCeGetStartXOfLine(canvas, line, &pSeg, &junk);
    return (_DtHelpCeAdvanceXOfLine( canvas, line, pSeg, xPos,
			&lnkInd, &junk, &lastLinkVisible));

} /* End StartXOfLine */

/******************************************************************************
 * Function: MaxXOfLine
 *
 *  MaxXOfLine determines the max X of a line segment.
 *
 *****************************************************************************/
static Unit
#ifdef _NO_PROTO
MaxXOfLine(canvas, line)
    CECanvasStruct	*canvas;
    CELines		 line;
#else
MaxXOfLine(
    CECanvasStruct	*canvas,
    CELines		 line)
#endif /* _NO_PROTO */
{
    CEBoolean	 lastWasLabel = FALSE;
    CEBoolean	 lastLinkVisible = FALSE;
    int          count        = line.length;
    int          start        = line.byte_index;
    int          len;
    int		 lnkInd = -1;
    Unit	 xPos;
    Unit	 tmpWidth;
    CESegment   *pSeg;

    xPos = _DtHelpCeGetStartXOfLine(canvas, line, &pSeg, &lastWasLabel);

    while (pSeg != NULL && count > 0)
      {
	xPos = _DtHelpCeAdvanceXOfLine(canvas, line, pSeg, xPos,
				&lnkInd, &lastWasLabel, &lastLinkVisible);

	_DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, count,
							&len, &tmpWidth, NULL);
	xPos  += tmpWidth;
	count -= len;
	start  = 0;
	pSeg   = pSeg->next_disp;
      }

    return xPos;

} /* End MaxXOfLine */

/*****************************************************************************
 * Function: SearchForClosestLine
 *
 * Purpose:  Initializes the 'new' structure with information indicating
 *	     what line is closest to the target_y.
 *		next->y         Set to target_y if no line straddles it.
 *				Otherwise, it will be set to the minimum
 *				y of all lines straddling the target_y.
 *		next->x         Set to target_x if no line straddles
 *				target_y or if target_x is before the
 *				first line straddling target_y.
 *		next->line_idx  Set to -1 if no line straddles target_y.
 *				Otherwise, set to the first line that
 *				straddles target_x or is the minimum x
 *				that is greater than target_x of all the
 *				lines straddling target_x.
 *		next->char_idx  Set to -1 if no straddles target_y.
 *				Otherwise, set to the character that
 *				resides at target_x if target_x is in
 *				the middle of the line.  Set to zero if
 *				target_x is before the line, and set to
 *				the line count if target_x is after the
 *				line.
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
SearchForClosestLine (canvas, target_x, target_y, next)
    CECanvasStruct	*canvas;
    Unit		 target_x;
    Unit		 target_y;
    DtHelpCeSelectStruct *next;
#else
SearchForClosestLine (
    CECanvasStruct	*canvas,
    Unit		 target_x,
    Unit		 target_y,
    DtHelpCeSelectStruct *next)
#endif /* _NO_PROTO */
{
    int		 i;
    int		 maxI;
    Unit	 lineY;
    Unit	 endX;
    Unit	 begX;
    Unit	 maxX = -1;
    CELines	*lines = canvas->txt_lst;

    *next = defaultSelect;
    for (i = 0; i < canvas->txt_cnt; i++)
      {
	/*
	 * get the maximum y of the line
	 * if it straddles the target y, process it.
	 */
	lineY = lines[i].baseline + lines[i].descent;
	if (StraddlesPt(target_y, lines[i].baseline - lines[i].ascent, lineY))
	  {
	    /*
	     * Is this the minimum y of all the maximum y values of the
	     * line straddling the target y?
	     */
	    if (next->y  == -1 || next->y > lineY)
	        next->y = lineY;

	    /*
	     * Get the maximum X position of the line.
	     * If this is the maximum X of all the lines straddling
	     * the target y, remember it.
	     */
	    endX = canvas->max_x_lst[i];
	    if (maxX < endX && endX < target_x)
	      {
		maxX = endX;
		maxI = i;
	      }

	    /*
	     * Does this line straddle the x?
	     */
	    begX = StartXOfLine(canvas, lines[i]);
	    if (StraddlesPt(target_x, begX, endX))
	      {
		next->line_idx = i;
		next->char_idx = _DtHelpCeGetCharIdx(canvas,lines[i],target_x);
	      }
	  }
      }

    /*
     * remember what the target x was for this line.  If the target x is
     * less than the start of the line, then the selection process will
     * highlight the space before the line.  If its in the middle, it
     * will just highlight starting at the character.  If it's after the
     * end, the rest will be cut off at the end of the line.
     */
    next->x = target_x;

    /*
     * If we found a line straddling the target y, but it does not
     * straddle the target_x, check max x for the correct info.
     */
    if (next->line_idx == -1 && maxX > -1)
      {
	next->line_idx = maxI;
	next->char_idx = lines[maxI].length;
      }

    /*
     * didn't find a line straddling the target_y, set y.
     */
    if (next->y == -1)
        next->y = target_y;
}

/*****************************************************************************
 * Function: MarkLinesOutsideBoundary
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
MarkLinesOutsideBoundary (canvas, top_y, top_x, bot_y, bot_x)
    CECanvasStruct	*canvas,
    Unit		 top_y;
    Unit		 top_x;
    Unit		 bot_y;
    Unit		 bot_x;
#else
MarkLinesOutsideBoundary (
    CECanvasStruct	*canvas,
    Unit		 top_y,
    Unit		 top_x,
    Unit		 bot_y,
    Unit		 bot_x)
#endif /* _NO_PROTO */
{
    int       i;
    Unit      maxY;
    Unit      minY;
    CELines  *lines = canvas->txt_lst;

    for (i = 0; i < canvas->txt_cnt; i++)
      {
        maxY = lines[i].baseline + lines[i].descent;
	minY = lines[i].baseline - lines[i].ascent;

	/*
	 * is this line outside the boundary?
	 * If so, mark it so it's not processed.
	 */
        if (maxY < top_y || minY > bot_y )
	    SetProcessed(lines[i]);

	/*
	 * does it straddle the top?
	 */
	else if (StraddlesPt(top_y, minY, maxY))
	  {
	    /*
	     * Does it begin before the selection?
	     * If so, mark it so it's not processed.
	     */
	    if (canvas->max_x_lst[i] < top_x)
		SetProcessed(lines[i]);
	  }
	
	/*
	 * does it straddle the bottom?
	 */
	else if (StraddlesPt(bot_y, minY, maxY))
	  {
	    /*
	     * Does it start after the selection?
	     * If so, mark it so it's not processed.
	     */
	    if (StartXOfLine(canvas, lines[i]) > bot_x)
		SetProcessed(lines[i]);
	  }
      }
}

/*****************************************************************************
 * Function: SetSelected
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
SetSelected (canvas, start, end, flag)
    CECanvasStruct	*canvas;
    DtHelpCeSelectStruct		 start;
    DtHelpCeSelectStruct		 end;
    CEBoolean		 flag;
#else
SetSelected (
    CECanvasStruct	*canvas,
    DtHelpCeSelectStruct start,
    DtHelpCeSelectStruct end,
    CEBoolean		 flag)
#endif /* _NO_PROTO */
{
    int    i;
    int    len;
    int    count;
    int    start_char;
    int    lnkInd;
    Unit   dstX;
    Unit   topY;
    Unit   botY;
    Unit   superWidth;
    Unit   subWidth;
    Unit   superY;
    Unit   subY;
    Unit   scriptX;
    Unit   segWidth;
    Flags  oldFlag = CESelectedFlag;
    Flags  newFlag = CESelectedFlag;
    CESegment  *pSeg;
    CEBoolean   lstWasLabel;
    CEBoolean   lstLinkVis;
    CEBoolean   lstWasSuper;
    CEBoolean   lstWasSub;
    CEBoolean   trimmed;

    CELines   *lines = canvas->txt_lst;

    if (flag == False)
	newFlag = 0;
    else
	oldFlag = 0;

    if (Equal(start, end))
	return;

    for (i = 0; i < canvas->txt_cnt; i++)
      {
	topY  = lines[i].baseline - lines[i].ascent;
	botY  = lines[i].baseline + lines[i].descent;

	if (InRegion(topY, botY, start.y, end.y))
	  {
	    /*
	     * get the start of the text.
	     */
	    lstWasLabel = False;
	    lstLinkVis  = False;
	    lstWasSuper = False;
	    lstWasSub   = False;
	    lnkInd      = -1;
	    dstX        = _DtHelpCeGetStartXOfLine(canvas, lines[i],
							&pSeg, &lstWasLabel);
	    start_char  = lines[i].byte_index;
	    count       = lines[i].length;

	    while (pSeg != NULL && _CEIsNoop(pSeg->seg_type))
	      {
	        start_char = 0;
	        pSeg       = pSeg->next_disp;
	      }

	    /*
	     * advance the starting point
	     */
	    dstX = _DtHelpCeAdvanceXOfLine(canvas, lines[i], pSeg, dstX,
					&lnkInd, &lstWasLabel, &lstLinkVis);
	    /*
	     * take into account super/sub scripting
	     */
	    dstX = _DtHelpCeAdjustForSuperSub(canvas, pSeg, dstX, &scriptX,
					&superWidth, &superY, &subWidth, &subY,
					&lstWasSuper, &lstWasSub);

	    /*
	     * set this flag so that the first pass of 'while (cnt > 0)'
	     * doesn't do it again.
	     */
	    trimmed = True;

	    if (StraddlesPt(start.y, topY, botY))
	      {
		/*
		 * skip this item?
		 * I.E. is this line before the start or after the end?
		 */
		if (canvas->max_x_lst[i] < start.x ||
					end.y == start.y && end.x <= dstX )
		    continue;

		/*
		 * does this line start the selection?
		 */
		if (i == start.line_idx && start.x > dstX)
		  {
		    int cnt = start.char_idx;

		    while (cnt > 0)
		      {
			if (trimmed == False)
			  {
			    /*
			     * advance the starting point
			     */
			    dstX = _DtHelpCeAdvanceXOfLine(canvas,
					lines[i], pSeg, dstX,
					&lnkInd, &lstWasLabel, &lstLinkVis);
			    /*
			     * take into account super/sub scripting
			     */
			    dstX = _DtHelpCeAdjustForSuperSub(canvas,
					pSeg, dstX, &scriptX,
					&superWidth, &superY, &subWidth, &subY,
					&lstWasSuper, &lstWasSub);
			  }

			/*
			 * take into account the length of the segment
			 */
			_DtHelpCeDetermineWidthOfSegment(canvas, pSeg,
					start_char, cnt,
					&len, &segWidth, &trimmed);

			dstX       += segWidth;
			start_char += len;
			if (trimmed == False)
			  {
			    start_char = 0;
			    pSeg       = pSeg->next_disp;
			  }

			trimmed = False;

			cnt -= len;

		      }

		    count -= start.char_idx;
		  }

		/*
		 * otherwise this line is after the line that starts
		 * the selection. Stick with its start x.
		 */
	      }

	    /*
	     * does this straddle the end point?
	     */
	    if (StraddlesPt(end.y, topY, botY))
	      {
		/*
		 * does this start after the end of the selection?
		 * if so, skip.
		 */
		if (end.x <= dstX)
		    continue;

		/*
		 * Does this segment end after the end of the selection?
		 * If so, trim how much gets highlighted.
		 */
		if (canvas->max_x_lst[i] > end.x)
		    count -= (lines[i].length - end.char_idx);
	      }

	    if (count > 0)
	        (void) _DtHelpCeDrawSegments(canvas, lines[i],
					pSeg, start_char, count, &lnkInd,
					dstX, dstX, &scriptX,
					&superWidth, &superY, &subWidth, &subY,
					&lstWasSub, &lstWasSuper, &lstWasLabel,
					&lstLinkVis, oldFlag, newFlag);
	  }
      }
    _DtHelpCeDrawGraphics(canvas, start.x, start.y, end.x, end.y);
}

/*****************************************************************************
 * Function: AdjustSelection
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
AdjustSelection (canvas, next)
    CECanvasStruct	*canvas;
    DtHelpCeSelectStruct next;
#else
AdjustSelection (
    CECanvasStruct	*canvas,
    DtHelpCeSelectStruct next)
#endif /* _NO_PROTO */
{
    DtHelpCeSelectStruct  start = canvas->select_start;
    DtHelpCeSelectStruct  end   = canvas->select_end;

    if (!(Equal(next, end)))
      {
	if (next.line_idx != -1 && next.line_idx == canvas->select_end.line_idx
		&&
	    next.char_idx != -1 && next.char_idx == canvas->select_end.char_idx)
	    return;

	if (GreaterThan(next, end))
	  {
	    if (LessThanEq(start, end))
		SetSelected (canvas, end, next, True);

	    else if (GreaterThanEq(start, next))
		SetSelected (canvas, end, next, False);

	    else /* end < start < next */
	      {
		SetSelected (canvas, end  , start, False);
		SetSelected (canvas, start, next  , True);
	      }
	  }
	else /* if (next < end) */
	  {
	    if (LessThanEq(start, next))
		SetSelected (canvas, next, end, False);

	    else if (GreaterThanEq(start, end))
		SetSelected (canvas, next, end, True);

	    else /* next < start < end */
	      {
		SetSelected (canvas, start, end  , False);
		SetSelected (canvas, next  , start, True);
	      }
	  }
      }

    canvas->select_end = next;
}

/*****************************************************************************
 * Function: SkipOtherLines
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
SkipOtherLines (lines, max_cnt, idx, target_y, ret_idx)
    CELines	*lines;
    int		 max_cnt;
    int		 idx;
    Unit	 target_y;
    int		*ret_idx;
#else
SkipOtherLines(
    CELines	*lines,
    int		 max_cnt,
    int		 idx,
    Unit	 target_y,
    int		*ret_idx)
#endif /* _NO_PROTO */
{
    while (idx < max_cnt && NotProcessed(lines[idx]) &&
			lines[idx].baseline - lines[idx].ascent > target_y)
        idx++;

    *ret_idx = idx;
}

/*****************************************************************************
 * Function: CheckAndSwitchPoints
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
CheckAndSwitchPoints (pt1, pt2)
    DtHelpCeSelectStruct *pt1;
    DtHelpCeSelectStruct *pt2;
#else
CheckAndSwitchPoints(
    DtHelpCeSelectStruct *pt1,
    DtHelpCeSelectStruct *pt2)
#endif /* _NO_PROTO */
{
    DtHelpCeSelectStruct temp;

    if (pt1->y > pt2->y || (pt1->y == pt2->y && pt1->x >  pt2->x))
      {
	temp = *pt2;
	*pt2 = *pt1;
	*pt1 = temp;
      }
}

/*****************************************************************************
 * Function: AddSegmentToData
 *
 *****************************************************************************/
static Unit
#ifdef _NO_PROTO
AddSegmentToData(canvas, start_x, line_idx, char_idx, copy_cnt, end_flag,
		 ret_data)
    CECanvasStruct	*canvas;
    Unit		 start_x;
    int			 line_idx;
    int			 char_idx;
    int			 copy_cnt;
    Flags		 end_flag;
    SelectionInfo	*ret_data;
#else
AddSegmentToData(
    CECanvasStruct	*canvas,
    Unit		 start_x,
    int			 line_idx,
    int			 char_idx,
    int			 copy_cnt,
    Flags		 end_flag,
    SelectionInfo	*ret_data)
#endif /* _NO_PROTO */
{
    CELines	 line   = canvas->txt_lst[line_idx];
    int		 result = 0;
    int		 count  = line.length;
    int		 start  = line.byte_index;
    int		 lnkInd = -1;
    int		 cnt;
    int		 len;
    Unit	 segWidth;
    Unit	 xPos  = line.text_x;
    char	*pChar;
    CESegment	*pSeg  = line.seg_ptr;
    CESpecial	*pSpc;
    Flags	 flag  = 0;
    CEBoolean	 done  = False;
    CEBoolean	 lastWasLabel    = False;
    CEBoolean	 lastLinkVisible = FALSE;

    xPos = _DtHelpCeGetStartXOfLine(canvas, line, &pSeg, &lastWasLabel);

    while (done == False && char_idx)
      {
	/*
	 * advance past the link and traversal info
	 */
	xPos = _DtHelpCeAdvanceXOfLine(canvas, line, pSeg, xPos,
				&lnkInd, &lastWasLabel, &lastLinkVisible);

        /*
         * advance the pointer by the width
         */
        _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, count,
							&cnt, &segWidth, NULL);
	if (cnt < char_idx)
	  {
	    xPos     += segWidth;
	    pSeg      = pSeg->next_disp;
	    count    -= cnt;
	    char_idx -= cnt;
	    start     = 0;
	  }
        else 
          {
            _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, char_idx,
							&cnt, &segWidth, NULL);
	    xPos  += segWidth;
	    start += cnt;
	    count -= cnt;
            done = True;
          }
      }

    if (start_x > xPos)
	start_x = xPos;

    while (result == 0 && pSeg != NULL && copy_cnt > 0)
      {
	/*
	 * advance past the link and traversal info
	 */
	xPos = _DtHelpCeAdvanceXOfLine(canvas, line, pSeg, xPos,
				&lnkInd, &lastWasLabel, &lastLinkVisible);

        switch (_CEGetPrimaryType(pSeg->seg_type))
          {
            case CE_STRING:
            case CE_LABEL:
            case CE_CAPTION:

                pChar = _CEStringOfSegment(pSeg) + start;
                len   = strlen (pChar);

                if (len > copy_cnt)
                    len = copy_cnt;

                segWidth = _DtHelpCeGetStringWidth(canvas,pSeg,
						NULL,NULL,NULL,pChar,len);
		if (copy_cnt == len)
		    flag = end_flag;

		if (canvas->virt_functions._CEBuildSelectionString != NULL)
		    result=(*(canvas->virt_functions._CEBuildSelectionString))(
				canvas->client_data, ret_data,
				pChar, len,
				_CEGetCharLen(pSeg->seg_type),
				_CEFontOfSegment(pSeg),
				xPos - start_x, flag);

                xPos     += segWidth;
                start_x   = xPos;
                copy_cnt -= len;
                start     = 0;
                break;

            case CE_GRAPHIC:
                if (_CEGetGraphicType(pSeg->seg_type) == CE_IN_LINE)
                    xPos += _CEGraphicWidth(canvas,_CEGraphicIdxOfSeg(pSeg));
                break;

            case CE_SPECIAL:
                pSpc = pSeg->seg_handle.spc_handle;
		if (copy_cnt == 1)
		    flag = end_flag;

                if (canvas->virt_functions._CEBuildSelectionSpc != NULL)
                    result = (*(canvas->virt_functions._CEBuildSelectionSpc))(
                        canvas->client_data,
			ret_data,
                        pSpc->spc_handle,
                        xPos - start_x, flag);

                copy_cnt--;
                xPos    += pSpc->width;
                start_x  = xPos;
                break;
          }
        pSeg = pSeg->next_disp;
      }

    if (result < 0)
	return -1;

    return start_x;
}

/*****************************************************************************
 * Function: BuildLine
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
BuildLine(canvas, prev_y, target_x, line_idx, char_idx, copy_cnt, end_flag,
	ret_x, ret_data)
    CECanvasStruct	*canvas;
    Unit		 prev_y;
    Unit		 target_x;
    int			 line_idx;
    int			 char_idx;
    int			 copy_cnt;
    Flags		 end_flag;
    Unit		*ret_x;
    SelectionInfo	*ret_data;
#else
BuildLine(
    CECanvasStruct	*canvas,
    Unit		 prev_y,
    Unit		 target_x,
    int			 line_idx,
    int			 char_idx,
    int			 copy_cnt,
    Flags		 end_flag,
    Unit		*ret_x,
    SelectionInfo	*ret_data)
#endif /* _NO_PROTO */
{
    Unit         topY;
    CELines	*lines = canvas->txt_lst;

    topY = lines[line_idx].baseline - lines[line_idx].ascent;
    if (topY > prev_y && canvas->virt_functions._CEBuildSelectionString != NULL)
      {
	int   newLines;
	Unit  lineSize = (canvas->font_info.ascent + canvas->font_info.descent)
				/ 2;
	if (lineSize < 1)
	    lineSize = 1;

	newLines = (topY - prev_y) / lineSize;
	while (newLines > 0)
	  {
	    newLines--;
	    if ((*(canvas->virt_functions._CEBuildSelectionString))(
				canvas->client_data, ret_data,
				NULL, 0, 1, NULL, 0, CEEndOfLine) != 0)
		return -1;
	  }
      }

    *ret_x = AddSegmentToData (canvas, target_x, line_idx, char_idx, copy_cnt,
							end_flag, ret_data);
    SetProcessed(lines[line_idx]);

    if (*ret_x == -1)
	return -1;
    return 0;
}

/*****************************************************************************
 * Function: FindMinX
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
FindMinX (lines, txt_cnt, target_y, ret_line)
    CELines	*lines;
    int		 txt_cnt;
    Unit	 target_y;
    int		*ret_line;
#else
FindMinX (
    CELines	*lines,
    int		 txt_cnt,
    Unit	 target_y,
    int		*ret_line)
#endif /* _NO_PROTO */
{
    int	  i;
    int   cnt    = 0;
    Unit  curMin = -1;
    Unit  curX;

    for (i = 0; i < txt_cnt; i++)
      {
	if (NotProcessed(lines[i]))
	  {
	    if (lines[i].baseline - lines[i].ascent < target_y &&
			target_y <= lines[i].baseline + lines[i].descent)
	      {
	        cnt++;
	        curX = lines[i].text_x;
	        if (_CEIsLabel(lines[i].seg_ptr->seg_type))
		    curX = lines[i].label_x;

	        if (curMin == -1 || curMin > curX)
		  {
	            curMin    = curX;
		    *ret_line = i;
		  }
              }
          }
      }
    return cnt;
}

/*****************************************************************************
 * Function: FindNextMinY
 *
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
FindNextMinY(lines, max_cnt, target_y, ret_y)
    CELines	*lines;
    int		 max_cnt;
    Unit	 target_y;
    Unit	*ret_y;
#else
FindNextMinY(
    CELines	*lines,
    int		 max_cnt,
    Unit	 target_y,
    Unit	*ret_y)
#endif /* _NO_PROTO */
{
    int		i     = 0;
    Unit	maxY;
    CEBoolean	found = False;

    while (i < max_cnt)
     {
	if (NotProcessed(lines[i]))
	  {
	    maxY = lines[i].baseline + lines[i].descent;
	    if (target_y == -1 || maxY < target_y)
	      {
		found    = True;
		target_y = maxY;
	      }
	    SkipOtherLines (lines, max_cnt, i+1, target_y, &i);
	  }
	else
	    i++;
     }

    *ret_y = target_y;
    return found;
}

/*****************************************************************************
 * Function: GetSelectedText
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
GetSelectedText (canvas, next, end, ret_data)
    CECanvasStruct	*canvas;
    DtHelpCeSelectStruct next;
    DtHelpCeSelectStruct end;
    SelectionInfo	*ret_data;
#else
GetSelectedText(
    CECanvasStruct	*canvas,
    DtHelpCeSelectStruct next,
    DtHelpCeSelectStruct end,
    SelectionInfo	*ret_data)
#endif /* _NO_PROTO */
{
    Unit   maxY;
    Unit   botY;
    int    i;
    int    lineCnt  = 0;
    int    junk;
    int    result   = 0;
    int    cpyCnt   = 0;
    int    txtCnt   = canvas->txt_cnt;
    Flags      endFlag;
    CEBoolean  processing = True;
    CELines   *lines = canvas->txt_lst;

    for (i = 0; i < txtCnt; i++)
	ClearProcessed(lines[i]);

    MarkLinesOutsideBoundary(canvas, next.y, next.x, end.y, end.x);

    maxY = next.y;
    if (next.line_idx == -1)
      {
	/*
	 * find the first selected line
	 */
	if (FindNextMinY(lines, txtCnt, -1, &next.y) == False)
	    return 0;

	next.x  = 0;
	lineCnt = FindMinX(lines, txtCnt, next.y, &next.line_idx);
	next.char_idx = 0;
      }
    else
	lineCnt = FindMinX(lines, txtCnt, next.y, &junk);

    while (processing == True && result == 0)
      {
	/*
	 * process the next line of text.
	 */
	do
	  {
	    endFlag = 0;
	    cpyCnt  = lines[next.line_idx].length - next.char_idx;
	    if (next.line_idx == end.line_idx)
		cpyCnt = cpyCnt - lines[next.line_idx].length + end.char_idx;
	    else if (lineCnt == 1)
		endFlag = CEEndOfLine;

	    result  = BuildLine(canvas, maxY, next.x,
					next.line_idx, next.char_idx,
					cpyCnt, endFlag,
					&next.x, ret_data);
	    botY = lines[next.line_idx].baseline + lines[next.line_idx].descent;
	    if (botY > maxY)
		maxY = botY;

	    next.char_idx = 0;
	    lineCnt       = FindMinX(lines, txtCnt, next.y, &next.line_idx);

	  } while (result == 0 && lineCnt > 0);

	if (result == 0)
	  {
	    next.x = 0;
	    processing = FindNextMinY(lines, txtCnt, -1, &next.y);
	    if (processing == True)
		lineCnt = FindMinX(lines, txtCnt, next.y, &next.line_idx);
	  }
      }

    return result;

} /* End GetSelectedText */

/******************************************************************************
 *                             Semi-Public Functions
 ******************************************************************************/
/*****************************************************************************
 * Function:	_DtHelpCeDrawSelection()
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDrawSelection (canvas, y1, y2)
    CECanvasStruct	*canvas;
    Unit		 y1;
    Unit		 y2;
#else
_DtHelpCeDrawSelection (
    CECanvasStruct	*canvas,
    Unit		 y1,
    Unit		 y2)
#endif /* _NO_PROTO */
{
    DtHelpCeSelectStruct start = canvas->select_start;
    DtHelpCeSelectStruct end   = canvas->select_end;

    CheckAndSwitchPoints(&start, &end);
    SetSelected (canvas, start, end, True);
}

/******************************************************************************
 *                             Public Functions
 ******************************************************************************/
/*****************************************************************************
 * Function:	CEGetSelectedData()
 *
 * Purpose:	Indicate the end point for a selection.
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpCeGetSelectionData (canvas_handle, type, ret_select)
    CanvasHandle	 canvas_handle;
    int			 type;
    SelectionInfo	*ret_select;
#else
_DtHelpCeGetSelectionData (
    CanvasHandle	 canvas_handle,
    int			 type,
    SelectionInfo	*ret_select)
#endif /* _NO_PROTO */
{
    CECanvasStruct     *canvas = (CECanvasStruct *) canvas_handle;

    *ret_select = NULL;
    return(GetSelectedText(canvas, canvas->select_start,
					canvas->select_end, ret_select));
}

/*****************************************************************************
 * Function:	_DtHelpCeProcessSelection()
 *
 * Purpose:	Indicate an new point for a selection.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeProcessSelection (canvas_handle, x, y, mode)
    CanvasHandle	canvas_handle;
    Unit		x;
    Unit		y;
    CEBoolean		mode;
#else
_DtHelpCeProcessSelection (
    CanvasHandle	canvas_handle,
    Unit		x,
    Unit		y,
    CEBoolean		mode)
#endif /* _NO_PROTO */
{
    int			 i;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    DtHelpCeSelectStruct temp;

    switch (mode)
      {
	case CESelectionClear:
		CheckAndSwitchPoints(&(canvas->select_start),
							&(canvas->select_end));

	case CESelectionStart:
		SetSelected(canvas, canvas->select_start,
						canvas->select_end, False);

		canvas->select_start = defaultSelect;
		if (mode == CESelectionStart)
		  {
		    if (canvas->max_x_cnt < canvas->txt_cnt)
		      {
			if (canvas->max_x_cnt == 0)
			    canvas->max_x_lst = (Unit *) malloc(
						sizeof(Unit) * canvas->txt_cnt);
			else
			    canvas->max_x_lst = (Unit *) realloc(
						canvas->max_x_lst,
						sizeof(Unit) * canvas->txt_cnt);
			if (canvas->max_x_lst == NULL)
			  {
			    canvas->select_end = canvas->select_start;
			    canvas->max_x_cnt  = 0;
			    return;
			  }
			canvas->max_x_cnt = canvas->txt_cnt;
		      }

		    for (i = 0; i < canvas->txt_cnt; i++)
			canvas->max_x_lst[i] = MaxXOfLine(canvas,
							canvas->txt_lst[i]);

		    SearchForClosestLine(canvas, x, y, &(canvas->select_start));
		  }

		canvas->select_end   = canvas->select_start;
		break;

	case CESelectionEnd:
	case CESelectionUpdate:
		SearchForClosestLine(canvas, x, y, &temp);

		AdjustSelection (canvas, temp);
		if (mode == CESelectionEnd)
		    CheckAndSwitchPoints(&(canvas->select_start),
							&(canvas->select_end));
		break;
      }
}
