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
#pragma ident "%W %G"
#endif

#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <MotifApp/CmdList.h>

#ifndef _TOOLBARCONFIG_HH
#define _TOOLBARCONFIG_HH


class ToolbarConfig {
  
public:
    ToolbarConfig(Widget, Widget, Widget);
    virtual ~ToolbarConfig();

    // All XmStringTable arguments refer to the entire list of items in
    // the XmList widget.
    void update(XmStringTable, int count, Boolean);
    void useIcons(XmStringTable, Boolean);
    void insertItems(XmStringTable, int size, int count, int pos);
    void removeItem(int);
    void selectItem(int);
    void swapLabels(XmStringTable, int, int);
    void showToolbarHelp(int);
    void clearStatus();
    void buttonListCheck();
    static void buttonCB(Widget, XtPointer, XtPointer);
    static void toolbarEH(Widget, XtPointer, XEvent *, Boolean *);
    static Cardinal toolbarOrderProc(Widget);

private:
    void setLabels(XmStringTable, int, int);
    int _currently_selected;
    Widget _parent;
    Widget _container1;
    Widget _container2;
    Widget _status_help;
    Widget _cmd_list;
    DtVirtArray<Widget> *_button_list;
    Boolean _useIcons;
    Boolean _first_time;
};

#endif
