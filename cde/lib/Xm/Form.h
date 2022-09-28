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
/*   $XConsortium: Form.h /main/cde1_maint/2 1995/08/18 19:04:33 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmForm_h
#define _XmForm_h


#include <Xm/BulletinB.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Form Widget  */

externalref WidgetClass xmFormWidgetClass;

typedef struct _XmFormClassRec * XmFormWidgetClass;
typedef struct _XmFormRec      * XmFormWidget;


/* ifndef for Fast Subclassing  */

#ifndef XmIsForm
#define XmIsForm(w)	XtIsSubclass(w, xmFormWidgetClass)
#endif  /* XmIsForm */

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateForm() ;
extern Widget XmCreateFormDialog() ;

#else

extern Widget XmCreateForm( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern Widget XmCreateFormDialog( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmForm_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
