/* $XConsortium: DialogBox.h /main/cde1_maint/1 1995/07/17 18:27:09 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		DialogBox.h
***
***	project:	MotifPlus Widgets
***
***	description:	Public include file for DtDialogBox class.
***	
***	
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtDialogBox_h
#define _DtDialogBox_h

#include <Xm/Xm.h>
#include <Dt/DtStrDefs.h>

#ifndef DtIsDialogBox
#define DtIsDialogBox(w) XtIsSubclass(w, dtDialogBoxWidgetClass)
#endif /* XmIsDialogBox */

#ifdef _NO_PROTO

extern Widget _DtCreateDialogBox() ;
extern Widget __DtCreateDialogBoxDialog() ;
extern Widget _DtDialogBoxGetButton() ;
extern Widget _DtDialogBoxGetWorkArea() ;

#else

extern Widget _DtCreateDialogBox( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern Widget __DtCreateDialogBoxDialog( 
                        Widget ds_p,
                        String name,
                        ArgList db_args,
                        Cardinal db_n) ;
extern Widget _DtDialogBoxGetButton( 
                        Widget w,
                        Cardinal pos) ;
extern Widget _DtDialogBoxGetWorkArea( 
                        Widget w) ;

#endif /* _NO_PROTO */

extern WidgetClass dtDialogBoxWidgetClass;

typedef struct _DtDialogBoxClassRec * DtDialogBoxWidgetClass;
typedef struct _DtDialogBoxRec      * DtDialogBoxWidget;


#define XmBUTTON	11


#define XmCR_DIALOG_BUTTON	100

typedef struct
{
	int		reason;
	XEvent *	event;
	Cardinal	button_position;
	Widget		button;
} DtDialogBoxCallbackStruct;


#endif /* _DtDialogBox_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
