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

//#include <X11/Intrinsic.h>
#include <stdlib.h>
#include <DtMail/ToolbarConfig.hh>

#ifndef _TOOLBARLISTUIITEM_HH
#define _TOOLBARLISTUIITEM_HH


// CLASS ToolbarListUiItem
///////////////////////////////////////////////////////////
class ToolbarListUiItem : public ListUiItem {
  
public:
    ToolbarListUiItem(Widget, ToolbarConfig *, int, char*, Widget);
    virtual ~ToolbarListUiItem();
    void writeFromUiToSource();
    void writeFromSourceToUi();
    
    void handleAddButtonPress();
    void handleDeleteButtonPress();
    void handleMoveUpButtonPress();
    void handleMoveDownButtonPress();
    static void handleListItemSelection(Widget, XtPointer, XtPointer);

private:
    ToolbarConfig *_toolbar;
    Widget _cmd_list_widget;
    DtVirtArray<XmString> *_toolbar_list_save;
    DtVirtArray<XmString> *_command_list_save;

    char *convertToMailrcFormat(char *);
    static int myStrCmp(const void *,const void *);
};

#endif
