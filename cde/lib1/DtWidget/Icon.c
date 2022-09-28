/* $XConsortium: Icon.c /main/cde1_maint/4 1995/10/09 09:34:48 pascale $ */
/**---------------------------------------------------------------------
***	
***	file:		Icon.c
***
***	project:	MotifPlus Widgets
***
***	description:	Source code for DtIconGadget class.
***	
***
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***-------------------------------------------------------------------*/


/*-------------------------------------------------------------
**	Include Files
*/

#include <stdio.h>
#ifdef __apollo
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <X11/Xatom.h>
#include <Xm/XmP.h>
#include <Xm/CacheP.h>
#include <Xm/ExtObjectP.h>
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>
#include <Dt/IconP.h>
#include <Dt/MacrosP.h>
#include <Dt/DtMsgsP.h>
#include <Dt/Dnd.h>


/*-------------------------------------------------------------
**	Public Interface
**-------------------------------------------------------------
*/

WidgetClass	dtIconGadgetClass;

#define Min(x, y)    (((x) < (y)) ? (x) : (y))
#define Max(x, y)    (((x) > (y)) ? (x) : (y))
#define Limit(x, lim)  (((lim) == 0 || (x) <= (lim))? (x): (lim))

/*-------------------------------------------------------------
**	Forward Declarations
*/

#ifdef _NO_PROTO
extern void 	_DtRegisterNewConverters ();
extern void	_XmSelectColorDefault ();
#else
extern void 	_DtRegisterNewConverters ( void );
extern void	_XmSelectColorDefault ( Widget, int, XrmValue * );
#endif

#define SPACING_DEFAULT		2
#define MARGIN_DEFAULT		2
#define UNSPECIFIED_DIMENSION	9999
#define UNSPECIFIED_CHAR	255

#define WARN_ALIGNMENT		_DtMsgIcon_0000
#define WARN_BEHAVIOR		_DtMsgIcon_0001
#define WARN_FILL_MODE		_DtMsgIcon_0002
#define WARN_PIXMAP_POSITION	_DtMsgIcon_0003
#define WARN_MARGIN		_DtMsgIcon_0004
#define WARN_SHADOW_TYPE	_DtMsgIcon_0005


#define MgrParent(g) (XmIsManager(XtParent(g)) ? (XtParent(g)) : (XtParent(XtParent(g))))

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern int _DtIconCacheCompare() ;
extern void _DtReCacheIcon() ;
extern void _DtAssignIcon_StringHeight() ;
extern void _DtAssignIcon_Foreground() ;
extern void _DtAssignIcon_Background() ;
extern void _DtIconRegisterDropsite() ;
extern Boolean _XmIsMissedImage();
extern Boolean _XmRemoveMissedImage();
#else

extern int _DtIconCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;
extern void _DtReCacheIcon( 
                        DtIconGadget g) ;
extern void _DtAssignIcon_StringHeight( 
                        DtIconGadget g,
                        Dimension value) ;
extern void _DtAssignIcon_Foreground( 
                        DtIconGadget g,
                        Pixel value) ;
extern void _DtAssignIcon_Background( 
                        DtIconGadget g,
                        Pixel value) ;

extern void _DtIconRegisterDropsite(
			Widget w) ;
extern Boolean _XmIsMissedImage(char*);
extern Boolean _XmRemoveMissedImage(char*);

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void GetDefaultBackground() ;
static void GetDefaultForeground() ;
static void GetDefaultFillMode() ;
static void GetSpacing() ;
static void GetString() ;
static void IconEventHandler() ;
static void ClickTimeout() ;
static void IconArm() ;
static void IconDisarm() ;
static void IconActivate() ;
static void IconDrag() ;
static void IconPopup() ;
static void IconEnter() ;
static void IconLeave() ;
static void ClassInitialize() ;
static void SecondaryObjectCreate() ;
static void InitializePosthook() ;
static Boolean SetValuesPrehook() ;
static void GetValuesPrehook() ;
static void GetValuesPosthook() ;
static Boolean SetValuesPosthook() ;
static void QualifyIconLocalCache() ;
static void Initialize() ;
static void Destroy() ;
static void Resize() ;
static void Redisplay() ;
static Boolean SetValues() ;
static void BorderHighlight();
static void BorderUnhighlight();
static void ArmAndActivate() ;
static void InputDispatch() ;
static Boolean VisualChange() ;
static void GetSize() ;
static void GetPositions() ;
static void Draw() ;
static void CallCallback() ;
static void UpdateGCs() ;
static Cardinal GetIconClassSecResData() ;
static XtPointer GetIconClassResBase() ;
static Boolean LoadPixmap() ;
static void ClassPartInitialize ();
static void HighlightBorder();
static void UnhighlightBorder();

#else

static void GetDefaultBackground( 
                        Widget g,
                        int offset,
                        XrmValue *value) ;
static void GetDefaultForeground( 
                        Widget g,
                        int offset,
                        XrmValue *value) ;
static void GetDefaultFillMode( 
                        Widget w,
                        int offset,
                        XrmValue *value) ;
static void GetSpacing( 
                        Widget w,
                        int offset,
                        XtArgVal *value) ;
static void GetString( 
                        Widget w,
                        int offset,
                        XtArgVal *value) ;
static void IconEventHandler( 
                        Widget w,
                        XtPointer client_data,
                        XButtonEvent *event) ;
static void ClickTimeout( 
                        Widget w,
                        XtIntervalId *id) ;
static void IconArm( 
                        Widget w,
                        XEvent *event) ;
static void IconDisarm( 
                        Widget w,
                        XEvent *event) ;
static void IconActivate( 
                        Widget w,
                        XEvent *event) ;
static void IconDrag( 
                        Widget w,
                        XEvent *event) ;
static void IconPopup( 
                        Widget w,
                        XEvent *event) ;
static void IconEnter( 
                        Widget w,
                        XEvent *event) ;
static void IconLeave( 
                        Widget w,
                        XEvent *event) ;
static void ClassInitialize( void ) ;
static void SecondaryObjectCreate( 
                        Widget req,
                        Widget new_w,
                        ArgList args,
                        Cardinal *num_args) ;
static void InitializePosthook( 
                        Widget req,
                        Widget new,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValuesPrehook( 
                        Widget oldParent,
                        Widget refParent,
                        Widget newParent,
                        ArgList args,
                        Cardinal *num_args) ;
static void GetValuesPrehook( 
                        Widget newParent,
                        ArgList args,
                        Cardinal *num_args) ;
static void GetValuesPosthook( 
                        Widget new,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValuesPosthook( 
                        Widget current,
                        Widget req,
                        Widget new,
                        ArgList args,
                        Cardinal *num_args) ;
static void QualifyIconLocalCache( 
                        DtIconGadget g) ;
static void Initialize( 
                        Widget request_w,
                        Widget new_w) ;
static void Destroy( 
                        Widget w) ;
static void Resize( 
                        Widget w) ;
static void Redisplay( 
                        Widget w,
                        XEvent *event,
                        Region region) ;
static Boolean SetValues( 
                        Widget current_w,
                        Widget request_w,
                        Widget new_w) ;
static void BorderHighlight(
                        DtIconGadget g) ;
static void BorderUnhighlight(
                        DtIconGadget g) ;
static void ArmAndActivate( 
                        Widget w,
                        XEvent *event) ;
static void InputDispatch( 
                        Widget w,
                        XButtonEvent *event,
                        Mask event_mask) ;
static Boolean VisualChange( 
                        Widget w,
                        Widget current_w,
                        Widget new_w) ;
static void GetSize( 
                        DtIconGadget g,
                        Dimension *w,
                        Dimension *h) ;
static void GetPositions( 
                        DtIconGadget g,
                        Position w,
                        Position h,
                        Dimension h_t,
                        Dimension s_t,
                        Position *pix_x,
                        Position *pix_y,
                        Position *str_x,
                        Position *str_y) ;
static void Draw( 
                        DtIconGadget g,
                        Drawable drawable,
                        Position x,
                        Position y,
                        Dimension w,
                        Dimension h,
                        Dimension h_t,
                        Dimension s_t,
                        unsigned char s_type,
                        unsigned char fill_mode) ;
static void CallCallback( 
                        DtIconGadget g,
                        XtCallbackList cb,
                        int reason,
                        XEvent *event) ;
static void UpdateGCs( 
                        DtIconGadget g) ;
static Cardinal GetIconClassSecResData( 
                        WidgetClass class,
                        XmSecondaryResourceData **data_rtn) ;
static XtPointer GetIconClassResBase( 
                        Widget widget,
                        XtPointer client_data) ;
static Boolean LoadPixmap( 
                        DtIconGadget new,
                        String pixmap) ;
static void ClassPartInitialize ( WidgetClass	wc);
static void HighlightBorder( Widget w );
static void UnhighlightBorder( Widget w );

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

/*      External Cache Procs
*/
#ifdef _NO_PROTO
extern void _XmCacheCopy();
extern void _XmCacheDelete();
extern XtPointer _XmCachePart();
extern Cardinal _XmSecondaryResourceData ();
#else
extern void _XmCacheDelete( XtPointer data );
extern void _XmCacheCopy( XtPointer src, XtPointer dest, size_t size );
extern Cardinal _XmSecondaryResourceData( XmBaseClassExt, XmSecondaryResourceData **, 
			  XtPointer, String, String, XmResourceBaseProc);
extern XtPointer _XmCachePart( XmCacheClassPartPtr cp, XtPointer cpart, size_t  size );
#endif


/*-------------------------------------------------------------
**	Resource List
*/
#define I_Offset(field) \
	XtOffset (DtIconGadget, icon.field)

#define I_Cache_Offset(field) \
	XtOffset (DtIconCacheObject, icon_cache.field)

static XtResource resources[] = 
{
	{
		XmNset,
		XmCSet, XmRBoolean, sizeof (Boolean),
		I_Offset (set), XmRImmediate, (XtPointer) False
	},
	{
		XmNshadowType,
		XmCShadowType, XmRShadowType, sizeof (unsigned char),
		I_Offset (shadow_type),
		XmRImmediate, (XtPointer) XmSHADOW_ETCHED_OUT
	},
	{
		XmNborderType,
		XmCBorderType, XmRBorderType, sizeof (unsigned char),
		I_Offset (border_type),
		XmRImmediate, (XtPointer) DtRECTANGLE
	},
	{
		XmNcallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		I_Offset (callback), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNfontList,
		XmCFontList, XmRFontList, sizeof (XmFontList),
		I_Offset (font_list), XmRString, "Fixed"
	},
	{
		XmNimageName,
		XmCString, XmRString, sizeof (String),
		I_Offset (image_name), XmRImmediate, (XtPointer) NULL
	},
	{
		XmNpixmap,
		XmCPixmap, XmRPixmap, sizeof (Pixmap),
		I_Offset (pixmap),
		XmRImmediate, (XtPointer) XmUNSPECIFIED_PIXMAP
	},
	{
		XmNstring,
		XmCXmString, XmRXmString, sizeof (_XmString),
		I_Offset (string),
		XmRImmediate, (XtPointer) XmUNSPECIFIED_STRING
	},
	{
		XmNpixmapForeground,
		XmCForeground, XmRPixel, sizeof (Pixel),
		I_Offset (pixmap_foreground),
		XmRCallProc, (XtPointer) GetDefaultForeground
	},
	{
		XmNpixmapBackground,
		XmCBackground, XmRPixel, sizeof (Pixel),
		I_Offset (pixmap_background),
		XmRCallProc, (XtPointer) GetDefaultBackground
	},
	{
		XmNmaxPixmapWidth,
		XmCMaxWidth, XmRDimension, sizeof (Dimension),
		I_Offset (max_pixmap_width), XmRImmediate, (XtPointer) 0
	},
	{
		XmNmaxPixmapHeight,
		XmCMaxHeight, XmRDimension, sizeof (Dimension),
		I_Offset (max_pixmap_height), XmRImmediate, (XtPointer) 0
	},
	{
		XmNunderline,
		XmCUnderline, XmRBoolean, sizeof (Boolean),
		I_Offset (underline), XmRImmediate, (XtPointer) False
        },
	{
		XmNdropSiteOperations, XmCDropSiteOperations,
                XmRDropSiteOperations, sizeof(unsigned char),
                I_Offset(operations), XmRImmediate, (XtPointer) XmDROP_NOOP
	},
	{
		XmNdropCallback,
		XmCDropCallback, XmRCallback, sizeof (XtCallbackList),
		I_Offset (drop_callback), XmRImmediate, (XtPointer) NULL
	}
};


static XtResource cache_resources[] =
{
	{
		XmNbehavior,
		XmCBehavior, XmRBehavior, sizeof (unsigned char),
		I_Cache_Offset (behavior),
		XmRImmediate, (XtPointer) XmICON_BUTTON
	},
	{
		XmNrecomputeSize,
		XmCRecomputeSize, XmRBoolean, sizeof (Boolean),
		I_Cache_Offset (recompute_size), XmRImmediate, (XtPointer) True
	},
	{
		"drawShadow",
		"DrawShadow", XmRBoolean, sizeof (Boolean),
		I_Cache_Offset (draw_shadow), XmRImmediate, (XtPointer) False
	},
	{
		XmNfillOnArm,
		XmCFillOnArm, XmRBoolean, sizeof (Boolean),
		I_Cache_Offset (fill_on_arm), XmRImmediate, (XtPointer) True
	},
	{
		XmNforeground,
		XmCForeground, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (foreground),
		XmRCallProc, (XtPointer) GetDefaultForeground
	},
	{
		XmNbackground,
		XmCBackground, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (background),
		XmRCallProc, (XtPointer) GetDefaultBackground
	},
	{
		XmNarmColor,
		XmCArmColor, XmRPixel, sizeof (Pixel),
		I_Cache_Offset (arm_color),
		XmRCallProc, (XtPointer) _XmSelectColorDefault
	},
	{
		XmNspacing,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (spacing),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNmarginHeight,
		XmCMarginHeight, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (margin_height),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNmarginWidth,
		XmCMarginWidth, XmRDimension, sizeof (Dimension),
		I_Cache_Offset (margin_width),
		XmRImmediate, (XtPointer) UNSPECIFIED_DIMENSION
	},
	{
		XmNpixmapPosition,
		XmCPixmapPosition, XmRPixmapPosition, sizeof (unsigned char),
		I_Cache_Offset (pixmap_position),
		XmRImmediate, (XtPointer) UNSPECIFIED_CHAR
	},
	{
		XmNstringPosition,
		XmCStringPosition, XmRStringPosition, sizeof (unsigned char),
		I_Cache_Offset (string_position),
		XmRImmediate, (XtPointer) UNSPECIFIED_CHAR
	},
	{
		XmNalignment,
		XmCAlignment, XmRAlignment, sizeof (unsigned char),
		I_Cache_Offset (alignment),
		XmRImmediate, (XtPointer) XmALIGNMENT_BEGINNING
	},
	{
		XmNfillMode,
		XmCFillMode, XmRFillMode, sizeof (unsigned char),
		I_Cache_Offset (fill_mode),
		XmRCallProc, (XtPointer) GetDefaultFillMode
	}
};


static XmSyntheticResource syn_resources[] =
{
	{
		XmNstring, sizeof (_XmString),
		I_Offset (string), GetString, NULL
	},
};


static XmSyntheticResource cache_syn_resources[] =
{
	{
		XmNspacing, sizeof (Dimension),
		I_Cache_Offset (spacing), GetSpacing, NULL
	},
	{
		XmNmarginWidth, sizeof (Dimension),
		I_Cache_Offset (margin_width),
		_XmFromHorizontalPixels, 
                _XmToHorizontalPixels
	},
	{
		XmNmarginHeight, sizeof (Dimension),
		I_Cache_Offset (margin_height),
		_XmFromVerticalPixels, 
                _XmToVerticalPixels, 
	}
};

#undef	I_Offset
#undef	I_Cache_Offset
	

/*-------------------------------------------------------------
**	Cache Class Record
*/

static XmCacheClassPart IconClassCachePart = {
        {NULL, 0, 0},            /* head of class cache list */
        _XmCacheCopy,           /* Copy routine         */
        _XmCacheDelete,         /* Delete routine       */
        _DtIconCacheCompare,       /* Comparison routine   */
};

/*-------------------------------------------------------------
**	Base Class Extension Record
*/
static XmBaseClassExtRec       iconBaseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    XmInheritInitializePrehook,               /* initialize prehook   */
    SetValuesPrehook,                         /* set_values prehook   */
    InitializePosthook,                       /* initialize posthook  */
    SetValuesPosthook,                        /* set_values posthook  */
    (WidgetClass)&dtIconCacheObjClassRec,   /* secondary class      */
    (XtInitProc)SecondaryObjectCreate,        /* creation proc        */
    (XmGetSecResDataFunc) GetIconClassSecResData,    /* getSecResData */
    {NULL},                                     /* fast subclass        */
    GetValuesPrehook,                         /* get_values prehook   */
    GetValuesPosthook,                        /* get_values posthook  */
    NULL,                                     /* classPartInitPrehook */
    NULL,                                     /* classPartInitPosthook*/
    NULL,                                     /* ext_resources        */
    NULL,                                     /* compiled_ext_resources*/
    0,                                        /* num_ext_resources    */
    FALSE,                                    /* use_sub_resources    */
    XmInheritWidgetNavigable,                 /* widgetNavigable      */
    XmInheritFocusChange,                     /* focusChange          */
};

/*-------------------------------------------------------------
**	Icon Cache Object Class Record
*/
externaldef (dticoncacheobjclassrec)
DtIconCacheObjClassRec dtIconCacheObjClassRec =
{
  {
      /* superclass         */    (WidgetClass) &xmExtClassRec,
      /* class_name         */    "DtIcon",
      /* widget_size        */    sizeof(DtIconCacheObjRec),
      /* class_initialize   */    NULL,
      /* chained class init */    NULL,
      /* class_inited       */    False,
      /* initialize         */    NULL,
      /* initialize hook    */    NULL,
      /* realize            */    NULL,
      /* actions            */    NULL,
      /* num_actions        */    0,
      /* resources          */    cache_resources,
      /* num_resources      */    XtNumber(cache_resources),
      /* xrm_class          */    NULLQUARK,
      /* compress_motion    */    False,
      /* compress_exposure  */    False,
      /* compress enter/exit*/    False,
      /* visible_interest   */    False,
      /* destroy            */    NULL,
      /* resize             */    NULL,
      /* expose             */    NULL,
      /* set_values         */    NULL,
      /* set values hook    */    NULL,
      /* set values almost  */    NULL,
      /* get values hook    */    NULL,
      /* accept_focus       */    NULL,
      /* version            */    XtVersion,
      /* callback offsetlst */    NULL,
      /* default trans      */    NULL,
      /* query geo proc     */    NULL,
      /* display accelerator*/    NULL,
      /* extension record   */    NULL,
   },

   {
      /* synthetic resources */   cache_syn_resources,
      /* num_syn_resources   */   XtNumber(cache_syn_resources),
      /* extension           */   NULL,
   },
};

/*-------------------------------------------------------------
**	Class Record
*/
externaldef (dticonclassrec)
DtIconClassRec dtIconClassRec =
{
	/*	Core Part
	*/
	{	
		(WidgetClass) &xmGadgetClassRec, /* superclass		*/
		"DtIcon",			/* class_name		*/
		sizeof (DtIconRec),		/* widget_size		*/
		ClassInitialize,		/* class_initialize	*/
		ClassPartInitialize,		/* class_part_initialize*/
		False,				/* class_inited		*/
		(XtInitProc) Initialize,	/* initialize		*/
		NULL,				/* initialize_hook	*/
		NULL,				/* realize		*/
		NULL,				/* actions		*/
		0,				/* num_actions		*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/
		NULLQUARK,			/* xrm_class		*/
		True,				/* compress_motion	*/
		True,				/* compress_exposure	*/
		True,				/* compress_enterleave	*/
		False,				/* visible_interest	*/	
		(XtWidgetProc) Destroy, 	/* destroy		*/	
		(XtWidgetProc) Resize,		/* resize		*/
		(XtExposeProc) Redisplay,	/* expose		*/	
		(XtSetValuesFunc) SetValues,	/* set_values		*/	
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,				/* get_values_hook	*/
		NULL,				/* accept_focus		*/	
		XtVersion,			/* version		*/
		NULL,				/* callback private	*/
		NULL,				/* tm_table		*/
		NULL,				/* query_geometry	*/
		NULL,				/* display_accelerator	*/
		(XtPointer)&iconBaseClassExtRec,/* extension		*/
	},

	/*	XmGadget Part
	*/
	{
	(XtWidgetProc)	BorderHighlight,	/* border_highlight	*/
	(XtWidgetProc)	BorderUnhighlight,	/* border_unhighlight	*/
	(XtActionProc)	ArmAndActivate,		/* arm_and_activate	*/
	(XmWidgetDispatchProc)	InputDispatch,	/* input_dispatch	*/
	(XmVisualChangeProc)	VisualChange,	/* visual_change	*/
		syn_resources,			/* get_resources	*/
		XtNumber (syn_resources),	/* num_get_resources	*/
		&IconClassCachePart,		/* class_cache_part	*/
		NULL,	 			/* extension		*/
	},

	/*	DtIconGadget Part
	*/
	{
		GetSize,			/* get_size		*/
		GetPositions,			/* get_positions	*/
		Draw,				/* draw			*/
		CallCallback,			/* call_callback	*/
		UpdateGCs,			/* update_gcs		*/
		True,				/* optimize_redraw	*/
		NULL,				/* class_cache_part	*/
		NULL,				/* extension		*/
	}
};


externaldef (dticongadgetclass) WidgetClass dtIconGadgetClass = (WidgetClass) &dtIconClassRec;


/*-------------------------------------------------------------
**	Private Functions
**-------------------------------------------------------------
*/

/************************************************************************
 *
 *  The border highlighting and unhighlighting routines.
 *
 *  These routines were originally in Obsolete.c but can not depend
 *  on these routines to live forever. Therefore, copied into my
 *  own space.
 *
 ************************************************************************/

static void
#ifdef _NO_PROTO
HighlightBorder( w )
        Widget w ;
#else
HighlightBorder(
        Widget w )
#endif /* _NO_PROTO */
{
    if(    XmIsPrimitive( w)    ) {
        (*(xmPrimitiveClassRec.primitive_class.border_highlight))( w) ;
    }  else  {
        if(    XmIsGadget( w)    ) {
            (*(xmGadgetClassRec.gadget_class.border_highlight))( w) ;
        }
    }
    return ;
}

static void
#ifdef _NO_PROTO
UnhighlightBorder( w )
        Widget w ;
#else
UnhighlightBorder(
        Widget w )
#endif /* _NO_PROTO */
{
    if(    XmIsPrimitive( w)    )
    {
        (*(xmPrimitiveClassRec.primitive_class.border_unhighlight))( w) ;
        }
    else
    {   if(    XmIsGadget( w)    )
        {
            (*(xmGadgetClassRec.gadget_class.border_unhighlight))( w) ;
            }
        }
    return ;
    }



/*-------------------------------------------------------------
**	GetMaskGC
**		Get normal and background graphics contexts.
*/
static GC
#ifdef _NO_PROTO
GetMaskGC( g, x, y )
    DtIconGadget g ;
    Position x, y;
#else
GetMaskGC(
        DtIconGadget g,
	  Position x, 
	  Position y)
#endif /* _NO_PROTO */
{
    if (G_Mask(g) != XmUNSPECIFIED_PIXMAP) {

	XSetClipOrigin(XtDisplay(g),
		       G_ClipGC(g),
		       x, y);
	return G_ClipGC(g);
    }
    else {
	return G_NormalGC(g);
    }
}

/*-------------------------------------------------------------
**	GetDefaultBackground
**		Copy background pixel from Manager parent.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetDefaultBackground( g, offset, value )
        Widget g ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultBackground(
        Widget g,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static Pixel		pixel;
	XmManagerWidget		parent = (XmManagerWidget) XtParent (g);

	value->addr = (XtPointer) &pixel;
	value->size = sizeof (Pixel);

	if (XmIsManager ((Widget) parent))
		pixel = M_Background (parent);
	else
		_XmBackgroundColorDefault (g, offset, value);
}


/*-------------------------------------------------------------
**	GetDefaultForeground
**		Copy foreground pixel from Manager parent.
*/
static void 
#ifdef _NO_PROTO
GetDefaultForeground( g, offset, value )
        Widget g ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultForeground(
        Widget g,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static Pixel		pixel;
	XmManagerWidget		parent = (XmManagerWidget) XtParent (g);

	value->addr = (XtPointer) &pixel;
	value->size = sizeof (Pixel);

	if (XmIsManager ((Widget) parent))
		pixel = M_Foreground (parent);
	else
		_XmForegroundColorDefault (g, offset, value);
}



/*-------------------------------------------------------------
**	GetDefaultFillMode
**		Get default fill mode.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetDefaultFillMode( w, offset, value )
        Widget w ;
        int offset ;
        XrmValue *value ;
#else
GetDefaultFillMode(
        Widget w,
        int offset,
        XrmValue *value )
#endif /* _NO_PROTO */
{
	static unsigned char	fill_mode;
	DtIconGadget	g =	(DtIconGadget) w;

	value->addr = (XtPointer) &fill_mode;
	value->size = sizeof (unsigned char);

	if (G_ShadowThickness (g) == 0)
		fill_mode = XmFILL_PARENT;
	else
		fill_mode = XmFILL_SELF;
}


/*-------------------------------------------------------------
**	GetSpacing
**		Convert from pixels to horizontal or vertical units.
*/
static void 
#ifdef _NO_PROTO
GetSpacing( w, offset, value )
        Widget w ;
        int offset ;
        XtArgVal *value ;
#else
GetSpacing(
        Widget w,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
	DtIconCacheObject icon_co = (DtIconCacheObject) w;

        if (G_CachePixmapPosition (icon_co) == XmPIXMAP_TOP ||
            G_CachePixmapPosition (icon_co) == XmPIXMAP_BOTTOM ||
            G_CachePixmapPosition (icon_co) == XmPIXMAP_MIDDLE)
                _XmFromVerticalPixels ((Widget)icon_co, offset, value);
        else
                _XmFromHorizontalPixels ((Widget)icon_co, offset, value);
}



/*-------------------------------------------------------------
**	GetString
**		Convert string from internal to external form.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetString( w, offset, value )
        Widget w ;
        int offset ;
        XtArgVal *value ;
#else
GetString(
        Widget w,
        int offset,
        XtArgVal *value )
#endif /* _NO_PROTO */
{
	DtIconGadget	g =	(DtIconGadget) w;
	XmString	string;

	string = _XmStringCreateExternal (G_FontList (g), G_String (g));

	*value = (XtArgVal) string;
}


/*-------------------------------------------------------------
**	IconEventHandler
**		Event handler for middle button events.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
IconEventHandler( w, client_data, event )
        Widget w ;
        XtPointer client_data ;
        XButtonEvent *event ;
#else
IconEventHandler(
        Widget w,
        XtPointer client_data,
        XButtonEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget		g =	NULL;

	g = (DtIconGadget) _XmInputInGadget (w, event->x, event->y);
	
	if (!g || !DtIsIcon ((Widget)g))
		return;

	if (event->button == Button2 || event->button == Button3)
	{
		if (event->type == ButtonPress)
			InputDispatch ((Widget) g, event, XmARM_EVENT);
		else if (event->type == ButtonRelease)
			InputDispatch ((Widget) g, event, XmACTIVATE_EVENT);
	}
}



/*-------------------------------------------------------------
**	ClickTimeout
**		Clear Click flags.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
ClickTimeout( w, id )
        Widget w ;
        XtIntervalId *id ;
#else
ClickTimeout(
        Widget w,
        XtIntervalId *id )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;
	Time		last_time, end_time;

	if (! G_Armed (g))
	{
		G_ClickTimerID (g) = 0;
		XtFree ((char *)G_ClickEvent (g));
		G_ClickEvent (g) = NULL;
		return;
	}

	last_time = XtLastTimestampProcessed (XtDisplay (g));
		/*
  		 * fix for bug# 4504
		 */
	if( G_ClickEvent (g) == NULL) return;

	end_time = G_ClickEvent (g) -> time + (Time)
				XtGetMultiClickTime (XtDisplay (g)); 

/*	Sync and reset timer if server interval may not have elapsed.
*/
	if ((last_time < end_time) && G_Sync (g))
	{
		G_Sync (g) = False;
		XSync (XtDisplay (g), False);
		G_ClickTimerID (g) = 
			XtAppAddTimeOut (XtWidgetToApplicationContext ((Widget)g),
					(unsigned long) 50, 
					(XtTimerCallbackProc)ClickTimeout, 
					(XtPointer) g);
	}
/*	Handle Select action.
*/
	else
	{
		G_ClickTimerID (g) = 0;
		G_Armed (g) = False;
		G_CallCallback (g, G_Callback (g), XmCR_SELECT,
					(XEvent *)G_ClickEvent (g));

	        if ((G_Behavior (g) == XmICON_DRAG) &&
                    (G_ShadowThickness (g) == 0))
                {
                   /* Do nothing */
                }
                else
		   G_Expose ((Widget)g, (XEvent*)G_ClickEvent (g), NULL);
		XtFree ((char *)G_ClickEvent (g));
		G_ClickEvent (g) = NULL;
	}
}



/*-------------------------------------------------------------
**	Action Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	IconArm
**		Handle Arm action.
*/
static void 
#ifdef _NO_PROTO
IconArm( w, event )
        Widget w ;
        XEvent *event ;
#else
IconArm(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;

	if (G_Behavior (g) == XmICON_LABEL)
		return;

	G_Armed (g) = True;

	if (G_Behavior (g) == XmICON_DRAG)
		G_CallCallback (g, G_Callback (g), XmCR_ARM, event);

	if ((G_Behavior (g) == XmICON_DRAG) &&
            (G_ShadowThickness (g) == 0))
        {
           /* Do nothing */
        }
        else
	   G_Expose ((Widget)g, event, NULL);
}



/*-------------------------------------------------------------
**	IconDisarm
**		Handle Disarm action.
*/
static void 
#ifdef _NO_PROTO
IconDisarm( w, event )
        Widget w ;
        XEvent *event ;
#else
IconDisarm(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;

	if (! G_Armed (g) || G_Behavior (g) == XmICON_LABEL)
		return;

	G_Armed (g) = False;

if (G_Behavior (g) == XmICON_DRAG)
	{
	G_CallCallback (g, G_Callback (g), XmCR_DISARM, event);
	G_Expose ((Widget)g, event, NULL);
	}
}



/*-------------------------------------------------------------
**	IconActivate
**		Handle Activate action.
*/
static void 
#ifdef _NO_PROTO
IconActivate( w, event )
        Widget w ;
        XEvent *event ;
#else
IconActivate(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget		g = 	(DtIconGadget) w;
	unsigned long		delay;
	XButtonEvent *		b_event = (XButtonEvent *) event;

	if (! G_Armed (g))
		return;

	if (G_Behavior (g) == XmICON_BUTTON)
	{
		G_Armed (g) = False;
		G_CallCallback (g, G_Callback (g), XmCR_ACTIVATE, event);
		G_Expose ((Widget)g, event, NULL);
	}

	else if (G_Behavior (g) == XmICON_TOGGLE)
	{
		G_Armed (g) = False;
		G_Set (g) = ! G_Set (g);

		G_CallCallback (g, G_Callback (g), XmCR_VALUE_CHANGED, event);
	}

	else if (G_Behavior (g) == XmICON_DRAG)
	{
		if (G_ClickTimerID (g))
		{
			G_ClickTimerID (g) = 0;
			XtFree ((char *)G_ClickEvent (g));
			G_ClickEvent (g) = NULL;
			G_Armed (g) = False;
			G_CallCallback (g, G_Callback (g),
					XmCR_DEFAULT_ACTION, event);
		}
		else
		{
			delay = (unsigned long)
				XtGetMultiClickTime (XtDisplay (g)); 
			G_ClickEvent (g) = (XButtonEvent *)
				XtMalloc (sizeof (XButtonEvent));
			*(G_ClickEvent (g)) = *b_event;
			G_Sync (g) = True;
			G_ClickTimerID (g) = 
				XtAppAddTimeOut (
					XtWidgetToApplicationContext ((Widget)g),
					delay, (XtTimerCallbackProc)ClickTimeout, 
					(XtPointer) g);
		}

                if (G_ShadowThickness (g) > 0)
		   G_Expose ((Widget)g, event, NULL);
	}
}



/*-------------------------------------------------------------
**	IconDrag
**		Handle Drag action.
*/
static void 
#ifdef _NO_PROTO
IconDrag( w, event )
        Widget w ;
        XEvent *event ;
#else
IconDrag(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;

	if (G_Behavior (g) == XmICON_DRAG)
		G_CallCallback (g, G_Callback (g), XmCR_DRAG, event);
}


/*-------------------------------------------------------------
**      IconPopup
**            Handle button 3 - popup's
*/
static void
#ifdef _NO_PROTO
IconPopup( w, event )
        Widget w ;
        XEvent *event ;
#else
IconPopup(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
        DtIconGadget   g =     (DtIconGadget) w;

        G_CallCallback (g, G_Callback (g), XmCR_POPUP, event);
}



/*-------------------------------------------------------------
**	IconEnter
**		Handle Enter action.
*/
static void 
#ifdef _NO_PROTO
IconEnter( w, event )
        Widget w ;
        XEvent *event ;
#else
IconEnter(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;

        _XmEnterGadget (w, (XEvent *)event,
                                 (String *)NULL,(Cardinal *)0);

	if (G_Armed (g))
		{
		if ((G_Behavior (g) == XmICON_BUTTON) ||
		    (G_Behavior (g) == XmICON_TOGGLE))
			G_Expose ((Widget)g, event, NULL);
		}
}


/*-------------------------------------------------------------
**	IconLeave
**		Handle Leave action.
*/
static void 
#ifdef _NO_PROTO
IconLeave( w, event )
        Widget w ;
        XEvent *event ;
#else
IconLeave(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconGadget	g = 	(DtIconGadget) w;

	_XmLeaveGadget (w, (XEvent *)event,
                               (String *)NULL,  (Cardinal *)0);
	if (G_Armed (g))
	{
	if ((G_Behavior (g) == XmICON_BUTTON) ||
	    (G_Behavior (g) == XmICON_TOGGLE))
		{
		G_Armed (g) = False;
		G_Expose ((Widget)g, event, NULL);
		G_Armed (g) = True;
		}
	}
}



/*-------------------------------------------------------------
**	Core Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	ClassInitialize
**		Initialize gadget class.
*/
static void 
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize( void )
#endif /* _NO_PROTO */
{
	_DtRegisterNewConverters ();

	iconBaseClassExtRec.record_type = XmQmotif;
}

/*-------------------------------------------------------------
**	ClassPartInitialize
**		Initialize gadget class part.
*/
static void
#ifdef _NO_PROTO
ClassPartInitialize (wc)
	WidgetClass	wc;
#else
ClassPartInitialize (
	WidgetClass	wc)
#endif
{
	DtIconGadgetClass ic =	(DtIconGadgetClass) wc;
	DtIconGadgetClass super =	(DtIconGadgetClass) ic->rect_class.superclass;

#ifdef _NO_PROTO
	if (C_GetSize (ic) == (XtWidgetProc) _XtInherit)
		C_GetSize (ic) = C_GetSize (super);
	if (C_GetPositions (ic) == (XtWidgetProc) _XtInherit)
		C_GetPositions (ic) = C_GetPositions (super);
	if (C_Draw (ic) == (XtWidgetProc) _XtInherit)
		C_Draw (ic) = C_Draw (super);
	if (C_CallCallback (ic) == (XtWidgetProc) _XtInherit)
		C_CallCallback (ic) = C_CallCallback (super);
	if (C_UpdateGCs (ic) == (XtWidgetProc) _XtInherit)
		C_UpdateGCs (ic) = C_UpdateGCs (super);
#else
	if (C_GetSize (ic) == (GetSizeProc) _XtInherit)
		C_GetSize (ic) = C_GetSize (super);
	if (C_GetPositions (ic) == (GetPositionProc) _XtInherit)
		C_GetPositions (ic) = C_GetPositions (super);
	if (C_Draw (ic) == (DrawProc) _XtInherit)
		C_Draw (ic) = C_Draw (super);
	if (C_CallCallback (ic) == (CallCallbackProc) _XtInherit)
		C_CallCallback (ic) = C_CallCallback (super);
	if (C_UpdateGCs (ic) == (UpdateGCsProc) _XtInherit)
		C_UpdateGCs (ic) = C_UpdateGCs (super);
#endif
}

/*-------------------------------------------------------------
**	Cache Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**      _DtIconCacheCompare
**
*/
int 
#ifdef _NO_PROTO
_DtIconCacheCompare( A, B )
        XtPointer A ;
        XtPointer B ;
#else
_DtIconCacheCompare(
        XtPointer A,
        XtPointer B )
#endif /* _NO_PROTO */
{
   DtIconCacheObjPart *icon_inst = (DtIconCacheObjPart *) A ;
   DtIconCacheObjPart *icon_cache_inst = (DtIconCacheObjPart *) B ;

   if ((icon_inst->fill_on_arm == icon_cache_inst->fill_on_arm) &&
       (icon_inst->recompute_size== icon_cache_inst->recompute_size) &&
       (icon_inst->pixmap_position== icon_cache_inst->pixmap_position) &&
       (icon_inst->string_position== icon_cache_inst->string_position) &&
       (icon_inst->alignment == icon_cache_inst->alignment) &&
       (icon_inst->behavior == icon_cache_inst->behavior) &&
       (icon_inst->draw_shadow == icon_cache_inst->draw_shadow) &&
       (icon_inst->fill_mode == icon_cache_inst->fill_mode) &&
       (icon_inst->margin_width== icon_cache_inst->margin_width) &&
       (icon_inst->margin_height== icon_cache_inst->margin_height) &&
       (icon_inst->string_height== icon_cache_inst->string_height) &&
       (icon_inst->spacing== icon_cache_inst->spacing) &&
       (icon_inst->foreground== icon_cache_inst->foreground) &&
       (icon_inst->background== icon_cache_inst->background) &&
       (icon_inst->arm_color== icon_cache_inst->arm_color))
       return 1;
   else
       return 0;

}


/*-------------------------------------------------------------
**      SecondaryObjectCreate
**
*/
static void 
#ifdef _NO_PROTO
SecondaryObjectCreate( req, new_w, args, num_args )
        Widget req ;
        Widget new_w ;
        ArgList args ;
        Cardinal *num_args ;
#else
SecondaryObjectCreate(
        Widget req,
        Widget new_w,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
  XmBaseClassExt              *cePtr;
  XmWidgetExtData             extData;
    WidgetClass                 wc;
    Cardinal                    size;
    XtPointer                   newSec, reqSec;

    cePtr = _XmGetBaseClassExtPtr(XtClass(new_w), XmQmotif);

    wc = (*cePtr)->secondaryObjectClass;
    size = wc->core_class.widget_size;

    newSec = _XmExtObjAlloc(size);
    reqSec = _XmExtObjAlloc(size);

    /*
     * fetch the resources in superclass to subclass order
     */

    XtGetSubresources(new_w,
                      newSec,
                      NULL, NULL,
                      wc->core_class.resources,
                      wc->core_class.num_resources,
                      args, *num_args );

    extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    extData->widget = (Widget)newSec;
    extData->reqWidget = (Widget)reqSec;

    ((DtIconCacheObject)newSec)->ext.extensionType = XmCACHE_EXTENSION;
    ((DtIconCacheObject)newSec)->ext.logicalParent = new_w;

    _XmPushWidgetExtData(new_w, extData,
                         ((DtIconCacheObject)newSec)->ext.extensionType);
    memcpy(reqSec, newSec, size);

    /*
     * fill out cache pointers
     */
    Icon_Cache(new_w) = &(((DtIconCacheObject)extData->widget)->icon_cache);
    Icon_Cache(req) = &(((DtIconCacheObject)extData->reqWidget)->icon_cache);

}


/*-------------------------------------------------------------
**      InitializePostHook
**
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
InitializePosthook( req, new, args, num_args )
        Widget req ;
        Widget new ;
        ArgList args ;
        Cardinal *num_args ;
#else
InitializePosthook(
        Widget req,
        Widget new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData     ext;
    DtIconGadget lw = (DtIconGadget)new;

    /*
     * - register parts in cache.
     * - update cache pointers
     * - and free req
     */

     Icon_Cache(lw) = (DtIconCacheObjPart *)
           _XmCachePart( Icon_ClassCachePart(lw),
                         Icon_Cache(lw),
                         sizeof(DtIconCacheObjPart));

    /*
     * might want to break up into per-class work that gets explicitly
     * chained. For right now, each class has to replicate all
     * superclass logic in hook routine
     */

    /*     * free the req subobject used for comparisons
     */
     _XmPopWidgetExtData((Widget) lw, &ext, XmCACHE_EXTENSION);
    _XmExtObjFree((XtPointer)ext->widget);
    _XmExtObjFree((XtPointer)ext->reqWidget);
    XtFree( (char *) ext);
}

/*-------------------------------------------------------------
**	SetValuesPrehook
**
*/
/* ARGSUSED */
static Boolean 
#ifdef _NO_PROTO
SetValuesPrehook( oldParent, refParent, newParent, args, num_args )
        Widget oldParent ;
        Widget refParent ;
        Widget newParent ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValuesPrehook(
        Widget oldParent,
        Widget refParent,
        Widget newParent,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData             extData;
    XmBaseClassExt              *cePtr;
    WidgetClass                 ec;
    DtIconCacheObject          newSec, reqSec;
    Cardinal                    size;

    cePtr = _XmGetBaseClassExtPtr(XtClass(newParent), XmQmotif);
    ec = (*cePtr)->secondaryObjectClass;
    size = ec->core_class.widget_size;

    /* allocate copies and fill from cache */
    newSec = (DtIconCacheObject) _XmExtObjAlloc(size);
    reqSec = (DtIconCacheObject) _XmExtObjAlloc(size);

    newSec->object.self = (Widget)newSec;
    newSec->object.widget_class = ec;
    newSec->object.parent = XtParent(newParent);
    newSec->object.xrm_name = newParent->core.xrm_name;
    newSec->object.being_destroyed = False;
    newSec->object.destroy_callbacks = NULL;
    newSec->object.constraints = NULL;

    newSec->ext.logicalParent = newParent;
    newSec->ext.extensionType = XmCACHE_EXTENSION;

    memcpy(&(newSec->icon_cache),
           Icon_Cache(newParent),
           sizeof(DtIconCacheObjPart));

    extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    extData->widget = (Widget)newSec;
    extData->reqWidget = (Widget)reqSec;
    _XmPushWidgetExtData(newParent, extData, XmCACHE_EXTENSION);

    _XmGadgetImportSecondaryArgs(newParent, args, num_args);
    XtSetSubvalues((XtPointer)newSec,
                   ec->core_class.resources,
                   ec->core_class.num_resources,
                   args, *num_args);

    _XmExtImportArgs((Widget)newSec, args, num_args);

    memcpy((XtPointer)reqSec, (XtPointer)newSec, size);

    Icon_Cache(newParent) = &(((DtIconCacheObject)newSec)->icon_cache);
    Icon_Cache(refParent) =
			&(((DtIconCacheObject)extData->reqWidget)->icon_cache);

    return FALSE;
}


/*-------------------------------------------------------------
**	GetValuesPrehook
**
*/
static void 
#ifdef _NO_PROTO
GetValuesPrehook( newParent, args, num_args )
        Widget newParent ;
        ArgList args ;
        Cardinal *num_args ;
#else
GetValuesPrehook(
        Widget newParent,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData             extData;
    XmBaseClassExt              *cePtr;
    WidgetClass                 ec;
    DtIconCacheObject          newSec;
    Cardinal                    size;

    cePtr = _XmGetBaseClassExtPtr(XtClass(newParent), XmQmotif);
    ec = (*cePtr)->secondaryObjectClass;
    size = ec->core_class.widget_size;

    newSec = (DtIconCacheObject)_XmExtObjAlloc(size);

    newSec->object.self = (Widget)newSec;
    newSec->object.widget_class = ec;
    newSec->object.parent = XtParent(newParent);
    newSec->object.xrm_name = newParent->core.xrm_name;
    newSec->object.being_destroyed = False;
    newSec->object.destroy_callbacks = NULL;
    newSec->object.constraints = NULL;

    newSec->ext.logicalParent = newParent;
    newSec->ext.extensionType = XmCACHE_EXTENSION;

    memcpy( &(newSec->icon_cache),
            Icon_Cache(newParent),
            sizeof(DtIconCacheObjPart));

    extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    extData->widget = (Widget)newSec;
    _XmPushWidgetExtData(newParent, extData, XmCACHE_EXTENSION);

    XtGetSubvalues((XtPointer)newSec,
                   ec->core_class.resources,
                   ec->core_class.num_resources,
                   args, *num_args);

    _XmExtGetValuesHook((Widget)newSec, args, num_args);
}



/*-------------------------------------------------------------
**	GetValuesPosthook
**
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
GetValuesPosthook( new, args, num_args )
        Widget new ;
        ArgList args ;
        Cardinal *num_args ;
#else
GetValuesPosthook(
        Widget new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
 XmWidgetExtData             ext;

 _XmPopWidgetExtData(new, &ext, XmCACHE_EXTENSION);

 _XmExtObjFree((XtPointer)ext->widget);
 XtFree((char *)ext);
}



/*-------------------------------------------------------------
**	SetValuesPosthook
**
*/
/* ARGSUSED */
static Boolean 
#ifdef _NO_PROTO
SetValuesPosthook( current, req, new, args, num_args )
        Widget current ;
        Widget req ;
        Widget new ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValuesPosthook(
        Widget current,
        Widget req,
        Widget new,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmWidgetExtData             ext;

    /*
     * - register parts in cache.
     * - update cache pointers
     * - and free req
     */


      /* assign if changed! */
      if (!_DtIconCacheCompare((XtPointer)Icon_Cache(new),
                          (XtPointer)Icon_Cache(current)))

      {
         _XmCacheDelete(Icon_Cache(current));  /* delete the old one */
          Icon_Cache(new) = (DtIconCacheObjPart *)
              _XmCachePart(Icon_ClassCachePart(new),
                           Icon_Cache(new),
                           sizeof(DtIconCacheObjPart));
      } else
          Icon_Cache(new) = Icon_Cache(current);

      _XmPopWidgetExtData(new, &ext, XmCACHE_EXTENSION);

      _XmExtObjFree((XtPointer)ext->widget);
      _XmExtObjFree((XtPointer)ext->reqWidget);

      XtFree((char *)ext);

      return FALSE;
}


/*--------------------------------------------------------------------------
**      Cache Assignment Help
**     		These routines are for manager widgets that go into Icon's
**     		fields and set them, instead of doing a SetValues.
**--------------------------------------------------------------------------
*/

static DtIconCacheObjPart local_cache;
static Boolean local_cache_inited = FALSE;


/*--------------------------------------------------------------------------
**	QualifyIconLocalCache
**		Checks to see if local cache is set up
*/
static void 
#ifdef _NO_PROTO
QualifyIconLocalCache( g )
        DtIconGadget g ;
#else
QualifyIconLocalCache(
        DtIconGadget g )
#endif /* _NO_PROTO */
{
    if (!local_cache_inited)
    {
        local_cache_inited = TRUE;
        ClassCacheCopy(Icon_ClassCachePart(g))(Icon_Cache(g), &local_cache,
            sizeof(local_cache));
    }
}

/************************************************************************
 *
 * _DtReCacheIcon()
 * Check to see if ReCaching is necessary as a result of fields having
 * been set by a mananger widget. This routine is called by the
 * manager widget in their SetValues after a change is made to any
 * of Icon's cached fields.
 *
 ************************************************************************/

void 
#ifdef _NO_PROTO
_DtReCacheIcon( g )
        DtIconGadget g ;
#else
_DtReCacheIcon(
        DtIconGadget g )
#endif /* _NO_PROTO */
{
     if (local_cache_inited &&
        (!_DtIconCacheCompare( (XtPointer)&local_cache, (XtPointer)Icon_Cache(g))))
     {
           _XmCacheDelete( (XtPointer) Icon_Cache(g));   /* delete the old one */
           Icon_Cache(g) = (DtIconCacheObjPart *)_XmCachePart(
               Icon_ClassCachePart(g), (XtPointer) &local_cache, sizeof(local_cache));
     }
     local_cache_inited = FALSE;
}


/*--------------------------------------------------------------------------
**	_DtAssignIcon_StringHeight
**
*/
void 
#ifdef _NO_PROTO
_DtAssignIcon_StringHeight( g, value )
        DtIconGadget g ;
        Dimension value ;
#else
_DtAssignIcon_StringHeight(
        DtIconGadget g,
        Dimension value )
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.string_height = value;
}



/*--------------------------------------------------------------------------
**	_DtAssignIcon_Foreground
**
*/
void 
#ifdef _NO_PROTO
_DtAssignIcon_Foreground( g, value )
        DtIconGadget g ;
        Pixel value ;
#else
_DtAssignIcon_Foreground(
        DtIconGadget g,
        Pixel value )
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.foreground = value;
}


/*--------------------------------------------------------------------------
**	_DtAssignIcon_Background
**
*/
void 
#ifdef _NO_PROTO
_DtAssignIcon_Background( g, value )
        DtIconGadget g ;
        Pixel value ;
#else
_DtAssignIcon_Background(
        DtIconGadget g,
        Pixel value )
#endif /* _NO_PROTO */
{
       QualifyIconLocalCache(g);
       local_cache.background = value;
}

/*********************************************************************
 *
 *  GetParentBackgroundGC
 *      Get the graphics context used for erasing their highlight border.
 *
 *********************************************************************/
static void
#ifdef _NO_PROTO
GetParentBackgroundGC( g )
        DtIconGadget g ;
#else
GetParentBackgroundGC(
        DtIconGadget g )
#endif /* _NO_PROTO */
{
   XGCValues values;
   XtGCMask  valueMask;
   Widget    parent = XtParent((Widget)g);

   valueMask = GCForeground | GCBackground;
   values.foreground = parent->core.background_pixel;

   if (XmIsManager(parent))

      values.background = ((XmManagerWidget) parent)->manager.foreground;
   else
      values.background = ((XmPrimitiveWidget) parent)->primitive.foreground;

   if ((parent->core.background_pixmap != None) &&
       (parent->core.background_pixmap != XmUNSPECIFIED_PIXMAP))
   {
      valueMask |= GCFillStyle | GCTile;
      values.fill_style = FillTiled;
      values.tile = parent->core.background_pixmap;
   }

   G_SavedParentBG(g) = parent->core.background_pixel;

   G_ParentBackgroundGC(g) = XtGetGC (parent, valueMask, &values);
}


/*-------------------------------------------------------------
**	Initialize
**		Initialize a new gadget instance.
*/
static void 
#ifdef _NO_PROTO
Initialize( request_w, new_w )
        Widget request_w ;
        Widget new_w ;
#else
Initialize(
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
	DtIconGadget	request =	(DtIconGadget) request_w,
			new =		(DtIconGadget) new_w;
	Window		root;
	int		int_x = 0, int_y = 0;
	unsigned int	int_w = 0, int_h = 0,
			int_bw, depth;
	Dimension	w, h;
	EventMask	mask;
	String		name = NULL;

	G_Sync (new) = False;

/*	Validate behavior.
*/
	if (G_Behavior (new) != XmICON_LABEL &&
	    G_Behavior (new) != XmICON_BUTTON &&
	    G_Behavior (new) != XmICON_TOGGLE &&
	    G_Behavior (new) != XmICON_DRAG)
	{
		_XmWarning ((Widget)new, (char*)WARN_BEHAVIOR);
		G_Behavior (new) = XmICON_BUTTON;
	}

/*	Set the input mask for events handled by Manager.
*/
	G_EventMask (new) = (XmARM_EVENT | XmACTIVATE_EVENT |
			XmMULTI_ARM_EVENT | XmMULTI_ACTIVATE_EVENT |
			XmHELP_EVENT | XmFOCUS_IN_EVENT | XmKEY_EVENT |
			XmFOCUS_OUT_EVENT | XmENTER_EVENT | XmLEAVE_EVENT);

/*	Add event handler for icon events.
*/
	if (G_Behavior (new) == XmICON_DRAG)
	{
		mask = ButtonPressMask|ButtonReleaseMask;
		XtAddEventHandler (XtParent (new), mask, False,
				(XtEventHandler) IconEventHandler, 0);
	}

	G_ClickTimerID (new) = 0;
	G_ClickEvent (new) = NULL;

	G_Armed (new) = False;

	G_Mask (new) = NULL;

	if (G_Pixmap (new) == XmUNSPECIFIED_PIXMAP)
		G_Pixmap (new) = NULL;

	if (G_ImageName (new) || G_Pixmap (new))
	{
		if (G_ImageName (new))
		{

/*	Try to get pixmap from image name.
*/
			G_Pixmap (new) = 
				XmGetPixmap (XtScreen (new), G_ImageName (new),
					G_PixmapForeground (new),
					G_PixmapBackground (new));
			if (G_Pixmap (new) != XmUNSPECIFIED_PIXMAP) 
			  {
			      name = G_ImageName (new);
			      G_Mask (new) = 
				_DtGetMask(XtScreen (new), G_ImageName
					   (new));
			  }
			else
			{
/* warning? */				
				name = NULL;
				G_Pixmap (new) = NULL;
			}
		}

/*	Update width and height; copy image name.
*/
		if (G_Pixmap (new))
		{
			XGetGeometry (XtDisplay (new), G_Pixmap (new),
				&root, &int_x, &int_y, &int_w, &int_h,
				&int_bw, &depth);
		}
		if (name)
		{
			G_ImageName (new) = XtNewString(name);
		}
		else
			G_ImageName (new) = NULL;
	}
	G_PixmapWidth(new) = Limit((Dimension) int_w, G_MaxPixmapWidth(new));
	G_PixmapHeight(new) = Limit((Dimension) int_h, G_MaxPixmapHeight(new));

/*	Validate fill mode.
*/
	if (G_FillMode (new) != XmFILL_NONE &&
	    G_FillMode (new) != XmFILL_PARENT &&
	    G_FillMode (new) != XmFILL_TRANSPARENT &&
	    G_FillMode (new) != XmFILL_SELF)
	{
		_XmWarning ((Widget)new, (char*)WARN_FILL_MODE);
		if (G_ShadowThickness (new) > 0)
			G_FillMode (new) = XmFILL_SELF;
		else
			G_FillMode (new) = XmFILL_PARENT;
	}

/*	Validate pixmap position.
*/
	if (G_StringPosition (new) != UNSPECIFIED_CHAR)
		G_PixmapPosition (new) = G_StringPosition (new);

	if (G_PixmapPosition (new) == UNSPECIFIED_CHAR)
		G_PixmapPosition (new) = XmPIXMAP_LEFT;
	else if (G_PixmapPosition (new) != XmPIXMAP_LEFT &&
		 G_PixmapPosition (new) != XmPIXMAP_RIGHT &&
		 G_PixmapPosition (new) != XmPIXMAP_TOP &&
		 G_PixmapPosition (new) != XmPIXMAP_BOTTOM &&
		 G_PixmapPosition (new) != XmPIXMAP_MIDDLE)
	{
		_XmWarning ((Widget)new, (char*)WARN_PIXMAP_POSITION);
		G_PixmapPosition (new) = XmPIXMAP_LEFT;
	}
	G_StringPosition (new) = G_PixmapPosition (new);

/*	Validate alignment.
*/
	if (G_Alignment (new) != XmALIGNMENT_BEGINNING &&
	    G_Alignment (new) != XmALIGNMENT_CENTER &&
	    G_Alignment (new) != XmALIGNMENT_END)
	{
		_XmWarning ((Widget)new, (char*)WARN_ALIGNMENT);
		G_Alignment (new) = XmALIGNMENT_BEGINNING;
	}

/*	Validate shadow type.
*/
	if (G_ShadowType (new) != XmSHADOW_IN &&
	    G_ShadowType (new) != XmSHADOW_OUT &&
	    G_ShadowType (new) != XmSHADOW_ETCHED_IN &&
	    G_ShadowType (new) != XmSHADOW_ETCHED_OUT)
	{
		_XmWarning ((Widget)new, (char*)WARN_SHADOW_TYPE);
		if (G_Behavior (new) == XmICON_BUTTON)
			G_ShadowType (new) = XmSHADOW_OUT;
		else if (G_Behavior (new) == XmICON_TOGGLE)
			G_ShadowType (new) = (G_Set (new))
				? XmSHADOW_ETCHED_IN : XmSHADOW_ETCHED_OUT;
	}

/*	Copy fontlist.
*/
	if (G_FontList (new) == NULL)
		G_FontList (new) =
			_XmGetDefaultFontList ((Widget)new, XmBUTTON_FONTLIST);
	G_FontList (new) = XmFontListCopy (G_FontList (new));

	if ((unsigned int)G_String (new) == (unsigned int)XmUNSPECIFIED_STRING)
		G_String (new) = (_XmString) NULL;
        
	if (G_String (new))
	{
		G_String (new) = _XmStringCreate ((XmString)G_String (new));
		_XmStringExtent (G_FontList (new), G_String (new),
					&w, &h);
                if (G_Underline(new))
                   h++;
	}
	else
		w = h = 0;
	
	G_StringWidth (new) = w;
	G_StringHeight (new) = h;

/*	Convert margins to pixel units.
*/
	if (G_UnitType (new) != XmPIXELS)
	{
		G_MarginWidth (new) = 
			_XmToHorizontalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_MarginWidth (new));
		G_MarginHeight (new) = 
			_XmToVerticalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_MarginHeight (new));
	}

/*	Check for unspecified margins.
*/
	if (G_MarginWidth (request) == UNSPECIFIED_DIMENSION)
		G_MarginWidth (new) = MARGIN_DEFAULT;
	if (G_MarginHeight (request) == UNSPECIFIED_DIMENSION)
		G_MarginHeight (new) = MARGIN_DEFAULT;

/*	Convert spacing.
*/
	if (G_Spacing (new) == UNSPECIFIED_DIMENSION)
	{
		G_Spacing (new) = G_StringHeight (new) / 5;
		if (G_Spacing (new) < SPACING_DEFAULT)
			G_Spacing (new) = SPACING_DEFAULT;
	}
	else if (G_Spacing (new) && G_UnitType (new) != XmPIXELS)
	{
		G_Spacing (new) = 
			(G_PixmapPosition (new) == XmPIXMAP_LEFT ||
			 G_PixmapPosition (new) == XmPIXMAP_RIGHT)
			? _XmToHorizontalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_Spacing (new))
			: _XmToVerticalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_Spacing (new));
	}

/*	Set width and height.
*/
	if (G_Width (request) == 0 || G_Height (request) == 0)
	{
		G_GetSize (new, &w, &h);
		if (G_Width (request) == 0)
			G_Width (new) = w;
		if (G_Height (request) == 0)
			G_Height (new) = h;
	}

/*  	Get graphics contexts.
*/
	G_NormalGC (new) = NULL;
	G_ClipGC (new) = NULL;
	G_BackgroundGC (new) = NULL;
	G_ArmedGC (new) = NULL;
	G_ArmedBackgroundGC (new) = NULL;
	G_UpdateGCs (new);

        GetParentBackgroundGC(new);

        if (G_Operations(new) != XmDROP_NOOP) {
	   _DtIconRegisterDropsite(new_w);
        }
}



/*-------------------------------------------------------------
**	Destroy
**		Release resources allocated for gadget.
*/
static void 
#ifdef _NO_PROTO
Destroy( w )
        Widget w ;
#else
Destroy(
        Widget w )
#endif /* _NO_PROTO */
{
	DtIconGadget	g =	(DtIconGadget) w;
	XmManagerWidget mw = (XmManagerWidget) XtParent(g);

	if (G_ClickTimerID (g))
		XtRemoveTimeOut (G_ClickTimerID (g));

	XtFree ((char *)G_ClickEvent (g));

	if (G_String (g) != NULL)
		_XmStringFree (G_String (g));

	if (G_ImageName (g) != NULL)
	{
		XtFree (G_ImageName (g));
		if (G_Mask (g) != XmUNSPECIFIED_PIXMAP)
		  XmDestroyPixmap (XtScreen(w),G_Mask (g));
		XmDestroyPixmap (XtScreen(w),G_Pixmap (g));
	}

	XmFontListFree (G_FontList (g));

	_XmCacheDelete(Icon_Cache(w));

	XtReleaseGC ((Widget)mw, G_NormalGC (g));
	XtReleaseGC ((Widget)mw, G_ClipGC (g));
	XtReleaseGC ((Widget)mw, G_BackgroundGC (g));
	XtReleaseGC ((Widget)mw, G_ArmedGC (g));
	XtReleaseGC ((Widget)mw, G_ArmedBackgroundGC (g));

/* remove event handler if last Icon in parent? */
}



/*-------------------------------------------------------------
**	Resize
**		Set clip rect?
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
Resize( w )
        Widget w ;
#else
Resize(
        Widget w )
#endif /* _NO_PROTO */
{
}


/*-------------------------------------------------------------
**	Redisplay
**		Redisplay gadget.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
Redisplay( w, event, region )
        Widget w ;
        XEvent *event ;
        Region region ;
#else
Redisplay(
        Widget w,
        XEvent *event,
        Region region )
#endif /* _NO_PROTO */
{
	DtIconGadget	g =		(DtIconGadget) w;
	Dimension	s_t =		G_ShadowThickness (g);
	unsigned char	fill_mode =	G_FillMode (g);

	if (! XtIsManaged (g))
		return;

/*	Draw gadget to window.
*/
	G_Draw (g, XtWindow (g), G_X (g), G_Y (g), G_Width (g), G_Height (g),
		G_HighlightThickness (g), s_t, G_ShadowType (g), fill_mode);

/*	Draw highlight if highlighted.
*/
	if (G_Highlighted (g))
                BorderHighlight( (DtIconGadget)g );
	else if (_XmDifferentBackground ((Widget)g, XtParent (g)))
                BorderUnhighlight( (DtIconGadget)g );
}



/*-------------------------------------------------------------
**	SetValues
**		
*/
/* ARGSUSED */
static Boolean 
#ifdef _NO_PROTO
SetValues( current_w, request_w, new_w )
        Widget current_w ;
        Widget request_w ;
        Widget new_w ;
#else
SetValues(
        Widget current_w,
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
	DtIconGadget	current =	(DtIconGadget) current_w,
			new =		(DtIconGadget) new_w;
	DtIconGadgetClass	wc =	(DtIconGadgetClass) XtClass (new);

	Window		root;
	int		int_x = 0, int_y = 0;
	unsigned int	int_w = 0, int_h = 0,
			int_bw, depth;
	Dimension	w, h;
	Boolean		new_image_name = False,
			redraw_flag = False,
			draw_pixmap = False,
			draw_string = False,
			draw_shadow = False;
	Dimension	h_t = G_HighlightThickness (new),
			s_t = G_ShadowThickness (new),
			p_x, p_y, s_x, s_y;
	String		name = NULL;

/*	If unchanged, reuse old image name
*/
	if (G_ImageName (current) != G_ImageName (new) &&
	    G_ImageName (new) &&
	    G_ImageName (current) &&
	    strcmp(G_ImageName (current), G_ImageName (new)) == 0)
	{
		G_ImageName (new) = G_ImageName (current);
	}

/*	Validate behavior
*/
	if (G_Behavior (new) != G_Behavior (current))
	{
		if (G_Behavior (new) != XmICON_LABEL &&
		    G_Behavior (new) != XmICON_BUTTON &&
		    G_Behavior (new) != XmICON_TOGGLE &&
		    G_Behavior (new) != XmICON_DRAG)
		{
			_XmWarning ((Widget)new, (char*)WARN_BEHAVIOR);
			G_Behavior (new) = G_Behavior (current);
		}

		if (G_Behavior (new) == XmICON_DRAG)
		{
			EventMask	mask;

			mask = ButtonPressMask|ButtonReleaseMask;
			XtAddEventHandler (XtParent (new), mask, False,
					(XtEventHandler) IconEventHandler, 0);
		}
	}

/*	Reset the interesting input types.
*/
	G_EventMask (new) |= (XmARM_EVENT | XmACTIVATE_EVENT |
			XmMULTI_ARM_EVENT | XmMULTI_ACTIVATE_EVENT |
			XmHELP_EVENT | XmFOCUS_IN_EVENT | XmKEY_EVENT |
			XmFOCUS_OUT_EVENT | XmENTER_EVENT | XmLEAVE_EVENT);

/*	Check for new image name.
*/
	if (G_ImageName (new) && (G_ImageName (current) != G_ImageName (new)))
		new_image_name = True;

/*	Validate shadow type.
*/
	if ((G_ShadowType (new) != G_ShadowType (current)) ||
	    (G_Behavior (new) == XmICON_TOGGLE &&
	     G_Set (new) != G_Set (current)))
	{
		if (G_ShadowType (new) != XmSHADOW_IN &&
		    G_ShadowType (new) != XmSHADOW_OUT &&
		    G_ShadowType (new) != XmSHADOW_ETCHED_IN &&
		    G_ShadowType (new) != XmSHADOW_ETCHED_OUT)
		{
			_XmWarning ((Widget)new, (char*)WARN_SHADOW_TYPE);
			G_ShadowType (new) = G_ShadowType (current); 
		}

/*	Disallow change if conflict with set or armed state.
*/
		else if (((G_Behavior (new) == XmICON_TOGGLE) &&
			  ((G_Set (new) && ! G_Armed (new)) ||
			   (! G_Set (new) && G_Armed (new)))) ||
			 ((G_Behavior (new) == XmICON_BUTTON) &&
			  (G_Armed (new))))
		{
			if (G_ShadowType (new) == XmSHADOW_OUT)
				G_ShadowType (new) = XmSHADOW_IN;
			else if (G_ShadowType (new) == XmSHADOW_ETCHED_OUT)
				G_ShadowType (new) = XmSHADOW_ETCHED_IN;
		}
		else if (((G_Behavior (new) == XmICON_TOGGLE) &&
			  ((G_Set (new) && G_Armed (new)) ||
			   (! G_Set (new) && ! G_Armed (new)))) ||
			 ((G_Behavior (new) == XmICON_BUTTON) &&
			  (! G_Armed (new))))
		{
			if (G_ShadowType (new) == XmSHADOW_IN)
				G_ShadowType (new) = XmSHADOW_OUT;
			else if (G_ShadowType (new) == XmSHADOW_ETCHED_IN)
				G_ShadowType (new) = XmSHADOW_ETCHED_OUT;
		}

		if (G_ShadowType (new) != G_ShadowType (current))
			draw_shadow = True;
	}

/*	Validate alignment.
*/
	if (G_Alignment (new) != G_Alignment (current))
	{
		if (G_Alignment (new) != XmALIGNMENT_BEGINNING &&
		    G_Alignment (new) != XmALIGNMENT_CENTER &&
		    G_Alignment (new) != XmALIGNMENT_END)
		{
			_XmWarning ((Widget)new, (char*)WARN_ALIGNMENT);
			G_Alignment (new) = G_Alignment (current);
		}
		else
			redraw_flag = True;
	}


/*	Validate fill mode.
*/
	if (G_FillMode (new) != G_FillMode (current))
	{
		if (G_FillMode (new) != XmFILL_NONE &&
		    G_FillMode (new) != XmFILL_PARENT &&
		    G_FillMode (new) != XmFILL_TRANSPARENT &&
		    G_FillMode (new) != XmFILL_SELF)
		{
			_XmWarning ((Widget)new, (char*)WARN_FILL_MODE);
			G_FillMode (new) = G_FillMode (current);
		}
	}

/*	Validate pixmap position.
*/
	if (G_StringPosition (new) != G_StringPosition (current))
		G_PixmapPosition (new) = G_StringPosition (new);

	if (G_PixmapPosition (new) != G_PixmapPosition (current))
	{
		if (G_PixmapPosition (new) != XmPIXMAP_LEFT &&
		    G_PixmapPosition (new) != XmPIXMAP_RIGHT &&
		    G_PixmapPosition (new) != XmPIXMAP_TOP &&
		    G_PixmapPosition (new) != XmPIXMAP_BOTTOM &&
		    G_PixmapPosition (new) != XmPIXMAP_MIDDLE)
		{
			_XmWarning ((Widget)new, (char*)WARN_PIXMAP_POSITION);
			G_PixmapPosition (new) = G_PixmapPosition (current); 
		}
		else
			redraw_flag = True;

		G_StringPosition (new) = G_PixmapPosition (new);
	}

/*	Update pixmap if pixmap foreground or background changed.
*/
	if (G_PixmapForeground (current) != G_PixmapForeground (new) ||
	    G_PixmapBackground (current) != G_PixmapBackground (new))
	{
		if (G_Pixmap (current) == G_Pixmap (new) &&
		    (G_ImageName (new) != NULL) &&
		    (! new_image_name))
		{
			draw_pixmap = True;
			if (G_Mask(new) != XmUNSPECIFIED_PIXMAP) 
			  XmDestroyPixmap( XtScreen(new), G_Mask(current));
			XmDestroyPixmap (XtScreen(new),G_Pixmap (current));
			G_Pixmap (new) = 
				XmGetPixmap (XtScreen (new), G_ImageName (new),
					G_PixmapForeground (new),
					G_PixmapBackground (new));
			if (G_Pixmap(new) != XmUNSPECIFIED_PIXMAP) 
			  G_Mask (new) = 
			    _DtGetMask(XtScreen (new), G_ImageName (new));

			/*
			 * This is because of ImageCache bug/feature
			 * if a missed entry is created , it always
			 * remains there, even if the image file is now
			 * present . This causes problems in the renaming
			 * procedure of dtfile app. If a missed entry is
			 * created, we remove it so when next time a lookup
			 * is done, an image will be returned if image_file is 
			 * present.
			 */

			if(_XmIsMissedImage (G_ImageName (new)))
                                _XmRemoveMissedImageEntry(G_ImageName (new));
		}
	}


/*	Check for change in image name.
*/
	if (new_image_name)
	{

/*	Try to get pixmap from image name.
*/
		if (G_ImageName (current) != NULL) 
		  {
		      if (G_Mask(new) != XmUNSPECIFIED_PIXMAP)
			XmDestroyPixmap (XtScreen(new),G_Mask(current));
		      XmDestroyPixmap (XtScreen(new),G_Pixmap (current));
		  }
		G_Pixmap (new) = 
			XmGetPixmap (XtScreen (new), G_ImageName (new),
				G_PixmapForeground (new),
				G_PixmapBackground (new));

		if (G_Pixmap (new) != XmUNSPECIFIED_PIXMAP)
		{
    		    G_Mask(new) = (Pixmap)_DtGetMask(XtScreen(new), G_ImageName(new));
		    XGetGeometry (XtDisplay (new), G_Pixmap (new),
				  &root, &int_x, &int_y, &int_w, &int_h,
				  &int_bw, &depth);
		    name = G_ImageName (new);
		    w = Limit((Dimension) int_w, G_MaxPixmapWidth(new));
		    h = Limit((Dimension) int_h, G_MaxPixmapHeight(new));
		}
		else
		{
			name = NULL;
			G_Pixmap (new) = NULL;
			w = 0;
			h = 0;
		}

/*	If resetting to current image name, then reuse old copy.
*/
		if (name && G_ImageName (current)
		    && (! strcmp (G_ImageName (new), G_ImageName (current))))
		{
			G_ImageName (new) = G_ImageName (current);
			name = G_ImageName (current);
		}
		else 
		{
		    if (name)
		      G_ImageName (new) = XtNewString(name);
		    else
		      G_ImageName (new) = NULL;
		    if (G_ImageName (current))
		      XtFree (G_ImageName (current));
		}

		if (G_Pixmap (new) != G_Pixmap (current))
		{
			if ((G_Pixmap (new) != NULL) &&
			    (G_PixmapWidth (new) == w) &&
			    (G_PixmapHeight (new) == h))
			{
				draw_pixmap = True;
			}
			else
			{
				redraw_flag = True;
				G_PixmapWidth (new) = w;
				G_PixmapHeight (new) = h;
			}
		}
	}

/*	Release image name and pixmap if name set to null.
*/
	else if (G_Pixmap (new) == G_Pixmap (current))
	{
		if ((G_ImageName (current) != NULL) && 
		    (G_ImageName (new) == NULL))
		{
			redraw_flag = True;
			XtFree (G_ImageName (current));
			if (G_Mask(new) != XmUNSPECIFIED_PIXMAP)
			  XmDestroyPixmap (XtScreen(new),G_Mask (current));
			XmDestroyPixmap (XtScreen(new),G_Pixmap (current));
			G_Pixmap (new) = NULL;			
			G_PixmapWidth (new) = 0;
			G_PixmapHeight (new) = 0;
		}
	}

/*	Process change in pixmap.
*/
	else if (G_Pixmap (new) != G_Pixmap (current)) 
	{
		if (G_Pixmap (new))
		{
			XGetGeometry (XtDisplay (new), G_Pixmap (new), &root,
					&int_x, &int_y, &int_w, &int_h,
					&int_bw, &depth);
			w = Limit((Dimension) int_w, G_MaxPixmapWidth(new));
			h = Limit((Dimension) int_h, G_MaxPixmapHeight(new));
		}
		else
		{
			if (G_ImageName (current) != NULL)
			{
				XtFree (G_ImageName (current));
				if (G_Mask(new) != XmUNSPECIFIED_PIXMAP)
				  XmDestroyPixmap (XtScreen(new),G_Mask (current));
				XmDestroyPixmap (XtScreen(new),G_Pixmap (current));
				G_ImageName (new) = NULL;
			}
			w = h = 0;
		}

		if (G_Pixmap (new) &&
		    (G_PixmapWidth (new) == w) &&
		    (G_PixmapHeight (new) == h))
		{
			draw_pixmap = True;
		}
		else
		{
			redraw_flag = True;
			G_PixmapWidth (new) = w;
			G_PixmapHeight (new) = h;
		}
	}

        if( ( G_MaxPixmapWidth(new) != G_MaxPixmapWidth(current)) ||
             (G_MaxPixmapHeight(new) != G_MaxPixmapHeight(current)) )
        {
                if (G_Pixmap (new))
                {
                        XGetGeometry (XtDisplay (new), G_Pixmap (new), &root,
                                        &int_x, &int_y, &int_w, &int_h,
                                        &int_bw, &depth);
                        w = Limit((Dimension) int_w, G_MaxPixmapWidth(new));
                        h = Limit((Dimension) int_h, G_MaxPixmapHeight(new));
                }
                else
                {
                        w = h = 0;
                }

                if (G_Pixmap (new) &&
                    (G_PixmapWidth (new) == w) &&
                    (G_PixmapHeight (new) == h))
                {
                        draw_pixmap = True;
                }
                else
                {
                        redraw_flag = True;
                        G_PixmapWidth (new) = w;
                        G_PixmapHeight (new) = h;
                }
        }

			
/*	Update GCs if foreground, background or mask changed.
*/
	if (G_Foreground (current) != G_Foreground (new) ||
	    G_Background (current) != G_Background (new) ||
	    ((G_Mask (current) != G_Mask(new)) &&
	     (G_Pixmap (current) != G_Pixmap (new))) ||
	    G_ArmColor (current) != G_ArmColor (new))
	{
		if (G_ShadowThickness (new) > 0 &&
		    G_Behavior(new) != XmICON_DRAG &&
		    G_Background (current) != G_Background (new))
			redraw_flag = True;
		else
			draw_string = True;
		G_UpdateGCs (new);
	}

/*	Convert dimensions to pixel units.
*/
	if (G_UnitType (new) != XmPIXELS)
	{
		G_MarginWidth (new) = 
			_XmToHorizontalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_MarginWidth (new));
		G_MarginHeight (new) = 
			_XmToVerticalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_MarginHeight (new));
	}

/*	Convert spacing.
*/
	if (G_UnitType (new) != G_UnitType (current) &&
	    G_UnitType (new) != XmPIXELS)
	{
		G_Spacing (new) = 
			(G_PixmapPosition (new) == XmPIXMAP_LEFT ||
			 G_PixmapPosition (new) == XmPIXMAP_RIGHT)
			? _XmToHorizontalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_Spacing (new))
			: _XmToVerticalPixels ((Widget)new, G_UnitType (new),
					(XtArgVal *)G_Spacing (new));
	}

/*	Process change in string or font list.
*/

	if (G_String (new) != G_String (current) ||
	    G_FontList (new) != G_FontList (current) ||
	    G_Underline (new) != G_Underline (current))
	{
		if (G_FontList (new) != G_FontList (current))
		{
			if (G_FontList (new) == NULL)
				G_FontList (new) = G_FontList (current);
			else
			{
				XmFontListFree (G_FontList (current));
				G_FontList (new) =
					XmFontListCopy (G_FontList (new));
			}
		}
		if (G_String (new))
		{
			if (G_String (new) != G_String (current))
			{
				if (G_String (current))
					_XmStringFree (G_String (current));
				G_String (new) =
					_XmStringCreate ((XmString)G_String (new));
			}
			else
				_XmStringUpdate (G_FontList (new),
						G_String (new));
			_XmStringExtent (G_FontList (new), G_String (new),
					&w, &h);
                        if (G_Underline(new))
                           h++;
		}
		else
			w = h = 0;

		G_StringWidth (new) = w;
		G_StringHeight (new) = h;

		G_Spacing (new) = (Dimension) G_StringHeight (new) / 5;
		if (G_Spacing (new) < SPACING_DEFAULT)
			G_Spacing (new) = SPACING_DEFAULT;

		if ((G_String (new) != NULL) &&
		    (G_StringWidth (new) == G_StringWidth (current)) &&
		    (G_StringHeight (new) == G_StringHeight (current)))
			draw_string = True;
		else
			redraw_flag = True;
	}

/*	Check for other changes requiring redraw.
*/
	if (G_HighlightThickness (new) != G_HighlightThickness (current) ||
	    G_ShadowThickness (new) != G_ShadowThickness (current) ||
	    G_MarginWidth (new) != G_MarginWidth (current) ||
	    G_MarginHeight (new) != G_MarginHeight (current) ||
	    G_Spacing (new) != G_Spacing (current))
	{
		redraw_flag = True;
	}

/*	Update size.
*/
	if (redraw_flag ||
	    (G_RecomputeSize (new) && ! G_RecomputeSize (current)))
	{
		if (G_RecomputeSize (new) &&
		    (G_Width (current) == G_Width (new) ||
		     G_Height (current) == G_Height (new)))
		{
			G_GetSize (new, &w, &h);
			if (G_Width (current) == G_Width (new))
				G_Width (new) = w;
			if (G_Height (current) == G_Height (new))
				G_Height (new) = h;
		}
	}

/*	Set redraw flag if this class doesn't optimize redraw.
*/
	else if (! C_OptimizeRedraw (wc))
	{
		if (draw_shadow || draw_pixmap || draw_string)
			redraw_flag = True;
	}	

/*	Optimize redraw if managed.
*/
	else if (XtIsManaged (new) && XtIsRealized(new))
	{
/*	Get string and pixmap positions if necessary.
*/
		if ((draw_pixmap && G_Pixmap (new)) ||
		    (draw_string && G_String (new)))
		{
			G_GetPositions (new, G_Width (new), G_Height (new),
				h_t, G_ShadowThickness (new),
				(Position *)&p_x, (Position *)&p_y, 
				(Position *)&s_x, (Position *)&s_y);
		}
/*	Copy pixmap, clip if necessary.
*/
		if (draw_pixmap && G_Pixmap (new) &&
		    G_Pixmap (new) != XmUNSPECIFIED_PIXMAP)
		  {
		      w = (p_x + s_t + h_t >= G_Width (new))
			? 0 : Min (G_PixmapWidth (new),
				   G_Width (new) - p_x - s_t - h_t);
		      h = (p_y + s_t + h_t >= G_Height (new))
			? 0 : Min (G_PixmapHeight (new),
				   G_Height (new) - p_y - s_t - h_t);
		      if (w > 0 && h > 0) {
			  XCopyArea 
			    (XtDisplay (new), G_Pixmap (new), XtWindow (new),
			     GetMaskGC(new, G_X(new) + p_x, G_Y(new) + p_y),
			     0, 0,
			     w, h, G_X (new) + p_x, G_Y (new) + p_y);
		      }
		}
/*	Draw string with normal or armed background; clip if necessary.
*/
		if (draw_string && G_String (new))
		{
			GC		gc;
			XRectangle	clip;
			unsigned char	behavior = G_Behavior (new);

			if ((behavior == XmICON_BUTTON ||
			     behavior == XmICON_DRAG) &&
			    G_FillOnArm (new) && G_Armed (new))
				gc = G_ArmedGC (new);
			else if (behavior == XmICON_TOGGLE &&
				 G_FillOnArm (new) &&
				 ((G_Armed (new) && !G_Set (new)) ||
				  (!G_Armed (new) && G_Set (new))))
				gc = G_ArmedGC (new);
			else
				gc = G_NormalGC (new);

			clip.x = G_X (new) + s_x;
			clip.y = G_Y (new) + s_y;
			clip.width = (s_x + s_t + h_t >= G_Width (new))
				? 0 : Min (G_StringWidth (new),
					G_Width (new) - s_x - s_t - h_t);
			clip.height = (s_y + s_t + h_t >= G_Height (new))
				? 0 : Min (G_StringHeight (new),
					G_Height (new) - s_y - s_t - h_t);
			if (clip.width > 0 && clip.height > 0)
                        {
				_XmStringDrawImage (XtDisplay (new),
					 XtWindow (new), G_FontList (new),
					G_String (new), gc,
					G_X (new) + s_x, G_Y (new) + s_y,
					clip.width, XmALIGNMENT_CENTER,
					XmSTRING_DIRECTION_L_TO_R, &clip);

				if (G_Underline(new))
				{
				   _XmStringDrawUnderline (XtDisplay (new),
					    XtWindow (new), G_FontList (new),
					   G_String (new), gc,
					   G_X (new) + s_x, G_Y (new) + s_y,
					   clip.width, XmALIGNMENT_CENTER,
					   XmSTRING_DIRECTION_L_TO_R, &clip,
                                           G_String(new));
				}
                        }
		}
/*	Draw shadow.
*/
		if (draw_shadow && G_DrawShadow(new))
		{
                   if(G_BorderType(new) == DtRECTANGLE || !G_Pixmap(new))
		      _DtManagerDrawShadow (MgrParent (new), XtWindow (new),
                                G_X (new), G_Y (new),
				G_Width (new), G_Height (new), h_t,
				G_ShadowThickness (new), G_ShadowType (new));
                   else
                      G_CallCallback (new, G_Callback (new),
                                                    XmCR_SHADOW, NULL);
		}
	}

        if (G_Operations(current) != G_Operations(new)) {
           if (G_Operations(current) == XmDROP_NOOP){
	      _DtIconRegisterDropsite(new_w);
           } else {
              if (G_Operations(new) == XmDROP_NOOP)
	         DtDndDropUnregister(new_w);
              else {
                 Arg args[1];

	         XtSetArg(args[0], XmNdropSiteOperations, G_Operations(new));
	         XmDropSiteUpdate(new_w, args, 1);
              }
           }
        }

	return (redraw_flag);
}



/*-------------------------------------------------------------
**	Gadget Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	BorderHighlight
**
*/
static void 
#ifdef _NO_PROTO
BorderHighlight( g )
        DtIconGadget g ;
#else
BorderHighlight( DtIconGadget g)
#endif /* _NO_PROTO */
{
   register int width;
   register int height;

   width = g->rectangle.width;
   height = g->rectangle.height;

   if (width == 0 || height == 0) return;

   if (g->gadget.highlight_thickness == 0) return;

   g->gadget.highlighted = True;
   g->gadget.highlight_drawn = True;

   if(G_BorderType(g) == DtRECTANGLE || !G_Pixmap(g))
      HighlightBorder ((Widget)g);

   G_CallCallback (g, G_Callback (g), XmCR_HIGHLIGHT, NULL);
}

/*-------------------------------------------------------------
**	BorderUnhighlight
**
*/
static void 
#ifdef _NO_PROTO
BorderUnhighlight( g )
        DtIconGadget g ;
#else
BorderUnhighlight( DtIconGadget g)
#endif /* _NO_PROTO */
{

   register int window_width;
   register int window_height;
   register highlight_width;

   window_width = g->rectangle.width;
   window_height = g->rectangle.height;

   if (window_width == 0 || window_height == 0) return;

   highlight_width = g->gadget.highlight_thickness;
   if (highlight_width == 0) return;

   g->gadget.highlighted = False;
   g->gadget.highlight_drawn = False;

   if(G_BorderType(g) == DtRECTANGLE || !G_Pixmap(g))
      UnhighlightBorder ((Widget)g);

   G_CallCallback (g, G_Callback (g), XmCR_UNHIGHLIGHT, NULL);
}

/*-------------------------------------------------------------
**	ArmAndActivate
**		Invoke Activate.
*/
static void 
#ifdef _NO_PROTO
ArmAndActivate( w, event )
        Widget w ;
        XEvent *event ;
#else
ArmAndActivate(
        Widget w,
        XEvent *event )
#endif /* _NO_PROTO */
{
	IconArm (w, event);
	IconActivate (w, event);
}



/*-------------------------------------------------------------
**	InputDispatch
**		Process event dispatched from parent or event handler.
*/
static void 
#ifdef _NO_PROTO
InputDispatch( w, event, event_mask )
        Widget w ;
        XButtonEvent *event ;
        Mask event_mask ;
#else
InputDispatch(
        Widget w,
        XButtonEvent *event,
        Mask event_mask )
#endif /* _NO_PROTO */
{
	DtIconGadget	g =	(DtIconGadget) w;

	if (event_mask & XmARM_EVENT || event_mask & XmMULTI_ARM_EVENT)
		if (event->button == Button2)
			IconDrag (w, (XEvent*) event);	
                else if (event->button == Button3)
                        IconPopup (w, (XEvent*) event);
		else
			IconArm (w, (XEvent*) event);
	else if (event_mask & XmACTIVATE_EVENT ||
		 event_mask & XmMULTI_ACTIVATE_EVENT)
	{
                if (event->button == Button3)
                        ;
		else if (event->x >= G_X (g) &&
			 event->x <= G_X (g) + G_Width (g) &&
			 event->y >= G_Y (g) &&
			 event->y <= G_Y (g) + G_Height (g))
			IconActivate (w, (XEvent*) event);
		else
			IconDisarm (w, (XEvent*) event);
	}
	else if (event_mask & XmHELP_EVENT)
		_XmSocorro (w, (XEvent *)event,
                                     (String *)NULL,(Cardinal)0);
	else if (event_mask & XmENTER_EVENT)
		IconEnter (w, (XEvent *)event);
	else if (event_mask & XmLEAVE_EVENT)
		IconLeave (w, (XEvent *)event);
	else if (event_mask & XmFOCUS_IN_EVENT)
		_XmFocusInGadget (w, (XEvent *)event,
                                    (String *)NULL,(Cardinal)0);
	else if (event_mask & XmFOCUS_OUT_EVENT)
		_XmFocusOutGadget (w, (XEvent *)event,
                                    (String *)NULL,(Cardinal)0);
}



/*-------------------------------------------------------------
**	VisualChange
**		Update GCs when parent visuals change.
*/
static Boolean 
#ifdef _NO_PROTO
VisualChange( w, current_w, new_w )
        Widget w ;
        Widget current_w ;
        Widget new_w ;
#else
VisualChange(
        Widget w,
        Widget current_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
    XmManagerWidget		current =	(XmManagerWidget) current_w;
    XmManagerWidget		new = 		(XmManagerWidget) new_w;
    DtIconGadget		g = 		(DtIconGadget) w;
    Boolean			update = 	False;
    
    /*	If foreground or background was the same as the parent, and parent
     **	foreground or background has changed, then update gcs and pixmap.
     */
    /* (can't really tell if explicitly set to be same as parent!
     **  -- could add flags set in dynamic default procs for fg and bg)
     */
    if (G_Foreground (g) == M_Foreground (current) &&
	M_Foreground (current) != M_Foreground (new))
      {
	  _DtAssignIcon_Foreground(g, M_Foreground (new));
	  update = True;
      }
    
    if (G_Background (g) == M_Background (current) &&
	M_Background (current) != M_Background (new))
      {
	  _DtAssignIcon_Background(g, M_Background (new));
	  update = True;
      }
    
    if (G_PixmapForeground (g) == M_Foreground (current) &&
	M_Foreground (current) != M_Foreground (new))
      {
	  G_PixmapForeground(g) =  M_Foreground (new);
	  update = True;
      }
    
    if (G_PixmapBackground (g) == M_Background (current) &&
	M_Background (current) != M_Background (new))
      {
	  G_PixmapBackground(g) = M_Background (new);
	  update = True;
      }
    
    if (update)
      {
	  _DtReCacheIcon(g);
	  G_UpdateGCs (g);
	  
	  if (G_ImageName (g) != NULL)
	    {
		if (G_Mask(g) != XmUNSPECIFIED_PIXMAP)
		  XmDestroyPixmap (XtScreen(g),G_Mask(g));
		XmDestroyPixmap (XtScreen(w),G_Pixmap (g));
		G_Pixmap (g) = 
		  XmGetPixmap (XtScreen (g), G_ImageName (g),
			       G_PixmapForeground (g),
			       G_PixmapBackground (g));
		if (G_Pixmap (g) != XmUNSPECIFIED_PIXMAP)
		  G_Mask(g) = (Pixmap)_DtGetMask(XtScreen(g), G_ImageName(g));
		return (True);
	    }
	  else
	    return (False);
      }
}


/*-------------------------------------------------------------
**	Icon Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	GetSize
**		Compute size.
*/
static void 
#ifdef _NO_PROTO
GetSize( g, w, h )
        DtIconGadget g ;
        Dimension *w ;
        Dimension *h ;
#else
GetSize(
        DtIconGadget g,
        Dimension *w,
        Dimension *h )
#endif /* _NO_PROTO */
{
	Dimension	s_t = G_ShadowThickness (g),
			h_t = G_HighlightThickness (g),
			p_w = G_PixmapWidth (g),
			p_h = G_PixmapHeight (g),
			m_w = G_MarginWidth (g),
			m_h = G_MarginHeight (g),
			s_w = G_StringWidth (g),
			s_h = G_StringHeight (g),
			v_pad = 2 * (s_t + h_t + m_h),
			h_pad = 2 * (s_t + h_t + m_w),
			spacing = G_Spacing (g);
	
	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

/*	Get width and height.
*/
	switch ((int) G_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
		case XmPIXMAP_BOTTOM:
			*w = Max (p_w, s_w) + h_pad;
			*h = p_h + s_h + v_pad + spacing;
			break;
		case XmPIXMAP_LEFT:
		case XmPIXMAP_RIGHT:
			*w = p_w + s_w + h_pad + spacing;
			*h = Max (p_h, s_h) + v_pad;
			break;
		case XmPIXMAP_MIDDLE:
			*w = Max (p_w, s_w) + h_pad;
			*h = Max (p_h, s_h) + v_pad;
			break;
		default:
			break;
	}
}



/*-------------------------------------------------------------
**	GetPositions
**		Get positions of string and pixmap.
*/
static void 
#ifdef _NO_PROTO
GetPositions( g, w, h, h_t, s_t, pix_x, pix_y, str_x, str_y )
        DtIconGadget g ;
        Position w ;
        Position h ;
        Dimension h_t ;
        Dimension s_t ;
        Position *pix_x ;
        Position *pix_y ;
        Position *str_x ;
        Position *str_y ;
#else
GetPositions(
        DtIconGadget g,
        Position w,
        Position h,
        Dimension h_t,
        Dimension s_t,
        Position *pix_x,
        Position *pix_y,
        Position *str_x,
        Position *str_y )
#endif /* _NO_PROTO */
{
	Dimension	p_w =		G_PixmapWidth (g),
			p_h =		G_PixmapHeight (g),
			s_w =		G_StringWidth (g),
			s_h =		G_StringHeight (g),
			m_w =		G_MarginWidth (g),
			m_h =		G_MarginHeight (g),
			spacing =	G_Spacing (g),
			h_pad =		s_t + h_t + m_w,
			v_pad =		s_t + h_t + m_h,
			width =		w - 2 * h_pad,
			height =	h - 2 * v_pad;
	Position	p_x =		h_pad,
			p_y =		v_pad,
			s_x =		h_pad,
			s_y =		v_pad;
	unsigned char	align =		G_Alignment (g);

	if (((p_w == 0) && (p_h == 0)) || ((s_w == 0) && (s_h == 0)))
		spacing = 0;

/*	Set positions
*/
	switch ((int) G_PixmapPosition (g))
	{
		case XmPIXMAP_TOP:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (p_h && height > p_h + s_h + spacing)
				p_y += (height - p_h - s_h - spacing)/2;
			if (p_h)
				s_y = p_y + p_h + spacing;
			else
				s_y += (height - s_h)/2;
			break;
		case XmPIXMAP_BOTTOM:
			if (align == XmALIGNMENT_CENTER)
			{
				if (p_w && width > p_w)
					p_x += (width - p_w)/2;
				if (s_w && width > s_w)
					s_x += (width - s_w)/2;
			}
			else if (align == XmALIGNMENT_END)
			{
				if (p_w && width > p_w)
					p_x += width - p_w;
				if (s_w && width > s_w)
					s_x += width - s_w;
			}
			if (s_h && height > p_h + s_h + spacing)
				s_y += (height - p_h - s_h - spacing)/2;
			if (s_h)
				p_y = s_y + s_h + spacing;
			else
				p_y += (height - s_h)/2;
			break;
		case XmPIXMAP_LEFT:
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			s_x += p_w + spacing;
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			break;
		case XmPIXMAP_RIGHT:
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			p_x += s_w + spacing;
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			break;
		case XmPIXMAP_MIDDLE:
			if (p_w && width > p_w)
		 	   p_x += (width - p_w)/2;
			if (s_w && width > s_w)
			   s_x += (width - s_w)/2;
			if (s_h && height > s_h)
				s_y += (height - s_h)/2;
			if (p_h && height > p_h)
				p_y += (height - p_h)/2;
			break;
		default:
			break;
	}

	*pix_x = p_x;
	*pix_y = p_y;
	*str_x = s_x;
	*str_y = s_y;
}



/*-------------------------------------------------------------
**	Draw
**		Draw gadget to drawable.
*/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
Draw( g, drawable, x, y, w, h, h_t, s_t, s_type, fill_mode )
        DtIconGadget g ;
        Drawable drawable ;
        Position x ;
        Position y ;
        Dimension w ;
        Dimension h ;
        Dimension h_t ;
        Dimension s_t ;
        unsigned char s_type ;
        unsigned char fill_mode ;
#else
Draw(
        DtIconGadget g,
        Drawable drawable,
        Position x,
        Position y,
        Dimension w,
        Dimension h,
        Dimension h_t,
        Dimension s_t,
        unsigned char s_type,
        unsigned char fill_mode )
#endif /* _NO_PROTO */
{
	XmManagerWidget	mgr =	(XmManagerWidget) XtParent (g);
	Display *	d = 	XtDisplay (g);
	GC		gc;
	XRectangle	clip;
	Position	p_x, p_y, s_x, s_y;
	Dimension	width, height;
	unsigned char	behavior =	G_Behavior (g);
        Position        adj_x, adj_y;
	int rec_width=0,begin=0,diff=0;

/*	Fill with icon or manager background or arm color.
*/
        if (G_SavedParentBG(g) != XtParent(g)->core.background_pixel) {
         XtReleaseGC((Widget)mgr, G_ParentBackgroundGC(g));
         GetParentBackgroundGC(g);
        }

	if (fill_mode == XmFILL_SELF)
	{
		if ((behavior == XmICON_BUTTON || behavior == XmICON_DRAG) &&
		     G_FillOnArm (g) && G_Armed (g))
			gc = G_ArmedBackgroundGC (g);
		else if (behavior == XmICON_TOGGLE && G_FillOnArm (g) &&
			 ((G_Armed (g) && !G_Set (g)) ||
			  (!G_Armed (g) && G_Set (g))))
			gc = G_ArmedBackgroundGC (g);
		else
			gc = G_BackgroundGC (g);
	}
	else if (fill_mode == XmFILL_PARENT)
		gc = G_ParentBackgroundGC (g);

	if ((fill_mode != XmFILL_NONE) && (fill_mode != XmFILL_TRANSPARENT))
		XFillRectangle (d, drawable, gc, x + h_t, y + h_t,
				w - 2 * h_t, h - 2 * h_t);

/*	Get pixmap and string positions.
*/
	G_GetPositions (g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

/*	Copy pixmap.
*/
	if (G_Pixmap (g))
	{
		width = (p_x + s_t + h_t >= G_Width (g))
			? 0 : Min (G_PixmapWidth (g),
				G_Width (g) - p_x - s_t - h_t);
		height = (p_y + s_t + h_t >= G_Height (g))
			? 0 : Min (G_PixmapHeight (g),
				G_Height (g) - p_y - s_t - h_t);
		if (width > 0 && height > 0)
		  {
		      if (fill_mode == XmFILL_TRANSPARENT)
                        {
			    adj_x = s_t + h_t + G_MarginWidth(g);
			    adj_y = s_t + h_t + G_MarginHeight(g);
                            switch (G_PixmapPosition(g))
                            {
                               case XmPIXMAP_TOP:
                               case XmPIXMAP_BOTTOM:
			          XFillRectangle(d, drawable, 
                                                 G_ParentBackgroundGC(g),
					         x + p_x - adj_x, 
                                                 y + p_y - adj_y,
					         width + (2 * adj_y),
        				         height + (2 * adj_x) -
                                                                 (s_t + h_t));
                                  break;
                               case XmPIXMAP_LEFT:
                               case XmPIXMAP_RIGHT:
			          XFillRectangle(d, drawable, 
                                                 G_ParentBackgroundGC(g),
					         x + p_x - adj_x, 
                                                 y + p_y - adj_y,
					         width + (2 * adj_y) -
                                                                 (s_t + h_t),
        				         height + (2 * adj_x));
                                  break;
                               case XmPIXMAP_MIDDLE:
			          XFillRectangle(d, drawable, 
                                                 G_ParentBackgroundGC(g),
					         x + p_x - adj_x, 
                                                 y + p_y - adj_y,
					         width + (2 * adj_y),
        				         height + (2 * adj_x));
                                  break;
                            }
                        }
		      XCopyArea (d, G_Pixmap (g), drawable, 
				 GetMaskGC(g, x + p_x, y + p_y),
				 0, 0, width, height, x + p_x, y + p_y);
		  }
	}

/*	Draw string.
*/
	if ((behavior == XmICON_BUTTON || behavior == XmICON_DRAG) &&
	     G_FillOnArm (g) && G_Armed (g))
		gc = G_ArmedGC (g);
	else if (behavior == XmICON_TOGGLE && G_FillOnArm (g) &&
		 ((G_Armed (g) && !G_Set (g)) ||
		  (!G_Armed (g) && G_Set (g))))
		gc = G_ArmedGC (g);
	else
		gc = G_NormalGC (g);

	if (G_String (g))
	{
		clip.x = x + s_x;
		clip.y = y + s_y;
                switch (G_PixmapPosition(g))
                {
                   case XmPIXMAP_TOP:
                   case XmPIXMAP_BOTTOM:
		      clip.width = (s_x + s_t + h_t >= G_Width (g))
			      ? 0 : Min (G_StringWidth (g),
           		            G_Width (g) - s_x);
		      clip.height = (s_y + s_t + h_t >= G_Height (g))
		      	      ? 0 : Min (G_StringHeight (g),
				     G_Height (g) - s_y - s_t - h_t);
                      break;
                   case XmPIXMAP_LEFT:
                   case XmPIXMAP_RIGHT:
		      clip.width = (s_x + s_t + h_t >= G_Width (g))
			      ? 0 : Min (G_StringWidth (g),
           		            G_Width (g) - s_x - s_t - h_t);
		      clip.height = (s_y + s_t + h_t >= G_Height (g))
		      	      ? 0 : Min (G_StringHeight (g),
				     G_Height (g) - s_y);
                      break;
                   case XmPIXMAP_MIDDLE:
		      clip.width = (s_x + s_t + h_t >= G_Width (g))
			      ? 0 : Min (G_StringWidth (g),
           		            G_Width (g) - s_x);
		      clip.height = (s_y + s_t + h_t >= G_Height (g))
		      	      ? 0 : Min (G_StringHeight (g),
				     G_Height (g) - s_y);
                      break;
                 }
		if (clip.width > 0 && clip.height > 0)
                {
                        if (fill_mode == XmFILL_TRANSPARENT)
                        {
                           adj_x = s_t + h_t + G_MarginWidth(g);
                           adj_y = s_t + h_t + G_MarginHeight(g);
                           switch (G_PixmapPosition(g))
                           {
                              case XmPIXMAP_TOP:
                              case XmPIXMAP_BOTTOM:
                                 XFillRectangle(d, drawable, 
                                                G_ParentBackgroundGC(g),
                                                clip.x - adj_x, 
                                                clip.y - adj_y + s_t + h_t,
                                                clip.width + (2 * adj_y),
                                                clip.height + (2 * adj_x) - 
                                                                (s_t + h_t));
                                 break;
                              case XmPIXMAP_RIGHT:
                              case XmPIXMAP_LEFT:
				begin = clip.x - adj_x + s_t + h_t;
                                rec_width = clip.width + (2 * adj_y) 
							     -(s_t + h_t);
				if(G_PixmapPosition(g) == XmPIXMAP_LEFT &&
				    begin > (x+p_x) && begin < (x+p_x+width))
                                {
                                   /*
                                    * XmPIXMAP_LEFT -- the rectangle starts
				    * inside the pixmap
                                    */
                                        diff = x+p_x+width - begin;
                                        begin+=diff;
                                        rec_width-=diff;
                                }
                                else if(G_PixmapPosition(g) == XmPIXMAP_RIGHT &&
				    (rec_width+begin) > (x+p_x))
                                {
                                    /*
                                     * PIXMAP_RIGHT -- rectangle drawn into
                                     * the pixmap
                                     */
                                        diff = ( rec_width + begin) - (x+p_x);
                                        rec_width-=diff;
                                }

                                 XFillRectangle(d, drawable, 
                                                G_ParentBackgroundGC(g),
                                                begin, 
                                                clip.y - adj_y,
                                                rec_width,
                                                clip.height + (2 * adj_x));
                                 break;
                              case XmPIXMAP_MIDDLE:
                                 XFillRectangle(d, drawable, 
                                                G_ParentBackgroundGC(g),
                                                clip.x - adj_x,
                                                clip.y - adj_y,
                                                clip.width + (2 * adj_y),
                                                clip.height + (2 * adj_x));
                                 break;
                            }
                        }

			_XmStringDrawImage (d, drawable, G_FontList (g),
				G_String (g), gc, x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip);

			if (G_Underline(g))
			{
			   _XmStringDrawUnderline (d, drawable, G_FontList (g),
				G_String (g), gc, x + s_x, y + s_y,
				clip.width, XmALIGNMENT_BEGINNING,
				XmSTRING_DIRECTION_L_TO_R, &clip,
                                G_String(g));
			}
                }
	}

        /* Potentially fill the area between the label and the pixmap */
        if ((fill_mode == XmFILL_TRANSPARENT) && G_Pixmap(g) && G_String(g) &&
           (height > 0) && (width > 0) && (clip.width > 0) && (clip.height > 0))
        {
           switch (G_PixmapPosition(g))
           {
              case XmPIXMAP_TOP:
                      XFillRectangle(d, drawable, G_ParentBackgroundGC(g),
                                     x + Max(s_x, p_x), y + p_y + height,
                                     Min(clip.width, width),
                                     s_y - (p_y + height));
                      break;

              case XmPIXMAP_BOTTOM:
                      XFillRectangle(d, drawable, G_ParentBackgroundGC(g),
                                     x + Max(s_x, p_x), y + s_y + clip.height,
                                     Min(clip.width, width),
                                     p_y - (s_y + clip.height));
                      break;

              case XmPIXMAP_RIGHT:
                      XFillRectangle(d, drawable, G_ParentBackgroundGC(g),
                                     x + s_x + clip.width, y + Max(s_y, p_y),
                                     p_x - (s_x + clip.width),
                                     Min(clip.height, height));
                      break;
              case XmPIXMAP_LEFT:
                      XFillRectangle(d, drawable, G_ParentBackgroundGC(g),
                                     x + p_x + width, y + Max(s_y, p_y),
                                     s_x - (p_x + width),
                                     Min(clip.height, height));
                      break;
              case XmPIXMAP_MIDDLE:
                      XFillRectangle(d, drawable, G_ParentBackgroundGC(g),
                                     x + Max(s_x, p_x), y + Max(s_y, p_y),
                                     Min(clip.width, width),
                                     Min(clip.height, height));
                      break;
           }
        }


/*	Draw shadow.
*/
	if (G_ShadowThickness (g) > 0 && G_DrawShadow(g)) 
           if(G_BorderType(g) == DtRECTANGLE || !G_Pixmap(g))
		{
		if (((G_Behavior (g) == XmICON_BUTTON) && G_Armed (g)) ||
		    ((G_Behavior (g) == XmICON_TOGGLE) &&
		      (!G_Set (g) && G_Armed (g)) ||
		      (G_Set (g) && !G_Armed (g))))
			_DtManagerDrawShadow (MgrParent(g), drawable, x, y,
					w, h, h_t, s_t, XmSHADOW_IN);
		else
			_DtManagerDrawShadow (MgrParent(g), drawable, x, y,
					w, h, h_t, s_t, XmSHADOW_OUT);
		}
           else
              G_CallCallback (g, G_Callback (g), XmCR_SHADOW, NULL);
}



/*-------------------------------------------------------------
**	CallCallback
**		Call callback, if any, with reason and event.
*/
static void 
#ifdef _NO_PROTO
CallCallback( g, cb, reason, event )
        DtIconGadget g ;
        XtCallbackList cb ;
        int reason ;
        XEvent *event ;
#else
CallCallback(
        DtIconGadget g,
        XtCallbackList cb,
        int reason,
        XEvent *event )
#endif /* _NO_PROTO */
{
	DtIconCallbackStruct	cb_data;

	if (cb != NULL)
	{
		cb_data.reason = reason;
		cb_data.event = event;
		cb_data.set = G_Set (g);
		XtCallCallbackList ((Widget) g, cb, &cb_data);
	}
}





/*-------------------------------------------------------------
**	UpdateGCs
**		Get normal and background graphics contexts.
**		Use standard mask to maximize caching opportunities.
*/
static void 
#ifdef _NO_PROTO
UpdateGCs( g )
        DtIconGadget g ;
#else
UpdateGCs(
        DtIconGadget g )
#endif /* _NO_PROTO */
{
	XGCValues	values;
	XtGCMask	value_mask;
	XmManagerWidget	mw = (XmManagerWidget) XtParent(g);
	XFontStruct *	font;
	int		index;

	if (G_NormalGC (g))
		XtReleaseGC ((Widget)mw, G_NormalGC (g));
	if (G_ClipGC (g))
		XtReleaseGC ((Widget)mw, G_ClipGC (g));
	if (G_BackgroundGC (g))
		XtReleaseGC ((Widget)mw, G_BackgroundGC (g));
	if (G_ArmedGC (g))
		XtReleaseGC ((Widget)mw, G_ArmedGC (g));
	if (G_ArmedBackgroundGC (g))
		XtReleaseGC ((Widget)mw, G_ArmedBackgroundGC (g));

/*	Get normal GC.
*/
	value_mask = GCForeground | GCBackground | GCFillStyle;
	values.foreground = G_Foreground (g);
	values.background = G_Background (g);
	values.fill_style = FillSolid;
	if (_XmFontListSearch(G_FontList (g), "default",
                              (short *)&index, &font)) {
	   value_mask |= GCFont;
	   values.font = font->fid;
        }
	G_NormalGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get background GC.
*/
	values.foreground = G_Background (g);
	values.background = G_Foreground (g);
	G_BackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get armed GC.
*/
	values.foreground = G_Foreground (g);
	values.background = G_ArmColor (g);
	G_ArmedGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

/*	Get armed background GC.
*/
	values.foreground = G_ArmColor (g);
	values.background = G_Background (g);
	G_ArmedBackgroundGC (g) = XtGetGC ((Widget)mw, value_mask, &values);

	if (G_Mask(g) != XmUNSPECIFIED_PIXMAP) {
	    value_mask |= GCClipMask;
	    values.clip_mask = G_Mask(g);
	    values.foreground = G_Foreground (g);
	    values.background = G_Background (g);
	    G_ClipGC (g) = XtGetGC ((Widget)mw, value_mask, &values);
	}
	else
	  G_ClipGC (g) = NULL;
}


/*-------------------------------------------------------------
**	GetIconClassSecResData ( )
**		Class function to be called to copy secondary resource
**		for external use.  i.e. copy the cached resources and
**		send it back.
**-------------------------------------------------------------
*/
/* ARGSUSED */
static Cardinal 
#ifdef _NO_PROTO
GetIconClassSecResData( class, data_rtn )
        WidgetClass class ;
        XmSecondaryResourceData **data_rtn ;
#else
GetIconClassSecResData(
        WidgetClass class,
        XmSecondaryResourceData **data_rtn )
#endif /* _NO_PROTO */
{   int arrayCount = 0;
    XmBaseClassExt  bcePtr;
    String  resource_class, resource_name;
    XtPointer  client_data;

    bcePtr = &( iconBaseClassExtRec);
    client_data = NULL;
    resource_class = NULL;
    resource_name = NULL;
    arrayCount =
      _XmSecondaryResourceData ( bcePtr, data_rtn, client_data,
                                resource_name, resource_class,
                            (XmResourceBaseProc) (GetIconClassResBase));

    return (arrayCount);
}


/*-------------------------------------------------------------
**	GetIconClassResBase ()
**		retrun the address of the base of resources.
**		- Not yet implemented.
**-------------------------------------------------------------
*/
/* ARGSUSED */
static XtPointer 
#ifdef _NO_PROTO
GetIconClassResBase( widget, client_data )
        Widget widget ;
        XtPointer client_data ;
#else
GetIconClassResBase(
        Widget widget,
        XtPointer client_data )
#endif /* _NO_PROTO */
{   XtPointer  widgetSecdataPtr;
    int  icon_cache_size = sizeof (DtIconCacheObjPart);
    char *cp;

    widgetSecdataPtr = (XtPointer) (XtMalloc ( icon_cache_size +1));

    if (widgetSecdataPtr)
      { cp = (char *) widgetSecdataPtr;

#ifndef       SVR4
        bcopy ( (char *) ( Icon_Cache(widget)), (char *) cp, icon_cache_size);
#else
        memmove ( (char *)cp , (char *) ( Icon_Cache(widget)), icon_cache_size);
#endif

          }

    return (widgetSecdataPtr);
}



/*-------------------------------------------------------------
**	Public Entry Points
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	_DtCreateIcon
**		Create a new gadget instance.
**-------------------------------------------------------------
*/
Widget 
#ifdef _NO_PROTO
_DtCreateIcon( parent, name, arglist, argcount )
        Widget parent ;
        String name ;
        ArgList arglist ;
        Cardinal argcount ;
#else
_DtCreateIcon(
        Widget parent,
        String name,
        ArgList arglist,
        Cardinal argcount )
#endif /* _NO_PROTO */
{
	return (XtCreateWidget (name, dtIconGadgetClass, 
			parent, arglist, argcount));
}



/*-------------------------------------------------------------
**	_DtIconGetState
**		Return state of Icon.
**-------------------------------------------------------------
*/
Boolean 
#ifdef _NO_PROTO
_DtIconGetState( w )
        Widget w ;
#else
_DtIconGetState(
        Widget w )
#endif /* _NO_PROTO */
{
	DtIconGadget g =	(DtIconGadget) w;

	return (G_Set (g));
}


/*-------------------------------------------------------------
**	_DtIconSetState
**		Set state of Icon.
**-------------------------------------------------------------
*/
void 
#ifdef _NO_PROTO
_DtIconSetState( w, state, notify )
        Widget w ;
        Boolean state ;
        Boolean notify ;
#else
_DtIconSetState(
        Widget w,
        Boolean state,
        Boolean notify )
#endif /* _NO_PROTO */
{
	DtIconGadget g =	(DtIconGadget) w;

	if (G_Behavior (g) != XmICON_TOGGLE || state == G_Set (g))
		return;

	G_Set (g) = state;
	G_Expose ((Widget)g, NULL, NULL);

	if (notify)
		G_CallCallback (g, G_Callback (g), XmCR_VALUE_CHANGED, NULL);
}



/*-------------------------------------------------------------
**	_DtIconDraw
**		Render gadget to drawable without highlight.
**-------------------------------------------------------------
*/
Drawable 
#ifdef _NO_PROTO
_DtIconDraw( widget, drawable, x, y, fill )
        Widget widget ;
        Drawable drawable ;
        Position x ;
        Position y ;
        Boolean fill ;
#else
_DtIconDraw(
        Widget widget,
        Drawable drawable,
        Position x,
        Position y,
        Boolean fill )
#endif /* _NO_PROTO */
{
	DtIconGadget	g =		(DtIconGadget) widget;
	Dimension	h_t =		G_HighlightThickness (g),
			w =		G_Width (g) - 2 * h_t,
			h =		G_Height (g) - 2 * h_t;
	unsigned char	fill_mode;

	if (!drawable || drawable == XmUNSPECIFIED_PIXMAP)
		drawable = (Drawable)
			XCreatePixmap (XtDisplay (g),
				RootWindowOfScreen (XtScreen (g)),
				w, h, DefaultDepthOfScreen (XtScreen (g)));

	fill_mode = (fill) ? XmFILL_SELF : XmFILL_PARENT;
	
	G_Draw (g, drawable, x, y, w, h,
		0, G_ShadowThickness (g), G_ShadowType (g), fill_mode);

	return (drawable);
}



/***************************************************************************/


/* Pixmap cache information */
typedef struct {
   Display * d;
   Pixmap id;
   Dimension h;
   Dimension w;
} PmCache, *PmCachePtr;

static int numCache = 0;
static int sizeCache = 0;
static PmCachePtr cache = NULL;


/*
 * Load the specified pixmap; use our cached size, if possible, to
 * save a server round trip.
 */

static Boolean 
#ifdef _NO_PROTO
LoadPixmap( new, pixmap )
        DtIconGadget new ;
        String pixmap ;
#else
LoadPixmap(
        DtIconGadget new,
        String pixmap )
#endif /* _NO_PROTO */

{
   Display * display = XtDisplay(new);
   Window root;
   unsigned int int_h, int_w, int_bw, depth;
   int x, y;
   int i;
   Pixmap pm = XmGetPixmap(XtScreen(new), pixmap, G_PixmapForeground(new),
                           G_PixmapBackground(new));
   Pixmap mask;

   if (pm != XmUNSPECIFIED_PIXMAP)
     mask = _DtGetMask(XtScreen(new), pixmap);

   if (pm == XmUNSPECIFIED_PIXMAP)
      return(True);

   G_Pixmap(new) = pm;
   G_Mask(new) = mask;
   G_ImageName(new) = XtNewString(pixmap);

   /* Have we cached this pixmap already? */
   for (i = 0; i < numCache; i++)
   {
      if ((cache[i].id == pm) && (cache[i].d == display))
      {
         /* Found a match */
         G_PixmapWidth(new) = Limit(cache[i].w, G_MaxPixmapWidth(new));
         G_PixmapHeight(new) = Limit(cache[i].h, G_MaxPixmapWidth(new));
         return(False);
      }
   }

   /* Not in the cache; add it */
   if (numCache >= sizeCache)
   {
      sizeCache += 10;
      cache = (PmCachePtr)XtRealloc((char*)cache, sizeof(PmCache) * sizeCache);
   }
   XGetGeometry(XtDisplay(new), pm, &root, &x, &y, &int_w, &int_h,
                &int_bw, &depth);
   cache[numCache].d = display;
   cache[numCache].id = pm;
   cache[numCache].h = int_h;
   cache[numCache].w = int_w;
   numCache++;
   G_PixmapWidth(new) = Limit((Dimension)int_w, G_MaxPixmapWidth(new));
   G_PixmapHeight(new) = Limit((Dimension)int_h, G_MaxPixmapHeight(new));
   return(False);
}



Widget 
#ifdef _NO_PROTO
_DtDuplicateIcon( parent, widget, string, pixmap, user_data, underline )
        Widget parent ;
        Widget widget ;
        XmString string ;
        String pixmap ;
        XtPointer user_data ;
        Boolean underline;
#else
_DtDuplicateIcon(
        Widget parent,
        Widget widget,
        XmString string,
        String pixmap,
        XtPointer user_data,
        Boolean underline )
#endif /* _NO_PROTO */
{
   DtIconGadget gadget;
   int size;
   DtIconGadget new;
   Dimension h, w;

   /* Create the new instance structure */
   gadget = (DtIconGadget) widget;
   size = XtClass(gadget)->core_class.widget_size;
   new = (DtIconGadget)XtMalloc(size);

   /* Copy the master into the duplicate */

#ifndef       SVR4
   bcopy((char *)gadget, (char *)new, (int)size);
#else
   memmove((char *)new, (char *)gadget, (int)size);
#endif

   Icon_Cache(new) = (DtIconCacheObjPart *)
                       _XmCachePart(Icon_ClassCachePart(new),
                                    Icon_Cache(new),
                                    sizeof(DtIconCacheObjPart));

   /* Certain fields need to be updated */
   new->object.parent = parent;
   new->object.self = (Widget)new;
   G_FontList(new) = XmFontListCopy(G_FontList(gadget));

   /* Certain fields should not be inherited by the clone */
   new->object.destroy_callbacks = NULL;
   new->object.constraints = NULL;
   new->gadget.help_callback = NULL;
   new->icon.drop_callback = NULL;
   new->rectangle.managed = False;
   G_Callback(new) = NULL;

   /* Set the user_data field */
   new->gadget.user_data = user_data;

   /* Process the optional pixmap name */
   if ((pixmap == NULL) || LoadPixmap(new, pixmap))
   {
      /* No pixmap to load */
      G_ImageName(new) = NULL;
      G_Pixmap(new) = NULL;
      G_PixmapWidth(new) = 0;
      G_PixmapHeight(new) = 0;
   }

   /* Process the required label string */
   G_String(new) = _XmStringCreate(string);
   _XmStringExtent(G_FontList(new), G_String(new), &w, &h);
   G_Underline(new) = underline;
   if (G_Underline(new))
      h++;
   G_StringWidth(new) = w;
   _DtAssignIcon_StringHeight(new, h);

   _DtReCacheIcon(new);

   /* Get copies of the GC's */
   G_NormalGC(new) = NULL;
   G_BackgroundGC(new) = NULL;
   G_ArmedGC(new) = NULL;
   G_ArmedBackgroundGC(new) = NULL;
   G_ClipGC(new) = NULL;
   UpdateGCs(new);

   /* Size the gadget */
   G_GetSize(new, &w, &h);
   G_Width(new) = w;
   G_Height(new) = h;

   /* Insert the duplicate into the parent's child list */
   (*(((CompositeWidgetClass)XtClass(XtParent(new)))->composite_class.
       insert_child))((Widget)new);

   return ((Widget) new);
}


Boolean 
#ifdef _NO_PROTO
_DtIconSelectInTitle( widget, pt_x, pt_y )
        Widget widget ;
        Position pt_x ;
        Position pt_y ;
#else
_DtIconSelectInTitle(
        Widget widget,
        Position pt_x,
        Position pt_y )
#endif /* _NO_PROTO */

{
   DtIconGadget	g = 	(DtIconGadget) widget;
   Position	x, y;
   Dimension	w, h, h_t, s_t;
   XRectangle	clip;
   Position	p_x, p_y, s_x, s_y;

   h_t = 0;
   s_t = G_ShadowThickness(g);
   x = G_X(g);
   y = G_Y(g);
   w = G_Width (g); 
   h = G_Height (g);
   G_GetPositions (g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

   if (G_String (g))
   {
      clip.x = x + s_x;
      clip.y = y + s_y;
      clip.width = (s_x + s_t + h_t >= G_Width (g))
         ? 0 : Min (G_StringWidth (g), G_Width (g) - s_x - s_t - h_t);
      clip.height = (s_y + s_t + h_t >= G_Height (g))
         ? 0 : Min (G_StringHeight (g), G_Height (g) - s_y - s_t - h_t);
      if (clip.width <= 0 || clip.height <= 0)
         return(False);
      else
      {
         if ((pt_x >= clip.x) && (pt_y >= clip.y) &&
             (pt_x <= clip.x + clip.width) && (pt_y <= clip.y + clip.height))
            return(True);
         else
            return(False);
      }
   }
   else
      return(False);
}


/*
 * Returns a pointer to a static storage area; must not be freed.
 */

XRectangle * 
#ifdef _NO_PROTO
_DtIconGetTextExtent( widget )
        Widget widget ;
#else
_DtIconGetTextExtent(
        Widget widget )
#endif /* _NO_PROTO */

{
   DtIconGadget	g = 	(DtIconGadget) widget;
   Position	x, y;
   Dimension	w, h, h_t, s_t;
   static XRectangle	clip;
   Position	p_x, p_y, s_x, s_y;

   h_t = 0;
   s_t = G_ShadowThickness(g);
   x = G_X(g);
   y = G_Y(g);
   w = G_Width (g); 
   h = G_Height (g);
   G_GetPositions (g, w, h, h_t, s_t, &p_x, &p_y, &s_x, &s_y);

   if (G_String (g))
   {
      clip.x = x + s_x;
      clip.y = y + s_y;
      clip.width = (s_x + s_t + h_t >= G_Width (g))
         ? 0 : Min (G_StringWidth (g), G_Width (g) - s_x - s_t - h_t);
      clip.height = (s_y + s_t + h_t >= G_Height (g))
         ? 0 : Min (G_StringHeight (g), G_Height (g) - s_y - s_t - h_t);

      if (clip.width <= 0)
         clip.width = 0;

      if (clip.height <= 0)
         clip.height = 0;
   }
   else
   {
      clip.x = 0;
      clip.y = 0;
      clip.height = 0;
      clip.width = 0;
   }

   return(&clip);
}

/*-------------------------------------------------------------
**	_DtIconGetIconRects
**		Returns rects occupied by label and pixmap
*/
void 
#ifdef _NO_PROTO
_DtIconGetIconRects( g, flags, rect1, rect2 )
        DtIconGadget g ;
        unsigned char *flags ;
        XRectangle *rect1 ;
        XRectangle *rect2 ;
#else
_DtIconGetIconRects(
        DtIconGadget g,
        unsigned char *flags,
        XRectangle *rect1,
        XRectangle *rect2 )
#endif /* _NO_PROTO */

{
   Position	p_x, p_y, s_x, s_y;
   Dimension	width, height;
   Position     adj_x, adj_y;
   Dimension    h_t, s_t;
   
   h_t = G_HighlightThickness(g);
   s_t = G_ShadowThickness(g);

   adj_x = G_MarginWidth(g);
   adj_y = G_MarginHeight(g);
   G_GetPositions(g, G_Width(g), G_Height(g), h_t, s_t, &p_x, &p_y, &s_x, &s_y);
   *flags = 0;

   if (G_Pixmap (g))
   {
      width = (p_x + s_t + h_t >= G_Width (g)) ? 0 : 
                 Min (G_PixmapWidth (g), G_Width (g) - p_x - s_t - h_t);
      height = (p_y + s_t + h_t >= G_Height (g)) ? 0 : 
                 Min (G_PixmapHeight (g), G_Height (g) - p_y - s_t - h_t);
      if (width > 0 && height > 0)
      {
         rect1->x = G_X(g) + p_x - adj_x; 
         rect1->y = G_Y(g) + p_y - adj_y;
         rect1->width = width + (2 * adj_y);
         rect1->height = height + (2 * adj_x);
         *flags |= XmPIXMAP_RECT;
      }
   }

   if (G_String(g))
   {
      width = (s_x + s_t + h_t >= G_Width (g)) ? 0 : 
                    Min (G_StringWidth (g), G_Width (g) - s_x - s_t - h_t);
      height = (s_y + s_t + h_t >= G_Height (g)) ? 0 : 
                    Min (G_StringHeight (g), G_Height (g) - s_y - s_t - h_t);
      if (width > 0 && height > 0)
      {
         rect2->x = G_X(g) + s_x - adj_x;
         rect2->y = G_Y(g) + s_y - adj_y;
         rect2->width = width + (2 * adj_y);
         rect2->height = height + (2 * adj_x);
         *flags |= XmLABEL_RECT;
      }
   }
}

/* ARGSUSED */
/* Do animation when everything is completed.
 * Note: DropDestroy callback is the only notification after the melt has
 * been completed.
 */
static void
#ifdef _NO_PROTO
AnimateCallback(w, clientData, callData)
    Widget      w ;
    XtPointer   clientData ;
    XtPointer   callData ;
#else
AnimateCallback(
    Widget      w,
    XtPointer   clientData,
    XtPointer   callData )
#endif /* NO_PROTO */
{
  DtDndDropAnimateCallback call_data = (DtDndDropAnimateCallback) callData;
  DtIconGadget g = (DtIconGadget) w;

  if (G_DropCallback(g)) {
     XtCallCallbackList(w, G_DropCallback(g), callData);
  }
}

/* ARGSUSED */
static void
#ifdef _NO_PROTO
TransferCallback( w, clientData, callData)
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
TransferCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */
{
  DtDndTransferCallback call_data = (DtDndTransferCallback) callData;
  DtIconGadget g = (DtIconGadget) w;

  call_data->x += G_X(g);
  call_data->y += G_Y(g);

  if (G_DropCallback(g)) {
     XtCallCallbackList(w, G_DropCallback(g), callData);
  }
}

/*-------------------------------------------------------------
**	_DtIconRegisterDropsite
**		Registers the Icon as a dropsite.
*/
void 
#ifdef _NO_PROTO
_DtIconRegisterDropsite( w )
        Widget w;
#else
_DtIconRegisterDropsite(
        Widget w)
#endif /* _NO_PROTO */

{
    static XtCallbackRec transferCB[] = { {TransferCallback, NULL},
                                               {NULL, NULL} };
    static XtCallbackRec animateCB[] = { {AnimateCallback, NULL},
                                               {NULL, NULL} };
    DtIconGadget g =	(DtIconGadget) w;
    XRectangle rects[2];
    unsigned char flags;
    int numRects = 0;
    Arg args[5];
    Cardinal n;

    _DtIconGetIconRects(g, &flags, &rects[0], &rects[1]);

    if (flags & XmPIXMAP_RECT) {
	rects[0].x -= G_X(g);
	rects[0].y -= G_Y(g);
	numRects++;
    }

    if (flags & XmLABEL_RECT) {
	rects[1].x -= G_X(g);
	rects[1].y -= G_Y(g);
       if (!numRects) rects[0] = rects[1];
       numRects++;
    }

    n = 0;
    if (numRects) {
	XtSetArg(args[n], XmNdropRectangles, rects); n++;
    	XtSetArg(args[n], XmNnumDropRectangles, numRects); n++;
    }
    XtSetArg(args[n], XmNanimationStyle, XmDRAG_UNDER_SHADOW_IN); n++;
    XtSetArg(args[n], DtNtextIsBuffer, True); n++;
    XtSetArg(args[n], DtNdropAnimateCallback, animateCB); n++;

    DtDndDropRegister(w, DtDND_FILENAME_TRANSFER|DtDND_BUFFER_TRANSFER,
	G_Operations(g), transferCB,
	args, n);
}
