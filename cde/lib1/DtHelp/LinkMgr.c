/* $XConsortium: LinkMgr.c /main/cde1_maint/1 1995/07/17 17:47:06 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        LinkMgr.c
 **
 **   Project:     Cde Help System
 **
 **   Description: Hypertext manager for the core engine of the help
 **                system.  Processes requests from the UI to move, turn
 **                on, or turn off the traversal indicator, to return
 **                information about a hypertext link and to determine
 **                if a spot selected contains a hypertext link.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern	int	errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "LinkMgrI.h"
#include "StringFuncsI.h"
#include "UtilSDLI.h"
#include "VirtFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	void	DrawTraversalIndicator ();
#else
static	void	DrawTraversalIndicator (
			CECanvasStruct	*canvas,
			CEBoolean	 draw_flag,
			Unit		*ret_x,
			Unit		*ret_y,
			Unit		*ret_height);
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/
#define	GROW_SIZE	3

/*****************************************************************************
 *		Private Functions
 *****************************************************************************/
/*****************************************************************************
 * Function: ResolveLink (CESegment *vir_list, char **spec, int *ret_type)
 *
 * Purpose:
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ResolveLink (canvas, vir_list, ret_spec, ret_type, iterations)
    CECanvasStruct	 *canvas;
    CESegment		 *vir_list;
    char		**ret_spec;
    int			 *ret_type;
    int			  iterations;
#else
ResolveLink (
    CECanvasStruct	 *canvas,
    CESegment		 *vir_list,
    char		**ret_spec,
    int			 *ret_type,
    int			  iterations)
#endif /* _NO_PROTO */
{
    int          result = -1;
    CEElement	*pEl;

    *ret_type = CELinkType_SameVolume;

    if (iterations > 100)
	return -1;

    if (vir_list != NULL && _CEIsElement(vir_list->seg_type))
        pEl = _CEElementOfSegment(vir_list);

    while (vir_list != NULL && _CEIsElement(vir_list->seg_type) &&
				pEl != NULL && pEl->el_type != SdlElementSnb)
      {
	vir_list = vir_list->next_seg;
        if (vir_list != NULL && _CEIsElement(vir_list->seg_type))
            pEl = _CEElementOfSegment(vir_list);
      }

    if (vir_list != NULL && _CEIsElement(vir_list->seg_type))
      {
	CESegment  *snList = pEl->seg_list;
	char       *rid    = *ret_spec;

	if (snList != NULL)
	    pEl = _CEElementOfSegment(snList);

	/*
	 * skip the head.
	 */
	if (snList != NULL && _CEIsElement(snList->seg_type) &&
				pEl != NULL && pEl->el_type == SdlElementHead)
          {
	    snList = snList->next_seg;
	    if (snList != NULL)
	        pEl = _CEElementOfSegment(snList);
          }

	/*
	 * look for a link that matches the spec.
	 */
	while (snList != NULL && _CEIsElement(snList->seg_type) &&
		pEl != NULL &&
		_DtHelpCeStrCaseCmp(pEl->attributes.id_vals.rid, rid) != 0)
          {
	    snList = snList->next_seg;
	    if (snList != NULL)
	        pEl = _CEElementOfSegment(snList);
          }

	/*
	 * the only reason that the previous while loop would stop is if
	 * there were not more system notation blocks to look at or a
	 * match was found. If a match wasn't found, just return. The
	 * ret_spec and ret_type are already set.
	 */
	result = 0;
	if (snList != NULL)
	  {
	    int		 len;
	    char	 buffer[128];
	    CESegment	*dataSeg;
#define	GET_XID		(pEl->attributes.strings.not_strs.xid)
#define	GET_NOT_STR(x)	(pEl->attributes.strings.not_strs.x)

	    switch (pEl->el_type)
	      {
		case SdlElementCrossDoc:
			*ret_type = CELinkType_CrossLink;
			*ret_spec = GET_XID;
			break;

		case SdlElementManPage:
			*ret_type = CELinkType_ManPage;
			*ret_spec = GET_XID;
			break;

		case SdlElementSysCmd:
			*ret_type = CELinkType_Execute;
			*ret_spec = GET_NOT_STR(cmd);
			break;

		case SdlElementCallback:
			*ret_type = CELinkType_AppDefine;
			*ret_spec = GET_NOT_STR(data);
			break;

		case SdlElementTextFile:
			*ret_type = CELinkType_TextFile;
			len = strlen (GET_XID) + 1;
			if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_OFFSET))
			  {
			    sprintf(buffer, " %d",
					pEl->attributes.num_vals.offset);
			    len += strlen(buffer);
			  }

			if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_FORMAT))
			    len = len + strlen(GET_NOT_STR(format)) + 1;

			*ret_spec = (char *) malloc (sizeof(char) * len);
			if (*ret_spec != NULL)
			  {
			    strcpy (*ret_spec, GET_XID);
			    if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_OFFSET))
				strcat (*ret_spec, buffer);
			    if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_FORMAT))
				strcat (*ret_spec, GET_NOT_STR(format));
			  }
			else
			    result = -1;
			break;

		case SdlElementSwitch:
			result  = -1;
			dataSeg = pEl->seg_list;

			if (dataSeg != NULL && _CEIsString(dataSeg->seg_type)
				&& dataSeg->seg_handle.str_handle != NULL)
			  {
			    char  *valueData = NULL;

			    result = _DtHelpCeRunInterp(
					canvas->virt_functions._CEFilterExecCmd,
						canvas->client_data,
						pEl->attributes.interp,
						_CEStringOfSegment(dataSeg),
						&valueData);
			    if (result == 0)
			      {
				int   value = atoi(valueData);
				char *idRefs = 
				       pEl->attributes.strings.el_strs.branches;

				iterations++;

				while (value > 0 && *idRefs != '\0')
				  {
				    /*
				     * skip this id.
				     */
				    while(*idRefs != ' ' && *idRefs != '\0')
					idRefs++;

				    /*
				     * skip the spaces
				     */
				    while(*idRefs == ' ')
					idRefs++;

				    /*
				     * decrement the index.
				     */
				    value--;
				  }

				/*
				 * if the value returned is more than
				 * the id list or we hit the end, back
				 * up to the first idref
				 */
				if (value > 0 || *idRefs == '\0')
				    idRefs = 
				       pEl->attributes.strings.el_strs.branches;

				/*
				 * duplicate the id and null out the
				 * extraneous ids.
				 */
				rid    = strdup (idRefs);
				idRefs = rid;
				while(*idRefs != ' ' && *idRefs != '\0')
					idRefs++;
				*idRefs = '\0';

				result = ResolveLink (canvas, vir_list,
							ret_spec,
							ret_type, iterations);
				free (rid);
			      }

			    if (valueData != NULL)
				free(valueData);
			  }
			break;
	      }
	  }
      }
    else
	result = 0;

    return result;
}

/*****************************************************************************
 * Function: IsLineSelected (
 *
 * Purpose:	Call a virtual function to draw the traversal indicator
 *****************************************************************************/
static	CEBoolean
#ifdef _NO_PROTO
IsLineSelected (start, end, line, line_idx, char_idx, length, dst_x, ret_len)
    DtHelpCeSelectStruct	start;
    DtHelpCeSelectStruct	end;
    CELines			line;
    int				line_idx;
    int				char_idx;
    int				length;
    Unit			dst_x;
    int				*ret_len;
#else
IsLineSelected (
    DtHelpCeSelectStruct	start,
    DtHelpCeSelectStruct	end,
    CELines			line,
    int				line_idx,
    int				char_idx,
    int				length,
    Unit			dst_x,
    int				*ret_len)
#endif /* _NO_PROTO */
{
    Unit  maxY = line.baseline + line.descent;
    Unit  minY = line.baseline - line.ascent;

    *ret_len = length;
    if (start.y == -1 || maxY < start.y || minY >= end.y)
	return False;

    /*
     * starts the selection?
     */
    if (line_idx == start.line_idx)
      {
	/*
	 * does this line straddle the start?
	 */
        if (start.char_idx > char_idx)
	  {
	    /*
	     * draw the unselected part
	     */
	    *ret_len = start.char_idx - char_idx;
	    return False;
	  }

	/*
	 * does this line end the selection?
	 */
	if (line_idx == end.line_idx)
	  {
	    /*
	     * does this line straddle the end?
	     */
	    if (char_idx >= end.char_idx)
	      {
		/*
		 * draw this unselected. Its after the selected part.
		 */
		return False;
	      }

	    if (char_idx + length > end.char_idx)
	      {
		/*
		 * draw the selected part
		 */
		*ret_len = end.char_idx - char_idx;
	      }
	  }

	/*
	 * draw the current *ret_len as selected
	 */
      }

    /*
     * does this start the selection?
     */
    else if (line_idx == end.line_idx)
      {
	/*
	 * does not start the selection.
	 * does end the selection.
	 */
	if (char_idx >= end.char_idx)
	    return False;

	/*
	 * straddle the end position?
	 */
	if (char_idx + length > end.char_idx)
	    *ret_len = end.char_idx - char_idx;
	/*
	 * draw the current *ret_len as selected
	 */
      }

    /*
     * start.y != -1
     * start.y <= maxY && minY < end.y
     */
    else if (minY < start.y)
      {
	/*
	 * straddles the start y
	 */
	if (dst_x < start.x)
	    return False;

	/*
	 * dst_x > start.x
	 */
	if (start.y != end.y)
	    return True;

	/*
	 * dst_x >= end.x
	 */
	if (dst_x > end.x)
	    return False;
      }
    /*
     * start.y <= minY and maxY
     * minY < end.y
     */
    else if (end.y <= maxY)
      {
	/*
	 * straddles the end y position
	 */
	if (dst_x >= end.x)
	    return False;
      }
    /*
     * start.y <= minY and maxY
     * minY && maxY < end.y
     */
    return True;
}

/*****************************************************************************
 * Function: DrawTraversalIndicator (CECanvasStruct *canvas, CEBoolean flag)
 *
 * Purpose: (Un)draws the traversal around the currently active link.
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
DrawTraversalIndicator (canvas, draw_flag, ret_x, ret_y)
    CECanvasStruct	*canvas;
    CEBoolean		 draw_flag;
    Unit		*ret_x;
    Unit		*ret_y;
    Unit		*ret_height;
#else
DrawTraversalIndicator (
    CECanvasStruct	*canvas,
    CEBoolean		 draw_flag,
    Unit		*ret_x,
    Unit		*ret_y,
    Unit		*ret_height)
#endif /* _NO_PROTO */
{
    int     hIndex = 0;
    int     count;
    int     len;
    int     start;
    Unit    height;
    Unit    dstX;
    Unit    topY;
    Unit    topX;
    Flags   oldFlag = 0;
    Flags   newFlag = 0;

    CESegment	*pSeg;
    CEGraphics	*pGS;

    /*
     * get the hypertext index.
     */
    hIndex = canvas->cur_hyper;

    /*
     * determine the flags for rendering.
     */
    if (draw_flag)
        newFlag = CETraversalFlag;
    else
        oldFlag = CETraversalFlag;

    /*
     * determine the location of the hypertext segment.
     */
    pSeg = canvas->hyper_segments[hIndex].seg_ptr;
    if (_CEIsGraphic (pSeg->seg_type))
      {
	pGS    = _CEGraphicStructPtr(canvas, _CEGraphicIdxOfSeg(pSeg));
	height = pGS->height;
	dstX   = pGS->pos_x;
	topY   = pGS->pos_y;

	newFlag |= (CELinkFlag | CETraversalBegin | CETocTopicBegin |
					CETraversalEnd | CETocTopicEnd);
	oldFlag |= (CELinkFlag | CETraversalBegin | CETocTopicBegin |
					CETraversalEnd | CETocTopicEnd);

	switch(_DtHelpCeGetLinkHint(canvas->link_data,pGS->lnk_idx))
	  {
	    case CEWindowHint_PopupWindow:
				newFlag |= CELinkPopUp;
				oldFlag |= CELinkPopUp;
				break;
	    case CEWindowHint_NewWindow:
				newFlag |= CELinkNewWindow;
				oldFlag |= CELinkNewWindow;
				break;
	  }

	if (canvas->virt_functions._CEDrawGraphic != NULL)
	    (*(canvas->virt_functions._CEDrawGraphic))(
			canvas->client_data, pGS->handle,
			dstX, topY, oldFlag, newFlag);

	topX = dstX;
      }
    else
      {
	int	  wrkChr;
	int	  totCnt;
	int	  curIdx       = 0;
	Unit      tmpWidth;
	Unit      superWidth   = 0;
	Unit      superY       = 0;
	Unit      subWidth     = 0;
	Unit      subY         = 0;
	Unit      scriptX      = 0;
	CEBoolean lastWasSub   = False;
	CEBoolean lastWasSuper = False;
	CEBoolean lastWasLabel = True;
	CEBoolean lstLnkVis    = False;
	CESegment *tmpSeg;
        int txtLine   = canvas->hyper_segments[hIndex].h_indx;
	int linkIndex = canvas->hyper_segments[hIndex].seg_ptr->link_index;
	int lstLnk    = -1;

	topY  = canvas->txt_lst[txtLine].baseline
					- canvas->txt_lst[txtLine].ascent;
	start = canvas->txt_lst[txtLine].byte_index;
	count = canvas->txt_lst[txtLine].length;

	dstX  = _DtHelpCeGetStartXOfLine(canvas, canvas->txt_lst[txtLine],
					&pSeg, &lastWasLabel);

	while (pSeg->link_index != linkIndex)
	  {
	    /*
	     * get the corrected x
	     */
	    dstX = _DtHelpCeAdvanceXOfLine (canvas, canvas->txt_lst[txtLine],
			pSeg, dstX, &lstLnk, &lastWasLabel, &lstLnkVis);

	    /*
	     * move the text x position base on if this is a super or
	     * sub script - or not.
	     */
	    dstX = _DtHelpCeAdjustForSuperSub(canvas, pSeg, dstX, &scriptX,
				&superWidth, &superY, &subWidth, &subY,
				&lastWasSuper, &lastWasSub);

	    /*
	     * get the width of the segment.
	     */
	    _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start, count,
							&len, &tmpWidth, NULL);
	    dstX += tmpWidth;

	    /*
	     * update pointers
	     */
	    lstLnk  = pSeg->link_index;
	    count  -= len;
	    curIdx += len;
	    pSeg    = pSeg->next_disp;
	    start   = 0;
	  }

	height = 0;
	topX   = dstX;
	while (txtLine < canvas->txt_cnt && linkIndex == pSeg->link_index
					&& _CEIsNotGraphic(pSeg->seg_type))
	  {
	    /*
	     * get the corrected x
	     */
	    dstX = _DtHelpCeAdvanceXOfLine (canvas, canvas->txt_lst[txtLine],
			pSeg, dstX, &lstLnk, &lastWasLabel, &lstLnkVis);

	    /*
	     * move the text x position base on if this is a super or
	     * sub script - or not.
	     */
	    dstX = _DtHelpCeAdjustForSuperSub(canvas, pSeg, dstX, &scriptX,
				&superWidth, &superY, &subWidth, &subY,
				&lastWasSuper, &lastWasSub);

	    /*
	     * now count up the number of bytes to display for
	     * the traversal.
	     */
	    totCnt = count;
	    tmpSeg = pSeg;
	    count  = 0;
	    wrkChr = start;
	    while  (totCnt > 0 && tmpSeg != NULL
			&& tmpSeg->link_index == linkIndex
			&& _CEIsNotGraphic(tmpSeg->seg_type))
	      {
		_DtHelpCeDetermineWidthOfSegment(canvas, tmpSeg, wrkChr,
						totCnt, &len, NULL, NULL);
		totCnt    -= len;
		count     += len;
		wrkChr     = 0;
		tmpSeg     = tmpSeg->next_disp;
	      }

	    /*
	     * set the begin flag.
	     */
	    newFlag |= (CETraversalBegin | CETocTopicBegin);
	    oldFlag |= (CETraversalBegin | CETocTopicBegin);
	    while  (count > 0 && pSeg != NULL && pSeg->link_index == linkIndex
			 && _CEIsNotGraphic(pSeg->seg_type))
	      {
		/*
		 * the original count for the traversal.
		 */
		len = count;

		/*
		 * if there is selected text, determine, how much
		 */
		if (canvas->select_start.y != -1)
		  {
		    /*
		     * clear the selected flag
		     */
		    oldFlag &= ~(CESelectedFlag);
		    newFlag &= ~(CESelectedFlag);

		    if (IsLineSelected(canvas->select_start,
					canvas->select_end,
					canvas->txt_lst[txtLine],
					txtLine,
					curIdx, count, dstX, &len))
		      {
			/*
			 * set the selected flag.
			 */
			oldFlag |= CESelectedFlag;
			newFlag |= CESelectedFlag;
		      }
		  }

		/*
		 * if this is the last segment(s) of the traversal
		 * set the end flags.
		 */
		if (len == count)
		  {
		    newFlag |= (CETraversalEnd | CETocTopicEnd);
		    oldFlag |= (CETraversalEnd | CETocTopicEnd);
		  }

		/*
		 * render the segments
		 */
		dstX = _DtHelpCeDrawSegments(canvas, canvas->txt_lst[txtLine],
				pSeg, start, len, &lstLnk, dstX, dstX,
				&scriptX,&superWidth,&superY,&subWidth,&subY,
				&lastWasSub, &lastWasSuper,&lastWasLabel,
				&lstLnkVis, oldFlag, newFlag);

		count  -= len;
		curIdx += len;
		if (count > 0)
		  {
		    int  maxCnt = count + len;
		    /*
		     * not all of the traversal line was displayed because
		     * part of it is selected. So skip what's been rendered,
		     * and do it again.
		     */
		    totCnt = len;
		    while (totCnt > 0)
		      {
		        /*
		         * get the byte length of the segment processed.
		         */
		        _DtHelpCeDetermineWidthOfSegment(canvas, pSeg, start,
						maxCnt, &len, NULL, NULL);
		        /*
		         * increment the start index by the number of total
		         * bytes processed. If this is more that what is in
		         * the segment, then the if stmt will catch this and
		         * set the start index to zero.
		         */
			if (len > totCnt)
			  {
			    len    = totCnt;
		            start += len;
			  }
		        else /* if (len <= totCnt) */
			  {
			    start = 0;
			    pSeg = pSeg->next_disp;
			  }

		        /*
		         * reduce the total number of bytes processed by the
		         * number in this segment.
		         */
		        totCnt -= len;
		        maxCnt -= len;
		      }
	          }
	        newFlag &= ~(CETraversalBegin | CETocTopicBegin);
	        oldFlag &= ~(CETraversalBegin | CETocTopicBegin);
	      }

	    height += canvas->txt_lst[txtLine].ascent
				+ canvas->txt_lst[txtLine].descent;
	    txtLine++;
	    if (txtLine < canvas->txt_cnt)
	      {
		start  = canvas->txt_lst[txtLine].byte_index;
		count  = canvas->txt_lst[txtLine].length;
		superWidth   = 0;
		superY       = 0;
		subWidth     = 0;
		subY         = 0;
		scriptX      = 0;
                lstLnk       = -1;
		lastWasSuper = False;
		lastWasSub   = False;
		lstLnkVis    = False;

	        /*
	         * get the correct x
	         */
	        dstX = _DtHelpCeGetStartXOfLine (canvas,
					canvas->txt_lst[txtLine],
					&pSeg, &lastWasLabel);
	      }
          }
      }

    if (ret_x)
	*ret_x = topX;

    if (ret_y)
	*ret_y = topY;

    if (ret_height)
	*ret_height = height;

} /* End DrawTraversalIndicator */

/*****************************************************************************
 *		Semi-Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function:    int _DtHelpCeAddLinkToList (link_data, char *link, int type,
 *							char *description)
 *
 * Parameters:
 *              link	Specifies the hypertext link specification.
 *              type	Specifies the type of hypertext link.
 *              link	Specifies the hypertext link description.
 *
 * Returns      The index into the list of hypertext links.
 *		-1 for errors.
 *
 * errno Values:
 *		DtErrorMalloc
 *
 * Purpose:     Place a hypertext link into an array.
 *
 * Note:	The link and description pointers are hereafter owned by
 *		the hypertext list. The caller should not free or realloc
 *		these pointers.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAddLinkToList (link_data, link, type, hint, description)
    CELinkData	 *link_data;
    char	 *link;
    int		  type;
    int		  hint;
    char	 *description;
#else
_DtHelpCeAddLinkToList (
    CELinkData	*link_data,
    char	*link,
    int		 type,
    int		 hint,
    char	*description)
#endif /* _NO_PROTO */
{
    int    i = 0;

    if (link == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    while (i < link_data->max && link_data->list[i].link != NULL)
	i++;

    if (i >= link_data->max)
      {
	link_data->max += GROW_SIZE;
	if (link_data->list != NULL)
	    link_data->list = (CELinkEntry *) realloc(link_data->list,
				(sizeof (CELinkEntry) * link_data->max));
	else
	    link_data->list = (CELinkEntry *) malloc(
				sizeof (CELinkEntry) * link_data->max);

	if (link_data->list == NULL)
	  {
	/*
	    errno = DtErrorMalloc;
	*/
	    return -1;
	  }

	while (i < link_data->max)
	    link_data->list[i++].link = NULL;

	i -= GROW_SIZE;
      }

    link_data->list[i].link    = link;
    link_data->list[i].type    = type;
    link_data->list[i].hint    = hint;
    link_data->list[i].descrip = description;

    return i;
}

/******************************************************************************
 * Function:    void _DtHelpCeRmLinkFromList (link_index)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:     Remove a hypertext link from the array.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeRmLinkFromList (link_data, link_index )
    CELinkData	 link_data;
    int		 link_index;
#else
_DtHelpCeRmLinkFromList (
    CELinkData	 link_data,
    int		 link_index )
#endif /* _NO_PROTO */
{
    if (link_index < link_data.max && link_data.list[link_index].link)
      {
        free (link_data.list[link_index].link);
        if (link_data.list[link_index].descrip)
	    free (link_data.list[link_index].descrip);

	link_data.list[link_index].link    = NULL;
	link_data.list[link_index].descrip = NULL;
      }
}

/******************************************************************************
 * Function:    int _DtHelpCeGetLinkType (link_index)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:     Return the hypertext link type.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetLinkType (link_data, link_index )
    CELinkData	 link_data;
    int		 link_index;
#else
_DtHelpCeGetLinkType (
    CELinkData	 link_data,
    int		 link_index )
#endif /* _NO_PROTO */
{
    int  value = -1;

    if (link_index < link_data.max && link_data.list[link_index].link)
	value = link_data.list[link_index].type;

    return value;
}

/******************************************************************************
 * Function:    int _DtHelpCeGetLinkHint (link_index)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:     Return the hypertext link type.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetLinkHint (link_data, link_index )
    CELinkData	 link_data;
    int		 link_index;
#else
_DtHelpCeGetLinkHint (
    CELinkData	 link_data,
    int		 link_index )
#endif /* _NO_PROTO */
{
    int  value = -1;

    if (link_index < link_data.max && link_data.list[link_index].link)
	value = link_data.list[link_index].hint;

    return value;
}

/******************************************************************************
 * Function:    char *_DtHelpCeGetLinkSpec (link_index)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:     Return the hypertext link type.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetLinkSpec (link_data, link_index )
    CELinkData	 link_data;
    int		 link_index;
#else
_DtHelpCeGetLinkSpec (
    CELinkData	 link_data,
    int		 link_index )
#endif /* _NO_PROTO */
{
    char  *ptr = NULL;

    if (link_index < link_data.max && link_data.list[link_index].link)
	ptr = link_data.list[link_index].link;

    return ptr;
}

/******************************************************************************
 * Function:    int _DtHelpCeGetLinkInfo (link_index, ret_info)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:     Return the hypertext link type.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetLinkInfo (canvas, link_data, link_index, ret_info)
    CanvasHandle	 canvas_handle;
    CELinkData		 link_data;
    int			 link_index;
    CEHyperTextCallbackStruct *ret_info;
#else
_DtHelpCeGetLinkInfo (
    CanvasHandle	 canvas_handle,
    CELinkData		 link_data,
    int			 link_index,
    CEHyperTextCallbackStruct *ret_info)
#endif /* _NO_PROTO */
{
    int    type;
    int    hint;
    int    result = -1;
    char  *spec;
    CECanvasStruct *canvas  = (CECanvasStruct *) canvas_handle;
    CESegment      *virPage = canvas->element_lst;

    if (link_index < link_data.max && link_data.list[link_index].link)
      {
	result = 0;

	spec = link_data.list[link_index].link;
	hint = link_data.list[link_index].hint;

	/*
	 * is this a SDL document? If so look in the snb for its
	 * resolution skipping the asyn block if any.
	 */

	while (virPage != NULL && _CEIsElement(virPage->seg_type) &&
		_CEElementTypeOfSegment(virPage) != SdlElementVirpage)
	    virPage = virPage->next_seg;

	if (virPage != NULL)
	    result = ResolveLink(canvas,
				virPage->seg_handle.el_handle->seg_list,
				&spec, &type, 1);
	else
	    type = link_data.list[link_index].type;

	ret_info->hyper_type    = type;
	ret_info->win_hint      = hint;
	ret_info->specification = spec;
	ret_info->description   = link_data.list[link_index].descrip;
      }

    return result;
}

/*****************************************************************************
 *		Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	void _DtHelpCeProcessLink (CanvasHandle canvas, Unit x,
 *						Unit y);
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeProcessLink (canvas_handle, x1, y1, x2, y2, &ret_info)
    CanvasHandle canvas_handle;
    Unit	 x1;
    Unit	 y1;
    Unit	 x2;
    Unit	 y2;
    CEHyperTextCallbackStruct *ret_info;
#else
_DtHelpCeProcessLink (
    CanvasHandle canvas_handle,
    Unit	 x1,
    Unit	 y1,
    Unit	 x2,
    Unit	 y2,
    CEHyperTextCallbackStruct *ret_info)
#endif /* _NO_PROTO */
{
    int			 hIndex;
    int			 line;
    int			 len;
    int			 count;
    int			 startChar;
    Unit		 topY;
    Unit		 botY;
    Unit		 startX;
    Unit		 endX;
    char		*pChar;
    CEBoolean		 junk;
    CEBoolean		 found = False;
    CESegment		*pSeg  = NULL;
    CEGraphics		*pGS;
    CECanvasStruct	*canvas   = (CECanvasStruct *) canvas_handle;
    CELinkSegments	*lnkSegs  = canvas->hyper_segments;

    /*
     * go through the list
     */
    for (hIndex = 0; !found && hIndex < canvas->hyper_count; hIndex++)
      {
	pSeg = lnkSegs[hIndex].seg_ptr;
	if (_CEIsGraphic(pSeg->seg_type))
	  {
	    pGS    = _CEGraphicStructPtr(canvas, _CEGraphicIdxOfSeg(pSeg));

	    startX = pGS->pos_x;
	    endX   = startX + pGS->width;

	    topY   = pGS->pos_y;
	    botY   = topY + pGS->height;

	    if (x1 >= startX && x1 < endX && x2 >= startX && x2 < endX &&
		y1 >= topY   && y1 < botY && y2 >= topY   && y2 < botY)
	      {
		found = True;
		hIndex--; /* back up one, because, the for loop will incr */
	      }
	  }
      }

    if (!found)
      {
	botY = 0;
	topY = 0;
	line = 0;
	while (line < canvas->txt_cnt && found == False)
	  {
	    topY = canvas->txt_lst[line].baseline -
						canvas->txt_lst[line].ascent;
	    botY = canvas->txt_lst[line].baseline +
						canvas->txt_lst[line].descent;
	    if (topY <= y1 && y1 <= botY)
	      {
		int  lnkIndx = -1;
		CEBoolean lstVisible = False;

		count     = canvas->txt_lst[line].length;
	        pSeg      = canvas->txt_lst[line].seg_ptr;
		startChar = canvas->txt_lst[line].byte_index;
		startX    = canvas->txt_lst[line].text_x;
	
		/*
		 * labels start a little differently.
		 * Their start position is the 'label_x' element.
		 */
		if (_CEIsLabel(pSeg->seg_type))
		    startX = canvas->txt_lst[line].label_x;
	
		while (count > 0 && found == False)
		  {
		    if (_CEIsNotLabel(pSeg->seg_type) &&
					startX < canvas->txt_lst[line].text_x)
			startX = canvas->txt_lst[line].text_x;
	
		    /*
		     * adjust the starting position by the link space
		     */
		    junk = _CEIsVisibleLink(pSeg->seg_type);
		    lstVisible = _DtHelpCeModifyXpos (canvas->link_info, pSeg,
				junk,
				lstVisible, lnkIndx, &startX);
		    /*
		     * adjust the starting position by the traversal space
		     */
		    junk = _CEIsHyperLink(pSeg->seg_type);
		    (void) _DtHelpCeModifyXpos (canvas->traversal_info, pSeg,
				junk,
				((CEBoolean) True), lnkIndx, &startX);

		    lnkIndx = pSeg->link_index;

		    /*
		     * If this is a graphic, we've already checked it.
		     */
		    if (_CEIsGraphic(pSeg->seg_type))
		      {
			startX = startX + _CEGraphicWidth(canvas,
						_CEGraphicIdxOfSeg(pSeg));
			pSeg = pSeg->next_disp;
			continue;
		      }
	
		    /*
		     * skip no-op
		     */
		    if (_CEIsNoop(pSeg->seg_type))
			len = 0;
		    /*
		     * check special
		     */
		    else if (_CEIsSpecial(pSeg->seg_type))
		      {
			len  = 1;
			endX = startX + pSeg->seg_handle.spc_handle->width;
		      }
		    else
		      {
		        /*
		         * initialize the pointer to the string
		         */
		        pChar = _CEStringOfSegment(pSeg) + startChar;
	
		        /*
		         * get the length of the current string.
		         * If it is longer than the line count indicates,
		         * it must be wrapped to the next line. We are
		         * only interested in in the part of the line
		         * that is on the line selected.
		         */
		        len = strlen (pChar);
		        if (len > count)
			    len = count;
	
		        /*
		         * calculate the ending pixel postion for
		         * this string segment.
		         */
		        endX = startX + _DtHelpCeGetStringWidth(canvas, pSeg, NULL,
						NULL, NULL, pChar, len);
		      }
	
		    /*
		     * test to see if the selected segment was this segment.
		     */
		    if (x1 >= startX && x1 < endX && x2 >= startX && x2 < endX)
		      {
			found = True;
			/*
			 * Find the hypertext entry.
			 */
			hIndex = 0;
			while (hIndex < canvas->hyper_count &&
				lnkSegs->seg_ptr->link_index != pSeg->link_index
						&&
				   _CEIsNotGraphic(lnkSegs->seg_ptr->seg_type))
			  {
			    lnkSegs++;
			    hIndex++;
			  }
		      }
		    else
		      {
			/*
			 * go to the next segment.
			 */
			pSeg = pSeg->next_disp;
	
			/*
			 * adjust for the new begining.
			 */
			startX    = endX;
			count     = count - len;
			startChar = 0;
		      }
		  }
	      }
	    line++;
          }
      }

    /*
     * check to see if we found a segment and 
     * see if it is a hypertext segment
     */
    if (found)
      {
	found = False;
	if (_CEIsHyperLink(pSeg->seg_type) &&
			_DtHelpCeGetLinkInfo(canvas,canvas->link_data,
						pSeg->link_index,ret_info) == 0)
	  {
	    canvas->cur_hyper = hIndex;
	    found = True;
          }
      }

    return found;

}  /* End _DtHelpCeProcessLink */

/*****************************************************************************
 * Function:	void _DtHelpCeGetTraversalSelection ()
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeGetTraversalSelection (canvas_handle, &ret_info)
    CanvasHandle canvas_handle;
    CEHyperTextCallbackStruct *ret_info;
#else
_DtHelpCeGetTraversalSelection (
    CanvasHandle canvas_handle,
    CEHyperTextCallbackStruct *ret_info)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CESegment		*pSeg;

	/* Check for the existence of hyperlinks in the canvas.  Only get link
	 * information if there are hyperlinks, i.e. hyper_count > 0.
	 */ 
	if (canvas->hyper_count > 0) {
    	pSeg = canvas->hyper_segments[canvas->cur_hyper].seg_ptr;
    	if (_DtHelpCeGetLinkInfo(canvas,canvas->link_data,pSeg->link_index,ret_info) == 0)
			return True;
	}

    return False;
}

/*****************************************************************************
 * Function:	void _DtHelpCeMoveTraversal ()
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeMoveTraversal (canvas_handle, cmd, wrap, render,
			rid, ret_x, ret_y, ret_height)
    CanvasHandle  canvas_handle;
    int		  cmd;
    CEBoolean	  wrap;
    CEBoolean	  render;
    char	 *rid;
    Unit	 *ret_x;
    Unit	 *ret_y;
    Unit	 *ret_height;
#else
_DtHelpCeMoveTraversal (
    CanvasHandle  canvas_handle,
    int		  cmd,
    CEBoolean	  wrap,
    CEBoolean	  render,
    char	 *rid,
    Unit	 *ret_x,
    Unit	 *ret_y,
    Unit	 *ret_height)
#endif /* _NO_PROTO */
{
    int			 newIndex;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;

    if (canvas->hyper_count == 0)
	return False;

    newIndex = canvas->cur_hyper;
    if (rid != NULL)
      {
	int	lnkIndx;
	char	*lnkId;
	CEBoolean found = False;

	newIndex = 0;
	while (!found && newIndex < canvas->hyper_count)
	  {
	    lnkIndx = canvas->hyper_segments[newIndex].seg_ptr->link_index;
	    lnkId   = _DtHelpCeGetLinkSpec(canvas->link_data, lnkIndx);

	    if (_DtHelpCeStrCaseCmp(lnkId, rid) == 0)
		found = True;
	    else
	        newIndex++;
	  }

	if (found == False)
	    newIndex = canvas->cur_hyper;
      }
    else
      {
        switch (cmd)
          {
	    case CETraversalTop:
			newIndex = 0;
			break;

	    case CETraversalNext:
			newIndex++;
			if (newIndex >= canvas->hyper_count)
			  {
			    newIndex--;
			    if (wrap == True)
			        newIndex = 0;
			  }
			break;

	    case CETraversalPrev:
			newIndex--;
			if (newIndex < 0)
			  {
			    newIndex = 0;
			    if (wrap == True)
			        newIndex = canvas->hyper_count - 1;
			  }
			break;

	    case CETraversalBottom:
			newIndex = canvas->hyper_count - 1;
			break;
          }
      }

    if (render == False)
      {
	canvas->cur_hyper = newIndex;
	return False;
      }

    /*
     * turn off the old traversal
     */
    if (cmd == CETraversalOff)
	DrawTraversalIndicator (canvas, False, NULL, NULL, NULL);

    /*
     * turn off the old traversal and turn on the new one.
     */
    else if (newIndex != canvas->cur_hyper)
      {
	DrawTraversalIndicator (canvas, False, NULL, NULL, NULL);

        canvas->cur_hyper = newIndex;
	DrawTraversalIndicator (canvas, True, ret_x, ret_y, ret_height);
	return True;
      }

    /*
     * Other wise turn on the traversal
     */
    else if (cmd == CETraversalOn)
      {
	DrawTraversalIndicator (canvas, True, ret_x, ret_y, ret_height);
	return True;
      }

    return False;
}

/*****************************************************************************
 * Function:	void _DtHelpCeLinkPosition ()
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	A handle to the canvas or NULL if an error occurs.
 *
 * Purpose:
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeLinkPosition (canvas_handle, indx, ret_x, ret_y, ret_width, ret_height)
    CanvasHandle  canvas_handle;
    int		  indx;
    Unit	 *ret_x;
    Unit	 *ret_y;
    Unit	 *ret_width;
    Unit	 *ret_height;
#else
_DtHelpCeLinkPosition (
    CanvasHandle  canvas_handle,
    int		  indx,
    Unit	 *ret_x,
    Unit	 *ret_y,
    Unit	 *ret_width,
    Unit	 *ret_height)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CESegment		*pSeg;
    CEGraphics		*pGS;
    CEBoolean		 found = False;
    CEBoolean		 junk;

    if (canvas->hyper_count == 0)
	return False;

    pSeg = canvas->hyper_segments[indx].seg_ptr;
    if (_CEIsGraphic(pSeg->seg_type))
      {
        pGS    = _CEGraphicStructPtr(canvas, _CEGraphicIdxOfSeg(pSeg));

        *ret_x = pGS->pos_x;
        *ret_y = pGS->pos_y;
        *ret_width  = pGS->width;
        *ret_height = pGS->height;
	 found = True;
      }
    else
      {
	int  len;
	int  line      = canvas->hyper_segments[indx].h_indx;
	int  lnkIndx   = -1;
	int  count     = canvas->txt_lst[line].length;
	int  startChar = canvas->txt_lst[line].byte_index;
	Unit startX    = canvas->txt_lst[line].text_x;
	Unit endX;
	char	  *pChar;
	CEBoolean  lstVisible = False;

	pSeg    = canvas->txt_lst[line].seg_ptr;
	*ret_y  = canvas->txt_lst[line].baseline -
					canvas->txt_lst[line].ascent;
	*ret_height = canvas->txt_lst[line].ascent +
					canvas->txt_lst[line].descent;
    
	/*
	 * labels start a little differently.
	 * Their start position is the 'label_x' element.
	 */
	if (_CEIsLabel(pSeg->seg_type))
	    startX = canvas->txt_lst[line].label_x;
    
	while (count > 0 && !found && pSeg != NULL)
	  {
	    if (_CEIsNotLabel(pSeg->seg_type) &&
				startX < canvas->txt_lst[line].text_x)
		startX = canvas->txt_lst[line].text_x;
    
	    /*
	     * adjust the starting position by the link space
	     */
	    junk = _CEIsVisibleLink(pSeg->seg_type);
	    lstVisible = _DtHelpCeModifyXpos (canvas->link_info, pSeg,
			junk,
			lstVisible, lnkIndx, &startX);
	    /*
	     * adjust the starting position by the traversal space
	     */
	    junk = _CEIsHyperLink(pSeg->seg_type);
	    (void) _DtHelpCeModifyXpos (canvas->traversal_info, pSeg,
			junk,
			((CEBoolean) True), lnkIndx, &startX);

	    lnkIndx = pSeg->link_index;

	    /*
	     * If this is a graphic, we've already checked it.
	     */
	    if (_CEIsGraphic(pSeg->seg_type))
	      {
		startX = startX + _CEGraphicWidth(canvas,
					_CEGraphicIdxOfSeg(pSeg));
		pSeg = pSeg->next_disp;
		continue;
	      }
   
	    /*
	     * skip no-op
	     */
	    if (_CEIsNoop(pSeg->seg_type))
		len = 0;
  
	    /*
	     * check special
	     */
	    else if (_CEIsSpecial(pSeg->seg_type))
	      {
		len  = 1;
		endX = startX + pSeg->seg_handle.spc_handle->width;
	      }
	    else
	      {
	        /*
	         * initialize the pointer to the string
	         */
	        pChar = _CEStringOfSegment(pSeg) + startChar;
 
	        /*
	         * get the length of the current string.
	         * If it is longer than the line count indicates,
	         * it must be wrapped to the next line. We are
	         * only interested in in the part of the line
	         * that is on the line selected.
	         */
	        len = strlen (pChar);
	        if (len > count)
		    len = count;
	     
	        /*
	         * calculate the ending pixel postion for
	         * this string segment.
	         */
	        endX = startX + _DtHelpCeGetStringWidth(canvas, pSeg, NULL,
					NULL, NULL, pChar, len);

	      }

	    /*
	     * test to see if the selected segment was this segment.
	     */
	    if (pSeg == canvas->hyper_segments[indx].seg_ptr)
	      {
		found = True;
		*ret_x     = startX;
		*ret_width = endX - startX;
	      }
	    else
	      {
		/*
		 * go to the next segment.
		 */
		pSeg = pSeg->next_disp;

		/*
		 * adjust for the new begining.
		 */
		startX    = endX;
		count     = count - len;
		startChar = 0;
	      }
	  }
      }

    return found;
}
