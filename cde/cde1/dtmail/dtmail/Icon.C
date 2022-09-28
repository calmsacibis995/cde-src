/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Icon.C	1.30 05/09/96"
#endif

#ifdef DEBUG_CB_REASON
#define PRINTCB(str) printf("%s\n",str)
#else
#define PRINTCB(str) 	
#endif

#include <Dt/Dnd.h>
#include "Icon.h"
#include "RoamApp.h"
#include "DtMailEditor.hh"
#include "Editor.hh"
#include <Dt/Editor.h>

#include <X11/keysym.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Icon::Icon ( 
    Attachment *classparent, 
    char *name, 
    unsigned char *label, /* label string */
    unsigned short /* type */,
    Widget parent,
    int  indx
) : UIComponent (name)
{
    DtMail::BodyPart	*bp;
    DtMailEnv mail_error;
    char *type = NULL, *icon_name = NULL; 
    char *icon_filename = NULL, *host_prefix = NULL;
    int n = 0;
    Arg args[20];
    int	btn1_transfer = 0;

    _parent = classparent; 
    _indx = indx;
    _is_selected = FALSE;
    _is_armed = FALSE;

    // Get the pixmap image file name for this attachment.
    bp = _parent->getBodyPart();
    bp->getContents(mail_error, NULL, NULL, &type, NULL, NULL, NULL);

    // Retrieve the host name.
    host_prefix = DtDtsDataTypeToAttributeValue(type,
						DtDTS_DA_DATA_HOST,
						NULL);

    // Get the name of the icon.
    icon_name = (char *) DtDtsDataTypeToAttributeValue(type,
						    DtDTS_DA_ICON,
						    NULL);
    // Retrieve icon file name
    icon_filename = XmGetIconFileName(XtScreen(parent),
					NULL,
					icon_name,
					host_prefix,
					DtMEDIUM);

    n = 0;
    XtSetArg (args[n], XmNshadowThickness, 0);          n++;
    XtSetArg (args[n], XmNfillOnArm, FALSE);             n++;
    XtSetArg (args[n], XmNhighlightThickness, 2);       n++;
    XtSetArg (args[n], XmNimageName, icon_filename);    n++;
    XtSetArg (args[n], XmNstring, (char *)label);	n++;
    XtSetArg( args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
    XtSetArg( args[n], XmNpixmapPosition, XmPIXMAP_TOP); n++;
    XtSetArg( args[n], XmNbehavior, XmICON_DRAG); n++;
    XtSetArg( args[n], XmNfillMode, XmFILL_PARENT); n++;

    _cur_fg = _parent->parent()->owner()->textEditor()->get_text_foreground();
    _cur_bg = _parent->parent()->owner()->textEditor()->get_text_background();

    XtSetArg( args[n], XmNbackground, _cur_bg); n++;
    XtSetArg( args[n], XmNforeground, _cur_fg); n++;
    
    _w = _DtCreateIcon(parent,"iconGadget", args,n);

    XtAddEventHandler(XtParent(_w), Button1MotionMask, FALSE,
		(XtEventHandler)&Icon::dragMotionHandler,
		(XtPointer)this);
    
    XtVaGetValues((Widget)XmGetXmDisplay(XtDisplay(XtParent(_w))),
	    "enableBtn1Transfer", &btn1_transfer,
	    NULL);

    if (btn1_transfer != True) {
	XtAddEventHandler(XtParent(_w), Button2MotionMask, FALSE,
		(XtEventHandler)&Icon::dragMotionHandler,
		(XtPointer)this);
    }

    XtAddCallback(
	_w, 
	XmNcallback, 
	(XtCallbackProc)&Icon::iconCallback,
	(XtPointer)this
    );

    if (type) {
	free(type);
	type = NULL;
    }
    
    if (host_prefix) {
	DtDtsFreeAttributeValue(host_prefix);
	host_prefix = NULL;
    }
    if (icon_name) {
	DtDtsFreeAttributeValue(icon_name);
	icon_name = NULL;
    }
    if (icon_filename) {
	free(icon_filename);
	icon_filename = NULL;
    }
}

Icon::~Icon() 
{
    int btn1_transfer = 0;

    XtRemoveCallback(
        _w, 
        XmNcallback,
        (XtCallbackProc)&Icon::iconCallback,
        (XtPointer)this
    );
    XtRemoveEventHandler(XtParent(_w), Button1MotionMask, FALSE,
                (XtEventHandler)&Icon::dragMotionHandler,
                (XtPointer)this);
    
    XtVaGetValues((Widget)XmGetXmDisplay(XtDisplay(XtParent(_w))),
	    "enableBtn1Transfer", &btn1_transfer,
	    NULL);

    if (btn1_transfer != True) {
	XtRemoveEventHandler(XtParent(_w), Button2MotionMask, FALSE,
		(XtEventHandler)&Icon::dragMotionHandler,
		(XtPointer)this);
    }

}

void
Icon::iconCallback(Widget , XtPointer client_data, XtPointer call_data)
{
	DtIconCallbackStruct *cb = (DtIconCallbackStruct *)call_data;
	Icon *obj = (Icon *)client_data;
	XEvent *event;

	event = cb->event;
	if (event == NULL)
 	    return;
	if ((event->type == ButtonPress) &&
	    (! IsModifierKey(XLookupKeysym((XKeyEvent *) event, 0)))) {
	    if (event->xbutton.state & ControlMask)
	      {
		  /* Cntrl Button press processing*/
		  // Need to handle it.  Postponed...
		  return;
	      }
	}

	switch(cb->reason) {
		case XmCR_UNHIGHLIGHT:
			PRINTCB("unhighlight");
			break;
		case XmCR_HIGHLIGHT:
			PRINTCB("highlight");
			break;
		case XmCR_ACTIVATE:
			PRINTCB("activate");
			break;
		case XmCR_DISARM:
			obj->disarm();
			PRINTCB("disarm");
			break;
		case XmCR_ARM:
			obj->arm();
			PRINTCB("arm");
			break;
		case XmCR_SELECT:
			PRINTCB("select");
			obj->select();
			break;
		case XmCR_DEFAULT_ACTION:
			PRINTCB("default_action");
			obj->defaultAction();
			break;
		case XmCR_DRAG:
			PRINTCB("drag");
			break;
		default:
			break;
	}	
}

void
Icon::dragMotionHandler(
    Widget	widget,
    XtPointer	clientData,
    XEvent	*event)
{
    Icon	*icon = (Icon *) clientData;

    icon->parent()->parent()->owner()->attachDragMotionHandler(widget, event);
}

void
Icon::invert()
{
	Arg args[2];
	int n;
	Pixel swap;

	n = 0;
    	XtSetArg( args[n], XmNbackground, _cur_fg); n++;
    	XtSetArg( args[n], XmNforeground, _cur_bg); n++;
	XtSetValues(_w, args, n);
	
	swap = _cur_fg;
	_cur_fg = _cur_bg;
	_cur_bg = swap;
}


void
Icon::arm()
{
	// deselect others in all cases
	_parent->set_selected();

	if(_is_armed == TRUE)
		return;

	_is_selected = TRUE;

	invert();

	_is_armed = TRUE;
}
	
void
Icon::disarm()
{
	if(_is_armed == FALSE || _is_selected == TRUE )
		return;

	invert();

	_is_armed = FALSE;
}
	
void
Icon::unselect()
{
	if(_is_selected == FALSE)
		return;

	_is_selected = FALSE;

	disarm();
}


void
Icon::primitiveSelect()
{
	if(_is_selected == TRUE || _is_armed == TRUE)
		return;

	_is_selected = TRUE;

	invert();

	_is_armed = TRUE;
}

void
Icon::select()
{
	if(_is_selected == TRUE)
		return;

	arm();
}

void
Icon::defaultAction()
{
	select();
	_parent->handleDoubleClick();
}
