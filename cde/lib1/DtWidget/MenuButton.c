/* $XConsortium: MenuButton.c /main/cde1_maint/3 1995/10/09 09:35:17 pascale $ */
/*
 *        Copyright (C) 1986,1991  Sun Microsystems, Inc
 *                    All rights reserved.
 *          Notice of copyright on this source code
 *          product does not indicate publication.
 *
 * RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by
 * the U.S. Government is subject to restrictions as set forth
 * in subparagraph (c)(1)(ii) of the Rights in Technical Data
 * and Computer Software Clause at DFARS 252.227-7013 (Oct. 1988)
 * and FAR 52.227-19 (c) (June 1987).
 *
 *    Sun Microsystems, Inc., 2550 Garcia Avenue,
 *    Mountain View, California 94043.
 *
 */
/*
 * The DtMenuButton widget is rigged with the Motif widget binary compatibilit
 * mechanism. All Motif-specific changes for this mechanism are preceded
 * by a comment including the string "MotifBc".
 *
 * For a description of the Motif widget binary compatibility mechanism
 * see the reference manual entry on XmResolveAllPartOffsets().
 *
 */

#include <Dt/DtMsgsP.h>
#include "MenuButtonP.h"
#include "DtStrDefs.h"

#include <Xm/LabelP.h>
#include <Xm/MenuUtilP.h>
#include <Xm/DrawP.h>

#include <X11/keysymdef.h>
/*
 * MotifBc
 */
#define DtMenuButtonIndex (XmLabelIndex + 1)
static XmOffsetPtr ipot; /* Instance part offset table */
static XmOffsetPtr cpot; /* Constraint part offset table */

#ifndef Max
#define Max(x, y)       (((x) > (y)) ? (x) : (y))
#endif

#define GLYPH_PIX_SPACE    4	 /* pixels between label and bit map */

/*
 * MotifBc
 */
#define CascadingCallback(w) XmField(w,ipot,DtMenuButton,cascading_callback,XtCallbackList)
#define MenuRect(w) XmField(w,ipot,DtMenuButton,menu_rect,XRectangle)
#define LNormalGC(w) XmField(w,ipot,XmLabel,normal_GC,GC)
#define LAlignment(w) XmField(w,ipot,XmLabel,alignment,unsigned char)
#define LRecomputeSize(w) XmField(w,ipot,XmLabel,recompute_size,Boolean)
#define LLabelType(w) XmField(w,ipot,XmLabel,label_type,unsigned char)

/* Access macro definitions */
#define MB_PVT_SMENU(mb)	XmField(mb,ipot,DtMenuButton,private_submenu,Boolean)
#define MB_LAST_TIMESTAMP(mb)	XmField(mb,ipot,DtMenuButton,last_timestamp,Time)
#define MB_SMENU(mb)		XmField(mb,ipot,DtMenuButton,submenu,Widget)
#define MB_PIXMAP(mb)		XmField(mb,ipot,DtMenuButton,menu_pixmap,Pixmap)
#define MB_GLYPH_X(mb) 		(XmField(mb,ipot,DtMenuButton,menu_rect,XRectangle)).x
#define MB_GLYPH_Y(mb)		(XmField(mb,ipot,DtMenuButton,menu_rect,XRectangle)).y
#define MB_GLYPH_WIDTH(mb)	(XmField(mb,ipot,DtMenuButton,menu_rect,XRectangle)).width
#define MB_GLYPH_HEIGHT(mb)	(XmField(mb,ipot,DtMenuButton,menu_rect,XRectangle)).height
#define MB_ARMED(mb)	 	XmField(mb,ipot,DtMenuButton,armed,Boolean)
#define MB_POPPED_UP(mb) 	XmField(mb,ipot,DtMenuButton,popped_up,Boolean)
#define MB_GC(mb)		XmField(mb,ipot,DtMenuButton,gc,GC)


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO
static void     ClassInitialize ();

static void AdjustMenuButtonSize() ;
static void Arm() ;
static void ArmAndActivate() ;

static void CalculateMenuGlyphSize() ;
static void CallCascadingCallbacks() ;

static void Destroy() ;
static void Disarm() ;
static void DrawArrow();
static void Draw3DShadows() ;
static void DrawMenuGlyph() ;

static void GetGC();

static void Initialize() ;

static void LocateMenuGlyph() ;

static void MenuButtonHandler() ;

static void Popup();

static void Redisplay() ;
static void Resize() ;

static void Select() ;
static Boolean SetValues() ;
static void GetTopManager() ;

#else
static void     ClassInitialize (void);

static void AdjustMenuButtonSize( 
	DtMenuButtonWidget menubtn,
	Boolean adjustWidth,
	Boolean adjustHeight) ;
static void Arm( 
	DtMenuButtonWidget mb) ;
static void ArmAndActivate( 
	Widget wid,
	XEvent *event,
	String *params,
	Cardinal *num_params) ;

static void CalculateMenuGlyphSize( 
	DtMenuButtonWidget menubtn) ;
static void CallCascadingCallbacks( 
	Widget w,
	XEvent *event) ;

static void Destroy( 
	Widget wid) ;
static void Disarm( 
	DtMenuButtonWidget mb,
	Boolean pop_down) ;
static void DrawArrow(
	Widget wid);
static void Draw3DShadows( 
	DtMenuButtonWidget mb) ;
static void DrawMenuGlyph( 
	DtMenuButtonWidget mb) ;

static void GetGC(
	DtMenuButtonWidget mb);

static void Initialize( 
	Widget w_req,
	Widget w_new,
	ArgList args,
	Cardinal *num_args) ;

static void LocateMenuGlyph( 
	DtMenuButtonWidget menubtn) ;

static void MenuButtonHandler( 
	Widget wid,
	XtPointer cd,   
	XEvent *event,
	Boolean *cont_to_dispatch) ;

static void Popup(
	DtMenuButtonWidget mb,
	XEvent *event,
	Boolean call_cbacks );

static void Redisplay( 
	Widget mb,
	XEvent *event,
	Region region) ;
static void Resize( 
	Widget mb) ;

static void Select( 
	Widget wid,
	XEvent *event,
	String *param,
	Cardinal *num_param) ;
static Boolean SetValues( 
	Widget cw,
	Widget rw,
	Widget nw,
	ArgList args,
	Cardinal *num_args) ;
static void GetTopManager(
        Widget w,
        Widget *topManager) ;


#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*
 * event translation tables for menubutton.
 */

static _DtConst char menuButton_translations[] = "\
	<Key>space:Select()\n\
	<Key>osfSelect:Select()";

static XtActionsRec menuButton_actions [] = {
	{"Select",	Select}
};
	
/* MotifBc */
#define DtOffset(field) XmPartOffset(DtMenuButton,field)
#define XmPrimOffset(field) XmPartOffset(XmPrimitive,field)
#define XmLabelOffset(field) XmPartOffset(XmLabel,field)
static XmPartResource resources[] = {
	{	
	DtNcascadingCallback, 
	DtCCallback, 
	XmRCallback,
	sizeof (XtCallbackList),
	DtOffset(cascading_callback), 
	XmRCallback,
	NULL
	},
	{	
	DtNsubMenuId, 
	DtCMenuWidget,				/* submenu */
	XmRMenuWidget, 
	sizeof (Widget),
	DtOffset(submenu), 
	XmRMenuWidget, 
	(XtPointer) NULL
	},
	{	
	DtNcascadePixmap, 
	DtCPixmap, 
	XmRPrimForegroundPixmap,
	sizeof(Pixmap),
	DtOffset(menu_pixmap), 
	XmRImmediate,
	(XtPointer) XmUNSPECIFIED_PIXMAP
	},
	{
	XmNshadowThickness,
	XmCShadowThickness,
	XmRHorizontalDimension,
	sizeof (Dimension),
	XmPrimOffset(shadow_thickness),
	XmRImmediate,
	(XtPointer) 2
	},
	{
	XmNtraversalOn,
	XmCTraversalOn,
	XmRBoolean,
	sizeof(Boolean),
	XmPrimOffset(traversal_on),
	XmRImmediate,
	(XtPointer) TRUE
	},
	{
	XmNhighlightThickness,
	XmCHighlightThickness,
	XmRHorizontalDimension,
	sizeof (Dimension),
	XmPrimOffset(highlight_thickness),
	XmRImmediate,
	(XtPointer) 2
	},
	{
	XmNmarginWidth, 
	XmCMarginWidth, 
	XmRHorizontalDimension, 
	sizeof (Dimension),
	XmLabelOffset(margin_width), 
	XmRImmediate,
	(XtPointer)6 
	},
	};       
	
externaldef(xmmenubuttonclassrec) 
	DtMenuButtonClassRec dtMenuButtonClassRec = {
	{			/* core class record */
	(WidgetClass) &xmLabelClassRec,		/* superclass ptr	*/
	"DtMenuButton",				/* class_name	*/
	sizeof(DtMenuButtonPart),		/* size of Pulldown widget */
	(XtProc)ClassInitialize,		/* class init proc */
	NULL,					/* chained class init */
	FALSE,					/* class is not init'ed */
	Initialize,				/* widget init proc */
	NULL,					/* init_hook proc */
	XtInheritRealize,			/* widget realize proc */
	menuButton_actions,			/* class action table */
	XtNumber (menuButton_actions),		/* num of actions */
	(XtResourceList)resources,		/* this class's resource list*/
	XtNumber (resources),			/* resource_count */
	NULLQUARK,				/* xrm_class	        */
	TRUE,					/* compress motion */
	XtExposeCompressMaximal,		/* compress exposure */
	TRUE,					/* compress enter-leave */
	FALSE,					/* no VisibilityNotify */
	Destroy,				/* class destroy proc */
	Resize,					/* class resize proc */
	Redisplay,				/* expose proc */
	SetValues,				/* set_value proc */
	NULL,					/* set_value_hook proc */
	XtInheritSetValuesAlmost,		/* set_value_almost proc */
	NULL,					/* get_values_hook */
	NULL,					/* class accept focus proc */
	XtVersionDontCheck,			/* current version */
	NULL,					/* callback offset list */
	(char*)menuButton_translations,		/* default translation table */
	XtInheritQueryGeometry,			/* query geo proc */
	NULL,				        /* display accelerator*/
	(XtPointer)NULL,			/* extension */
	},
	{
			/* Primitive Class record */
	XmInheritWidgetProc,			/* border_highlight */
	XmInheritWidgetProc,			/* border_uhighlight */
	XtInheritTranslations,			/* translations */
	ArmAndActivate,				/* arm & activate */
	NULL,					/* get resources */
	0,					/* num get_resources */
	(XtPointer)NULL,			/* extension */
	},
	{			/* Label Class record */
	XmInheritWidgetProc,			/* set override callback */
	XmInheritMenuProc,			/* menu procedures       */
	XtInheritTranslations,			/* menu traversal xlation */
	NULL,					/* extension */
	},
	{			/* menu_button class record */
	    NULL,				/* extension */  
	}
};
	
	
/*
 * Now make a public symbol that points to this class record.
 */
	
externaldef(dtmenubuttonwidgetclass) 
	WidgetClass dtMenuButtonWidgetClass = 
		(WidgetClass) &dtMenuButtonClassRec;
	
/*
 * MotifBc, to calculate offset var. ipot
 */
static void
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize(void)
#endif
{
    XmResolveAllPartOffsets(dtMenuButtonWidgetClass, &ipot, &cpot);
}

static void 
#ifdef _NO_PROTO
Draw3DShadows( mb )
    DtMenuButtonWidget mb ;
#else
Draw3DShadows(
    DtMenuButtonWidget mb )
#endif /* _NO_PROTO */
{
	if (XtIsRealized(mb))
	 _XmDrawShadows (XtDisplay (mb), XtWindow (mb),
		mb->primitive.top_shadow_GC,
		mb->primitive.bottom_shadow_GC,
		mb->primitive.highlight_thickness,
		mb->primitive.highlight_thickness,
		mb->core.width - 2 * 
		mb->primitive.highlight_thickness,
		mb->core.height - 2 * 
		mb->primitive.highlight_thickness,
		mb->primitive.shadow_thickness,
		(MB_ARMED(mb) == TRUE) ? XmSHADOW_IN: XmSHADOW_OUT);
}
	
	
static void 
#ifdef _NO_PROTO
DrawMenuGlyph( mb )
    DtMenuButtonWidget mb ;
#else
DrawMenuGlyph(
    DtMenuButtonWidget mb )
#endif /* _NO_PROTO */
{
	if ((MB_GLYPH_WIDTH(mb) != 0))  
		if(MB_PIXMAP(mb) != XmUNSPECIFIED_PIXMAP) 
	 		XCopyArea (XtDisplay(mb), 
	 			MB_PIXMAP(mb), 
	 			XtWindow(mb),
	 			LNormalGC(mb), 0, 0, 
	 			MB_GLYPH_WIDTH(mb), MB_GLYPH_HEIGHT(mb),
	 			MB_GLYPH_X(mb), MB_GLYPH_Y(mb));
		else 
			DrawArrow((Widget)mb);	
}
	
/*
 * Redisplay the widget.
 */
static void 
#ifdef _NO_PROTO
Redisplay( mb, event, region )
    Widget mb ;
    XEvent *event ;
    Region region ;
#else
Redisplay(
    Widget mb,
    XEvent *event,
    Region region )
#endif /* _NO_PROTO */
{
	if (XtIsRealized (mb)) {
		/* Label expose method does the initial work */
		(* xmLabelClassRec.core_class.expose)(mb, event, region) ;
	
		DrawMenuGlyph((DtMenuButtonWidget) mb);
		Draw3DShadows ((DtMenuButtonWidget) mb);
	}
}

static void 
#ifdef _NO_PROTO
Arm( mb )
    DtMenuButtonWidget mb ;
#else
Arm(
    DtMenuButtonWidget mb )
#endif /* _NO_PROTO */
{
	XmProcessTraversal( (Widget) mb, XmTRAVERSE_CURRENT);
	if (MB_ARMED(mb) == FALSE) {
		MB_ARMED(mb) =  TRUE;
		DrawMenuGlyph(mb);
		Draw3DShadows (mb);
	}
}

static void 
#ifdef _NO_PROTO
ArmAndActivate( wid, event, params, num_params )
    Widget wid ;
    XEvent *event ;
    String *params ;
    Cardinal *num_params ;
#else
ArmAndActivate(
    Widget wid,
    XEvent *event,
    String *params,
    Cardinal *num_params )
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
	
	if(MB_SMENU(mb) == (Widget)NULL	||
		event == (XEvent*) NULL)
		return;

	if (!_XmIsEventUnique(event))
	   return;
	
	Popup(mb, event, TRUE);
	_XmSetInDragMode((Widget)mb,FALSE);
	if (!XmProcessTraversal(MB_SMENU(mb), XmTRAVERSE_CURRENT))
                   XtSetKeyboardFocus(XtParent(MB_SMENU(mb)), MB_SMENU(mb));

	_XmRecordEvent(event);
}


static void 
#ifdef _NO_PROTO
Disarm( mb, pop_down )
    DtMenuButtonWidget mb ;
    Boolean pop_down ;
#else
Disarm(
    DtMenuButtonWidget mb,
    Boolean pop_down )
#endif /* _NO_PROTO */
{
	if (MB_ARMED(mb) == TRUE) {
		MB_ARMED(mb) =  FALSE;
	
		if (pop_down == TRUE 	&& 
			MB_SMENU(mb) != (Widget)NULL)
				XtUnmanageChild(MB_SMENU(mb));
	
		Draw3DShadows(mb);
		DrawMenuGlyph(mb);
	}
}

/*ARGSUSED*/
static void 
#ifdef _NO_PROTO
Select( wid, event, param, num_param )
    Widget wid ;
    XEvent *event ;
    String *param ;
    Cardinal *num_param ;
#else
Select(
    Widget wid,
    XEvent *event,
    String *param,
    Cardinal *num_param )
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
	
	if(MB_POPPED_UP(mb) == TRUE) {
		Disarm(mb,TRUE);
		return;
	}
	
	(* (((DtMenuButtonClassRec *)(mb->core.widget_class))->
		primitive_class.arm_and_activate))
			((Widget) mb, event, NULL, NULL);
}

static void
#ifdef _NO_PROTO
PreMenuButtonHandler(wid, cd, event, cont_to_dispatch)
Widget wid;
XtPointer cd;
XEvent *event;
Boolean *cont_to_dispatch;
#else
PreMenuButtonHandler(
    Widget wid,
    XtPointer cd,
    XEvent *event,
Boolean *cont_to_dispatch)
#endif /* _NO_PROTO */
{
       DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
        
        if(MB_SMENU(mb) == (Widget)NULL                         ||
                event->xany.type != ButtonPress                 ||
                event->xbutton.time <= MB_LAST_TIMESTAMP(mb))
                        return;

	if(event->xbutton.button==Button1||event->xbutton.button==Button3)
	{
		String btnstr=NULL;
		XtVaGetValues(MB_SMENU(mb), XmNmenuPost, &btnstr, NULL);
		if (btnstr==NULL || !strcmp(btnstr,"") || 
		    !strcmp(btnstr, "<Btn3Down>"))
			event->xbutton.button=Button3;
		else if (!strcmp(btnstr, "<Btn1Down>"))
			event->xbutton.button=Button1;
		else
			XtWarning(MB_POST);
 	}
}

static void 
#ifdef _NO_PROTO
MenuButtonHandler(wid, cd, event, cont_to_dispatch)
Widget wid;
XtPointer cd;
XEvent *event;
Boolean *cont_to_dispatch;
#else
MenuButtonHandler(
    Widget wid,
    XtPointer cd,
    XEvent *event,
Boolean *cont_to_dispatch)
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
	

	if(MB_SMENU(mb) == (Widget)NULL 			||
		event->xany.type != ButtonPress 		|| 
		event->xbutton.time <= MB_LAST_TIMESTAMP(mb))
			return;
	else
		MB_LAST_TIMESTAMP(mb) = event->xbutton.time;
	
	Popup (mb, event, TRUE);

}


static void 
#ifdef _NO_PROTO
CallCascadingCallbacks( w, event )
    Widget w ;
    XEvent *event ;
#else
CallCascadingCallbacks(
    Widget w,
    XEvent *event )
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget)w;
	XmAnyCallbackStruct cback;
	
	if(MB_POPPED_UP(mb) == TRUE)
		return;
	
	cback.reason = DtCR_CASCADING;
	cback.event = event;
	
	XtCallCallbackList ((Widget) mb, 
		CascadingCallback(mb), &cback);
}

static void
PopdownCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget)client_data;

	Disarm(mb,FALSE);
	MB_POPPED_UP(mb) = FALSE;
	MB_LAST_TIMESTAMP(mb) =  XtLastTimestampProcessed(XtDisplay(mb));
}

static void
PopupCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget)client_data;

	Arm(mb);
	MB_POPPED_UP(mb) = TRUE;
}

/*
 * Call the cascading callbacks and popup any submenu. 
 */

static void 
#ifdef _NO_PROTO
Popup( mb, event, call_cbacks )
    DtMenuButtonWidget mb ;
    XEvent *event ;
    Boolean call_cbacks ;
#else
Popup(
    DtMenuButtonWidget mb,
    XEvent *event,
    Boolean call_cbacks )
#endif /* _NO_PROTO */
{
	Position x=0, y=0; 
	Position root_x=0, root_y=0; 
	Position save_x = 0, save_y = 0;
	Dimension sw=0, sh=0;
	int dx = 0, dy = 0;
	Screen *screen;

	if((MB_SMENU(mb) == (Widget)NULL) 	|| 
		(MB_POPPED_UP(mb) == TRUE))
		return;
	
	if (call_cbacks)
		CallCascadingCallbacks((Widget)mb, event);
	
	x =  (Position)((int)XtWidth((Widget)mb) - (int)XtWidth(MB_SMENU(mb)))/2;
	y  = XtHeight((Widget)mb) - mb->primitive.highlight_thickness + 1;
	XtTranslateCoords((Widget)mb, x,y, &root_x, &root_y);
	
	/* Check if not completely on the screen */
	screen = XtScreen(mb);
	sw = WidthOfScreen(screen);
	sh = HeightOfScreen(screen);

	if((dx = root_x - sw + XtWidth(MB_SMENU(mb))) > 0)
		root_x -= dx; 
	if((dy = root_y - sh + XtHeight(MB_SMENU(mb))) > 0) {
		root_y -= (2 - (mb->primitive.highlight_thickness<<1));
		root_y -= (XtHeight(mb) + XtHeight(MB_SMENU(mb)));
	}
	
	save_x = event->xbutton.x_root;
	save_y = event->xbutton.y_root;
	
	event->xbutton.x_root = root_x;
	event->xbutton.y_root = root_y;

	/* Position the menu */
	XmMenuPosition(MB_SMENU(mb),(XButtonPressedEvent*)event);

	event->xbutton.x_root = save_x;
	event->xbutton.y_root = save_y;
	
	XtManageChild(MB_SMENU(mb));
}

/*
 * Get the menu glyph size set up.
 */
static void 
#ifdef _NO_PROTO
CalculateMenuGlyphSize( menubtn )
    DtMenuButtonWidget menubtn ;
#else
CalculateMenuGlyphSize(
    DtMenuButtonWidget menubtn )
#endif /* _NO_PROTO */
{
	Window rootwin;
	int x,y; /* must be int */
	unsigned int width, height, border, depth; /* must be int */
	
	if (MB_PIXMAP(menubtn) != XmUNSPECIFIED_PIXMAP) {
	   XGetGeometry(XtDisplay(menubtn), MB_PIXMAP(menubtn),
			&rootwin, &x, &y, &width, &height,
			&border, &depth);
	
	   MB_GLYPH_WIDTH(menubtn) = (Dimension) width;
	   MB_GLYPH_HEIGHT(menubtn) = (Dimension) height;
	} else {
		int ht, st;
		Dimension side;
		unsigned int text_height;
	
		ht = menubtn->primitive.highlight_thickness;
		st = menubtn->primitive.shadow_thickness;
		text_height = Lab_TextRect_height(menubtn);
	
		side = Max( (text_height * 2 / 3) + 2 * (ht + st),
	           (2*(ht + (st-1) +1)) +1 );
	   	MB_GLYPH_WIDTH(menubtn) = 
	   	MB_GLYPH_HEIGHT(menubtn) = side;
	}
}

/*
 * Set up the menu glyph location.  
 */
static void 
#ifdef _NO_PROTO
LocateMenuGlyph( menubtn )
    DtMenuButtonWidget menubtn ;
#else
LocateMenuGlyph(
    DtMenuButtonWidget menubtn )
#endif /* _NO_PROTO */
{
	Dimension buffer;
	
	MB_GLYPH_X(menubtn) = 
		XtWidth (menubtn) -
	 	menubtn->primitive.highlight_thickness -
		menubtn->primitive.shadow_thickness -
		Lab_MarginWidth(menubtn) -
		MB_GLYPH_WIDTH(menubtn);
	
	buffer = menubtn->primitive.highlight_thickness +
			 menubtn->primitive.shadow_thickness +
			 Lab_MarginHeight(menubtn);
	
	MB_GLYPH_Y(menubtn) = (Position)((int)buffer +
		(((int)XtHeight(menubtn) -  2*(int)buffer) - 
			(int)MB_GLYPH_HEIGHT(menubtn)) / 2);
}

/*
 * Make room for  menu glyph  in menu button.
 */
static void 
#ifdef _NO_PROTO
AdjustMenuButtonSize( menubtn, adjustWidth, adjustHeight )
    DtMenuButtonWidget menubtn ;
    Boolean adjustWidth ;
    Boolean adjustHeight ;
#else
AdjustMenuButtonSize(
    DtMenuButtonWidget menubtn,
    Boolean adjustWidth,
    Boolean adjustHeight )
#endif /* _NO_PROTO */
{
	Dimension delta;
	
	/*
	 *  Modify the size of the menubutton to acommadate the menu.
	 *  The menu should fit inside MarginRight.
	 */
	if ((int)(MB_GLYPH_WIDTH(menubtn) + GLYPH_PIX_SPACE) >
	  				(int)Lab_MarginRight(menubtn)) {
		delta = MB_GLYPH_WIDTH(menubtn) + GLYPH_PIX_SPACE -
		Lab_MarginRight(menubtn);
		Lab_MarginRight(menubtn) += delta;
	
		if (adjustWidth)
			XtWidth(menubtn) += delta;
		else {
			if (LAlignment(menubtn) == XmALIGNMENT_END)
				Lab_TextRect_x(menubtn) -= delta;
			else 
				if (LAlignment(menubtn) == XmALIGNMENT_CENTER)
					Lab_TextRect_x(menubtn) -= delta/2;
		}
	}
	
	/*
	 * the menu height should fit inside of 
	 * TextRect + marginTop + marginBottom
	 */
	delta = MB_GLYPH_HEIGHT(menubtn) +
		2 * (Lab_MarginHeight(menubtn) +
		menubtn->primitive.shadow_thickness +
		menubtn->primitive.highlight_thickness);
	
	if (delta > XtHeight(menubtn)) {
		delta -= XtHeight(menubtn);
		Lab_MarginTop(menubtn) += delta/2;
		Lab_TextRect_y(menubtn) += delta/2;
		Lab_MarginBottom(menubtn) += delta - (delta/2);
	 
		if (adjustHeight)
			XtHeight(menubtn) += delta;
	}
	
	LocateMenuGlyph(menubtn);
}

/*
 * Destroy the widget
 */
static void 
#ifdef _NO_PROTO
Destroy( wid )
    Widget wid ;
#else
Destroy(
    Widget wid )
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
	XmRowColumnWidget submenu = (XmRowColumnWidget) MB_SMENU(mb);
	Widget shell;
	Display *dpy = XtDisplay(wid);
	
	XtRemoveAllCallbacks ((Widget) mb, DtNcascadingCallback);
	
	if(submenu != (XmRowColumnWidget)NULL) {
		shell = XtParent((Widget)submenu);
		XtRemoveCallback(shell, XtNpopupCallback,PopupCallback, (XtPointer)mb);
		XtRemoveCallback(shell, XtNpopdownCallback, PopdownCallback,(XtPointer)mb);
	}
	
	if(MB_PVT_SMENU(mb) == TRUE) {
		XtDestroyWidget((Widget)submenu);
		MB_SMENU(mb) = NULL;
		MB_PVT_SMENU(mb) = FALSE;
	}
}
                     
/*
 * Resize Method.
 */
static void 
#ifdef _NO_PROTO
Resize( mb )
    Widget mb ;
#else
Resize(
    Widget mb )
#endif /* _NO_PROTO */
{
	if (mb != (Widget)NULL) {
		/* Label resize method lays out the label string */
		(* xmLabelClassRec.core_class.resize) (mb);

		LocateMenuGlyph ((DtMenuButtonWidget) mb);
	}
}

/*
 * Set Values Method.
 */
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
	DtMenuButtonWidget old = (DtMenuButtonWidget) cw ;
	DtMenuButtonWidget requested = (DtMenuButtonWidget) rw ;
	DtMenuButtonWidget new_w = (DtMenuButtonWidget) nw ;
	Boolean flag = FALSE;
	Boolean adjustWidth = FALSE;
	Boolean adjustHeight = FALSE;
	unsigned char rowcol_type = NULL;
	Display *dpy = XtDisplay(nw);
	Boolean menu_glyph_changed = FALSE;
	
	if (MB_SMENU(new_w) != (Widget)NULL) {
		XtVaGetValues(MB_SMENU(new_w), XmNrowColumnType, 
					&rowcol_type, NULL);
		if(rowcol_type != XmMENU_POPUP) {
			MB_SMENU(new_w) = NULL;
			XtError(MB_SUBMENU);
		}
	}

	if ((LRecomputeSize(new_w))	|| 
		(requested->core.width <= 0))
			adjustWidth = TRUE;
	
	if ((LRecomputeSize(new_w))	|| 
		(requested->core.height <= 0))
			adjustHeight = TRUE;
	
	/* get new pixmap size */
	if ((MB_PIXMAP(old) != MB_PIXMAP (new_w)) ||
		(Lab_TextRect_height(old) != Lab_TextRect_height(new_w))) {
			CalculateMenuGlyphSize (new_w);
			menu_glyph_changed = TRUE;
	}

	if ((old->primitive.foreground != 
		new_w->primitive.foreground)	||
		(old->core.background_pixel != 
			new_w->core.background_pixel)) {
				menu_glyph_changed = TRUE;
				GetGC(new_w);
	}

	/*
	 * Resize widget if submenu appeared or disappeared, or if the
	 * menu glyph changed.
	 */
	
	if ( menu_glyph_changed == TRUE	||
		(LLabelType(old) != LLabelType(new_w)) ||
		(MB_SMENU(old) != MB_SMENU(new_w))) {
	
	  	AdjustMenuButtonSize (new_w, adjustWidth, adjustHeight);
	
	  	flag = TRUE;
	
	   	if ((MB_SMENU(old) != MB_SMENU(new_w))) {
	
	 	  	if(MB_SMENU(new_w)) {
				XtRemoveEventHandler((Widget)new_w,
				        ButtonPressMask, False, 
					PreMenuButtonHandler, MB_SMENU(old));
	                	XtInsertEventHandler((Widget)new_w,
					ButtonPressMask, False, 
					PreMenuButtonHandler, MB_SMENU(new_w), 
					XtListHead);
	   			XtAddCallback(XtParent(
					MB_SMENU(new_w)), 
					XtNpopdownCallback, 
					PopdownCallback, (XtPointer)new_w);
	   			XtAddCallback(XtParent(
					MB_SMENU(new_w)), 
					XtNpopupCallback, 
					PopupCallback, (XtPointer)new_w);
	  		}
	
	  		if(MB_PVT_SMENU(old) == TRUE) {
				XtDestroyWidget(MB_SMENU(old));
				MB_PVT_SMENU(new_w) = FALSE;
	  		}
	
		}
	
	} else 	if ((new_w->primitive.highlight_thickness !=
				old->primitive.highlight_thickness)               ||
				(new_w->primitive.shadow_thickness !=
				old->primitive.shadow_thickness)                  ||
				(Lab_MarginRight (new_w) != Lab_MarginRight (old))   ||
				(Lab_MarginHeight (new_w) != Lab_MarginHeight (old)) ||
				(Lab_MarginTop (new_w) != Lab_MarginTop (old))	 ||
				(Lab_MarginBottom (new_w) != Lab_MarginBottom (old))) {
					CalculateMenuGlyphSize (new_w);
					AdjustMenuButtonSize (new_w,adjustWidth, adjustHeight);
					flag = TRUE;
			} else if ((Lab_MarginWidth(new_w) != Lab_MarginWidth(old)) ||
					(new_w->core.width != old->core.width)           ||
					(new_w->core.height != old->core.height)) {
						LocateMenuGlyph (new_w);
						flag = TRUE;
			}
	
	return (flag);
}

/*
 * Initialize
 */
static void 
#ifdef _NO_PROTO
Initialize( w_req, w_new, args, num_args )
    Widget w_req ;
    Widget w_new ;
    ArgList args ;
    Cardinal *num_args ;
#else
Initialize(
    Widget w_req,
    Widget w_new,
    ArgList args,
    Cardinal *num_args )
#endif /* _NO_PROTO */
{
	DtMenuButtonWidget  req = (DtMenuButtonWidget) w_req ;
	DtMenuButtonWidget  new_w = (DtMenuButtonWidget) w_new ;
	Widget topManager;
	Boolean adjustWidth = FALSE;
	Boolean adjustHeight = FALSE;
	Widget    parent = XtParent(new_w);
	unsigned char rowcol_type = NULL;
	char name[256];
	
	if ((XmIsRowColumn (parent))) {
		XtVaGetValues(parent, XmNrowColumnType, 
			&rowcol_type, NULL);
		if(rowcol_type != XmWORK_AREA)
			XtError(MB_PARENT);
	}

	sprintf(name,"submenu_%s",new_w->core.name);
	
	GetTopManager(w_new,&topManager);
	MB_SMENU(new_w) = XmCreatePopupMenu(topManager, name, NULL, 0);
        /* Remove our passive grab */
	XtUngrabButton(topManager, RC_PostButton(MB_SMENU(new_w)), AnyModifier);

	MB_PVT_SMENU(new_w) = TRUE;
	
	MB_ARMED(new_w) =  FALSE;
	MB_POPPED_UP(new_w) = FALSE;
	MB_LAST_TIMESTAMP(new_w) = 0;
	MB_GC(new_w) = (GC)NULL;
	
	if (req->core.width <= 0)
		adjustWidth = TRUE;
	
	if (req->core.height <= 0)
		adjustHeight = TRUE;
	
	CalculateMenuGlyphSize (new_w);
	AdjustMenuButtonSize (new_w, adjustWidth, adjustHeight);
	GetGC(new_w);
	
	if(MB_SMENU(new_w) != (Widget)NULL) {
		XtInsertEventHandler((Widget)new_w, ButtonPressMask,
                     False, PreMenuButtonHandler, MB_SMENU(new_w), XtListHead);
                XtAddEventHandler((Widget)new_w, ButtonPressMask,
                        FALSE, MenuButtonHandler, MB_SMENU(new_w));
		XtAddCallback(XtParent(MB_SMENU(new_w)), XtNpopdownCallback, 
				PopdownCallback, (XtPointer)new_w);
		XtAddCallback(XtParent(MB_SMENU(new_w)), XtNpopupCallback, 
				PopupCallback, (XtPointer)new_w);
	}
}


static void 
#ifdef _NO_PROTO
DrawArrow( wid )
    Widget wid ;
#else
DrawArrow(
    Widget wid )
#endif /* _NO_PROTO */
{
	GC gc, tsGC, bsGC;
	Pixel tsc, bsc;
	int ht,st;
	unsigned int text_height;

	DtMenuButtonWidget mb = (DtMenuButtonWidget) wid ;
	Window win = XtWindow(wid);
	Display *dpy = XtDisplay(wid);
	
	ht = mb->primitive.highlight_thickness;
	st = mb->primitive.shadow_thickness;
	text_height = Lab_TextRect_height(mb);

	tsc =  mb->primitive.top_shadow_color;
	bsc =  mb->primitive.bottom_shadow_color;

	tsGC = mb->primitive.top_shadow_GC;
	bsGC = mb->primitive.bottom_shadow_GC;
	gc = MB_GC(mb);

	/* armed arrow */
	if(MB_ARMED(mb) == TRUE) {
		XFillRectangle(dpy, win, gc, 
			MB_GLYPH_X(mb), MB_GLYPH_Y(mb), 
			MB_GLYPH_WIDTH(mb), MB_GLYPH_HEIGHT(mb));
		_XmDrawArrow(dpy, win,
			bsGC, tsGC, gc,
			MB_GLYPH_X(mb) + ht + st - 1,
			MB_GLYPH_Y(mb) + ht + st - 1,
			MB_GLYPH_WIDTH(mb) - 2*(ht + st - 1),
			MB_GLYPH_HEIGHT(mb) - 2*(ht + st - 1),
			st, XmARROW_DOWN);
	} else {
  		/* standard (unarmed) arrow */
	
		XFillRectangle(dpy, win, gc, 
			MB_GLYPH_X(mb), MB_GLYPH_Y(mb),
			MB_GLYPH_WIDTH(mb), MB_GLYPH_HEIGHT(mb));
		_XmDrawArrow(dpy, win,
			tsGC, bsGC, gc, 
		 	MB_GLYPH_X(mb) + ht + st - 1, 
		 	MB_GLYPH_Y(mb) + ht + st - 1, 
		 	MB_GLYPH_WIDTH(mb) - 2*(ht + st - 1),
		 	MB_GLYPH_HEIGHT(mb) - 2*(ht + st - 1), 
		 	st, XmARROW_DOWN);
	}

}

static void
#ifdef _NO_PROTO
GetGC(mb)
	DtMenuButtonWidget mb;
#else
GetGC(
	DtMenuButtonWidget mb)    
#endif /* _NO_PROTO */
{
	XGCValues values;
	Pixel bg;

	if(MB_GC(mb) != (GC)NULL) {
		XtReleaseGC((Widget)mb, MB_GC(mb));
		MB_GC(mb) = (GC)NULL;
	}

	bg = mb->core.background_pixel;
	values.foreground = values.background = bg;
        values.graphics_exposures = FALSE;
        MB_GC(mb)  = XtGetGC ((Widget) mb,
                GCForeground | GCBackground | GCGraphicsExposures, &values);
}

/*
 *************************************************************************
 *
 * Public Routines                                                        
 *
 *************************************************************************
 */

Widget 
#ifdef _NO_PROTO
DtCreateMenuButton( parent, name, al, ac )
    Widget parent ;
    char *name ;
    ArgList al ;
    Cardinal ac ;
#else
DtCreateMenuButton(
    Widget parent,
    char *name,
    ArgList al,
    Cardinal ac )
#endif /* _NO_PROTO */
{
	return XtCreateWidget(name,dtMenuButtonWidgetClass,parent, al, ac);
}

static void
#ifdef _NO_PROTO
GetTopManager( w, topManager )
        Widget w ;
        Widget *topManager ;
#else
GetTopManager(
        Widget w,
        Widget *topManager )
#endif /* _NO_PROTO */
{
   while (XmIsManager(XtParent(w)))
       w = XtParent(w);
 
   * topManager = w;
}
 

