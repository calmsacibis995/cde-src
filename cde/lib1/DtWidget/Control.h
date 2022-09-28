/* $XConsortium: Control.h /main/cde1_maint/1 1995/07/17 18:26:42 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		Control.h
***
***	project:	MotifPlus Widgets
***
***	description:	Public include file for DtControl class.
***	
***	
***			(c) Copyright 1992 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtControl_h
#define _DtControl_h

#include <Xm/Xm.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define XmCONTROL_NONE		0
#define XmCONTROL_BLANK		1
#define XmCONTROL_BUSY		2
#define XmCONTROL_BUTTON	3
#define XmCONTROL_CLIENT	4
#define XmCONTROL_DATE		5
#define XmCONTROL_MAIL		6
#define XmCONTROL_MONITOR	7
#define XmCONTROL_SWITCH	8

#define NUM_LIST_ITEMS		10

#define XmCR_BUSY_START		60
#define XmCR_BUSY_STOP		61
#define XmCR_MONITOR		62

#define DtMONITOR_OFF           0
#define DtMONITOR_ON            1

#define XmNuseLabelAdjustment	"use_label_adjustment"

#ifndef DtIsControl
#define DtIsControl(w) XtIsSubclass(w, dtControlGadgetClass)
#endif /* DtIsControl */

#ifdef _NO_PROTO

extern Widget _DtCreateControl ();
extern void _DtControlSetFileChanged ();
extern void _DtControlSetBusy ();
extern void _DtControlAddDropAnimationImage ();
extern void _DtControlAddPushAnimationImage ();
extern void _DtControlDoDropAnimation ();
extern void _DtControlDoPushAnimation ();
extern char _DtControlGetMonitorState();

#else

extern Widget _DtCreateControl (Widget, String, ArgList, int);
extern void _DtControlSetFileChanged (Widget, Boolean);
extern void _DtControlSetBusy (Widget, Boolean);
extern void _DtControlAddDropAnimationImage (Widget, String, int);
extern void _DtControlDoDropAnimation (Widget);
extern void _DtControlAddPushAnimationImage (Widget, String, int);
extern void _DtControlDoPushAnimation (Widget);
extern char _DtControlGetMonitorState(Widget);

#endif /* _NO_PROTO */

extern WidgetClass dtControlGadgetClass;

typedef struct _DtControlClassRec * DtControlClass;
typedef struct _DtControlRec      * DtControlGadget;

#define XmCR_FILE_CHANGED	51

typedef struct
    {
    int			 reason;
    XEvent		*event;
    Boolean		 set;
    unsigned char	 control_type;
    XtPointer		 push_function;
    XtPointer		 push_argument;
    Widget		 subpanel;
    long		 file_size;
    }	DtControlCallbackStruct;

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _DtControl_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
