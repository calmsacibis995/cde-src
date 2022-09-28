/* $XConsortium: Resize.c /main/cde1_maint/1 1995/07/17 17:50:11 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Resize.c
 **
 **   Project:     Display Area Library
 **
 **   Description: This body of code handles direct resize requests from
 **                the GUI.
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

/*
 * private includes
 */
#include "DisplayAreaP.h"
#include "CallbacksI.h"
#include "ResizeI.h"
#include "SetListI.h"
#include "XUICreateI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    Private Variables    ********/
/********    End Private Variables  ********/

/*********************************************************************
 *		Private Functions
 *********************************************************************/

/*********************************************************************
 *		Public Functions
 *********************************************************************/
/*********************************************************************
 * Function: ResizeDisplayArea
 *
 *    ResizeDisplayArea resizes the Display Area and re-formats the
 *	text based on a row and column values given.
 *
 *********************************************************************/
void
#ifdef _NO_PROTO
_DtHelpResizeDisplayArea (parent, pDAS, rows, columns)
	Widget			 parent;
	DtHelpDispAreaStruct	*pDAS;
	int			 rows;
	int			 columns;
#else
_DtHelpResizeDisplayArea (
	Widget			 parent,
	DtHelpDispAreaStruct	*pDAS,
	int			 rows,
	int			 columns )
#endif /* _NO_PROTO */
{
    int            count = 0;
    register int   n;
    unsigned long  char_width;
    Arg          args[5];
    Dimension    newWidth;
    Dimension    newHeight;
    Dimension    vertWidth  = 0;
    Dimension    horzHeight = 0;
    Dimension    tstWidth;
    Dimension    tstHeight;
    Dimension    oldWidth;
    Dimension    oldHeight;
    XEvent	 event;
    XmDrawnButtonCallbackStruct callBack;

    /*
     * check the values.
     */
    if (rows <= 0)
        rows = 1;
    if (columns <= 0)
        columns = 1;

   /* Set the size of the text view area to the requested number of columns */
   /* and lines. */
    char_width = pDAS->charWidth * columns;

    newWidth = char_width / 10 + (char_width % 10 ? 1 : 0) +
			2 * (pDAS->decorThickness + pDAS->marginWidth);
    newHeight = pDAS->lineHeight * rows + 2 * pDAS->decorThickness;

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
     * loop a couple of times at most to get the sizing correct.
     * This is caused by the fact we can't do a set values on
     * the drawn button itself, someone above it is rejecting
     * the resize request. So, resize the great-grandparent
     * (a couple of times if necessary).
     */
    do {
        /*
         * take into consideration the scrollbars.
         */
        tstWidth  = newWidth;
        tstHeight = newHeight;
        if (pDAS->vertScrollWid &&
            _DtHelpIS_AS_NEEDED(pDAS->neededFlags, _DtHelpVERTICAL_SCROLLBAR)
                                            && pDAS->vertIsMapped)
          {
            if (tstWidth > vertWidth)
                tstWidth -= vertWidth;
            else
                tstWidth = 0;
          }
        if (pDAS->horzScrollWid &&
            _DtHelpIS_AS_NEEDED(pDAS->neededFlags, _DtHelpHORIZONTAL_SCROLLBAR)
                                            && pDAS->horzIsMapped)
          {
            if (tstHeight > horzHeight)
                tstHeight -= horzHeight;
            else
                tstHeight = 0;
          }
      
      
        if (tstWidth == pDAS->dispWidth && tstHeight == pDAS->dispHeight)
            return;
    
        /*
         * kludge around the fact that some parent is not letting
         * me resize the drawn button. Get the parent's size and
         * resize that!
         */
        n = 0;
        XtSetArg(args[n], XmNwidth,  &oldWidth);                ++n;
        XtSetArg(args[n], XmNheight, &oldHeight);                ++n;
        XtGetValues(parent, args, n);
    
        /*
         * calculate its new size
         */
        oldWidth  = oldWidth - pDAS->dispWidth + tstWidth;
        oldHeight = oldHeight - pDAS->dispHeight + tstHeight;
    
        /*
         * set my parent's size to affect me.
         */
        n = 0;
        XtSetArg(args[n], XmNwidth,  oldWidth);                ++n;
        XtSetArg(args[n], XmNheight, oldHeight);                ++n;
        XtSetValues(parent, args, n);

    } while (count++ < 2);
    
    /*
     * check to see if the Set Values on my parent took care of everything.
     */
    if (pDAS->vertScrollWid &&
	_DtHelpIS_AS_NEEDED(pDAS->neededFlags, _DtHelpVERTICAL_SCROLLBAR)
					&& pDAS->vertIsMapped)
      {
	if (newWidth > vertWidth)
	    newWidth -= vertWidth;
	else
	    newWidth = 0;
      }

    if (pDAS->horzScrollWid &&
	_DtHelpIS_AS_NEEDED(pDAS->neededFlags, _DtHelpHORIZONTAL_SCROLLBAR)
					&& pDAS->horzIsMapped)
      {
	if (newHeight > horzHeight)
	    newHeight -= horzHeight;
	else
	    newHeight = 0;
      }

    if (newWidth == pDAS->dispWidth && newHeight == pDAS->dispHeight)
        return;

    /*
     * clean up
     */
    _DtHelpClearSelection (pDAS);

    /*
     * reset the scroll bars and possibly reformat the text.
     */
    _DtHelpSetScrollBars (pDAS, newWidth, newHeight);
    if (XtIsRealized (pDAS->dispWid))
	_DtHelpCleanAndDrawWholeCanvas(pDAS);

}  /* End _DtHelpResizeDisplayArea */
