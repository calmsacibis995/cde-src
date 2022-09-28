/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: DrawingAP.h,v $ $Revision: 1.22 $ $Date: 93/12/10 11:44:24 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawingAreaVisP_h
#define _XmDrawingAreaVisP_h

#include <Xm/ManagerP.h>
#include <Xm/DrawingAP.h>
#include "DrawingAVis.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XmRESIZE_SWINDOW	10


/* Constraint part record for DrawingAreaVis widget */

typedef struct _XmDrawingAreaVisConstraintPart
{
   char unused;
} XmDrawingAreaVisConstraintPart, * XmDrawingAreaVisConstraint;

/*  New fields for the DrawingAreaVis widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawingAreaVisClassPart;


/* Full class record declaration */

typedef struct _XmDrawingAreaVisClassRec
{
	CoreClassPart		   core_class;
	CompositeClassPart	   composite_class;
	ConstraintClassPart	   constraint_class;
	XmManagerClassPart	   manager_class;
	XmDrawingAreaClassPart	   drawing_area_class;
	XmDrawingAreaVisClassPart   drawing_areaVis_class;
} XmDrawingAreaVisClassRec;

externalref XmDrawingAreaVisClassRec xmDrawingAreaVisClassRec;


/* New fields for the DrawingAreaVis widget record */

typedef struct
{
	Visual			*visual;
} XmDrawingAreaVisPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmDrawingAreaVisRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmDrawingAreaPart	drawing_area;
	XmDrawingAreaVisPart	drawing_areaVis;
} XmDrawingAreaVisRec;



/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmDrawingAreaInput() ;

#else

extern void _XmDrawingAreaInput( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawingAreaVisP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
