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
/*   $XConsortium: ScrollBar.h /main/cde1_maint/2 1995/08/18 19:20:49 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrollBar_h
#define _XmScrollBar_h


#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


/*  ScrollBar Widget  */

externalref WidgetClass xmScrollBarWidgetClass;

typedef struct _XmScrollBarClassRec * XmScrollBarWidgetClass;
typedef struct _XmScrollBarRec      * XmScrollBarWidget;

/* ifndef for Fast Subclassing  */

#ifndef XmIsScrollBar
#define XmIsScrollBar(w)	XtIsSubclass(w, xmScrollBarWidgetClass)
#endif  /* XmIsScrollBar */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateScrollBar() ;
extern void XmScrollBarGetValues() ;
extern void XmScrollBarSetValues() ;

#else

extern Widget XmCreateScrollBar( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern void XmScrollBarGetValues( 
                        Widget w,
                        int *value,
                        int *slider_size,
                        int *increment,
                        int *page_increment) ;
extern void XmScrollBarSetValues( 
                        Widget w,
                        int value,
                        int slider_size,
                        int increment,
                        int page_increment,
#if NeedWidePrototypes
                        int notify) ;
#else
                        Boolean notify) ;
#endif /* NeedWidePrototypes */

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrollBar_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
