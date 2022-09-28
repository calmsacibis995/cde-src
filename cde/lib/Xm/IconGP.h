/* $XConsortium: IconGP.h /main/cde1_maint/2 1995/08/18 19:07:27 drk $ */
/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/* (c) Copyright 1990, 1991, 1992, 1993 HEWLETT-PACKARD COMPANY */

#ifndef _XmIconGP_h
#define _XmIconGP_h

#include <Xm/IconG.h>
#include <Xm/XmP.h>
#include <Xm/ExtObjectP.h>
#include <Xm/CacheP.h>


/*-------------------------------------------------------------
**	Cache Class Structure
*/

/*	Cache Class Part
*/
typedef struct _XmIconGCacheObjClassPart
{
    int foo;
} XmIconGCacheObjClassPart;

/*	Cache Full Class Record
*/
typedef struct _XmIconGCacheObjClassRec     /* label cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmIconGCacheObjClassPart            icon_class_cache;
} XmIconGCacheObjClassRec;

/*	Cache Actual Class
*/
externalref XmIconGCacheObjClassRec xmIconGCacheObjClassRec;


/*-------------------------------------------------------------
**	Cache Instance Structure
*/

/*	Cache Instance Part
*/
typedef struct _XmIconGCacheObjPart
{
	Dimension	margin_width;
	Dimension	margin_height;
	Dimension	string_height;
	Dimension	spacing;
	Pixel		foreground;
	Pixel		background;
	Pixel		arm_color;
	Boolean		fill_on_arm;
	Boolean		recompute_size;
	unsigned char	pixmap_position;
	unsigned char	string_position;
	unsigned char	alignment;
	unsigned char	behavior;
	unsigned char	fill_mode;
} XmIconGCacheObjPart;

typedef struct _XmIconGCacheObjRec
{
    ObjectPart               object;
    XmExtPart                ext;
    XmIconGCacheObjPart      icon_cache;
} XmIconGCacheObjRec;


typedef void (*XmGetPositionProc)(
#ifndef _NO_PROTO
	Widget,
#if NeedWidePrototypes
	int,
	int,
	int,
	int,
#else /* NeedWidePrototypes */
	Position,
	Position,
	Dimension,
	Dimension,
#endif /* NeedWidePrototypes */
	Position *,
	Position *,
	Position *,
	Position *
#endif
);
typedef void (*XmGetSizeProc)(
#ifndef _NO_PROTO
	Widget,
	Dimension *,
	Dimension *
#endif
);
typedef void (*XmDrawProc)(
#ifndef _NO_PROTO
	Widget,
	Drawable,
#if NeedWidePrototypes
	int,
	int,
	int,
	int,
	int,
	int,
	unsigned int,
	unsigned int
#else /* NeedWidePrototypes */
	Position,
	Position,
	Dimension,
	Dimension,
	Dimension,
	Dimension,
	unsigned char,
	unsigned char
#endif /* NeedWidePrototypes */
#endif
);
typedef void (*XmCallCallbackProc)(
#ifndef _NO_PROTO
	Widget,
	XtCallbackList,
	int,
	XEvent *
#endif
);
typedef void (*XmUpdateGCsProc)(
#ifndef _NO_PROTO
	Widget
#endif
);
/*-------------------------------------------------------------
**	Class Structure
*/

/*	Class Part
*/
typedef struct _XmIconGadgetClassPart
{
	XmGetSizeProc		get_size;
	XmGetPositionProc		get_positions;
	XmDrawProc		draw;
	XmCallCallbackProc	call_callback;
	XmUpdateGCsProc		update_gcs;
	Boolean			optimize_redraw;
	XmCacheClassPartPtr	cache_part;
	XtPointer			extension;
} XmIconGadgetClassPart;

/*	Full Class Record
*/
typedef struct _XmIconGadgetClassRec
{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	XmIconGadgetClassPart	icon_class;
} XmIconGadgetClassRec;

/*	Actual Class
*/
externalref XmIconGadgetClassRec xmIconGadgetClassRec;



/*-------------------------------------------------------------
**	Instance Structure
*/

/*	Instance Part
*/
typedef struct _XmIconGadgetPart
{
	Boolean		set;
	Boolean		armed;
	Boolean		sync;
	Boolean		underline;
	unsigned char	shadow_type;
	unsigned char	border_type;
	XtCallbackList	callback;
	XtIntervalId	click_timer_id;
	XButtonEvent *	click_event;
	String		image_name;
	Pixmap		pixmap;
	Pixmap		mask;
	Pixel		pixmap_foreground;
	Pixel		pixmap_background;
	XmFontList	font_list;
	_XmString	string;
	Dimension	string_width;
	Dimension	pixmap_width;
	Dimension	pixmap_height;
	GC		clip_gc;
	GC		normal_gc;
	GC		background_gc;
	GC		armed_gc;
	GC		armed_background_gc;
	GC		parent_background_gc;
        Pixel		saved_parent_background;
	XmIconGCacheObjPart *cache;
} XmIconGadgetPart;

/*	Full Instance Record
*/
typedef struct _XmIconGadgetRec
{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	XmIconGadgetPart icon;
} XmIconGadgetRec;


/*-------------------------------------------------------------
**	Class and Instance Macros
*/


/*	XmIconGadget Macros
*/
#define IG_GetSize(g,w,h) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.get_size) \
	(g,w,h)
#define IG_GetPositions(g,w,h,h_t,s_t,p_x,p_y,s_x,s_y) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.get_positions) \
	(g,w,h,h_t,s_t,p_x,p_y,s_x,s_y)
#define IG_Draw(g,d,x,y,w,h,h_t,s_t,s_type,fill) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.draw) \
	(g,d,x,y,w,h,h_t,s_t,s_type,fill)
#define IG_CallCallback(g,cb,r,e) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.call_callback) \
	(g,cb,r,e)
#define IG_UpdateGCs(g) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.update_gcs) \
	(g)

/*	Cached Instance Field Macros
*/
#define IG_FillOnArm(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> fill_on_arm)
#define IG_RecomputeSize(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> recompute_size)
#define IG_PixmapPosition(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> pixmap_position)
#define IG_StringPosition(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> string_position)
#define IG_Alignment(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> alignment)
#define IG_Behavior(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> behavior)
#define IG_FillMode(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> fill_mode)
#define IG_MarginWidth(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> margin_width)
#define IG_MarginHeight(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> margin_height)
#define IG_StringHeight(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> string_height)
#define IG_Spacing(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> spacing)
#define IG_Foreground(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> foreground)
#define IG_Background(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> background)
#define IG_ArmColor(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> arm_color)

/*	Non-Cached Instance Field Macros
*/
#define IG_Armed(g)		(g -> icon.armed)
#define IG_Set(g)		(g -> icon.set)
#define IG_Sync(g)		(g -> icon.sync)
#define IG_Callback(g)		(g -> icon.callback)
#define IG_ClickTimerID(g)	(g -> icon.click_timer_id)
#define IG_ClickInterval(g)	(g -> icon.click_interval)
#define IG_ClickEvent(g)		(g -> icon.click_event)
#define IG_ShadowType(g)		(g -> icon.shadow_type)
#define IG_BorderType(g)		(g -> icon.border_type)
#define IG_Pixmap(g)		(g -> icon.pixmap)
#define IG_Mask(g)		(g -> icon.mask)
#define IG_PixmapForeground(g)	(g -> icon.pixmap_foreground)
#define IG_PixmapBackground(g)	(g -> icon.pixmap_background)
#define IG_String(g)		(g -> icon.string)
#define IG_FontList(g)		(g -> icon.font_list)
#define IG_ImageName(g)		(g -> icon.image_name)
#define IG_StringWidth(g)	(g -> icon.string_width)
#define IG_PixmapWidth(g)	(g -> icon.pixmap_width)
#define IG_PixmapHeight(g)	(g -> icon.pixmap_height)
#define IG_BackgroundGC(g)	(g -> icon.background_gc)
#define IG_ArmedGC(g)		(g -> icon.armed_gc)
#define IG_ArmedBackgroundGC(g)	(g -> icon.armed_background_gc)
#define IG_NormalGC(g)		(g -> icon.normal_gc)
#define IG_ClipGC(g)		(g -> icon.clip_gc)
#define IG_Underline(g)		(g -> icon.underline)
#define IG_ParentBackgroundGC(g) (g -> icon.parent_background_gc)
#define IG_SavedParentBG(g)      (g -> icon.saved_parent_background)


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern int _XmIconGCacheCompare() ;
extern void _XmReCacheIconG() ;
extern void _XmAssignIconG_StringHeight() ;
extern void _XmAssignIconG_Foreground() ;
extern void _XmAssignIconG_Background() ;
extern Boolean _XmIconGadgetGetState() ;
extern void _XmIconGadgetSetState() ;
extern Drawable _XmIconGadgetDraw() ;
extern Widget _XmDuplicateIconG() ;
extern Boolean _XmIconGSelectInTitle() ;
extern XRectangle * _XmIconGGetTextExtent() ;
extern void _XmIconGGetIconRects() ;

#else

extern int _XmIconGCacheCompare( 
                        XtPointer ii,
                        XtPointer ici) ;
extern void _XmReCacheIconG( 
                        XmIconGadget g) ;
extern void _XmAssignIconG_StringHeight( 
                        XmIconGadget g,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignIconG_Foreground( 
                        XmIconGadget g,
                        Pixel value) ;
extern void _XmAssignIconG_Background( 
                        XmIconGadget g,
                        Pixel value) ;
extern Boolean _XmIconGadgetGetState( 
                        Widget w) ;
extern void _XmIconGadgetSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int state,
                        int notify) ;
#else
                        Boolean state,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern Drawable _XmIconGadgetDraw( 
                        Widget widget,
                        Drawable drawable,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int fill) ;
#else
                        Position x,
                        Position y,
                        Boolean fill) ;
#endif /* NeedWidePrototypes */
extern Widget _XmDuplicateIconG( 
                        Widget parent,
                        Widget widget,
                        XmString string,
                        String pixmap,
                        XtPointer user_data,
#if NeedWidePrototypes
                        int underline) ;
#else
                        Boolean underline) ;
#endif /* NeedWidePrototypes */
extern Boolean _XmIconGSelectInTitle( 
                        Widget widget,
#if NeedWidePrototypes
                        int pt_x,
                        int pt_y) ;
#else
                        Position pt_x,
                        Position pt_y) ;
#endif /* NeedWidePrototypes */
extern XRectangle * _XmIconGGetTextExtent( 
                        Widget widget) ;
extern void _XmIconGGetIconRects( 
                        Widget gw,
                        unsigned char *flags,
                        XRectangle *rect1,
                        XRectangle *rect2) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#endif /* _XmIconP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
