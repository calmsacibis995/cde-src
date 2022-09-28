/* $XConsortium: Callbacks.c /main/cde1_maint/1 1995/07/17 17:22:33 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Callbacks.c
 **
 **   Project:     Display Area Library
 **
 **
 **   Description: This body of code handles the callbacks for the
 **                Display Area.
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
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DrawnB.h>
#include <Xm/CutPaste.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "DisplayAreaP.h"
#include "CallbacksI.h"
#include "FontI.h"
#include "HyperTextI.h"
#include "SetListI.h"
#include "HelposI.h"
#include "XInterfaceI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	Boolean	 ConvertSelectionCB ();
static	void	 ScrollTimerCB ();
static	void	 StartSelection ();
#else
static	Boolean	 ConvertSelectionCB (
			Widget		 widget,
			Atom		*selection,
			Atom		*target,
			Atom		*type,
			XtPointer	*value,
			unsigned long	*length,
			int		*format );
static	void	 ScrollTimerCB (
			XtPointer	 client_data,
			XtIntervalId	*id );
static	void	 StartSelection (
			Widget		widget,
			XtPointer	client_data );
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    Private Defines                 ********/
#define	SCROLL_BAR_FLAGS 0x03
#define	HORIZONTAL	 0
#define	VERTICAL	 1

/********    End Private Defines             ********/

/********    Private Variable Declarations    ********/

static	struct {
	int	horizontal_reason;
	int	vertical_reason;
	int	scrollbar;
} SelectionScroll;

/********    End Private Variable Declarations    ********/

/******************************************************************************
 *                             Private Functions
 ******************************************************************************/
/*****************************************************************************
 * Function: ConvertSelectionCB
 *
 *    ConvertSelectionCB - this routine is called when someone asks for
 *				our selection.
 *
 *****************************************************************************/
static	Boolean
#ifdef _NO_PROTO
ConvertSelectionCB (widget, selection, target, type, value, length, format )
	Widget		 widget;
	Atom		*selection;
	Atom		*target;
	Atom		*type;
	XtPointer	*value;
	unsigned long	*length;
	int		*format;
#else
ConvertSelectionCB (
	Widget		 widget,
	Atom		*selection,
	Atom		*target,
	Atom		*type,
	XtPointer	*value,
	unsigned long	*length,
	int		*format )
#endif /* _NO_PROTO */
{
    Atom  TARGETS    = XmInternAtom(XtDisplay(widget), "TARGETS"  , False);
    Atom  TIMESTAMP  = XmInternAtom(XtDisplay(widget), "TIMESTAMP", False);
    Atom  TEXT       = XmInternAtom(XtDisplay(widget), "TEXT"     , False);
    Atom  CMP_TEXT   = XmInternAtom(XtDisplay(widget), "COMPOUND_TEXT",False);
    Atom  LOCALE;
    int   retStatus  = 0;
    char *testString = "ABC";  /* these are characters in XPCS, so... safe */
    char *tmpString  = NULL;
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;

    XTextProperty  tmpProp;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS == NULL || pDAS->primary == False || *selection != XA_PRIMARY)
	return False;

    retStatus = XmbTextListToTextProperty(XtDisplay(widget), &testString, 1,
				(XICCEncodingStyle)XTextStyle, &tmpProp);

    LOCALE = (Atom) 9999; /* XmbTextList... should always be able
				     * to convert XPCS characters; but in
				     * case its broken, this prevents a core
				     * dump.
				     */
    if (retStatus == Success)
      {
	LOCALE = tmpProp.encoding;
	XFree(tmpProp.value);
      }

    /*
     * List the targets understood
     */
    if (*target == TARGETS)
      {
	Atom *targs = (Atom *)XtMalloc((unsigned) (6 * sizeof(Atom)));

	*value   = (char *) targs;
	*targs++ = LOCALE;
	*targs++ = TARGETS;
	*targs++ = TIMESTAMP;
	*targs++ = TEXT;
	*targs++ = CMP_TEXT;
	*targs++ = XA_STRING;

	*type   = XA_ATOM;
	*length = (6 * sizeof(Atom)) >> 2;
	*format = 32;
      }
    else if (*target == TIMESTAMP)
      {
	Time *timestamp;
	timestamp  = (Time *) XtMalloc(sizeof(Time));
	*timestamp = pDAS->anchor_time;

	*value  = (char *) timestamp;
	*type   = XA_INTEGER;
	*length = sizeof(Time);
	*format = 32;
      }
    else if (*target == XA_STRING)
      {
	/*
	 * initialize the return type here in case we fail
	 */
	*type   = (Atom) XA_STRING;
	*format = 8;

	/*
	 * get the string
	 */
	_DtHelpCeGetSelectionData (pDAS->canvas, CESelectedText,
						(SelectionInfo *)(&tmpString));
	retStatus = -1;
	if (tmpString != NULL && *tmpString != '\0')
	    retStatus = XmbTextListToTextProperty(XtDisplay(widget),
				&tmpString, 1,
				(XICCEncodingStyle)XStringStyle, &tmpProp);

	/*
	 * free the original copy of the string and check the results
	 */
	if (tmpString != NULL)
	    free(tmpString);

	if (retStatus == Success || retStatus > 0)
	  {
	    *value  = (XtPointer) tmpProp.value;
	    *length = tmpProp.nitems;
	  }
	else
	  {
	    *value = NULL;
	    *length = 0;
	    return False;
	  }
      }
    else if (*target == TEXT)
      {
	/*
	 * set the type and format to those calculated
	 */
	*type   = tmpProp.encoding;	/* STRING or COMPOUND_TEXT */
	*format = tmpProp.format;

	/*
	 * get the string
	 */
	_DtHelpCeGetSelectionData (pDAS->canvas, CESelectedText,
						(SelectionInfo *)(&tmpString));

	retStatus = -1;
	if (tmpString != NULL && *tmpString != '\0')
	    retStatus = XmbTextListToTextProperty(XtDisplay(widget),
				&tmpString, 1,
				(XICCEncodingStyle)XStdICCTextStyle, &tmpProp);

	/*
	 * free the original copy of the string and check the results
	 */
	if (tmpString != NULL)
	    free(tmpString);

	if (retStatus == Success || retStatus > 0)
	  {
	    *value  = (XtPointer) tmpProp.value;
	    *length = tmpProp.nitems;
	  }
	else
	  {
	    *value = NULL;
	    *length = 0;
	    return False;
	  }
      }
    else if (*target == LOCALE)
      {
	/*
	 * pass the string straight through
	 */
	*type   = LOCALE;
	*format = 8;

	_DtHelpCeGetSelectionData (pDAS->canvas, CESelectedText,
						(SelectionInfo *)(value));
	*length = 0;
	if (*value != NULL)
	    *length = strlen((char *)(*value));
      }
    else if (*target == CMP_TEXT)
      {
	*type   = CMP_TEXT;
	*format = 8;

	/*
	 * get the selected text.
	 */
	_DtHelpCeGetSelectionData (pDAS->canvas, CESelectedText,
						(SelectionInfo *)(&tmpString));

	retStatus = -1;
	if (tmpString != NULL && *tmpString != '\0')
	    retStatus = XmbTextListToTextProperty(XtDisplay(widget),
			&tmpString, 1,
			(XICCEncodingStyle)XCompoundTextStyle, &tmpProp);

	/*
	 * free the original copy of the string and check the results
	 */
	if (tmpString != NULL)
	    free(tmpString);

	if (retStatus == Success || retStatus > 0)
	  {
	    *value  = (XtPointer) tmpProp.value;
	    *length = tmpProp.nitems;
	  }
	else
	  {
	    *value = NULL;
	    *length = 0;
	    return False;
	  }
      }
    else
	return False;

    return True;

} /* End ConvertSelectionCB */

/*****************************************************************************
 * Function: ScrollTimerCB
 *
 *    ScrollTimerCB - This routine is called when we have a timer
 *		go off with the mouse outside the Display Area during a
 *		selection.
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
ScrollTimerCB (client_data, id)
	XtPointer	 client_data;	/*  data from applicaiton   */
	XtIntervalId	*id;		/*  timer id                */
#else
ScrollTimerCB (
	XtPointer	 client_data,	/*  data from applicaiton   */
	XtIntervalId	*id )		/*  timer id                */
#endif /* _NO_PROTO */
{
    int    diffY = 0;
    int    diffX = 0;
    int    x;
    int    y;
    int    maxY;
    int    dispY;
    Arg    args[2];
    XmScrollBarCallbackStruct callData;
    DtHelpDispAreaStruct  *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (*id != pDAS->scr_timer_id)
      return;

    pDAS->scr_timer_id = NULL;

    maxY  = pDAS->maxYpos;
    dispY = pDAS->firstVisible + pDAS->dispUseHeight;

    if ((SelectionScroll.vertical_reason == XmCR_INCREMENT && maxY <= dispY)
		||
	(SelectionScroll.vertical_reason == XmCR_DECREMENT &&
		!pDAS->firstVisible))
	SelectionScroll.vertical_reason = XmCR_NONE;

    if ((SelectionScroll.horizontal_reason == XmCR_INCREMENT &&
	pDAS->maxX <= pDAS->virtualX + ((int)pDAS->dispUseWidth))
		||
	(SelectionScroll.horizontal_reason == XmCR_DECREMENT &&
		!pDAS->virtualX))
	SelectionScroll.horizontal_reason = XmCR_NONE;

    if ( SelectionScroll.vertical_reason == XmCR_NONE &&
		SelectionScroll.horizontal_reason == XmCR_NONE)
	return;

    y = pDAS->firstVisible;
    if (SelectionScroll.vertical_reason == XmCR_NONE)
	y = pDAS->scr_timer_y - pDAS->decorThickness;
    else if (SelectionScroll.vertical_reason == XmCR_INCREMENT)
      {
        y = y + pDAS->dispUseHeight + pDAS->lineHeight;
	if (y > pDAS->maxYpos)
	    y = pDAS->maxYpos;

        diffY = y - pDAS->firstVisible - pDAS->dispUseHeight;
      }
    else if (SelectionScroll.vertical_reason == XmCR_DECREMENT)
      {
	y -= pDAS->lineHeight;
	if (y < 0)
	    y = 0;

        diffY = y - pDAS->firstVisible;
      }

    if (SelectionScroll.horizontal_reason == XmCR_NONE)
	x = pDAS->scr_timer_x - pDAS->decorThickness;
    else
      {
	if (SelectionScroll.horizontal_reason == XmCR_INCREMENT)
          {
	    diffX = (int) (pDAS->charWidth / 10);
	    x = pDAS->dispUseWidth;

	    if (x + pDAS->virtualX + diffX > pDAS->maxX)
		diffX = pDAS->maxX - x - pDAS->virtualX;
          }
	else if (SelectionScroll.horizontal_reason == XmCR_DECREMENT)
          {
	    diffX = -((int)(pDAS->charWidth / 10));
	    x = 0;

	    if (pDAS->virtualX + diffX < 0)
		diffX = -(pDAS->virtualX);
          }
      }

    _DtHelpCeProcessSelection (pDAS->canvas,
			(x + diffX + pDAS->virtualX - pDAS->decorThickness),
			y, CESelectionUpdate);

    if (diffX)
      {
	callData.reason = SelectionScroll.horizontal_reason;
	callData.value  = pDAS->virtualX + diffX;
	_DtHelpHorzScrollCB (pDAS->horzScrollWid, client_data,
					(XtPointer) &callData);

	if (pDAS->horzScrollWid)
	  {
	    XtSetArg (args[0], XmNvalue, pDAS->virtualX);
	    XtSetValues (pDAS->horzScrollWid, args, 1);
	  }

	if (diffY != 0 && pDAS->vertScrollWid)
	  {
	    XtSetArg (args[0], XmNvalue, y);
	    XtSetValues (pDAS->vertScrollWid, args, 1);
	  }
      }
    else
      {
	callData.reason = SelectionScroll.vertical_reason;
	callData.value  = pDAS->firstVisible + diffY;
	_DtHelpVertScrollCB (pDAS->vertScrollWid, client_data,
					(XtPointer) &callData);

	if (pDAS->vertScrollWid)
	  {
	    XtSetArg (args[0], XmNvalue, pDAS->firstVisible);
	    XtSetValues (pDAS->vertScrollWid, args, 1);
	  }
      }

    pDAS->scr_timer_id = XtAppAddTimeOut (
		XtWidgetToApplicationContext (pDAS->dispWid),
		((long) XtGetMultiClickTime(XtDisplay(pDAS->dispWid))),
		ScrollTimerCB, client_data);

} /* End ScrollTimerCB */

/******************************************************************************
 * Function: DrawWholeCanvas
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DrawWholeCanvas(pDAS)
    DtHelpDispAreaStruct  *pDAS;
#else
DrawWholeCanvas(
    DtHelpDispAreaStruct  *pDAS)
#endif /* _NO_PROTO */
{
    Unit    top;
    Unit    bottom;

    _DtHelpCeMoveTraversal(pDAS->canvas,CETraversalOff, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
				NULL, NULL, NULL, NULL);

    top     = pDAS->firstVisible;
    bottom  = top + pDAS->dispUseHeight;

    _DtHelpCeScrollCanvas (pDAS->canvas, 0, top, pDAS->dispWidth, bottom);

    /*
     * If we have a hypertext link boxed, draw it
     */
    _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
							NULL, NULL, NULL, NULL);

} /* End DrawWholeCanvas */

/******************************************************************************
 *                          Semi Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function: _DtHelpCleanAndDrawWholeCanvas
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCleanAndDrawWholeCanvas(pDAS)
    DtHelpDispAreaStruct  *pDAS;
#else
_DtHelpCleanAndDrawWholeCanvas(
    DtHelpDispAreaStruct  *pDAS)
#endif /* _NO_PROTO */
{
    XClearArea (XtDisplay(pDAS->dispWid), XtWindow(pDAS->dispWid),
				pDAS->decorThickness, pDAS->decorThickness,
				pDAS->dispUseWidth,
				pDAS->dispUseHeight,
				False);
    DrawWholeCanvas (pDAS);

}

/******************************************************************************
 * Function: _DtHelpCancelSelection
 *
 * Returns : True    if a selection was active and cancelled.
 *           False   if a selection was not active.
 *
 *****************************************************************************/
Boolean
#ifdef _NO_PROTO
_DtHelpCancelSelection(pDAS)
    DtHelpDispAreaStruct  *pDAS;
#else
_DtHelpCancelSelection(
    DtHelpDispAreaStruct  *pDAS)
#endif /* _NO_PROTO */
{
    Boolean selActive = False;

    if (pDAS->select_state == _DtHelpSelectingText && pDAS->primary == True)
      {
	selActive = True;
	if (pDAS->scr_timer_id)
	  {
	    XtRemoveTimeOut (pDAS->scr_timer_id);
	    pDAS->scr_timer_id = NULL;
	  }
	_DtHelpClearSelection (pDAS);
      }

    return selActive;
}

/******************************************************************************
 *                          Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function: _DtHelpExposeCB
 *
 *    _DtHelpExposeCB handles the exposure events for a Text Graphic Area.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpExposeCB(widget, client_data, call_data)
        Widget  widget;
	XtPointer client_data;
	XtPointer call_data;
#else
_DtHelpExposeCB(
        Widget  widget,
	XtPointer client_data,
	XtPointer call_data )
#endif /* _NO_PROTO */
{
  /*
    Arg    args[4];

    Dimension  height;
    Dimension  width;
    */
  DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

  XmDrawnButtonCallbackStruct *callback = (XmDrawnButtonCallbackStruct *) call_data;

  if (callback->reason != XmCR_EXPOSE ||
      (pDAS->neededFlags & (1 << (VisibilityFullyObscured + 3))))
    return;

    /*
     * get the width and height.
     */

    /*
      XtSetArg(args[0], XmNwidth, &width);
      XtSetArg(args[1], XmNheight, &height);
      XtGetValues(widget, args, 2);

      * if this exposure is a result of a resize,
      * wait for the resize to handle it.

      The resize never got the request -

      if (width != pDAS->dispWidth || height != pDAS->dispHeight)
      return;
      */

    if (!(callback->event) || callback->event->xexpose.count)
	return;

    /*
     * re-draw the information in the display area
     */
    DrawWholeCanvas (pDAS);

}  /* End _DtHelpExposeCB */

/*********************************************************************
 * Function: _DtHelpResizeCB
 *
 *    _DtHelpResizeCB handles the exposure events for a Text Graphic Area.
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
_DtHelpResizeCB(widget, client_data, call_data)
        Widget  widget;
	XtPointer client_data;
	XtPointer call_data;
#else
_DtHelpResizeCB(
        Widget  widget,
	XtPointer client_data,
	XtPointer call_data )
#endif /* _NO_PROTO */
{
    int n;
    Arg  args[4];
    Dimension width;
    Dimension height;

    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
    XmDrawnButtonCallbackStruct *callback = (XmDrawnButtonCallbackStruct *) call_data;

    /* --------------------------------------------------------------- */
    /*  Allow for the call_data to be NULL - could be called manually  */
    /* --------------------------------------------------------------- */

    if (callback != NULL && callback->reason != XmCR_RESIZE)
	return;

    /*
     * get the width and height of the form.
     */

    n = 0;
    XtSetArg(args[n], XmNwidth, &width); n++;
    XtSetArg(args[n], XmNheight, &height); n++;
    XtGetValues(XtParent(widget), args, n);

    if (width == pDAS->formWidth && height == pDAS->formHeight)
      return;

    pDAS->formWidth  = width;
    pDAS->formHeight = height;

    /*
     * get the width and height.
     */

    n = 0;
    XtSetArg(args[n], XmNwidth, &width); n++;
    XtSetArg(args[n], XmNheight, &height); n++;
    XtGetValues(widget, args, n);

    if (width == pDAS->dispWidth && height == pDAS->dispHeight)
      return;

    _DtHelpClearSelection (pDAS);

    /*
     * reset the scroll bars and possibly reformat the text for the size.
     */
    (void) _DtHelpSetScrollBars (pDAS, width, height);
    if (XtIsRealized (pDAS->dispWid))
        _DtHelpCleanAndDrawWholeCanvas (pDAS);

    /*
     * I will get an expose event after the resize.
     */

}  /* End _DtHelpResizeCB */

/***************************************************************************
 * Function:  _DtHelpVertScrollCB
 *
 * _DtHelpVertScrollCB is called when the vertical scroll bar is changed.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpVertScrollCB( widget, clientData, callData )
        Widget widget ;
        XtPointer clientData ;
        XtPointer callData ;
#else
_DtHelpVertScrollCB(
        Widget widget,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct      *pDAS     = (DtHelpDispAreaStruct *) clientData;
    XmScrollBarCallbackStruct *callBack =
					(XmScrollBarCallbackStruct *) callData;
    int         diff   = pDAS->lineHeight;
    int         srcY, dstY;
    int         clearY;
    int		reason = callBack->reason;
    Unit        absTop;
    Unit        absBot;
    Display    *dpy;
    Window      win;

    /*
     * if the policy is XmEXPLICIT, don't want the focus on the scrollbar
     */
    if (callBack->event != NULL && callBack->event->type == ButtonPress &&
	      _XmGetFocusPolicy(XtParent(XtParent(pDAS->dispWid))) != XmPOINTER)
	XmProcessTraversal(pDAS->dispWid, XmTRAVERSE_CURRENT);

    /*
     * check to make sure we don't do a rerender when we don't have to.
     */
    if (pDAS->firstVisible == callBack->value)
	return;

    /* If a drag occured, reset the reason to increment, decrement, page    */
    /* increment, or page decrement depending on the distance and direction */
    /* dragged. */
    if (callBack->reason == XmCR_DRAG || callBack->reason == XmCR_VALUE_CHANGED)
      {
	diff = callBack->value - pDAS->firstVisible;

	if (diff > 0 && diff <= ((int) pDAS->dispUseHeight))
            reason = XmCR_INCREMENT;
	else if (diff < 0 && -(diff) <= ((int) pDAS->dispUseHeight))
	  {
            reason = XmCR_DECREMENT;
	    diff   = -diff;
	  }
        else if (diff > ((int) pDAS->dispUseHeight))
            reason = XmCR_PAGE_DECREMENT;
        else
            reason = XmCR_PAGE_INCREMENT;
      }
    else if (callBack->reason == XmCR_INCREMENT ||
					callBack->reason == XmCR_DECREMENT)
      {
	diff = callBack->value - pDAS->firstVisible;
	if (diff < 0)
	    diff = -diff;
      }

    /* Reset first visible to the returned scrollbar value. */
    pDAS->firstVisible = callBack->value;

    /* For page increment and decrement, call _DtHelpCleanAndDrawWholeCanvas
     * to clear the view area and redisplay the text.
     *
     * For increment and decrement,
     * use XCopyArea to move the visible lines and draw the cleared out line.
     */
    if (!pDAS->maxYpos ||
		(pDAS->neededFlags & (1 << (VisibilityFullyObscured + 3))))
        return;

    dpy = XtDisplay (widget);
    win = XtWindow (pDAS->dispWid);

    if (reason == XmCR_PAGE_INCREMENT || reason == XmCR_PAGE_DECREMENT ||
		reason == XmCR_TO_TOP || reason == XmCR_TO_BOTTOM)
	_DtHelpCleanAndDrawWholeCanvas (pDAS);
    else
      {
        if (reason == XmCR_INCREMENT)
	  {
	    dstY   = pDAS->decorThickness;
	    srcY   = dstY + diff;
	    clearY = pDAS->dispHeight - pDAS->decorThickness - diff;
	  }
	else
	  {
	    srcY   = pDAS->decorThickness;
	    dstY   = srcY + diff;
	    clearY = srcY;
	  }
        XCopyArea(dpy, win, win, pDAS->normalGC, pDAS->decorThickness, srcY,
		pDAS->dispUseWidth, (pDAS->dispUseHeight - diff),
		pDAS->decorThickness, dstY);

        XClearArea(dpy, win, pDAS->decorThickness, clearY,
		 pDAS->dispUseWidth, ((unsigned int) diff), False);

	if (pDAS->neededFlags & (1 << (VisibilityPartiallyObscured + 3)))
	  {
	    /*
	     * redraw all the information
	     */
	    DrawWholeCanvas (pDAS);
	  }
	else
	  {
	    /*
	     * draw the line that sits on the cleared line
	     */
	    absTop = clearY + pDAS->firstVisible - pDAS->decorThickness;
	    absBot = absTop + diff;

	    _DtHelpCeScrollCanvas (pDAS->canvas, 0, absTop,
					pDAS->dispWidth, absBot);
	  }
      }

}  /* End _DtHelpVertScrollCB */

/***************************************************************************
 * Function:  _DtHelpHorzScrollCB
 *
 * _DtHelpHorzScrollCB is called when the horizontal scroll bar is changed.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpHorzScrollCB( widget, clientData, callData )
        Widget widget ;
        XtPointer clientData ;
        XtPointer callData ;
#else
_DtHelpHorzScrollCB(
        Widget widget,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) clientData;
    XmScrollBarCallbackStruct *callBack =
					(XmScrollBarCallbackStruct *) callData;
    int		 diff = (int)(pDAS->charWidth / 10);
    int		 srcX;
    int		 dstX;
    int		 clearX;
    int		 reason = callBack->reason;
    Unit	 absLeft;
    Unit	 absRight;
    Unit	 absY;
    Display	*dpy;
    Window	 win;

    /*
     * if the policy is XmEXPLICIT, don't want the focus on the scrollbar
     */
    if (callBack->event != NULL && callBack->event->type == ButtonPress &&
	      _XmGetFocusPolicy(XtParent(XtParent(pDAS->dispWid))) != XmPOINTER)
	XmProcessTraversal(pDAS->dispWid, XmTRAVERSE_CURRENT);

    /*
     * check to make sure we don't do a rerender when we don't have to.
     */
    if (pDAS->virtualX == callBack->value)
	return;

    /* If a drag occured, reset the reason to increment, decrement, page    */
    /* increment, or page decrement depending on the distance and direction */
    /* dragged. */
    if (callBack->reason == XmCR_DRAG || callBack->reason == XmCR_VALUE_CHANGED)
      {
	diff = callBack->value - pDAS->virtualX;

	if (diff > 0 && diff <= ((int) pDAS->dispUseWidth))
            reason = XmCR_INCREMENT;
	else if (diff < 0 && -(diff) <= ((int) pDAS->dispUseWidth))
	  {
            reason = XmCR_DECREMENT;
	    diff   = -diff;
	  }
        else if (diff > ((int) pDAS->dispUseWidth))
            reason = XmCR_PAGE_DECREMENT;
        else
            reason = XmCR_PAGE_INCREMENT;
      }
    else if (callBack->reason == XmCR_INCREMENT ||
					callBack->reason == XmCR_DECREMENT)
      {
	diff = callBack->value - pDAS->virtualX;
	if (diff < 0)
	    diff = -diff;
      }

    /* Reset first visible to the returned scrollbar value. */
    pDAS->virtualX = callBack->value;

    /* For page increment and decrement, call _DtHelpCleanAndDrawWholeCanvas
     * to clear the view area and redisplay the text.
     *
     * For increment and decrement,
     * use XCopyArea to move the visible lines and draw the cleared out line.
     */
    if (!pDAS->maxX || !pDAS->visibleCount ||
		(pDAS->neededFlags & (1 << (VisibilityFullyObscured + 3))))
        return;

    dpy = XtDisplay (widget);
    win = XtWindow (pDAS->dispWid);

    /* For page increment and decrement, clear the view area and call
     * View_DtHelpExposeCB to redisplay the text. For increment and decrement,
     * use XCopyArea to move the visible lines and draw the cleared out line.
     */
    if (reason == XmCR_PAGE_INCREMENT || reason == XmCR_PAGE_DECREMENT ||
		reason == XmCR_TO_TOP || reason == XmCR_TO_BOTTOM)
	_DtHelpCleanAndDrawWholeCanvas (pDAS);
    else
      {
        if (reason == XmCR_INCREMENT)
	  {
	    dstX   = pDAS->decorThickness;
	    srcX   = dstX + diff;
	    clearX = pDAS->dispWidth - pDAS->decorThickness - diff;
	  }
	else
	  {
	    srcX   = pDAS->decorThickness;
	    dstX   = srcX + diff;
	    clearX = srcX;
	  }

        XCopyArea(dpy, win, win, pDAS->normalGC, srcX, pDAS->decorThickness,
			pDAS->dispUseWidth - diff, pDAS->dispUseHeight,
			dstX, pDAS->decorThickness);

        XClearArea(dpy, win, clearX, pDAS->decorThickness,
		((unsigned int) diff), pDAS->dispUseHeight, False);

	if (pDAS->neededFlags & (1 << (VisibilityPartiallyObscured + 3)))
	  {
	    /*
	     * redraw all the information
	     */
	    DrawWholeCanvas (pDAS);
	  }
	else
	  {
	    /*
	     * draw the line that sits on the cleared line
	     */
	    absLeft  = clearX + pDAS->virtualX - pDAS->decorThickness;
	    absRight = absLeft + diff;
	    absY     = pDAS->firstVisible - pDAS->decorThickness;

	    _DtHelpCeScrollCanvas (pDAS->canvas, absLeft, absY,
				absRight, absY + pDAS->dispHeight);

	  }
      }

}  /* End _DtHelpHorzScrollCB */

/***************************************************************************
 * Function:  _DtHelpClickOrSelectCB
 *
 * _DtHelpClickOrSelectCB is called when the vertical scroll bar is changed.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpClickOrSelectCB( widget, clientData, callData )
        Widget widget ;
        XtPointer clientData ;
        XtPointer callData ;
#else
_DtHelpClickOrSelectCB(
        Widget widget,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */
{
    XmDrawnButtonCallbackStruct *callBack =
				(XmDrawnButtonCallbackStruct *) callData;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct*) clientData;

    /*
     * If this is not an ARM call or entered through an Arm&Activate
     * (event-type will be keypress or keyrelease) throw it away.
     */
    if (callBack->reason != XmCR_ARM || callBack->event == NULL ||
	callBack->event->type == KeyPress ||
	callBack->event->type == KeyRelease)
	return;

    pDAS->timerX       = callBack->event->xbutton.x;
    pDAS->timerY       = callBack->event->xbutton.y;
    pDAS->select_state = _DtHelpCopyOrLink;

}  /* End _DtHelpClickOrSelectCB */

/*****************************************************************************
 * Function: _DtHelpEndSelectionCB
 *
 *
 * Called by: Callback for the Selection mechanism
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpEndSelectionCB (w, client_data, call_data)
	Widget		w;		/*  widget id           */
	XtPointer	client_data;	/*  data from applicaiton   */
	XtPointer	call_data;	/*  data from widget class  */
#else
_DtHelpEndSelectionCB (
	Widget		w,		/*  widget id           */
	XtPointer	client_data,	/*  data from applicaiton   */
	XtPointer	call_data )	/*  data from widget class  */
#endif /* _NO_PROTO */
{
    XmDrawnButtonCallbackStruct *callback =
                                (XmDrawnButtonCallbackStruct *) call_data;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct*) client_data;
    int    newX;
    int    newY;

    if (callback->reason != XmCR_DISARM || callback->event == NULL ||
	callback->event->type == KeyPress ||
	callback->event->type == KeyRelease)
        return;

    /*
     * if a scroll timer is active, we are selecting text.
     * stop it.
     */
    if (pDAS->scr_timer_id)
      {
	XtRemoveTimeOut (pDAS->scr_timer_id);
	pDAS->scr_timer_id = NULL;
      }

    newX = callback->event->xbutton.x;
    newY = callback->event->xbutton.y;
    if (pDAS->select_state == _DtHelpCopyOrLink)
      {
	if (abs (pDAS->timerX - newX) <= pDAS->moveThreshold &&
			abs (pDAS->timerY - newY) <= pDAS->moveThreshold)
	  {
	    _DtHelpClearSelection (pDAS);

	    /*
	     * If this is null, we came the the Arm&Activate routine
	     * which means a key was pressed, so we ignore this call.
	     */
	    if (callback->event)
	        /*
	         * find the hypertext link and process it.
	         */
                _DtHelpProcessHyperSelection (pDAS, pDAS->timerX, pDAS->timerY,
							callback->event);
	  }
	else
	    StartSelection (w, client_data);
        return;
      }

    /*
     * The user was doing a selection, finish it up.
     */
    if (pDAS->select_state != _DtHelpNothingDoing)
      {
	_DtHelpCeProcessSelection(pDAS->canvas,
		(newX + pDAS->virtualX - pDAS->decorThickness),
		(newY + pDAS->firstVisible - pDAS->decorThickness),
		CESelectionEnd);
	_DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False, True,
				NULL, NULL, NULL, NULL);

      }

    pDAS->select_state = _DtHelpNothingDoing;
    return;

}  /* End _DtHelpEndSelectionCB */

/***************************************************************************
 * Function:  _DtHelpMouseMoveCB
 *
 * _DtHelpMouseMoveCB tracks the mouse movement for the Selection mechanism
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpMouseMoveCB( widget, client_data, event )
        Widget	   widget ;
        XtPointer  client_data ;
        XEvent	  *event ;
#else
_DtHelpMouseMoveCB(
        Widget	   widget,
        XtPointer  client_data,
        XEvent	  *event )
#endif /* _NO_PROTO */
{
    Unit   newX;
    Unit   newY;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    /*
     * If a selection is not in progress, don't do anything.
     */
    if (pDAS->select_state == _DtHelpNothingDoing || event->type != MotionNotify)
	return;

    if (pDAS->scr_timer_id)
      {
	XtRemoveTimeOut (pDAS->scr_timer_id);
	pDAS->scr_timer_id = NULL;
      }

    newX = event->xmotion.x;
    newY = event->xmotion.y;

    if (pDAS->select_state == _DtHelpCopyOrLink)
      {
	if (abs (newX - pDAS->timerX) < pDAS->moveThreshold &&
			abs (newY - pDAS->timerY) < pDAS->moveThreshold)
	    return;

	StartSelection (widget, client_data);
	return;
      }
    if (newY < ((int) pDAS->decorThickness) && pDAS->firstVisible)
	SelectionScroll.vertical_reason = XmCR_DECREMENT;

    else if ((newY >
	    (((int) pDAS->dispHeight)-((int)pDAS->decorThickness))) &&
	    (pDAS->maxYpos >
		(((int)pDAS->firstVisible)
				+ ((int)pDAS->dispUseHeight))))
	SelectionScroll.vertical_reason = XmCR_INCREMENT;
    else
	SelectionScroll.vertical_reason = XmCR_NONE;

    if (newX < ((int) pDAS->decorThickness) && pDAS->virtualX)
	SelectionScroll.horizontal_reason = XmCR_DECREMENT;
    else if (newX > ((int) pDAS->dispWidth) &&
	    pDAS->maxX > pDAS->virtualX + ((int) pDAS->dispUseWidth))
	SelectionScroll.horizontal_reason = XmCR_INCREMENT;
    else
	SelectionScroll.horizontal_reason = XmCR_NONE;

    if (SelectionScroll.vertical_reason != XmCR_NONE ||
	SelectionScroll.horizontal_reason != XmCR_NONE)
      {
	pDAS->scr_timer_x  = newX;
	pDAS->scr_timer_y  = newY;
	pDAS->scr_timer_id =
			XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
		((long) XtGetMultiClickTime(XtDisplay(pDAS->dispWid))),
				ScrollTimerCB, (XtPointer) pDAS);
	return;
      }

    newX = newX + pDAS->virtualX - pDAS->decorThickness;
    if (newX < 0)
	newX = 0;

    newY = newY + pDAS->firstVisible - pDAS->decorThickness;
    if (newY < 0)
	newY = 0;

    _DtHelpCeProcessSelection(pDAS->canvas, newX, newY, CESelectionUpdate);

} /* End _DtHelpMouseMoveCB */

/*****************************************************************************
 * Function: StartSelection
 *
 *    StartSelection - If this routine is called, the user has initiated a
 *                   selection.
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
StartSelection (widget, client_data)
	Widget		widget;		/*  widget id               */
	XtPointer	client_data;	/*  data from applicaiton   */
#else
StartSelection (
	Widget		widget,		/*  widget id               */
	XtPointer	client_data )	/*  data from applicaiton   */
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct*) client_data;

    /*
     * If this widget doesn't own the primary selection, get it.
     */
    _DtHelpGetClearSelection(widget, client_data);

    /*
     * check to see if we have the primary selection.
     */
    if (pDAS->primary == True)
      {
	_DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOff, False, True,
				NULL, NULL, NULL, NULL);
	_DtHelpCeProcessSelection(pDAS->canvas,
			pDAS->timerX + pDAS->virtualX - pDAS->decorThickness,
			pDAS->timerY + pDAS->firstVisible -pDAS->decorThickness,
			CESelectionStart);

	pDAS->select_state  = _DtHelpSelectingText;
	pDAS->text_selected = True;
      }

}  /* End StartSelection */

/*****************************************************************************
 * Function: _DtHelpLoseSelectionCB
 *
 *    _DtHelpLoseSelectionCB - This routine is called when we lose the selection
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpLoseSelectionCB (widget, selection)
	Widget	 widget;
	Atom	*selection;
#else
_DtHelpLoseSelectionCB (
	Widget	 widget,
	Atom	*selection )
#endif /* _NO_PROTO */
{
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL && pDAS->dispWid == widget && *selection == XA_PRIMARY)
      {
	_DtHelpClearSelection (pDAS);
	pDAS->primary       = False;
	pDAS->text_selected = False;
      }
} /* End _DtHelpLoseSelectionCB */

/*****************************************************************************
 * Function: _DtHelpClearSelection
 *
 *    Clears the selection pointers and variables
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpClearSelection ( pDAS )
    DtHelpDispAreaStruct *pDAS;
#else
_DtHelpClearSelection ( DtHelpDispAreaStruct *pDAS )
#endif /* _NO_PROTO */
{
    if (pDAS->primary == True)
      {
        _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOff, False, True,
				NULL, NULL, NULL, NULL);
        _DtHelpCeProcessSelection(pDAS->canvas, 0, 0, CESelectionClear);
        _DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False, True,
				NULL, NULL, NULL, NULL);
        pDAS->select_state  = _DtHelpNothingDoing;
        pDAS->text_selected = False;
      }
}

/***************************************************************************
 * Function:  _DtHelpFocusCB
 *
 * _DtHelpFocusCB tracks the traversal of the hypertext.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpFocusCB( widget, client_data, event )
        Widget	   widget ;
        XtPointer  client_data ;
        XEvent	  *event ;
#else
_DtHelpFocusCB(
        Widget	   widget,
        XtPointer  client_data,
        XEvent	  *event )
#endif /* _NO_PROTO */
{
    Boolean                oldFlag;
    Boolean                newFlag = False;
    DtHelpDispAreaStruct  *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (pDAS->hyperCall == NULL ||
		(event->type != FocusIn && event->type != FocusOut) ||
		!event->xfocus.send_event)
	return;

    /*
     * get the old flag
     */
    oldFlag = (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG) ? True : False;

    /*
     * get the new flag
     */
    if (event->type == FocusIn)
	newFlag = True;

    if (oldFlag != newFlag)
      {
	if (newFlag == False)
	  {
            _DtHelpCeMoveTraversal (pDAS->canvas, CETraversalOff, False,
				(XtIsRealized(widget) ? True : False),
				NULL, NULL, NULL, NULL);
            pDAS->neededFlags = pDAS->neededFlags & ~(_DT_HELP_FOCUS_FLAG);
	  }
	else
	  {
            pDAS->neededFlags = pDAS->neededFlags | _DT_HELP_FOCUS_FLAG;
            _DtHelpCeMoveTraversal (pDAS->canvas, CETraversalOn, False,
				(XtIsRealized(widget) ? True : False),
				NULL, NULL, NULL, NULL);
	  }
      }

} /* End _DtHelpFocusCB */

/***************************************************************************
 * Function:  _DtHelpEnterLeaveCB
 *
 * _DtHelpEnterLeaveCB tracks the traversal of the hypertext.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpEnterLeaveCB( widget, client_data, event )
        Widget     widget ;
        XtPointer  client_data ;
        XEvent    *event ;
#else
_DtHelpEnterLeaveCB(
        Widget     widget,
        XtPointer  client_data,
        XEvent    *event )
#endif /* _NO_PROTO */
{
    Boolean                oldFlag;
    Boolean                newFlag = False;
    DtHelpDispAreaStruct  *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (pDAS->hyperCall == NULL ||
                (event->type != EnterNotify && event->type != LeaveNotify))
        return;

    /*
     * get the old flag
     */
    oldFlag = (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG) ? True : False;

    /*
     * get the new flag
     */
    if (event->type == FocusIn)
	newFlag = True;

    if (oldFlag != newFlag)
      {
	if (oldFlag == True)
	  {
            _DtHelpCeMoveTraversal (pDAS->canvas, CETraversalOff, False,
				(XtIsRealized(widget) ? True : False),
				NULL, NULL, NULL, NULL);
            pDAS->neededFlags = pDAS->neededFlags & ~(_DT_HELP_FOCUS_FLAG);
	  }
	else
	  {
            pDAS->neededFlags = pDAS->neededFlags | _DT_HELP_FOCUS_FLAG;
            _DtHelpCeMoveTraversal (pDAS->canvas, CETraversalOn, False,
				(XtIsRealized(widget) ? True : False),
				NULL, NULL, NULL, NULL);
	  }
      }
} /* End _DtHelpEnterLeaveCB */

/***************************************************************************
 * Function:  _DtHelpVisibilityCB
 *
 * _DtHelpVisibilityCB tracks whether the window becomes obscured.
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpVisibilityCB( widget, client_data, event )
        Widget	   widget ;
        XtPointer  client_data ;
        XEvent	  *event ;
#else
_DtHelpVisibilityCB(
        Widget	   widget,
        XtPointer  client_data,
        XEvent	  *event )
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct  *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (event->type != VisibilityNotify)
	return;

    /*
     * save the scrollbar and focus flags while clearing the visibility flags.
     */
    pDAS->neededFlags = pDAS->neededFlags &
				(_DT_HELP_FOCUS_FLAG | SCROLL_BAR_FLAGS);

    /*
     * set the visibility flag
     */
    pDAS->neededFlags = pDAS->neededFlags |
				(1 << (event->xvisibility.state + 3));

} /* End _DtHelpVisibilityCB */

/*****************************************************************************
 * Function: _DtHelpInitiateClipboard
 *
 *    _DtHelpInitiateClipboard
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpInitiateClipboard (pDAS)
    DtHelpDispAreaStruct  *pDAS;
#else
_DtHelpInitiateClipboard (
    DtHelpDispAreaStruct  *pDAS)
#endif /* _NO_PROTO */
{
    /*
     * check to see if we have the primary selection
     * before trying for the clipboard.
     */
    if (pDAS->primary == True && pDAS->text_selected == True)
      {
        long      itemId = 0L;               	     /* clipboard item id */
        long      dataId = 0L;                       /* clipboard data id */
        int       status;                            /* clipboard status  */
        XmString  clipLabel;
        Display  *dpy = XtDisplay(pDAS->dispWid);
        Window    win = XtWindow(pDAS->dispWid);
        char     *atomName;
        char     *tmpString;
        XTextProperty tmpProp;

        /*
         * get the selected text
         */
        _DtHelpCeGetSelectionData (pDAS->canvas, CESelectedText,
						(SelectionInfo *)(&tmpString));

        /*
         * Using the Xm clipboard facilities,
         * copy the selected text to the clipboard
         */
        if (tmpString != NULL)
	  {
            clipLabel = XmStringCreateLocalized ("DT_HELP");

            /*
	     * start copy to clipboard
	     */
            status = XmClipboardStartCopy(dpy, win, clipLabel,
					XtLastTimestampProcessed(dpy),
					pDAS->dispWid, NULL, &itemId);

	    /*
	     * no longer need the label
	     */
	    XmStringFree(clipLabel);

            if (status != ClipboardSuccess)
	      {
	        free(tmpString);
                return;
              }

	    status = XmbTextListToTextProperty(dpy, &tmpString, 1,
			                    (XICCEncodingStyle)XStdICCTextStyle,
					    &tmpProp);
	    /*
	     * free the original copy of the string and check the results.
	     */
	    free(tmpString);

            if (status != Success && status <= 0)
	      {
	        XmClipboardCancelCopy(dpy, win, itemId);
	        return;
              }

            atomName = XGetAtomName(dpy, tmpProp.encoding);

            /* move the data to the clipboard */
            status = XmClipboardCopy(dpy, win, itemId, atomName,
			          (XtPointer)tmpProp.value, tmpProp.nitems,
				  0, &dataId);

            XtFree(atomName);

            if (status != ClipboardSuccess)
	      {
	        XmClipboardCancelCopy(dpy, win, itemId);
                XFree((char*)tmpProp.value);
	        return;
	      }

            /*
	     * end the copy to the clipboard
	     */
            status = XmClipboardEndCopy (dpy, win, itemId);

            XFree((char*)tmpProp.value);
          }
      }

}  /* End _DtHelpInitiateClipboard */

/***************************************************************************
 * Function:  _DtHelpMoveBtnFocusCB
 *
 * _DtHelpMoveBtnFocusCB tracks the mouse movement for the Selection mechanism
 *
 **************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpMoveBtnFocusCB( widget, client_data, event )
        Widget	   widget ;
        XtPointer  client_data ;
        XEvent	  *event ;
#else
_DtHelpMoveBtnFocusCB(
        Widget	   widget,
        XtPointer  client_data,
        XEvent	  *event )
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    /*
     * if the policy is XmEXPLICIT, don't want the focus on the scrollbar
     */
    if (event->type == ButtonPress &&
	      _XmGetFocusPolicy(XtParent(XtParent(pDAS->dispWid))) != XmPOINTER)
	XmProcessTraversal(pDAS->dispWid, XmTRAVERSE_CURRENT);

}

/*****************************************************************************
 * Function: _DtHelpGetClearSelection
 *
 *  _DtHelpGetClearSelection - If this routine is called,
 *            the user has initiated a selection.
 *
 *****************************************************************************/
 void
#ifdef _NO_PROTO
_DtHelpGetClearSelection (widget, client_data)
	Widget		widget;		/*  widget id               */
	XtPointer	client_data;	/*  data from applicaiton   */
#else
_DtHelpGetClearSelection (
	Widget		widget,		/*  widget id               */
	XtPointer	client_data )	/*  data from applicaiton   */
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    /*
     * If this widget doesn't own the primary selection, get it.
     */
    if (pDAS->primary != True)
      {
        if (XtOwnSelection (widget, XA_PRIMARY,
			XtLastTimestampProcessed(XtDisplay(widget)),
			(XtConvertSelectionProc) ConvertSelectionCB,
			(XtLoseSelectionProc) _DtHelpLoseSelectionCB,
			(XtSelectionDoneProc) NULL))
	  {
	    pDAS->primary = True;
	    pDAS->anchor_time = XtLastTimestampProcessed(XtDisplay(widget));
	  }
      }
    else
	_DtHelpClearSelection (pDAS);

}  /* End _DtHelpGetClearSelection */
