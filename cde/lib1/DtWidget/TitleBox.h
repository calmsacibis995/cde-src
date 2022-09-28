/* $XConsortium: TitleBox.h /main/cde1_maint/1 1995/07/17 18:33:07 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		TitleBox.h
***
***	project:	MotifPlus Widgets
***
***	description:	Public include file for DtTitleBox class.
***	
***	
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtTitleBox_h
#define _DtTitleBox_h

#include <Xm/Xm.h>
#include <Dt/DtStrDefs.h>

#ifndef DtIsTitleBox
#define DtIsTitleBox(w) XtIsSubclass(w, DtTitleBoxClass)
#endif /* DtIsTitleBox */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget _DtCreateTitleBox() ;
extern Widget _DtTitleBoxGetTitleArea() ;
extern Widget _DtTitleBoxGetWorkArea() ;

#else

extern Widget _DtCreateTitleBox( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern Widget _DtTitleBoxGetTitleArea( 
                        Widget w) ;
extern Widget _DtTitleBoxGetWorkArea( 
                        Widget w) ;

#endif /* _NO_PROTO */

extern WidgetClass	dtTitleBoxWidgetClass;

typedef struct _DtTitleBoxClassRec * DtTitleBoxWidgetClass;
typedef struct _DtTitleBoxRec      * DtTitleBoxWidget;


#define XmTITLE_TOP	0
#define XmTITLE_BOTTOM	1

#define XmTITLE_AREA	1


#endif /* _DtTitleBox_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
