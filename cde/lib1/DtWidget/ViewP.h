/* $XConsortium: ViewP.h /main/cde1_maint/1 1995/07/17 18:33:55 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		ViewP.h
***
***	project:	MotifPlus Widgets
***
***	description:	Private include file for DtView class.
***	
***	
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtViewP_h
#define _DtViewP_h

#include <Xm/XmP.h>
#include <Xm/BulletinBP.h>
#include <Xm/FormP.h>
#include <Dt/View.h>



/*-------------------------------------------------------------
**	Class Structure
*/

/*	Class Part
*/
typedef struct _DtViewClassPart
{
	caddr_t			extension;
} DtViewClassPart;

/*	Full Class Record
*/
typedef struct _DtViewClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmBulletinBoardClassPart	bulletin_board_class;
	XmFormClassPart		form_class;
	DtViewClassPart	view_class;
} DtViewClassRec;

/*	Actual Class
*/
extern DtViewClassRec dtViewClassRec;




/*-------------------------------------------------------------
**	Instance Structure
*/

/*	Instance Part
*/
typedef struct _DtViewPart
{
	Dimension	width_increment;
	Dimension	height_increment;
	Dimension	left_inset;
	Dimension	right_inset;
	Dimension	top_inset;
	Dimension	bottom_inset;
	unsigned char	box_type;
        Boolean         subpanel_unpost_on_select;
        Boolean         subpanel_torn;
} DtViewPart;

/*	Full Instance Record
*/
typedef struct _DtViewRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmBulletinBoardPart	bulletin_board;
	XmFormPart		form;
	DtViewPart		view;
} DtViewRec;


/*-------------------------------------------------------------
**	Constraint Structure
*/

/*	Constraint Part
*/
typedef struct _DtViewConstraintPart
{
	unsigned char	foo_bar;
} DtViewConstraintPart, * DtViewConstraint;

/*	Full Constraint Record
*/
typedef struct _DtViewConstraintRec
{
	XmManagerConstraintPart		manager_constraint;
	XmFormConstraintPart		form_constraint;
 	DtViewConstraintPart	view_constraint;
} DtViewConstraintRec, * DtViewConstraintPtr;


/*-------------------------------------------------------------
**	Class and Instance Macros
*/

/*	DtView Class Macros
*/	

/*	DtView Instance Macros
*/
#define M_BottomShadowColor(m)	(m -> manager.bottom_shadow_color)
#define M_TopShadowColor(m)	(m -> manager.top_shadow_color)
#define M_HighlightColor(m)	(m -> manager.highlight_color)
#define M_BackgroundPixmap(m)	(m -> core.background_pixmap)
#define M_LeftInset(m)		(m -> view.left_inset)
#define M_RightInset(m)		(m -> view.right_inset)
#define M_TopInset(m)		(m -> view.top_inset)
#define M_BottomInset(m)	(m -> view.bottom_inset)
#define M_BoxType(m)		(m -> view.box_type)
#define M_WidthIncrement(m)	(m -> view.width_increment)
#define M_HeightIncrement(m)	(m -> view.height_increment)


#endif /* _DtViewP_h */

/* DON'T ADD ANYTHING AFTER THIS #endif */
