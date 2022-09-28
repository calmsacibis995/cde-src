/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */ 
/* 
 *  Motif Release 1.2.3
 */ 
/*
 *  DrawingAVis.c,v based on DrawingA.c $Revision: 1.52 $ $Date: 93/12/10"
 * 
 *  A subclass of the DrawingArea widget, this widget allows one to
 *  specify a visual different than its Shell ancestor.  The depth
 *  and colormap resources should also be specified, but the code
 *  attempts to set these appropriately if they are omitted.
 *							-kapono carter
 *							 11/22/94 
 */
/*
 *  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY
 */

#include <Xm/XmP.h>
#include "DrawingAVisP.h"
#include <Xm/DrawingAP.h>
#include <Xm/TransltnsP.h>


#define XmRID_RESIZE_POLICY     0x2016	/* Verify this in RepTypeI.h */

#define defaultTranslations	_XmDrawingA_defaultTranslations
#define traversalTranslations	_XmDrawingA_traversalTranslations


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void ClassInitialize() ;
static void ClassPartInitialize() ;
static void Initialize() ;
static void Redisplay() ;
static void Realize() ;
static void Destroy() ;
static Boolean SetValues() ;
static void CvtVisualToColormap() ;
static void _XmDelWMColormapWindows() ;
static void _XmAddWMColormapWindows() ;

#else

static void ClassInitialize( void ) ;
static void ClassPartInitialize( 
                        WidgetClass w_class) ;
static void Initialize( 
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void Redisplay( 
                        Widget wid,
                        XEvent *event,
                        Region region) ;
static void Realize(
			Widget w,
			XtValueMask *p_valueMask,
			XSetWindowAttributes *attributes) ;
static void Destroy(
			Widget w) ;
static Boolean SetValues( 
                        Widget cw,
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void CvtVisualToColormap(
			Widget w,
			Visual *vis) ;
static void _XmDelWMColormapWindows(
			Widget w) ;
static void _XmAddWMColormapWindows(
			Widget w) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static XtActionsRec actionsList[] =
{
   { "DrawingAreaInput", _XmDrawingAreaInput },
};


/*  Resource definitions for DrawingAreaVis
 */

static XmSyntheticResource syn_resources[] =
{
	{	XmNmarginWidth,
		sizeof (Dimension),
		XtOffsetOf( struct _XmDrawingAreaVisRec,
						drawing_area.margin_width),
		_XmFromHorizontalPixels,
		_XmToHorizontalPixels
	},

	{	XmNmarginHeight,
		sizeof (Dimension),
		XtOffsetOf( struct _XmDrawingAreaVisRec,
						drawing_area.margin_height),
		_XmFromVerticalPixels,
		_XmToVerticalPixels
	},
};


static XtResource resources[] =
{
	{	XmNvisual,
		XmCVisual, XmRVisual, sizeof (Visual *),
		XtOffsetOf( struct _XmDrawingAreaVisRec, drawing_areaVis.visual),
		XmRImmediate, (XtPointer) CopyFromParent
	},

};


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    NULL,				/* InitializePrehook	*/
    NULL,				/* SetValuesPrehook	*/
    NULL,				/* InitializePosthook	*/
    NULL,				/* SetValuesPosthook	*/
    NULL,				/* secondaryObjectClass	*/
    NULL,				/* secondaryCreate	*/
    NULL,               		/* getSecRes data	*/
    { 0 },      			/* fastSubclass flags	*/
    NULL,				/* getValuesPrehook	*/
    NULL,				/* getValuesPosthook	*/
    NULL,				/* classPartInitPrehook */
    NULL,				/* classPartInitPosthook*/
    NULL,				/* ext_resources        */
    NULL,				/* compiled_ext_resources*/
    0,    				/* num_ext_resources    */
    FALSE,                              /* use_sub_resources    */
    XmInheritWidgetNavigable,		/* widgetNavigable      */
    NULL				/* focusChange          */
};

externaldef( xmdrawingareaclassrec) XmDrawingAreaVisClassRec
xmDrawingAreaVisClassRec =
{
   {			/* core_class fields      */
      (WidgetClass) &xmDrawingAreaClassRec,	/* superclass         */
      "XmDrawingAreaVis",			/* class_name         */
      sizeof(XmDrawingAreaVisRec),		/* widget_size        */
      ClassInitialize,	        		/* class_initialize   */
      ClassPartInitialize,			/* class_part_init    */
      FALSE,					/* class_inited       */
      Initialize,       			/* initialize         */
      NULL,					/* initialize_hook    */
      Realize,					/* realize            */
      actionsList,				/* actions	      */
      XtNumber(actionsList),			/* num_actions	      */
      resources,				/* resources          */
      XtNumber(resources),			/* num_resources      */
      NULLQUARK,				/* xrm_class          */
      TRUE,					/* compress_motion    */
      FALSE,					/* compress_exposure  */
      TRUE,					/* compress_enterlv   */
      FALSE,					/* visible_interest   */
      Destroy,			                /* destroy            */
      XtInheritResize,         			/* resize             */
      Redisplay,	        		/* expose             */
      SetValues,                		/* set_values         */
      NULL,					/* set_values_hook    */
      XtInheritSetValuesAlmost,	        	/* set_values_almost  */
      NULL,					/* get_values_hook    */
      NULL,					/* accept_focus       */
      XtVersion,				/* version            */
      NULL,					/* callback_private   */
      defaultTranslations,			/* tm_table           */
      XtInheritQueryGeometry,                  	/* query_geometry     */
      NULL,             	                /* display_accelerator*/
      (XtPointer)&baseClassExtRec,              /* extension          */
   },
   {		/* composite_class fields */
      XtInheritGeometryManager,			/* geometry_manager   */
      XtInheritChangeManaged,			/* change_managed     */
      XtInheritInsertChild,			/* insert_child       */
      XtInheritDeleteChild,     		/* delete_child       */
      NULL,                                     /* extension          */
   },

   {		/* constraint_class fields */
      NULL,					/* resource list        */   
      0,					/* num resources        */   
      0,					/* constraint size      */   
      NULL,					/* init proc            */   
      NULL,					/* destroy proc         */   
      NULL,					/* set values proc      */   
      NULL,                                     /* extension            */
   },

   {		/* manager_class fields */
      traversalTranslations,			/* translations           */
      syn_resources,				/* syn_resources      	  */
      XtNumber (syn_resources),			/* num_get_resources 	  */
      NULL,					/* syn_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,					/* extension           */    
   },

   {		/* drawingArea class */     
      (XtPointer) NULL,				/* extension pointer */
   },	
   {		/* drawingAreaVis class */     
      (XtPointer) NULL,				/* extension pointer */
   }
};

externaldef( xmdrawingareawidgetclass) WidgetClass xmDrawingAreaVisWidgetClass
				= (WidgetClass) &xmDrawingAreaVisClassRec ;


/****************************************************************/
static void 
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize( void )
#endif /* _NO_PROTO */
{   
  baseClassExtRec.record_type = XmQmotif ;
}
/****************************************************************/
static void 
#ifdef _NO_PROTO
ClassPartInitialize( w_class )
        WidgetClass w_class ;
#else
ClassPartInitialize(
        WidgetClass w_class )
#endif /* _NO_PROTO */
{   
    _XmFastSubclassInit( w_class, XmDRAWING_AREA_BIT) ;

    return ;
}


/****************************************************************
 *
 *	Realize
 *
 ****************************************************************/
static void
#ifdef _NO_PROTO
Realize( w, p_valueMask, attributes )
        Widget w ;
        XtValueMask *p_valueMask ;
        XSetWindowAttributes *attributes ;
#else
Realize(
        Widget w,
        XtValueMask *p_valueMask,
        XSetWindowAttributes *attributes )
#endif /* _NO_PROTO */
{
   Mask valueMask = *p_valueMask;
   XmDrawingAreaVisWidget da = (XmDrawingAreaVisWidget) w;

    /*  Make sure height and width are not zero.
    */
   if (!XtWidth(w)) XtWidth(w) = 1 ;
   if (!XtHeight(w)) XtHeight(w) = 1 ;
 
   valueMask |= CWBitGravity | CWDontPropagate;
   attributes->bit_gravity = NorthWestGravity;
   attributes->do_not_propagate_mask =
      ButtonPressMask | ButtonReleaseMask |
      KeyPressMask | KeyReleaseMask | PointerMotionMask;

   if (da->drawing_areaVis.visual) {
        XtCreateWindow (w, InputOutput, da->drawing_areaVis.visual, valueMask,
							attributes);
	_XmAddWMColormapWindows(w);
   }
   else
        XtCreateWindow (w, InputOutput, CopyFromParent, valueMask,
							attributes);
}

/*
 *************************************************************************
 *
 * Destroy -
 *
 ****************************procedure*header*****************************
 */
static void
Destroy(Widget widget)
{
        _XmDelWMColormapWindows(widget);
}

/****************************************************************
 * Let pass thru zero size, we'll catch them in Realize
 ****************/
static void 
#ifdef _NO_PROTO
Initialize( rw, nw, args, num_args )
        Widget rw ;
        Widget nw ;
        ArgList args ;
        Cardinal *num_args ;
#else
Initialize(
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
	XmDrawingAreaVisWidget new_w = (XmDrawingAreaVisWidget) nw ;

	/*  If a non-default visual is being used, make sure the depth
	 *  and colormap are correct.
	 */
	if (new_w->drawing_areaVis.visual) {
	   XVisualInfo vinfo_tmp, *vinfo;
	   int num;
	   			/* check depth first */
	   vinfo_tmp.screen = DefaultScreen(XtDisplay(nw));
	   vinfo_tmp.visualid = new_w->drawing_areaVis.visual->visualid;
	   vinfo_tmp.class = new_w->drawing_areaVis.visual->class;
	   vinfo = XGetVisualInfo(XtDisplay(nw),
			VisualScreenMask | VisualIDMask | VisualClassMask,
			&vinfo_tmp, &num);
	   if (!num)
		_XmWarning(NULL, "Bad visual.");
	   else
		if (vinfo->depth != nw->core.depth) {
			nw->core.depth = vinfo->depth;
			rw->core.depth = vinfo->depth;
		}
	   XFree(vinfo);
				/* now check colormap */
	   CvtVisualToColormap(nw, new_w->drawing_areaVis.visual);
	}

	if(    (new_w->drawing_area.resize_policy != XmRESIZE_SWINDOW)
            && !XmRepTypeValidValue( XmRID_RESIZE_POLICY,
                            new_w->drawing_area.resize_policy, (Widget) new_w) )
        {   new_w->drawing_area.resize_policy = XmRESIZE_ANY ;
        } 

        return ;
}


/****************************************************************
 * General redisplay function called on exposure events.
 ****************/
static void 
#ifdef _NO_PROTO
Redisplay( wid, event, region )
        Widget wid ;
        XEvent *event ;
        Region region ;
#else
Redisplay(
        Widget wid,
        XEvent *event,
        Region region )
#endif /* _NO_PROTO */
{
    XmDrawingAreaVisWidget da = (XmDrawingAreaVisWidget) wid ;
    XmDrawingAreaCallbackStruct cb;

    cb.reason = XmCR_EXPOSE;
    cb.event = event;
    cb.window = XtWindow (da);

    XtCallCallbackList ((Widget) da, da->drawing_area.expose_callback, &cb);

    _XmRedisplayGadgets( (Widget) da, event, region);
    return ;
}

/****************************************************************/
static Boolean 
#ifdef _NO_PROTO
SetValues( cw, rw, nw, args, num_args )
        Widget cw ;
        Widget rw ;
        Widget nw ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValues(
        Widget cw,
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmDrawingAreaVisWidget current = (XmDrawingAreaVisWidget) cw ;
    XmDrawingAreaVisWidget new_w = (XmDrawingAreaVisWidget) nw ;

    if (new_w->drawing_areaVis.visual != current->drawing_areaVis.visual) {
	new_w->drawing_areaVis.visual = current->drawing_areaVis.visual;
	_XmWarning(NULL, "Cannot change drawing area's XmNvisual.");
    }

    if(    (new_w->drawing_area.resize_policy != XmRESIZE_SWINDOW)
       && !XmRepTypeValidValue( XmRID_RESIZE_POLICY,
			       new_w->drawing_area.resize_policy, 
			       (Widget) new_w)  ) {   
	new_w->drawing_area.resize_policy = 
	    current->drawing_area.resize_policy ;
    } 

    /* If new margins, re-enforce them using movewidget, 
       then update the width and height so that XtSetValues does
       the geometry request */
    if (XtIsRealized((Widget) new_w) &&
        (((new_w->drawing_area.margin_width !=
          current->drawing_area.margin_width) ||
         (new_w->drawing_area.margin_height !=
          current->drawing_area.margin_height)))) {
	    
	/* move the child around if necessary */
	_XmGMEnforceMargin((XmManagerWidget)new_w,
			   new_w->drawing_area.margin_width,
			   new_w->drawing_area.margin_height,
			   False); /* use movewidget, no request */
	_XmGMCalcSize ((XmManagerWidget)new_w, 
		       new_w->drawing_area.margin_width, 
		       new_w->drawing_area.margin_height, 
		       &new_w->core.width, &new_w->core.height);
    }

    return( False) ;
}
   
/*
 *************************************************************************
 *
 * Obtain a colormap from the given visual.
 *
 *************************************************************************
 */
static void 
CvtVisualToColormap(Widget w, Visual *vis)
{
    Colormap cmap;
    Window   rootwin;
    Screen   *screen;
    Boolean	   share;

    screen = XtScreen(w);
    rootwin = RootWindowOfScreen(screen);
    share = False;

    if (vis == DefaultVisualOfScreen(screen))
	cmap = DefaultColormapOfScreen(screen);
    else if (w->core.colormap == (XtParent(w))->core.colormap)
    {
	/* if dynamic visual, look for pre-allocated cmap to share */
	if (vis->class % 2) {
	   Atom            actual_type;
	   int             actual_format;
	   unsigned long   nitems, bytes_after;
	   XStandardColormap *scm;

	   /* get the property's size */
	   XGetWindowProperty(XtDisplay(w), rootwin,
		XA_RGB_DEFAULT_MAP, 0L, 1L, False,
		AnyPropertyType, &actual_type,
		&actual_format, &nitems, &bytes_after,
		(unsigned char **)&scm);
 
	   /* get the entire property */
	   XGetWindowProperty(XtDisplay(w), rootwin,
		XA_RGB_DEFAULT_MAP, 0L,
		bytes_after/4 + 1, False,
		AnyPropertyType, &actual_type,
		&actual_format, &nitems, &bytes_after,
		(unsigned char **)&scm);
 
	   /* search for match */
	   nitems /= (sizeof(XStandardColormap)/4);
	   for (; nitems > 0; ++scm, --nitems)
		if (scm->visualid == vis->visualid) {
		   cmap = scm->colormap;
		   share = True;
		}
	}
	if (!share)	/* nothing to share; create a new one */
	   cmap = XCreateColormap(XtDisplay(w), rootwin, vis, AllocNone);
    }
    w->core.colormap = cmap;
}
   
/*
 *************************************************************************
 *
 * Code to cache colormap for each Shell Widget.
 *
 *************************************************************************
 */
 
typedef struct _CMShellItem {
        Widget  shell;
        Widget  *list;
        int     count;
        struct _CMShellItem *next;
} CMShellItem;
 
static CMShellItem *CMShellList = NULL;
 
Widget
_XmGetShellOfWidget(register Widget w)
{
        while(w != (Widget) NULL && !XtIsShell(w))
                w = w->core.parent;
        return(w);
}

/*
 * Look for the correct per shell widget list
 */
static CMShellItem *
_FindShellItem(Widget w)
{
        Widget sw = _XmGetShellOfWidget(w);
        CMShellItem *n = CMShellList;
 
        for (; n && (n->shell != sw); n = n->next);
 
        return(n);
}
 
/*
 * Find the correct per shell widget list,
 * add it if it doesn't exists.
 */
static CMShellItem *
_AddShellItem(Widget w)
{
        Widget sw = _XmGetShellOfWidget(w);
        CMShellItem *n;
 
 
        if (!(n = _FindShellItem(w))) {
                n = XtNew(CMShellItem);
                n->shell = sw;
                n->list = (Widget *)NULL;
                n->count = 0;
                n->next = CMShellList;
                CMShellList = n;
        }
 
        return(n);
}
 
/*
 * See if the widget already exists in
 * the shell's widget list.
 */
static int
_FindWidgetItem(CMShellItem *n, Widget w)
{
        register int i;
 
        for (i = 0; i < n->count; ++i)
                if (n->list[i] == w)
                        break;
 
        if (i < n->count)
                return(True);
        else
                return(False);
}
 
/*
 * Add the widget to the shell's widget list
 * if it doesn't already exists.
 */
static int
_AddWidgetItem(CMShellItem *n, Widget w)
{
        Widget *tmp;
 
        if (_FindWidgetItem(n, w))
                return(False);
 
        tmp = (Widget *)XtMalloc(sizeof(Widget) *
                (n->count+1));
 
        *tmp = w;
 
        memmove((void*)(tmp+1),(const void*)n->list,
                (size_t)(n->count * sizeof(Widget)));
 
        ++(n->count);
        XtFree((char *)n->list);
        n->list = tmp;
 
        return(True);
}
 
/*
 * Delete the widget to the shell's widget
 * list if it exists.
 */
static int
_DelWidgetItem(CMShellItem *n, Widget w)
{
        register int i, j;
        Widget *tmp;
 
        if (n->count < 1)
                return(False);
 
        if (!_FindWidgetItem(n, w))
                return(False);
 
        if (n->count > 1) {
                tmp = (Widget *)XtMalloc(sizeof(Widget) *
                        (n->count-1));
 
                for (i=0,j=0; i<n->count; ++i)
                        if (n->list[i] != w) {
                                tmp[j] = n->list[i];
                                ++j;
                        }
        } else
                tmp = (Widget *)NULL;
 
        --(n->count);
        XtFree((char *)n->list);
        n->list = tmp;
        return(True);
}
 
 
/*
 * Add a widget to it's shell's WM_COLORMAP
 * property list if it doesn't already exists.
 */
static void
_XmAddWMColormapWindows (Widget w)
{
        CMShellItem *n = _AddShellItem(w);
 
        if (_AddWidgetItem(n, w))
                XtSetWMColormapWindows(n->shell,
                        n->list, n->count);
}
 
/*
 * Delete a widget to it's shell's WM_COLORMAP
 * property list if it exists.
 */
static void
_XmDelWMColormapWindows (Widget w)
{
        CMShellItem *n = _FindShellItem(w);
 
        if (n == NULL)
                return;
 
        if (_DelWidgetItem(n, w))
                XtSetWMColormapWindows(n->shell,
                        n->list, n->count);
}
   
/****************************************************************
 * This function creates and returns a DrawingAreaVis widget.
 ****************/
Widget 
#ifdef _NO_PROTO
XmCreateDrawingAreaVis( p, name, args, n )
        Widget p ;
        String name ;
        ArgList args ;
        Cardinal n ;
#else
XmCreateDrawingAreaVis(
        Widget p,
        String name,
        ArgList args,
        Cardinal n )
#endif /* _NO_PROTO */
{


    return( XtCreateWidget( name, xmDrawingAreaVisWidgetClass, p, args, n)) ;
}
