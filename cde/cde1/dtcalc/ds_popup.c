/* $XConsortium: ds_popup.c /main/cde1_maint/2 1995/10/10 09:22:40 rcs $ */
/*									*
 *  ds_popup.c                                                          *
 *   Contains some common functions used by the popups throughout       * 
 *   the Desktop Calculator.                                            * 
 *                                                                      * 
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/VendorSEP.h>

#include <X11/ShellP.h>
#include <X11/Shell.h>

#include "calctool.h"
#include "motif.h"

void _DtChildPosition     P((Widget, Widget, Position *, Position *)) ;

/* ARGSUSED */
void
#ifdef _NO_PROTO
_DtmapCB(w,client_data,call_data)
         Widget w;
         XtPointer client_data;
         XtPointer call_data;
#else
_DtmapCB(
         Widget w,
         XtPointer client_data,
         XtPointer call_data )
#endif /* _NO_PROTO */
{
   Arg         args[2];
   Widget      parent;
   Position newX, newY;

   parent = (Widget)client_data;

   if (parent)
   {
      _DtChildPosition(w, parent, &newX, &newY);
      XtSetArg(args[0], XmNx, newX);
      XtSetArg(args[1], XmNy, newY);
      XtSetValues(w, args, 2);
      XSync(X->dpy, False);
   }
}


/*
 */

void
#ifdef _NO_PROTO
_DtMappedCB(w,client_data,call_data)
         Widget w;
         XtPointer client_data;
         XtPointer call_data;
#else
_DtMappedCB(
         Widget w,
         XtPointer client_data,
         XtPointer call_data )
#endif /* _NO_PROTO */
{
  _DtmapCB(w, client_data, call_data);

  XtRemoveCallback(w, XmNmapCallback, (XtCallbackProc)_DtMappedCB, 
                                        client_data);
}


/*
 * This is the generic function for registering the map callback.
 */

void
#ifdef _NO_PROTO
_DtGenericMapWindow (shell, parent)
     Widget shell;
     Widget parent;
#else
_DtGenericMapWindow (
     Widget shell,
     Widget parent )
#endif /* _NO_PROTO */
{
   XtAddCallback (parent, XmNmapCallback,
                  (XtCallbackProc)_DtMappedCB, (XtPointer) shell);
}


/*
 * _DtChildPosition:
 *   Choose a position for a popup window ("child") so that the main
 *   window ("parent") is not obscured.  The child will be positioned
 *   to the right, below, left, or above the parent, depending on where
 *   there is the most space.
 */

void
#ifdef _NO_PROTO
_DtChildPosition(w, parent, newX, newY)
         Widget w;
         Widget parent;
         Position *newX, *newY;
#else
_DtChildPosition(
         Widget w,
         Widget parent,
         Position *newX,
         Position *newY)
#endif /* _NO_PROTO */
{
   Position pY, pX;
   XmVendorShellExtObject vendorExt;
   XmWidgetExtData        extData;
   int xOffset, yOffset;
   int pHeight, myHeight, sHeight;
   int pWidth, myWidth, sWidth;
   enum { posRight, posBelow, posLeft, posAbove } pos;
   int space;

   /* get x, y offsets for the parent's window frame */
   extData = _XmGetWidgetExtData(parent, XmSHELL_EXTENSION);
   if (extData)
   {
     vendorExt = (XmVendorShellExtObject)extData->widget;
     xOffset = vendorExt->vendor.xOffset;
     yOffset = vendorExt->vendor.yOffset;
   }
   else
     xOffset = yOffset = 0;

   /* get size/position of screen, parent, and widget */
   sHeight = HeightOfScreen(XtScreen(parent));;
   sWidth = WidthOfScreen(XtScreen(parent));
   pX = XtX(parent) - xOffset;
   pY = XtY(parent) - yOffset;
   pHeight = XtHeight(parent) + yOffset + xOffset;
   pWidth = XtWidth(parent) + 2*xOffset;
   myHeight = XtHeight(w) + yOffset + xOffset;
   myWidth = XtWidth(w) + 2*xOffset;

   /*
    * Determine how much space would be left if the child was positioned
    * to the right, below, left, or above the parent.  Choose the child
    * positioning so that the maximum space is left.
    */
   pos = posBelow;
   space = sHeight - (pY + pHeight + myHeight);

   if (pY - myHeight > space)
   {
      pos = posAbove;
      space = pY - myHeight;
   }

   if(space <= 0)
   {
      pos = posRight;
      sWidth - (pX + pWidth + myWidth);
      if (pX - myWidth > space)
      {
         pos = posLeft;
         space = pX - myWidth;
      }
   }

   /* Given relative positioning, determine x, y coordinates for the child */

   switch (pos)
   {
     case posRight:
       *newX = pX + pWidth - 5;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posBelow:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY + pHeight;
       break;

     case posLeft:
       *newX = pX - myWidth + 10;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posAbove:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY - myHeight;
       break;
   }
#ifdef AIX_ILS
   /*
    * Defect 177440, put HelpDialog within the monitor
    */
    if ( *newX < 0 )
	*newX = 0;
    if ( *newY < 0 )
        *newY = 0;

    if ( ( *newX + myWidth ) > sWidth ) {
	Position _tmp;

	_tmp = sWidth - myWidth;
	if ( _tmp >= 0 )
	    *newX = _tmp;	
    }

    if ( ( *newY + myHeight ) > sHeight ) {
	Position _tmp;

	_tmp = sHeight - myHeight;
	if ( _tmp >= 0 )
	    *newY = _tmp;	
    }
#endif /* AIX_ILS */
}

