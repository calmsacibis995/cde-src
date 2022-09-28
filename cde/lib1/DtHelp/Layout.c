/* $XConsortium: Layout.c /main/cde1_maint/1 1995/07/17 17:43:40 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Layout.c
 **
 **   Project:     Cde DtHelp
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
#include "CESegmentI.h"
#include "CanvasI.h"
#include "LayoutSDLI.h"
#include "LayoutUtilI.h"
#include "VirtFuncsI.h"

#ifdef NLS16
#include <nl_types.h>
#endif

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/
#define	JUSTIFY_CENTER	CE_CAPTION_CENTER
#define	JUSTIFY_LEFT	CE_CAPTION_LEFT
#define	JUSTIFY_RIGHT	CE_CAPTION_RIGHT

#define	GROW_SIZE	10

#define	CheckAddToHyperList(a, b) \
	_DtHelpCeCheckAddToHyperList(a, b, &(layout->info.lst_vis), \
			&(layout->info.lst_hyper), &(layout->info.cur_len))

#define	SaveInfo(canvas, layout, max_width, rmargin) \
	_DtHelpCeSaveInfo (canvas, &(layout->info), max_width, \
					rmargin, SdlJustifyLeft)

typedef	struct	{
    Unit		 id_Ypos;
    Unit		 graph_Xpos;
    unsigned int	 cur_start;
    CESegment		*cur_seg;
    void		*target_seg;
    CELayoutInfo	 info;
} LayoutInfo;

/*****************************************************************************
 *		Private Function Declarations
 *****************************************************************************/
#ifdef _NO_PROTO
static	int		AdjustGraphicPosition ();
static	int		CalcCharOffset ();
static	void		CCDFCanvasLayout ();
static	void		CheckSaveInfo ();
static	CEBoolean	CreateTables ();
static	int		JustifyText();
static	void		LineInfoClearAndSet();
static	void		LoadGraphic ();
static	CEBoolean	ProcessCaption();
static	CEBoolean	ProcessJustified();
static	short		ProcessStringSegment();
static	void		SaveGraphic ();
#else
static	int		AdjustGraphicPosition (
				int		 max_width,
				int		 graphic_width,
				int		 left,
				int		 right,
				int		 pos_type );
static	int		CalcCharOffset (
				Unit		 average,
				int		 number );
static	void		CCDFCanvasLayout (
				CanvasHandle	 canvas_handle,
				LayoutInfo	*layout);
static	void		CheckSaveInfo (
				CECanvasStruct *canvas,
				LayoutInfo	*layout,
				Unit		 max_width,
				Unit		 rmargin);
static	CEBoolean	CreateTables(
				CECanvasStruct	*canvas,
				LayoutInfo	*layout,
				int		 maxWidth,
				int		 findent,
				int		 lmargin,
				int		 rmargin,
				int		 gspace);
static	int		JustifyText(
				int	  justify_type,
				Unit	  leftPos,
				Unit	  width,
				Unit	  textWidth );
static	void		LineInfoClearAndSet(
				LayoutInfo	*layout,
				int          x_pos,
				CESegment  *segment_ptr,
				int          byte_index );
static	void		LoadGraphic (
				CECanvasStruct	*canvas,
				LayoutInfo	*layout,
				CESegment	*segment );
static	CEBoolean	ProcessCaption(
				CECanvasStruct *canvas,
				LayoutInfo	*layout,
				Unit	  maxWidth,
				Unit	  left,
				Unit	  right );
static	CEBoolean	ProcessJustified(
				CECanvasStruct   *canvas,
				LayoutInfo	*layout,
				CEGraphics *pGS,
				int	  graphicX,
				int	  maxWidth,
				int	  findent,
				int	  lmargin,
				int	  rmargin,
				int	  gspace);
static	short		ProcessStringSegment(
				CECanvasStruct   *canvas,
				LayoutInfo	*layout,
				int	  work_width,
				int	  max_width,
				int	  left,
				int	  right,
				int	  justify);
static	void		SaveGraphic (
				CECanvasStruct   *canvas,
				LayoutInfo	*layout,
				CESegment	*segment);
#endif /* _NO_PROTO */

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
 * Function:    void SetLineInfo (int x_pos, CESegment *segment,
 *                                int byte_index)
 *
 * Parameters:
 *
 * Returns:     Nothing.
 *
 *****************************************************************************/
static  void
#ifdef _NO_PROTO
LineInfoClearAndSet (layout, x_pos, segment_ptr, byte_index)
    LayoutInfo	*layout;
    int          x_pos;
    CESegment  *segment_ptr;
    int          byte_index;
#else
LineInfoClearAndSet (
    LayoutInfo	*layout,
    int          x_pos,
    CESegment  *segment_ptr,
    int          byte_index )
#endif /* _NO_PROTO */
{
    layout->cur_seg   = segment_ptr;
    layout->cur_start = byte_index;

    layout->info.text_x_pos  = x_pos;
    layout->info.label_x_pos = x_pos;

    layout->info.line_seg    = segment_ptr;
    layout->info.line_start  = byte_index;

    layout->info.line_bytes = 0;
    layout->info.cur_len    = 0;
}

/******************************************************************************
 * Function:	AdjustGraphicPosition
 *
 * Returns:	The layout->info.text_x_pos based on the positioning information.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
AdjustGraphicPosition (max_width, graphic_width, left, right, pos_type)
    int    max_width;
    int    graphic_width;
    int    left;
    int    right;
    int    pos_type;
#else
AdjustGraphicPosition (
    int    max_width,
    int    graphic_width,
    int    left,
    int    right,
    int    pos_type )
#endif /* _NO_PROTO */
{
    switch (pos_type)
      {
	case CE_AL_RIGHT:
		return (max_width - right - graphic_width);
	case CE_AL_CENTER:
		return ((left + max_width - right - graphic_width) / 2);
      }

    /*
     * this is also for CE_AL_LEFT
     */
    return left;
}

/******************************************************************************
 * Function: CalcCharOffset
 *
 * Calculates an Offset based on the average width of a character and the
 * number of characters desired.
 *****************************************************************************/
static int
#ifdef _NO_PROTO
CalcCharOffset (average, number)
    Unit	average;
    int		number;
#else
CalcCharOffset (
    Unit	average,
    int		number)
#endif /* _NO_PROTO */
{
/*
    return ((average * number) / 10 + (((charWidth * number) % 10) ? 1 : 0));
*/
    return (average * number);
}

/******************************************************************************
 * Function: CheckSaveInfo
 *
 * Initializes a line table element to the segment it should display only if
 * there is information in the buffer.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
CheckSaveInfo (canvas, layout, max_width, r_margin)
    CECanvasStruct   *canvas;
    LayoutInfo	*layout;
    Unit		 max_width;
    Unit		 r_margin;
#else
CheckSaveInfo (
    CECanvasStruct	*canvas,
    LayoutInfo		*layout,
    Unit		 max_width,
    Unit		 r_margin)
#endif /* _NO_PROTO */
{
    if (layout->info.line_bytes || layout->info.gr_cnt)
	SaveInfo (canvas, layout, max_width, r_margin);
}

/******************************************************************************
 * Function: LoadGraphic
 *
 * Initializes a graphic table element to the graphic it should display
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
LoadGraphic (canvas, layout, type)
    CECanvasStruct	*canvas;
    LayoutInfo	*layout;
    CESegment	*segment;
#else
LoadGraphic (
    CECanvasStruct	*canvas,
    LayoutInfo	*layout,
    CESegment	*segment)
#endif /* _NO_PROTO */
{
    int  result = -1;

    if (_CEIsNotInitialized(segment->seg_type))
      {
        char  *fileName = segment->seg_handle.file_handle;

	if (canvas->virt_functions._CELoadGraphic != NULL)
	  {
            int    index    = _DtHelpCeAllocGraphicStruct(canvas);

	    if (index >= 0)
	      {
	        CEGraphics	*pGr = _CEGraphicStructPtr(canvas, index);
	        result = (*(canvas->virt_functions._CELoadGraphic))(
				canvas->client_data,
				NULL, NULL,
				fileName,
				NULL, NULL,
				&(pGr->width), &(pGr->height), &(pGr->handle));
	        if (result != 0)
		    canvas->graphic_cnt--;
	        else
	          {
		    _CEGraphicStructType(canvas, index) =
				_CESetInitialized(segment->seg_type);
		    _CEGraphicLnk(canvas, index) = segment->link_index;
		    segment->seg_handle.grph_idx = index;

		    if (_CEGetGraphicType(segment->seg_type) == CE_IN_LINE)
		        _DtHelpCeAddToInLineList (&(layout->info), index);
	          }
	      }
	  }

	if (result != 0)
	    segment->seg_type = _CESetNoop(segment->seg_type);

	segment->seg_type = _CESetInitialized(segment->seg_type);
	free (fileName);
      }
}

/******************************************************************************
 * Function: SaveGraphic
 *
 * Initializes a graphic table element to the graphic it should display and
 * the line it should display it at.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
SaveGraphic (layout, segment)
    CECanvasStruct	*canvas;
    LayoutInfo	*layout;
    CESegment	*segment;
#else
SaveGraphic (
    CECanvasStruct	*canvas,
    LayoutInfo	*layout,
    CESegment	*segment)
#endif /* _NO_PROTO */
{
    CEGraphics	*pGS = _CEGraphicStructPtr(canvas,_CEGraphicIdxOfSeg(segment));

    pGS->pos_y  = layout->info.y_pos;
    pGS->pos_x  = layout->graph_Xpos;

}

/******************************************************************************
 * Function: JustifyText
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
static int
#ifdef _NO_PROTO
JustifyText(justify_type, leftPos, width, textWidth)
    int	  justify_type;
    Unit	  leftPos;
    Unit	  width;
    Unit	  textWidth;
#else
JustifyText(
    int	  justify_type,
    Unit	  leftPos,
    Unit	  width,
    Unit	  textWidth )
#endif /* _NO_PROTO */
{
    Unit  result = 0;

    switch (justify_type)
      {
	case JUSTIFY_CENTER:
		result = (width - textWidth ) / 2;
		break;

	case JUSTIFY_RIGHT:
		result = width - textWidth;
		break;
      }

    result += leftPos;

    if (result < leftPos)
	result = leftPos;

    return result;
}

/******************************************************************************
 * Function: ProcessJustified
 *
 * Processes the tables for a left or right justified graphic.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ProcessJustified(canvas, layout, pGS, graphicX, maxWidth,
		 findent,lmargin,rmargin, gspace)
    CECanvasStruct *canvas,
    LayoutInfo	*layout;
    CEGraphics *pGS;
    int	  graphicX;
    int	  maxWidth;
    int	  findent;
    int	  lmargin;
    int	  rmargin;
    int	  gspace;
#else
ProcessJustified(
    CECanvasStruct *canvas,
    LayoutInfo	*layout,
    CEGraphics *pGS,
    int	  graphicX,
    int	  maxWidth,
    int	  findent,
    int	  lmargin,
    int	  rmargin,
    int	  gspace)
#endif /* _NO_PROTO */
{
    int     newLine;
    int     gLine;
    int     diff;
    int     oldLines;
    CEBoolean terminatorFound;

    /*
     * save the current information.
     */
    CheckSaveInfo (canvas, layout, maxWidth, rmargin);
 
    /*
     * save the graphic
     */
    layout->graph_Xpos = graphicX;
    SaveGraphic (canvas, layout, layout->cur_seg);
    gLine = _CEGraphicIdxOfSeg(layout->cur_seg);
   
    /*
     * check to see if it exceeds our maximum X
     */
    if (layout->graph_Xpos + pGS->width > layout->info.max_x_pos)
	layout->info.max_x_pos = layout->graph_Xpos + pGS->width;

    /*
     * If we have a new line on the graphic,
     * save a blank line to the side.
     */
    newLine = _CEIsNewLine(layout->cur_seg->seg_type);
    if (newLine)
	SaveInfo (canvas, layout, maxWidth, rmargin);

    /*
     * save some flags
     */
    terminatorFound = (layout->cur_seg->next_disp == NULL ? 1 : 0);

    /*
     * save the old number of
     * lines to format
     */
    oldLines    = layout->info.format_y;
    layout->info.format_y = layout->info.y_pos + pGS->height;

    if (!terminatorFound)
      {
        layout->cur_seg = layout->cur_seg->next_disp;
	LineInfoClearAndSet (layout, findent, layout->cur_seg, 0);

        /*
         * format lines to the side
	 * of the graphic.
         */
        terminatorFound = CreateTables(canvas, layout, maxWidth, findent,
						lmargin, rmargin, gspace);
      }
    /*
     * Adjust the graphic within the bracket of the text
     */
    diff = (layout->info.y_pos - canvas->graphic_lst[gLine].pos_y - pGS->height) / 2;
    if (diff > 0)
        canvas->graphic_lst[gLine].pos_y += diff;

    /*
     * reset the layout->info.format_y indicator.
     */
    layout->info.format_y = oldLines;

    return terminatorFound;

} /* End ProcessJustified */

/******************************************************************************
 * Function: ProcessStringSegment
 *
 * chops a string segment up until its completely used.
 *
 * Returns:
 *	0	if the entire string segment was processed.
 *	1	if the terminator was set.
 *	2	if the required number of lines were processed.
 *	3	if the entire string and the required number of lines.
 *	4	if the entire string, the required number of lines & terminator
 *****************************************************************************/
static short
#ifdef _NO_PROTO
ProcessStringSegment(canvas, layout, work_width, max_width, left, right, justify)
    CECanvasStruct *canvas,
    LayoutInfo	*layout;
    int	  work_width;
    int	  max_width;
    int	  left;
    int	  right;
    int	  justify;
#else
ProcessStringSegment(
    CECanvasStruct *canvas,
    LayoutInfo	*layout,
    int	  work_width,
    int	  max_width,
    int	  left,
    int	  right,
    int	  justify )
#endif /* _NO_PROTO */
{
    int	   stringLen;
    int	   textWidth;
    int	   nWidth;
    int	   spaceSize = 0;
    int	   oldType;
    int	   retStart;
    int	   retCount;
    char	 *pChar;
    CEBoolean       done;
    CEBoolean       bigDone    = False;
    CEBoolean       terminate  = False;
    CEBoolean       formatDone = False;
    CESegment   *retSeg;
    CESegment   *tmpSeg;

    if (layout->cur_seg->seg_handle.generic)
      {
	oldType = _CEGetPrimaryType (layout->cur_seg->seg_type);

	while (!bigDone && !formatDone)
	  {
	    /*
	     * adjust the character pointer and get the
	     * length of the string.
	     */
	    pChar = _CEStringOfSegment(layout->cur_seg) + layout->cur_start;
	    stringLen = strlen (pChar);
    
	    /*
	     * get the pixel width of the text string.
	     */
	    textWidth = _DtHelpCeGetStringWidth (canvas, layout->cur_seg,
					NULL, NULL, NULL,
					pChar, stringLen) +
			+ _DtHelpCeGetTraversalWidth(canvas, layout->cur_seg, layout->info.lst_hyper);

	    /*
	     * Will it fit in the current width?
	     */
	    if (textWidth <= work_width)
	      {
		/*
		 * Yes, this segment or part of a segment can fit in the
		 * current width. But can the last character of this
		 * segment end a line and can the beginning of the next
		 * segment start a new line?
		 */
		if (_DtHelpCeCheckLineSyntax (canvas, layout->cur_seg,
					layout->cur_start, stringLen) == TRUE)
		  {
	            /*
	             * check to see if this a hypertext that needs
	             * to be remembered.
	             */
	            CheckAddToHyperList (canvas, layout->cur_seg);

		    /*
		     * The line syntax is good.
		     * Update the global and width variables.
		     */
		    work_width  -= textWidth;
		    layout->info.line_bytes += stringLen;
		    layout->info.cur_len  += textWidth;
    
		    /*
		     * remember if this is the terminator to the paragraph
		     * since we increment past it later.
		     */
		    if (layout->cur_seg->next_disp == NULL)
			terminate = True;
    
		    /*
		     * Check to see if this segment forces an end
		     */
		    if (_CEIsNewLine (layout->cur_seg->seg_type))
		      {
			/*
			 * Adjust the location of the text if need be.
			 */
			layout->info.text_x_pos = JustifyText (justify,
						layout->info.text_x_pos,
						(max_width - left - right),
						layout->info.cur_len);
			/*
			 * save.
			 */
			CheckSaveInfo (canvas, layout, max_width, right);
    
			/*
			 * Initialize the global line variables.
			 */
			LineInfoClearAndSet (layout, left,
						(layout->cur_seg + 1), 0);
		      }
		    else
		      {
		        /*
		         * increment the segment variables.
		         */
		        layout->cur_seg = layout->cur_seg->next_disp;
		        layout->cur_start = 0;
		      }

		    if (layout->info.format_y != _CEFORMAT_ALL &&
					layout->info.y_pos >= layout->info.format_y)
			formatDone = True;

		    return (terminate + formatDone * 2);
		  }

		/*
		 * CheckLineSyntax says that either this line couldn't
		 * end a line or the next segment couldn't start a line.
		 * Therefore, find out how much of the next segment or
		 * segments we need to incorporate to satisfy the Line
		 * Syntax rules.
		 */
		nWidth = _DtHelpCeGetNextWidth (canvas, oldType,
					layout->info.lst_hyper,
					(layout->cur_seg + 1), 0,
/*
 * Note...The lang, charset and font_spec parameters to this call are
 * NULL (and can be) because the font *has already* been resolved
 * when read in.
 */
					NULL, NULL, NULL,
					NULL, NULL, NULL,
					&retSeg, &retStart, &retCount);
		/*
		 * will this segment + the next segment fit?
		 */
		if (textWidth + nWidth <= work_width)
		  {
	            /*
	             * check to see if this a hypertext that needs
	             * to be remembered.
	             */
	            CheckAddToHyperList (canvas, layout->cur_seg);

		    /*
		     * YEAH Team!! It Fits!!
		     *
		     * Update the global and width variables.
		     */
		    work_width  = work_width - textWidth - nWidth;
		    layout->info.line_bytes = layout->info.line_bytes
							+ stringLen + retCount;
		    layout->info.cur_len  = layout->info.cur_len
							+ textWidth + nWidth;
    
		    /*
		     * check the hypertext links inbetween
		     */
		    if (layout->cur_seg != retSeg)
		      {
		        tmpSeg = layout->cur_seg;
		        do
		          {
		            tmpSeg = tmpSeg->next_disp;
			    CheckAddToHyperList (canvas, tmpSeg);
		          } while (tmpSeg != retSeg);
		      }

		    /*
		     * Check to see if this exceeds our maximum X
		     */
		    layout->cur_seg = retSeg;
		    layout->cur_start = retStart;
    
		    if (_CEGetPrimaryType (layout->cur_seg->seg_type)
								== oldType)
		      {
			/*
			 * find out if the returned segment is at its end
			 */
			pChar = _CEStringOfSegment(layout->cur_seg);
			stringLen = strlen (pChar);
			if (stringLen == layout->cur_start)
			  {
			    /*
			     * At the end of the segment,
			     * check for newline and termination
			     */
		            if (layout->cur_seg->next_disp == NULL)
				terminate = True;
    
			    /*
			     * Check to see if this segment forces an end
			     */
			    if (_CEIsNewLine (layout->cur_seg->seg_type))
			      {
				/*
				 * Adjust the location of the text if need be.
				 */
				layout->info.text_x_pos = JustifyText (justify,
						layout->info.text_x_pos,
						(max_width - left - right),
						layout->info.cur_len);
				CheckSaveInfo (canvas, layout, max_width, right);
    
				/*
				 * Initialize the global line variables.
				 */
			        LineInfoClearAndSet (layout, left,
						(layout->cur_seg + 1), 0);
			      }
			    else
			      {
			        /*
			         * go to the next segment
			         */
		                layout->cur_seg = layout->cur_seg->next_disp;
			        layout->cur_start = 0;
			      }
			  }
		      }
		    if (layout->info.format_y != _CEFORMAT_ALL && layout->info.y_pos >= layout->info.format_y)
			formatDone = True;

		    return (terminate + formatDone * 2);
		  }
	      }
    
	    /*
	     * the text width plus the next segment is too big
	     * to fit. Reduce the current segment if possible
	     */
	    done = False;
	    textWidth = 0;
	    stringLen = 0;
	    while (!done)
	      {
		nWidth = _DtHelpCeGetNextWidth (canvas, oldType,
					layout->info.lst_hyper,
					layout->cur_seg, layout->cur_start,
/*
 * Note...The lang, charset and font_spec parameters to this call are
 * NULL (and can be) because the font *has already* been resolved
 * when read in.
 */
					NULL, NULL, NULL,
					NULL, NULL, NULL,
					&retSeg, &retStart, &retCount);
		if (textWidth + nWidth <= work_width)
		  {
	             /*
	              * check to see if this a hypertext that needs
	              * to be remembered.
	              */
	             CheckAddToHyperList (canvas, layout->cur_seg);

		    layout->cur_seg   = retSeg;
		    layout->cur_start   = retStart;
		    stringLen += retCount;
		    textWidth += nWidth;
		    spaceSize = 0;
		    if (_CEGetCharLen(layout->cur_seg->seg_type) == 1)
		      {
			pChar = _CEStringOfSegment(layout->cur_seg)
							+ layout->cur_start;
			if (*pChar == ' ')
			  {
			    spaceSize = _DtHelpCeGetStringWidth(canvas, layout->cur_seg,
							NULL, NULL, NULL,
							pChar, 1)
					+ _DtHelpCeGetTraversalWidth (canvas,
							layout->cur_seg,
							layout->info.lst_hyper);
			    textWidth += spaceSize;
			    stringLen++;
			    layout->cur_start++;
			  }
		      }
		  }
		else
		  {
		    /*
		     * Done trying to find a segment that will
		     * fit in the size given
		     */
		    done = True;
		  }
	      }

	    /*
	     * Update the global variables
	     */
	    layout->info.line_bytes += stringLen;
	    layout->info.cur_len  += textWidth;

	    if (!textWidth && !layout->info.line_bytes)
	      {
	        /*
	         * check to see if this a hypertext that needs
	         * to be remembered.
	         */
	        CheckAddToHyperList (canvas, layout->cur_seg);

		/*
		 * Couldn't find a smaller, have to
		 * go with the larger segment.
		 */
		layout->info.line_bytes += retCount;
		layout->info.cur_len  += nWidth;

		/*
		 * check the hypertext links inbetween
		 */
		if (layout->cur_seg != retSeg)
		  {
		    tmpSeg = layout->cur_seg;
		    do
		      {
		        tmpSeg = tmpSeg->next_disp;
		        CheckAddToHyperList (canvas, tmpSeg);
		      } while (tmpSeg != retSeg);
		  }

		/*
		 * Going with the larger segment,
		 * update the global pointers correctly
		 */
		layout->cur_seg = retSeg;
		layout->cur_start = retStart;

                /*
                 * if the next segment to process is a no-op or
                 * at the end of the current string segment, check for
                 * the terminator flag.
                 */
                if (
                    (_CEIsNoop (layout->cur_seg->seg_type) ||
                        (
                            _CEIsString (layout->cur_seg->seg_type)
                                &&
            strlen(_CEStringOfSegment(layout->cur_seg)+layout->cur_start) == 0
                        )
                     ) && layout->cur_seg->next_disp == NULL)
                    terminate = True;

		/*
		 * If we had to do a bigger segment,
		 * then we're done processing the target segment.
		 */
		bigDone = True;
	      }
	    else if (spaceSize)
	      {
		/*
		 * If a space was included as the last character,
		 * remove it now.
		 */
		layout->info.line_bytes--;
		layout->info.cur_len -= spaceSize;
	      }

	    /*
	     * Adjust the location of the text if need be.
	     */
	    layout->info.text_x_pos = JustifyText (justify, layout->info.text_x_pos,
				(max_width - left - right), layout->info.cur_len);

	    /*
	     * Save the information
	     */
	    SaveInfo (canvas, layout, max_width, right);

	    /*
	     * Skip the spaces.
	     */
	    if (_CEGetPrimaryType (layout->cur_seg->seg_type) == oldType &&
		_CEIsNotStaticString (layout->cur_seg->seg_type)    &&
		_CEGetCharLen (layout->cur_seg->seg_type) == 1)
	      {
		/*
		 * skip spaces
		 */
		pChar = _CEStringOfSegment(layout->cur_seg) + layout->cur_start;
		while (*pChar == ' ')
		  {
		    pChar++;
		    layout->cur_start++;
		  }

		/*
		 * are we at the end of the segment?
		 */
		if (*pChar == '\0')
		  {
		    /*
		     * Done with this segment, check for termination
		     */
		    layout->cur_seg = layout->cur_seg->next_disp;
		    if (layout->cur_seg == NULL)
			terminate = True;

		    layout->cur_start = 0;
		    bigDone  = True;
		  }
	      }


	    /*
	     * Initialize the global variables
	     */
	    LineInfoClearAndSet (layout, left, layout->cur_seg, layout->cur_start);

	    /*
	     * reset our working width
	     */
	    work_width = max_width - left - right;

	    /*
	     * find out if we need more lines
	     */
	    if (layout->info.format_y != _CEFORMAT_ALL &&
				layout->info.y_pos >= layout->info.format_y)
		formatDone = True;
	  }
      }
    else if (_CEIsNewLine (layout->cur_seg->seg_type))
      {
	/*
	 * Adjust the location of the text if need be.
	 */
	layout->info.text_x_pos = JustifyText (justify, layout->info.text_x_pos,
				(max_width - left - right), layout->info.cur_len);

	/*
	 * Force a save - even if it is an empty line.
	 */
	SaveInfo (canvas, layout, max_width, right);

	/*
	 * remember if this is the terminator to the paragraph
	 * since we increment past it later.
	 */
	if (layout->cur_seg->next_disp == NULL)
	    terminate = True;

	/*
	 * Initialize the global line variables.
	 */
	LineInfoClearAndSet (layout, left, (layout->cur_seg + 1), 0);
      }
    return (terminate + formatDone * 2);

} /* End ProcessStringSegment */

/******************************************************************************
 * Function: ProcessCaption
 *
 * Since the caption is above the figure, we have to look ahead for the
 * figure information.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ProcessCaption(canvas, layout, maxWidth, left, right)
    CECanvasStruct *canvas;
    LayoutInfo	*layout;
    Unit	  maxWidth;
    Unit	  left;
    Unit	  right;
#else
ProcessCaption(
    CECanvasStruct *canvas,
    LayoutInfo	*layout,
    Unit	  maxWidth,
    Unit	  left,
    Unit	  right )
#endif /* _NO_PROTO */
{
    int		      saveLinePos = layout->info.text_x_pos;
    int               capPos;
    short	      flag = 0;

    layout->info.text_x_pos = left;

    capPos = _CEGetCaptionPosition(layout->cur_seg->seg_type);
    while (_CEIsCaption(layout->cur_seg->seg_type) && flag != 1 && flag != 3)
      {
	CheckAddToHyperList (canvas, layout->cur_seg);
	flag = ProcessStringSegment (canvas, layout, (maxWidth - left - right),
				maxWidth, layout->info.text_x_pos, right, capPos);
      }

    if (layout->info.line_bytes)
      {
	layout->info.text_x_pos = JustifyText (capPos, layout->info.text_x_pos,
					(maxWidth - left - right),
					layout->info.cur_len);
	SaveInfo (canvas, layout, maxWidth, right);
      }

    layout->info.text_x_pos = saveLinePos;

    /*
     * return the termination flag
     */
    if (flag > 1)
	flag -= 2;

    return flag;

} /* End ProcessCaption */

/******************************************************************************
 * Function: CreateTables
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
CreateTables(canvas, layout, maxWidth, findent, lmargin, rmargin, gspace)
    CECanvasStruct   *canvas;
    LayoutInfo	*layout;
    int	  maxWidth;
    int	  findent;
    int	  lmargin;
    int	  rmargin;
    int	  gspace;
#else
CreateTables(
    CECanvasStruct   *canvas,
    LayoutInfo	*layout,
    int	  maxWidth,
    int	  findent,
    int	  lmargin,
    int	  rmargin,
    int	  gspace )
#endif /* _NO_PROTO */
{
    Unit   width;
    Unit   textWidth;
    Unit   scount;
    Unit   tlen;
    int    newLine;
    int    type;
    int    oldY;
    char  *cPtr;
    CEGraphics  *pGS;
    CEBoolean terminatorFound = False;

    width = maxWidth - layout->info.text_x_pos - rmargin;

    while (!terminatorFound)
      {
        /*
         * check to see if this segment is the segment we want as our first
         * visible line.
         */
        if (layout->cur_start == 0 && layout->target_seg && layout->cur_seg == layout->target_seg)
            layout->id_Ypos = layout->info.y_pos;

	/*
	 * look at the primary attribute.
	 *   It should be _XvhSTRING, XvhGRAPHIC, _XvhPARAGRAPH, or CE_NOOP
	 */
	switch (_CEGetPrimaryType (layout->cur_seg->seg_type))
          {
	    case CE_LABEL:
		    /*
		     * If there is any information in the 'buffer'
		     * save it.
		     */
		    CheckSaveInfo (canvas, layout, maxWidth, rmargin);

		    /*
		     * If we were formatting lines next to a justified
		     * graphic, layout->info.format_y will be zero
		     * when we are done.
		     * If we are done, return to the caller. Since we
		     * haven't updated the segment ptr, it will still be
		     * pointing at this label and the caller should
		     * process it correctly.
		     */
	    	    if (layout->info.format_y != _CEFORMAT_ALL &&
				layout->info.y_pos >= layout->info.format_y)
			return False;

		    /*
		     * Need to save this label away.
		     * For labels, the label_x element really tells us the x
		     * pixel position of the label and text_x element will be
		     * for where the text starts after the label.
		     */
		    LineInfoClearAndSet (layout, findent, layout->cur_seg, 0);
		    textWidth  = 0;
		    if (layout->cur_seg->seg_handle.generic)
		      {
			do
			  {
			    CheckAddToHyperList (canvas, layout->cur_seg);

			    tlen = strlen (_CEStringOfSegment(layout->cur_seg));
			    layout->info.line_bytes += tlen;

			    /*
			     * If the label is too large for the
			     * left margin, place the next text
			     * two average spaces after the label.
			     */
			    textWidth +=
				_DtHelpCeGetStringWidth(canvas, layout->cur_seg, NULL,
						NULL, NULL,
					    _CEStringOfSegment(layout->cur_seg),
						tlen)
				+ _DtHelpCeGetTraversalWidth(canvas,
							layout->cur_seg,
							layout->info.lst_hyper);

			    /*
			     * check flags and go to the next segment.
			     */
			    newLine  = _CEIsNewLine (layout->cur_seg->seg_type);
			    layout->cur_seg = layout->cur_seg->next_disp;
			    if (layout->cur_seg == NULL)
				terminatorFound = True;

			  } while (!terminatorFound && !newLine &&
				    _CEIsLabel(layout->cur_seg->seg_type));

			/*
			 * check to see if it exceeds our maximum X
			 */
			if (textWidth + findent > layout->info.max_x_pos)
			    layout->info.max_x_pos = textWidth + findent;
		      }
		    else
		      {
			/*
			 * check flags and go to the next segment.
			 */
			newLine  = _CEIsNewLine (layout->cur_seg->seg_type);
			layout->cur_seg = layout->cur_seg->next_disp;
			if (layout->cur_seg == NULL)
			    terminatorFound = True;
		      }
		    layout->cur_start  = 0;

		    if (textWidth + layout->info.label_x_pos
			+ CalcCharOffset(canvas->font_info.average_width, 1)
							    > lmargin)
			/*
			 * char width is really a floating point number
			 * with tenth's value. To make it an integer, it
			 * has been multiplied by 10. Therefore tests
			 * against this value must take it into account.
			 * And beware the remainder if you divide char
			 * width.
			 */
			layout->info.text_x_pos = textWidth + layout->info.label_x_pos +
			    CalcCharOffset(canvas->font_info.average_width, 1);
		    else
			layout->info.text_x_pos = lmargin;
		    layout->info.cur_len  = 0;

		    /*
		     * update the width to whats left over after the label.
		     */
		    width = maxWidth - layout->info.text_x_pos - rmargin;

		    /*
		     * If there is no more room, put the text associated
		     * with this label on the next line.
		     */
		    if (width <= 0 || newLine)
		      {
			SaveInfo (canvas, layout, maxWidth, rmargin);

			/*
			 * Initialize to the next segment.
			 */
		        LineInfoClearAndSet(layout, lmargin,
							layout->cur_seg, 0);

			/*
			 * whoops, return to caller.
			 * We've place the 'correct'
			 * number of lines next to the graphic.
			 */
	    	    	if (layout->info.format_y != _CEFORMAT_ALL &&
					layout->info.y_pos >= layout->info.format_y)
			    return terminatorFound;

			/*
			 * starting a new line,
			 * get the total width available.
			 */
			width = maxWidth - lmargin - rmargin;
		      }
		    break;

	    case CE_STRING:
		    if (_CEIsStaticString(layout->cur_seg->seg_type))
		      {
			/*
			 * check to see if this segment is hypertext.
			 * If so, add it to the list if it hasn't been
			 * added yet.
			 */
			CheckAddToHyperList (canvas, layout->cur_seg);

			/*
			 * include the static segment.
			 * update length and layout->cur_start positions.
			 */
			cPtr = _CEStringOfSegment(layout->cur_seg);
			if (cPtr)
			  {
			    scount      = strlen (cPtr);
			    textWidth   = _DtHelpCeGetStringWidth(canvas,
						layout->cur_seg, NULL, NULL,
						NULL, cPtr, scount)
					+ _DtHelpCeGetTraversalWidth( canvas,
							layout->cur_seg,
							layout->info.lst_hyper);

			    layout->info.line_bytes += scount;
			    layout->info.cur_len  += textWidth;
			  }

			/*
			 * initialize for the next segment.
			 */
			if (layout->cur_seg->next_disp == NULL)
			    terminatorFound = True;
			/*
			 * If this is the end of the static line,
			 * save the pre-formatted line information
			 */
			if (_CEIsNewLine(layout->cur_seg->seg_type))
			  {
			    SaveInfo (canvas, layout, maxWidth, rmargin);

			    LineInfoClearAndSet (layout, lmargin,
						(layout->cur_seg + 1), 0);

			    /*
			     * starting a new line,
			     * get the total width available.
			     */
			    width = maxWidth - lmargin - rmargin;
			  }
			else
			  {
			    layout->cur_seg = layout->cur_seg->next_disp;
			    layout->cur_start = 0;
			  }

	    	    	if (layout->info.format_y != _CEFORMAT_ALL &&
					layout->info.y_pos >= layout->info.format_y)
			    return terminatorFound;
		      }
		    else
		      {
			scount = ProcessStringSegment (canvas, layout, width,
			    maxWidth, lmargin, rmargin, JUSTIFY_LEFT);

			/*
			 * if we hit the end of the segment, check
			 * for the terminator flag.
			 */
			if (scount == 1 || scount == 3)
			  {
			    terminatorFound = True;
			    scount--;
			  }

			if (scount)
			    return terminatorFound;

			width = maxWidth - layout->info.text_x_pos -
						layout->info.cur_len - rmargin;
		      }
		    break;

	    case CE_CAPTION:
		    /*
		     * save any current information.
		     */
		    CheckSaveInfo (canvas, layout, maxWidth, rmargin);

		    /*
		     * check to see if this segment is hypertext.
		     * If so, add it to the list if it hasn't been
		     * added yet.
		     */
		    CheckAddToHyperList (canvas, layout->cur_seg);

		    /*
		     * If we were formatting lines next to a justified
		     * graphic, layout->info.format_y will be zero when we are
		     * done.
		     *
		     * If we are done, return to the caller. Since we
		     * haven't updated the segment ptr, it will still be
		     * pointing at this caption and the caller should
		     * process it correctly.
		     */
	    	    if (layout->info.format_y != _CEFORMAT_ALL &&
					layout->info.y_pos >= layout->info.format_y)
			return False;
	
		    layout->info.line_seg   = layout->cur_seg;
		    layout->info.line_start = 0;
		    layout->info.line_bytes   = 0;
		    layout->info.text_x_pos  = lmargin;

		    terminatorFound = ProcessCaption (canvas, layout,
					    maxWidth, lmargin, rmargin);
		    /*
		     * initialize for the next segment
		     */
		    LineInfoClearAndSet (layout, lmargin, layout->cur_seg, 0);

		    /*
		     * starting a new line,
		     * get the total width available.
		     */
		    width = maxWidth - lmargin - rmargin;
		    break;

	    case CE_GRAPHIC:
		    if (_CEIsNotInitialized(layout->cur_seg->seg_type))
			LoadGraphic (canvas, layout, layout->cur_seg);
		    else if (_CEGetGraphicType(layout->cur_seg->seg_type)
								== CE_IN_LINE)
		        _DtHelpCeAddToInLineList (&(layout->info),
					_CEGraphicIdxOfSeg(layout->cur_seg));

		    if (_CEIsNotGraphic(layout->cur_seg->seg_type))
			break;

		    pGS  = _CEGraphicStructPtr(canvas,
					_CEGraphicIdxOfSeg(layout->cur_seg));
		    type = _CEGetGraphicType(layout->cur_seg->seg_type);

		    oldY = layout->info.y_pos + pGS->height;

		    /*
		     * get the traversal width
		     */
		    tlen = _DtHelpCeGetTraversalWidth(canvas, layout->cur_seg,
							layout->info.lst_hyper);

		    switch (type)
		      {
			case CE_IN_LINE:
				/*
				 * If the in-line graphic won't fit on
				 * this line, put it on the next.
				 */
				if (pGS->width + tlen > width)
				  {
				    CheckSaveInfo (canvas, layout,
							maxWidth, rmargin);
				    width = maxWidth - lmargin - rmargin;
				  }
	
				/*
				 * check to see if this segment is
				 * a hypertext. If so, add it to the
				 * list if it hasn't been added yet.
				 */
				CheckAddToHyperList (canvas, layout->cur_seg);
				tlen = _DtHelpCeGetTraversalWidth(canvas,
							layout->cur_seg,
							layout->info.lst_hyper);
	
				/*
				 * calculate it's x pixel position.
				 * layout->info.text_x_pos does not take into
				 * account the margin padding.
				 */
				layout->graph_Xpos = layout->info.text_x_pos
							+ layout->info.cur_len;
				SaveGraphic (canvas, layout, layout->cur_seg);
	
				/*
				 * check to see if it exceeds our maximum X
				 */
				if (layout->graph_Xpos + pGS->width + tlen
							> layout->info.max_x_pos)
				    layout->info.max_x_pos = layout->graph_Xpos
							+ pGS->width + tlen;

				/*
				 * update the width
				 */ 
				layout->info.cur_len += (pGS->width + tlen);
				width  = maxWidth - layout->info.cur_len;

				/*
				 * remember some flags
				 */
				newLine = _CEIsNewLine(layout->cur_seg->seg_type);
			        layout->cur_seg = layout->cur_seg->next_disp;
				if (layout->cur_seg == NULL)
				    terminatorFound = True;

				/*
				 * get the average character width
				 */
				tlen = canvas->font_info.average_width;
				if (width < tlen || newLine)
				  {
				    SaveInfo(canvas, layout, maxWidth, rmargin);

				    LineInfoClearAndSet (layout, lmargin,
							layout->cur_seg, 0);

				    /*
				     * starting a new line,
				     * get the total width available.
				     */
				    width = maxWidth - lmargin - rmargin;
				  }
	
				break;
	
			case CE_ALONE_CENTERED:
				/*
				 * save any current information.
				 */
				CheckSaveInfo(canvas,layout,maxWidth, rmargin);
	
				/*
				 * check to see if this segment is
				 * a hypertext. If so, add it to the
				 * list if it hasn't been added yet.
				 */
				tlen = 0;
				if (_CEIsHyperLink(layout->cur_seg->seg_type))
				    tlen = canvas->traversal_info.space_before;
				if (_CEIsVisibleLink(layout->cur_seg->seg_type))
				    tlen += canvas->link_info.space_before;

				CheckAddToHyperList (canvas, layout->cur_seg);
				/*
				 * calculate it's x pixel position.
				 */
				layout->graph_Xpos = AdjustGraphicPosition (
						    maxWidth,
						    ((int) pGS->width),
						    lmargin,
						    rmargin,
				_CEGetGraphicPosition(layout->cur_seg->seg_type)
					    );
				/*
				 * check to see it it will truly fit on the
				 * line.
				 */
				if (layout->graph_Xpos - tlen < lmargin)
				    layout->graph_Xpos = lmargin + tlen;
	
				SaveGraphic (canvas, layout, layout->cur_seg);
	
				/*
				 * check to see if it exceeds our maximum X
				 */
				tlen = _DtHelpCeGetTraversalWidth(canvas,
							layout->cur_seg,
							layout->info.lst_hyper);
				if (layout->graph_Xpos + pGS->width + tlen
							> layout->info.max_x_pos)
				    layout->info.max_x_pos = layout->graph_Xpos
							+ pGS->width + tlen;

				/*
				 * if this has a new line on it,
				 * save and extra line.
				 */
				if (_CEIsNewLine (layout->cur_seg->seg_type))
				    SaveInfo(canvas, layout, maxWidth, rmargin);

				if (layout->cur_seg->next_disp == NULL)
				    terminatorFound = True;
				else
				  {
			            layout->cur_seg   =
						layout->cur_seg->next_disp;
				    layout->cur_start = 0;
				  }

				break;
	
			case CE_LEFT_JUSTIFIED:
				/*
				 * check to see if this segment is
				 * a hypertext. If so, add it to the
				 * list if it hasn't been added yet.
				 */
				CheckAddToHyperList (canvas, layout->cur_seg);
	
				tlen += (pGS->width +
					    CalcCharOffset(
						canvas->font_info.average_width,
						gspace));

				terminatorFound = ProcessJustified (
							canvas, layout, pGS,
			(lmargin + (_CEIsHyperLink(layout->cur_seg->seg_type)
				? (canvas->traversal_info.space_before
				  + (_CEIsVisibleLink(layout->cur_seg->seg_type)
					? canvas->link_info.space_before : 0))
				: 0)),
							maxWidth,
							(tlen + findent),
							(tlen + lmargin),
							rmargin,
							gspace);

				break;
	
			case CE_RIGHT_JUSTIFIED:
				/*
				 * check to see if this segment is
				 * a hypertext. If so, add it to the
				 * list if it hasn't been added yet.
				 */
				CheckAddToHyperList (canvas, layout->cur_seg);
	
				tlen += (pGS->width +
					    CalcCharOffset(
						canvas->font_info.average_width,
						gspace));

				terminatorFound = ProcessJustified (
							canvas, layout, pGS,
		(maxWidth - rmargin - pGS->width -
			(_CEIsHyperLink(layout->cur_seg->seg_type) ?
				(canvas->traversal_info.space_after
				  + (_CEIsVisibleLink(layout->cur_seg->seg_type)
					? canvas->link_info.space_after : 0))
				: 0)),
							maxWidth, findent,
							lmargin,
							(rmargin + tlen),
							gspace);

				break;
		      }

		    /*
		     * initialize where the next line starts,
		     * its margin and lengths
		     */
		    if (type != CE_IN_LINE)
		      {
			LineInfoClearAndSet (layout, lmargin,
					layout->cur_seg, layout->cur_start);
			if (oldY > layout->info.y_pos)
			    layout->info.y_pos = oldY;

			/*
			 * starting a new line,
			 * get the total width available.
			 */
			width = maxWidth - lmargin - rmargin;
		      }

		    break;

	    case CE_NOOP:
		    if (_CEIsNewLine(layout->cur_seg->seg_type))
		      {
			SaveInfo (canvas, layout, maxWidth, rmargin);
			LineInfoClearAndSet (layout, lmargin,
					layout->cur_seg->next_disp, 0);

			/*
			 * starting a new line,
			 * get the total width available.
			 */
			width = maxWidth - lmargin - rmargin;
		      }
		    else
		        layout->cur_seg = layout->cur_seg->next_disp;
		    if (layout->cur_seg == NULL)
			terminatorFound = True;
		    break;

	    default:
		    layout->cur_seg = layout->cur_seg->next_disp;
		    if (layout->cur_seg == NULL)
			terminatorFound = True;
	  }
      }
    CheckSaveInfo (canvas, layout, maxWidth, rmargin);
    return terminatorFound;
}

/*****************************************************************************
 * Function:    static void CCDFCanvasLayout (CanvasHandle canvas);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 * 
 *****************************************************************************/
static void
#ifdef _NO_PROTO
CCDFCanvasLayout (canvas_handle, layout)
    CanvasHandle	 canvas_handle;
    LayoutInfo		*layout;
#else
CCDFCanvasLayout (
    CanvasHandle	 canvas_handle,
    LayoutInfo		*layout)
#endif /* _NO_PROTO */
{
    int			 count;
    Unit		 dispWidth;
    Unit		 charWidth;
    Unit		 sideMargin;
    CECanvasStruct	*canvas      = (CECanvasStruct *) canvas_handle;
    CEParagraph		*paraList;

    count    = canvas->para_count;
    paraList = canvas->para_lst;

    dispWidth           = canvas->metrics.width;
    layout->info.y_pos  = canvas->metrics.top_margin;
    sideMargin          = canvas->metrics.side_margin;
    charWidth           = canvas->font_info.average_width;

    layout->info.max_x_pos = 0;
    layout->info.lst_hyper   = -1;
    layout->info.lst_vis     = False;
    layout->info.join        = False;
    layout->id_Ypos     = 0;
    layout->info.gr_list = NULL;
    layout->info.gr_max  = 0;

    while (count > 0)
      {
	LineInfoClearAndSet (layout,
			CalcCharOffset (charWidth, paraList->first_indent),
			paraList->seg_list, 0);
	layout->info.gr_cnt = 0;
	layout->info.format_y  = _CEFORMAT_ALL;

	_DtHelpCeAddLines (canvas, paraList->lines_before, False,
							&(layout->info.y_pos));
	layout->info.text_x_pos += sideMargin;
	(void) CreateTables(canvas, layout, dispWidth, layout->info.text_x_pos,
		CalcCharOffset (charWidth, paraList->left_margin) + sideMargin,
		CalcCharOffset (charWidth, paraList->right_margin) + sideMargin,
		paraList->spacing);

        _DtHelpCeAddLines (canvas, paraList->lines_after, False,
							&(layout->info.y_pos));

	paraList++;
	count--;
      }

   if (layout->info.gr_list != NULL)
       free(layout->info.gr_list);

}  /* End CCDFCanvasLayout */

/*****************************************************************************
 * Function:    static int CompareHyperSegments (CanvasHandle canvas);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 * 
 *****************************************************************************/
static int
#ifdef _NO_PROTO
CompareHyperSegments (a, b)
    const void	*a;
    const void	*b;
#else
CompareHyperSegments (
    const void	*a,
    const void	*b)
#endif /* _NO_PROTO */
{
    CELinkSegments *linkA = (CELinkSegments *) a;
    CELinkSegments *linkB = (CELinkSegments *) b;
    Unit            centA = linkA->y_pos + (linkA->height >> 1);
    Unit            centB = linkB->y_pos + (linkB->height >> 1);

    if (linkA->y_pos + linkA->height < centB && centA < linkB->y_pos)
	return -1;

    if (linkB->y_pos + linkB->height < centA && centB < linkA->y_pos)
	return 1;

    if (linkA->x_pos != linkB->x_pos)
	return ((linkA->x_pos < linkB->x_pos) ? -1 : 1);

    if (linkA->y_pos != linkB->y_pos)
	return ((linkA->y_pos < linkB->y_pos) ? -1 : 1);

    if (linkA->height != linkB->height)
	return ((linkA->height < linkB->height) ? -1 : 1);

    if (linkA->width != linkB->width)
	return ((linkA->width < linkB->width) ? -1 : 1);

    return 0;
}

/*****************************************************************************
 *              Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:    void _DtHelpCeResizeCanvas (CanvasHandle canvas);
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
_DtHelpCeResizeCanvas (canvas_handle, *ret_width, *ret_height)
    CanvasHandle	canvas_handle;
    Unit		*ret_width;
    Unit		*ret_height;
#else
_DtHelpCeResizeCanvas (
    CanvasHandle	canvas_handle,
    Unit		*ret_width,
    Unit		*ret_height )
#endif /* _NO_PROTO */
{
    int			 i;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CanvasMetrics	 oldMetrics = canvas->metrics;
    LayoutInfo		 layOut;
    CEBoolean		 result = True;

    /*
     * check to see if the width has changed - if not,
     * don't do anything (but re-initialize the metrics
     * to get the new height).
     */
    if (canvas->virt_functions._CEGetCanvasMetrics != NULL)
        (*(canvas->virt_functions._CEGetCanvasMetrics))(canvas->client_data,
					&(canvas->metrics), NULL, NULL, NULL);

    if (oldMetrics.width == canvas->metrics.width)
	return False;

    if (canvas->element_lst != NULL)
	result = _DtHelpCeResizeSDLCanvas(canvas_handle, ret_width, ret_height);
    else
      {
        /*
         * Re-Layout the information.
         * First step - invalidate some counters.
         */
        canvas->hyper_count = 0;/* zero this only because we re-process */
				/* do not zero cur_hyper or we'll loose */
				/* where we are in the TOC              */
        canvas->txt_cnt     = 0;
    
        for (i = 0; i < canvas->graphic_cnt; i++)
          {
	    canvas->graphic_lst[i].pos_y = -1;
	    canvas->graphic_lst[i].pos_x = -1;
          }

        /*
         * Layout the information if there is anything to do
         */
        layOut.target_seg = NULL;
        CCDFCanvasLayout (canvas, &layOut);

        /*
         * return the maximum height and width used
         */
        if (ret_width != NULL)
            *ret_width  = layOut.info.max_x_pos;
        if (ret_height != NULL)
            *ret_height = layOut.info.y_pos;
      }
    
    /*
     * sort the links correctly. First, establish the x,y,width,height
     * of each link.
     */
    for (i = 0; i < canvas->hyper_count; i++)
	_DtHelpCeLinkPosition(canvas, i, &(canvas->hyper_segments[i].x_pos),
				&(canvas->hyper_segments[i].y_pos),
				&(canvas->hyper_segments[i].width),
				&(canvas->hyper_segments[i].height));

    qsort (canvas->hyper_segments, canvas->hyper_count, sizeof(CELinkSegments),
				CompareHyperSegments);

    return result;

}  /* End _DtHelpCeResizeCanvas */

/*****************************************************************************
 * Function:    void _DtHelpCeSetTopic (CanvasHandle canvas);
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
_DtHelpCeSetTopic (canvas_handle, topic_handle, scroll_percent,
			*ret_width, *ret_height, *ret_y)
    CanvasHandle	canvas_handle;
    TopicHandle		topic_handle;
    int			scroll_percent;
    Unit		*ret_width;
    Unit		*ret_height;
    Unit		*ret_y;
#else
_DtHelpCeSetTopic (
    CanvasHandle	canvas_handle,
    TopicHandle		topic_handle,
    int			scroll_percent,
    Unit		*ret_width,
    Unit		*ret_height,
    Unit		*ret_y )
#endif /* _NO_PROTO */
{
    int                  i;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CETopicStruct	*topic  = (CETopicStruct *) topic_handle;
    LayoutInfo		layOut;

    /*
     * clean the canvas
     */
    _DtHelpCeCleanCanvas (canvas_handle);

    if (topic->para_num == 0 && topic->element_list != NULL)
	_DtHelpCeSetSdlTopic(canvas_handle, topic_handle, scroll_percent,
						ret_width, ret_height, ret_y);
    else
      {
        /*
         * attach to the canvas
         */
        canvas->para_lst = topic->para_list;

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
        layOut.target_seg = topic->id_seg;
        CCDFCanvasLayout (canvas, &layOut);
    
        /*
         * destroy the topic handle
         */
        free ((char *) topic);

        /*
         * return the maximum height and width used
         * And the location of the id.
         */
        if (ret_width != NULL)
            *ret_width = layOut.info.max_x_pos;
        if (ret_height != NULL)
            *ret_height = layOut.info.y_pos;

        if (scroll_percent != -1)
	    layOut.id_Ypos = scroll_percent * layOut.info.y_pos / 1000;

        if (ret_y != NULL)
            *ret_y = layOut.id_Ypos;
      }

    /*
     * sort the links correctly. First, establish the x,y,width,height
     * of each link.
     */
    for (i = 0; i < canvas->hyper_count; i++)
	_DtHelpCeLinkPosition(canvas, i, &(canvas->hyper_segments[i].x_pos),
				&(canvas->hyper_segments[i].y_pos),
				&(canvas->hyper_segments[i].width),
				&(canvas->hyper_segments[i].height));

    qsort (canvas->hyper_segments, canvas->hyper_count, sizeof(CELinkSegments),
				CompareHyperSegments);

}  /* End _DtHelpCeSetTopic */
