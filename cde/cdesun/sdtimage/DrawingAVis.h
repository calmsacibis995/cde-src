/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: DrawingA.h,v $ $Revision: 1.19 $ $Date: 93/03/03 16:25:42 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawingAreaVis_h
#define _XmDrawingAreaVis_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmDrawingAreaVisWidgetClass;

typedef struct _XmDrawingAreaVisClassRec * XmDrawingAreaVisWidgetClass;
typedef struct _XmDrawingAreaVisRec      * XmDrawingAreaVisWidget;


#ifndef XmIsDrawingAreaVis
#define XmIsDrawingAreaVis(w)  (XtIsSubclass (w, xmDrawingAreaVisWidgetClass))
#endif



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateDrawingAreaVis() ;

#else

extern Widget XmCreateDrawingAreaVis( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawingAreaVis_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
