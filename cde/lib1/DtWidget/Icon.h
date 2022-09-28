/* $XConsortium: Icon.h /main/cde1_maint/1 1995/07/17 18:30:04 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		Icon.h
***
***	project:	MotifPlus Widgets
***
***	description:	Public include file for DtIcon gadget class.
***	
***	
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtIcon_h
#define _DtIcon_h

#include <Xm/Xm.h>
#include <Dt/DtStrDefs.h>

#ifndef DtIsIcon
#define DtIsIcon(w) XtIsSubclass(w, dtIconGadgetClass)
#endif /* DtIsIcon */

typedef struct _DtIconClassRec * DtIconGadgetClass;
typedef struct _DtIconRec      * DtIconGadget;
typedef struct _DtIconCacheObjRec   * DtIconCacheObject;

#ifdef _NO_PROTO

extern Widget _DtCreateIcon() ;
extern Boolean _DtIconGetState() ;
extern void _DtIconSetState() ;
extern Drawable _DtIconDraw() ;
extern Widget _DtDuplicateIcon() ;
extern Boolean _DtIconSelectInTitle() ;
extern XRectangle * _DtIconGetTextExtent() ;
extern void _DtIconGetIconRects() ;

#else

extern Widget _DtCreateIcon( 
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern Boolean _DtIconGetState( 
                        Widget w) ;
extern void _DtIconSetState( 
                        Widget w,
                        Boolean state,
                        Boolean notify) ;
extern Drawable _DtIconDraw( 
                        Widget widget,
                        Drawable drawable,
                        Position x,
                        Position y,
                        Boolean fill) ;
extern Widget _DtDuplicateIcon( 
                        Widget parent,
                        Widget widget,
                        XmString string,
                        String pixmap,
                        XtPointer user_data,
                        Boolean underline) ;
extern Boolean _DtIconSelectInTitle( 
                        Widget widget,
                        Position pt_x,
                        Position pt_y) ;
extern XRectangle * _DtIconGetTextExtent( 
                        Widget widget) ;
extern void _DtIconGetIconRects( 
                        DtIconGadget g,
                        unsigned char *flags,
                        XRectangle *rect1,
                        XRectangle *rect2) ;

#endif /* _NO_PROTO */


extern WidgetClass dtIconGadgetClass;


#define XmCR_SELECT	XmCR_SINGLE_SELECT
#define XmCR_DROP	50
#define XmCR_POPUP	51
#define XmCR_HIGHLIGHT	52
#define XmCR_UNHIGHLIGHT	53
#define XmCR_SHADOW	54

typedef struct
{
	int		reason;
	XEvent *	event;
	Boolean		set;
} DtIconCallbackStruct;


#define XmPIXMAP_TOP	0
#define XmPIXMAP_BOTTOM	1
#define XmPIXMAP_LEFT	2
#define XmPIXMAP_RIGHT	3
#define XmPIXMAP_MIDDLE	4

#define XmSTRING_BOTTOM	0
#define XmSTRING_TOP	1
#define XmSTRING_RIGHT	2
#define XmSTRING_LEFT	3

#define XmICON_LABEL	0
#define XmICON_BUTTON	1
#define XmICON_TOGGLE	2
#define XmICON_DRAG	3

#define XmFILL_NONE	0
#define XmFILL_PARENT	1
#define XmFILL_SELF	2
#define XmFILL_TRANSPARENT	3

#define XmPIXMAP_RECT  0x01
#define XmLABEL_RECT   0x02

#define DtRECTANGLE     0
#define DtNON_RECTANGLE 1


#endif /* _XmIcon_h */

/* DON'T ADD ANYTHING AFTER THIS #endif */
