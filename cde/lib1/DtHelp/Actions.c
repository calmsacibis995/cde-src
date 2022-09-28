/* $XConsortium: Actions.c /main/cde1_maint/1 1995/07/17 17:20:00 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Actions.c
 **
 **   Project:     Display Area Library
 **
 **   Description: This body of code handles the actions for the
 **                Display Area.
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <Xm/AtomMgr.h>
#include <Xm/DrawnB.h>
#include <Xm/Xm.h>


/*
 * private includes
 */
#include "DisplayAreaP.h"
#include "ActionsI.h"
#include "CallbacksI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    End Private Defines             ********/

/********    Private Variable Declarations    ********/

/********    End Private Variable Declarations    ********/

/******************************************************************************
 *                             Private Functions
 ******************************************************************************/
/******************************************************************************
 *                          Semi Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function: _DtHelpCopyAction
 *
 *    _DtHelpCopyAction - Copy the current info to the clipboard
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCopyAction (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpCopyAction (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    _DtHelpInitiateClipboard(pDAS);;

}  /* End _DtHelpCopyAction */

/*****************************************************************************
 * Function: _DtHelpDeSelectAll
 *
 *    _DtHelpDeSelectAll - Deselects the information in the widget.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDeSelectAll (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpDeSelectAll (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
        _DtHelpClearSelection (pDAS);

}  /* End _DtHelpDeSelectAll */

/*****************************************************************************
 * Function: _DtHelpSelectAll
 *
 *    _DtHelpSelectAll - Selects all the information in the widget.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpSelectAll (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpSelectAll (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
      {
	_DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOff, False, True,
						NULL, NULL, NULL, NULL);
	_DtHelpGetClearSelection (widget, (XtPointer) pDAS);
	_DtHelpCeProcessSelection(pDAS->canvas, 0, 0, CESelectionStart);
	_DtHelpCeProcessSelection(pDAS->canvas, pDAS->dispUseWidth,
						pDAS->maxYpos, CESelectionEnd);
	_DtHelpCeMoveTraversal(pDAS->canvas, CETraversalOn, False, True,
						NULL, NULL, NULL, NULL);
	pDAS->text_selected = True;
      }

}  /* End _DtHelpSelectAll */

/*****************************************************************************
 * Function: _DtHelpActivateLink
 *
 *    _DtHelpSelectAll - Selects all the information in the widget.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpActivateLink (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpActivateLink (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    Arg    args[2];
    DtHelpDispAreaStruct *pDAS;
    CEHyperTextCallbackStruct ceHyper;
    DtHelpHyperTextStruct   callData;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
      {
	if (_DtHelpCeGetTraversalSelection(pDAS->canvas, &ceHyper))
	  {
	    callData.reason        = XmCR_ACTIVATE;
	    callData.event         = event;
	    callData.window        = XtWindow (pDAS->dispWid);
	    callData.specification = ceHyper.specification;
	    callData.hyper_type    = ceHyper.hyper_type;
	    callData.window_hint   = ceHyper.win_hint;
	    (*(pDAS->hyperCall)) (pDAS, pDAS->clientData, &callData);
	  }
      }

}  /* End _DtHelpActivateLink */

/*****************************************************************************
 * Function: _DtHelpPageUpOrDown
 *
 *    _DtHelpPageUpOrDown - Selects all the information in the widget.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpPageUpOrDown (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpPageUpOrDown (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    int        keyPressed;
    Unit       newY;
    Unit       diff;
    Arg        args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
      {
        diff = pDAS->dispUseHeight - pDAS->lineHeight;

	keyPressed = atoi(*params);
	if (keyPressed == 0)
	    diff = -diff;

        newY = pDAS->firstVisible + diff;

	/*
	 * Is the new Y position too large?
	 * If so, adjust.
	 */
        if (newY + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
            newY = pDAS->maxYpos - pDAS->dispUseHeight;

	/*
	 * Is the new Y before the begining?
	 * If so, zero it.
	 */
        if (newY < 0)
            newY = 0;

        if (newY != pDAS->firstVisible)
          {
            pDAS->firstVisible = newY;
            XtSetArg(args[0], XmNvalue, newY);
            XtSetValues (pDAS->vertScrollWid, args, 1);

            _DtHelpCleanAndDrawWholeCanvas (pDAS);
          }
      }

}  /* End _DtHelpPageUpOrDown */

/*****************************************************************************
 * Function: _DtHelpPageLeftOrRight
 *
 *    _DtHelpPageLeftOrRight - Selects all the information in the widget.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpPageLeftOrRight (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpPageLeftOrRight (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    int        keyPressed;
    Unit       newX;
    Unit       diff;
    Arg        args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
      {
        diff = pDAS->dispUseWidth - ((int) pDAS->charWidth / 10);

	keyPressed = atoi(*params);
	if (keyPressed == 0)
	    diff = -diff;

        newX = pDAS->virtualX + diff;

	/*
	 * Is the new X position too large?
	 * If so, adjust.
	 */
        if (newX + ((int)pDAS->dispUseWidth) > pDAS->maxX)
            newX = pDAS->maxX - pDAS->dispUseWidth;

	/*
	 * Is the new X before the begining?
	 * If so, zero it.
	 */
        if (newX < 0)
            newX = 0;

        if (newX != pDAS->virtualX)
          {
            pDAS->virtualX = newX;
            XtSetArg(args[0], XmNvalue, newX);
            XtSetValues (pDAS->horzScrollWid, args, 1);

            _DtHelpCleanAndDrawWholeCanvas (pDAS);
          }
      }

}  /* End _DtHelpPageLeftOrRight */

/*****************************************************************************
 * Function: _DtHelpNextLink
 *
 *    _DtHelpNextLink - Moves the traversal to the requested hypertext link.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpNextLink (widget, event, params, num_params)
    Widget          widget;
    XEvent          *event;
    String          *params;
    Cardinal        *num_params;
#else
_DtHelpNextLink (
    Widget          widget,
    XEvent          *event,
    String          *params,
    Cardinal        *num_params)
#endif /* _NO_PROTO */
{
    int        direction = CETraversalNext;
    Unit       diff;
    Unit       newY;
    Unit       newX;
    Unit       height;
    Unit       top;
    Arg        args[2];
    DtHelpDispAreaStruct *pDAS;

    XtSetArg(args[0], XmNuserData, &pDAS);
    XtGetValues(widget, args, 1);

    if (pDAS != NULL)
      {
	switch(atoi(*params))
	  {
	    case 0: direction = CETraversalPrev;
		    break;
	    case 2: direction = CETraversalTop;
		    break;
	    case 3: direction = CETraversalBottom;
		    break;
	  }

	if (_DtHelpCeMoveTraversal(pDAS->canvas, direction, False,
				(XtIsRealized(pDAS->dispWid) ? True : False),
				NULL, &newX, &newY, &height) == True)
	  {
	    /*
	     * take into account our traversal indicator
	     */
	    newY   -= pDAS->lineThickness;
	    height += (2 * pDAS->lineThickness);
    
	    top  = pDAS->firstVisible;
	    diff = ((int) pDAS->dispUseHeight) * 2 / 3;
	    if (newY < top)
	      {
		top = newY;
		if (direction == CETraversalTop && newY <= diff)
		    top = 0;
	      }
	    else if (newY + height > top + ((int) pDAS->dispUseHeight))
	      {
		top = newY + height - ((int) pDAS->dispUseHeight);
		if (direction == CETraversalBottom &&
						newY >= (pDAS->maxYpos - diff))
		    top = pDAS->maxYpos - pDAS->dispUseHeight;
	      }
    
	    if (top != pDAS->firstVisible)
	      {
		pDAS->firstVisible = top;
    
		if (top + ((int)pDAS->dispUseHeight) > pDAS->maxYpos)
		  {
		    pDAS->firstVisible = pDAS->maxYpos - pDAS->dispUseHeight;
		    if (pDAS->firstVisible < 0)
			pDAS->firstVisible = 0;
		  }
    
		XtSetArg (args[0], XmNvalue, pDAS->firstVisible);
		XtSetValues (pDAS->vertScrollWid, args, 1);
    
		/*
		 * re-draw the information
		 */
		_DtHelpCleanAndDrawWholeCanvas (pDAS);
	      }
	  }
      }

}  /* End _DtHelpNextLink */
