/* $XConsortium: IconG.h /main/cde1_maint/1 1995/07/17 20:26:40 drk $ */
/*
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif top of 2.0 tree as of Tue Jun  1 05:18:29 EDT 1993
 */
#ifndef _XmIconG_h
#define _XmIconG_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 * IconGadget defines                                                   *
 ************************************************************************/
        /* XmRViewType */
enum {  XmLARGE_ICON,
        XmSMALL_ICON
        };
        /* XmRVisualEmphasis */
enum {  XmSELECTED,
        XmNOT_SELECTED
        };


/* Class record constants */
extern	WidgetClass	xmIconGadgetClass;

typedef struct _XmIconGadgetClassRec * XmIconGadgetClass;
typedef struct _XmIconGadgetRec      * XmIconGadget;

#ifndef XmIsIconGadget
#define XmIsIconGadget(w) XtIsSubclass(w, xmIconGadgetClass)
#endif /* XmIsIconGadget */

#define XmNlargeIcon            "largeIcon"
#define XmNlargeIconMask        "largeIconMask"
#define XmNlargeIconPixmap      "largeIconPixmap"
#define XmNsmallIcon            "smallIcon"
#define XmNsmallIconMask        "smallIconMask"
#define XmNsmallIconPixmap      "smallIconPixmap"
#define XmNviewType             "viewType"
#define XmNvisualEmphasis       "VisualEmphasis"

#define XmCIcon                 "Icon"
#define XmCViewType             "ViewType"
#define XmCVisualEmphasis       "VisualEmphasis"

#define XmRViewType             "ViewType"
#define XmRVisualEmphasis       "VisualEmphasis"

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern Widget XmCreateIconGadget() ;

#else
extern Widget XmCreateIconGadget(
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIconG_h */

/* DON'T ADD ANYTHING AFTER THIS #endif */

