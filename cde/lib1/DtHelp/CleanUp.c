/* $XConsortium: CleanUp.c /main/cde1_maint/1 1995/07/17 17:24:39 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        CleanUp.c
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
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <stdlib.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "CleanUpI.h"
#include "FormatSDLI.h"
#include "LinkMgrI.h"
#include "VirtFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/
/*****************************************************************************
 *		Private Variables
 *****************************************************************************/
/*****************************************************************************
 *		Semi-Private Variables
 *****************************************************************************/
/*****************************************************************************
 *		Private Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	void FreeSDLStructures (CECanvasStruct *canvas,
 *					CESegment *seg_list)
 *
 * Parameters:
 *		canvas		Specifies the canvas.
 *		seg_list	Specifies the SDL segment list.
 *
 * Returns:	Nothing
 *
 * Purpose:	Free all memory associated with an SDL list.
 *****************************************************************************/
static void
#ifdef _NO_PROTO
FreeSDLStructures (canvas, seg_list, link_data)
    CECanvasStruct	*canvas;
    CESegment		*seg_list;
    CELinkData		*link_data;
#else
FreeSDLStructures (
    CECanvasStruct	*canvas,
    CESegment		*seg_list,
    CELinkData		*link_data)
#endif /* _NO_PROTO */
{
    int		  idx;
    CEElement	 *pEl;
    CESpecial	 *pSpc;
    CESegment	 *nextSeg;
    CESegment	 *topSeg = NULL;
    CEGraphics	 *pGraph;

    while (seg_list != NULL)
      {
	if (link_data != NULL && seg_list->link_index != -1)
	    _DtHelpCeRmLinkFromList(*link_data, seg_list->link_index);

	nextSeg = seg_list->next_seg;
	switch (_CEGetPrimaryType(seg_list->seg_type))
	  {
	    case SDL_ELEMENT:
			pEl = _CEElementOfSegment(seg_list);
			FreeSDLStructures(canvas, pEl->seg_list, link_data);
			_DtHelpCeFreeSdlAttribs(pEl->el_type,
							&(pEl->attributes));
			free((void *)pEl);
			break;
	    case SDL_STRING:
			if (_CEIsNotDuplicate(seg_list->seg_type))
			    free(_CEStringOfSegment(seg_list));

			free(seg_list->seg_handle.str_handle);
			break;
	    case SDL_GRAPHIC:
			if (canvas != NULL
				&& _CEIsInitialized(seg_list->seg_type)
				&& _CEIsNotDuplicate(seg_list->seg_type))
			  {
			    idx    = _CEGraphicIdxOfSeg(seg_list);
			    pGraph = _CEGraphicStructPtr(canvas,idx);

			    _DtHelpCeDestroyGraphic(canvas, pGraph->handle);
			    _CEGraphicStructType(canvas,idx) = 0;
			  }

			break;
	    case CE_SPECIAL:
			if (_CEIsInitialized(seg_list->seg_type))
			  {
			    pSpc = seg_list->seg_handle.spc_handle;
			    if (canvas != NULL &&
				_CEIsValidSpc(seg_list->seg_type) &&
			       canvas->virt_functions._CEDestroySpecial != NULL)
				(*(canvas->virt_functions._CEDestroySpecial))(
					canvas->client_data, pSpc->spc_handle);
			    free((void *)pSpc);
			  }
	  }
	if (seg_list->top == True)
	  {
	    if (topSeg != NULL)
	        free ((void *)topSeg);
	    topSeg = seg_list;
	  }

	seg_list = nextSeg;
      }

    if (topSeg != NULL)
	free((void *)topSeg);

} /* End FreeSDLStructures */

/*****************************************************************************
 *		Semi-Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	void _DtHelpCeFreeSdlStructs (CECanvasStruct *canvas,
 *					CESegment *seg_list)
 *
 * Parameters:
 *		canvas		Specifies the canvas.
 *		seg_list	Specifies the SDL segment list.
 *
 * Returns:	Nothing
 *
 * Purpose:	Free all memory associated with an SDL list.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeFreeSdlStructs (canvas, seg_list, link_data)
    CECanvasStruct	 *canvas;
    CESegment		**seg_list;
    CELinkData		*link_data;
#else
_DtHelpCeFreeSdlStructs (
    CECanvasStruct	*canvas,
    CESegment		**seg_list,
    CELinkData		*link_data)
#endif /* _NO_PROTO */
{
/*
    _DtHelpCeFreeGraphics(canvas);
*/
    if (seg_list != NULL)
      {
        FreeSDLStructures(canvas, *seg_list, link_data);
	*seg_list = NULL;
      }

} /* End _DtHelpCeFreeSdlStructs */

/******************************************************************************
 * Function:    void _DtHelpCeFreeSdlAttribs (enum SdlElement element,
 *                              CESDLAttrStruct *attributes)
 *
 * Parameters:
 *
 * Returns:     
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeFreeSdlAttribs(element, attributes)
    enum SdlElement	 element;
    CESDLAttrStruct	*attributes;
#else
_DtHelpCeFreeSdlAttribs(
    enum SdlElement	 element,
    CESDLAttrStruct	*attributes)
#endif /* _NO_PROTO */
{
    const SDLElementAttrList *myList;
    const SDLAttribute	*attrib;
    void		*varOffset;
    char		**strPtr;

    myList = __CEGetSDLElementAttrs(element);
    if (myList != NULL &&
	(attributes->str1_values != 0 || attributes->str2_values != 0))
      {
	while (myList->sdl_attr_define != -1)
	  {
	    if (_CEIsAttrAString(myList->sdl_attr_define) &&
		_CEIsAttrSet(*attributes, myList->sdl_attr_define))
	      {
	        attrib = __CEGetSDLAttribEntry(myList->sdl_attr_define);

	        if (attrib->sdl_attr_define != -1)
	          {
		    varOffset = ((char *) attributes) + attrib->field_ptr;
		    strPtr    = (char **) varOffset;
		    if (*strPtr != NULL)
		        free(*strPtr);
	          }
	      }
	    myList++;
	  }
      }
}
