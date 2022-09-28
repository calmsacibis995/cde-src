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
#pragma ident "@(#)Icon.C	1.36 03/12/97"
#endif

#ifdef DEBUG_CB_REASON
#define PRINTCB(str) printf("%s\n",str)
#else
#define PRINTCB(str) 	
#endif

#include <string.h>
#include <ctype.h>
#include <Dt/Dnd.h>
#include "Icon.h"
#include "RoamApp.h"
#include "DtMailEditor.hh"
#include "Editor.hh"
#include <Dt/Editor.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <SDtMail/MailRc.hh>

#include <X11/keysym.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>

Icon::Icon ( 
    Attachment *classparent, 
    char *name, 
    unsigned char *label, /* label string */
    Widget parent,
    int  indx,
    SdmBoolean considerBandwidth
) : UIComponent (name)
{
    SdmError mail_error;
    char buf[64];
    char *mime_type = buf;
    char *type = NULL, *icon_name = NULL; 
    char *icon_filename = NULL, *host_prefix = NULL;
    int n = 0;
    Arg args[20];
    int	btn1_transfer = 0;

    _parent = classparent; 
    _indx = indx;
    _is_selected = Sdm_False;
    _is_armed = Sdm_False;

    SdmBoolean approxDataType = Sdm_False;
    if (considerBandwidth) {
      SdmMailRc *mail_rc;
      SdmError error;
      theRoamApp.connection()->connection()->SdmMailRcFactory(error, mail_rc);
      if (mail_rc->IsValueDefined("slownet"))
        approxDataType = Sdm_True;
    }

    // Note, type should not be freed, since it points to internal (cached) Attachment data.
    type = classparent->getDtType(approxDataType);

    // Retrieve the host name.
    host_prefix = SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue(type,
						DtDTS_DA_DATA_HOST,
						NULL);

    // Get the name of the icon.
    icon_name = (char *) SdmDataTypeUtility::SafeDtDtsDataTypeToAttributeValue(type,
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
    XtSetArg (args[n], XmNfillOnArm, Sdm_False);             n++;
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

    installDestroyHandler();

    XtAddEventHandler(XtParent(_w), Button1MotionMask, Sdm_False,
		(XtEventHandler)&Icon::dragMotionHandler,
		(XtPointer)this);
    
    XtVaGetValues((Widget)XmGetXmDisplay(XtDisplay(XtParent(_w))),
	    "enableBtn1Transfer", &btn1_transfer,
	    NULL);

    if (btn1_transfer != True) {
	XtAddEventHandler(XtParent(_w), Button2MotionMask, Sdm_False,
		(XtEventHandler)&Icon::dragMotionHandler,
		(XtPointer)this);
    }

    XtAddCallback(
	_w, 
	XmNcallback, 
	(XtCallbackProc)&Icon::iconCallback,
	(XtPointer)this
    );

    if (host_prefix) {
	SdmDataTypeUtility::SafeDtDtsFreeAttributeValue(host_prefix);
	host_prefix = NULL;
    }
    if (icon_name) {
	SdmDataTypeUtility::SafeDtDtsFreeAttributeValue(icon_name);
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
    XtRemoveEventHandler(XtParent(_w), Button1MotionMask, Sdm_False,
                (XtEventHandler)&Icon::dragMotionHandler,
                (XtPointer)this);
    
    XtVaGetValues((Widget)XmGetXmDisplay(XtDisplay(XtParent(_w))),
	    "enableBtn1Transfer", &btn1_transfer,
	    NULL);

    if (btn1_transfer != True) {
	XtRemoveEventHandler(XtParent(_w), Button2MotionMask, Sdm_False,
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

	if(_is_armed == Sdm_True)
		return;

	_is_selected = Sdm_True;

	invert();

	_is_armed = Sdm_True;
}
	
void
Icon::disarm()
{
	if(_is_armed == Sdm_False || _is_selected == Sdm_True )
		return;

	invert();

	_is_armed = Sdm_False;
}
	
void
Icon::unselect()
{
	if(_is_selected == Sdm_False)
		return;

	_is_selected = Sdm_False;

	disarm();
}


void
Icon::primitiveSelect()
{
	if(_is_selected == Sdm_True || _is_armed == Sdm_True)
		return;

	_is_selected = Sdm_True;

	invert();

	_is_armed = Sdm_True;
}

void
Icon::select()
{
	if(_is_selected == Sdm_True)
		return;

	arm();
}

void
Icon::defaultAction()
{
	select();
	_parent->handleDoubleClick();
}
