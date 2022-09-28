/* $XConsortium: GadgetUtil.c /main/cde1_maint/1 1995/07/17 20:42:09 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           GadgetUtil.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    This file contains the set of functions defined in
 *                   the Xm file GadgetUtil.c (excluding the two input
 *                   functions which are already redefined in File.c).
 *                   These functions are needed locally, to prevent the
 *                   linker from trying to include the corresponding Xm
 *                   file, thus causing a "multiply defined symbol' error
 *                   to occur.  Some compilers will not generate an output
 *                   file under this condition.
 *
 *   FUNCTIONS: _XmConfigureObject
 *		_XmDispatchGadgetInput
 *		_XmMoveObject
 *		_XmRedisplayGadgets
 *		_XmResizeObject
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <Xm/XmP.h>
#include <Xm/GadgetP.h>
#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <Xm/DropSMgr.h>

/************************************************************************
 *
 *  XmConfigureObject
 *	Change the dimensional aspects of a widget or gadget.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
_XmConfigureObject( wid, x, y, width, height, border_width )
        Widget wid ;
        Position x ;
        Position y ;
        Dimension width ;
        Dimension height ;
        Dimension border_width ;
#else
_XmConfigureObject(
        Widget wid,
#if NeedWidePrototypes
        int x,
        int y,
        int width,
        int height,
        int border_width )
#else
        Position x,
        Position y,
        Dimension width,
        Dimension height,
        Dimension border_width )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
        RectObj g = (RectObj) wid ;
	XmDropSiteStartUpdate(wid);
   if (XtIsWidget (g))
   {
      if (!width)  width++;                 /* Stupid X protocol... */
      if (!height) height++;
      XtConfigureWidget( (Widget) g, x, y, width, height, border_width);
   }
   else
   {
      if (g->rectangle.x != x ||g->rectangle.y != y ||
          g->rectangle.width != width || g->rectangle.height != height)
      {
         if (XtIsRealized (g) && XtIsManaged (g))
            XClearArea (XtDisplay (g), XtWindow (g),
                        g->rectangle.x, g->rectangle.y,
                        g->rectangle.width, g->rectangle.height, True);
                             
         g->rectangle.x = x;
         g->rectangle.y = y;
         g->rectangle.width = width;
         g->rectangle.height = height;
         g->rectangle.border_width = 0;

         if (g->object.widget_class->core_class.resize)
            (*(g->object.widget_class->core_class.resize))( (Widget) g);

         if (XtIsRealized (g) && XtIsManaged (g))
         {
            XClearArea (XtDisplay (g), XtWindow (g),
                        g->rectangle.x, g->rectangle.y,
                        g->rectangle.width, g->rectangle.height, True);
/*
            if (g->object.widget_class->core_class.expose)
               (*(g->object.widget_class->core_class.expose))( (Widget) g,
                                                                   NULL, NULL);
*/
         }
      }
   }
	XmDropSiteEndUpdate(wid);
}




/************************************************************************
 *
 *  _XmResizeObject
 *	Change the width or height of a widget or gadget.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
_XmResizeObject( wid, width, height, border_width )
        Widget wid ;
        Dimension width ;
        Dimension height ;
        Dimension border_width ;
#else
_XmResizeObject(
        Widget wid,
#if NeedWidePrototypes
        int width,
        int height,
        int border_width )
#else
        Dimension width,
        Dimension height,
        Dimension border_width )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
        RectObj g = (RectObj) wid ;
	XmDropSiteStartUpdate(wid);
   if (XtIsWidget (g))
      XtResizeWidget ((Widget) g, width, height, border_width);
   else
      _XmConfigureObject((Widget) g, g->rectangle.x, g->rectangle.y, 
                                                             width, height, 0);
	XmDropSiteEndUpdate(wid);
}




/************************************************************************
 *
 *  _XmMoveObject
 *	Change the origin of a widget or gadget.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
_XmMoveObject( wid, x, y )
        Widget wid ;
        Position x ;
        Position y ;
#else
_XmMoveObject(
        Widget wid,
#if NeedWidePrototypes
        int x,
        int y )
#else
        Position x,
        Position y )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
        RectObj g = (RectObj) wid ;
	XmDropSiteStartUpdate(wid);
   if (XtIsWidget (g))
      XtMoveWidget ((Widget) g, x, y);
   else
      _XmConfigureObject((Widget) g, x, y,
                                   g->rectangle.width, g->rectangle.height, 0);
	XmDropSiteEndUpdate(wid);
}


/************************************************************************
 *
 *  _XmRedisplayGadgets
 *	Redisplay any gadgets contained within the manager mw which
 *	are intersected by the region.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
_XmRedisplayGadgets( w, event, region )
        Widget w ;
        register XEvent *event ;
        Region region ;
#else
_XmRedisplayGadgets(
        Widget w,
        register XEvent *event,
        Region region )
#endif /* _NO_PROTO */
{
   CompositeWidget mw = (CompositeWidget) w ;
   register int i;
   register Widget child;

   for (i = 0; i < mw->composite.num_children; i++)
   {
      child = mw->composite.children[i];
      if (XmIsGadget(child) && XtIsManaged(child))
      {
         if (region == NULL)
         {
            if (child->core.x < event->xexpose.x + event->xexpose.width      &&
                (Position)(child->core.x + child->core.width) > event->xexpose.x &&
                child->core.y < event->xexpose.y + event->xexpose.height     &&
                (Position)(child->core.y + child->core.height) > event->xexpose.y)
            {
               if (child->core.widget_class->core_class.expose)
                  (*(child->core.widget_class->core_class.expose))
                     (child, event, region);
            }
         }
         else
         {
            if (XRectInRegion (region, child->core.x, child->core.y,
                               child->core.width, child->core.height))
            {
               if (child->core.widget_class->core_class.expose)
                  (*(child->core.widget_class->core_class.expose))
                     (child, event, region);
            }
         }
      }
   }
}




/************************************************************************
 *
 *  _XmDispatchGadgetInput
 *	Call the gadgets class function and send the desired data to it.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
_XmDispatchGadgetInput( wid, event, mask )
        Widget wid ;
        XEvent *event ;
        Mask mask ;
#else
_XmDispatchGadgetInput(
        Widget wid,
        XEvent *event,
        Mask mask )
#endif /* _NO_PROTO */
{
        XmGadget g = (XmGadget) wid ;
   if (g->gadget.event_mask & mask && XtIsSensitive (g) && XtIsManaged (g))
      if (event != NULL) {
         XEvent synth_event;

#define CopyEvent(source, dest, type) \
    source.type = dest->type

         switch(mask) {
	   case XmENTER_EVENT:
                   CopyEvent(synth_event, event, xcrossing);
		   if (event->type != EnterNotify) {
		      synth_event.type = EnterNotify;
                   }
                   break;
	   case XmLEAVE_EVENT:
                   CopyEvent(synth_event, event, xcrossing);
		   if (event->type != LeaveNotify) {
		      synth_event.type = LeaveNotify;
                   }
                   break;
	   case XmFOCUS_IN_EVENT:
                   CopyEvent(synth_event, event, xfocus);
		   if (event->type != FocusIn) {
		      synth_event.type = FocusIn;
		   }
		   break;
	   case XmFOCUS_OUT_EVENT:
                   CopyEvent(synth_event, event, xfocus);
		   if (event->type != FocusIn) {
		      synth_event.type = FocusOut;
		   }
		   break;
	   case XmMOTION_EVENT:
                   CopyEvent(synth_event, event, xmotion);
		   if (event->type != MotionNotify) {
		      event->type = MotionNotify;
		   }
		   break;
	   case XmARM_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != ButtonPress &&
		       event->type != KeyPress) {
		      synth_event.type = ButtonPress;
		   }
		   break;
	   case XmACTIVATE_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != ButtonRelease &&
		       event->type != KeyPress) {
		      synth_event.type = ButtonRelease;
		   }
		   break;
	   case XmKEY_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != KeyPress &&
		       event->type != ButtonPress) {
		      synth_event.type = KeyPress;
		   }
		   break;
	   case XmHELP_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != KeyPress) {
		      synth_event.type = KeyPress;
		   }
		   break;
           default:
		   memcpy((char*)&synth_event, (char*)event,
		      (int)sizeof(synth_event));
		   break;
         }
   
         (*(((XmGadgetClass) (g->object.widget_class))->
             gadget_class.input_dispatch)) ((Widget) g, 
                                               (XEvent *) &synth_event, mask) ;
      } else
         (*(((XmGadgetClass) (g->object.widget_class))->
             gadget_class.input_dispatch)) ((Widget) g,
                                                      (XEvent *) event, mask) ;
}

