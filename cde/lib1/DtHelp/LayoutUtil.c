/* $XConsortium: LayoutUtil.c /main/cde1_maint/1 1995/07/17 17:45:55 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:       LayoutUtil.c
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

/*
 * system includes
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "LayoutUtilI.h"
#include "SnrefI.h"
#include "VirtFuncsI.h"

#if defined(NLS16) || defined(MESSAGE_CAT)
#include <nl_types.h>
#endif

#ifndef NL_CAT_LOCALE
static const int NL_CAT_LOCALE = 0;
#endif

/******************************************************************************
 *
 * Private Defines
 *
 *****************************************************************************/
#define	GROW_SIZE	10
#define	CheckFormat(x) \
	(((x)->format_y == -1 || (x)->format_y > (x)->y_pos) ? False : True)

/******************************************************************************
 *
 * Private Variables
 *
 *****************************************************************************/
static  char *OneByteCantBeginList = "])}`\"\'.,;?:!";
static  char *OneByteCantEndList   = "[({`\"";

static	CELayoutInfo	DefLayInfo =
    {
	NULL, 		/* CESegment	*line_seg;		*/
	 0,		/* unsigned int	 line_start;		*/
	 0,		/* unsigned int	 line_bytes;		*/
	 0,		/* Unit		 cur_len;		*/
	 0,		/* Unit		 max_x_pos;		*/
	 0,		/* Unit		 y_pos;			*/
	 0,		/* Unit		 text_x_pos;		*/
	 0,		/* Unit		 label_x_pos;		*/
	NULL,		/* int		*gr_list;		*/
	 0,		/* int		 gr_cnt;		*/
	 0,		/* int		 gr_max;		*/
	-1,		/* int		 lst_hyper;		*/
	_CEFORMAT_ALL,	/* int		 format_y;		*/
	FALSE,		/* CEBoolean	 lst_vis;		*/
	FALSE,		/* CEBoolean	 join;			*/
    };

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	CheckMultiCantEndList
 *
 * Returns:	True	if the character matches one of the characters in
 *			the MultiCantEndList.
 *		False	if the character does not match an item in
 *			the MultiCantEndList.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
CheckMultiCantEndList (string, list)
    char	*string;
    wchar_t	*list;
#else
CheckMultiCantEndList (
    char	*string,
    wchar_t	*list)
#endif /* _NO_PROTO */
{
    wchar_t  longChar;

    if (list != NULL)
      {
        mbstowcs (&longChar, string, 1);

        while (*list)
	  {
	    if (*list == longChar)
	        return True;
	    list++;
          }
      }

    return False;
}

/******************************************************************************
 * Function:	CheckMultiCantBeginList
 *
 * Returns:	True	if the character matches one of the characters in
 *			the MultiCantBeginList.
 *		False	if the character does not match an item in
 *			the MultiCantBeginList.
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
CheckMultiCantBeginList (string, list)
    char	*string;
    wchar_t	*list;
#else
CheckMultiCantBeginList (
    char	*string,
    wchar_t	*list)
#endif /* _NO_PROTO */
{
    wchar_t  longChar;

    if (list != NULL)
      {
        mbstowcs (&longChar, string, 1);

        while (*list)
	  {
	    if (*list == longChar)
	        return True;
	    list++;
	  }
      }

    return False;
}

/******************************************************************************
 * Function:	FindMaxGraphicHeight
 *
 * Returns:	The maximum height found in the list of graphic entities.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
FindMaxGraphicHeight (canvas, list, count)
    CECanvasStruct	*canvas;
    int			*list;
    int			 count;
#else
FindMaxGraphicHeight (
    CECanvasStruct	*canvas,
    int			*list,
    int			 count)
#endif /* _NO_PROTO */
{
    int          i;
    int          max = 0;
    CEGraphics  *pGS;

    for (i = 0; i < count; i++)
      {
	pGS = _CEGraphicStructPtr(canvas, list[i]);
	if (pGS->height > max)
	    max = pGS->height;
      }

    return max;
}

/******************************************************************************
 * Function: AdjustInLine (canvas, top, bottom)
 *
 * Returns:  Adjusts the ypos of the inline graphics.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
AdjustInLine (canvas, top, bottom, count, list)
    CECanvasStruct      *canvas;
    int			 top;
    int			 bottom;
    int			 count;
    int			*list;
#else
AdjustInLine (
    CECanvasStruct      *canvas,
    int			 top,
    int			 bottom,
    int			 count,
    int			*list)
#endif /* _NO_PROTO */
{
    int    i;
    int    myIndex;
    int    diff;

    CEGraphics *pGS;

    for (i = 0; i < count; i++)
      {
	myIndex = list[i];
	pGS     = _CEGraphicStructPtr(canvas, myIndex);
	diff    = bottom - top - pGS->height;
	canvas->graphic_lst[myIndex].pos_y = top + diff;
      }
}

/******************************************************************************
 *
 * Private Layout Utility Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    void _DtHelpCeInitLayoutInfo ()
 *
 * Parameters:
 *
 * Returns:     Nothing.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeInitLayoutInfo (canvas, layout)
    CECanvasStruct      *canvas;
    CELayoutInfo	*layout;
#else
_DtHelpCeInitLayoutInfo (
    CECanvasStruct      *canvas,
    CELayoutInfo	*layout)
#endif /* _NO_PROTO */
{
   *layout = DefLayInfo;

   layout->y_pos = canvas->metrics.top_margin;
}

/******************************************************************************
 * Function:    int _DtHelpCeGetTraversalWidth ()
 *
 * Parameters:
 *
 * Returns:     Nothing.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetTraversalWidth (canvas, p_seg, lst_hyper)
    CECanvasStruct      *canvas;
    CESegment           *p_seg;
    int			 lst_hyper;
#else
_DtHelpCeGetTraversalWidth (
    CECanvasStruct      *canvas,
    CESegment           *p_seg,
    int			 lst_hyper)
#endif /* _NO_PROTO */
{
    int  value = 0;
    int  lnkBefore = 0;
    int  lnkAfter  = 0;

    if (lst_hyper != p_seg->link_index)
      {
        if (_CEIsVisibleLink(p_seg->seg_type))
	  {
	    lnkBefore = canvas->link_info.space_before;
	    lnkAfter  = canvas->link_info.space_after;
	  }
        if (_CEIsHyperLink(p_seg->seg_type))
          {
            if (lst_hyper != -1)
                value += (canvas->traversal_info.space_after + lnkAfter);

            value += (canvas->traversal_info.space_before
				+ canvas->traversal_info.space_after
				+ lnkBefore
				+ lnkAfter);
          }
      }

    return value;
}

/******************************************************************************
 * Function: _DtHelpCeAddLines
 *
 * makes sure the last x number of lines are blank.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeAddLines (canvas, number, force, ret_y )
    CECanvasStruct	*canvas;
    int			 number;
    CEBoolean		 force;
    int			*ret_y;
#else
_DtHelpCeAddLines (
    CECanvasStruct	*canvas,
    int			 number,
    CEBoolean		 force,
    int			*ret_y)
#endif /* _NO_PROTO */
{
    int    i = 0;
    int    max  = 0;
    int    tmp  = 0;
    Unit   blankSize;
    Unit   lineSize;
    Unit   ascent  = canvas->font_info.ascent;
    Unit   descent = canvas->font_info.descent;
    CEGraphics *pGS;

    if (number <= 0)
        return;

    lineSize = (ascent + descent) / 2; /* lineHeight - canvas->leading) / 2; */
    if (lineSize == 0)
        lineSize = 1;

    max = *ret_y;
    if (force == False)
      {
        /*
         * Find out what the ypos of the last line of text is
         */
        if (canvas->txt_lst && canvas->txt_cnt)
          {
            i   = canvas->txt_cnt - 1;
            max = canvas->txt_lst[i].baseline + canvas->txt_lst[i].descent;
          }

        /*
         * Find out what the ypos of the last graphic is.
         */
        if (canvas->graphic_lst && canvas->graphic_cnt)
          {
            i = canvas->graphic_cnt - 1;
	    while (i > -1 && canvas->graphic_lst[i].pos_y == -1)
	        i--;

            if (i > -1)
              {
                pGS = _CEGraphicStructPtr(canvas, i);
                tmp = pGS->pos_y + pGS->height;

                /*
                 * if the graphic extends below the text, it's ypos becomes
                 * the maximum ypos.
                 */
                if (tmp > max)
                    max = tmp;
              }
          }
      }

    /*
     * What does this leave us as a blank region
     */
    blankSize = *ret_y - max;

    /*
     * calculate the number of lines as an absolute pixel region
     */
    tmp = number * lineSize;

    /*
     * if this requested size is larger than the current
     * blank region, adjust the global Y position to allow
     * the extra room
     */
    if (tmp > blankSize)
        *ret_y = *ret_y + (tmp - blankSize);
}

/******************************************************************************
 * Function:	CheckOneByteCantEndList
 *
 * Returns:	True	if the character matches one of the characters in
 *			the OneByteCantEndList.
 *		False	if the character does not match an item in
 *			the OneByteCantEndList.
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeCheckOneByteCantEndList (string, cant_end_list)
    char	*string;
    char	*cant_end_list;
#else
_DtHelpCeCheckOneByteCantEndList (
    char	*string,
    char	*cant_end_list)
#endif /* _NO_PROTO */
{
    int  i;

    for (i = 0; cant_end_list[i]; i++)
	if (cant_end_list[i] == *string)
	    return True;

    return False;
}

/******************************************************************************
 * Function:	CheckOneByteCantBeginList
 *
 * Returns:	True	if the character matches one of the characters in
 *			the OneByteCantBeginList.
 *		False	if the character does not match an item in
 *			the OneByteCantBeginList.
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeCheckOneByteCantBeginList (string, cant_begin_list)
    char	*string;
    char	*cant_begin_list;
#else
_DtHelpCeCheckOneByteCantBeginList (
    char	*string,
    char	*cant_begin_list)
#endif /* _NO_PROTO */
{
    int  i;

    for (i = 0; cant_begin_list[i]; i++)
	if (cant_begin_list[i] == *string)
	    return True;

    return False;
}

/******************************************************************************
 * Function:	CheckLineSyntax
 *
 * Purpose:	Checks the line syntax. Will not allow a segment to end
 *		a line if:
 *			the segment does not end with a hypen.
 *			the segment does not end with a space and the
 *				next segment does not begin with a space.
 *			the segment ends with a two byte characters that
 *				can not end a line.
 *			The next segment starts with a two byte character
 *				that can not begin a line.
 *			the segment ends with an one-byte open type and
 *				the next segment starts with a
 *				two byte character.
 *			the segment ends with a two byte character and
 *				the next segment starts with a one-byte
 *				close type.
 *
 * Returns:	True	if the character matches one of the characters in
 *			the MultiCantBeginList.
 *		False	if the character does not match an item in
 *			the MultiCantBeginList.
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeCheckLineSyntax (canvas, pSeg, start, str_len)
    CECanvasStruct	 *canvas;
    CESegment		 *pSeg;
    int			  start;
    int			  str_len;
#else
_DtHelpCeCheckLineSyntax (
    CECanvasStruct	 *canvas,
    CESegment		 *pSeg,
    int			  start,
    int			  str_len)
#endif /* _NO_PROTO */
{
    int     oldType;
    int     myStrLen;
    int     charSize;
    char    *pChar;
    char    *nextChar;
    char    *lastChar;
    CEBoolean isString = False;

    /*
     * Check the flags.
     */
    if (_CEIsNewLine (pSeg->seg_type) || pSeg->next_disp == NULL)
        return True;

    if (_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
					|| _CEIsCaption(pSeg->seg_type))
	isString = True;

    if (isString == True)
      {
        oldType  = _CEGetPrimaryType (pSeg->seg_type);
        pChar    = _CEStringOfSegment(pSeg) + start;
        myStrLen = strlen (pChar);
        charSize = _CEGetCharLen (pSeg->seg_type);
        lastChar = pChar + str_len - charSize;


	if (str_len <= 0)
	    return True;

        /*
         * check to make sure the last character is a valid last
         * character.
         */
        if (charSize == 1 && (*lastChar == ' ' || *lastChar == '-'))
            return True;

        /*
         * If this string is a multi-byte, check the list of
         * mult-bytes that can't end a line.
         */
        if (charSize != 1 &&
                CheckMultiCantEndList(lastChar, canvas->multi_cant_end) == True)
            return False;

        if (myStrLen > str_len)
          {
	    /*
	     * go to the next character.
	     */
	    nextChar = lastChar + charSize;

	    if (charSize == 1 && *nextChar != ' ')
	        return False;

	    /*
	     * If this is multi-byte, check the list of multi-byte
	     * that can't begin a line.
	     */
	    if (charSize != 1 &&
		    CheckMultiCantBeginList(nextChar,canvas->multi_cant_begin)
								== True)
	        return False;

	    return True;
          }
      }


    /*
     * go to the next segment and see if it can
     * start a new line.
     */
    do
      {
        pSeg = pSeg->next_disp;
      } while (pSeg != NULL && _CEIsNoop (pSeg->seg_type)
				&& _CEIsNotNewLine(pSeg->seg_type));
    /*
     * anything but another regular string says true.
     */
    if (pSeg == NULL)
        return True;

    if (_CEIsSpecial (pSeg->seg_type) || _CEIsNonBreakingChar(pSeg->seg_type))
        return False;

    if (isString && oldType != _CEGetPrimaryType(pSeg->seg_type))
        return True;

    if (!(_CEIsString(pSeg->seg_type) || _CEIsLabel(pSeg->seg_type)
					    || _CEIsCaption(pSeg->seg_type)))
	return False;

    /*
     * Check the first character of the next segment to see if
     * it can start a new line.
     */
    nextChar = _CEStringOfSegment(pSeg);
    if (_CEGetCharLen (pSeg->seg_type) == 1)
      {
        if (*nextChar == ' ')
            return True;

        /*
         * If the previous line was a single byte string,
         * it couldn't end a line and neither could this
         * single byte string start a line......
         */
        if (charSize == 1)
            return False;
      }

    /*
     * If this is multi-byte, check the list of multi-byte
     * that can't begin a line.
     */
    if (_CEGetCharLen (pSeg->seg_type) != 1)
      {
        if (CheckMultiCantBeginList(nextChar,canvas->multi_cant_begin)
                                                            == True)
            return False;

        /*
         * the next segment can begin a line.
         * if the previous line was a multi-byte, it could end line.
         */
        if (charSize != 1)
            return True;
      }

    /*
     * Still more to check - 1 byte punctuation around multi-byte.
     */
    if (charSize == 1 &&
        _DtHelpCeCheckOneByteCantEndList(lastChar, OneByteCantEndList) == True)
        return False;
    
    if (charSize != 1 &&
            _DtHelpCeCheckOneByteCantBeginList(nextChar,OneByteCantBeginList)
                                                            == True)
        return False;

    return True;
}

/******************************************************************************
 * Function:	_DtHelpCeGetNextWidth
 *
 * Purpose:	Determines the width of the next legal segment.
 *
 * Returns:	The width of the next legal segment.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetNextWidth (canvas, old_type, lst_hyper, pSeg, start,
		lang, char_set, font_spec,
		prev_seg, toss, snb,
		nextSeg, nextStart, widthCount)
    CECanvasStruct	 *canvas;
    int			  old_type;
    int			  lst_hyper;
    CESegment		 *pSeg;
    int			  start;
    char		 *lang;
    char		 *char_set;
    CanvasFontSpec	 *font_spec;
    CESegment		 *prev_seg;
    CESegment		 *toss,
    CEElement		 *snb,
    CESegment		**nextSeg;
    int			 *nextStart;
    int			 *widthCount;
#else
_DtHelpCeGetNextWidth (
    CECanvasStruct	 *canvas,
    int			  old_type,
    int			  lst_hyper,
    CESegment		 *pSeg,
    int			  start,
    char		 *lang,
    char		 *char_set,
    CanvasFontSpec	 *font_spec,
    CESegment		 *prev_seg,
    CESegment		 *toss,
    CEElement		 *snb,
    CESegment		**nextSeg,
    int			 *nextStart,
    int			 *widthCount)
#endif /* _NO_PROTO */
{
    int      result;
    int      len = 0;
    int      tLen;
    int      charSize;
    int      curWidth;
    int      myLength;
    int      nextLen = 0;
    char    *pChar;
    char    *tChar;
    CEBoolean  good_len;

    while (pSeg != NULL && _CEGetPrimaryType (pSeg->seg_type) == CE_NOOP &&
		_CEIsNotNewLine (pSeg->seg_type))
      {
	pSeg = pSeg->next_disp;
	start = 0;
      }

    if (nextSeg != NULL)
        *nextSeg = pSeg;
    if (nextStart != NULL)
        *nextStart = start;

    if (pSeg == NULL)
	return 0;

    if (_CEIsElement(pSeg->seg_type)
		&& (_CEElementOfSegment(pSeg)->el_type) == SdlElementSnRef
					&& _CEIsNotInitialized(pSeg->seg_type))
      {
	_DtHelpCeResolveSdlSnref(canvas, toss, snb, pSeg, prev_seg);
	pSeg = prev_seg->next_disp;
	if (pSeg == NULL)
	    return 0;
      }

    if (_CEIsSpecial(pSeg->seg_type))
      {
	if (_CEIsNotInitialized(pSeg->seg_type))
	  {
	    if (_DtHelpCeResolveSpc(canvas, pSeg->seg_handle.file_handle,
			lang, char_set, font_spec,
			&(pSeg->seg_handle.spc_handle)) == 0)
		pSeg->seg_type = _CESetValidSpc(pSeg->seg_type);
	    pSeg->seg_type = _CESetInitialized(pSeg->seg_type);
	  }

	if (_CEIsValidSpc(pSeg->seg_type))
	  {
	    len = 1;
	    curWidth    = _CESpcOfSegment(pSeg)->width;
	  }
	else
	    return 0;
      }
    else if (_CEIsNonBreakingChar(pSeg->seg_type))
      {
	pChar = _CEStringOfSegment(pSeg) + start;
	len   = strlen (pChar);

	curWidth = _DtHelpCeGetStringWidth (canvas, pSeg,
				    lang, char_set, font_spec, pChar, len)
			+ _DtHelpCeGetTraversalWidth(canvas, pSeg, lst_hyper);
      }
    else
      {
	if (_CEGetPrimaryType (pSeg->seg_type) != old_type ||
		(old_type == CE_NOOP &&
		    (_CEIsNewLine (pSeg->seg_type) || pSeg->next_disp == NULL)))
	    return 0;
    
	pChar    = _CEStringOfSegment(pSeg) + start;
	charSize = _CEGetCharLen (pSeg->seg_type);
	myLength = strlen (pChar);
    
	if (charSize == 1)
	  {
	    /*
	     * 1 byte font
	     */
	    tChar = pChar;
	    len = 0;
	    do
	      {
		good_len = True;
		result = _DtHelpCeStrcspn (tChar, " -", 1, &tLen);
		len += tLen;
    
		/*
		 * check for '-text'
		 */
		if (result == 0 && tChar[tLen] == '-' && tLen == 0 &&
		    _DtHelpCeCheckLineSyntax(canvas,pSeg,start,(len+1))==False)
		  {
		    len++;
		    tLen++;
		    tChar += tLen;
		    good_len = False;
		  }
    
	      } while (!good_len);
    
	    /*
	     * found either a space or a hypen or null byte.
	     * If we found a hypen, include it.
	     */
	    if (pChar[len] == '-')
		len++;
    
	    curWidth = _DtHelpCeGetStringWidth (canvas, pSeg,
				    lang, char_set, font_spec, pChar, len)
			+ _DtHelpCeGetTraversalWidth(canvas, pSeg, lst_hyper);
    
	    /*
	     * Did we find a space or hypen?
	     * If not, can this segment stand alone?
	     */
	    if (result == 0 ||
		    _DtHelpCeCheckLineSyntax(canvas,pSeg,start,len) == True)
	      {
		if (nextSeg != NULL)
		    *nextSeg   = pSeg;
		if (nextStart != NULL)
		    *nextStart = start + len;
		if (widthCount != NULL)
		    *widthCount = len;
		 return curWidth;
	      }
	  }
	else
	  {
	    /*
	     * multi-byte
	     */
	    len = 0;
    
	    while (len < myLength)
	      {
		len += charSize;
		if (_DtHelpCeCheckLineSyntax(canvas,pSeg,start,len) == True)
		  {
		    pChar    = _CEStringOfSegment(pSeg) + start;
		    curWidth = _DtHelpCeGetStringWidth (canvas, pSeg, lang, char_set,
						font_spec, pChar, len)
			    + _DtHelpCeGetTraversalWidth(canvas,pSeg,lst_hyper);
    
		    if (nextSeg != NULL)
		        *nextSeg    = pSeg;
		    if (nextStart != NULL)
		        *nextStart  = start + len;
		    if (widthCount != NULL)
		        *widthCount = len;
		    return curWidth;
		  }
	      }
    
	    /*
	     * Didn't find a smaller segment that satisfied the requirements.
	     * Determine the length of the current segment.
	     */
	    curWidth = _DtHelpCeGetStringWidth (canvas, pSeg, lang, char_set,
							font_spec, pChar, len)
				    + _DtHelpCeGetTraversalWidth(canvas, pSeg,
							lst_hyper);
	  }
      }

    /*
     * sigh...need to go further...this segment can't end a line
     * either.
     */
    prev_seg = pSeg;
    pSeg = pSeg->next_disp;
    if (pSeg != NULL)
      {
        start = 0;
        curWidth += _DtHelpCeGetNextWidth (canvas, 
			_CEGetPrimaryType (prev_seg->seg_type), lst_hyper,
				pSeg, start, lang, char_set, font_spec,
				prev_seg, toss, snb,
				nextSeg, nextStart, &nextLen);

      }

    if (widthCount != NULL)
        *widthCount = len + nextLen;
    return (curWidth);
}

/******************************************************************************
 * Function: _DtHelpCeSaveInfo
 *
 * Initializes a line table element to the segment it should display.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeSaveInfo (canvas, layout, max_width, r_margin, txt_justify)
    CECanvasStruct	*canvas;
    CELayoutInfo	*layout;
    Unit		 max_width;
    Unit		 r_margin;
    enum SdlOption	 txt_justify;
#else
_DtHelpCeSaveInfo (
    CECanvasStruct	*canvas,
    CELayoutInfo	*layout,
    Unit		 max_width,
    Unit		 r_margin,
    enum SdlOption	 txt_justify)
#endif /* _NO_PROTO */
{
    /*****************************************************************
     * The ascent for a line is described as the number of units
     * above the baseline.
     *
     * The descent for a line is described as the number of units
     * below the baseline.
     * 
     * Neither the ascent or decent value includes the baseline
     ****************************************************************/
    int    len;
    int    start      = layout->line_start;
    int    count      = layout->line_bytes;
    int    maxGHeight;
    int    txtCnt     = canvas->txt_cnt;
    Unit   maxAscent  = 0;
    Unit   maxDescent = 0;
    Unit   maxSpecial = 0;
    Unit   superY     = 0;
    Unit   subY       = 0;
    Unit   fontAscent;
    Unit   fontDescent;
    CEBoolean fndLnk = False;
    CEBoolean visLnk = False;

    char  *pChar;

    CESegment       *pSeg = layout->line_seg;

    if (txtCnt >= canvas->txt_max)
      {
	canvas->txt_max += GROW_SIZE;
	if (canvas->txt_lst)
	    canvas->txt_lst = (CELines *) realloc (
				(void *) canvas->txt_lst,
				(sizeof(CELines) * canvas->txt_max));
	else
	    canvas->txt_lst = (CELines *) malloc (
				(sizeof(CELines) * canvas->txt_max));
/*
 * NOTE....should this routine return a value?
 * If (re)alloc error occurs, this simply ignores the problem.
 */
	if (canvas->txt_lst == NULL)
	  {
	    canvas->txt_max = 0;
	    canvas->txt_cnt = 0;
	    return;
	  }
      }

    while (pSeg != NULL && count > 0)
      {
	len = 0;

	if (_CEIsVisibleLink(pSeg->seg_type))
	    visLnk = True;

	if (_CEIsHyperLink(pSeg->seg_type))
	    fndLnk = True;

	if (_CEIsSpecial(pSeg->seg_type))
	  {
	    len = 1;
	    if (_CESpcOfSegment(pSeg)->ascent == -1)
	      {
	        if (maxSpecial < _CESpcOfSegment(pSeg)->height)
		    maxSpecial = _CESpcOfSegment(pSeg)->height;
	      }
	    else
	      {
		fontAscent  = _CESpcOfSegment(pSeg)->ascent;
		fontDescent = _CESpcOfSegment(pSeg)->height - fontAscent;
	        if (fontAscent > maxAscent)
		    maxAscent = fontAscent;
	        if (fontDescent > maxDescent)
		    maxDescent = fontDescent;
	      }
	  }
	else if (_CEIsString(pSeg->seg_type) &&
					_CEIsSuperScript(pSeg->seg_type))
	  {
	    _DtHelpCeGetFontMetrics(canvas, _CEFontOfSegment(pSeg),
			&fontAscent, &fontDescent, NULL, NULL, NULL);

	    if (fontAscent + superY > maxAscent)
		maxAscent = fontAscent + superY;

	    if (fontDescent - superY > maxDescent)
		maxDescent = fontDescent - superY;

	    pChar = _CEStringOfSegment(pSeg);
	    len   = strlen (pChar);
	  }
	else if (_CEIsString(pSeg->seg_type) &&
					_CEIsSubScript(pSeg->seg_type))
	  {
	    _DtHelpCeGetFontMetrics(canvas, _CEFontOfSegment(pSeg),
			&fontAscent, &fontDescent, NULL, NULL, NULL);

	    if (fontAscent - subY > maxAscent)
		maxAscent = fontAscent - subY;

	    if (fontDescent + subY > maxDescent)
		maxDescent = fontDescent + subY;

	    pChar = _CEStringOfSegment(pSeg);
	    len   = strlen (pChar);
	  }
	else if (_CEIsString (pSeg->seg_type) || _CEIsCaption(pSeg->seg_type)
			|| _CEIsLabel(pSeg->seg_type))
	  {
	    _DtHelpCeGetFontMetrics(canvas, _CEFontOfSegment(pSeg),
			&fontAscent, &fontDescent, NULL, &superY, &subY);

	    if (fontAscent > maxAscent)
		maxAscent = fontAscent;
	    if (fontDescent > maxDescent)
		maxDescent = fontDescent;

	    pChar = _CEStringOfSegment(pSeg) + start;
	    len   = strlen (pChar);

	    if (len > count)
		len = count;

	    start = 0;
	  }

	/*
	 * decrement the count
	 */
	count -= len;

	/*
	 * If this segment terminates the paragraph
	 * force the end of the loop.
	 */
        pSeg = pSeg->next_disp;
      }

    /*
     * if there isn't any string on this line, take defaults
     */
    if (layout->line_bytes == 0)
      {
	maxAscent  = canvas->font_info.ascent;
	maxDescent = canvas->font_info.descent;
      }
    else if (txt_justify != SdlJustifyLeft)
      {
	/*
	 * justify the line.
	 */
	Unit   workWidth = max_width - layout->text_x_pos -
						r_margin - layout->cur_len;
	if (txt_justify == SdlJustifyCenter)
	    workWidth = workWidth / 2;

	if (workWidth < 0)
	    workWidth = 0;

	layout->text_x_pos += workWidth;
      }

    /*
     * adjust for any special characters found
     */
    if (maxSpecial > maxAscent + maxDescent)
	maxAscent = maxSpecial - maxDescent;

    /*
     * adjust for any inline graphics found.
     */
    maxGHeight = FindMaxGraphicHeight (canvas, layout->gr_list, layout->gr_cnt);
    if ((layout->gr_cnt && layout->line_bytes == 0) ||
				(maxGHeight > maxAscent + maxDescent))
	maxAscent = maxGHeight - maxDescent;

    /*
     * adjust ascent and descent by the traversal and link info
     */
    maxDescent += canvas->metrics.leading;
    if (fndLnk)
      {
	maxAscent  += canvas->traversal_info.space_above;
        maxDescent += canvas->traversal_info.space_below;
	if (visLnk)
	  {
	    maxAscent  += canvas->link_info.space_above;
            maxDescent += canvas->link_info.space_below;
	  }
      }

    /*
     * save the line information, if there is a string here.
     */
    if (layout->line_bytes > 0)
      {
	canvas->txt_lst[txtCnt].lnk_flag   = fndLnk;
        canvas->txt_lst[txtCnt].text_x     = layout->text_x_pos;
        canvas->txt_lst[txtCnt].label_x    = layout->label_x_pos;
        canvas->txt_lst[txtCnt].baseline   = layout->y_pos + maxAscent;
        canvas->txt_lst[txtCnt].ascent     = maxAscent;
        canvas->txt_lst[txtCnt].descent    = maxDescent;
        canvas->txt_lst[txtCnt].byte_index = layout->line_start;
        canvas->txt_lst[txtCnt].length     = layout->line_bytes;
        canvas->txt_lst[txtCnt].seg_ptr    = layout->line_seg;

        canvas->txt_cnt++;
      }
    /*
     * blank line is one half the normal size line
     */
    else if (layout->gr_cnt == 0)
      {
	maxAscent  = (maxAscent + maxDescent) / 2;
	maxDescent = 0;
      }

    /*
     * if there is inline graphics, adjust the y position.
     */
    if (layout->gr_cnt)
	AdjustInLine (canvas, layout->y_pos,
			layout->y_pos + maxAscent + maxDescent,
			layout->gr_cnt, layout->gr_list);

    if (layout->text_x_pos + layout->cur_len > layout->max_x_pos)
	layout->max_x_pos = layout->text_x_pos + layout->cur_len;

    /*
     * zero the inline graphic array
     */
    layout->gr_cnt = 0;

    /*
     * zero the string info
     */
    layout->line_bytes = 0;
    layout->cur_len    = 0;
    layout->lst_hyper  = -1;
    layout->lst_vis    = False;
    layout->join       = False;

    /*
     * adjust where the next line is positioned.
     */
    layout->y_pos = layout->y_pos + maxAscent + maxDescent + 1;
}

/******************************************************************************
 * Function: CheckAddToHyperList
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeCheckAddToHyperList (canvas, p_seg, lst_vis, lst_hyper, cur_len)
    CECanvasStruct	*canvas;
    CESegment		*p_seg;
    CEBoolean		*lst_vis;
    int			*lst_hyper;
    Unit		*cur_len;
#else
_DtHelpCeCheckAddToHyperList (
    CECanvasStruct	*canvas,
    CESegment		*p_seg,
    CEBoolean		*lst_vis,
    int			*lst_hyper,
    Unit		*cur_len)
#endif /* _NO_PROTO */
{
    int   nxtHyper = canvas->hyper_count;
    int   prevIndx = nxtHyper - 1;
    CEBoolean junk;

    if (_CEIsHyperLink (p_seg->seg_type))
      {
	if (nxtHyper >= canvas->hyper_max)
	  {
	    canvas->hyper_max += GROW_SIZE;
	    if (canvas->hyper_segments)
	        canvas->hyper_segments = (CELinkSegments *) realloc (
	                    (char *) canvas->hyper_segments,
	                    (sizeof (CELinkSegments) * canvas->hyper_max));
	    else
	        canvas->hyper_segments = (CELinkSegments *) malloc (
	                    sizeof (CELinkSegments) * canvas->hyper_max);
/*
 * NOTE....should this routine return a value?
 * If (re)alloc error occurs, this simply ignores the problem.
 */
	    if (canvas->hyper_segments == NULL)
	      {
		canvas->hyper_max   = 0;
		canvas->hyper_count = 0;
		return;
	      }
	  }

	if (_CEIsGraphic(p_seg->seg_type) || prevIndx < 0
			||
	     (_CEIsGraphic(canvas->hyper_segments[prevIndx].seg_ptr->seg_type)
				&&
		_CEIsNotGraphic(p_seg->seg_type))
			||
	     p_seg->link_index !=
			canvas->hyper_segments[prevIndx].seg_ptr->link_index)
	  {
	    /*
	     * save the start of the hypertext link sequence.
	     */
	    canvas->hyper_segments[nxtHyper].seg_ptr = p_seg;
	    canvas->hyper_segments[nxtHyper].h_indx  = canvas->txt_cnt;
	    if (_CEIsGraphic(p_seg->seg_type))
		canvas->hyper_segments[nxtHyper].h_indx =
						_CEGraphicIdxOfSeg(p_seg);

	    canvas->hyper_count++;
	  }
      }

    /*
     * take into account the link metrics.
     */
    junk = _CEIsVisibleLink(p_seg->seg_type);
    *lst_vis = _DtHelpCeModifyXpos(canvas->link_info, p_seg, junk,
			*lst_vis, *lst_hyper,
			cur_len);
    /*
     * take into account the traversal metrics
     */
    junk = _CEIsHyperLink(p_seg->seg_type);
    (void) _DtHelpCeModifyXpos(canvas->traversal_info, p_seg, junk,
			((CEBoolean) True), *lst_hyper,
			cur_len);

    *lst_hyper = p_seg->link_index;
}

/******************************************************************************
 * Function: ProcessStringSegment
 *
 * chops a string segment up until its completely used.
 *
 * Returns:
 *	0	if the entire string segment was processed.
 *	1	if the required number of lines were processed.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeProcessStringSegment(canvas, lay_info, max_width, l_margin, r_margin,
			toss, snb, cur_seg, cur_start,
			lang, char_set, font_specs,
			txt_justify, stat_flag)
    CECanvasStruct	*canvas;
    CELayoutInfo	*lay_info;
    Unit		 max_width;
    Unit		 l_margin;
    Unit		 r_margin;
    CESegment		*toss;
    CEElement		*snb;
    CESegment		*cur_seg;
    unsigned int	*cur_start;
    char		*lang;
    char		*char_set;
    CanvasFontSpec	*font_specs;
    enum SdlOption	 txt_justify;
    CEBoolean		 stat_flag;
#else
_DtHelpCeProcessStringSegment(
    CECanvasStruct	*canvas,
    CELayoutInfo	*lay_info,
    Unit		 max_width,
    Unit		 l_margin,
    Unit		 r_margin,
    CESegment		*toss,
    CEElement		*snb,
    CESegment		*cur_seg,
    unsigned int	*cur_start,
    char		*lang,
    char		*char_set,
    CanvasFontSpec	*font_specs,
    enum SdlOption	 txt_justify,
    CEBoolean		 stat_flag)
#endif /* _NO_PROTO */
{
    Unit   workWidth;
    Unit   stringLen;
    Unit   textWidth;
    Unit   nWidth;
    Unit   spaceSize = 0;
    int	   oldType;
    int	   retStart;
    int	   retCount;
    char	 *pChar;
    CEBoolean    done    = False;
    CESegment   *retSeg;

    if (cur_seg->seg_handle.generic)
      {
	if (lay_info->cur_len == 0)
	  {
	    lay_info->line_seg   = cur_seg;
	    lay_info->line_start = *cur_start;
	  }

	oldType = _CEGetPrimaryType (cur_seg->seg_type);

	while (1)
	  {
	    /*
	     * recalculate the width
	     */
	    workWidth = max_width - lay_info->cur_len - r_margin - l_margin;

	    /*
	     * adjust the character pointer and get the
	     * length of the string.
	     */
	    pChar     = _CEStringOfSegment(cur_seg) + *cur_start;
	    stringLen = strlen (pChar);
    
	    /*
	     * get the pixel width of the text string.
	     */
	    textWidth = _DtHelpCeGetStringWidth (canvas, cur_seg,
			lang, char_set, font_specs,
			pChar, stringLen)
			+ _DtHelpCeGetTraversalWidth(canvas, cur_seg,
					lay_info->lst_hyper);
	    /*
	     * Will it fit in the current width?
	     */
	    if (stat_flag == True || textWidth <= workWidth)
	      {
		/*
		 * Yes, this segment or part of a segment can fit in the
		 * current width. But can the last character of this
		 * segment end a line and can the beginning of the next
		 * segment start a new line?
		 */
		if (stat_flag == True ||
			_DtHelpCeCheckLineSyntax (canvas, cur_seg,
					*cur_start, stringLen) == TRUE)
		  {
	            /*
	             * check to see if this a hypertext that needs
	             * to be remembered.
	             */
	            _DtHelpCeCheckAddToHyperList (canvas, cur_seg,
					&(lay_info->lst_vis),
					&(lay_info->lst_hyper),
					&(lay_info->cur_len));

		    /*
		     * The line syntax is good.
		     * Update the global and width variables.
		     */
		    lay_info->line_bytes += stringLen;
		    lay_info->cur_len    += textWidth;
		    lay_info->join        = False;
		    if (_CEIsNonBreakingChar(cur_seg->seg_type))
			lay_info->join = True;
    
		    /*
		     * Check to see if this segment forces an end
		     */
		    if (_CEIsNewLine (cur_seg->seg_type) &&
			(lay_info->line_bytes || lay_info->gr_cnt))
			    _DtHelpCeSaveInfo (canvas, lay_info, max_width,
							r_margin, txt_justify);

		    return 0;
		  }
    
		/*
		 * CheckLineSyntax says that either this line couldn't
		 * end a line or the next segment couldn't start a line.
		 * Therefore, find out how much of the next segment or
		 * segments we need to incorporate to satisfy the Line
		 * Syntax rules.
		 */
		nWidth = _DtHelpCeGetNextWidth (canvas, oldType,
				lay_info->lst_hyper,
				cur_seg->next_disp, 0,
				lang, char_set, font_specs,
				cur_seg, toss, snb,
				&retSeg, &retStart, &retCount);
		/*
		 * will this segment + the next segment fit?
		 */
		if (textWidth + nWidth <= workWidth)
		  {
	            /*
	             * check to see if this a hypertext that needs
	             * to be remembered.
	             */
	            _DtHelpCeCheckAddToHyperList (canvas, cur_seg,
					&(lay_info->lst_vis),
					&(lay_info->lst_hyper),
					&(lay_info->cur_len));

		    /*
		     * YEAH Team!! It Fits!!
		     *
		     * Update the global and width variables.
		     */
		    lay_info->line_bytes += stringLen;
		    lay_info->cur_len    += textWidth;
		    lay_info->join        = False;
    
		    return 0;
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
				lay_info->lst_hyper,
				cur_seg, *cur_start,
				lang, char_set, font_specs,
				cur_seg, toss, snb,
				&retSeg, &retStart, &retCount);

		if (retSeg == cur_seg && textWidth + nWidth <= workWidth)
		  {
	            /*
	             * check to see if this a hypertext that needs
	             * to be remembered.
	             */
	            _DtHelpCeCheckAddToHyperList (canvas, cur_seg,
					&(lay_info->lst_vis),
					&(lay_info->lst_hyper),
					&(lay_info->cur_len));

		    lay_info->join = False;
		    *cur_start     = retStart;
		    stringLen     += retCount;
		    textWidth     += nWidth;
		    spaceSize      = 0;
		    if (_CEGetCharLen(cur_seg->seg_type) == 1)
		      {
			pChar = _CEStringOfSegment(cur_seg)
							+ *cur_start;
			if (*pChar == ' ')
			  {
			    spaceSize = _DtHelpCeGetStringWidth(canvas,
						cur_seg,
				lang, char_set, font_specs,
						pChar, 1)
					+ _DtHelpCeGetTraversalWidth (canvas,
							cur_seg, lay_info->lst_hyper);
			    textWidth += spaceSize;
			    stringLen++;
			    (*cur_start)++;
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
	    lay_info->line_bytes += stringLen;
	    lay_info->cur_len  += textWidth;

	    if (lay_info->join == True || lay_info->line_bytes == 0)
	      {
	        /*
		 * This line would be empty if we followed the rules.
		 * Or it would break a line improperly.
		 * Force this onto the line.
	         * check to see if this a hypertext that needs
	         * to be remembered.
	         */
	        _DtHelpCeCheckAddToHyperList (canvas, cur_seg,
					&(lay_info->lst_vis),
					&(lay_info->lst_hyper),
					&(lay_info->cur_len));

		/*
		 * Couldn't find a smaller, have to
		 * go with the larger segment.
		 */
		pChar      = _CEStringOfSegment(cur_seg) + *cur_start;
		stringLen  = strlen (pChar);
		if (retCount > 0 && retCount < stringLen)
		    stringLen = retCount;

		lay_info->line_bytes += stringLen;
		lay_info->cur_len  += (_DtHelpCeGetStringWidth(canvas, cur_seg,
				lang, char_set, font_specs, pChar, stringLen)
					+ _DtHelpCeGetTraversalWidth (canvas,
					cur_seg, lay_info->lst_hyper));
	        lay_info->join = False;

		/*
		 * If we had to do a bigger segment,
		 * then we're done processing the target segment.
		 */
		if (stringLen == strlen(pChar))
		  {
		    if (_DtHelpCeCheckLineSyntax (canvas, cur_seg,
					*cur_start, stringLen) == False)
	                lay_info->join = True;
		    return 0;
		  }

		*cur_start     = retStart;
	      }
	    else if (spaceSize)
	      {
		/*
		 * If a space was included as the last character,
		 * remove it now.
		 */
		lay_info->line_bytes--;
		lay_info->cur_len -= spaceSize;
	      }

	    /*
	     * Save the information
	     */
	    _DtHelpCeSaveInfo (canvas, lay_info, max_width, r_margin, txt_justify);

	    /*
	     * Skip the spaces.
	     */
	    pChar = _CEStringOfSegment(cur_seg) + *cur_start;
	    while (*pChar == ' ')
	      {
		pChar++;
		(*cur_start)++;
	      }

	    /*
	     * are we at the end of the segment?
	     */
	    if (*pChar == '\0')
	        return 0;

	    /*
	     * Initialize the global variables
	     */
	    lay_info->line_seg   = cur_seg;
	    lay_info->line_start = *cur_start;
	    lay_info->text_x_pos = l_margin;

	    if (CheckFormat(lay_info) == True)
		return 1;
	  }
      }
    else if (_CEIsNewLine (cur_seg->seg_type))
      {
	/*
	 * Force a save - even if it is an empty line.
	 */
	_DtHelpCeSaveInfo (canvas, lay_info, max_width, r_margin, txt_justify);
      }

    return 0;

} /* End _DtHelpCeProcessStringSegment */

/******************************************************************************
 * Function: int _DtHelpCeAllocGraphicStruct()
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAllocGraphicStruct (canvas)
    CECanvasStruct  *canvas;
#else
_DtHelpCeAllocGraphicStruct (
    CECanvasStruct  *canvas)
#endif /* _NO_PROTO */
{
    CEGraphics *pGr;
    int         result = canvas->graphic_cnt;

    if (canvas->graphic_cnt >= canvas->graphic_max)
      {
        canvas->graphic_max += GROW_SIZE;
        if (canvas->graphic_lst)
            canvas->graphic_lst = (CEGraphics *) realloc (
                        (void *) canvas->graphic_lst,
                        (sizeof(CEGraphics)*canvas->graphic_max));
        else
            canvas->graphic_lst = (CEGraphics *) malloc (
                        (sizeof(CEGraphics)*canvas->graphic_max));

        if (canvas->graphic_lst == NULL)
            return -1;
      }

    pGr = _CEGraphicStructPtr(canvas,result);

    pGr->type    = _CESetGraphic(0);
    pGr->lnk_idx = -1;
    pGr->pos_x   = -1;
    pGr->pos_y   = -1;
    pGr->height  = 0;
    pGr->width   = 0;
    pGr->handle  = NULL;

    canvas->graphic_cnt++;
    return result;
}

/******************************************************************************
 * Function: AddToInLineList (index)
 *
 * Adds the pGS pointer to a list. The ypos of the graphic may be changed
 * later.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeAddToInLineList (info, index)
    CELayoutInfo *info;
    int		  index;
#else
_DtHelpCeAddToInLineList (
    CELayoutInfo *info,
    int		  index)
#endif /* _NO_PROTO */
{
    if (info->gr_cnt >= info->gr_max)
      {
	info->gr_max += GROW_SIZE;
	if (info->gr_list)
	    info->gr_list = (int *) realloc ((void *) info->gr_list,
				(sizeof(int) * info->gr_max));
	else
	    info->gr_list = (int *) malloc (sizeof(int) * info->gr_max);
/*
 * NOTE....should this routine return a value?
 * If (re)alloc error occurs, this simply ignores the problem.
 */
	if (info->gr_list == NULL)
	  {
	    info->gr_cnt = 0;
	    info->gr_max = 0;
	    return;
	  }
      }

    info->gr_list[info->gr_cnt++] = index;
}

/******************************************************************************
 * Function:	_DtHelpCeLoadMultiInfo
 *
 * Returns:	Loads the multi-byte formatting table for the current locale.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeLoadMultiInfo (canvas)
    CECanvasStruct  *canvas;
#else
_DtHelpCeLoadMultiInfo (
    CECanvasStruct  *canvas)
#endif /* _NO_PROTO */
{
#ifdef MESSAGE_CAT
    int       len;
    char     *ptr;
    nl_catd   cat_fd;

    cat_fd = catopen ("fmt_tbl", NL_CAT_LOCALE);
    if (cat_fd != ((nl_catd) -1))
      {
	/*
	 * Get the list of characters that can't begin a line.
	 */
	ptr = catgets (cat_fd, 1, 1, "");
	len = strlen (ptr) + 1;
	canvas->multi_cant_begin = (wchar_t *) malloc (len * sizeof (wchar_t));
	if (canvas->multi_cant_begin != NULL &&
			mbstowcs(canvas->multi_cant_begin,ptr,len) == -1)
	  {
	    free (canvas->multi_cant_begin);
	    canvas->multi_cant_begin = NULL;
	  }

	/*
	 * Get the list of characters that can't end a line.
	 */
	ptr = catgets (cat_fd, 1, 2, "");
	len = strlen (ptr) + 1;
	canvas->multi_cant_end = (wchar_t *) malloc (len * sizeof (wchar_t));
	if (canvas->multi_cant_end != NULL &&
				mbstowcs(canvas->multi_cant_end,ptr,len) == -1)
	  {
	    free (canvas->multi_cant_end);
	    canvas->multi_cant_end = NULL;
	  }

	/*
	 * Get the spacing flag. I.E. when does a internal newline
	 * get turned into a space.
	 *      1 means all the time.
	 *      0 means only between a multibyte string and
	 *        a singlebyte string.
	 */
	ptr = catgets (cat_fd, 1, 3, "1");
	canvas->nl_to_space = atoi(ptr);

	catclose (cat_fd);
      }
    else
#endif
      {
	canvas->multi_cant_begin = NULL;
	canvas->multi_cant_end   = NULL;
	canvas->nl_to_space = 1;
      }
}

