/* $XConsortium: Snref.c /main/cde1_maint/1 1995/07/17 17:52:10 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Snref.c
 **
 **   Project:     Cde Help System
 **
 **   Description: Retrieves information from the SDL element <snb> to
 **                resolve an <snref>. Also used to duplicate async
 **                blocks.
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
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * private includes
 */
#include "bufioI.h"
#include "Canvas.h"
#include "Access.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "CleanUpI.h"
#include "SnrefI.h"
#include "StringFuncsI.h"
#include "UtilSDLI.h"

#ifdef NLS16
#include <nl_types.h>
#endif

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/

/*****************************************************************************
 *		Private Function Declarations
 *****************************************************************************/
#ifdef _NO_PROTO
#else
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
/*********************************************************************
 * Function: ProcessScript
 *
 *    ProcessScript calls a script and maybe gets data.
 *
 *********************************************************************/
static	CESegment *
#ifdef _NO_PROTO
ProcessScript(ok_routine,client_data,font_specs,link_index,interp_type,data,
		ret_last)
    VStatus		(*ok_routine)();
    ClientData		 client_data;
    CanvasFontSpec	*font_specs;
    int		         link_index;
    enum SdlOption	 interp_type;
    char		*data;
    CESegment		**ret_last;
#else
ProcessScript(
    VStatus		(*ok_routine)(),
    ClientData		 client_data,
    CanvasFontSpec	*font_specs,
    int		         link_index,
    enum SdlOption	 interp_type,
    char		*data,
    CESegment		**ret_last)
#endif /* _NO_PROTO */
{
    char	*wrtBuf   = NULL;
    CESegment	*retSeg   = NULL;
    CESegment	*prevSeg  = NULL;

    *ret_last = NULL;
    if (_DtHelpCeRunInterp(ok_routine,client_data,interp_type,data,&wrtBuf) == 0
			&& wrtBuf != NULL && strlen (wrtBuf) != 0)
      {
        /*
         * Save the strings.
         */
        (void) _DtHelpCeSaveString(&retSeg, ret_last, &prevSeg, wrtBuf,
				font_specs, link_index, MB_CUR_MAX, 0, False);

        free (wrtBuf);
      }

    return retSeg;

}  /* End ProcessScript */

/******************************************************************************
 * Function: DuplicateElement
 *
 *****************************************************************************/
static CESegment *
#ifdef _NO_PROTO
DuplicateElement(canvas, toss, parent_el, src_seg,
			src_str, last_str, top_disp, link_index, vis_link, flag)
    CECanvasStruct	 *canvas;
    CESegment		 *toss;
    CEElement		 *parent_el;
    CESegment		 *src_seg;
    CESegment		**src_str;
    CESegment		**last_str;
    CESegment		**top_disp;
    int			  link_index;
    CEBoolean		  vis_link;
    CEBoolean		  flag;
#else
DuplicateElement(
    CECanvasStruct	 *canvas,
    CESegment		 *toss,
    CEElement		 *parent_el,
    CESegment		 *src_seg,
    CESegment		**src_str,
    CESegment		**last_str,
    CESegment		**top_disp,
    int			  link_index,
    CEBoolean		  vis_link,
    CEBoolean		  flag)
#endif /* _NO_PROTO */
{
    CESegment	*nextSeg;
    CESegment	*junkSeg = NULL;
    CESegment	*topSeg  = NULL;
    CESegment	*prevSeg = NULL;
    CEElement	*srcEl   = NULL;
    CEElement	*dstEl   = NULL;
    CEElement	*semanticEl;
    CEString	*pStr    = NULL;
    int          junk    = 0;
    int          gIndex;
    int          oldIndex = link_index;

    while (src_seg != NULL)
      {
	link_index = oldIndex;
	if (_CEIsHyperLink(src_seg->seg_type))
	    link_index = src_seg->link_index;

	if (_DtHelpCeAllocSegment(1, &junk, &junkSeg, &nextSeg) != 0)
	  {
	    if (topSeg != NULL)
	        _DtHelpCeFreeSdlStructs(canvas, &(topSeg), NULL);
	    return NULL;
	  }

	nextSeg->seg_type           = _CESetDuplicate(src_seg->seg_type);
	nextSeg->link_index         = link_index;
	if (link_index != -1)
	  {
	    if (vis_link == True)
		nextSeg->seg_type = _CESetHyperFlag(nextSeg->seg_type);
	    else
		nextSeg->seg_type = _CESetGhostLink(nextSeg->seg_type);
	  }
	nextSeg->seg_handle.generic = NULL;
	nextSeg->next_seg           = NULL;
	nextSeg->next_disp          = NULL;

	switch (_CEGetPrimaryType(nextSeg->seg_type))
	  {
	    case CE_ELEMENT:
			srcEl = _CEElementOfSegment(src_seg);
			if (_DtHelpCeAllocElement(srcEl->el_type,
					&(parent_el->attributes), &dstEl) != 0)
			  {
			    free(nextSeg);
			    if (topSeg != NULL)
			        _DtHelpCeFreeSdlStructs(canvas, &(topSeg), NULL);
			    return NULL;
			  }
			/*
			 * merge the attributes into the new elements.
			 */
			dstEl->format_break           = srcEl->format_break;
			_DtHelpCeMergeSdlAttribInfo(srcEl, dstEl);
			_DtHelpCeSdlLevel(dstEl) = _DtHelpCeSdlLevel(parent_el);
			if ((srcEl->attributes.enum_values & SDL_ATTR_CLASS)
					||
			    (srcEl->attributes.str1_values & SDL_ATTR_SSI)
					||
			    (srcEl->attributes.num_values & SDL_ATTR_LEVEL))
			  {
			    semanticEl = _DtHelpCeMatchSemanticStyle(toss,
						_DtHelpCeSdlClass(dstEl),
						_DtHelpCeSdlLevel(dstEl),
						_DtHelpCeSdlSsiString(srcEl));

			    _DtHelpCeMergeSdlAttribInfo(semanticEl, dstEl);
			  }

			dstEl->seg_list = DuplicateElement(
							canvas,
							toss,
							dstEl,
							srcEl->seg_list,
							src_str,
							last_str,
							top_disp,
							link_index,
							vis_link,
							False);

			if (dstEl->seg_list == NULL)
			  {
			    free(nextSeg->seg_handle.el_handle);
			    free(nextSeg);
			    return NULL;
			  }
			nextSeg->seg_handle.el_handle = dstEl;
			break;

	    case CE_STRING:
			pStr = (CEString *) malloc (sizeof(CEString));
			if (pStr == NULL)
			  {
			    free(nextSeg);
			    if (topSeg != NULL)
				_DtHelpCeFreeSdlStructs(canvas, &(topSeg), NULL);
			    return NULL;
			  }

			nextSeg->seg_handle.str_handle = pStr;

			/*
			 * duplicate the string pointer,
			 * clear the initalize flag (because we may use
			 * a different font for rendering) and the
			 * duplicate flag is set (so that we don't free
			 * the string multiple times).
			 */
			pStr->string = src_seg->seg_handle.str_handle->string;
			pStr->font_specs = &(parent_el->attributes.font_specs);
			nextSeg->seg_type =
					_CEClearInitialized(nextSeg->seg_type);

			/*
			 * If the source string says the next element
			 * is a display continuation, then this element
			 * should continue for the copy.
			 */
			if (*src_str != NULL && (*src_str)->next_disp != NULL)
			    (*last_str)->next_disp = nextSeg;

			*src_str  = src_seg;
			*last_str = nextSeg;

			/*
			 * have any other displayable segment been seen yet?
			 */
			if (*top_disp == NULL)
			    *top_disp = nextSeg;

			break;

	    case CE_GRAPHIC:
			gIndex = _DtHelpCeAllocGraphicStruct(canvas);
			if (gIndex < 0)
			  {
			    free(nextSeg);
			    if (topSeg != NULL)
				_DtHelpCeFreeSdlStructs(canvas, &(topSeg), NULL);
			    return NULL;
			  }

		        _CEGraphicIdxOfSeg(nextSeg) = gIndex;
			_CEGraphicStruct(canvas,gIndex) = 
				_CEGraphicStruct(canvas,
					_CEGraphicIdxOfSeg(src_seg));

			_CEGraphicLnk(canvas,gIndex) = link_index;

			/*
			 * set the segment type
			 */
			if (parent_el->attributes.clan == SdlClassInLine ||
				parent_el->attributes.clan == SdlClassButton)
			    nextSeg->seg_type =
				_CESetGraphicType(nextSeg->seg_type,CE_IN_LINE);

			nextSeg->seg_type =
					_CESetInitialized(nextSeg->seg_type);
			_CEGraphicStructType(canvas,gIndex) =
							nextSeg->seg_type;

			/*
			 * If the source string says the next element
			 * is a display continuation, then this element
			 * should continue for the copy.
			 */
			if (*src_str != NULL && (*src_str)->next_disp != NULL)
			    (*last_str)->next_disp = nextSeg;

			*src_str  = src_seg;
			*last_str = nextSeg;

			/*
			 * have any other displayable segment been seen yet?
			 */
			if (*top_disp == NULL)
			    *top_disp = nextSeg;

			break;

	    case CE_SPECIAL:
			/*
			 * During the actual layout of the element,
			 * the spc handle will be malloc'ed and initalized.
			 */
			nextSeg->seg_handle.spc_handle = NULL;
			nextSeg->seg_type              =
					_CEClearInitialized(nextSeg->seg_type);
			/*
			 * If the source string says the next element
			 * is a display continuation, then this element
			 * should continue for the copy.
			 */
			if (*src_str != NULL && (*src_str)->next_disp != NULL)
			    (*last_str)->next_disp = nextSeg;

			*src_str  = src_seg;
			*last_str = nextSeg;

			/*
			 * have any other displayable segment been seen yet?
			 */
			if (*top_disp == NULL)
			    *top_disp = nextSeg;

			break;
	  }

	if (prevSeg != NULL)
	    prevSeg->next_seg = nextSeg;

	if (topSeg == NULL)
	    topSeg = nextSeg;

	prevSeg = nextSeg;
	src_seg = src_seg->next_seg;

	/*
	 * check for just wanting to do one item in a segment list
	 */
	if (flag)
	    src_seg = NULL;
      }

    return topSeg;
}

/******************************************************************************
 * Function: ResolveNotationElement
 *
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ResolveNotationElement(canvas, ri_font, nt_seg, nt_el,
			ret_seg, ret_last, ret_top)
    CECanvasStruct	*canvas;
    CanvasFontSpec	*ri_font;
    CESegment		*nt_seg;
    CEElement		*nt_el;
    CESegment		**ret_seg;
    CESegment		**ret_last;
    CESegment		**ret_top;
#else
ResolveNotationElement(
    CECanvasStruct	*canvas,
    CanvasFontSpec	*ri_font,
    CESegment		*nt_seg,
    CEElement		*nt_el,
    CESegment		**ret_seg,
    CESegment		**ret_last,
    CESegment		**ret_top)
#endif /* _NO_PROTO */
{
    int		 dataLen;
    int		 len;
    CESegment	*dataSeg;
    char	*dataStr = NULL;

    *ret_seg = NULL;

    switch (nt_el->el_type)
      {
	/*
	 * this notation element is a graphic.
	 */
        case SdlElementGraphic:
		/*
		 * have we already resolved this graphic?
		 */
	    	if (_CEIsNotInitialized(nt_seg->seg_type))
	    	  {
		    CEGraphics   *pGr;
		    int		  gIdx = _DtHelpCeAllocGraphicStruct(canvas);

		    if (gIdx < 0)
			return False;

		    pGr = _CEGraphicStructPtr(canvas,gIdx);
		    if (_DtHelpCeLoadGraphic(canvas, NULL, nt_el,
				&(pGr->handle), &(pGr->width), &(pGr->height))
					!= 0)
		      {
			canvas->graphic_cnt--;
			return False;
		      }

		    nt_seg->seg_type = _CESetInitialized(nt_seg->seg_type);
		    nt_el->seg_list->seg_type =
				_CESetInitialized(nt_el->seg_list->seg_type);
		    _CEGraphicIdxOfSeg(nt_el->seg_list) = gIdx;
		    _CEGraphicStructType(canvas,gIdx)   = nt_seg->seg_type;
	    	  }
/* extraneous, just drop through.
		return True;
*/

        case SdlElementText:
		return True;

	case SdlElementScript:
		dataSeg = nt_el->seg_list;
		/*
		 * when the information is duplicated,
		 * the correct link_index will be set.
		 */
		*ret_seg = ProcessScript(
				canvas->virt_functions._CEFilterExecCmd,
				canvas->client_data,
				ri_font, -1, nt_el->attributes.interp,
				_CEStringOfSegment(dataSeg),
				ret_last);

		*ret_top = *ret_seg;
		if (*ret_seg != NULL)
		    return True;
      }
    return False;
}

/******************************************************************************
 * Function: ResolveRefItem
 *
 *****************************************************************************/
static CEBoolean
#ifdef _NO_PROTO
ResolveRefItem(canvas, toss, snb_el, ref_seg, ref_el, lst_disp, ret_seg)
    CECanvasStruct	 *canvas;
    CESegment		 *toss;
    CEElement		 *snb_el;
    CESegment		 *ref_seg;
    CEElement		 *ref_el;
    CESegment		 *lst_disp;
    CESegment		**ret_seg;
#else
ResolveRefItem(
    CECanvasStruct	 *canvas,
    CESegment		 *toss,
    CEElement		 *snb_el,
    CESegment		 *ref_seg,
    CEElement		 *ref_el,
    CESegment		 *lst_disp,
    CESegment		**ret_seg)
#endif /* _NO_PROTO */
{
    CESegment	*snSeg;
    CEElement	*snEl;
    CESegment	*srcStr  = NULL;
    CESegment	*lastSeg = NULL;
    CESegment	*topDisp = NULL;
    CEBoolean	 found   = False;

    if (snb_el != NULL && snb_el->seg_list != NULL)
      {
	snSeg = snb_el->seg_list;
	while (snSeg != NULL &&
			snSeg->seg_handle.el_handle->el_type == SdlElementHead)
	    snSeg  = snSeg->next_seg;

	while (snSeg != NULL && !found)
	  {
	    snEl = snSeg->seg_handle.el_handle;
	    if (_DtHelpCeStrCaseCmp(ref_el->attributes.id_vals.rid,
				snEl->attributes.id_vals.id) == 0)
	        found = ResolveNotationElement(canvas,
				&(ref_el->attributes.font_specs),
				snSeg, snEl, ret_seg, &lastSeg, &topDisp);
	    
	    if (!found)
	        snSeg  = snSeg->next_seg;
	  }

	if (found == True)
	  {  
	    if ((*ret_seg) == NULL)
	        *ret_seg = DuplicateElement(canvas, toss, ref_el,
					snSeg,  &srcStr, &lastSeg, &topDisp,
					ref_seg->link_index,
					_CEIsVisibleLink(ref_seg->seg_type),
					True);
	    if ((*ret_seg) != NULL)
	      {
		/*
		 * attach the next displayable piece to the resolved
		 * snref item
		 */
		if (lastSeg != NULL)
		    lastSeg->next_disp = ref_seg->next_disp;

		/*
		 * attach the top of the displayable segments to
		 * the last displayable segment encountered.
		 */
		if (lst_disp != NULL && topDisp != NULL)
		    lst_disp->next_disp = topDisp;

		return True;
	      }
	  }
      }
    return False;
}

/******************************************************************************
 * Function: ResolveSnref
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeResolveSdlSnref(canvas, toss, snb_el, snref_seg, lst_disp)
    CECanvasStruct	*canvas;
    CESegment		*toss;
    CEElement		*snb_el;
    CESegment		*snref_seg;
    CESegment		*lst_disp;
#else
_DtHelpCeResolveSdlSnref(
    CECanvasStruct	*canvas,
    CESegment		*toss,
    CEElement		*snb_el,
    CESegment		*snref_seg,
    CESegment		*lst_disp)
#endif /* _NO_PROTO */
{
    CEBoolean	 found = True;
    CESegment   *refSeg   = NULL;
    CESegment   *refList;
    CESegment   *validSeg = NULL;
    CESegment   *nextSeg  = NULL;
    CESegment   *nextDisp = NULL;
    CEElement	*refEl;
    CEElement	*snrefEl;

    if (_CEIsNotInitialized(snref_seg->seg_type))
      {
	/*
	 * the snref's segment list. It contains the items
	 * to try.
	 */
	snrefEl = snref_seg->seg_handle.el_handle;
	refSeg  = snrefEl->seg_list;
	if (refSeg != NULL)
	    nextDisp = refSeg->next_disp;

	found  = False;

	/*
	 * while there are refitems to try and they haven't been resolved.
	 * keep trying.
	 */
	while (refSeg != NULL && !found)
	  {
	    /*
	     * remember the next segment.
	     */
	    nextSeg = refSeg->next_seg;

	    /*
	     * get the element handle for this segment.
	     */
	    refEl   = refSeg->seg_handle.el_handle;

	    /*
	     * If we've gone through all the other reference elements
	     * and the only one left is the default, take it.
	     */
	    if (refEl->el_type == SdlElementAltText)
		found = True;

	    /*
	     * Try to resolve the reference.
	     */
	    else if (ResolveRefItem(canvas,toss,snb_el,refSeg,refEl,lst_disp,&validSeg)
				== True)
	      {
		/*
		 * good reference, now stick it on the end of any other
		 * information provided by the element
		 */
		found = True;

		refList = refEl->seg_list;

		if (refList != NULL)
		  {
		    while (refList->next_seg != NULL)
		        refList = refList->next_seg;
		    refList->next_seg = validSeg;
		  }
		else
		    refEl->seg_list = validSeg;
	      }
	    else
	      {
		/*
		 * remove this reference - it is invalid
		 */
		_DtHelpCeFreeSdlStructs(canvas, &(refSeg), NULL);
	        refSeg = nextSeg;
	      }
	  }

	/*
	 * We found a reference segment (meaning 'refSeg' is valid)
	 */
	if (found)
	  {
	    /*
	     * If there are more reference segements, free the remaining
	     * reference segements.
	     */
	    if (refSeg->next_seg != NULL)
	      {
	        refList = refSeg->next_seg;
	        do {
	            nextSeg = refList->next_seg;
	            _DtHelpCeFreeSdlStructs(canvas, &(refList), NULL);
	            refList = nextSeg;
	          } while (refList != NULL);
	      }
	  }
	/*
	 * we didn't find the reference, hop over this reference item
	 * and reset the display list to the next displayable item
	 */
	else if (lst_disp != NULL)
	    lst_disp->next_disp = nextDisp;

	snrefEl->seg_list   = refSeg;
	snref_seg->seg_type = _CESetInitialized(snref_seg->seg_type);
      }
}

/******************************************************************************
 * Function: _DtHelpCeResolveAsyncBlocks
 *
 * Initializes the display line and graphic tables.
 *****************************************************************************/
CESegment *
#ifdef _NO_PROTO
_DtHelpCeResolveAsyncBlocks(canvas, toss, parent, id)
    CECanvasStruct	*canvas;
    CESegment		*toss;
    CEElement		*parent;
    char		*id;
#else
_DtHelpCeResolveAsyncBlocks(
    CECanvasStruct	*canvas,
    CESegment		*toss,
    CEElement		*parent,
    char		*id)
#endif /* _NO_PROTO */
{
    CESegment   *junk   = NULL;
    CESegment   *junk2  = NULL;
    CESegment   *junk3  = NULL;
    CESegment   *newBlk = NULL;
    CESegment   *async  = canvas->element_lst;

    while (async != NULL &&  _CEIsElement(async->seg_type) &&
	_CEElementOfSegment(async)->el_type == SdlElementBlockAsync &&
	_DtHelpCeStrCaseCmp(_DtHelpCeSdlIdString(_CEElementOfSegment(async)),id)!=0)
	async = async->next_seg;

    if (async == NULL || _CEIsNotElement(async->seg_type) ||
		_CEElementOfSegment(async)->el_type != SdlElementBlockAsync)
	return NULL;

    newBlk = DuplicateElement(canvas, toss, parent, async, &junk, &junk2,
						&junk3, -1, False, True);
    if (newBlk != NULL)
	_CEElementOfSegment(newBlk)->el_type = SdlElementBlock;

    return (newBlk);
}
