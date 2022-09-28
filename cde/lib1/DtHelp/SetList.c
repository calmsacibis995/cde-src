/* $XConsortium: SetList.c /main/cde1_maint/1 1995/07/17 17:51:37 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	SetList.c
 **
 **   Project:     Text Graphic Display Library
 **
 **   Description: This body of code creates the Line and Graphic Tables for
 **		a Display Area.
 **
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
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "DisplayAreaP.h"
#include "CallbacksI.h"
#include "SetListI.h"
#include "XUICreateI.h"

#ifdef NLS16
#include <nl_types.h>
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/******************************************************************************
 *
 * Private defines and variables
 *
 *****************************************************************************/

#define	GROW_SIZE	10
#define	ALL_LINES	-1

/******************************************************************************
 *
 * Private Macros
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
SetMaxPositions (pDAS, max_x, max_y)
	DtHelpDispAreaStruct	*pDAS;
	Unit	 max_x;
	Unit	 max_y;
#else
SetMaxPositions (
	DtHelpDispAreaStruct	*pDAS,
	Unit	 max_x,
	Unit	 max_y)
#endif /* _NO_PROTO */
{
    pDAS->maxX    = max_x;
    pDAS->maxYpos = max_y + pDAS->marginHeight;
}

/******************************************************************************
 *
 * Semi Public Functions - these routines called by other modules within
 *			   the library.
 *
 *****************************************************************************/
/******************************************************************************
 * Function: _DtHelpSetScrollBars
 *
 * Sets/Resets the vertical scroll bar values.
 *****************************************************************************/
enum   DoSomething  { DoNothing, DoManage, DoUnmanage, DoManageNomap };
Boolean
#ifdef _NO_PROTO
_DtHelpSetScrollBars (pDAS, new_width, new_height)
	DtHelpDispAreaStruct	*pDAS;
	Dimension	 new_width;
	Dimension	 new_height;
#else
_DtHelpSetScrollBars (
	DtHelpDispAreaStruct	*pDAS,
	Dimension	 new_width,
	Dimension	 new_height)
#endif /* _NO_PROTO */
{
    int    n = 0;
    int    dispN = 0;

    int    changeWidth = 0;
    int    changeHeight = 0;

    int    slideSize;
    int    maximum;
    int    pageInc;
    int    oldScrollPercent = 10000;

    Unit   ret_width;
    Unit   ret_height;

    Arg    dispArgs[10];
    Arg    args[10];

    Boolean    changed = False;

    Dimension	 marginWidth;
    Dimension	 marginHeight;
    Dimension	 horzHeight;
    Dimension	 vertWidth;

    XRectangle	 rectangle[1];

    enum DoSomething doVert = DoNothing;
    enum DoSomething doHorz = DoNothing;

    /*
     * remember the old first line.
     */
    if (pDAS->firstVisible && pDAS->maxYpos)
        oldScrollPercent = pDAS->firstVisible * 10000 / pDAS->maxYpos;

    /*
     * get the width and height of the scroll bars.
     */
    if (pDAS->vertScrollWid)
      {
        XtSetArg (args[0], XmNwidth , &vertWidth);
        XtGetValues (pDAS->vertScrollWid, args, 1);
      }

    if (pDAS->horzScrollWid)
      {
        XtSetArg (args[0], XmNheight , &horzHeight);
        XtGetValues (pDAS->horzScrollWid, args, 1);
      }

    /*
     * Settle the sizing issue before we remap the scroll bars.
     */
    do
      {
	/*
	 * set the height.
	 */
	if (new_height != pDAS->dispHeight)
	  {
	    changed = True;
	    pDAS->dispHeight    = new_height;
	    pDAS->dispUseHeight = new_height - 2 * pDAS->decorThickness;
	    if (((int) new_height) < 2 * ((int) pDAS->decorThickness))
	        pDAS->dispUseHeight = 0;

	    pDAS->visibleCount  = ((int) pDAS->dispUseHeight) /
				   pDAS->lineHeight;

	    if (pDAS->visibleCount < 0)
	        pDAS->visibleCount = 0;
	  }

	/*
	 * Check to see if we need the vertical scrollbar
	 */
	if (pDAS->vertScrollWid)
	  {
	    if (pDAS->maxYpos <= ((int) pDAS->dispUseHeight))
	      {
		if (changeWidth != 1 && pDAS->vertIsMapped &&
		_DtHelpIS_AS_NEEDED (pDAS->neededFlags, _DtHelpVERTICAL_SCROLLBAR))
		  {
		    changeWidth = 1;
		    new_width += vertWidth;
		  }
	      }
	    else if (pDAS->vertIsMapped == False && changeWidth != -1)
	      {
		changeWidth = -1;
		if (new_width > vertWidth)
		    new_width -= vertWidth;
		else
		    new_width = 0;
	      }
	  }

	/*
	 * check to see if we've changed width
	 */
	if (new_width != pDAS->dispWidth)
	  {
	    /*
	     * set the new width
	     */
	    changed = True;
	    pDAS->dispWidth    = new_width;
	    pDAS->dispUseWidth = new_width - 2 * pDAS->decorThickness;
	    if (((int)new_width) < 2 * ((int) pDAS->decorThickness))
		pDAS->dispUseWidth = 0;

	    /*
	     * re-layout the information.
	     */
	    if (_DtHelpCeResizeCanvas(pDAS->canvas,&ret_width,&ret_height) == True)
	        SetMaxPositions (pDAS, ret_width, ret_height);

	  }
	if (pDAS->horzScrollWid)
	  {
	    /*
	     * find out if we need the horizontal scrollbar.
	     */
	    if (pDAS->maxX <= ((int) pDAS->dispUseWidth))
	      {
	        if (changeHeight != 1 && pDAS->horzIsMapped &&
			_DtHelpIS_AS_NEEDED (pDAS->neededFlags,
						_DtHelpHORIZONTAL_SCROLLBAR))
		  {
			changeHeight = 1;
			new_height += horzHeight;
		  }
	      }
	    else if (changeHeight != -1 && pDAS->horzIsMapped == False)
	      {
	        changeHeight = -1;
		if (new_height > horzHeight)
		    new_height -= horzHeight;
		else
		    new_height = 0;
	      }
          }

      } while (new_height != pDAS->dispHeight);

    /*
     * If I've changed my size, I've got to change my clip masks.
     */
    if (changed)
      {
	/*
	 * If the display area hasn't been realized, the resetting of
	 * the bottom right corner offsets will not cause a resize
	 * event to occur. Rather the overall window will grow by
	 * the width or height of the scroll bar and not cause the
	 * display area to shrink. Therefore when the exposure event
	 * is called the width or height will be not be equal to what
	 * I think it should be causing the code to believe a resize
	 * is in process. The short of the matter is that the exposure
	 * will not happen and we end up with a blank window.
	 *
	 * So for this case, resize the display area to what we think it
	 * should be and the exposure event will then happen.
	if (!XtIsRealized (pDAS->dispWid))
	 */
          {
	    XtSetArg(dispArgs[dispN], XmNwidth, pDAS->dispWidth);    ++dispN;
	    XtSetArg(dispArgs[dispN], XmNheight, pDAS->dispHeight);  ++dispN;
          }

	rectangle[0].x      = pDAS->decorThickness;
	rectangle[0].y      = pDAS->decorThickness;
	rectangle[0].width  = pDAS->dispUseWidth;
	rectangle[0].height = pDAS->dispUseHeight;

	XSetClipRectangles(XtDisplay(pDAS->dispWid), pDAS->normalGC, 0, 0,
					rectangle, 1, Unsorted);
	XSetClipRectangles(XtDisplay(pDAS->dispWid), pDAS->invertGC, 0, 0,
					rectangle, 1, Unsorted);

	if (pDAS->resizeCall)
	    (*(pDAS->resizeCall)) (pDAS->clientData);
      }


    /*
     * reset the first visible line, if needed.
     */
    if (oldScrollPercent != 10000)
	pDAS->firstVisible = pDAS->maxYpos * oldScrollPercent / 10000;

    if (pDAS->firstVisible + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
	pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;

    if (pDAS->firstVisible < 0)
        pDAS->firstVisible = 0;


    /*
     * Set the arguments accordingly.
     */
    if (pDAS->vertScrollWid)
      {
	/*
	 * determine if we have to map or unmap the vertical scrollbar
	 */
	if (pDAS->maxYpos > ((int) pDAS->dispUseHeight))
	  {
	    if (pDAS->vertIsMapped == False)
	      {
		doVert = DoManage;

	        XtSetArg(dispArgs[dispN], XmNrightAttachment, XmATTACH_WIDGET);
				++dispN;
	        XtSetArg(dispArgs[dispN], XmNrightWidget, pDAS->vertScrollWid);
				++dispN;
	      }
	  }
	else if (_DtHelpIS_AS_NEEDED (pDAS->neededFlags, _DtHelpVERTICAL_SCROLLBAR)
					&& pDAS->vertIsMapped)
	  {
	    doVert = DoUnmanage;
	    XtSetArg(dispArgs[dispN], XmNrightAttachment, XmATTACH_FORM); dispN++;
	  }
      }

    /*
     * determine if we need the horizontal scroll bar.
     */

    if (pDAS->horzScrollWid)
      {
	if (pDAS->maxX > ((int) pDAS->dispUseWidth))
	  {
	    if (pDAS->horzIsMapped == False)
	      {
		doHorz = DoManage;

	        XtSetArg(dispArgs[dispN], XmNbottomAttachment,XmATTACH_WIDGET); dispN++;
	        XtSetArg(dispArgs[dispN], XmNbottomWidget,pDAS->horzScrollWid); dispN++;
	      }
	  }
	else if (_DtHelpIS_AS_NEEDED (pDAS->neededFlags, _DtHelpHORIZONTAL_SCROLLBAR)
					&& pDAS->horzIsMapped)
	  {
	    doHorz = DoUnmanage;
	    XtSetArg(dispArgs[dispN], XmNbottomAttachment, XmATTACH_FORM); dispN++;
	  }
      }

    /*
     * get the margins, widths and height.
     */
    XtSetArg (args[0], XmNmarginWidth , &marginWidth);
    XtSetArg (args[1], XmNmarginHeight, &marginHeight);
    XtGetValues (XtParent (pDAS->dispWid), args, 2);

    /*
     * have to map the scroll bars before the drawn button can be
     * attached to them and can't unmap the scroll bars until
     * the drawn button has been unattached from them. But we
     * want to do the re-attachments all at once to minimize the
     * resizing.
     */
    if (doHorz == DoManage)
      {
	if (!XtIsRealized(pDAS->horzScrollWid))
	    XtSetMappedWhenManaged (pDAS->horzScrollWid, True);
	else
	    XtMapWidget (pDAS->horzScrollWid);
	pDAS->horzIsMapped = True;
      }

    if (doVert == DoManage)
      {
	if (!XtIsRealized(pDAS->vertScrollWid))
	    XtSetMappedWhenManaged (pDAS->vertScrollWid, True);
	else
	    XtMapWidget (pDAS->vertScrollWid);
	pDAS->vertIsMapped = True;
      }

    if (pDAS->vertScrollWid && pDAS->vertIsMapped && doVert != DoUnmanage)
      {
	if (pDAS->horzScrollWid && pDAS->horzIsMapped && doHorz != DoUnmanage)
	    XtSetArg(args[0], XmNbottomOffset, (marginHeight + horzHeight));
	else
	    XtSetArg(args[0], XmNbottomOffset, marginHeight);

	n = 1;

	/*
	 * set the slider size.
	 */
	slideSize = pDAS->dispUseHeight;
	if (slideSize < 1)
    	slideSize = 1;

	/*
	 * determine the maximum size
	 */
	maximum = pDAS->maxYpos;

	if (maximum < slideSize)
    	maximum = slideSize;

	if (maximum < 1)
    	maximum = 1;

	/*
	 * determine the page increment.
	 */
	pageInc = 1;
	if (((int)pDAS->dispUseHeight) - pDAS->lineHeight > 2)
    	pageInc = pDAS->dispUseHeight - pDAS->lineHeight;

	XtSetArg (args[n], XmNvalue        , pDAS->firstVisible);	n++;
	XtSetArg (args[n], XmNsliderSize   , slideSize);		n++;
	XtSetArg (args[n], XmNmaximum      , maximum);			n++;
	XtSetArg (args[n], XmNpageIncrement, pageInc);			n++;
	XtSetValues (pDAS->vertScrollWid, args, n);
      }

    if (pDAS->horzScrollWid && pDAS->horzIsMapped && doHorz != DoUnmanage)
      {
	if (pDAS->vertScrollWid && pDAS->vertIsMapped && doVert != DoUnmanage)
	    XtSetArg(args[0], XmNrightOffset, (marginWidth + vertWidth));
	else
	    XtSetArg(args[0], XmNrightOffset, marginWidth);

	n = 1;
	/*
	 * determine the slider size.
	 */
	slideSize = pDAS->dispUseWidth - 1;
	if (slideSize < 1)
    	slideSize = 1;

	/*
	 * determine the maximum size
	 */
	maximum = slideSize;
	if (maximum < pDAS->maxX)
    	maximum = pDAS->maxX - 1;

	if (maximum < 1)
    	maximum = 1;

	/*
	 * determine the page increment.
	 */
	pageInc = 1;
	if (((int) pDAS->dispUseWidth) > 2)
    	pageInc = pDAS->dispUseWidth - 1;

	/*
	 * Reset the virtual X value, if should changed.
	 */
	if (pDAS->virtualX &&
    	(pDAS->maxX - pDAS->virtualX < ((int) pDAS->dispUseWidth)))
	  {
            pDAS->virtualX = pDAS->maxX - pDAS->dispUseWidth;
            if (pDAS->virtualX < 0)
              pDAS->virtualX = 0;
	  }

	XtSetArg (args[n], XmNvalue        , pDAS->virtualX);   n++;
	XtSetArg (args[n], XmNsliderSize   , slideSize);        n++;
	XtSetArg (args[n], XmNmaximum      , maximum);          n++;
	XtSetArg (args[n], XmNpageIncrement, pageInc);		n++;
	XtSetValues (pDAS->horzScrollWid, args, n);
      }

    /*
     * set the display area args
     */
    if (dispN)
      XtSetValues (pDAS->dispWid, dispArgs, dispN);


    /*
     * unmap those we don't want.
     */
    if (doHorz == DoUnmanage)
      {
	if (!XtIsRealized(pDAS->horzScrollWid))
	    XtSetMappedWhenManaged (pDAS->horzScrollWid, False);
	else
	    XtUnmapWidget(pDAS->horzScrollWid);
        pDAS->horzIsMapped = False;
      }

    if (doVert == DoUnmanage)
      {
	if (!XtIsRealized(pDAS->vertScrollWid))
	    XtSetMappedWhenManaged (pDAS->vertScrollWid, False);
	else
	    XtUnmapWidget (pDAS->vertScrollWid);
	pDAS->vertIsMapped = False;
      }

    /*
     * return whether or not we generated a resize (and hence an expose
     * event) by resizing the display area.
     */

    return (dispN ? True : False);

} /* End _DtHelpSetScrollBars */

/******************************************************************************
 *
 * Public Functions - those an application developer can call.
 *
 *****************************************************************************/

/*********************************************************************
 * Function: SetList
 *
 *    SetList creates a Text Graphic area with the appropriate scroll bars.
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDisplayAreaSetList (pDAS, topic_handle, append_flag, scroll_percent)
    DtHelpDispAreaStruct	*pDAS;
    TopicHandle			 topic_handle;
    Boolean			 append_flag;
    int				 scroll_percent;
#else
_DtHelpDisplayAreaSetList (
    DtHelpDispAreaStruct	*pDAS,
    TopicHandle			 topic_handle,
    Boolean			 append_flag,
    int				 scroll_percent)
#endif /* _NO_PROTO */
{
    Unit              width;
    Unit	      height;
    Unit	      scrollY;

    /*
     * clear the selection
     */

    _DtHelpClearSelection ( pDAS );

    /*
     * If there is anything to do
     */

    _DtHelpCeSetTopic (pDAS->canvas, topic_handle, scroll_percent,
			&width, &height, &scrollY);

    /* -------------------------------------------------------------------------- */
    /*  When setting selection and the new rendered text fits within the visible  */
    /*  portion of the display area, the width needs to be adjusted to trigger a  */
    /*  resize on the display area which will redisplay the text in visible area  */
    /* -------------------------------------------------------------------------- */

    if (pDAS->dispWid != NULL &&
        XtIsManaged(pDAS->dispWid) && XtWindow(pDAS->dispWid))
      width += 1;

    /*
     * Reset the scroll bars and perhaps do an expose.
     */

    SetMaxPositions (pDAS, width, height);

    pDAS->firstVisible = scrollY;

    if (pDAS->firstVisible + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
	pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;

    if (pDAS->firstVisible < 0)
        pDAS->firstVisible = 0;

    (void) _DtHelpSetScrollBars (pDAS, pDAS->dispWidth, pDAS->dispHeight);
    if (XtIsRealized (pDAS->dispWid))
      _DtHelpCleanAndDrawWholeCanvas(pDAS);

    /* ---------------------------------------------------------------------------- */
    /*  Resize the display area to its expected size - only when setting selection  */
    /* ---------------------------------------------------------------------------- */

    if (pDAS->dispWid != NULL &&
        XtIsManaged(pDAS->dispWid) && XtWindow(pDAS->dispWid))
      {
        width -= 1;

        /*
         * Reset the scroll bars and perhaps do an expose.
         */

        SetMaxPositions (pDAS, width, height);

        pDAS->firstVisible = scrollY;

        if (pDAS->firstVisible + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
          pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;

        if (pDAS->firstVisible < 0)
          pDAS->firstVisible = 0;

        (void) _DtHelpSetScrollBars (pDAS, pDAS->dispWidth, pDAS->dispHeight);
        if (XtIsRealized (pDAS->dispWid))
          _DtHelpCleanAndDrawWholeCanvas(pDAS);
      }

    return;
}  /* End _DtHelpDisplayAreaSetList */

/*****************************************************************************
 * Function: void _DtHelpDisplayAreaDimensionsReturn (DtHelpDispAreaStruct *pDAS,
 *					int *ret_width,	int *ret_height)
 *
 * Parameters	pDAS		Specifies the Display Area.
 *		ret_rows	Returns the number of rows in
 *				the Display Area.
 *		ret_columns	Returns the number of columns in
 *				the Display Area.
 *
 * Returns:	nothing
 *
 * Purpose:	Allows access to the height and width of a Display Area.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDisplayAreaDimensionsReturn (pDAS, ret_rows, ret_columns)
    DtHelpDispAreaStruct	*pDAS;
    short			*ret_rows;
    short			*ret_columns;
#else
_DtHelpDisplayAreaDimensionsReturn (
    DtHelpDispAreaStruct	*pDAS,
    short			*ret_rows,
    short			*ret_columns )
#endif /* _NO_PROTO */
{
    *ret_rows = ((int)pDAS->dispUseHeight) / pDAS->lineHeight;
    *ret_columns = ((int) pDAS->dispUseWidth) / (pDAS->charWidth / 10);

}  /* End _DtHelpDisplayAreaDimensionsReturn */

/*****************************************************************************
 * Function: int _DtHelpGetScrollbarValue (DtHelpDispAreaStruct *pDAS)
 *
 * Parameters	pDAS		Specifies the Display Area.
 *
 * Returns:	a value from -1 to 100.
 *		-1 means the vertical scrollbar was not created when
 *		the display area was created.
 *		0 to 100 is the percentage the vertical scroll bar is
 *		scrolled displaying the current topic.
 *
 * Purpose:	Allows the system to query the location of the scrollbar
 *		and remember for smart-backtracking.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpGetScrollbarValue (pDAS)
    DtHelpDispAreaStruct	*pDAS;
#else
_DtHelpGetScrollbarValue (
    DtHelpDispAreaStruct	*pDAS)
#endif /* _NO_PROTO */
{
    int  percent = -1;

    if (pDAS->vertScrollWid != NULL)
      {
	percent = 0;
	if (pDAS->firstVisible && pDAS->maxYpos)
	    percent = (pDAS->firstVisible * 1000) / pDAS->maxYpos;
      }

    return percent;

}  /* End _DtHelpGetScrollbarValue */

/*****************************************************************************
 * Function: int _DtHelpUpdatePath (DtHelpDispAreaStruct *pDAS,
 *
 * Parameters	pDAS		Specifies the Display Area.
 *
 * Returns:	a value from -1 to 100.
 *		-1 means the vertical scrollbar was not created when
 *		the display area was created.
 *		0 to 100 is the percentage the vertical scroll bar is
 *		scrolled displaying the current topic.
 *
 * Purpose:	Allows the system to query the location of the scrollbar
 *		and remember for smart-backtracking.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpUpdatePath (pDAS, volume_handle, loc_id)
    DtHelpDispAreaStruct	*pDAS;
    VolumeHandle		 volume_handle;
    char			*loc_id;
#else
_DtHelpUpdatePath (
    DtHelpDispAreaStruct	*pDAS,
    VolumeHandle		 volume_handle,
    char			*loc_id)
#endif /* _NO_PROTO */
{
    int		result;
    Unit	width;
    Unit	height;
    Unit	tocY;
    Unit	tocHeight;
    Unit	valid = 0;

    result = _DtHelpCeSetTocPath(pDAS->canvas, volume_handle, loc_id,
			((CEBoolean) XtIsRealized(pDAS->dispWid)),
			&width, &height, &tocY, &tocHeight, &valid);
    if (result != -1)
      {
        /*
         * Reset the scroll bars and perhaps do an expose.
         */
        SetMaxPositions (pDAS, width, height);

	/*
	 * figure out where the first line should be...
	 */
	if (valid & CETocPoints)
	  {
	    tocY      -= pDAS->lineThickness;
	    tocHeight += (2 * pDAS->lineThickness);

	    if (tocY < pDAS->firstVisible ||
			tocY + tocHeight >
				pDAS->firstVisible + ((int)pDAS->dispUseHeight))
	      {
	        pDAS->firstVisible  = tocY;
	        if (pDAS->firstVisible + ((int)pDAS->dispUseHeight)
						> pDAS->maxYpos)
		    pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;

	        if (pDAS->firstVisible < 0)
		    pDAS->firstVisible = 0;
	      }

	    /*
	     * move the traversal box
	     */
	    _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False,
			((CEBoolean) XtIsRealized(pDAS->dispWid)),
					loc_id, NULL, NULL, NULL);
          }

        /*
         * Reset the scroll bars and perhaps do an expose.
         */
        (void) _DtHelpSetScrollBars(pDAS,pDAS->dispWidth,pDAS->dispHeight);
        if (XtIsRealized (pDAS->dispWid))
	    _DtHelpCleanAndDrawWholeCanvas(pDAS);
	 else if ((valid & CETocPoints) &&
		tocY + tocHeight >
			pDAS->firstVisible + ((int)pDAS->dispUseHeight))
	  {
	    pDAS->firstVisible  = tocY;
	    if (pDAS->firstVisible + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
		pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;
	    if (pDAS->firstVisible < 0)
		pDAS->firstVisible = 0;

	    (void) _DtHelpSetScrollBars(pDAS,pDAS->dispWidth,pDAS->dispHeight);
	  }
      }

    return result;

}  /* End _DtHelpUpdatePath */
