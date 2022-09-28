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
/*   $XConsortium: TemplateB.h /main/cde1_maint/2 1995/08/18 19:24:56 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTemplateB_h
#define _XmTemplateB_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmTemplateBoxWidgetClass;

typedef struct _XmTemplateBoxClassRec * XmTemplateBoxWidgetClass;
typedef struct _XmTemplateBoxRec      * XmTemplateBoxWidget;


#ifndef XmIsTemplateBox
#define XmIsTemplateBox(w)  (XtIsSubclass (w, xmTemplateBoxWidgetClass))
#endif



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateTemplateBox() ;
extern Widget XmCreateTemplateDialog() ;

#else

extern Widget XmCreateTemplateBox( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateTemplateDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTemplateB_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
