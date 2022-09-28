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
/*   $XConsortium: DrawnBP.h /main/cde1_maint/2 1995/08/18 19:01:29 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDButtonP_h
#define _XmDButtonP_h

#include <Xm/DrawnB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DrawnButton class structure */

typedef struct _XmDrawnButtonClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawnButtonClassPart;


/* Full class record declaration for DrawnButton class */

typedef struct _XmDrawnButtonClassRec {
    CoreClassPart	  core_class;
    XmPrimitiveClassPart  primitive_class;
    XmLabelClassPart      label_class;
    XmDrawnButtonClassPart drawnbutton_class;
} XmDrawnButtonClassRec;


externalref  XmDrawnButtonClassRec xmDrawnButtonClassRec;


/* DrawnButton instance record */

typedef struct _XmDrawnButtonPart
{
   Boolean 	    pushbutton_enabled;
   unsigned char    shadow_type;
   XtCallbackList   activate_callback;
   XtCallbackList   arm_callback;
   XtCallbackList   disarm_callback;
   XtCallbackList   expose_callback;
   XtCallbackList   resize_callback;

   Boolean 	    armed;
   Dimension        old_width;
   Dimension        old_height;
   Dimension        old_shadow_thickness;
   Dimension        old_highlight_thickness;
   XtIntervalId     timer;
   unsigned char    multiClick;         /* KEEP/DISCARD resource */
   int              click_count;
   Time		    armTimeStamp;

} XmDrawnButtonPart;


/* Full instance record declaration */

typedef struct _XmDrawnButtonRec {
    CorePart	     core;
    XmPrimitivePart  primitive;
    XmLabelPart      label;
    XmDrawnButtonPart drawnbutton;
} XmDrawnButtonRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
