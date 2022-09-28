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
/*   $XConsortium: DragOverS.h /main/cde1_maint/2 1995/08/18 18:59:39 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragOverS_h
#define _XmDragOverS_h

#include <Xm/Xm.h>
#include <Xm/DragC.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
 *
 * DragOverShell Widget
 *
 ***********************************************************************/

/* Class record constants */

typedef struct _XmDragOverShellRec 	*XmDragOverShellWidget;
typedef struct _XmDragOverShellClassRec 	*XmDragOverShellWidgetClass;

externalref WidgetClass	xmDragOverShellWidgetClass;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragOverS_h */
/* DON'T ADD STUFF AFTER THIS #endif */
