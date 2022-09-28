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
#pragma ident "@(#)MenuBar.C	1.53 04/15/97"
#endif

///////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// MenuBar.C: A menu bar whose panes support items
//            that execute Cmd's
//////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// MODIFIED TO SUPPORT SUBMENUS - not described in Book
///////////////////////////////////////////////////////////

#include "Application.h"
#include "MenuBar.h"
#include "Cmd.h"
#include "CmdList.h"
#include "ToggleButtonInterface.h"
#include "ButtonGadgetInterface.h"
#include "ButtonInterface.h"
#include "Help.hh"
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/PushBG.h>
#include <Xm/Label.h>

MenuBar::MenuBar ( char *name, Widget pulldown) 
: UIComponent ( name )
{
    _w = pulldown;
}

MenuBar::MenuBar ( Widget parent, char *name, unsigned char type ) 
: UIComponent ( name )
{
    // Base widget is a Motif menu bar widget
    
    if(parent == NULL)
	return;

    Arg args[1];

    if( type == XmMENU_POPUP) {
	XtSetArg(args[0], XmNwhichButton, theApplication->bMenuButton()); 
    	_w = XmCreatePopupMenu( parent, _name, args, 1 );
    }
    else if (type == XmMENU_PULLDOWN) {
	_w = XmCreatePulldownMenu( parent, _name, NULL, 0 );
    }
    else { // default to Menu Bar
	type = XmMENU_BAR;
    	_w = XmCreateMenuBar ( parent, _name, NULL, 0 );
    }

    installDestroyHandler();
}

Widget MenuBar::addCommands (Widget *menuBarCascade,
			     CmdList *list,
			     Boolean help,
			     unsigned char type,
			     Boolean useWorkProc)
{
    return( createPulldown ( _w, list, menuBarCascade, help, type, useWorkProc ) );
}

Widget MenuBar::addCommands ( CmdList *list,
			      Boolean help,
			      unsigned char type,
			      Boolean useWorkProc)
{
    return( createPulldown ( _w, list, help, type, useWorkProc ) );
}

Widget 
MenuBar::createPulldown (Widget parent, 
			 CmdList *list, 
			 Widget *cascade, 
			 Boolean help,
			 unsigned char type,
			 Boolean useWorkProc)
{
    Widget pulldown;
    Arg  args[2];
    char * helpId;
    XmString label_str;
    char name[200];	// The mapped name, take out special chars.

    if (type != XmMENU_POPUP && type != XmMENU_PULLDOWN)
	type = XmMENU_BAR;
    
    // Create a pulldown menu pane for this list of commands
    
    if( type == XmMENU_BAR) {
	ButtonInterface::mapName(list->name(), name);
	pulldown = XmCreatePulldownMenu(parent, name, NULL, 0);
	label_str = XmStringCreateLocalized(list->getLabel());
	XtVaSetValues(pulldown, XmNlabelString, label_str, NULL);

	// Each entry in the menu bar must have a cascade button
	// from which the user can pull down the pane

	if (cascade != NULL && *cascade != NULL)
	    XtVaSetValues (*cascade, 
			   XmNsubMenuId, pulldown, 
			   XmNlabelString, label_str,
			   NULL);
	else {
	    XtSetArg(args[0], XmNsubMenuId, pulldown);
	    XtSetArg(args[1], XmNlabelString, label_str);
	    *cascade = XtCreateWidget (name, 
				       xmCascadeButtonWidgetClass,
				       parent, 
				       args, 2);
	    if (help) {
		XtSetArg (args[0], XmNmenuHelpWidget, *cascade);
		XtSetValues (parent, args, 1);
	    }
			
	    XtManageChild ( *cascade );
	    list->setCascadeWidget(*cascade);

	    // Install callbacks for each of the 
	    // pulldown menus so we can get
	    // On Item help for them.

	    helpId = XtName(*cascade);
	    if (!strcmp(helpId, "Mailbox")) {
		XtAddCallback(*cascade, XmNhelpCallback, 
		HelpCB, DTMAILCONTAINERMENUID);
	    } else if (!strcmp(helpId, "Move")) {
		XtAddCallback(*cascade, XmNhelpCallback, HelpCB, DTMAILMOVEMENUID);
	    } else if (!strcmp(helpId, "Message")) {
		XtAddCallback(*cascade, XmNhelpCallback, HelpCB, DTMAILMESSAGEMENUID);
	    } else if (!strcmp(helpId, "Options")) {
		XtAddCallback(*cascade, XmNhelpCallback, HelpCB, DTMAILOPTIONSMENUID);
	    } else if (!strcmp(helpId, "View")) {
		XtAddCallback(*cascade, XmNhelpCallback, HelpCB, DTMAILVIEWMENUID);
	    } else if (!strcmp(helpId, "Compose")) {
		XtAddCallback(*cascade, XmNhelpCallback, HelpCB, DTMAILCOMPOSEMENUID);
	    }
	}
	XmStringFree(label_str);
    } else
	pulldown = _w;

    list->setPaneWidget(pulldown);

    // If useWorkProc is true, then kick off a workproc to actually build the
    // menus.  This is used primarily on startup.
    if (useWorkProc) {
	MenuBarContext *mbCtx = new MenuBarContext;
	XtWorkProcId id;

	mbCtx->mb = this;
	mbCtx->menupane = pulldown;
	mbCtx->list = list;

	list->setPending(True);
	id = XtAppAddWorkProc(XtWidgetToApplicationContext(_w),
			      createPulldownWP,
			      mbCtx);
    } else {
	// Loop through the cmdList, creating a menu 
	// entry for each command. 
	Widget *head_wl, *wl;
	Cardinal num_wl = 0;
	int i;
	head_wl = wl = new Widget[ list->size() ];
    
	for ( i = 0; i < list->size(); i++) {
	    if(!strcmp((*list)[i]->className(), "CmdList")) {
		Widget pane = createPulldown(pulldown,
					     (CmdList*) (*list)[i],
					     FALSE,
					     XmMENU_BAR,
					     useWorkProc);
		label_str = XmStringCreateLocalized(((CmdList *)(*list)[i])->getLabel());
		XtVaSetValues(pane, 
			      XmNlabelString, label_str,
			      NULL);
		XmStringFree(label_str);
	    } else {
		if ( !strcmp((*list)[i]->className(),"SeparatorCmd")) {
		    *(wl++) = XtCreateWidget ( (*list)[i]->name(),
					       xmSeparatorWidgetClass,
					       pulldown,
					       NULL, 0);
		} else if (!strcmp((*list)[i]->className(),"ToggleButtonCmd")) {
		    CmdInterface *ci;
		    ci = new ToggleButtonInterface(pulldown, (*list)[i] );
		    *(wl++) = ci->baseWidget();
		} else if(!strcmp((*list)[i]->className(),"LabelCmd")) {
		    label_str = XmStringCreateLocalized((*list)[i]->getLabel());
		    ButtonInterface::mapName((*list)[i]->name(), name);
		    *(wl++) = XtVaCreateWidget (name,
						xmLabelWidgetClass,
						pulldown,
						XmNlabelString, label_str,
						NULL);
		    XmStringFree(label_str);
		} else {
		    CmdInterface *ci;
		    ci  = new ButtonGadgetInterface ( pulldown, (*list)[i] );
		    *(wl++) = ci->baseWidget();
		}
		num_wl++;
	    }
	}
	XtManageChildren (head_wl, num_wl);
	delete head_wl;
    }
    return(pulldown);
}

Widget 
MenuBar::createPulldown (Widget parent, 
			 CmdList *list, 
			 Boolean help ,
			 unsigned char type,
			 Boolean useWorkProc)
{
    Widget pulldown, cascade;
    Arg  args[1];
    char * helpId;
    XmString label_str;
    char name[200];

    name[0] = '\0';

    if (type != XmMENU_POPUP && type != XmMENU_PULLDOWN)
	type = XmMENU_BAR;
    
    // Create a pulldown menu pane for this list of commands
    
    if( type == XmMENU_BAR) {
	ButtonInterface::mapName(list->name(), name);
	pulldown = XmCreatePulldownMenu(parent,
					name, NULL, 0 );
	label_str = XmStringCreateLocalized(list->getLabel());
	XtVaSetValues(pulldown,
		      XmNlabelString, label_str,
		      NULL);
	// Each entry in the menu bar must have a cascade button
	// from which the user can pull down the pane

	XtSetArg(args[0], XmNsubMenuId, pulldown);
	cascade = XtCreateWidget (name, 
				   xmCascadeButtonWidgetClass,
				   parent, 
				   args, 1);
	XtVaSetValues(cascade,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);
	if (help) {
	    XtSetArg (args[0], XmNmenuHelpWidget, cascade);
	    XtSetValues (parent, args, 1);
	}

	XtManageChild ( cascade );
	list->setCascadeWidget(cascade);

	// Install callbacks for each of the 
	// pulldown menus so we can get
	// On Item help for them.
	helpId = XtName(cascade);
	if (!strcmp(helpId, "Mailbox")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILCONTAINERMENUID);
	} else if (!strcmp(helpId, "Move")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILMOVEMENUID);
	} else if (!strcmp(helpId, "Message")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILMESSAGEMENUID);
	} else if (!strcmp(helpId, "Options")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILOPTIONSMENUID);
	} else if (!strcmp(helpId, "View")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILVIEWMENUID);
	} else if (!strcmp(helpId, "Compose")) {
	    XtAddCallback(cascade, XmNhelpCallback, 
			    HelpCB, DTMAILCOMPOSEMENUID);
	}
    } else
	pulldown = _w;

    list->setPaneWidget(pulldown);

    if (useWorkProc) {
	MenuBarContext *mbCtx = new MenuBarContext;
	XtWorkProcId id;

	mbCtx->mb = this;
	mbCtx->menupane = pulldown;
	mbCtx->list = list;

	list->setPending(True);
	id = XtAppAddWorkProc(XtWidgetToApplicationContext(_w),
			      &MenuBar::createPulldownWP,
			      mbCtx);
    } else {
	// Loop through the cmdList, creating a menu 
	// entry for each command. 
	Widget *head_wl, *wl;
	Cardinal num_wl = 0;
	int i;
	head_wl = wl = new Widget[ list->size() ];
    
	for ( i = 0; i < list->size(); i++) {
	    if(!strcmp((*list)[i]->className(), "CmdList")) {
		Widget pane = createPulldown(pulldown,
					     (CmdList*) (*list)[i],
					     FALSE,
					     XmMENU_BAR,
					     useWorkProc);
		label_str = XmStringCreateLocalized(((CmdList *)(*list)[i])->getLabel());
		XtVaSetValues(pane, 
			      XmNlabelString, label_str,
			      NULL);
		XmStringFree(label_str);
	    } else {
		if ( !strcmp((*list)[i]->className(),"SeparatorCmd")) {
		    *(wl++) = XtCreateWidget ( (*list)[i]->name(),
					       xmSeparatorWidgetClass,
					       pulldown,
					       NULL, 0);
		} else if (!strcmp((*list)[i]->className(),"ToggleButtonCmd")) {
		    CmdInterface *ci;
		    ci = new ToggleButtonInterface(pulldown, (*list)[i] );
		    *(wl++) = ci->baseWidget();
		} else if(!strcmp((*list)[i]->className(),"LabelCmd")) {
		    label_str = XmStringCreateLocalized((*list)[i]->getLabel());
		    ButtonInterface::mapName((*list)[i]->name(), name);
		    *(wl++) = XtVaCreateWidget (name,
						xmLabelWidgetClass,
						pulldown,
						XmNlabelString, label_str,
						NULL);
		    XmStringFree(label_str);
		} else {
		    CmdInterface *ci;
		    ci  = new ButtonGadgetInterface ( pulldown, (*list)[i] );
		    *(wl++) = ci->baseWidget();
		}
		num_wl++;
	    }
	}
	XtManageChildren (head_wl, num_wl);
	delete head_wl;
    }
    return(pulldown);
}

Boolean
MenuBar::createPulldownWP(XtPointer client_data)
{
    MenuBarContext *mbCtx = (MenuBarContext *)client_data;
    XmString label_str;
    char name[200];
    int i;
    Widget *head_wl, *wl;
    Cardinal num_wl = 0;

    mbCtx->list->setPending(False);

    // Has the user quit the mailbox before we have had a chance to run?
    // If so, call the global delete routine on the list and say good-bye.
    if (mbCtx->list->isDestroyed()) {
      mbCtx->list->deleteMe();
      delete mbCtx;
      return True;
    }

    head_wl = wl = new Widget[ mbCtx->list->size() ];
    name[0] = '\0';

    // Loop through the cmdList, creating a menu 
    // entry for each command. 
    for ( i = 0; i < mbCtx->list->size(); i++) {

	if(!strcmp((*mbCtx->list)[i]->className(), "CmdList")) {
	    Widget pane = mbCtx->mb->createPulldown(mbCtx->menupane,
						    (CmdList*) (*mbCtx->list)[i],
						    (Boolean) FALSE,
						    XmMENU_BAR,
						    TRUE);
	    label_str = XmStringCreateLocalized((*mbCtx->list)[i]->getLabel());
	    XtVaSetValues(pane, 
			  XmNlabelString, label_str,
			  NULL);
	    XmStringFree(label_str);
	} else {
	    if ( !strcmp((*mbCtx->list)[i]->className(),"SeparatorCmd")) {
		*(wl++) = XtCreateWidget ( (*mbCtx->list)[i]->name(),
					  xmSeparatorWidgetClass,
					  mbCtx->menupane,
					  NULL, 0);
	    } else if (!strcmp((*mbCtx->list)[i]->className(),"ToggleButtonCmd")) {
		CmdInterface *ci;
		ci = new ToggleButtonInterface(mbCtx->menupane, (*mbCtx->list)[i] );
		*(wl++) = ci->baseWidget();
	    } else if(!strcmp((*mbCtx->list)[i]->className(),"LabelCmd")) {
		label_str = XmStringCreateLocalized((*mbCtx->list)[i]->getLabel());
                ButtonInterface::mapName((*mbCtx->list)[i]->name(), name);
                *(wl++) = XtVaCreateWidget (name,
					    xmLabelWidgetClass,
					    mbCtx->menupane,
					    XmNlabelString, label_str,
					    NULL);
	        XmStringFree(label_str);
	    } else {
		CmdInterface *ci;
		ci  = new ButtonGadgetInterface(mbCtx->menupane, (*mbCtx->list)[i]);
		*(wl++) = ci->baseWidget();
	    }
	    num_wl++;
	}
    }
    XtManageChildren (head_wl, num_wl);

    // If we have registered a function to call after the menu is
    // created, call it now.
    mbCtx->list->menuComplete();

    delete [] head_wl;
    delete mbCtx;

    return True;
}

// SR - Added to handle dynamic menus

Widget
MenuBar::addCommands(
    Widget pulldown,
    CmdList *new_list
)
{
    // if(isValidMenuPane(pulldown) == FALSE)
    // 	return NULL;

    int    i, num_children=0;
    WidgetList children=NULL;
    Boolean haveNoSeparator;
    int newItemIndex, numPBUnmanaged, tmpPBUnmanaged;

    // This means the 'menubar' type is a pulldown. 
    if (pulldown == NULL)
	pulldown = _w;

    XtVaGetValues(pulldown,
	XmNnumChildren, &num_children,
	XmNchildren, &children,
	NULL);

    // Handle the creation or management of the Separator.

    haveNoSeparator = TRUE;
    numPBUnmanaged = 0;
    
    for (i=0; (i < num_children); i++) {
	Widget wid = (Widget) children[i];
	if (XtIsSubclass(wid, xmSeparatorWidgetClass)) {
	    XtManageChild(wid);
	    haveNoSeparator = FALSE;
	}
	else if (XtIsSubclass(wid, xmPushButtonGadgetClass)) {
	    if (!XtIsManaged(wid)) { // If widget is unmanaged
		numPBUnmanaged++;
	    }
	}
    }
    if (haveNoSeparator) {
	XtCreateManagedWidget ("Separator",
		xmSeparatorWidgetClass,
		pulldown,
		NULL, 0);
	haveNoSeparator = FALSE;
    }
    
    // Now handle the pushButton case

    newItemIndex = 0;
    tmpPBUnmanaged = 0;

    // Loop through the cmdList, creating a menu 
    // entry for each command. 
	
    for (newItemIndex = 0; newItemIndex < new_list->size(); newItemIndex++) {

	tmpPBUnmanaged = numPBUnmanaged;

	if (numPBUnmanaged > 0) { // If there exists unmanaged PBs
	    for (i = 0; 
		 (i < num_children) && 
		 (tmpPBUnmanaged == numPBUnmanaged); 
		 i++) {

		Widget wid = (Widget) children[i];
		
		if (XtIsSubclass(wid, xmPushButtonGadgetClass)) {
		    if (!XtIsManaged(wid)) { 
			// If widget is unmanaged
			// Set its label to be the newItemIndex widget's.
			// Manage it.
			// Bump up newItemIndex
			XmString lstr = XmStringCreateLocalized(
                                (char *) (*new_list)[newItemIndex]->getLabel());
			XtVaSetValues(wid,
			    XmNlabelString, 
			    lstr,
			    NULL);
			XmStringFree(lstr);
			XtManageChild(wid);
			numPBUnmanaged--;
		    }
		}
	    }
	}
	else {		// No unmanaged push buttons available
	    CmdInterface *ci;
	    ci  = new ButtonGadgetInterface ( 
		pulldown, (*new_list)[newItemIndex] 
	    );
	    ci->manage();
	}
    }

    return(pulldown);
}

void
MenuBar::removeOnlyCommands(Widget pulldown, CmdList *redundant_list)
{
  int i, j;
  Cardinal num_children, dli = 0;
  WidgetList children, destroyList;
    
  XtVaGetValues(pulldown,
                XmNnumChildren, &num_children,
                XmNchildren, &children,
                NULL);
     
  destroyList = new Widget[num_children];

  // Loop through widget list.  Destroy those widgets that map to those
  // in the redundant list.
  for (i=0; i<num_children; i++) {
    Widget wid = (Widget) children[i];

    if (XtIsSubclass(wid, xmPushButtonGadgetClass)) {
      for (j=0; j<redundant_list->size(); j++) {
        char name[200];
        ButtonInterface::mapName((*redundant_list)[j]->name(), name);
        if (strcmp(XtName(wid), name) == 0) {
          // The redundant item has been found.
          destroyList[dli++] = wid;
          break;
        }
      }
    }
  }
  for (i = 0; i < dli; i++)
    XtDestroyWidget(destroyList[i]);

  delete destroyList;
}
void
MenuBar::removeCommands(Widget pulldown, CmdList *redundant_list)
{
  int i, j;
  Cardinal num_children, dli = 0;
  WidgetList children, destroyList;

  if (pulldown == NULL)
    pulldown = _w;

  XtVaGetValues(pulldown,
                XmNnumChildren, &num_children,
                XmNchildren, &children,
                NULL);

  destroyList = new Widget[num_children];

  // Loop through widget list.  Destroy those widgets that map to those in
  // the redundant list.
  for (i = 0; i < num_children; i++) {
    Widget wid = (Widget) children[i];

    if (XtIsSubclass(wid, xmSeparatorWidgetClass)) {
      destroyList[dli++] = wid;
    }
    else {
      XmString str=NULL;
      String label=NULL;
      XtVaGetValues(wid, XmNlabelString, &str, NULL);
      if (str == NULL)
        continue;
      XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &label);
      XmStringFree(str);
      if (label == NULL)
        continue;

      for (j=0; j<redundant_list->size(); j++) {
        if (strcmp(label, (*redundant_list)[j]->getLabel()) == 0) {
          // The redundant item has been found.
          destroyList[dli++] = wid;
          break;
        }
      }
      XtFree(label);
    }
  }

  for (i = 0; i < dli; i++)
    XtDestroyWidget(destroyList[i]);

  delete destroyList;
}

void
MenuBar::addCommand(
    Widget pulldown,
    Cmd *cmd
)
{
    if (pulldown == NULL)
	pulldown = _w;

    CmdInterface *ci;
    ci  = new ButtonGadgetInterface ( pulldown, cmd);
    ci->manage();
}

void
MenuBar::changeLabel(
    Widget pulldown,
    int	index,
    char * name
)
{
    // if(isValidMenuPane(pulldown) == FALSE)
    // 	return;

    int managed_widgets, i, num_children=0;
    WidgetList children=NULL;
    XmString label;
    Widget wid;

    XtVaGetValues(pulldown,
	XmNnumChildren, &num_children,
	XmNchildren, &children,
	NULL);
    
    // Some widgets may be unmanaged, so find the real index
    for (managed_widgets=0, i=0; 
		managed_widgets <= index && i < num_children; i++) {
        wid = (Widget) children[i];
        if (XtIsManaged(wid)) managed_widgets++;
    }
    if (--i >= num_children) return;

    wid = (Widget) children[i];

    label = XmStringCreateLocalized(name);

    XtVaSetValues(wid,
	XmNlabelString, label,
	NULL);
    XmStringFree(label);
}

void
MenuBar::changeLabel(Widget pulldown,
		     const char * button_name,
		     const char * label)
{
    int num_children=0;
    WidgetList children=NULL;

    XtVaGetValues(pulldown,
		  XmNnumChildren, &num_children,
		  XmNchildren, &children,
		  NULL);

    // Locate the appropriate widget in the list.
    for (int wid = 0; wid < num_children; wid++) {
	XmString button_label=NULL;
	String label_str=NULL;

	XtVaGetValues(children[wid], XmNlabelString, &button_label, NULL);
	if (button_label == NULL) continue;
	XmStringGetLtoR(button_label, XmSTRING_DEFAULT_CHARSET, &label_str);
	XmStringFree(button_label);

	if (strcmp(label_str, button_name) == 0) {
	    XtFree(label_str);
	    if (XtIsManaged(children[wid])) {
    		XmString lstr = XmStringCreateLocalized((char *)label);
	   	XtVaSetValues(children[wid],
			  XmNlabelString, lstr,
			  NULL);
    		XmStringFree(lstr);
		break;
	    }
	}
    }
}
    
void
MenuBar::rotateLabels(
    Widget pulldown,
    int startindex,
    int endindex
)
{
    // if(isValidMenuPane(pulldown) == FALSE)
    // 	return;

    int    num_managed_wids=0, i, j, num_children=0, num_to_change;
    WidgetList children=NULL;
    XmString label, endlabel;
    Widget prevwid, wid;

    if (startindex < 0 || endindex < 0)
	return;

    XtVaGetValues(pulldown,
	XmNnumChildren, &num_children,
	NULL);

    if (startindex >= num_children || endindex >= num_children)
	return;
    
    num_to_change = endindex - startindex;
    if (num_children < num_to_change || num_to_change == 0) 
	return;

    XtVaGetValues(pulldown,
	XmNchildren, &children,
	NULL);

    // Some of the widgets may be unmanaged: find the first managed widget
    for (i = startindex; i < num_children; i++) {
        if (XtIsManaged(children[i]))
		break;
	startindex++;
    }

    if (startindex == num_children) 
	return;

    // Find the real endindex
    endindex = startindex+1;
    while (endindex < num_children) {
        if (XtIsManaged(children[endindex])) 
		num_managed_wids++;
	if (num_managed_wids == num_to_change)
		// We have found the endindex at this point
		break;
	endindex++;
    } 
 
    if (endindex == num_children) 
	return;

    wid = (Widget) children[endindex];
    XtVaGetValues(wid,
	XmNlabelString, &label,
	NULL);
    endlabel = XmStringCopy(label);

    j = i = endindex; 
    while (i > startindex) {

	do {
		if (--j < startindex) break;
		prevwid = (Widget) children[j];
	} while (!XtIsManaged(prevwid));

	XtVaGetValues(prevwid,
		XmNlabelString, &label,
		NULL);

	XtVaSetValues(wid,
		XmNlabelString, label,
		NULL);

	i = j;

	wid = (Widget) children[i];
    }

    wid = (Widget) children[startindex];
    XtVaSetValues(wid,
	XmNlabelString, endlabel,
	NULL);
    
    XmStringFree(endlabel);
}

void
MenuBar::removeCommand(
    Widget pulldown,
    int index
)
{
    //if(isValidMenuPane(pulldown) == FALSE)
    //   return;

    int   managed_widgets, i, num_children=0;
    WidgetList children=NULL;

    XtVaGetValues(pulldown,
        XmNnumChildren, &num_children,
        XmNchildren, &children,
        NULL);

    // Some widgets may be unmanaged, so find the real index
    for (managed_widgets=0, i=0;
                managed_widgets <= index && i < num_children; i++) {
        Widget wid = (Widget) children[i];
        if (XtIsManaged(wid)) managed_widgets++;
    }

    if (--i < num_children)
    	XtUnmanageChild(children[i]);
}
