/* $XConsortium: CanvasToc.c /main/cde1_maint/1 1995/07/17 17:24:23 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File: CanvasToc.c
 **
 **   Project:
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
#include <stdio.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "bufioI.h"
#include "Access.h"
#include "AccessP.h"
#include "AccessI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "FontAttrI.h"
#include "FormatCCDFI.h"
#include "FormatSDLI.h"
#include "StringFuncsI.h"

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
/*****************************************************************************
 *		Private Functions
 *****************************************************************************/
static void
#ifdef _NO_PROTO
MarkTocTopic(canvas_handle, rid, render,
				ret_toc_y, ret_toc_height, ret_valid)
    CanvasHandle  canvas_handle;
    char         *rid;
    CEBoolean	  render;
    Unit	 *ret_toc_y;
    Unit	 *ret_toc_height;
    Unit	 *ret_valid;
#else
MarkTocTopic(
    CanvasHandle  canvas_handle,
    char         *rid,
    CEBoolean	  render,
    Unit	 *ret_toc_y,
    Unit	 *ret_toc_height,
    Unit	 *ret_valid)
#endif /* _NO_PROTO */
{
    int			 newIndex = 0;
    int			 lnkIndx;
    char		*lnkId;
    CECanvasStruct      *canvas   = (CECanvasStruct *) canvas_handle;

    if (ret_valid != NULL)
        *ret_valid = (*ret_valid & ~(CETocPoints));

    if (canvas->hyper_count == 0)
	return;

    while (newIndex < canvas->hyper_count)
      {
	lnkIndx = canvas->hyper_segments[newIndex].seg_ptr->link_index;
	lnkId   = _DtHelpCeGetLinkSpec(canvas->link_data, lnkIndx);

	if (_DtHelpCeStrCaseCmp(lnkId, rid) == 0)
	  {
	    if (canvas->toc_on == True)
		_DtHelpCeTurnTocMarkOff(canvas, render);
	
	    canvas->toc_loc = canvas->hyper_segments[newIndex].h_indx;
	    _DtHelpCeTurnTocMarkOn(canvas, render, ret_toc_y, ret_toc_height);
	    if (ret_valid != NULL)
	        *ret_valid |= CETocPoints;

	    return;
	  }
	else
	   newIndex++;
      }

    return;
}

/*****************************************************************************
 *		Semi-Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	void _DtHelpCeTurnTocMarkOn (CanvasHandle canvas,
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	void
 *
 * Purpose:
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeTurnTocMarkOn (canvas_handle, render, ret_toc_y, ret_toc_height)
    CanvasHandle  canvas_handle;
    CEBoolean	  render;
    Unit	 *ret_toc_y;
    Unit	 *ret_toc_height;
#else
_DtHelpCeTurnTocMarkOn (
    CanvasHandle  canvas_handle,
    CEBoolean	  render,
    Unit	 *ret_toc_y,
    Unit	 *ret_toc_height)
#endif /* _NO_PROTO */
{
    int result = -1;
    CECanvasStruct *canvas = (CECanvasStruct *) canvas_handle;

    if (canvas->toc_flag == True)
      {
	if (canvas->toc_on == False && render)
            (void) _DtHelpCeDrawText (canvas, canvas->txt_lst[canvas->toc_loc],
			canvas->txt_lst[canvas->toc_loc].label_x, -1,
				0, ((Flags)CETocTopicFlag));
        canvas->toc_on = True;
	*ret_toc_y      = canvas->txt_lst[canvas->toc_loc].baseline -
				canvas->txt_lst[canvas->toc_loc].ascent;
	*ret_toc_height = canvas->txt_lst[canvas->toc_loc].ascent +
				canvas->txt_lst[canvas->toc_loc].descent;
	result = 0;
      }

    return result;

} /* End _DtHelpCeTurnTocMarkOn */

/*****************************************************************************
 * Function:	void _DtHelpCeTurnTocMarkOff (CanvasHandle canvas,
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	void
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeTurnTocMarkOff (canvas_handle, render)
    CanvasHandle  canvas_handle;
    CEBoolean	  render;
#else
_DtHelpCeTurnTocMarkOff (
    CanvasHandle  canvas_handle,
    CEBoolean	  render)
#endif /* _NO_PROTO */
{
    CECanvasStruct *canvas = (CECanvasStruct *) canvas_handle;

    if (canvas->toc_flag == True && canvas->toc_on == True)
      {
	if (render)
            (void) _DtHelpCeDrawText (canvas, canvas->txt_lst[canvas->toc_loc],
			canvas->txt_lst[canvas->toc_loc].label_x, -1,
				((Flags)CETocTopicFlag), 0);
        canvas->toc_on = False;
      }

} /* End _DtHelpCeTurnTocMarkOff */

/*****************************************************************************
 *		Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	int _DtHelpCeSetTocPath (CanvasHandle canvas,
 *			VolumeHandle volume, id);
 *
 * Parameters:
 *		canvas		Specifies the handle for the canvas.
 *
 * Returns:	-1 if errors.
 *		 0 if no errors.  ret_valid contains an OR of
 *		   flag CEDimensions and CETocPoints. If CEDimensions
 *		   is set, ret_width and ret_height are valid. If
 *		   CETocPoints is set, ret_toc_y and ret_toc_height
 *		   are valid.
 *
 * Purpose:
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeSetTocPath (canvas_handle, volume, id, render, ret_width, ret_height,
					ret_toc_y, ret_toc_height, ret_valid)
    CanvasHandle  canvas_handle;
    VolumeHandle  volume;
    char         *id;
    CEBoolean	  render;
    Unit	 *ret_width;
    Unit	 *ret_height;
    Unit	 *ret_toc_y;
    Unit	 *ret_toc_height;
    Unit	 *ret_valid;
#else
_DtHelpCeSetTocPath (
    CanvasHandle  canvas_handle,
    VolumeHandle  volume,
    char         *id,
    CEBoolean	  render,
    Unit	 *ret_width,
    Unit	 *ret_height,
    Unit	 *ret_toc_y,
    Unit	 *ret_toc_height,
    Unit	 *ret_valid)
#endif /* _NO_PROTO */
{
    int             result   = 0;
    char	   *real_id;
    CECanvasStruct *canvas   = (CECanvasStruct *) canvas_handle;
    TopicHandle     topicHandle;
    _DtHelpCeLockInfo lockInfo;

    if (canvas_handle == NULL || volume == NULL || id == NULL)
	return -1;

    /*
     * This becomes my volume; want to ensure that it doesn't
     * get closed out from under me, so tell the system that
     * i'm using it.
     */
    if (_DtHelpCeLockVolume(canvas_handle, volume, &lockInfo) != 0)
	return -1;

    if (_DtHelpCeUpVolumeOpenCnt(volume) == -1)
      {
	_DtHelpCeUnlockVolume(lockInfo);
        return -1;
      }

    result = _DtHelpCeMapTargetToId(volume, id, &real_id);

    if (result == 0)
      {
	result = _DtHelpCeGetVolumeFlag(volume);
        if (result == 1)
	    result = _DtHelpCeFrmtSdlPathAndChildren(canvas, volume,
							lockInfo.fd,
							real_id, &topicHandle);
        else if (result == 0)
	    result = _DtHelpCeFrmtCcdfPathAndChildren(canvas, volume,
							real_id, &topicHandle);
      }

    if (result == 0)
      {
	/*
	 * doing a SetTopic will clean the information
	 * currently associated with canvas plus close
	 * any volume currently associated with the canvas
	 */
	_DtHelpCeSetTopic (canvas, topicHandle, -1,
						ret_width, ret_height, NULL);
	canvas->toc_flag = True;
	canvas->toc_on   = False;
	canvas->toc_loc  = 0;
	canvas->volume   = volume;

	if (ret_valid != NULL)
	    *ret_valid = 0;

	MarkTocTopic(canvas, real_id, render,
					ret_toc_y, ret_toc_height, ret_valid);
      }

    /*
     * didn't successfully format a path, so close my copy
     * of the volume.
     */
    else
        _DtHelpCeCloseVolume (canvas, volume);

    _DtHelpCeUnlockVolume(lockInfo);
    return result;

} /* End _DtHelpCeSetTocPath */
