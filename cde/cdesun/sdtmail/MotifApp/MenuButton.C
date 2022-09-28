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
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MenuBar.C	1.36 23 Jun 1995"
#endif

//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////

// MenuBar.C: A menu bar whose panes support items that execute Cmd's

#include "Application.h"
#include "MenuButton.h"
#include "Cmd.h"
#include "CmdList.h"
#include "ButtonGadgetInterface.h"
#include "Help.hh"
#include <Dt/MenuButton.h>

MenuButton::MenuButton (Widget parent, char *button_name) 
  : UIComponent (button_name)
{
  XmString    label_str = NULL;
  Arg args[8];

  _w = NULL;

  if (parent == NULL)
    return;

  label_str = XmStringCreateLocalized(button_name);
  int n = 0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_NONE);  ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  ++n;
  XtSetArg(args[n], XmNshadowThickness, 1); n++;
  XtSetArg(args[n], XmNmarginTop, 1); n++;
  XtSetArg(args[n], XmNmarginBottom, 1); n++;
  XtSetArg(args[n], XmNmarginWidth, 2); n++;
  XtSetArg(args[n], XmNmarginHeight, 1); n++;
  XtSetArg(args[n], XmNlabelString, label_str);  ++n;
  _menubutton_menu = NULL;
  _require_update = TRUE;

  _w = DtCreateMenuButton(parent, "menubutton", args, n);
  XmStringFree(label_str);

  installDestroyHandler();
}

Widget
MenuButton::addCommands (CmdList *list)
{
  WidgetList children = NULL;
  int numChildren = 0, i;

  if (!_w) return NULL;

  int list_size = list ? list->size() : 0;

  // Keep the original popup menu - just destroy its children
  if (_menubutton_menu) {
    XtVaGetValues(_menubutton_menu, 
                  XmNchildren, &children,
                  XmNnumChildren, &numChildren, NULL);
    XtUnmanageChildren(children, numChildren);
    for (i = 0; i < numChildren; i++)
      XtDestroyWidget(children[i]);
  }
  else {
    _menubutton_menu = XmCreatePopupMenu(_w, "menubutton_menu", 0, 0);
    XtVaSetValues(_w, DtNsubMenuId, _menubutton_menu, NULL);
  }

  WidgetList manageList = new Widget[list_size];
  for (i = 0; i < list_size; i++) {
    CmdInterface *ci = new ButtonGadgetInterface(_menubutton_menu, (*list)[i]);
    manageList[i] = ci->baseWidget();
  }

  XtManageChildren(manageList, list_size);
  delete manageList;

  XtManageChild(_w);

  if (list)
    list->setPaneWidget(_menubutton_menu);

  return _w;
}
