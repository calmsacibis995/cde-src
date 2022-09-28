/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident " %W %G "
#endif

#include "RoamApp.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <DtMail/options_util.h>
#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/Xm.h>
#include <DtMail/ToolbarConfig.hh>


////////////////////////////////////////////////////////////////////
ToolbarConfig::ToolbarConfig(Widget parent,
			     Widget cmd_list,
			     Widget status)
{
    _first_time = True;
    _currently_selected = -1;
    _parent = parent;
    _container1 = XtParent(_parent);
    _container2 = XtParent(_container1);
    _status_help = status;
    _cmd_list = cmd_list;
    _button_list = new DtVirtArray<Widget>(10);
    _useIcons = TRUE;
    XtVaSetValues(_parent,
		  XmNinsertPosition, &ToolbarConfig::toolbarOrderProc,
		  NULL);
    unsigned int width = theRoamApp.mailView(0)->width();
//     XtVaSetValues(_parent,
// 		  XmNwidth, width,
// 		  NULL);
}

///////////////////////////////////////////////////////////////////
ToolbarConfig::~ToolbarConfig()
{
    delete _button_list;
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::update(XmStringTable items, int count, Boolean use_icons)
{
    Widget button;
    WidgetList childList;
    int i;

//     XtVaGetValues(_parent,
// 		  XmNchildren, &childList,
// 		  XmNnumChildren, &i,
// 		  NULL);
//     XtUnmanageChildren(childList, i);
//     if (!_first_time) {
	XtVaSetValues(_container1, XmNresizePolicy, XmRESIZE_NONE, NULL);
	XtVaSetValues(_container2, XmNresizePolicy, XmRESIZE_NONE, NULL);
//     } else {
// 	_first_time = False;
//     }
//     XtUnmanageChild(_container1);
    XtUnmanageChild(_parent);

    // Remove the current buttons in the toolbar
    if (_button_list->length() != 0) {
	for (i=(_button_list->length())-1; i>=0; i--) {
	    XtDestroyWidget((*_button_list)[i]);
	    _button_list->remove(i);
	}
    }

    // Add a button for each element in the items array.  These buttons
    // must be widgets and not gadgets because we install event handlers
    // on them.
    for (i=0; i<count; i++) {
	button = XtVaCreateManagedWidget("toolbarButton",
				  xmPushButtonWidgetClass,
				  _parent,
				  XmNuserData, i,
				  NULL);
	XtAddCallback(button, XmNactivateCallback, &ToolbarConfig::buttonCB,
		      (XtPointer) this);
	_button_list->append(button);
    }
    _useIcons = use_icons;
    setLabels(items, count, 0);

//     XtVaGetValues(_parent,
// 		  XmNchildren, &childList,
// 		  XmNnumChildren, &i,
// 		  NULL);
    XtVaSetValues(_container1, XmNresizePolicy, XmRESIZE_ANY, NULL);
    XtVaSetValues(_container2, XmNresizePolicy, XmRESIZE_ANY, NULL);
//    XtManageChildren(childList, i);
    XtManageChild(_parent);
//     XtManageChild(_container1);

    XmListSelectPos(_cmd_list, count, True);
}

///////////////////////////////////////////////////////////////////
Cardinal ToolbarConfig::toolbarOrderProc(Widget w)
{
    Cardinal index;

    XtVaGetValues(w, XmNuserData, &index, NULL);
    return (index);
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::toolbarEH(Widget w,
			      XtPointer client_data,
			      XEvent *event,
			      Boolean * )
{
    ToolbarConfig *obj = (ToolbarConfig *) client_data;
    int index;

    XtVaGetValues(w, XmNuserData, &index, NULL);
    switch (event->type) {
    case EnterNotify:
	obj->showToolbarHelp(index);
	break;
    case LeaveNotify:
	obj->clearStatus();
	break;
    }
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::showToolbarHelp( int index )
{
    XmStringTable items;
    char *str;

    XtVaGetValues(_cmd_list,
		  XmNitems, &items,
		  NULL);
    XmStringGetLtoR(items[index], XmSTRING_DEFAULT_CHARSET, &str);
    XtVaSetValues(_status_help,
		  XmNlabelString, items[index],
		  XmNlabelType, XmSTRING,
		  NULL);
}    

///////////////////////////////////////////////////////////////////
void ToolbarConfig::clearStatus()
{
    XmString label_str = XmStringCreateLocalized(" ");

    XtVaSetValues(_status_help,
		  XmNlabelString, label_str,
		  NULL);
    XmStringFree(label_str);
}    


///////////////////////////////////////////////////////////////////
void ToolbarConfig::buttonCB(Widget w,
			     XtPointer client_data,
			     XtPointer)
{
    ToolbarConfig *obj = (ToolbarConfig *)client_data;
    int index;

    XtVaGetValues(w, XmNuserData, &index, NULL);
    XmListSelectPos(obj->_cmd_list, index + 1, True);
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::insertItems(XmStringTable items,
				int size,
				int count,
				int pos)
{
    int i;
    Widget button;
    WidgetList childList;

    // These buttons must be widgets and not gadgets because we install
    // event handlers on them.
    for (i=0; i<count; i++) {
	button = XtVaCreateWidget("toolbarButton",
				  xmPushButtonWidgetClass,
				  _parent,
				  XmNuserData, pos+i,
				  NULL);
	XtAddCallback(button, XmNactivateCallback, &ToolbarConfig::buttonCB,
		      (XtPointer) this);
	_button_list->insert(button, pos+i);
    }
    for (i=pos; i<_button_list->length(); i++) {
	XtVaSetValues((*_button_list)[i],
		      XmNuserData, i,
		      NULL);
    }
    setLabels(items, size, pos);

    XtVaGetValues(_parent,
		  XmNchildren, &childList,
		  XmNnumChildren, &i,
		  NULL);
    XtManageChildren(childList, i);
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::setLabels(XmStringTable items,
			      int size,
			      int start_pos)
{
    CmdList *toolbar_cmds = theRoamApp.mailView(0)->getToolbarCmds();
    DtVirtArray<char *> *toolbar_icons = theRoamApp.mailView(0)->getToolbarIcons();
    Pixmap cmd_glyph;
    char *str;
    int i, j;
    Pixel fg, bg;

    if (_useIcons) {
	if (!XtIsManaged(_status_help))
	    XtManageChild(_status_help);
	XtVaGetValues(_parent,
		      XmNforeground,  &fg,
		      XmNbackground,  &bg,
		      NULL);
	for (i=start_pos; i<size; i++) {
	    XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &str);
	    for(j=0; j<toolbar_cmds->size(); j++) {
		if (strcmp((*toolbar_cmds)[j]->getLabel(), str) == 0) {
		    cmd_glyph = XmGetPixmap(XtScreen(_parent),
					    (*toolbar_icons)[j], fg, bg);
		    break;
		}
	    }
	    XtVaSetValues((*_button_list)[i],
			  XmNlabelType, XmPIXMAP,
			  XmNlabelPixmap, cmd_glyph,
			  NULL);
	    XtAddEventHandler((*_button_list)[i],
			      EnterWindowMask | LeaveWindowMask,
			      False, &ToolbarConfig::toolbarEH,
			      (XtPointer) this);
	}
    } else {
	if (XtIsManaged(_status_help))
	    XtUnmanageChild(_status_help);
	for (i=start_pos; i<size; i++) {
	    XtVaSetValues((*_button_list)[i],
			  XmNlabelType, XmSTRING,
			  XmNlabelString, items[i],
			  NULL);
	    XtRemoveEventHandler((*_button_list)[i],
			      EnterWindowMask | LeaveWindowMask,
			      False, &ToolbarConfig::toolbarEH,
			      (XtPointer) this);
	}
    }
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::useIcons(XmStringTable items,
			     Boolean use_icons)
{
    CmdList *toolbar_cmds = theRoamApp.mailView(0)->getToolbarCmds();
    DtVirtArray<char *> *toolbar_icons = theRoamApp.mailView(0)->getToolbarIcons();
    int i, j;
    char *str;
    Pixmap icon;
    Pixel fg, bg;

    if (_useIcons != use_icons) {
	XtVaGetValues(_parent,
		      XmNforeground,  &fg,
		      XmNbackground,  &bg,
		      NULL);
	XtVaSetValues(_container1, XmNresizePolicy, XmRESIZE_NONE, NULL);
	XtVaSetValues(_container2, XmNresizePolicy, XmRESIZE_NONE, NULL);
// 	XtUnmanageChild(_container1);
	XtUnmanageChild(_parent);
	if (use_icons) {
	    XtManageChild(_status_help);
	    for (i=0; i<_button_list->length(); i++) {
		XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &str);
		for(j=0; j<toolbar_cmds->size(); j++) {
		    if (strcmp((*toolbar_cmds)[j]->getLabel(), str) == 0) {
			icon = XmGetPixmap(XtScreen(_parent),
					   (*toolbar_icons)[j], fg, bg);
			break;
		    }
		}
		XtVaSetValues((*_button_list)[i],
			      XmNlabelType, XmPIXMAP,
			      XmNlabelPixmap, icon,
			      NULL);
		XtAddEventHandler((*_button_list)[i],
				  EnterWindowMask | LeaveWindowMask,
				  False, &ToolbarConfig::toolbarEH,
				  (XtPointer) this);
	    }
	} else {
	    XtUnmanageChild(_status_help);
	    for (i=0; i<_button_list->length(); i++) {
		XtVaSetValues((*_button_list)[i],
			      XmNlabelType, XmSTRING,
			      XmNlabelString, items[i],
			      NULL);
		XtRemoveEventHandler((*_button_list)[i],
				     EnterWindowMask | LeaveWindowMask,
				     False, &ToolbarConfig::toolbarEH,
				     (XtPointer) this);
	    }
	}
	XtVaSetValues(_container1, XmNresizePolicy, XmRESIZE_ANY, NULL);
	XtVaSetValues(_container2, XmNresizePolicy, XmRESIZE_ANY, NULL);
	XtManageChild(_parent);
// 	XtManageChild(_container1);
	_useIcons = use_icons;
    }
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::swapLabels(XmStringTable items, int itemA, int itemB)
{
    CmdList *toolbar_cmds = theRoamApp.mailView(0)->getToolbarCmds();
    DtVirtArray<char *> *toolbar_icons = theRoamApp.mailView(0)->getToolbarIcons();
    char *str1, *str2;
    Pixmap icon1, icon2;
    int index, size;
    Pixel fg, bg;

    size = toolbar_cmds->size();

    if (_useIcons) {
	XtVaGetValues(_parent,
		      XmNforeground,  &fg,
		      XmNbackground,  &bg,
		      NULL);
	XmStringGetLtoR(items[itemA], XmSTRING_DEFAULT_CHARSET, &str1);
	for(index=0; index<size; index++) {
	    if (strcmp((*toolbar_cmds)[index]->getLabel(), str1) == 0) {
		icon1 = XmGetPixmap(XtScreen(_parent),
				    (*toolbar_icons)[index], fg, bg);
		break;
	    }
	}
	XmStringGetLtoR(items[itemB], XmSTRING_DEFAULT_CHARSET, &str2);
	for(index=0; index<size; index++) {
	    if (strcmp((*toolbar_cmds)[index]->getLabel(), str2) == 0) {
		icon2 = XmGetPixmap(XtScreen(_parent),
				    (*toolbar_icons)[index], fg, bg);
		break;
	    }
	}
	
	XtVaSetValues((*_button_list)[itemA],
		      XmNlabelType, XmPIXMAP,
		      XmNlabelPixmap, icon2,
		      NULL);
	XtVaSetValues((*_button_list)[itemB],
		      XmNlabelType, XmPIXMAP,
		      XmNlabelPixmap, icon1,
		      NULL);
    } else {
	XtVaSetValues((*_button_list)[itemA],
		      XmNlabelType, XmSTRING,
		      XmNlabelString, items[itemB],
		      NULL);
	XtVaSetValues((*_button_list)[itemB],
		      XmNlabelType, XmSTRING,
		      XmNlabelString, items[itemA],
		      NULL);
    }
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::removeItem(int index)
{
    int i;

    XtDestroyWidget((*_button_list)[index]);
    _button_list->remove(index);

    if (_button_list->length() == 0)
	return;

    for (i=index; i<_button_list->length(); i++) {
	XtVaSetValues((*_button_list)[i],
		      XmNuserData, i,
		      NULL);
    }

//     XtVaSetValues((*_button_list)[_currently_selected],
// 		  XmNshowAsDefault, 3,
// 		  NULL);
}

///////////////////////////////////////////////////////////////////
void ToolbarConfig::selectItem(int index)
{
    if (_currently_selected == index)
	return;

//     XtVaSetValues((*_button_list)[index],
// 		  XmNshowAsDefault, 3,
// 		  NULL);
//     if (_currently_selected >= 0 &&
// 	_currently_selected < _button_list->length()) {

// 	XtVaSetValues((*_button_list)[_currently_selected],
// 		      XmNshowAsDefault, 0,
// 		      NULL);
//     }
    _currently_selected = index;
}
