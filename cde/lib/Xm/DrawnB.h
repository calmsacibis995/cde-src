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
/*   $XConsortium: DrawnB.h /main/cde1_maint/2 1995/08/18 19:01:18 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * DrawnButton Widget
 *
 ***********************************************************************/

#ifndef _XmDButton_h
#define _XmDButton_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsDrawnButton
#define XmIsDrawnButton(w) XtIsSubclass(w, xmDrawnButtonWidgetClass)
#endif /* XmIsDrawnButton */

/* DrawnButon Widget */

externalref WidgetClass xmDrawnButtonWidgetClass;

typedef struct _XmDrawnButtonClassRec *XmDrawnButtonWidgetClass;
typedef struct _XmDrawnButtonRec      *XmDrawnButtonWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateDrawnButton() ;

#else

extern Widget XmCreateDrawnButton( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDButton_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
