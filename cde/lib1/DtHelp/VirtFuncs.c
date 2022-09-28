/* $XConsortium: VirtFuncs.c /main/cde1_maint/1 1995/07/17 17:53:46 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:  VirtFuncs.c
 **
 **   Project: Cde 1.0
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
#include "CanvasError.h"
#include "CESegmentI.h"
#include "CanvasI.h"
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
#define	GROW_SIZE	5
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
 *		Semi-Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function: int _DtHelpCeLoadGraphic
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpCeLoadGraphic (canvas, topic_xid,
	pGraphEl, ret_handle, ret_width, ret_height)
    CECanvasStruct  *canvas;
    char	    *topic_xid;
    CEElement       *pGraphEl;
    GraphicPtr	    *ret_handle;
    Unit	    *ret_width;
    Unit	    *ret_height;
#else
_DtHelpCeLoadGraphic (
    CECanvasStruct  *canvas,
    char	    *topic_xid,
    CEElement	    *pGraphEl,
    GraphicPtr	    *ret_handle,
    Unit	    *ret_width,
    Unit	    *ret_height)
#endif /* _NO_PROTO */
{
    VStatus result = -2;

    if (canvas->virt_functions._CELoadGraphic != NULL)
      {
        result = (*(canvas->virt_functions._CELoadGraphic))(
                            canvas->client_data,
			    canvas->name,
			    topic_xid,
			    pGraphEl->attributes.strings.not_strs.xid,
			    pGraphEl->attributes.strings.not_strs.format,
			    pGraphEl->attributes.strings.not_strs.method,
			    ret_width,
			    ret_height,
			    ret_handle);
      }

    return result;
}

/******************************************************************************
 * Function:    Unit _DtHelpCeGetStringWidth (CECanvasStruct *canvas, CESegment *segment,
 *                                      char *string, int len)
 *
 * Parameters:
 *
 * Returns:
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpCeGetStringWidth (canvas, segment, lang, charset, font_specs, string, len)
    CECanvasStruct      *canvas;
    CESegment           *segment;
    char		*lang;
    char		*charset;
    CanvasFontSpec	*font_specs;
    char                *string;
    int                  len;
#else
_DtHelpCeGetStringWidth (
    CECanvasStruct      *canvas,
    CESegment           *segment,
    char		*lang,
    char		*charset,
    CanvasFontSpec	*font_specs,
    char                *string,
    int                  len)
#endif /* _NO_PROTO */
{
    VStatus	result = -1;

    if (_CEIsNotInitialized(segment->seg_type))
      {
	CEString  *strSeg = segment->seg_handle.str_handle;

	if (canvas->virt_functions._CEGetFont != NULL)
	    (*(canvas->virt_functions._CEGetFont))(
			canvas->client_data,
			lang, charset,
			*(strSeg->font_specs),
			&(strSeg->string),
			&(strSeg->font_handle));

	segment->seg_type = _CESetInitialized(segment->seg_type);
      }

    if (canvas->virt_functions._CEGetStringWidth != NULL)
	result = (*(canvas->virt_functions._CEGetStringWidth)) (
			canvas->client_data,
			string, len,
			_CEGetCharLen(segment->seg_type),
			_CEFontOfSegment(segment));
    if (result < 0)
	result = 0;

    return result;

} /* End _DtHelpCeGetStringWidth */

/******************************************************************************
 * Function:    int _DtHelpCeResolveSpc (CECanvasStruct canvas,
 *					CEElement *spc_element);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpCeResolveSpc(canvas, spc_name, lang, char_set, font_specs, ret_spc)
    CECanvasStruct	*canvas;
    char		*spc_name;
    char		*lang;
    char		*char_set;
    CanvasFontSpec	*font_specs;
    CESpecial		**ret_spc;
#else
_DtHelpCeResolveSpc(
    CECanvasStruct	*canvas,
    char		*spc_name,
    char		*lang,
    char		*char_set,
    CanvasFontSpec	*font_specs,
    CESpecial		**ret_spc)
#endif /* _NO_PROTO */
{
    int		  result = -2;
    CESpecial	 *pSpc;

    if (canvas->virt_functions._CEResolveSpecial != NULL)
      {
        pSpc = (CESpecial *) malloc (sizeof(CESpecial));
        if (pSpc == NULL)
	  {
	    canvas->error = CEErrorMalloc;
	    return -1;
	  }

	pSpc->ascent = -1;
        result = (*(canvas->virt_functions._CEResolveSpecial))(
		canvas->client_data, lang, char_set, *font_specs,
		spc_name,
		&(pSpc->spc_handle), &(pSpc->width), &(pSpc->height),
		&(pSpc->ascent));

        if (result == 0)
	    *ret_spc = pSpc;
        else
	    free((void *)pSpc);
      }

    return result;

} /* End _DtHelpCeResolveSpc */

/******************************************************************************
 * Function:    void _DtHelpCeGetFontMetrics (CECanvasStruct canvas,
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeGetFontMetrics(canvas, font_handle, ret_ascent, ret_descent,
			ret_ave, ret_super_y, ret_sub_y)
    CECanvasStruct	*canvas;
    FontPtr		 font_handle;
    Unit		*ret_ascent;
    Unit		*ret_descent;
    Unit		*ret_ave;
    Unit		*ret_super_y;
    Unit		*ret_sub_y;
#else
_DtHelpCeGetFontMetrics(
    CECanvasStruct	*canvas,
    FontPtr		 font_handle,
    Unit		*ret_ascent,
    Unit		*ret_descent,
    Unit		*ret_ave,
    Unit		*ret_super_y,
    Unit		*ret_sub_y)
#endif /* _NO_PROTO */
{
    if (ret_ascent != NULL)
	*ret_ascent = 0;
    if (ret_descent != NULL)
	*ret_descent = 0;
    if (ret_ave != NULL)
	*ret_ave = 0;
    if (ret_super_y != NULL)
	*ret_super_y = 0;
    if (ret_sub_y != NULL)
	*ret_sub_y = 0;

    if (canvas->virt_functions._CEGetFontMetrics != NULL)
        (*(canvas->virt_functions._CEGetFontMetrics))(
		canvas->client_data, font_handle,
		ret_ascent, ret_descent, ret_ave, ret_super_y, ret_sub_y);

} /* End _DtHelpCeGetFontMetrics */

/******************************************************************************
 * Function:    void _DtHelpCeDestroyGraphic (CECanvasStruct canvas,
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDestroyGraphic(canvas, gr_handle)
    CECanvasStruct	*canvas;
    GraphicPtr		 gr_handle;
#else
_DtHelpCeDestroyGraphic(
    CECanvasStruct	*canvas,
    GraphicPtr		 gr_handle)
#endif /* _NO_PROTO */
{
    if (canvas->virt_functions._CEDestroyGraphic != NULL)
        (*(canvas->virt_functions._CEDestroyGraphic))(
		canvas->client_data, gr_handle);

} /* End _DtHelpCeDestroyGraphic */

/******************************************************************************
 * Function:    void _DtHelpCeDrawString (CECanvasStruct canvas,
 *
 * Parameters:
 *		canvas		Specifies the canvas in which to render.
 *		base_x		Specifies the x unit at which the
 *				string is to be rendered.
 *		base_y		Specifes the y unit at which the baseline
 *				of the string is to be rendered.
 *		string		Specifies the string to render.
 *		len		Specifies the number of bytes of the
 *				string to render.
 *		char_size	Specifies the number of bytes that
 *				comprise one character.
 *		font_handle	Specifies the font associated with this
 *				string.
 *		box_x,box_y	Specifies the upper left hand corner of
 *				the bounding box for this string. This
 *				includes any traversal and/or link offsets
 *				relevent for the line this segment occupies.
 *		box_height	Specifies the maximum height of the bounding
 *				box for this string. This includes any
 *				traversal and/or offsets relevent to this
 *				line.
 *		link_type	Specifies the link type if this string is
 *				part of a link.
 *		old_flags	Specifies .............
 *		new_flags	Specifies .............
 *
 * Returns: Nothing
 *
 * Purpose: Call the virtual function associated with the canvas to
 *          render a string.
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeDrawString(canvas, base_x, base_y, string, len, char_size,
			font_handle, box_x, box_y, box_height,
			link_type, old_flag, new_flag)
    CECanvasStruct	*canvas;
    Unit		 base_x;
    Unit		 base_y;
    char		*string;
    int			 len;
    int			 char_size;
    FontPtr		 font_handle;
    Unit		 box_x;
    Unit		 box_y;
    Unit		 box_height;
    int			 link_type;
    Flags		 old_flag;
    Flags		 new_flag;
#else
_DtHelpCeDrawString(
    CECanvasStruct	*canvas,
    Unit		 base_x,
    Unit		 base_y,
    char		*string,
    int			 len,
    int			 char_size,
    FontPtr		 font_handle,
    Unit		 box_x,
    Unit		 box_y,
    Unit		 box_height,
    int			 link_type,
    Flags		 old_flag,
    Flags		 new_flag)
#endif /* _NO_PROTO */
{
    if (canvas->virt_functions._CEDrawString != NULL)
	(*(canvas->virt_functions._CEDrawString))(
			canvas->client_data,
			base_x   , base_y  ,
			string   , len     , char_size , font_handle,
			box_x    , box_y   , box_height,
			link_type, old_flag, new_flag);

} /* End _DtHelpCeDrawString */
