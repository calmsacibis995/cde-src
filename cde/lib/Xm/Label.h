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
/*   $XConsortium: Label.h /main/cde1_maint/2 1995/08/18 19:08:30 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmLabel_h
#define _XmLabel_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Widget class and record definitions  */

externalref WidgetClass xmLabelWidgetClass;

typedef struct _XmLabelClassRec     * XmLabelWidgetClass;
typedef struct _XmLabelRec      * XmLabelWidget;

/*fast subclass define */
#ifndef XmIsLabel
#define XmIsLabel(w)     XtIsSubclass(w, xmLabelWidgetClass)
#endif /* XmIsLabel */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateLabel() ;

#else

extern Widget XmCreateLabel( 
                        Widget parent,
                        char *name,
                        Arg *arglist,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLabel_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
