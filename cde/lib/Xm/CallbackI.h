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
/*   $XConsortium: CallbackI.h /main/cde1_maint/2 1995/08/18 18:52:30 drk $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmCallbackI_h
#define _XmCallbackI_h

#include "XmI.h"

#ifdef __cplusplus
extern "C" {
#endif


#define _XtCBCalling 1
#define _XtCBFreeAfterCalling 2

typedef struct internalCallbackRec {
    unsigned short count;
    char           is_padded;   /* contains NULL padding for external form */
    char           call_state;  /* combination of _XtCB{FreeAfter}Calling */
    XtCallbackRec  callbacks;	/* first of an array of XtCallbackRec's */
} InternalCallbackRec, *InternalCallbackList;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmAddCallback() ;
extern void _XmRemoveCallback() ;
extern void _XmCallCallbackList() ;
extern void _XmRemoveAllCallbacks() ;

#else

extern void _XmAddCallback( 
                        InternalCallbackList *callbacks,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void _XmRemoveCallback( 
                        InternalCallbackList *callbacks,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void _XmCallCallbackList( 
                        Widget widget,
                        XtCallbackList callbacks,
                        XtPointer call_data) ;
extern void _XmRemoveAllCallbacks(
                        InternalCallbackList *callbacks) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCallbackI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
