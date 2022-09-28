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
/*   $XConsortium: TemplateBP.h /main/cde1_maint/2 1995/08/18 19:25:09 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTemplateBP_h
#define _XmTemplateBP_h

#include <Xm/BulletinBP.h>
#include <Xm/TemplateB.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	XtPointer		extension;      /* Pointer to extension record */
} XmTemplateBoxClassPart;


/* Full class record declaration */

typedef struct _XmTemplateBoxClassRec
{
	CoreClassPart			core_class;
	CompositeClassPart		composite_class;
	ConstraintClassPart		constraint_class;
	XmManagerClassPart		manager_class;
	XmBulletinBoardClassPart	bulletin_board_class;
	XmTemplateBoxClassPart		selection_box_class;
} XmTemplateBoxClassRec;

externalref XmTemplateBoxClassRec xmTemplateBoxClassRec;


/* New fields for the TemplateBox widget record */

typedef struct
{
	Boolean		minimize_buttons ;
} XmTemplateBoxPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmTemplateBoxRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmBulletinBoardPart	bulletin_board;
	XmTemplateBoxPart	template_box;
} XmTemplateBoxRec;



/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmGeoMatrix _XmTemplateBoxGeoMatrixCreate() ;
extern Boolean _XmTemplateBoxNoGeoRequest() ;

#else

extern XmGeoMatrix _XmTemplateBoxGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
extern Boolean _XmTemplateBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTemplateBP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
